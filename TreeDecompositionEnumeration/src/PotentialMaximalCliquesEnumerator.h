#ifndef POTENTIALMAXIMALCLIQUESENUMERATOR_H
#define POTENTIALMAXIMALCLIQUESENUMERATOR_H

#include "Graph.h"
#include "SubGraph.h"
#include "DataStructures.h"
#include <vector>

namespace tdenum {

/**
 * Constructs a list of potential maximal cliques, given a graph.
 *
 * Implements the algorithm described in the paper:
 * Listing all potential maximal cliques of a graph.
 * Vincent Bouchitte, Ioan Todinca, 2000
 */
class PotentialMaximalCliquesEnumerator {
    Graph graph;
    // Returns true <==> K is a potential maximal clique in G.
    // Assumes K is a subset of the vertices in G.
    bool IsPMC(NodeSet K, const Graph& G) const;
    // The iterative step of the algorithm.
    // Given graphs G1, G2, a vertex 'a' s.t. G2=G1\{a}, the minimal
    // seperators D1, D2 of G1 and G2 respectively and the potential
    // maximal cliques P2 of G2, calculates the set of potential maximal
    // cliques of G1 in polynomial time.
    NodeSetSet OneMoreVertex(const SubGraph& G1, const SubGraph& G2, Node a,
                  const NodeSetSet& D1, const NodeSetSet& D2,
                  const NodeSetSet& P2) const;
public:
    PotentialMaximalCliquesEnumerator(const Graph& g);
    // Return the set of PMCs
    NodeSetSet get() const;
};

} /* namespace tdenum */


#endif // POTENTIALMAXIMALCLIQUESENUMERATOR_H
