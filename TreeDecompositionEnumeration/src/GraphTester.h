#ifndef GRAPHTESTER_H_INCLUDED
#define GRAPHTESTER_H_INCLUDED

#include "Graph.h"
#include "TestUtils.h"

namespace tdenum {

#define GRAPH_TEST_TABLE \
    X(sort_nodes_by_deg) \
    X(map_back_to_original)

#define X(func) GRAPH_TEST_NAME__##func,
typedef enum {
    GRAPH_TEST_TABLE
    GRAPH_TEST_NAME__LAST
} GraphTesterFunctions;
#undef X

class GraphTester {
public:

    // Declare all functions and their on/off flags.
    #define X(_func) \
        bool _func() const; \
        bool flag_##_func;
    GRAPH_TEST_TABLE
    #undef X

    // Calls all test functions, unless start=false.
    GraphTester(bool start);

    // Calls all tests with flag_ values set to true.
    void go() const;

    // Sets / clears all flags
    void setAll();
    void clearAll();

};

}


#endif // GRAPHTESTER_H_INCLUDED
