#include "GraphStats.h"
#include "Graph.h"

namespace tdenum {

const long GraphStats::invalid_value = -1;

GraphStats::GraphStats() : GraphStats(Graph(), "") {}
GraphStats::GraphStats(const Graph& graph, const string& s) :
                       g(graph),
                       text(s),
                       n(g.getNumberOfNodes()),
                       m(g.getNumberOfEdges()),
                       ms_count(0),
                       pmc_count(0),
                       trng_count(0),
                       ms_valid(false),
                       pmc_valid(false),
                       trng_valid(false),
                       ms_count_limit(false),
                       ms_time_limit(false),
                       trng_count_limit(false),
                       trng_time_limit(false),
                       pmc_time_limit(false),
                       ms_calc_time(0),
                       pmc_calc_time(0)
{}

time_t GraphStats::actual_pmc_calc_time() const {
    return ms_calc_time + pmc_calc_time;
}

void GraphStats::set_all_invalid() {
    ms_valid = false;
    pmc_valid = false;
    trng_valid = false;
}

int GraphStats::get_n() const {
    return n;
}
int GraphStats::get_m() const {
    return m;
}
long GraphStats::get_ms_count(bool get_if_limit) const {
    return (ms_valid || (get_if_limit && (ms_count_limit || ms_time_limit)))
        ? ms_count : GraphStats::invalid_value;
}
long GraphStats::get_pmc_count(bool get_if_limit) const {
    return (pmc_valid || (get_if_limit && pmc_time_limit))
        ? pmc_count : GraphStats::invalid_value;
}
long GraphStats::get_trng_count(bool get_if_limit) const {
    return (trng_valid || (get_if_limit && (trng_count_limit || trng_time_limit)))
        ? trng_count : GraphStats::invalid_value;
}

bool GraphStats::valid(int fields) const {
    // A value is valid if it's being used and has a valid calculated value.
    return
        GRAPHSTATS_TEST_MS(fields) && ms_valid &&
        GRAPHSTATS_TEST_PMC(fields) && pmc_valid &&
        GRAPHSTATS_TEST_TRNG(fields) && trng_valid;
}

}
