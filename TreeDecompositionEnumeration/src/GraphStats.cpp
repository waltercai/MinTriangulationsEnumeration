#include "GraphStats.h"
#include "Graph.h"
#include "Utils.h"

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
    return
        (!GRAPHSTATS_TEST_MS(fields) || ms_valid) &&
        (!GRAPHSTATS_TEST_PMC(fields) || pmc_valid) &&
        (!GRAPHSTATS_TEST_TRNG(fields) || trng_valid);
}



ostream& operator<<(ostream& os, const GraphStats& gs) {
    os << "===PRINTING GRAPH STATS===" << endl;
    os << "Graph '" << gs.text << "' has " << gs.n << " nodes and " << gs.m << " edges:" << endl << gs.g;

    // MSs
    if (gs.valid(GRAPHSTATS_MS)) {
        os << "Printing " << gs.ms_count << " minimal separators within "
           << secs_to_hhmmss(gs.ms_calc_time) << ":" << endl << gs.ms << endl;
    }
    else if (gs.ms_count_limit || gs.ms_time_limit) {
        os << "Reached " << (gs.ms_count_limit ? "count" : "time") << " limit "
           << "at " << (gs.ms_count_limit ? TO_STRING(gs.ms_count) : secs_to_hhmmss(gs.ms_calc_time))
           << " for minimal separators." << endl;
    }

    // PMCs
    if (gs.valid(GRAPHSTATS_PMC)) {
        os << "Printing " << gs.pmc_count << " PMCs within "
           << secs_to_hhmmss(gs.pmc_calc_time) << ":" << endl << gs.pmc << endl;
    }
    else if (gs.pmc_time_limit) {
        os << "Reached time limit at " << secs_to_hhmmss(gs.pmc_calc_time)
           << " (including Minimal separators precalculation: "
           << secs_to_hhmmss(gs.actual_pmc_calc_time())
           << " for PMCs." << endl;
    }

    // Triangulations
    if (gs.valid(GRAPHSTATS_TRNG)) {
        os << "Found " << gs.trng_count << " triangulations within "
           << secs_to_hhmmss(gs.trng_calc_time) << "." << endl;
    }
    else if (gs.trng_count_limit || gs.trng_time_limit) {
        os << "Reached " << (gs.trng_count_limit ? "count" : "time") << " limit at "
           << (gs.trng_count_limit ? TO_STRING(gs.trng_count) : secs_to_hhmmss(gs.trng_calc_time))
           << " for minimal triangulations." << endl;
    }

    // That's it!
    os << "===DONE PRINTING GRAPH STATS===";
    return os;
}

}
