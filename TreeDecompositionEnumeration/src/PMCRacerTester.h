#ifndef PMCRACERTESTER_H_INCLUDED
#define PMCRACERTESTER_H_INCLUDED

#include "TestInterface.h"

namespace tdenum {

#define PMCRACERTESTER_TEST_TABLE \
    X(crosscheck_sanity) \
    X(crosscheck_insanity) \
    X(validate_accurate_times_basic) \
    X(validate_accurate_times_basic_twoalgs) \
    X(validate_accurate_times) \
    X(validate_accurate_times_batch_mode)

class PMCRacerTester : public TestInterface {
private:

    // Out of the total amount of existing tests, set those in the given
    // range [first,last)
    static const int total_tests_defined;
    PMCRacerTester& set_range(int first, int last);

    #define X(test) bool flag_##test;
    PMCRACERTESTER_TEST_TABLE
    #undef X
    PMCRacerTester& go();
public:
    PMCRacerTester();
    #define X(test) \
    PMCRacerTester& set_##test(); \
    PMCRacerTester& unset_##test(); \
    PMCRacerTester& set_only_##test(); \
    bool test() const;
    PMCRACERTESTER_TEST_TABLE
    #undef X
    PMCRacerTester& set_all();
    PMCRacerTester& clear_all();
};


}

#endif // PMCRACERTESTER_H_INCLUDED
