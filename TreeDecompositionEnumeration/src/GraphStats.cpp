#include "Graph.h"
#include "GraphReader.h"
#include "GraphStats.h"
#include "Utils.h"
#include <stdexcept>
using std::invalid_argument;

namespace tdenum {

/**
 * Useful for input validation in several methods
 */
#define ASSERT_IN_INDEX_RANGE_OR_OP(_i,_op) do { \
        if ((_i) < 0 || (_i) >= get_n()) { \
            TRACE(TRACE_LVL__ERROR, "Illegal index '" << _i << "', must be between 0 and " << get_n()); \
            _op; \
        } \
    } while(0)


/**
 * Some formatting constants (MI = metadata index).
 *
 * See read() and str() for usage in implementation, and make sure you know
 * what you're doing if you change these!
 */
enum {
    GRAPHSTATS_MI_N = 0,
    GRAPHSTATS_MI_P_OR_NONRANDOM,
    GRAPHSTATS_MI_INSTANCE,
    GRAPHSTATS_MI_TOTAL_CELLS /* Always last */
};
#define GRAPHSTATS_METADATA_ROWS (1)

// Populate data via the calculated data.
// Also sets validation flags if the source data is valid.
GraphStats& GraphStats::update_ms_count() {
    if (get_n() == 0) {
        return *this;
    }
    set_ms_count(ms.back().size());
    return *this;
}
GraphStats& GraphStats::update_ms_subgraph_count() {
    count_ms = vector<long>(get_n());
    for (unsigned i=0; i<ms.size(); ++i) {
        count_ms[i] = ms[i].size();
    }
    return *this;
}

GraphStats& GraphStats::update_pmc_count() { set_pmc_count(pmc.size()); return *this; }
GraphStats& GraphStats::update_trng_count() { set_trng_count(trng.size()); return *this; }

GraphStats& GraphStats::set_text(const string& txt) { text = txt; return *this; }
string GraphStats::add_ext_if_missing(const string& s) {
    string str = s;
    if (utils__get_extension(str) != GRAPHSTATS_FILE_EXTENSION) {
        str += string(".") + GRAPHSTATS_FILE_EXTENSION;
    }
    return str;
}
string GraphStats::str() const {

    ostringstream oss;
    string delim = ",";
    // Start with metadata
    oss << get_n()
        << delim << (is_random() ? UTILS__TO_STRING(get_p()) : GRAPHSTATS_METADATA_TXT_NOT_RANDOM)
        << delim << get_instance()
        << endl;
    // Now, actual graph:
    for (Node v: g.getNodesVector()) {
        for (Node u: g.getNeighbors({v})) {
            // Always assume simple graphs, and only insert
            // one instance of each edge
            if (v > u) continue;
            // Output in CSV comma-delimited format
            oss << v << "," << u << endl;
        }
    }

    return oss.str();
}

GraphStats::GraphStats(const Graph& graph) :
                   g(graph),
                   text(GRAPHSTATS_NO_FILE_KNOWN),
                   count_ms(graph.getNumberOfNodes(),0),  // vector
                   count_pmc(0),
                   count_trng(0),
                   reached_count_limit_flag_ms(false),
                   reached_count_limit_flag_trng(false),
                   reached_time_limit_flag_ms(false),
                   reached_time_limit_flag_trng(false),
                   mem_error_flag_ms(false),
                   mem_error_flag_trng(false),
                   calc_time_ms(0),
                   calc_time_trng(0),
                   ms(graph.getNumberOfNodes()) // vector
{
    // Init all PMCAlg maps
    for (auto i=PMCAlg::first(); i<PMCAlg::last(); ++i) {
        reached_time_limit_flag_pmc[i] = false;
        mem_error_flag_pmc[i] = false;
        calc_time_by_alg_pmc[i] = 0;
    }
    TRACE(TRACE_LVL__TEST, "Done with constructor");
}

bool GraphStats::equal_except_text(const GraphStats& gs) const {
    return g == gs.g &&
           count_ms == gs.count_ms &&   // Vector comparison. There will be more
           count_pmc == gs.count_pmc &&
           count_trng == gs.count_trng &&
           reached_count_limit_flag_ms == gs.reached_count_limit_flag_ms &&
           reached_count_limit_flag_trng == gs.reached_count_limit_flag_trng &&
           reached_time_limit_flag_ms == gs.reached_time_limit_flag_ms &&
           reached_time_limit_flag_trng == gs.reached_time_limit_flag_trng &&
           mem_error_flag_ms == gs.mem_error_flag_ms &&
           mem_error_flag_trng == gs.mem_error_flag_trng &&
           calc_time_ms == gs.calc_time_ms &&
           calc_time_trng == gs.calc_time_trng &&
           ms == gs.ms &&
           pmc == gs.pmc &&
           trng == gs.trng &&
           reached_time_limit_flag_pmc == gs.reached_time_limit_flag_pmc &&
           mem_error_flag_pmc == gs.mem_error_flag_pmc &&
           calc_time_by_alg_pmc == gs.calc_time_by_alg_pmc;
}
bool GraphStats::operator==(const GraphStats& gs) const { return (equal_except_text(gs) && text == gs.text); }
bool GraphStats::operator!=(const GraphStats& gs) const { return !(*this == gs); }

bool GraphStats::text_valid() const { return !utils__str_eq(text, GRAPHSTATS_NO_FILE_KNOWN) && !utils__str_empty(text); }

string GraphStats::get_dump_filepath() const {
    return text_valid() ? get_text() : GraphStats::get_default_filename_from_graph(get_graph());
}
string GraphStats::get_dump_filepath(const string& prepend_dir) const {
    // Sanity
    if (utils__str_empty(prepend_dir)) {
        return get_dump_filepath();
    }
    string basename = text_valid() ? utils__get_filename(get_text()) : GraphStats::get_default_filename_from_graph(get_graph());
    string prepend_dir_cpy = prepend_dir;
    if (prepend_dir_cpy.back() == '/' || prepend_dir_cpy.back() == '\\') {
        prepend_dir_cpy.pop_back();
    }
    return prepend_dir_cpy+"/"+basename;
}
string GraphStats::get_default_filename_from_graph(const Graph& g) {
    string filename = GRAPHSTATS_DEFAULT_FILENAME_EXPR;
    filename = utils__replace_substr_with_substr(filename, GRAPHSTATS_DEFAULT_FILENAME_TOKEN_N, UTILS__TO_STRING(g.getNumberOfNodes()));
    filename = utils__replace_substr_with_substr(filename, GRAPHSTATS_DEFAULT_FILENAME_TOKEN_P, UTILS__TO_STRING(g.getP()));
    filename = utils__replace_substr_with_substr(filename, GRAPHSTATS_DEFAULT_FILENAME_TOKEN_INST, UTILS__TO_STRING(g.getInstance()));
    return filename;
}


// Set / unset limits.
// These are all one-liners, no need to make it unreadable by breaking lines.
GraphStats& GraphStats::set_reached_time_limit_ms() { reached_time_limit_flag_ms = true; return *this; }
GraphStats& GraphStats::set_reached_time_limit_pmc(const PMCAlg& a) { reached_time_limit_flag_pmc[a] = true; return *this; }
GraphStats& GraphStats::set_reached_time_limit_pmc(const set<PMCAlg>& va) {
    for (PMCAlg a: va) {
        set_reached_time_limit_pmc(a);
    }
    return *this;
}
GraphStats& GraphStats::set_reached_time_limit_trng() { reached_time_limit_flag_trng = true; return *this; }
GraphStats& GraphStats::set_reached_count_limit_ms() { reached_count_limit_flag_ms = true; return *this; }
GraphStats& GraphStats::set_reached_count_limit_trng() { reached_count_limit_flag_trng = true; return *this; }
GraphStats& GraphStats::unset_reached_time_limit_ms() { reached_time_limit_flag_ms = false; return *this; }
GraphStats& GraphStats::unset_reached_time_limit_pmc(const PMCAlg& a) { reached_time_limit_flag_pmc[a] = false; return *this; }
GraphStats& GraphStats::unset_reached_time_limit_trng() { reached_time_limit_flag_trng = false; return *this; }
GraphStats& GraphStats::unset_reached_count_limit_ms() { reached_count_limit_flag_ms = false; return *this; }
GraphStats& GraphStats::unset_reached_count_limit_trng() { reached_count_limit_flag_trng = false; return *this; }
GraphStats& GraphStats::set_mem_error_ms() { mem_error_flag_ms = true; return *this; }
GraphStats& GraphStats::set_mem_error_pmc(const PMCAlg& a) { mem_error_flag_pmc[a] = true; return *this; }
GraphStats& GraphStats::set_mem_error_trng() { mem_error_flag_trng = true; return *this; }
bool GraphStats::reached_time_limit_ms() const { return reached_time_limit_flag_ms; }
bool GraphStats::reached_time_limit_pmc(const PMCAlg& a) const { return reached_time_limit_flag_pmc.at(a); }
bool GraphStats::reached_time_limit_trng() const { return reached_time_limit_flag_trng; }
bool GraphStats::reached_count_limit_ms() const { return reached_count_limit_flag_ms; }
bool GraphStats::reached_count_limit_trng() const { return reached_count_limit_flag_trng; }
bool GraphStats::mem_error_ms() const { return mem_error_flag_ms; }
bool GraphStats::mem_error_pmc(const PMCAlg& a) const { return mem_error_flag_pmc.at(a); }
bool GraphStats::mem_error_trng() const { return mem_error_flag_trng; }

bool GraphStats::ms_no_errors() const { return !reached_time_limit_ms() && !reached_count_limit_ms() && !mem_error_ms(); }
bool GraphStats::pmc_no_errors(const PMCAlg& a) const { return !reached_time_limit_pmc(a) && !mem_error_pmc(a); }
bool GraphStats::trng_no_errors() const { return !reached_time_limit_trng() && !reached_count_limit_trng() && !mem_error_trng(); }

// Flag updating setters
GraphStats& GraphStats::set_ms_count(long c) {
    if (!has_nodes()) {
        TRACE(TRACE_LVL__ERROR, "No nodes in the underlying graph, cannot set MS count");
        return *this;
    }
    count_ms.back() = c;
    return *this;
}
GraphStats& GraphStats::set_ms_subgraph_count(const vector<long>& vc) {
    if (vc.size() != get_n()) {
        TRACE(TRACE_LVL__ERROR, "The input MS count vector is of size " << vc.size() << ", whereas the"
              " graph has " << get_n() << " nodes");
        return *this;
    }
    count_ms = vc;
    return *this;
}
GraphStats& GraphStats::set_pmc_count(long c) { count_pmc = c; return *this; }
GraphStats& GraphStats::set_trng_count(long c) { count_trng = c; return *this; }
GraphStats& GraphStats::set_ms(const NodeSetSet& seps) {
    if (!has_nodes()) {
        TRACE(TRACE_LVL__ERROR, "Can't set minimal separators of graph with no nodes");
        return *this;
    }
    ms.back() = seps;
    update_ms_count();
    return *this;
}
GraphStats& GraphStats::set_ms_subgraphs(const vector<NodeSetSet>& vnss) {
    if (vnss.size() != get_n()) {
        TRACE(TRACE_LVL__ERROR, "The input MS vector is of size " << vnss.size() << ", whereas the"
              " graph has " << get_n() << " nodes");
        return *this;
    }
    ms = vnss;
    update_ms_subgraph_count();
    return *this;
}
GraphStats& GraphStats::set_pmc(const NodeSetSet& p) {
    pmc = p;
    update_pmc_count();
    return *this;
}
GraphStats& GraphStats::set_trng(const vector<ChordalGraph>& vt) {
    trng.clear();
    for (ChordalGraph t: vt) {
        add_trng(t);
    }
    update_trng_count();
    return *this;
}
GraphStats& GraphStats::set_instance(int inst) {
    if (inst <= 0) {
        TRACE(TRACE_LVL__ERROR, "Instance number should be at least 1. "
             << "Leaving the instance number as '" << get_instance() << "'");
        return *this;
    }
    g.declareRandom(get_p(),inst);
    return *this;
}
GraphStats& GraphStats::set_ms_subgraphs(unsigned index, const NodeSetSet& nss) {
    ASSERT_IN_INDEX_RANGE_OR_OP(index, return *this);
    ms[index] = nss;
    count_ms[index] = nss.size();   // Don't update flags!
    return *this;
}
GraphStats& GraphStats::add_sep(const NodeSet& ns) {
    if (!has_nodes()) {
        TRACE(TRACE_LVL__ERROR, "Can't add minimal separators to a graph with no nodes");
        return *this;
    }
    ms.back().insert(ns);
    count_ms.back() = ms.back().size();
    return *this;
}
GraphStats& GraphStats::increment_ms_count() {
    if (!has_nodes()) {
        TRACE(TRACE_LVL__ERROR, "Can't increment number of minimal separators of graph with no nodes");
        return *this;
    }
    ++(count_ms.back());
    TRACE(TRACE_LVL__TEST, "MS count set to " << count_ms.back());
    return *this;
}
GraphStats& GraphStats::add_trng(const ChordalGraph& t) {
    trng.push_back(t);
    update_trng_count();
    return *this;
}
GraphStats& GraphStats::increment_trng_count() { ++count_trng; return *this; }
GraphStats& GraphStats::set_ms_calc_time(time_t t) { calc_time_ms = t; return *this; }
GraphStats& GraphStats::set_pmc_calc_time(const PMCAlg& a, time_t t) { calc_time_by_alg_pmc[a] = t; return *this; }
GraphStats& GraphStats::set_pmc_calc_time(const set<PMCAlg>& sa, time_t t) { for (PMCAlg a: sa) set_pmc_calc_time(a,t); return *this; }
GraphStats& GraphStats::set_pmc_calc_time(const vector<PMCAlg>& va, time_t t) { for (PMCAlg a: va) set_pmc_calc_time(a,t); return *this; }
GraphStats& GraphStats::set_trng_calc_time(time_t t) { calc_time_trng = t; return *this; }
GraphStats& GraphStats::set_random() { g.declareRandom(get_p(), get_instance()); return *this; }
GraphStats& GraphStats::unset_random() { g.derandomize(); return *this; }
GraphStats& GraphStats::set_p(double prob) { g.declareRandom(prob, get_instance()); return *this; }

// Getters
bool GraphStats::has_nodes() const { return get_n() > 0; }
Graph GraphStats::get_graph() const { return g;}
unsigned GraphStats::get_n() const { return g.getNumberOfNodes();}
int GraphStats::get_m() const { return g.getNumberOfEdges();}
double GraphStats::get_p() const { return g.getP(); }
double GraphStats::get_ratio() const { return get_graph().getEdgeRatio(); }
int GraphStats::get_instance() const { return g.getInstance(); }
long GraphStats::get_ms_count() const { return (!has_nodes() ? 0 : count_ms.back()); }
long GraphStats::get_pmc_count() const { return count_pmc; }
long GraphStats::get_trng_count() const { return count_trng; }
vector<long> GraphStats::get_ms_subgraph_count() const { return count_ms; }
time_t GraphStats::get_ms_calc_time() const { return calc_time_ms;}
time_t GraphStats::get_trng_calc_time() const { return calc_time_trng; }
time_t GraphStats::get_pmc_calc_time(const PMCAlg& a) const {
    if (calc_time_by_alg_pmc.find(a) == calc_time_by_alg_pmc.end()) {
        TRACE(TRACE_LVL__ERROR, "PMC not calculated using the algorithm '" << a << "'");
        return -1;
    }
    return calc_time_by_alg_pmc.at(a);
}
bool GraphStats::is_random() const { return g.isRandom(); }
string GraphStats::get_text() const { return text; }
NodeSetSet GraphStats::get_ms() const { return get_n() == 0 ? NodeSetSet() : ms.back(); }
NodeSetSet GraphStats::get_subgraph_ms(unsigned index) const {
    ASSERT_IN_INDEX_RANGE_OR_OP(index, return NodeSetSet());
    return ms[index];
}
long GraphStats::get_ms_subgraph_count(unsigned index) const {
    ASSERT_IN_INDEX_RANGE_OR_OP(index, return -1);
    return count_ms[index];
}
vector<NodeSetSet> GraphStats::get_ms_subgraphs() const { return ms; }
NodeSetSet GraphStats::get_pmc() const { return pmc; }
vector<ChordalGraph> GraphStats::get_trng() const { return trng; }

ostream& operator<<(ostream& os, const GraphStats& gs) {
    os << gs.str();
    os << "Text: " << gs.text << endl;
    if (gs.is_random()) {
        os << "Graph is  random, p=" << gs.get_p() << ", instance " << gs.get_instance() << "." << endl;
    }
    else {
        os << "Graph is non-random." << endl;
    }
    return os;
}

bool GraphStats::dump(const string& path, bool force) {

    // First, check to see if we should dump
    if (!force && integrity(path)) {
        TRACE(TRACE_LVL__TEST, "Graph '" << path << "' already exists and is consistent, aborting dump");
        text = path;
        return true;
    }

    // Try to dump.
    // If successful, set from_file and update the filename if need be.
    // Otherwise, check if this instance is originally from file and if the file exists:
    // if so, keep from_file set and don't update the filename. Otherwise, unset from_file.
    string updated_path = add_ext_if_missing(path);
    if (utils__dump_string_to_file(this->str(), updated_path, false)) {
        TRACE(TRACE_LVL__NOISE, "Dumped graph to file '" << updated_path << "'");
        if (!integrity(updated_path)) {
            TRACE(TRACE_LVL__ERROR, "Uh oh.. dumped graph to '" << updated_path << "', " <<
                                    "but resulting file isn't consistent!" << endl <<
                                    "Original object:" << endl << *this << endl <<
                                    "Dumped object:" << endl << GraphStats::read(updated_path));
            return false;
        }
        //from_file = true;
        TRACE(TRACE_LVL__TEST, "dump(" << path << ") is SUCCESSFUL, returning..");
        text = updated_path;
        return true;
    }
    else {
        //from_file = (from_file && utils__file_exists(get_filepath(false)));
        TRACE(TRACE_LVL__ERROR, "Couldn't dump to new file '" << updated_path << "'");
        return false;
    }
}

#define GRAPHSTATS_READ_TRY(_op) do { \
        try { \
            _op; \
        } \
        catch (const invalid_argument& ia) { \
            TRACE(TRACE_LVL__ERROR, "Couldn't read metadata ('" << #_op << "' failed), " << \
                                    "invalid argument:" << endl << ia.what()); \
            return GraphStats(); \
        } \
    } while(0)

GraphStats GraphStats::read(const string& path, bool suppress_errors) {

    TRACE(TRACE_LVL__TEST, "In with path=" << path);
    string path_cpy = path;
    string filename = utils__get_filename(path_cpy);    // Graph text for graph files is always the filename
    string dirname = utils__get_dirname(path_cpy);
    dirname = utils__str_empty(dirname) ? string(".") : dirname;
    Graph g = Graph();

    // Sanity.
    // If no input filename given, assume NOT from file
    if (utils__str_empty(filename)) {
        if (!suppress_errors) TRACE(TRACE_LVL__ERROR, "No filename given! Returning an empty GS instance");
        return GraphStats();
    }
    // If the file doesn't exist, try adding the extension before giving up
    else if (!utils__file_exists(path_cpy)) {
        // Try to add the extension
        if (!utils__file_exists(add_ext_if_missing(path_cpy))) {
            if (!suppress_errors) TRACE(TRACE_LVL__ERROR, "No such file '" << path_cpy << "' " <<
                                    "or '" << add_ext_if_missing(path_cpy) << "', " <<
                                    "returning default graph");
            return GraphStats();
        }
        else {
            filename = add_ext_if_missing(filename);
            path_cpy = utils__merge_dir_basename(dirname, filename);
        }
    }
    // If we're here, the file indicated exists on disk.
    // Check the extension and continue
    if (utils__get_extension(filename) != GRAPHSTATS_FILE_EXTENSION) {
        // Not a GraphStats output file!
        g = GraphReader::read(path_cpy);
        if (GraphReader::last_read_failed()) {
            if (!suppress_errors) TRACE(TRACE_LVL__ERROR, "Couldn't read '" << path_cpy << "'! " <<
                                        "Returning empty GS instance");
            return GraphStats();
        }
    }
    else {
        TRACE(TRACE_LVL__TEST, "Reading CSV..");
        vector<vector<string> > cells = utils__read_csv(path_cpy);
        TRACE(TRACE_LVL__TEST, "Done. Got the following CSV table:" << endl << cells);
        // Sanity
        if (cells.size() < GRAPHSTATS_METADATA_ROWS) {
            if (!suppress_errors) TRACE(TRACE_LVL__ERROR, "Empty file '" << path_cpy << "'! " <<
                                        "Should at least contain metadata... returning an empty object");
            return GraphStats();
        }
        if (cells[0].size() < GRAPHSTATS_MI_TOTAL_CELLS) {
            if (!suppress_errors) TRACE(TRACE_LVL__ERROR, "Partial / nonexistent metadata in file '" <<
                                        path_cpy << "', returning empty object");
            return GraphStats();
        }
        TRACE(TRACE_LVL__TEST, "Done sanity checking");

        // Nodes and randomness
        GRAPHSTATS_READ_TRY(g = Graph(stoi(cells[0][GRAPHSTATS_MI_N])));
        TRACE(TRACE_LVL__TEST, "Constructed graph '" << g << "'");
        if (!utils__str_eq(cells[0][GRAPHSTATS_MI_P_OR_NONRANDOM], GRAPHSTATS_METADATA_TXT_NOT_RANDOM)) {
            GRAPHSTATS_READ_TRY(g.declareRandom(
                        stod(cells[0][GRAPHSTATS_MI_P_OR_NONRANDOM]),
                        stoi(cells[0][GRAPHSTATS_MI_INSTANCE])));
        }
        TRACE(TRACE_LVL__TEST, "Checked random (" << (g.isRandom() ? "true" : "false") << ")");

        // Edges.
        // Start immediately after the metadata row(s)
        for (unsigned row_i=GRAPHSTATS_METADATA_ROWS; row_i<cells.size(); ++row_i) {
            TRACE(TRACE_LVL__TEST, "In iteration with row " << (row_i-1) << "/" << (cells.size()-2));
            if (cells[row_i].size() != 2) {
                if (!suppress_errors) TRACE(TRACE_LVL__WARNING, "Reach a row with " << cells[row_i].size() << " columns,"
                                        << " expected two columns");
                continue;
            }
            GRAPHSTATS_READ_TRY(g.addEdge(stoi(cells[row_i][0]),stoi(cells[row_i][1])));
        }
    }
    TRACE(TRACE_LVL__TEST, "Done with logic block, creating GraphStats object and returning." << endl
                        << "Constructing with g=" << g << ", dirname=" << dirname << " and filename=" << filename);
    // Create the GS object.
    // The from_file flag is important: it sets the flag in the GS instance
    // and keeps the original file extension
    string full_path = dirname+"/"+filename;
    GraphStats gs = GraphStats(g).set_text(full_path);
    TRACE(TRACE_LVL__TEST, "Constructed GS object:" << endl << gs);
    return gs;
}
bool GraphStats::integrity_aux(const string& path) const {
    TRACE(TRACE_LVL__TEST, "In with path=" << path);
    if (!utils__file_exists(path)) {
        TRACE(TRACE_LVL__TEST, "integrity() failed: no such file '" << path << "'");
        return false;
    }
    GraphStats gs = GraphStats::read(path ,true);
    if (!gs.text_valid()) {
        TRACE(TRACE_LVL__TEST, "integrity() failed: read object is invalid");
        return false;
    }
    // Graph text is allowed to be different in this case
    if (!equal_except_text(gs)) {
        TRACE(TRACE_LVL__TEST, "integrity() failed: objects aren't equal." << endl
                        << "Original GS: " << *this << endl
                        << "Read GS: " << gs);
        return false;
    }
    // OK! We have integrity!
    return true;
}
string GraphStats::integrity_aux_path(const string& path) const {
    string path_cpy = path;
    if (utils__str_empty(path_cpy)) {
        return text_valid() ?
            get_text() :
            utils__get_dirname(path_cpy)+"/"+GraphStats::get_default_filename_from_graph(get_graph());
    }
    // Try adding a missing extension, or using the default filename
    else if (utils__str_empty(utils__get_filename(path_cpy))) {
        path_cpy = utils__get_dirname(path_cpy)+"/"+GraphStats::get_default_filename_from_graph(get_graph());
    }
    else if (!utils__file_exists(path_cpy)) {
        path_cpy = add_ext_if_missing(path_cpy);
    }
    return path_cpy;
}
bool GraphStats::integrity(const string& path) const { return integrity_aux(integrity_aux_path(path)); }
bool GraphStats::integrity(const string& path) {
    // The non-const version also updates text if a file is found
    TRACE(TRACE_LVL__TEST, "In");
    const GraphStats& gs = *this;
    string new_path = integrity_aux_path(path);
    bool result = gs.integrity_aux(new_path); // Call const version
    if (result && !text_valid()) {
        set_text(new_path);
    }
    return result;
}


}

