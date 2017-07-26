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
#define PMC_ALG_TABLE \
    X(NORMAL) \
    X(REVERSE_MS_PRECALC) \
    X(ASCENDING_DEG_REVERSE_MS) \
    X(DESCENDING_DEG_REVERSE_MS)

#define ALG_IS_REVERSE_MS_STRAIN(_alg) \
    (_alg == ALG_REVERSE_MS_PRECALC || \
     _alg == ALG_ASCENDING_DEG_REVERSE_MS || \
     _alg == ALG_DESCENDING_DEG_REVERSE_MS)

class PMCEnumerator {
public:
    typedef enum {
    #define X(name) ALG_##name,
        PMC_ALG_TABLE
        ALG_LAST
    #undef X
    } Alg;
private:

    static const string alg_names[PMCEnumerator::ALG_LAST+1];

    // Test class
    friend class PMCEnumeratorTester;

    // The graph in question
    Graph graph;

    // The algorithm to be used (defaults to default_alg)
    Alg alg;
    static const Alg default_alg = ALG_REVERSE_MS_PRECALC;

    // The minimal separators of the graph.
    // They are required for the algorithm to run correctly; if they
    // aren't given as a parameter, they need to be calculated.
    NodeSetSet ms;
    bool has_ms;

    // The PMCs (after calculation)
    NodeSetSet pmcs;

    // If the calculation is complete, set this to true.
    bool done;

    // If time limit exceeded, we need to know.
    time_t limit;
    time_t start_time;
    bool out_of_time;

    // Call to reset members to default values.
    void set_default_member_vals();

    // Returns true <==> K is a potential maximal clique in G.
    // Assumes K is a subset of the vertices in G.
    bool is_pmc(NodeSet K, const SubGraph& G);

    // The iterative step of the algorithm.
    // Given graphs G1, G2, a vertex 'a' s.t. G2=G1\{a}, the minimal
    // seperators D1, D2 of G1 and G2 respectively and the potential
    // maximal cliques P2 of G2, calculates the set of potential maximal
    // cliques of G1 in polynomial time.
    NodeSetSet one_more_vertex(const SubGraph& G1, const SubGraph& G2, Node a,
                  const NodeSetSet& D1, const NodeSetSet& D2,
                  const NodeSetSet& P2);

public:

    // Construct the enumerator with the given graph.
    // Optionally, enforce a time limit.
    PMCEnumerator(const Graph& g, time_t time_limit = 0);

    // Resets the instance to use a new graph (allows re-use of variable name).
    void reset(const Graph& g, time_t time_limit = 0);

    // Sets / gets the algorithm to be used.
    void set_algorithm(Alg a);
    PMCEnumerator::Alg get_alg() const;
    static string get_alg_name(Alg a);

    // If the minimal separators for the original graph has already been
    // calculated, inform the enumerator.
    // This also saves some time.
    void set_minimal_separators(const NodeSetSet& ms);

    // Returns the set of PMCs (calculates if need be).
    // Uses the algorithm specified by the user.
    NodeSetSet get();

    // Return the set of minimal separators.
    NodeSetSet get_ms();

    // Returns the underlying graph
    Graph get_graph() const;

    // If true, the calculation couldn't be completed in the allowed time.
    bool is_out_of_time() const;
};

} /* namespace tdenum */


#endif // POTENTIALMAXIMALCLIQUESENUMERATOR_H
