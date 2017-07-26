#include "GraphTester.h"
#include "Graph.h"
#include "TestUtils.h"

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
            TRACE(TRACE_LVL__TEST, "Graph before sort:" << endl << g);
            g.sortNodesByDegree(ascending);
            TRACE(TRACE_LVL__TEST, "Graph after " << (ascending? "a" : "de") << "scending sort:" << endl << g);
            for (int j=0; j+1<n; ++j) {
                ASSERT(ascending? (g.d(j) <= g.d(j+1)) : (g.d(j) >= g.d(j+1)));
            }
        }
    }
    return true;
}


GraphTester::GraphTester(bool start = true) {
    #define X(_func) flag_##_func = true;
    GRAPH_TEST_TABLE
    #undef X
    if (start) {
        go();
    }
}
void GraphTester::go() const {
    // Hacky, but it works
    START_TESTS();
    #define X(_func) if (flag_##_func) DO_TEST(_func);
    GRAPH_TEST_TABLE
    #undef X
    END_TESTS();
}
void GraphTester::setAll() {
    // Note to self: no good code comes after 1:00am
    #define X(_func) flag_##_func = true;
    GRAPH_TEST_TABLE
    #undef X
}
void GraphTester::clearAll() {
    #define X(_func) flag_##_func = false;
    GRAPH_TEST_TABLE
    #undef X
}

}
