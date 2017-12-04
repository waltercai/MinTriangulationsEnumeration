#include "PMCEnumerator.h"
#include "Tester.h"
#include <iostream>
using std::cout;
using std::endl;

namespace tdenum {

Tester::Tester() :
    TestInterface("MAIN TESTER")
    #define X(test) , flag_##test(true)
    TESTER_TEST_TABLE
    #undef X
    {}

// Set / unset
#define X(test) \
    Tester& Tester::set_##test() { flag_##test = true; return *this; } \
    Tester& Tester::unset_##test() { flag_##test = false; return *this; }
TESTER_TEST_TABLE
#undef X

// Set / unset all
Tester& Tester::set_all() {
    #define X(test) set_##test();
    TESTER_TEST_TABLE
    #undef X
    return *this;
}
Tester& Tester::clear_all() {
    #define X(test) unset_##test();
    TESTER_TEST_TABLE
    #undef X
    return *this;
}

Tester& Tester::go() {
    // The 'tests' are now class names of other testers.
    // They also inherit from TestInterface, so use the
    // start() and test_passed() methods for this purpose.
    #define X(test) \
        if (flag_##test) {DO_TEST(test);}
    TESTER_TEST_TABLE
    #undef X
    return *this;
}

}

/**
 * Main program.
 *
 * Pick and choose tests to run
 */
using namespace tdenum;


int main(int argc, char** argv) {
    // Make sure the PMC mode verifies sorting!
    PMCE_RUNMODE = PMCE_RUNMODE_VERIFY_SORT;
    Tester().clear_all()
//            .set_UtilsTester()
//            .set_GraphTester()
//            .set_GraphStatsTester()
//            .set_GraphProducerTester()
//            .set_PMCEnumeratorTester()
//            .set_StatisticRequestTester()
//            .set_PMCRacerTester()
            .set_DatasetTester()
            .start();
    return 0;
}

