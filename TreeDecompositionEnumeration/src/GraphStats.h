#ifndef GRAPHSTATS_H_INCLUDED
#define GRAPHSTATS_H_INCLUDED

#include "Graph.h"
#include "DataStructures.h"
#include <string>

using std::string;

/**
 * Not all metrics are required.
 * The DatasetStatisticsGenerator uses these flags, the GraphStats
 * class uses them to report to the user which values are valid.
 */
#define GRAPHSTATS_N 1
#define GRAPHSTATS_M 2
#define GRAPHSTATS_MS 4
#define GRAPHSTATS_PMC 8
#define GRAPHSTATS_TRNG 16
#define GRAPHSTATS_ALL (-1) // All bits are 1

// Use these macros to test if the bit is on or not in the argument
#define GRAPHSTATS_TEST_N(_flds) (_flds & GRAPHSTATS_N)
#define GRAPHSTATS_TEST_M(_flds) (_flds & GRAPHSTATS_M)
#define GRAPHSTATS_TEST_MS(_flds) (_flds & GRAPHSTATS_MS)
#define GRAPHSTATS_TEST_PMC(_flds) (_flds & GRAPHSTATS_PMC)
#define GRAPHSTATS_TEST_TRNG(_flds) (_flds & GRAPHSTATS_TRNG)

// Use these macros to turn bits on and / or off
#define GRAPHSTATS_SET_N(_flds) (_flds |= GRAPHSTATS_N)
#define GRAPHSTATS_SET_M(_flds) (_flds |= GRAPHSTATS_M)
#define GRAPHSTATS_SET_MS(_flds) (_flds |= GRAPHSTATS_MS)
#define GRAPHSTATS_SET_PMC(_flds) (_flds |= GRAPHSTATS_PMC)
#define GRAPHSTATS_SET_TRNG(_flds) (_flds |= GRAPHSTATS_TRNG)

#define GRAPHSTATS_UNSET_N(_flds) (_flds &= (GRAPHSTATS_ALL ^ GRAPHSTATS_N))
#define GRAPHSTATS_UNSET_M(_flds) (_flds &= (GRAPHSTATS_ALL ^ GRAPHSTATS_M))
#define GRAPHSTATS_UNSET_MS(_flds) (_flds &= (GRAPHSTATS_ALL ^ GRAPHSTATS_MS))
#define GRAPHSTATS_UNSET_PMC(_flds) (_flds &= (GRAPHSTATS_ALL ^ GRAPHSTATS_PMC))
#define GRAPHSTATS_UNSET_TRNG(_flds) (_flds &= (GRAPHSTATS_ALL ^ GRAPHSTATS_TRNG))

/**
 * If calculations take too long, limit the number of
 * minimal separators / triangulations calculated (PMCs
 * are calculated all-or-nothing anyway), and / or the amount
 * of time (in seconds) required.
 *
 * To run with no limit, set DSG_<MS|TRNG|PMC>_<COUNT|TIME>_LIMIT
 * to DSG_NO_LIMIT.
 *
 * The output statistic will contain a "t" for out-of-time, or
 * a "+" for hitting DSG_COUNT limits.
 */
#define GRAPHSTATS_NO_LIMIT (-1)
#define GRAPHSTATS_MS_COUNT_LIMIT (500000)
#define GRAPHSTATS_TRNG_COUNT_LIMIT GRAPHSTATS_MS_COUNT_LIMIT
#define GRAPHSTATS_MS_TIME_LIMIT (20*60) // Ten minutes
#define GRAPHSTATS_TRNG_TIME_LIMIT GRAPHSTATS_MS_TIME_LIMIT
#define GRAPHSTATS_PMC_TIME_LIMIT (5*60)



/**
 * This class represents graph statistical data.
 * Minimal separators, PMCs, triangulations, time metrics.
 */
namespace tdenum {

class GraphStats {
public:

    // General invalid value
    static const long invalid_value;

    Graph g;
    string text;
    int n;
    int m;
    long ms_count;
    long pmc_count;
    long trng_count;
    // Are the (n,m,ms,pmcs,triangs) fields valid for graph i?
    // Note: if the DSG didn't request PMCs, then even if valid
    // is true the PMCs should be zero.
    bool ms_valid;
    bool pmc_valid;
    bool trng_valid;
    // If the limit was overreached, set the relevant flag.
    bool ms_count_limit;
    bool ms_time_limit;
    bool trng_count_limit;
    bool trng_time_limit;
    bool pmc_time_limit;
    // The amount of time required for calculation the minimal separators.
    // Note: PMC calculation time may disregard the time required to calculate
    // the minimal separators of g (see actual_pmc_calc_time())
    time_t ms_calc_time;
    time_t pmc_calc_time;
    time_t trng_calc_time;

    // Data
    NodeSetSet ms;  // Minimal separators
    NodeSetSet pmc; // PMCs

    // Basic constructors.
    // We need a default constructor for containers
    GraphStats();
    GraphStats(const Graph&, const string&);

    // Include the time required to calculate the MSs
    time_t actual_pmc_calc_time() const;

    // Sets all validation flags
    void set_all_invalid();

    // If the value is invalid, returns a special value.
    // If the flag is set to 'true', if the value calculated
    // is invalid because of timeout / count limit, return the value
    // calculated (may be invalid!)
    int get_n() const;
    int get_m() const;
    long get_ms_count(bool get_if_limit = true) const;
    long get_pmc_count(bool get_if_limit = true) const;
    long get_trng_count(bool get_if_limit = true) const;

    // Given a bit mask of active metrics (see the defined GRAPHSTATS_USING_X),
    // return true iff all requested metrics are valid.
    bool valid(int fields = GRAPHSTATS_ALL) const;

    // Print to stream
    friend ostream& operator<<(ostream&, const GraphStats&);

};

}

#endif // GRAPHSTATS_H_INCLUDED
