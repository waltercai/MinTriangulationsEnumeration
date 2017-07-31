#include "PMCEnumerator.h"
#include "MinimalSeparatorsEnumerator.h"
#include "Utils.h"
#include <set>
#include <algorithm>

namespace tdenum {

#define CHECK_TIME_OR_RETURN(_x) do { \
        if (limit == 0) { \
            break; \
        } \
        time_t _t = time(NULL); \
        if (_t - start_time >= limit) { \
            out_of_time = true; \
            return _x; \
        } \
    } while(0)


const string PMCEnumerator::alg_names[PMCEnumerator::ALG_LAST+1] = {
    #define X(name) #name,
        PMC_ALG_TABLE
        "Invalid algorithm"
    #undef X
    };

PMCEnumerator::PMCEnumerator(const Graph& g, time_t time_limit) :
        graph(g),
        alg(default_alg),
        has_ms(false),
        done(false),
        limit(time_limit),
        start_time(time(NULL)),
        out_of_time(false)
{
    ms.clear();
    pmcs.clear();
    // Use current names as originals
    graph.forgetOriginalNames();
}

void PMCEnumerator::reset(const Graph& g, time_t time_limit) {
    *this = PMCEnumerator(g, time_limit);
}

void PMCEnumerator::set_algorithm(Alg a) {
    alg = a;
}
PMCEnumerator::Alg PMCEnumerator::get_alg() const {
    return alg;
}
string PMCEnumerator::get_alg_name(Alg a) {
    return PMCEnumerator::alg_names[a];
}
string PMCEnumerator::get_alg_name(int a) {
    return PMCEnumerator::get_alg_name(PMCEnumerator::Alg(a));
}

/**
 * Allows caller to report the minimal separators of the graph.
 * Remember to map the sets to the new node names!
 */
void PMCEnumerator::set_minimal_separators(const NodeSetSet& min_seps) {
    ms = graph.getNewNames(min_seps);
    has_ms = true;
}

/**
 * Returns minimal separators.
 */
NodeSetSet PMCEnumerator::get_ms() {
    if (!has_ms) {
        ms.clear();
        MinimalSeparatorsEnumerator mse(graph, UNIFORM);
        ms = mse.getAll();
        has_ms = true;
    }
    return ms;
}

Graph PMCEnumerator::get_graph() const {
    return graph;
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
NodeSetSet PMCEnumerator::get() {
    if (!done) {

        // Cleanup
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
        if (ALG_IS_SORTING_STRAIN(alg)) {
            tmp_graph.sortNodesByDegree(ALG_IS_SORTING_ASCENDING_STRAIN(alg));
        }
        else if (ALG_IS_RANDOM_RENAME_STRAIN(alg)) {
            tmp_graph.randomNodeRename();
        }
        vector<Node> nodes = tmp_graph.getNodesVector();

        // Start by creating all subgraphs.
        // This should not constitute a memory bottleneck... if so,
        // the graphs are WAY too large for these algorithms :)
        vector<SubGraph> subg;
        for (int i=1; i<=n; ++i) {
            vector<Node> subnodes = nodes;
            subnodes.resize(i);
            subg.push_back(SubGraph(tmp_graph, subnodes));
        }

        // Optionally use the (memory-inefficient) algorithm, which
        // calculates the minimal separators in advance:
        vector<NodeSetSet> sub_ms(n);
        if (ALG_IS_REVERSE_MS_STRAIN(alg)) {

            // Calculate the first set of minimal separators
            sub_ms[n-1] = tmp_graph.getNewNames(get_ms());

            // Use the algorithm described in the PDF
            for (int i=n-2; i>=0; --i) {

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
            }
        }

        // Now, back to the main algorithm.
        // If the minimal separators weren't pre-calculated, use
        // extra variables Msi and MSip1. Each iteration requires data on two
        // graphs: Gi and Gip1 (i plus 1).
        // Use pmcs instead of PMCip1 to save space and a copy at the end.
        NodeSetSet prev_pmcs, MSi, MSip1;

        // If the nodes of G are {a_1,...,a_n} then P1 = {{a1}}
        pmcs.insert(NodeSet({nodes[0]})); // Later, PMCi=PMCip1

        // MS1 should remain empty, so MSi=MSip1 is OK
        for (int i=1; i<n; ++i) {
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
            if (!ALG_IS_REVERSE_MS_STRAIN(alg)) {
                if (i == n-1) {
                    pmcs = one_more_vertex(subg[i], subg[i-1], a, tmp_graph.getNewNames(get_ms()), MSi, prev_pmcs);
                }
                else {
                    MinimalSeparatorsEnumerator DiEnumerator(subg[i], UNIFORM);
                    MSip1 = DiEnumerator.getAll();
                    pmcs = one_more_vertex(subg[i], subg[i-1], a, MSip1, MSi, prev_pmcs);
                }
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
            ms = tmp_graph.getOriginalNames(ALG_IS_REVERSE_MS_STRAIN(alg) ? sub_ms[n-1] : MSip1);
            has_ms = true;
        }

        // That's it! Translate to user-friendly state
        // pmcs now contains the correct set of PMCs.
        pmcs = tmp_graph.getOriginalNames(pmcs);
        done = true;
    }

    return pmcs;
}


NodeSetSet PMCEnumerator::one_more_vertex(
                  const SubGraph& G1, const SubGraph& G2, Node a,
                  const NodeSetSet& D1, const NodeSetSet& D2,
                  const NodeSetSet& P2) {
    NodeSetSet P1;

    // If a supports d(a)=0, then the regular algorithm won't add
    // {a} as a PMC, even though it should.
    // However, in this case: P1=P2 U {{a}} (a statement proven separately),
    // so we can simply catch this case and return the correct value of P1
    if (G1.d(a) == 0) {
        P1=P2;
        P1.insert(NodeSet({a}));
        return P1;
    }

    for (auto pmc2it = P2.begin(); pmc2it != P2.end(); ++pmc2it) {
        if (is_pmc(*pmc2it, G1)) {
            P1.insert(*pmc2it);
        }
        else {
            NodeSet pmc2a = *pmc2it;
            pmc2a.insert(pmc2a.end(), a); // should already be sorted as a is bigger than previous nodes
            if (is_pmc(pmc2a, G1)) {
                P1.insert(pmc2a);
            }
        }
        CHECK_TIME_OR_RETURN(P1);
    }
    for (auto Sit = D1.begin(); Sit != D1.end(); ++Sit) {
        NodeSet S = *Sit;
        NodeSet Sa = S;
        // Add a, if not already in:
        if (!std::binary_search(Sa.begin(), Sa.end(), a)) {
            Sa.insert(Sa.end(), a);
        }
        if (is_pmc(Sa, G1)) {
            P1.insert(Sa);
        }
        if (UTILS__IS_IN_CONTAINER(a,S) && !D2.isMember(S)) {
            // For each separator S, iterate over all full components C of G
            // associated with S. In other words, all connected components C
            // of G\S so that the set P of all elements of S that are adjacent
            // to some vertex of C supports P=S.
            // Sort S first so we can easily compare P=S later.
            //std::sort(S.begin(), S.end());

			BlockVec blocks = G1.getBlocks(S);
			for (unsigned int i=0; i<blocks.size(); ++i) {
                // We only want full components
                if (S != blocks[i]->S) {
                    continue;
                }
                for (auto sep2 = D2.begin(); sep2 != D2.end(); ++sep2) {
                    NodeSet TcapC;
                    std::set_intersection(sep2->begin(), sep2->end(),
                                          blocks[i]->C.begin(), blocks[i]->C.end(),
                                          std::back_inserter(TcapC));
                    NodeSet SuTcapC;
                    std::set_union(TcapC.begin(), TcapC.end(),
                                   S.begin(), S.end(),
                                   std::back_inserter(SuTcapC));
                    if (is_pmc(SuTcapC, G1)) {
                        P1.insert(SuTcapC);
                    }
                    CHECK_TIME_OR_RETURN(P1);
                }
            }
        }
        CHECK_TIME_OR_RETURN(P1);
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
        //std::sort(S[i].begin(), S[i].end());
        if (B[i]->S == K) {
            // Uh oh.. C[i] is a full component
            return false;
        }
        CHECK_TIME_OR_RETURN(false);
    }

    // For each x,y in K (that aren't equal) we need to check if
    // they're neighbors in G or both contained in some Si.
    for (i=0; i<K.size(); ++i) {
        Node x = K[i];
        // Find the S[i]s containing x
        vector<NodeSet> Sx;
        for (j=0; j<B.size(); ++j) {
            // They're all sorted, so use binary search
            if (std::binary_search(B[j]->S.begin(), B[j]->S.end(), x)) {
                Sx.push_back(B[j]->S);
            }
        }
        // For every unchecked y in K (scanning forward) check adjacency
        // in the graph F
        for (j=i+1; j<K.size(); ++j) {
            Node y = K[j];
            if (G.areNeighbors(x, y)) {
                continue;
            }
            bool foundSi = false;
            for (k=0; k<Sx.size(); ++k) {
                if (std::binary_search(Sx[k].begin(), Sx[k].end(), y)) {
                    foundSi = true;
                    break;
                }
                CHECK_TIME_OR_RETURN(false);
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




