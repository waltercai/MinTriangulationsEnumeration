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
GraphStats::GraphStats(const Graph& graph, const string& s, bool is_rand, double prob, int inst, bool ff) :
                       g(graph),
                       text(s),
                       n(g.getNumberOfNodes()),
                       m(g.getNumberOfEdges()),
                       count_ms(0),
                       count_pmc(0),
                       count_trng(0),
                       is_random_flag(is_rand),
                       p(prob),
                       actual_ratio(2*(double)graph.getNumberOfEdges()/(graph.getNumberOfNodes()*(graph.getNumberOfNodes()-1))),
                       instance(inst),
                       from_file(ff),
                       calculated_ms(false),
                       calculated_pmc(false),
                       calculated_trng(false),
                       time_limit_ms(0),
                       time_limit_pmc(0),
                       time_limit_trng(0),
                       count_limit_ms(0),
                       count_limit_trng(0),
                       reached_time_limit_flag_ms(false),
                       reached_time_limit_flag_pmc(false),
                       reached_time_limit_flag_trng(false),
                       reached_count_limit_flag_ms(false),
                       reached_count_limit_flag_trng(false),
                       mem_error_flag_ms(false),
                       mem_error_flag_pmc(false),
                       mem_error_flag_trng(false),
                       calc_time_ms(0),
                       calc_time_pmc(0),
                       calc_time_trng(0),
                       ms(graph.getNumberOfNodes())
{}

time_t GraphStats::actual_pmc_calc_time() const {
    return calc_time_ms + calc_time_pmc;
}

void GraphStats::set_all_invalid() {
    calculated_ms = calculated_pmc = calculated_trng = false;
}


// Set / unset limits
void GraphStats::set_ms_time_limit(time_t t) {
    time_limit_ms = t;
}
void GraphStats::set_pmc_time_limit(time_t t) {
    time_limit_pmc = t;
}
void GraphStats::set_trng_time_limit(time_t t) {
    time_limit_ms = t;
}
void GraphStats::set_ms_count_limit(long c) {
    count_limit_ms = c;
}
void GraphStats::set_trng_count_limit(long c) {
    count_limit_trng = c;
}
void GraphStats::unset_ms_time_limit() {
    time_limit_ms = 0;
}
void GraphStats::unset_pmc_time_limit() {
    time_limit_pmc = 0;
}
void GraphStats::unset_trng_time_limit() {
    time_limit_trng = 0;
}
void GraphStats::unset_ms_count_limit() {
    count_limit_ms = 0;
}
void GraphStats::unset_trng_count_limit() {
    count_limit_trng = 0;
}
time_t GraphStats::get_ms_time_limit() const {
    return time_limit_ms;
}
time_t GraphStats::get_pmc_time_limit() const {
    return time_limit_pmc;
}
time_t GraphStats::get_trng_time_limit() const {
    return time_limit_trng;
}
long GraphStats::get_ms_count_limit() const {
    return count_limit_ms;
}
long GraphStats::get_trng_count_limit() const {
    return count_limit_trng;
}
bool GraphStats::has_ms_time_limit() const {
    return time_limit_ms > 0;
}
bool GraphStats::has_pmc_time_limit() const {
    return time_limit_pmc > 0;
}
bool GraphStats::has_trng_time_limit() const {
    return time_limit_trng > 0;
}
bool GraphStats::has_ms_count_limit() const {
    return count_limit_ms > 0;
}
bool GraphStats::has_trng_count_limit() const {
    return count_limit_trng > 0;
}
void GraphStats::set_reached_time_limit_ms() {
    reached_time_limit_flag_ms = true;
}
void GraphStats::set_reached_time_limit_pmc() {
    reached_time_limit_flag_ms = true;
}
void GraphStats::set_reached_time_limit_trng() {
    reached_time_limit_flag_trng = true;
}
void GraphStats::set_reached_count_limit_ms() {
    reached_count_limit_flag_ms = true;
}
void GraphStats::set_reached_count_limit_trng() {
    reached_count_limit_flag_trng = true;
}
void GraphStats::set_mem_error_ms() {
    mem_error_flag_ms = true;
}
void GraphStats::set_mem_error_pmc() {
    mem_error_flag_pmc = true;
}
void GraphStats::set_mem_error_trng() {
    mem_error_flag_trng = true;
}
bool GraphStats::reached_time_limit_ms() const {
    return reached_time_limit_flag_ms;
}
bool GraphStats::reached_time_limit_pmc() const {
    return reached_time_limit_flag_ms;
}
bool GraphStats::reached_time_limit_trng() const {
    return reached_time_limit_flag_trng;
}
bool GraphStats::reached_count_limit_ms() const {
    return reached_count_limit_flag_ms;
}
bool GraphStats::reached_count_limit_trng() const {
    return reached_count_limit_flag_trng;
}
bool GraphStats::mem_error_ms() const {
    return mem_error_flag_ms;
}
bool GraphStats::mem_error_pmc() const {
    return mem_error_flag_ms;
}
bool GraphStats::mem_error_trng() const {
    return mem_error_flag_ms;
}

string GraphStats::get_text() const {
    return text;
}
Graph GraphStats::get_graph() const {
    return g;
}
int GraphStats::get_n() const {
    return n;
}
int GraphStats::get_m() const {
    return m;
}
PMCAlg GraphStats::get_pmc_alg() const {
    return alg;
}
long GraphStats::get_ms_count() const {
    return count_ms;
}
long GraphStats::get_pmc_count() const {
    return count_pmc;
}
long GraphStats::get_trng_count() const {
    return count_trng;
}
time_t GraphStats::get_ms_calc_time() const {
    return calc_time_ms;
}
time_t GraphStats::get_pmc_calc_time() const {
    return calc_time_pmc;
}
time_t GraphStats::get_trng_calc_time() const {
    return calc_time_trng;
}
time_t GraphStats::get_pmc_calc_time_by_alg(PMCAlg a) const {
    if (calc_time_by_alg_pmc.find(a) == calc_time_by_alg_pmc.end()) {
        TRACE(TRACE_LVL__ERROR, "PMC not calculated using the algorithm '" << a << "'");
        return -1;
    }
    return calc_time_by_alg_pmc.at(a);
}


void GraphStats::set_pmc_alg(PMCAlg a) {
    alg = a;
}
void GraphStats::set_ms_calc_time(time_t t) {
    calc_time_ms = t;
}
void GraphStats::set_pmc_calc_time(time_t t) {
    calc_time_pmc = t;
}
void GraphStats::set_trng_calc_time(time_t t) {
    calc_time_trng = t;
}
void GraphStats::set_pmc_calc_time_by_alg(PMCAlg a, time_t t) {
    calc_time_by_alg_pmc[a] = t;
}
void GraphStats::set_ms_count(long c) {
    count_ms = c;
}
void GraphStats::set_pmc_count(long c) {
    count_pmc = c;
}
void GraphStats::set_trng_count(long c) {
    count_trng = c;
}
void GraphStats::set_random() {
    is_random_flag = true;
}
void GraphStats::unset_random() {
    is_random_flag = false;
}
void GraphStats::set_p(double prob) {
    p = prob;
}
double GraphStats::get_p() const {
    return p;
}
void GraphStats::refresh_edge_ratio() {
    int N = g.getNumberOfNodes();
    int M = g.getNumberOfEdges();
    actual_ratio = 2*(double)M / (double(N)*double(N-1));
}
bool GraphStats::is_from_file() const {
    return from_file;
}
bool GraphStats::is_random() const {
    return is_random_flag;
}

bool GraphStats::ms_valid() const {
    return calculated_ms && !reached_count_limit_ms() && !reached_time_limit_ms();
}
bool GraphStats::pmc_valid() const {
    return calculated_pmc && !reached_time_limit_pmc();
}
bool GraphStats::trng_valid() const {
    return calculated_trng && !reached_count_limit_trng() && !reached_time_limit_trng();
}


void GraphStats::set_ms(const NodeSetSet& seps) {
    ms.back() = seps;
    set_ms_count(ms.size());
    calculated_ms = true;
}
void GraphStats::set_pmc(const NodeSetSet& p) {
    pmc = p;
    set_pmc_count(p.size());
    calculated_pmc = true;
}
void GraphStats::set_trng(const vector<ChordalGraph>& t) {
    trng = t;
    set_trng_count(t.size());
    calculated_trng = true;
}
NodeSetSet GraphStats::get_ms(bool get_if_limit) const {
    return ms[g.getNumberOfNodes()-1];
}
NodeSetSet GraphStats::get_subgraph_ms(int i, bool get_if_limit) const {
    if (i >= g.getNumberOfNodes() || i < 0) {
        TRACE(TRACE_LVL__ERROR, "Bad index '" << i << "' given as argument (must support 0<=i<=" << g.getNumberOfNodes() << ")");
        return NodeSetSet();
    }
    return ms[i];
}
NodeSetSet GraphStats::get_pmc(bool get_if_limit) const {
    return pmc;
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
        (!GRAPHSTATS_TEST_MS(fields) || ms_valid()) &&
        (!GRAPHSTATS_TEST_PMC(fields) || pmc_valid()) &&
        (!GRAPHSTATS_TEST_TRNG(fields) || trng_valid());
}



ostream& operator<<(ostream& os, const GraphStats& gs) {
    os << "===PRINTING GRAPH STATS===" << endl;
    os << "Graph '" << gs.text << "' has " << gs.n << " nodes and " << gs.m << " edges:" << endl << gs.g;

    // MSs
    if (gs.valid(GRAPHSTATS_MS)) {
        os << "Printing " << gs.count_ms << " minimal separators within "
           << utils__secs_to_hhmmss(gs.calc_time_ms) << ":" << endl << gs.ms << endl;
    }
    else if (gs.reached_count_limit_flag_ms || gs.reached_time_limit_flag_ms) {
        os << "Reached " << (gs.reached_count_limit_flag_ms ? "count" : "time") << " limit "
           << "at " << (gs.reached_count_limit_flag_ms ? UTILS__TO_STRING(gs.count_ms) : utils__secs_to_hhmmss(gs.calc_time_ms))
           << " for minimal separators." << endl;
    }

    // PMCs
    if (gs.valid(GRAPHSTATS_PMC)) {
        os << "Printing " << gs.count_pmc << " PMCs within "
           << utils__secs_to_hhmmss(gs.calc_time_pmc) << ":" << endl << gs.pmc << endl;
    }
    else if (gs.reached_time_limit_flag_pmc) {
        os << "Reached time limit at " << utils__secs_to_hhmmss(gs.calc_time_pmc)
           << " (including Minimal separators precalculation: "
           << utils__secs_to_hhmmss(gs.actual_pmc_calc_time())
           << " for PMCs." << endl;
    }

    // Triangulations
    if (gs.valid(GRAPHSTATS_TRNG)) {
        os << "Found " << gs.count_trng << " triangulations within "
           << utils__secs_to_hhmmss(gs.calc_time_trng) << "." << endl;
    }
    else if (gs.reached_count_limit_flag_trng || gs.reached_time_limit_flag_trng) {
        os << "Reached " << (gs.reached_count_limit_flag_trng ? "count" : "time") << " limit at "
           << (gs.reached_count_limit_flag_trng ? UTILS__TO_STRING(gs.count_trng) : utils__secs_to_hhmmss(gs.calc_time_trng))
           << " for minimal triangulations." << endl;
    }

    // That's it!
    os << "===DONE PRINTING GRAPH STATS===";
    return os;
}

}
