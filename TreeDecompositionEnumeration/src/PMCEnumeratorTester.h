#ifndef POTENTIALMAXIMALCLIQUESENUMERATORTESTER_H_INCLUDED
#define POTENTIALMAXIMALCLIQUESENUMERATORTESTER_H_INCLUDED

#include "TestUtils.h"
#include "Graph.h"
#include "PMCEnumerator.h"
#include "DirectoryIterator.h"

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
    /* Graphs of size 4 (there are 11 such graphs up to isomorphism) */ \
    X(fourgraphs) \
    /* Uses existing datasets and Nofar's code to cross-check the PMC \
       algorithm with Nofar's version */ \
    X(crosscheck)

#define X(func) PMCENUM_TEST_NAME__##func,
typedef enum {
    TEST_TABLE
    PMCENUM_TEST_NAME__LAST
} PMCEnumeratorTesterFunctions;
#undef X

namespace tdenum {

class PMCEnumeratorTester {
public:

    // Define all functions and on/off flags.
    // On/off flags are to be set directly, e.g:
    //     PMCEnumeratorTester p(false);
    //
    #define X(_func) \
        bool _func() const; \
        bool flag_##_func;
    TEST_TABLE
    #undef X

    // Calls all test functions, unless start=false.
    PMCEnumeratorTester(bool start);

    // Calls all tests with flag_ values set to true.
    void go() const;

    // Sets / clears all flags
    void setAll();
    void clearAll();

};

}
#endif // POTENTIALMAXIMALCLIQUESENUMERATORTESTER_H_INCLUDED
