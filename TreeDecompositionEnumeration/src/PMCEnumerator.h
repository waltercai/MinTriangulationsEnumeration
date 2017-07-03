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
class PMCEnumerator {
    // Test class
    friend class PMCEnumeratorTester;

    // The graph in question
    Graph graph;

    // The PMCs (after calculation)
    NodeSetSet pmcs;

    // We get the minimal separators as a by-product of the algorithm,
    // so store it here for later use.
    // In the future, maybe optionally allow the user to pass the set
    // of minimal separators as an argument to the constructor, and
    // calculate the minimal separators of subgraphs given the MSs of
    // the original graph. This may save a lot of time.
    NodeSetSet MS;

    // If the calculation is complete, set this to true.
    bool done;

    // If time limit exceeded, we need to know
    time_t limit;
    time_t start_time;
    bool out_of_time;

    // Returns true <==> K is a potential maximal clique in G.
    // Assumes K is a subset of the vertices in G.
    bool IsPMC(NodeSet K, const SubGraph& G);

    // The iterative step of the algorithm.
    // Given graphs G1, G2, a vertex 'a' s.t. G2=G1\{a}, the minimal
    // seperators D1, D2 of G1 and G2 respectively and the potential
    // maximal cliques P2 of G2, calculates the set of potential maximal
    // cliques of G1 in polynomial time.
    NodeSetSet OneMoreVertex(const SubGraph& G1, const SubGraph& G2, Node a,
                  const NodeSetSet& D1, const NodeSetSet& D2,
                  const NodeSetSet& P2);

    // get() iterates over all connected components of G and calls this
    // auxiliary function which expects a connected graph.
    NodeSetSet getConnected(const SubGraph&);

public:
    PMCEnumerator(const Graph& g, time_t time_limit = 0);

    // Resets the instance to use a new graph (allows re-use of variable name).
    void reset(const Graph& g, time_t time_limit = 0);

    // Returns the set of PMCs (calculates if need be)
    NodeSetSet get();

    // Return the set of minimal separators.
    NodeSetSet get_ms();

    // If true, the calculation couldn't be completed in the allowed time.
    bool is_out_of_time() const;
};

} /* namespace tdenum */


#endif // POTENTIALMAXIMALCLIQUESENUMERATOR_H
