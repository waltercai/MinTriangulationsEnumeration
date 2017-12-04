#ifndef POTENTIALMAXIMALCLIQUESENUMERATOR_H
#define POTENTIALMAXIMALCLIQUESENUMERATOR_H

#include "DataStructures.h"
#include "Graph.h"
#include "PMCAlg.h"
#include "StatisticRequest.h"
#include "SubGraph.h"
#include <omp.h>
#include <vector>

namespace tdenum {

// Different run modes for the enumerator.
// Some inner mechanisms are difficult to test via friend class
typedef enum PMCERunMode {
    PMCE_RUNMODE_FAST,
    PMCE_RUNMODE_VERIFY_SORT
} PMCERunMode;
extern int PMCE_RUNMODE;

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

    // Count the number of minimal separators in each subgraph.
    // This is calculated anyway, however this may be useful information
    // as different orderings of subgraphs may yield different amounts
    // of separators/
    vector<NodeSetSet> ms_subgraphs;
    vector<long> ms_subgraph_count;

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

    // Reads the totals, given an updated ms_subgraphs field
    void update_ms_subgraph_count();

    // Doesn't read from ms_subgraphs
    void read_ms_subgraph_count_from_vector(const vector<NodeSetSet>&);

public:

    // Construct the enumerator with the given graph.
    // Optionally, enforce a time limit.
    PMCEnumerator(const Graph& g, time_t time_limit = 0);
    ~PMCEnumerator();

    // Resets the instance to use a new graph (allows re-use of variable name).
    void reset(const Graph& g, time_t time_limit = 0);

    // Setters / getters
    void set_algorithm(PMCAlg a);
    PMCAlg get_alg() const;
    void set_time_limit(time_t);
    void unset_time_limit();
    void enable_parallel();
    void suppress_parallel();

    // If the minimal separators for the original graph has already been
    // calculated, inform the enumerator.
    // This also saves some time.
    void set_minimal_separators(const NodeSetSet& ms);

    // Returns the set of PMCs (calculates if need be).
    // Uses the algorithm specified by the user.
    NodeSetSet get(const StatisticRequest&);

    // Return the set of minimal separators.
    NodeSetSet get_ms();
    vector<NodeSetSet> get_ms_subgraphs();
    vector<long> get_ms_count_subgraphs();

    // Returns the underlying graph
    Graph get_graph() const;

    // If true, the calculation couldn't be completed in the allowed time.
    bool is_out_of_time() const;
};

} /* namespace tdenum */


#endif // POTENTIALMAXIMALCLIQUESENUMERATOR_H
