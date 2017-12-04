#include "GraphTester.h"
#include "Graph.h"

namespace tdenum {

#define SETUP(n) \
    Graph g(n)
#define RESET(n) \
    g.reset(n)

bool GraphTester::sort_nodes_by_deg() const {
    int n=6;
    SETUP(n);
    for (int ascending = 0; ascending < 2; ++ascending) {
        for (double p=0.1; p<0.95; p+=0.1) {
            g.randomize(p);
            g.sortNodesByDegree(ascending);
            for (int j=0; j+1<n; ++j) {
                ASSERT(ascending? (g.d(j) <= g.d(j+1)) : (g.d(j) >= g.d(j+1)));
            }
        }
    }
    return true;
}

bool GraphTester::map_back_to_original() const {
    /**
     * Construct specific graphs to test these, including composition
     * of mappings.
     *
     * Given the following graph:
     *
     *   0   4
     *  / \
     * 1---2--3
     *
     * The node set is {0,1,2,3,4}.
     * The ascending set is {4,3,0,1,2} or {4,3,1,0,2}.
     * The descending set is either of the reversed ascending sets.
     *
     * The node set returned by getNodesVector() is always {0,1,2,3,4},
     * so we want to make sure structural integrity remains.
     */
    TRACE(TRACE_LVL__TEST, "In");
    SETUP(5);
    g.addClique(NodeSet({0,1,2}));
    g.addEdge(2,3);
    ASSERT_EQ(g.getNodesVector(), NodeSet({0,1,2,3,4}));

    // Ascending sort
    TRACE(TRACE_LVL__TEST, "Ascending...");
    g.sortNodesByDegree(true);
    // The original nodes {0,1,2} are now named {2,3,4}
    ASSERT_EQ(NodeSet({0,1,2}), g.getOriginalNames({2,3,4}));
    ASSERT(g.areNeighbors(2,3));
    ASSERT(g.areNeighbors(2,4));
    ASSERT(g.areNeighbors(3,4));
    // Nodes 2 and 3 are now 1 and 4
    ASSERT_EQ(g.getOriginalName(4), 2);
    ASSERT_EQ(g.getOriginalName(1), 3);
    ASSERT(g.areNeighbors(1,4));
    // Node 4 is now first (0)
    ASSERT_EQ(g.getOriginalName(0),4);

    TRACE(TRACE_LVL__TEST, "Descending...");
    // Descending order (sequential mapping - also tests composition of mappings)
    g.sortNodesByDegree(false);
    // The original nodes {0,1,2} are still named {0,1,2}
    ASSERT_EQ(NodeSet({0,1,2}), g.getOriginalNames({0,1,2}));
    ASSERT(g.areNeighbors(0,1));
    ASSERT(g.areNeighbors(0,2));
    ASSERT(g.areNeighbors(1,2));
    // Nodes 2 and 3 are now 0 and 3
    ASSERT_EQ(g.getOriginalName(0), 2);
    ASSERT_EQ(g.getOriginalName(3), 3);
    ASSERT(g.areNeighbors(0,3));
    // Node 4 is now last (still 4)
    ASSERT_EQ(g.getOriginalName(4),4);

    return true;
}

bool GraphTester::creating_random() const {
    SETUP(5);
    ASSERT(!g.isRandom());
    g.randomize(0.5);
    ASSERT(g.isRandom());
    ASSERT_EQ(g.getInstance(), 1);
    ASSERT_EQ_FLOAT(g.getP(), 0.5);

    RESET(5);
    ASSERT(!g.isRandom());
    g.declareRandom(0.3,4);
    ASSERT(g.isRandom());
    ASSERT_EQ(g.getInstance(), 4);
    ASSERT_EQ_FLOAT(g.getP(), 0.3);

    return true;
}


GraphTester::GraphTester() :
    TestInterface("Graph Tester")
    #define X(_func) , flag_##_func(true)
    GRAPH_TEST_TABLE
    #undef X
    {}

GraphTester& GraphTester::go() {
    // Hacky, but it works
    #define X(_func) if (flag_##_func) DO_TEST(_func);
    GRAPH_TEST_TABLE
    #undef X
    return *this;
}
GraphTester& GraphTester::set_all() {
    // Note to self: no good code comes after 1:00am
    #define X(_func) flag_##_func = true;
    GRAPH_TEST_TABLE
    #undef X
    return *this;
}
GraphTester& GraphTester::clear_all() {
    #define X(_func) flag_##_func = false;
    GRAPH_TEST_TABLE
    #undef X
    return *this;
}

}
