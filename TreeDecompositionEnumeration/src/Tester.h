#ifndef TESTER_H_INCLUDED
#define TESTER_H_INCLUDED

#include "DatasetTester.h"
#include "GraphProducerTester.h"
#include "GraphStatsTester.h"
#include "GraphTester.h"
#include "PMCEnumeratorTester.h"
#include "PMCRacerTester.h"
#include "StatisticRequestTester.h"
#include "TestInterface.h"
#include "UtilsTester.h"

namespace tdenum {

/**
 * This is the class used for all tests.
 *
 * To test something new, inherit from the TestInterface class and
 * add the new class name to the X macro bellow.
 *
 * The order of tests in the table defines the order they're run!
 */
#define TESTER_TEST_TABLE \
    X(UtilsTester) \
    X(GraphTester) \
    X(GraphStatsTester) \
    X(GraphProducerTester) \
    X(PMCEnumeratorTester) \
    X(StatisticRequestTester) \
    X(PMCRacerTester) \
    X(DatasetTester)

// Static (compile-time) sanity check
#define X(test) const TestInterface& DUMMY_##test = test();
TESTER_TEST_TABLE
#undef X

class Tester : public TestInterface {
private:
    // Flags to turn specific tests on / off
    #define X(test) \
    bool flag_##test;
    TESTER_TEST_TABLE
    #undef X

    // Start
    Tester& go();

public:

    Tester();

    // Set / unset tests
    #define X(test) \
        Tester& set_##test(); \
        Tester& unset_##test();
    TESTER_TEST_TABLE
    #undef X

    // Set / unset all
    Tester& set_all();
    Tester& clear_all();

};

}

#endif // TESTER_H_INCLUDED
