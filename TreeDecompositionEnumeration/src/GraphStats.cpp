#include "GraphStats.h"
#include "Graph.h"
#include "Utils.h"

namespace tdenum {

#define X(_name,_) bool GRAPHSTATS_TEST_##_name(int _flds) { return (_flds & GRAPHSTATS_##_name); }
GRAPHSTATS_FIELD_TABLE
#undef X
#define X(_name,_) void GRAPHSTATS_SET_##_name(int& _flds) { _flds |= GRAPHSTATS_##_name; }
GRAPHSTATS_FIELD_TABLE
#undef X
#define X(_name,_) void GRAPHSTATS_UNSET_##_name(int& _flds) { _flds &= (GRAPHSTATS_ALL ^ GRAPHSTATS_##_name); }
GRAPHSTATS_FIELD_TABLE
#undef X

const long GraphStats::invalid_value = -1;

GraphStats::GraphStats() : GraphStats(Graph(), "") {}
GraphStats::GraphStats(const Graph& graph, const string& s, bool is_rand, double prob, int inst) :
                       g(graph),
                       text(s),
                       n(g.getNumberOfNodes()),
                       m(g.getNumberOfEdges()),
                       ms_count(0),
                       pmc_count(0),
                       trng_count(0),
                       is_random(is_rand),
                       p(prob),
                       actual_ratio(2*(double)graph.getNumberOfEdges()/(graph.getNumberOfNodes()*(graph.getNumberOfNodes()-1))),
                       instance(inst),
                       ms_valid(false),
                       pmc_valid(false),
                       trng_valid(false),
                       ms_time_limit(0),
                       pmc_time_limit(0),
                       trng_time_limit(0),
                       ms_count_limit(0),
                       trng_count_limit(0),
                       ms_reached_time_limit(false),
                       pmc_reached_time_limit(false),
                       trng_reached_time_limit(false),
                       ms_reached_count_limit(false),
                       trng_reached_count_limit(false),
                       ms_mem_error(false),
                       pmc_mem_error(false),
                       trng_mem_error(false),
                       ms_calc_time(0),
                       pmc_calc_time(0),
                       trng_calc_time(0),
                       ms(graph.getNumberOfNodes())
{}

time_t GraphStats::actual_pmc_calc_time() const {
    return ms_calc_time + pmc_calc_time;
}

void GraphStats::set_all_invalid() {
    ms_valid = false;
    pmc_valid = false;
    trng_valid = false;
}


// Set / unset limits
void GraphStats::set_ms_time_limit(time_t t) {
    ms_time_limit = t;
}
void GraphStats::set_pmc_time_limit(time_t t) {
    pmc_time_limit = t;
}
void GraphStats::set_trng_time_limit(time_t t) {
    ms_time_limit = t;
}
void GraphStats::set_ms_count_limit(long c) {
    ms_count_limit = c;
}
void GraphStats::set_trng_count_limit(long c) {
    trng_count_limit = c;
}
void GraphStats::unset_ms_time_limit() {
    ms_time_limit = 0;
}
void GraphStats::unset_pmc_time_limit() {
    pmc_time_limit = 0;
}
void GraphStats::unset_trng_time_limit() {
    trng_time_limit = 0;
}
void GraphStats::unset_ms_count_limit() {
    ms_count_limit = 0;
}
void GraphStats::unset_trng_count_limit() {
    trng_count_limit = 0;
}
time_t GraphStats::get_ms_time_limit() const {
    return ms_time_limit;
}
time_t GraphStats::get_pmc_time_limit() const {
    return pmc_time_limit;
}
time_t GraphStats::get_trng_time_limit() const {
    return trng_time_limit;
}
long GraphStats::get_ms_count_limit() const {
    return ms_count_limit;
}
long GraphStats::get_trng_count_limit() const {
    return trng_count_limit;
}
bool GraphStats::has_ms_time_limit() const {
    return ms_time_limit > 0;
}
bool GraphStats::has_pmc_time_limit() const {
    return pmc_time_limit > 0;
}
bool GraphStats::has_trng_time_limit() const {
    return trng_time_limit > 0;
}
bool GraphStats::has_ms_count_limit() const {
    return ms_count_limit > 0;
}
bool GraphStats::has_trng_count_limit() const {
    return trng_count_limit > 0;
}

#define MS_REACHED_LIMIT_OR_GET_ANYWAY \
    (ms_valid || (get_if_limit && (ms_reached_count_limit || ms_reached_time_limit)))
int GraphStats::get_n() const {
    return n;
}
int GraphStats::get_m() const {
    return m;
}
long GraphStats::get_ms_count(bool get_if_limit) const {
    return (MS_REACHED_LIMIT_OR_GET_ANYWAY)
        ? ms_count : GraphStats::invalid_value;
}
long GraphStats::get_pmc_count(bool get_if_limit) const {
    return (pmc_valid || (get_if_limit && pmc_reached_time_limit))
        ? pmc_count : GraphStats::invalid_value;
}
long GraphStats::get_trng_count(bool get_if_limit) const {
    return (trng_valid || (get_if_limit && (trng_reached_count_limit || trng_reached_time_limit)))
        ? trng_count : GraphStats::invalid_value;
}

void GraphStats::set_ms(const NodeSetSet& seps) {
    ms.back() = seps;
    ms_count = ms.size();
}
void GraphStats::set_pmc(const NodeSetSet& p) {
    pmc = p;
    pmc_count = p.size();
}
NodeSetSet GraphStats::get_ms(bool get_if_limit) const {
    return (MS_REACHED_LIMIT_OR_GET_ANYWAY)
        ? ms[g.getNumberOfNodes()-1] : NodeSetSet();
}
NodeSetSet GraphStats::get_subgraph_ms(int i, bool get_if_limit) const {
    return (MS_REACHED_LIMIT_OR_GET_ANYWAY)
        ? ms[g.getNumberOfNodes()-1] : NodeSetSet();
}
NodeSetSet GraphStats::get_pmc(bool get_if_limit) const {
    return (pmc_valid || (get_if_limit && pmc_reached_time_limit))
        ? pmc : NodeSetSet();
}

int GraphStats::get_instance() const {
    return instance;
}
void GraphStats::set_instance(int inst) {
    if (inst <= 0) {
        cout << "Instance number should be at least 1. "
             << "Leaving the instance number as '" << instance << "'" << endl;
        return;
    }
    instance = inst;
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
           << utils__secs_to_hhmmss(gs.ms_calc_time) << ":" << endl << gs.ms << endl;
    }
    else if (gs.ms_reached_count_limit || gs.ms_reached_time_limit) {
        os << "Reached " << (gs.ms_reached_count_limit ? "count" : "time") << " limit "
           << "at " << (gs.ms_reached_count_limit ? UTILS__TO_STRING(gs.ms_count) : utils__secs_to_hhmmss(gs.ms_calc_time))
           << " for minimal separators." << endl;
    }

    // PMCs
    if (gs.valid(GRAPHSTATS_PMC)) {
        os << "Printing " << gs.pmc_count << " PMCs within "
           << utils__secs_to_hhmmss(gs.pmc_calc_time) << ":" << endl << gs.pmc << endl;
    }
    else if (gs.pmc_reached_time_limit) {
        os << "Reached time limit at " << utils__secs_to_hhmmss(gs.pmc_calc_time)
           << " (including Minimal separators precalculation: "
           << utils__secs_to_hhmmss(gs.actual_pmc_calc_time())
           << " for PMCs." << endl;
    }

    // Triangulations
    if (gs.valid(GRAPHSTATS_TRNG)) {
        os << "Found " << gs.trng_count << " triangulations within "
           << utils__secs_to_hhmmss(gs.trng_calc_time) << "." << endl;
    }
    else if (gs.trng_reached_count_limit || gs.trng_reached_time_limit) {
        os << "Reached " << (gs.trng_reached_count_limit ? "count" : "time") << " limit at "
           << (gs.trng_reached_count_limit ? UTILS__TO_STRING(gs.trng_count) : utils__secs_to_hhmmss(gs.trng_calc_time))
           << " for minimal triangulations." << endl;
    }

    // That's it!
    os << "===DONE PRINTING GRAPH STATS===";
    return os;
}

}
