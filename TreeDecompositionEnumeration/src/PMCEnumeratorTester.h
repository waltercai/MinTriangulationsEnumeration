#ifndef POTENTIALMAXIMALCLIQUESENUMERATORTESTER_H_INCLUDED
#define POTENTIALMAXIMALCLIQUESENUMERATORTESTER_H_INCLUDED

#include "TestUtils.h"
#include "Graph.h"
#include "PMCEnumerator.h"

// Graphs with more nodes may cause slow tests
#define FAST_GRAPH_SIZE 10

#define TEST_TABLE \
    /* Make sure we don't have runtime errors... */ \
    X(sanity) \
    /* Graphs with no more than one vertex */ \
    X(trivialgraphs) \
    /* With n<=FAST_GRAPH_SIZE, output some random graphs . \
     * This just checks for basic runtime errors.. */ \
    X(randomgraphs) \
    /* Graphs of size 2 and 3, where the answer is easily checked */ \
    X(smallknowngraphs) \


namespace tdenum {

class PMCEnumeratorTester {
private:

    #define X(_func) bool _func();
    TEST_TABLE
    #undef X

public:

    // Calls all test functions
    PMCEnumeratorTester();

};

}
#endif // POTENTIALMAXIMALCLIQUESENUMERATORTESTER_H_INCLUDED
