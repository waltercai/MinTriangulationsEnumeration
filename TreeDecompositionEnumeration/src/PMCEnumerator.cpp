#include "PMCEnumerator.h"
#include "MinimalSeparatorsEnumerator.h"
#include "Utils.h"
#include <set>
#include <algorithm>

namespace tdenum {

int PMCE_RUNMODE = PMCE_RUNMODE_FAST;

/**
 * A macro used to stop everything and return if the time
 * limit is reached.
 *
 * Parallelized code can't do that, so use workarounds..
 */
#define CHECK_TIME_OR_OP(_op) do { \
        if (limit != 0 && difftime(time(NULL),start_time) >= limit) { \
            out_of_time = true; \
            TRACE(TRACE_LVL__WARNING, "Out of time!"); \
            _op; \
        } \
    } while(0)

/**
 * Verify the given container is sorted (check the runmode first).
 * Should be used in the parallel context of one_more_vertex or
 * is_pmc, respectively.
 */
#define VERIFY_SORT_OMV(_cont) do { \
        if (PMCE_RUNMODE == PMCE_RUNMODE_VERIFY_SORT) { \
            if (!std::is_sorted((_cont).begin(), (_cont).end())) { \
                TRACE(TRACE_LVL__ERROR, "Unsorted NodeSet " #_cont ": " << (_cont) << ". Returning an empty set"); \
                keep_running = false; \
                P1 = NodeSetSet(); \
                continue; \
            } \
        } \
    } while(0)

#define VERIFY_SORT_ISPMC(_cont) do { \
        if (PMCE_RUNMODE == PMCE_RUNMODE_VERIFY_SORT) { \
            if (!std::is_sorted((_cont).begin(), (_cont).end())) { \
                TRACE(TRACE_LVL__ERROR, "Unsorted NodeSet " #_cont ": " << (_cont) << ". Returning FALSE"); \
                return false; \
            } \
        } \
    } while(0)


const PMCAlg PMCEnumerator::default_alg = PMCAlg();

PMCEnumerator::PMCEnumerator(const Graph& g, time_t time_limit) :
        graph(g),
        alg(default_alg),
        has_ms(false),
        ms_subgraph_count(g.getNumberOfNodes()),
        allow_parallel(false),
        done(false),
        limit(time_limit),
        start_time(time(NULL)),
        out_of_time(false)
{
    omp_init_lock(&lock);
    ms.clear();
    pmcs.clear();
    // Use current names as originals
    graph.forgetOriginalNames();
}
PMCEnumerator::~PMCEnumerator() {
    omp_destroy_lock(&lock);
}

void PMCEnumerator::reset(const Graph& g, time_t time_limit) { *this = PMCEnumerator(g, time_limit); }

void PMCEnumerator::set_algorithm(PMCAlg a) { alg = a; }
PMCAlg PMCEnumerator::get_alg() const { return alg; }

void PMCEnumerator::set_time_limit(time_t t) { limit = t; }
void PMCEnumerator::unset_time_limit() { limit = 0; }

void PMCEnumerator::enable_parallel() { allow_parallel = true; }
void PMCEnumerator::suppress_parallel() { allow_parallel = false; }

/**
 * Allows caller to report the minimal separators of the graph.
 * Remember to map the sets to the new node names!
 */
void PMCEnumerator::set_minimal_separators(const NodeSetSet& min_seps) {
    ms = graph.getNewNames(min_seps);
    has_ms = true;
    if (graph.getNumberOfNodes() == 0) {
        TRACE(TRACE_LVL__ERROR, "No nodes (empty graph)! Can't set MS count");
        return;
    }
    ms_subgraph_count[graph.getNumberOfNodes()-1] = ms.size();
}

/**
 * Returns minimal separators.
 */
NodeSetSet PMCEnumerator::get_ms() {
    if (!has_ms) {
        ms.clear();
        if (graph.getNumberOfNodes() == 0) {
            TRACE(TRACE_LVL__NOISE, "Empty graph yields an empty set of minimal separators.");
            set_minimal_separators(NodeSetSet());
            return ms;
        }
        MinimalSeparatorsEnumerator mse(graph, UNIFORM);
        if (!mse.getAll(ms, difftime(limit,difftime(time(NULL),start_time)))) {
            out_of_time = true;
        }
        ms_subgraph_count[graph.getNumberOfNodes()-1] = ms.size();
        has_ms = true;
    }
    return ms;
}
vector<NodeSetSet> PMCEnumerator::get_ms_subgraphs() { return ms_subgraphs; }
vector<long> PMCEnumerator::get_ms_count_subgraphs() { return ms_subgraph_count; }

Graph PMCEnumerator::get_graph() const {
    return graph;
}

void PMCEnumerator::update_ms_subgraph_count() {
    read_ms_subgraph_count_from_vector(ms_subgraphs);
}
void PMCEnumerator::read_ms_subgraph_count_from_vector(const vector<NodeSetSet>& v) {
    if (v.size() != (size_t)graph.getNumberOfNodes()) {
        TRACE(TRACE_LVL__ERROR, "Size mismatch: got " << v.size() << " separators " <<
              "for a graph of size " << graph.getNumberOfNodes());
        return;
    }
    for (unsigned i=0; i<v.size(); ++i) {
        ms_subgraph_count[i] = v[i].size();
    }
}

/**
 * Returns all PMCs.
 * Iteratively finds all PMCs in subgraphs of sequentially increasing size.
 *
 * The NORMAL algorithm iteratively calculates the required minimal separators
 * from scratch.
 *
 * The REVERSE_MS_PRECALC algorithm starts by calculating all sets of minimal
 * separators - in reverse order - before starting the PMC algorithm.
 */
NodeSetSet PMCEnumerator::get(const StatisticRequest& sr) {

    if (!done) {

        // Cleanup
        start_time = time(NULL);
        pmcs.clear();

        /**
         * Make a temporary copy of the graph to work with.
         * As we may be renaming nodes, simply use the temp graph and
         * then convert the resulting ms and pmc values to the original
         * node names.
         */
        Graph tmp_graph = graph;
        int n = tmp_graph.getNumberOfNodes();

        // If the graph is empty..
        if (n <= 0) {
            done = true;
            return NodeSetSet();
        }

        // If the algorithm requires sorting, do so:
        if (alg.is_sorted()) {
            tmp_graph.sortNodesByDegree(alg.is_ascending());
        }
        else if (alg.is_random_node_rename()) {
            tmp_graph.randomNodeRename();
        }
        vector<Node> nodes = tmp_graph.getNodesVector();

        TRACE(TRACE_LVL__NOISE, "Done renaming. Calculating subgraphs...");

        // Start by creating all subgraphs.
        // This should not constitute a memory bottleneck... if so,
        // the graphs are WAY too large for these algorithms :)
        vector<SubGraph> subg;
        for (int i=1; i<=n; ++i) {
            vector<Node> subnodes = nodes;
            subnodes.resize(i);
            subg.push_back(SubGraph(tmp_graph, subnodes));
        }

        TRACE(TRACE_LVL__NOISE, "Done calculating subgraphs. Is the algorithm a reverse-MS type?");

        // Optionally use the (memory-inefficient) algorithm, which
        // calculates the minimal separators in advance:
        vector<NodeSetSet> sub_ms(n);
        sub_ms[n-1] = tmp_graph.getNewNames(get_ms());
        if (alg.is_reverse()) {

            TRACE(TRACE_LVL__NOISE, "Yes!");


            // Calculate the first set of minimal separators
            CHECK_TIME_OR_OP(return NodeSetSet());

            // Use the algorithm described in the PDF
            for (int i=n-2; i>=0; --i) {

                TRACE(TRACE_LVL__NOISE, "In reverse MS iteration, i=" << i);

                sub_ms[i].clear();

                // To prevent checking both S and S u {v}, if they exist
                NodeSetSet checked_seps;

                // For all S in the parent minimal separators:
                for (auto it=sub_ms[i+1].begin(); it!=sub_ms[i+1].end(); ++it) {
                    // Get S
                    NodeSet S = *it;

                    // S <- S\{v}
                    // Assume S is sorted in ascending order, and that each subgraph
                    // includes the i smallest nodes.
                    // Note that S may be the empty set - the same logic applies (the
                    // empty has two full components <==> the graph isn't connected, and
                    // then S is indeed a minimal separator).
                    if (!S.empty() && S[S.size()-1] == nodes[i+1]) {
                        S.pop_back();
                    }
                    // Make sure it hasn't been checked before
                    if (checked_seps.isMember(S)) {
                        continue;
                    }
                    // Get connected components of subg[i](S)
                    auto components = subg[i].getComponents(S);
                    // Count how many are full components. Stop at 2.
                    int full_components = 0;
                    for (auto C: components) {
                        if (subg[i].isFullComponent(C,S)) {
                            ++full_components;
                            if (full_components >= 2) {
                                break;
                            }
                        }
                    }
                    // Add S if there were at least two full components
                    if (full_components >= 2) {
                        sub_ms[i].insert(S);
                    }
                }
                ms_subgraph_count[i] = sub_ms[i].size();
            }
        }

        // Now, back to the main algorithm.
        // If the minimal separators weren't pre-calculated, use
        // extra variables Msi and MSip1. Each iteration requires data on two
        // graphs: Gi and Gip1 (i plus 1).
        // Use pmcs instead of PMCip1 to save space and a copy at the end.
        NodeSetSet prev_pmcs, MSi, MSip1;
        MSip1.clear();
        MSi.clear();

        // If the nodes of G are {a_1,...,a_n} then P1 = {{a1}}
        pmcs.insert(NodeSet({nodes[0]})); // Later, PMCi=PMCip1

        TRACE(TRACE_LVL__NOISE, "Done with reverse-MS part, starting main loop...");

        // MS1 should remain empty, so MSi=MSip1 is OK
        for (int i=1; i<n; ++i) {

            TRACE(TRACE_LVL__NOISE, "In PMC iterations after MS iterations, i=" << i);

            // Calculate MSip1 and PMCip1, and use OneMoreVertex.
            // This is the main function described in the paper.
            prev_pmcs = pmcs;
            MSi = MSip1;
            pmcs.clear();
            MSip1.clear();
            Node a = nodes[i];

            // Calculate MSip1 and then the next set of PMCs.
            // If there's no need, just run the next function.

            // The NORMAL algorithm requires calculation of separators
            if (!alg.is_reverse()) {
                if (i == n-1) {
                    TRACE(TRACE_LVL__OFF, "Last iteration, moving from:" << endl << subg[i-1] <<
                                           "To (by adding node " << a << "):" << endl << subg[i] <<
                                           "With minimal separators " << MSi << " and " <<
                                           tmp_graph.getNewNames(get_ms()) << ", respectively.");
                    pmcs = one_more_vertex(subg[n-1], subg[n-2], a, sub_ms[n-1], MSi, prev_pmcs);
                }
                else {
                    MinimalSeparatorsEnumerator DiEnumerator(subg[i], UNIFORM);
                    DiEnumerator.getAll(MSip1, difftime(limit,difftime(time(NULL),start_time)));
                    CHECK_TIME_OR_OP(return NodeSetSet());
                    sub_ms[i] = MSip1;
                    ms_subgraph_count[i] = MSip1.size();
                    pmcs = one_more_vertex(subg[i], subg[i-1], a, MSip1, MSi, prev_pmcs);
                }
                TRACE(TRACE_LVL__OFF, "Current pmcs: " << tmp_graph.getOriginalNames(pmcs));
            }
            else {
                pmcs = one_more_vertex(subg[i], subg[i-1], a, sub_ms[i], sub_ms[i-1], prev_pmcs);
                TRACE(TRACE_LVL__OFF, "With i=" << i << ", where the parent graph is:" << endl
                      << subg[i] << "and the subgraph is:" << endl << subg[i-1]
                      << "We have minimal separators " << sub_ms[i] << " and " << sub_ms[i-1]
                      << ", main graph / subgraph respectively. As a result, we got PMCs " << pmcs);
            }
        }

        // Update the minimal separators
        if (!has_ms) {
            ms = tmp_graph.getOriginalNames(alg.is_reverse() ? sub_ms[n-1] : MSip1);
            ms_subgraph_count[n-1] = ms.size();
            has_ms = true;
        }
        if (sr.test_ms_subgraphs()) {
            TRACE(TRACE_LVL__TEST, "Setting MS subgraphs to " << sub_ms);
            ms_subgraphs = sub_ms;
            update_ms_subgraph_count();
        }
        else if (sr.test_ms_subgraph_count()) {
            read_ms_subgraph_count_from_vector(sub_ms);
        }

        // That's it! Translate to user-friendly state
        // pmcs now contains the correct set of PMCs.
        pmcs = tmp_graph.getOriginalNames(pmcs);
        done = true;
    }

    // Update
    CHECK_TIME_OR_OP(((void)0));
    return pmcs;
}

/**
 * May use asynchronous code.
 * Note that in such code, the only shared variables used by
 * threads are P1 and G1. P1 is always wrapped by a locking method
 * insert_critical, and G1 is always sent as a const argument to
 * is_pmc, so it should be fine.
 * 'a' is only read.
 */
NodeSetSet PMCEnumerator::one_more_vertex(
                  const SubGraph& G1, const SubGraph& G2, Node a,
                  const NodeSetSet& D1, const NodeSetSet& D2,
                  const NodeSetSet& P2) {
    NodeSetSet P1;
    bool keep_running = true;   // For async code

    // If a supports d(a)=0, then the regular algorithm won't add
    // {a} as a PMC, even though it should.
    // However, in this case: P1=P2 U {{a}} (a statement proven separately),
    // so we can simply catch this case and return the correct value of P1
    if (G1.d(a) == 0) {
        P1=P2;
        P1.insert(NodeSet({a}));
        return P1;
    }

    TRACE(TRACE_LVL__NOISE, "Starting first parallel loop...");

    #pragma omp parallel if(allow_parallel)
    {
        for (auto pmc2it=P2.begin(); keep_running && pmc2it != P2.end(); ++pmc2it) {
            #pragma omp single nowait
            {
                auto potential = *pmc2it;
                if (is_pmc(potential, G1)) {
                    utils__insert_critical(potential, P1);
                }
                else {
                    NodeSet pmc2a = potential;
                    pmc2a.insert(pmc2a.end(), a); // should already be sorted as a is bigger than previous nodes
                    if (is_pmc(pmc2a, G1)) {
                        utils__insert_critical(pmc2a, P1);
                    }
                }
                #pragma omp critical
                {
                    CHECK_TIME_OR_OP(keep_running = false);
                }
            }
        }
    }
    CHECK_TIME_OR_OP(return P1);

    TRACE(TRACE_LVL__NOISE, "Done with first parallel loop, starting second...");

    #pragma omp parallel if(allow_parallel)
    {
        for (auto Sit = D1.begin(); keep_running && Sit != D1.end(); ++Sit) {
            #pragma omp single nowait
            {
                // Sort S first so we can easily compare P=S later.
                NodeSet S = *Sit;
                NodeSet Sa = S;
                VERIFY_SORT_OMV(Sa);

                // Add a, if not already in:
                if (!UTILS__IS_IN_SORTED_CONTAINER(a,Sa)) {
                    Sa.insert(Sa.end(), a);
                }
                if (is_pmc(Sa, G1)) {
                    utils__insert_critical(Sa, P1);
                }
                if (!UTILS__IS_IN_SORTED_CONTAINER(a,S) && !D2.isMember(S)) {

                    // For each separator S, iterate over all full components C of G
                    // associated with S. In other words, all connected components C
                    // of G\S so that the set P of all elements of S that are adjacent
                    // to some vertex of C supports P=S.

                    BlockVec blocks = G1.getBlocks(S);
                    for (unsigned int i=0; keep_running && i<blocks.size(); ++i) {
                        // We only want full components
                        if (S != blocks[i]->S) {
                            continue;
                        }
                        for (auto sep2 = D2.begin(); keep_running && sep2 != D2.end(); ++sep2) {
                            VERIFY_SORT_OMV(*sep2);
                            VERIFY_SORT_OMV(blocks[i]->C);
                            NodeSet TcapC;
                            UTILS__VECTOR_INTERSECTION(*sep2, blocks[i]->C, TcapC);
                            VERIFY_SORT_OMV(TcapC);
                            NodeSet SuTcapC;
                            UTILS__VECTOR_UNION(TcapC, S, SuTcapC);  // S is verified sorted (above)
                            VERIFY_SORT_OMV(SuTcapC);
                            if (is_pmc(SuTcapC, G1)) {
                                utils__insert_critical(SuTcapC, P1);
                            }
                            #pragma omp critical
                            {
                                CHECK_TIME_OR_OP(keep_running = false);
                            }
                        }
                    }
                }
                #pragma omp critical
                {
                    CHECK_TIME_OR_OP(keep_running = false);
                }
            }
        }
    }

    return P1;
}

/**
 * Uses theorem 8 in the paper.
 *
 * First, check if G has no full components associated with K.
 *
 * Next, we need to make sure F is a clique, where F is the graph constructed like so:
 * - Start with G
 * - For every connected component Ci in G, denote Si as the set of vertices in K that
 *   are adjacent to some vertex of Ci
 * - For every Si, 'complete' Si in G: meaning, make Si a clique in G. Denote the result G'.
 * - If G'=(V,E'), denote F=(K,E') (induced graph from G' using the vertices in K).
 * To do so, for every x in K find all elements y in K that are adjacent to x and all elements
 * z that are connected to x via some path P contained in some component Ci.
 * If, for every x in K, #TotalY + #TotalZ = |K|-1 then every x is adjacent to all other vertices
 * of K, so F is a clique and K is a PMC.
 * This works because if x isn't adjacent to y in K, then it'll be adjacent to y in F <==>
 * x,y are both in some (same) Si, meaning there's a path from x to y contained entirely in
 * Ci apart from the endpoints x and y.
 */

bool PMCEnumerator::is_pmc(NodeSet K, const SubGraph& G) {
	BlockVec B = G.getBlocks(K);
	unsigned int i,j,k;


    // Sort K so we can compare it easily to other vectors
    //std::sort(K.begin(), K.end());

    // Build the sets Si.
    // While doing so make sure we don't have any full components
    for (i=0; i<B.size(); ++i) {
        VERIFY_SORT_ISPMC(B[i]->S); // S must be sorted for the algorithm to work
        if (B[i]->S == K) {
            // Uh oh.. C[i] is a full component
            return false;
        }
        CHECK_TIME_OR_OP(return false);
    }

    // For each x,y in K (that aren't equal) we need to check if
    // they're neighbors in G or both contained in some Si.
    for (i=0; i<K.size(); ++i) {
        Node x = K[i];
        // Find the S[i]s containing x
        vector<NodeSet> Sx;
        for (j=0; j<B.size(); ++j) {
            CHECK_TIME_OR_OP(return false);
            // They're all sorted, so use binary search
            if (UTILS__IS_IN_SORTED_CONTAINER(x, B[j]->S)) {
                Sx.push_back(B[j]->S);
            }
        }
        // For every unchecked y in K (scanning forward) check adjacency
        // in the graph F
        for (j=i+1; j<K.size(); ++j) {
            CHECK_TIME_OR_OP(return false);
            Node y = K[j];
            if (G.areNeighbors(x, y)) {
                continue;
            }
            bool foundSi = false;
            for (k=0; k<Sx.size(); ++k) {
                // Sx is sorted by construction (otherwise, S is unsorted),
                // so IS_IN_CONTAINER can be safely used.
                if (UTILS__IS_IN_SORTED_CONTAINER(y, Sx[k])) {
                    foundSi = true;
                    break;
                }
                CHECK_TIME_OR_OP(return false);
            }
            if (!foundSi) {
                // x and y aren't connected in F...
                return false;
            }
        }
    }

    return true;
}

bool PMCEnumerator::is_out_of_time() const {
    return out_of_time;
}

} /* namespace tdenum */




