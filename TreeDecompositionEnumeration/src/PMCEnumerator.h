#ifndef POTENTIALMAXIMALCLIQUESENUMERATOR_H
#define POTENTIALMAXIMALCLIQUESENUMERATOR_H

#include "Graph.h"
#include "SubGraph.h"
#include "DataStructures.h"
#include "PMCAlg.h"
#include <omp.h>
#include <vector>

namespace tdenum {

// Different run modes for the enumerator.
// Some inner mechanisms are difficult to test via friend class
typedef enum PMCERunMode {
    PMCE_RUNMODE_FAST,
    PMCE_RUNMODE_VERIFY_SORT
} PMCERunMode;
#define PMCE_RUNMODE PMCE_RUNMODE_VERIFY_SORT

/**
 * Constructs a list of potential maximal cliques, given a graph.
 *
 * Implements the algorithm described in the paper:
 * Listing all potential maximal cliques of a graph.
 * Vincent Bouchitte, Ioan Todinca, 2000.
 */

class PMCEnumerator {
private:

    // Test class
    friend class PMCEnumeratorTester;

    // The graph in question, and a copy of the original for output.
    Graph graph;

    // The algorithm to be used (defaults to default_alg)
    PMCAlg alg;
    static const PMCAlg default_alg;

    // The minimal separators of the graph.
    // They are required for the algorithm to run correctly; if they
    // are not given as a parameter, they need to be calculated.
    NodeSetSet ms;
    bool has_ms;

    // The PMCs (after calculation)
    NodeSetSet pmcs;

    // If this is set to true, OMP #pragmas will be activated.
    bool allow_parallel;
    omp_lock_t lock;

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
    NodeSetSet one_more_vertex(const SubGraph& G1,
                               const SubGraph& G2,
                               Node a,
                               const NodeSetSet& D1,
                               const NodeSetSet& D2,
                               const NodeSetSet& P2);

public:

    // Construct the enumerator with the given graph.
    // Optionally, enforce a time limit.
    PMCEnumerator(const Graph& g, time_t time_limit = 0);
    ~PMCEnumerator();

    // Resets the instance to use a new graph (allows re-use of variable name).
    void reset(const Graph& g, time_t time_limit = 0);

    // Sets / gets the algorithm to be used.
    void set_algorithm(PMCAlg a);
    PMCAlg get_alg() const;

    // Toggle parallelization
    void enable_parallel();
    void suppress_parallel();

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
