#ifndef STATISTICREQUESTTESTER_H_INCLUDED
#define STATISTICREQUESTTESTER_H_INCLUDED

#include "TestInterface.h"

namespace tdenum {


#define STATISTICREQUESTTESTER_TEST_TABLE \
    X(default_flags_off) \
    X(equality) \
    X(testers)

#define GENERATE_STATREQS() \
    StatisticRequest sr_count_ms = StatisticRequest().set_count_ms(); \
    StatisticRequest sr_ms = StatisticRequest().set_ms(); \
    StatisticRequest sr_all_ms = StatisticRequest().set_count_ms().set_ms(); \
    StatisticRequest sr_count_pmc = StatisticRequest().set_count_pmc().set_single_pmc_alg(); \
    StatisticRequest sr_pmc = StatisticRequest().set_pmc().set_single_pmc_alg(); \
    StatisticRequest sr_ms_subgraph_count = StatisticRequest().set_ms_subgraph_count().set_single_pmc_alg(); \
    StatisticRequest sr_ms_subgraphs = StatisticRequest().set_ms_subgraphs().set_single_pmc_alg(); \
    StatisticRequest sr_all_pmc = StatisticRequest() \
            .set_count_pmc() \
            .set_pmc() \
            .set_ms_subgraph_count() \
            .set_ms_subgraphs() \
            .set_single_pmc_alg(); \
    StatisticRequest sr_count_trng = StatisticRequest().set_count_trng(); \
    StatisticRequest sr_trng = StatisticRequest().set_trng(); \
    StatisticRequest sr_all_trng = StatisticRequest().set_count_trng().set_trng()

class StatisticRequestTester : public TestInterface {

    // Flags, setter / getters and actual tests.
    #define X(test) \
    private: \
        bool flag_##test; \
        bool test() const; \
    public: \
        StatisticRequestTester& set_##test(); \
        StatisticRequestTester& unset_##test(); \
        StatisticRequestTester& set_only_##test();
    STATISTICREQUESTTESTER_TEST_TABLE
    #undef X

private:
    StatisticRequestTester& go();

public:

    StatisticRequestTester();

    StatisticRequestTester& set_all();
    StatisticRequestTester& clear_all();
};

}

#endif // STATISTICREQUESTTESTER_H_INCLUDED
