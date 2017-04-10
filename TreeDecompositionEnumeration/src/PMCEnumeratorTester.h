#ifndef POTENTIALMAXIMALCLIQUESENUMERATORTESTER_H_INCLUDED
#define POTENTIALMAXIMALCLIQUESENUMERATORTESTER_H_INCLUDED

#include "TestUtils.h"
#include "Graph.h"
#include "PMCEnumerator.h"

#define TEST_TABLE \
    /* Make sure we don't have runtime errors... */ \
    X(sanity) \
    /* Graphs with no more than one vertex */ \
    X(trivialgraphs)


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
