#include "StatisticRequest.h"
#include "Utils.h"

namespace tdenum {

StatisticRequest::StatisticRequest() :
    #define X(calculation,...) flag_##calculation(false),
    STATISTICREQUEST_FLAG_TABLE
    #undef X
    #define X(calculation) time_limit_##calculation(invalid_time_limit),
    STATISTICREQUEST_TIME_LIMITS
    #undef X
    #define X(calculation) count_limit_##calculation(invalid_count_limit),
    STATISTICREQUEST_COUNT_LIMITS
    #undef X
    pmc_algs_to_race()
    {}

StatisticRequest& StatisticRequest::reset() { return (*this = StatisticRequest()); }

string StatisticRequest::str() const {
    ostringstream oss;
    vector<string> vs;
    #define X(calculation,...) \
    if(test_##calculation()) { \
        vs.push_back(#calculation " is ON"); \
    }
    STATISTICREQUEST_FLAG_TABLE
    #undef X
    #define X(calculation) \
    if (test_time_limit_##calculation()) { \
        vs.push_back(string(#calculation) + string(" has a time limit of ") + \
                     utils__timestamp_to_hhmmss(get_time_limit_##calculation())); \
    }
    STATISTICREQUEST_TIME_LIMITS
    #undef X
    #define X(calculation) \
    if (test_count_limit_##calculation()) { \
        vs.push_back(string(#calculation) + string(" has a count limit of ") + \
                     UTILS__TO_STRING(get_count_limit_##calculation())); \
    }
    STATISTICREQUEST_COUNT_LIMITS
    #undef X
    oss << vs;
    return oss.str();
}
ostream& operator<<(ostream& os, const StatisticRequest& sr) { return (os << sr.str()); }

bool StatisticRequest::has_valid_pmc_alg() const { return !pmc_algs_to_race.empty(); }
bool StatisticRequest::valid_time_ms() const { return time_limit_ms != invalid_time_limit; }
bool StatisticRequest::valid_time_pmc() const { return time_limit_pmc != invalid_time_limit; }
bool StatisticRequest::valid_time_trng() const { return time_limit_trng != invalid_time_limit; }
bool StatisticRequest::valid_count_ms() const { return count_limit_ms != invalid_count_limit; }
bool StatisticRequest::valid_count_trng() const { return count_limit_trng != invalid_count_limit; }

// Manually set/unset calculations, and query the object.
// Each calculation has a set, unset and test method.
// Extra flag may be checked in setter and tester.
#define X(calculation,can_do,err_text) \
    StatisticRequest& StatisticRequest::set_##calculation() { \
        if (!can_do) { \
            TRACE(TRACE_LVL__WARNING, err_text); \
        } \
        flag_##calculation = true; \
        return *this; \
    } \
    StatisticRequest& StatisticRequest::unset_##calculation() { \
        flag_##calculation = false; \
        return *this; \
    } \
    bool StatisticRequest::test_##calculation() const { return flag_##calculation && can_do; }
STATISTICREQUEST_FLAG_TABLE
#undef X

bool StatisticRequest::operator==(const StatisticRequest& sr) const {
    return (true &&
            #define X(calc,...) \
            test_##calc() == sr.test_##calc() &&
            STATISTICREQUEST_FLAG_TABLE
            #undef X
            #define X(calc) \
            test_count_limit_##calc() == sr.test_count_limit_##calc() && \
            (!test_count_limit_##calc() ? true : (get_count_limit_##calc() == sr.get_count_limit_##calc())) &&
            STATISTICREQUEST_COUNT_LIMITS
            #undef X
            #define X(calc) \
            test_time_limit_##calc() == sr.test_time_limit_##calc() && \
            (!test_time_limit_##calc() ? true : (get_time_limit_##calc() == sr.get_time_limit_##calc())) &&
            STATISTICREQUEST_TIME_LIMITS
            #undef X
            true);
}
bool StatisticRequest::operator!=(const StatisticRequest& sr) const { return !(*this == sr); }
StatisticRequest& StatisticRequest::unset_all_ms_calculations() {
    unset_ms();
    unset_count_ms();
    return *this;
}
StatisticRequest& StatisticRequest::unset_all_pmc_calculations() {
    unset_pmc();
    unset_count_pmc();
    unset_ms_subgraphs();
    unset_ms_subgraph_count();
    return *this;
}
StatisticRequest& StatisticRequest::unset_all_trng_calculations() {
    unset_trng();
    unset_count_trng();
    return *this;
}
bool StatisticRequest::test_has_ms_calculation() const { return test_ms() || test_count_ms(); }
// MS subgraphs are calculated in the PMC section
bool StatisticRequest::test_has_pmc_calculation() const {
    return test_pmc() || test_count_pmc() || test_ms_subgraphs() || test_ms_subgraph_count();
}
bool StatisticRequest::test_has_trng_calculation() const { return test_trng() || test_count_trng(); }

// Time limit handlers
#define X(calculation) \
    StatisticRequest& StatisticRequest::set_time_limit_##calculation(time_t t) { time_limit_##calculation = t; return *this; } \
    StatisticRequest& StatisticRequest::unset_time_limit_##calculation(){ time_limit_##calculation = invalid_time_limit; return *this; } \
    bool StatisticRequest::test_time_limit_##calculation() const { return valid_time_##calculation(); } \
    time_t StatisticRequest::get_time_limit_##calculation() const { \
        if (!valid_time_##calculation()) { \
            TRACE(TRACE_LVL__WARNING, "Time limit value read, but no time limit set!"); \
        } \
        return time_limit_##calculation; \
    }
STATISTICREQUEST_TIME_LIMITS
#undef X

// Count limit handlers
#define X(calculation) \
    StatisticRequest& StatisticRequest::set_count_limit_##calculation(long c) { count_limit_##calculation = c; return *this; } \
    StatisticRequest& StatisticRequest::unset_count_limit_##calculation() { count_limit_##calculation = invalid_count_limit; return *this; } \
    bool StatisticRequest::test_count_limit_##calculation() const { return valid_count_##calculation(); } \
    long StatisticRequest::get_count_limit_##calculation() const { \
        if (!valid_count_##calculation()) { \
            TRACE(TRACE_LVL__WARNING, "Count limit value read, but no count limit set!"); \
        } \
        return count_limit_##calculation; \
    }
STATISTICREQUEST_COUNT_LIMITS
#undef X

StatisticRequest& StatisticRequest::set_single_pmc_alg(const PMCAlg& alg) {
    pmc_algs_to_race.clear();
    return add_alg_to_pmc_race(alg);
}
StatisticRequest& StatisticRequest::add_alg_to_pmc_race(const PMCAlg& alg) { pmc_algs_to_race.insert(alg); return *this; }
StatisticRequest& StatisticRequest::add_algs_to_pmc_race(const set<PMCAlg>& algs) {
    for (auto alg: algs) {
        add_alg_to_pmc_race(alg);
    }
    return *this;
}
StatisticRequest& StatisticRequest::remove_alg_from_pmc_race(const PMCAlg& alg) {
    pmc_algs_to_race.erase(alg);
    if (pmc_algs_to_race.empty()) {
        unset_all_pmc_calculations(); // No algorithm available!
    }
    return *this;
}
StatisticRequest& StatisticRequest::remove_algs_from_pmc_race(const set<PMCAlg>& algs) {
    for (auto alg: algs) {
        remove_alg_from_pmc_race(alg);
    }
    return *this;
}
StatisticRequest& StatisticRequest::set_all_algs_to_pmc_race() {
    for (auto alg: PMCAlg::get_all()) {
        add_alg_to_pmc_race(alg);
    }
    return *this;
}
StatisticRequest& StatisticRequest::remove_all_algs_from_pmc_race() { pmc_algs_to_race.clear(); return *this; }
bool StatisticRequest::is_active_alg(const PMCAlg& a) const { return utils__is_in_set(a,pmc_algs_to_race); }
set<PMCAlg> StatisticRequest::get_active_pmc_algs() const { return pmc_algs_to_race; }
vector<PMCAlg> StatisticRequest::get_active_pmc_algs_vector() const {
    vector<PMCAlg> v;
    for(PMCAlg a: pmc_algs_to_race) {
        v.push_back(a);
    }
    return v;
}

}
