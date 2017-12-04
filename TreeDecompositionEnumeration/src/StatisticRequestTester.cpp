#include "StatisticRequest.h"
#include "StatisticRequestTester.h"

namespace tdenum {

StatisticRequestTester::StatisticRequestTester() :
    TestInterface("StatisticRequest Tester")
    #define X(test) , flag_##test(true)
    STATISTICREQUESTTESTER_TEST_TABLE
    #undef X
    {}

#define X(test) \
StatisticRequestTester& StatisticRequestTester::set_##test() { flag_##test = true; return *this; } \
StatisticRequestTester& StatisticRequestTester::unset_##test() { flag_##test = false; return *this; } \
StatisticRequestTester& StatisticRequestTester::set_only_##test() { clear_all(); return set_##test(); }
STATISTICREQUESTTESTER_TEST_TABLE
#undef X

StatisticRequestTester& StatisticRequestTester::set_all() {
    #define X(test) set_##test();
    STATISTICREQUESTTESTER_TEST_TABLE
    #undef X
    return *this;
}
StatisticRequestTester& StatisticRequestTester::clear_all() {
    #define X(test) unset_##test();
    STATISTICREQUESTTESTER_TEST_TABLE
    #undef X
    return *this;
}
StatisticRequestTester& StatisticRequestTester::go() {
    #define X(test) if (flag_##test) {DO_TEST(test);}
    STATISTICREQUESTTESTER_TEST_TABLE
    #undef X
    return *this;
}



bool StatisticRequestTester::default_flags_off() const {

    StatisticRequest sr;
    ASSERT(!sr.test_count_ms());
    ASSERT(!sr.test_count_pmc());
    ASSERT(!sr.test_count_trng());
    ASSERT(!sr.test_ms());
    ASSERT(!sr.test_pmc());
    ASSERT(!sr.test_trng());
    ASSERT(!sr.test_ms_subgraphs());
    ASSERT(!sr.test_ms_subgraph_count());
    ASSERT(!sr.test_has_ms_calculation());
    ASSERT(!sr.test_has_pmc_calculation());
    ASSERT(!sr.test_has_trng_calculation());

    return true;
}
bool StatisticRequestTester::equality() const {

    GENERATE_STATREQS();

    // Equality
    ASSERT_EQ(sr_all_ms, sr_all_ms);
    ASSERT_EQ(sr_all_pmc, sr_all_pmc);
    ASSERT_EQ(sr_all_trng, sr_all_trng);
    ASSERT_EQ(sr_ms, sr_ms);
    ASSERT_EQ(sr_ms_subgraphs, sr_ms_subgraphs);
    ASSERT_EQ(sr_ms_subgraph_count, sr_ms_subgraph_count);
    ASSERT_EQ(sr_count_ms, sr_count_ms);
    ASSERT_EQ(sr_count_pmc, sr_count_pmc);
    ASSERT_EQ(sr_count_trng, sr_count_trng);
    ASSERT_EQ(sr_pmc, sr_pmc);
    ASSERT_EQ(sr_trng, sr_trng);

    // Inequality... test a few
    ASSERT_NEQ(sr_all_ms, sr_ms);
    ASSERT_NEQ(sr_all_ms, sr_count_ms);
    ASSERT_NEQ(sr_count_ms, sr_ms);

    ASSERT_EQ(sr_all_pmc, StatisticRequest().set_single_pmc_alg().set_pmc().set_count_pmc().set_ms_subgraphs().set_ms_subgraph_count());
    ASSERT_NEQ(sr_all_pmc, StatisticRequest().set_single_pmc_alg().set_count_pmc().set_ms_subgraphs().set_ms_subgraph_count());
    ASSERT_NEQ(sr_all_pmc, StatisticRequest().set_single_pmc_alg().set_pmc().set_ms_subgraphs().set_ms_subgraph_count());
    ASSERT_NEQ(sr_all_pmc, StatisticRequest().set_single_pmc_alg().set_pmc().set_count_pmc().set_ms_subgraph_count());
    ASSERT_NEQ(sr_all_pmc, StatisticRequest().set_single_pmc_alg().set_pmc().set_count_pmc().set_ms_subgraphs());

    return true;
}
bool StatisticRequestTester::testers() const {

    GENERATE_STATREQS();

    // Basics
    ASSERT(!sr_count_ms.test_count_limit_ms());
    ASSERT(!sr_ms.test_count_limit_ms());
    ASSERT(!sr_all_ms.test_count_limit_ms());
    ASSERT(!sr_count_pmc.test_count_limit_ms());
    ASSERT(!sr_pmc.test_count_limit_ms());
    ASSERT(!sr_ms_subgraphs.test_count_limit_ms());
    ASSERT(!sr_ms_subgraph_count.test_count_limit_ms());
    ASSERT(!sr_all_pmc.test_count_limit_ms());
    ASSERT(!sr_count_trng.test_count_limit_ms());
    ASSERT(!sr_trng.test_count_limit_ms());
    ASSERT(!sr_all_trng.test_count_limit_ms());

    ASSERT(!sr_count_ms.test_time_limit_ms());
    ASSERT(!sr_ms.test_time_limit_ms());
    ASSERT(!sr_all_ms.test_time_limit_ms());
    ASSERT(!sr_count_pmc.test_time_limit_ms());
    ASSERT(!sr_pmc.test_time_limit_ms());
    ASSERT(!sr_ms_subgraphs.test_time_limit_ms());
    ASSERT(!sr_ms_subgraph_count.test_time_limit_ms());
    ASSERT(!sr_all_pmc.test_time_limit_ms());
    ASSERT(!sr_count_trng.test_time_limit_ms());
    ASSERT(!sr_trng.test_time_limit_ms());
    ASSERT(!sr_all_trng.test_time_limit_ms());

    ASSERT(!sr_count_ms.test_count_limit_trng());
    ASSERT(!sr_ms.test_count_limit_trng());
    ASSERT(!sr_all_ms.test_count_limit_trng());
    ASSERT(!sr_count_pmc.test_count_limit_trng());
    ASSERT(!sr_pmc.test_count_limit_trng());
    ASSERT(!sr_ms_subgraphs.test_count_limit_trng());
    ASSERT(!sr_ms_subgraph_count.test_count_limit_trng());
    ASSERT(!sr_all_pmc.test_count_limit_trng());
    ASSERT(!sr_count_trng.test_count_limit_trng());
    ASSERT(!sr_trng.test_count_limit_trng());
    ASSERT(!sr_all_trng.test_count_limit_trng());

    ASSERT(!sr_count_ms.test_time_limit_trng());
    ASSERT(!sr_ms.test_time_limit_trng());
    ASSERT(!sr_all_ms.test_time_limit_trng());
    ASSERT(!sr_count_pmc.test_time_limit_trng());
    ASSERT(!sr_pmc.test_time_limit_trng());
    ASSERT(!sr_ms_subgraphs.test_time_limit_trng());
    ASSERT(!sr_ms_subgraph_count.test_time_limit_trng());
    ASSERT(!sr_all_pmc.test_time_limit_trng());
    ASSERT(!sr_count_trng.test_time_limit_trng());
    ASSERT(!sr_trng.test_time_limit_trng());
    ASSERT(!sr_all_trng.test_time_limit_trng());

    ASSERT(!sr_count_ms.test_time_limit_pmc());
    ASSERT(!sr_ms.test_time_limit_pmc());
    ASSERT(!sr_all_ms.test_time_limit_pmc());
    ASSERT(!sr_count_pmc.test_time_limit_pmc());
    ASSERT(!sr_pmc.test_time_limit_pmc());
    ASSERT(!sr_ms_subgraphs.test_time_limit_pmc());
    ASSERT(!sr_ms_subgraph_count.test_time_limit_pmc());
    ASSERT(!sr_all_pmc.test_time_limit_pmc());
    ASSERT(!sr_count_trng.test_time_limit_pmc());
    ASSERT(!sr_trng.test_time_limit_pmc());
    ASSERT(!sr_all_trng.test_time_limit_pmc());

    // Specifics
    ASSERT(sr_count_ms.test_count_ms());
    ASSERT(!sr_count_ms.test_count_pmc());
    ASSERT(!sr_count_ms.test_count_trng());
    ASSERT(!sr_count_ms.test_ms());
    ASSERT(!sr_count_ms.test_pmc());
    ASSERT(!sr_count_ms.test_trng());
    ASSERT(!sr_count_ms.test_ms_subgraphs());
    ASSERT(!sr_count_ms.test_ms_subgraph_count());
    ASSERT(sr_count_ms.test_has_ms_calculation());
    ASSERT(!sr_count_ms.test_has_pmc_calculation());
    ASSERT(!sr_count_ms.test_has_trng_calculation());

    ASSERT(!sr_ms.test_count_ms());  // Should NOT be set: if the user sets and unsets ms(), the count_ms() shouldn't become set!
    ASSERT(!sr_ms.test_count_pmc());
    ASSERT(!sr_ms.test_count_trng());
    ASSERT(sr_ms.test_ms());
    ASSERT(!sr_ms.test_pmc());
    ASSERT(!sr_ms.test_trng());
    ASSERT(!sr_ms.test_ms_subgraphs());
    ASSERT(!sr_ms.test_ms_subgraph_count());
    ASSERT(sr_ms.test_has_ms_calculation());
    ASSERT(!sr_ms.test_has_pmc_calculation());
    ASSERT(!sr_ms.test_has_trng_calculation());

    ASSERT(sr_all_ms.test_count_ms());
    ASSERT(!sr_all_ms.test_count_pmc());
    ASSERT(!sr_all_ms.test_count_trng());
    ASSERT(sr_all_ms.test_ms());
    ASSERT(!sr_all_ms.test_pmc());
    ASSERT(!sr_all_ms.test_trng());
    ASSERT(!sr_all_ms.test_ms_subgraphs());
    ASSERT(!sr_all_ms.test_ms_subgraph_count());
    ASSERT(sr_all_ms.test_has_ms_calculation());
    ASSERT(!sr_all_ms.test_has_pmc_calculation());
    ASSERT(!sr_all_ms.test_has_trng_calculation());

    ASSERT(!sr_count_pmc.test_count_ms());
    ASSERT(sr_count_pmc.test_count_pmc());
    ASSERT(!sr_count_pmc.test_count_trng());
    ASSERT(!sr_count_pmc.test_ms());
    ASSERT(!sr_count_pmc.test_pmc());
    ASSERT(!sr_count_pmc.test_trng());
    ASSERT(!sr_count_pmc.test_ms_subgraphs());
    ASSERT(!sr_count_pmc.test_ms_subgraph_count());
    ASSERT(!sr_count_pmc.test_has_ms_calculation());
    ASSERT(sr_count_pmc.test_has_pmc_calculation());
    ASSERT(!sr_count_pmc.test_has_trng_calculation());

    ASSERT(!sr_pmc.test_count_ms());
    ASSERT(!sr_pmc.test_count_pmc());
    ASSERT(!sr_pmc.test_count_trng());
    ASSERT(!sr_pmc.test_ms());
    ASSERT(sr_pmc.test_pmc());
    ASSERT(!sr_pmc.test_trng());
    ASSERT(!sr_pmc.test_ms_subgraphs());
    ASSERT(!sr_pmc.test_ms_subgraph_count());
    ASSERT(!sr_pmc.test_has_ms_calculation());
    ASSERT(sr_pmc.test_has_pmc_calculation());
    ASSERT(!sr_pmc.test_has_trng_calculation());

    ASSERT(!sr_ms_subgraph_count.test_count_ms());
    ASSERT(!sr_ms_subgraph_count.test_count_pmc());
    ASSERT(!sr_ms_subgraph_count.test_count_trng());
    ASSERT(!sr_ms_subgraph_count.test_ms());
    ASSERT(!sr_ms_subgraph_count.test_pmc());
    ASSERT(!sr_ms_subgraph_count.test_trng());
    ASSERT(!sr_ms_subgraph_count.test_ms_subgraphs());
    ASSERT(sr_ms_subgraph_count.test_ms_subgraph_count());
    ASSERT(!sr_ms_subgraph_count.test_has_ms_calculation());
    ASSERT(sr_ms_subgraph_count.test_has_pmc_calculation());
    ASSERT(!sr_ms_subgraph_count.test_has_trng_calculation());

    ASSERT(!sr_ms_subgraphs.test_count_ms());
    ASSERT(!sr_ms_subgraphs.test_count_pmc());
    ASSERT(!sr_ms_subgraphs.test_count_trng());
    ASSERT(!sr_ms_subgraphs.test_ms());
    ASSERT(!sr_ms_subgraphs.test_pmc());
    ASSERT(!sr_ms_subgraphs.test_trng());
    ASSERT(sr_ms_subgraphs.test_ms_subgraphs());
    ASSERT(!sr_ms_subgraphs.test_ms_subgraph_count());
    ASSERT(!sr_ms_subgraphs.test_has_ms_calculation());
    ASSERT(sr_ms_subgraphs.test_has_pmc_calculation());
    ASSERT(!sr_ms_subgraphs.test_has_trng_calculation());

    ASSERT(!sr_all_pmc.test_count_ms());
    ASSERT(sr_all_pmc.test_count_pmc());
    ASSERT(!sr_all_pmc.test_count_trng());
    ASSERT(!sr_all_pmc.test_ms());
    ASSERT(sr_all_pmc.test_pmc());
    ASSERT(!sr_all_pmc.test_trng());
    ASSERT(sr_all_pmc.test_ms_subgraphs());
    ASSERT(sr_all_pmc.test_ms_subgraph_count());
    ASSERT(!sr_all_pmc.test_has_ms_calculation());
    ASSERT(sr_all_pmc.test_has_pmc_calculation());
    ASSERT(!sr_all_pmc.test_has_trng_calculation());

    ASSERT(!sr_count_trng.test_count_ms());
    ASSERT(!sr_count_trng.test_count_pmc());
    ASSERT(sr_count_trng.test_count_trng());
    ASSERT(!sr_count_trng.test_ms());
    ASSERT(!sr_count_trng.test_pmc());
    ASSERT(!sr_count_trng.test_trng());
    ASSERT(!sr_count_trng.test_ms_subgraphs());
    ASSERT(!sr_count_trng.test_ms_subgraph_count());
    ASSERT(!sr_count_trng.test_has_ms_calculation());
    ASSERT(!sr_count_trng.test_has_pmc_calculation());
    ASSERT(sr_count_trng.test_has_trng_calculation());

    ASSERT(!sr_trng.test_count_ms());
    ASSERT(!sr_trng.test_count_pmc());
    ASSERT(!sr_trng.test_count_trng());
    ASSERT(!sr_trng.test_ms());
    ASSERT(!sr_trng.test_pmc());
    ASSERT(sr_trng.test_trng());
    ASSERT(!sr_trng.test_ms_subgraphs());
    ASSERT(!sr_trng.test_ms_subgraph_count());
    ASSERT(!sr_trng.test_has_ms_calculation());
    ASSERT(!sr_trng.test_has_pmc_calculation());
    ASSERT(sr_trng.test_has_trng_calculation());

    ASSERT(!sr_all_trng.test_count_ms());
    ASSERT(!sr_all_trng.test_count_pmc());
    ASSERT(sr_all_trng.test_count_trng());
    ASSERT(!sr_all_trng.test_ms());
    ASSERT(!sr_all_trng.test_pmc());
    ASSERT(sr_all_trng.test_trng());
    ASSERT(!sr_all_trng.test_ms_subgraphs());
    ASSERT(!sr_all_trng.test_ms_subgraph_count());
    ASSERT(!sr_all_trng.test_has_ms_calculation());
    ASSERT(!sr_all_trng.test_has_pmc_calculation());
    ASSERT(sr_all_trng.test_has_trng_calculation());

    return true;
}


}
