#ifndef TESTER_H_INCLUDED
#define TESTER_H_INCLUDED

#include "DatasetTester.h"
#include "GraphProducerTester.h"
#include "GraphStatsTester.h"
#include "GraphTester.h"
#include "PMCEnumeratorTester.h"
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
 * The order define in the table defines the order they're run!
 */
#define TESTER_TEST_TABLE \
    X(UtilsTester) \
    X(GraphTester) \
    X(GraphStatsTester) \
    X(GraphProducerTester) \
    X(PMCEnumeratorTester) \
    X(StatisticRequestTester) \
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

    // Overload this one, for printing purposes
//    const Tester& start_tester() const;

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

/*#define STAM_TABLE \
    X(1) \
    X(2) \
    X(3) \
    X(4) \
    X(5) \
    X(6) \
    X(7) \
    X(8) \
    X(9) \
    X(10) \
    X(11) \
    X(12) \
    X(13) \
    X(14) \
    X(15) \
    X(16) \
    X(17) \
    X(18) \
    X(19) \
    X(20) \
    X(21) \
    X(22) \
    X(23) \
    X(24) \
    X(25) \
    X(26)

class Foo {
private:
    #define X(_) bool func_##_() const;
    STAM_TABLE
    #undef X
public:
    Foo();
};*/

}

#endif // TESTER_H_INCLUDED
