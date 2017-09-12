#ifndef GRAPHSTATS_H_INCLUDED
#define GRAPHSTATS_H_INCLUDED

#include "Graph.h"
#include "DataStructures.h"
#include <string>

using std::string;

namespace tdenum {


/**
 * Not all metrics are required.
 * The DatasetStatisticsGenerator uses these flags, the GraphStats
 * class uses them to report to the user which values are valid.
 */
#define GRAPHSTATS_FIELD_TABLE \
    X(N,1) \
    X(M,2) \
    X(MS,4) \
    X(PMC,8) \
    X(TRNG,16) \
    X(MS_SUBGRAPH_COUNT,32) \
    X(ALL,-1)

// As in: const int GRAPHSTATS_M = 2;
#define X(_name,_id) const int GRAPHSTATS_##_name = _id;
GRAPHSTATS_FIELD_TABLE
#undef X

// Use these macros to test if the bit is on or not in the argument,
// as in: bool GRAPHSTATS_TEST_N(int _flds) { return (_flds & GRAPHSTATS_N); }
#define X(_name,_) bool GRAPHSTATS_TEST_##_name(int _flds);
GRAPHSTATS_FIELD_TABLE
#undef X

// Use these macros to turn bits on and / or off
#define X(_name,_) void GRAPHSTATS_SET_##_name(int& _flds);
GRAPHSTATS_FIELD_TABLE
#undef X
#define X(_name,_) void GRAPHSTATS_UNSET_##_name(int& _flds);
GRAPHSTATS_FIELD_TABLE
#undef X

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
class GraphStats {
public:

    // General invalid value
    static const long invalid_value;

    // Basic graph data
    Graph g;
    string text;
    int n;
    int m;
    long ms_count;
    long pmc_count;
    long trng_count;

    // Useful fields for random graphs
    bool is_random;
    double p, actual_ratio;
    int instance;

    // If this graph was read from a file, set this flag to true
    bool from_file;

    // Are the (n,m,ms,pmcs,triangs) fields valid for graph i?
    // Note: if the DSG didn't request PMCs, then even if valid
    // is true the PMCs should be zero.
    bool ms_valid;
    bool pmc_valid;
    bool trng_valid;

    // If the limit was overreached, set the relevant flag.
    time_t ms_time_limit;
    time_t pmc_time_limit;
    time_t trng_time_limit;
    long ms_count_limit;
    long trng_count_limit;
    bool ms_reached_time_limit;
    bool pmc_reached_time_limit;
    bool trng_reached_time_limit;
    bool ms_reached_count_limit;
    bool trng_reached_count_limit;
    bool ms_mem_error;
    bool pmc_mem_error;
    bool trng_mem_error;

    // The amount of time required for calculation the minimal separators.
    // Note: PMC calculation time may disregard the time required to calculate
    // the minimal separators of g (see actual_pmc_calc_time())
    time_t ms_calc_time;
    time_t pmc_calc_time;
    time_t trng_calc_time;

    // Data.
    // Minimal separators of all subgraphs may also be stored.
    vector<NodeSetSet> ms;  // Minimal separators
    NodeSetSet pmc;         // PMCs

    // Basic constructors.
    // We need a default constructor for containers.
    GraphStats();
    GraphStats(const Graph& g,
               const string& text,
               bool is_rand = false,
               double p = 0,
               int inst = 1,
               bool file = false);

    // Include the time required to calculate the MSs
    time_t actual_pmc_calc_time() const;

    // Sets all validation flags
    void set_all_invalid();

    // Set / unset limits
    void set_ms_time_limit(time_t);
    void set_pmc_time_limit(time_t);
    void set_trng_time_limit(time_t);
    void set_ms_count_limit(long);
    void set_trng_count_limit(long);
    void unset_ms_time_limit();
    void unset_pmc_time_limit();
    void unset_trng_time_limit();
    void unset_ms_count_limit();
    void unset_trng_count_limit();
    time_t get_ms_time_limit() const;
    time_t get_pmc_time_limit() const;
    time_t get_trng_time_limit() const;
    long get_ms_count_limit() const;
    long get_trng_count_limit() const;
    bool has_ms_time_limit() const;
    bool has_pmc_time_limit() const;
    bool has_trng_time_limit() const;
    bool has_ms_count_limit() const;
    bool has_trng_count_limit() const ;

    // If the value is invalid, returns a special value.
    // If the flag is set to 'true', if the value calculated
    // is invalid because of timeout / count limit, return the value
    // calculated (may be invalid!)
    int get_n() const;
    int get_m() const;
    long get_ms_count(bool get_if_limit = true) const;
    long get_pmc_count(bool get_if_limit = true) const;
    long get_trng_count(bool get_if_limit = true) const;

    // Data utility methods
    void set_ms(const NodeSetSet&);
    void set_pmc(const NodeSetSet&);
    NodeSetSet get_ms(bool get_if_limit = true) const;
    NodeSetSet get_subgraph_ms(int i, bool get_if_limit = true) const;
    NodeSetSet get_pmc(bool get_if_limit = true) const;

    // Graph instance
    int get_instance() const;
    void set_instance(int inst);

    // Given a bit mask of active metrics (see the defined GRAPHSTATS_USING_X),
    // return true iff all requested metrics are valid.
    bool valid(int fields = GRAPHSTATS_ALL) const;

    // Print to stream
    friend ostream& operator<<(ostream&, const GraphStats&);

};

}

#endif // GRAPHSTATS_H_INCLUDED
