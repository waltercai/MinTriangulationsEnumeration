#include "PMCEnumerator.h"
#include "MinimalSeparatorsEnumerator.h"
#include "Utils.h"
#include <set>

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




PMCEnumerator::PMCEnumerator(const Graph& g, time_t time_limit) :
    graph(g), done(false), limit(time_limit), start_time(time(NULL)), out_of_time(false) {}

void PMCEnumerator::reset(const Graph& g, time_t time_limit) {
    graph = g;
    done = false;
    limit = time_limit;
    start_time = time(NULL);
    out_of_time = false;
}

/**
 * Returns all PMCs (updates PMCs and MSs in the class)
 */
NodeSetSet PMCEnumerator::get() {
    if (!done) {
        NodeSetSet nss = getConnected(graph);
        done = true;
        pmcs = nss;
    }
    return pmcs;
}

/**
 * Returns minimal separators.
 */
NodeSetSet PMCEnumerator::get_ms() {
    if (!done) {
        get();
    }
    return MS;
}

/**
 * Returns all PMCs.
 * Iteratively finds all PMCs in subgraphs of sequencially increasing size.
 * Assumes g is a connected graph.
 */
NodeSetSet PMCEnumerator::getConnected(const SubGraph& g) {
    TRACE(TRACE_LVL__NOISE, "In with G=\n" << g << "Getting nodes vector...");
    vector<Node> nodes = g.getNodesVector();
    int n = g.getNumberOfNodes();
    if (n <= 0) {
        return NodeSetSet();
    }

    // Each iteration requires data on two graphs: Gi and Gip1 (i plus 1).
    NodeSetSet PMCi, PMCip1, MSi, MSip1;
    vector<SubGraph> subg;

    // Start by creating all subgraphs.
    // This should not constitute a memory bottleneck... if so,
    // the graphs are WAY too large for these algorithms :)
    for (int i=1; i<=n; ++i) {
        vector<Node> subnodes = nodes;
        subnodes.resize(i);
        subg.push_back(SubGraph(g, subnodes));
    }

    // If the nodes of G are {a_1,...,a_n} then P1 = {{a1}}
    NodeSet firstSet(1);
    firstSet[0] = nodes[0];
    PMCip1.insert(firstSet); // Later, PMCi=PMCip1

    // MS1 should remain empty, so MSi=MSip1 is OK

    for (int i=1; i<n; ++i) {
        // Calculate MSip1 and PMCip1, and use OneMoreVertex.
        // This is the main function described in the paper.
        PMCi = PMCip1;
        MSi = MSip1;
        MSip1.clear();
        Node a = nodes[i];

        // Calculate MSip1
        MinimalSeparatorsEnumerator DiEnumerator(subg[i], UNIFORM);
        while (DiEnumerator.hasNext()) {
            MSip1.insert(DiEnumerator.next());
        }

        // Calculate PMCip1
        PMCip1 = OneMoreVertex(subg[i], subg[i-1], a, MSip1, MSi, PMCi);
        TRACE(TRACE_LVL__DEBUG, "OneMoreVertex in iteration " << i
              << " with the graph G(i+1):" << endl << subg[i]
              << "got PMCs:" << endl << PMCip1);
    }

    // Update the minimal separators
    MS = MSip1;

    // That's it!
    return PMCip1;
}


NodeSetSet PMCEnumerator::OneMoreVertex(
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
        if (IsPMC(*pmc2it, G1)) {
            P1.insert(*pmc2it);
        }
        else {
            NodeSet pmc2a = *pmc2it;
            pmc2a.insert(pmc2a.end(), a);
            if (IsPMC(pmc2a, G1)) {
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
        if (IsPMC(Sa, G1)) {
            P1.insert(Sa);
        }
        if (std::find(S.begin(), S.end(), a) == S.end() && !D2.isMember(S)) {
            // For each separator S, iterate over all full components C of G
            // associated with S. In other words, all connected components C
            // of G\S so that the set P of all elements of S that are adjacent
            // to some vertex of C supports P=S.
            // Sort S first so we can easily compare P=S later.
            std::sort(S.begin(), S.end());
            vector<NodeSet> components = G1.getComponents(S);
            vector<NodeSet> adjacent(components.size());
            for (unsigned int i=0; i<components.size(); ++i) {
                // Sort the adjacency vector so we can easily compare the vectors later
                adjacent[i] = G1.getAdjacent(components[i], S);
                std::sort(adjacent[i].begin(), adjacent[i].end());
            }
            for (unsigned int i=0; i<components.size(); ++i) {
                // We only want full components
                if (S != adjacent[i]) {
                    continue;
                }
                for (auto sep2 = D2.begin(); sep2 != D2.end(); ++sep2) {
                    NodeSet TcapC;
                    std::set_intersection(sep2->begin(), sep2->end(),
                                          components[i].begin(), components[i].end(),
                                          std::back_inserter(TcapC));
                    NodeSet SuTcapC;
                    std::set_union(TcapC.begin(), TcapC.end(),
                                   S.begin(), S.end(),
                                   std::back_inserter(SuTcapC));
                    if (IsPMC(SuTcapC, G1)) {
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

bool PMCEnumerator::IsPMC(NodeSet K, const SubGraph& G) {
    vector<NodeSet> C = G.getComponents(K);
    vector<NodeSet> S(C.size());
    unsigned int i,j,k;


    // Sort K so we can compare it easily to other vectors
    std::sort(K.begin(), K.end());

    // Build the sets Si.
    // While doing so make sure we don't have any full components
    for (i=0; i<C.size(); ++i) {
        S[i] = G.getAdjacent(C[i], K);
        std::sort(S[i].begin(), S[i].end());
        if (S[i] == K) {
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
        for (j=0; j<S.size(); ++j) {
            // They're all sorted, so use binary search
            if (std::binary_search(S[j].begin(), S[j].end(), x)) {
                Sx.push_back(S[j]);
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




