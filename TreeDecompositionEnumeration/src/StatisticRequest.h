#ifndef STATISTICREQUEST_H_INCLUDED
#define STATISTICREQUEST_H_INCLUDED

#include <ostream>
#include <set>
#include "PMCAlg.h"
using std::ostream;
using std::set;

namespace tdenum {

/**
 * Text to display as an error when PMC calculations are requested but no
 * PMC algorithm was selected.
 */
#define STATISTICREQUEST_TXT_NO_PMCALG \
    "No PMC algorithm specified! Computations involving PMCs will NOT be calculated unless an algorithm is provided"

/**
 * Flags that can be turned on/off in the StatisticRequest class.
 *
 * Syntax is X(A,B,C) where:
 * - A is the name of the field to be calculated
 * - B is the boolean statement to test whether or not the computation
 *   may currently be set. For example, if count_pmc is requested, the
 *   user must set at least one PMC algorithm.
 * - C the text to be displayed if B is false.
 *
 * Note that count requests require less memory then actual object requests.
 */

#define STATISTICREQUEST_FLAG_TABLE \
    /* Request numeric statistics */ \
    X(count_ms,true,"") \
    X(count_pmc,has_valid_pmc_alg(),STATISTICREQUEST_TXT_NO_PMCALG) \
    X(count_trng,true,"") \
    /* Request actual objects (if get_ms is true, the actual minimal separators
     will be stored in the GraphStats object when calculated using the Dataset
     class) */\
    X(ms,true,"") \
    X(pmc,has_valid_pmc_alg(),STATISTICREQUEST_TXT_NO_PMCALG) \
    X(trng,true,"") \
    /* Certain PMC algorithms calculate the minimal separators of the n-1 subgraphs
       of a given graph. If PMC calculation (or enumeration) is on and a reverse-MS
       strain of PMC algorithm is one of the algorithms in the algorithm set, this
       flag can be set to true to invoke storage of all minimal separator sets for
       all subgraphs in the GraphStats object. */ \
    X(ms_subgraphs,has_valid_pmc_alg(),STATISTICREQUEST_TXT_NO_PMCALG) \
    X(ms_subgraph_count,has_valid_pmc_alg(),STATISTICREQUEST_TXT_NO_PMCALG)

/**
 * Calculation sections that may be limited by time or count.
 *
 * The PMC section calculates MSs and MS subgraphs (part of the algorithm),
 * so by limiting the PMC calculation the other calculations are also limited
 * by the same time constraint.
 */
#define STATISTICREQUEST_TIME_LIMITS \
    X(ms) \
    X(pmc) \
    X(trng)

#define STATISTICREQUEST_COUNT_LIMITS \
    X(ms) \
    X(trng)

/**
 * StatisticRequest class.
 *
 * To be used in conjunction with the Dataset object.
 *
 * This object determines the desired data to extract from a dataset. The
 * user may specify which numeric statistics are required, PMC algorithm
 * races... etc.
 */
class StatisticRequest {
private:

    friend class StatisticRequestTester;

    // Flags
    #define X(calculation,...) bool flag_##calculation;
    STATISTICREQUEST_FLAG_TABLE
    #undef X

    // Time limits
    #define X(calculation) time_t time_limit_##calculation;
    STATISTICREQUEST_TIME_LIMITS
    #undef X

    // Count limits
    #define X(calculation) long count_limit_##calculation;
    STATISTICREQUEST_COUNT_LIMITS
    #undef X

    // If count_pmc is set to true, and algorithm must be specified.
    // The default is NORMAL.
    // The algorithms vector will therefore always have at least one
    // item, and the user may request to 'race' different algorithms.
    // Hence, we need this vector.
    set<PMCAlg> pmc_algs_to_race;

    // Utility method
    bool has_valid_pmc_alg() const;

public:

    StatisticRequest();

    /**
     * Reset all flags
     */
    StatisticRequest& reset();

    /**
     * Output
     */
    string str() const;
    friend ostream& operator<<(ostream& os, const StatisticRequest& sr);

    /**
     * Manually set/unset calculations, and query the object.
     *
     * If the user specifies PMCs to be calculated and no algorithm
     * is selected as input, if the set<PMCAlg> field is empty a
     * warning will be reported and PMCs will NOT be calculated!
     */
    #define X(calculation,...) \
        StatisticRequest& set_##calculation(); \
        StatisticRequest& unset_##calculation(); \
        bool test_##calculation() const;
    STATISTICREQUEST_FLAG_TABLE
    #undef X

    // Compound methods
    bool operator==(const StatisticRequest&) const;
    bool operator!=(const StatisticRequest&) const;
    bool test_has_ms_calculation() const;
    bool test_has_pmc_calculation() const;
    bool test_has_trng_calculation() const;
    StatisticRequest& unset_all_ms_calculations();
    StatisticRequest& unset_all_pmc_calculations();
    StatisticRequest& unset_all_trng_calculations();

    // Set/unset/get limits
    // Time limits:
    #define X(calculation) \
        StatisticRequest& set_time_limit_##calculation(time_t); \
        StatisticRequest& unset_time_limit_##calculation(); \
        time_t get_time_limit_##calculation() const; \
        bool test_time_limit_##calculation() const;
    STATISTICREQUEST_TIME_LIMITS
    #undef X
    // Count limits:
    #define X(calculation) \
        StatisticRequest& set_count_limit_##calculation(long); \
        StatisticRequest& unset_count_limit_##calculation(); \
        long get_count_limit_##calculation() const; \
        bool test_count_limit_##calculation() const;
    STATISTICREQUEST_COUNT_LIMITS
    #undef X

    // Race different PMC algorithms.
    // Note that if the user removes all algorithms, PMCs will NOT be calculated
    // at all!
    StatisticRequest& set_single_pmc_alg(const PMCAlg& alg=PMCAlg()); // Removes all other algorithms
    StatisticRequest& add_alg_to_pmc_race(const PMCAlg&);
    StatisticRequest& add_algs_to_pmc_race(const set<PMCAlg>&);
    StatisticRequest& remove_alg_from_pmc_race(const PMCAlg&);
    StatisticRequest& remove_algs_from_pmc_race(const set<PMCAlg>&);
    StatisticRequest& set_all_algs_to_pmc_race();
    StatisticRequest& remove_all_algs_from_pmc_race();
    bool is_active_alg(const PMCAlg&) const;
    set<PMCAlg> get_active_pmc_algs() const;
    vector<PMCAlg> get_active_pmc_algs_vector() const;

};

}

#endif // STATISTICREQUEST_H_INCLUDED
