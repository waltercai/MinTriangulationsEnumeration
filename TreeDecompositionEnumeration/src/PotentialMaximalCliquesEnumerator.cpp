#include "PotentialMaximalCliquesEnumerator.h"
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
        return vector<NodeSet>();
    }

    // P[i] will be the PMCs of Gi (the subgraph with i+1 vertices of G)
    vector<NodeSetSet> P(n);
    // D[i] will be the minimal separators of Gi
    vector<NodeSetSet> D(n);
    // If the nodes of G are {a_1,...,a_n} then P0 = {{a1}}
    P[0].insert({nodes[0]});
    // D[0] should remain empty

    for (int i=1; i<n; ++i) {
        // Calculate Di, the relevant subgraphs Gi-1 and Gi, and use
        // OneMoreVertex.
        // This is the main function described in the paper.
        vector<Node> subnodes = nodes;
        Node a;
        subnodes.resize(i+1);
        a = subnodes.last();
        Gip1 = SubGraph(graph, subnodes)
        subnodes.resize(i);
        Gi = SubGraph(graph, subnodes)
        auto DiEnumerator = MinimalSeparatorsEnumerator(Gip1, UNIFORM);
        while (DiEnumerator.hasNext()) {
            D[i].insert(DiEnumerator.next());
        }
        P[i] = OneMoreVertex(Gip1, Gi, a, D[i], D[i-1], P[i-1]);
    }

    return P[n-1];
}


} /* namespace tdenum */
