#include "PotentialMaximalCliquesEnumerator.h"
#include "MinimalSeparatorsEnumerator.h"
#include <set>

namespace tdenum {

PotentialMaximalCliquesEnumerator::PotentialMaximalCliquesEnumerator(const Graph& g) : graph(g) {}

/**
 * Returns all PMCs.
 * Iteratively finds all PMCs in subgraphs of sequencially increasing size.
 */
NodeSetSet PotentialMaximalCliquesEnumerator::get() const {
    vector<Node> nodes = graph.getNodesVector();
    int n = graph.getNumberOfNodes();
    if (n <= 0) {
        return NodeSetSet();
    }

    // P[i] will be the PMCs of Gi (the subgraph with i+1 vertices of G)
    vector<NodeSetSet> P(n);
    // D[i] will be the minimal separators of Gi
    vector<NodeSetSet> D(n);
    // If the nodes of G are {a_1,...,a_n} then P0 = {{a1}}
    P[0].insert(NodeSet(nodes[0]));
    // D[0] should remain empty

    for (int i=1; i<n; ++i) {
        // Calculate Di, the relevant subgraphs Gi-1 and Gi, and use
        // OneMoreVertex.
        // This is the main function described in the paper.
        vector<Node> subnodes = nodes;
        Node a;
        subnodes.resize(i+1);
        a = subnodes.back();
        SubGraph Gip1 = SubGraph(graph, subnodes);
        subnodes.resize(i);
        SubGraph Gi = SubGraph(graph, subnodes);
        MinimalSeparatorsEnumerator DiEnumerator(Gip1, UNIFORM);
        while (DiEnumerator.hasNext()) {
            D[i].insert(DiEnumerator.next());
        }
        P[i] = OneMoreVertex(Gip1, Gi, a, D[i], D[i-1], P[i-1]);
    }

    return P[n-1];
}


NodeSetSet PotentialMaximalCliquesEnumerator::OneMoreVertex(
                  const SubGraph& G1, const SubGraph& G2, Node a,
                  const NodeSetSet& D1, const NodeSetSet& D2,
                  const NodeSetSet& P2) const {
    NodeSetSet P1;
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
    }
    for (auto Sit = D1.begin(); Sit != D1.end(); ++Sit) {
        NodeSet S = *Sit;
        NodeSet Sa = S;
        Sa.insert(Sa.end(), a);
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
                if (components[i] != adjacent[i]) {
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
bool PotentialMaximalCliquesEnumerator::IsPMC(NodeSet K, const Graph& G) const {
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
    }

    // For each x,y in K (that aren't equal) we need to check if
    // they're neighbors in G or both contained in some Si.
    for (i=0; i<K.size(); ++i) {
        Node x = K[i];
        // Find the S[i]s containing x
        vector<NodeSet> Sx;
        for (j=0; j<S.size(); ++i) {
            // They're all sorted, so use binary search
            if (std::binary_search(S[i].begin(), S[i].end(), x)) {
                Sx.insert(Sx.end(),S[i]);
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
            }
            if (!foundSi) {
                // x and y aren't connected in F...
                return false;
            }
        }
    }

    return true;
}


} /* namespace tdenum */
