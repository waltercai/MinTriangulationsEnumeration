#ifndef GRAPHSTATS_H_INCLUDED
#define GRAPHSTATS_H_INCLUDED

#include "ChordalGraph.h"
#include "DataStructures.h"
#include "Graph.h"
#include "PMCAlg.h"
#include "StatisticRequest.h"
#include <map>
#include <string>

using std::string;
using std::map;

namespace tdenum {
/**
 * This class represents statistical data for graphs (a kind of wrapper class).
 *
 * This class is used for graph IO: storing and reading graphs and statistical graph data
 * from file.
 *
 * To read an existing GS instance use the static read() method. To dump an instance to disk,
 * use dump().
 * Note that if the GRAPHSTATS_FILE_EXTENSION is missing from the input to read() or dump(), it
 * will NOT appended to the input path!
 *
 * The 'text' field is used by the Dataset class and is assumed to be the path to the file
 * the GS instance represents:
 * - If the instance was successfully constructed via read(), the text field will be the filename
 *   given.
 * - If read() failed, text==GRAPHSTATS_INVALID_TEXT_FIELD and the instance will be constructed using
 *   a default Graph() and no extra data.
 * - If dump() is called, the text field will be updated to contain the actual filename used for
 *   writing (dump() may add GRAPHSTATS_FILE_EXTENSION).
 */

#define GRAPHSTATS_METADATA_TXT_NOT_RANDOM ("NOT_RANDOM") /* No commas! */

/**
 * Text set as the GS text when the source (graph) file is unknown / doesn't exist
 */
#define GRAPHSTATS_NO_FILE_KNOWN (string("NO GRAPH FILE KNOWN"))

/**
 * These strings are used for random graph (or any graph) file names.
 * Make sure none of the tokens exist as characters in the rest of the
 * filename!
 */
#define GRAPHSTATS_DEFAULT_FILENAME_TOKEN_N "N"
#define GRAPHSTATS_DEFAULT_FILENAME_TOKEN_P "P"
#define GRAPHSTATS_DEFAULT_FILENAME_TOKEN_INST "I"
#define GRAPHSTATS_FILE_EXTENSION "grp"
#define GRAPHSTATS_DEFAULT_FILENAME_EXPR (\
        string("Default_graph_name_") + \
        string(GRAPHSTATS_DEFAULT_FILENAME_TOKEN_N) + \
        string("_") + \
        string(GRAPHSTATS_DEFAULT_FILENAME_TOKEN_P) + \
        string("_inst") + \
        string(GRAPHSTATS_DEFAULT_FILENAME_TOKEN_INST) + \
        string(".") + \
        string(GRAPHSTATS_FILE_EXTENSION) \
    )

/**
 * Not all metrics are required.
 * The Dataset object uses these flags, the GraphStats
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
#define X(_name,_) void GRAPHSTATS_SET_##_name(int& _flds); \
                   void GRAPHSTATS_UNSET_##_name(int& _flds);
GRAPHSTATS_FIELD_TABLE
#undef X

/**
 * This class represents graph statistical data.
 * Minimal separators, PMCs, triangulations, time metrics.
 */
class GraphStats {
private:

    friend class GraphStatsTester;
    friend class GraphProducerTester;

    // Basic graph data.
    // Minimal separators may be stored for all subgraphs; ms_count.back()
    // is the real number of MSs in the graph
    Graph g;
    string text;
    vector<long> count_ms;
    long count_pmc;
    long count_trng;

    // If the limit was overreached, set the relevant flag.
    bool reached_count_limit_flag_ms;
    bool reached_count_limit_flag_trng;
    bool reached_time_limit_flag_ms;
    map<PMCAlg,bool> reached_time_limit_flag_pmc;
    bool reached_time_limit_flag_trng;
    bool mem_error_flag_ms;
    map<PMCAlg,bool> mem_error_flag_pmc;
    bool mem_error_flag_trng;

    // The amount of time required for calculation the minimal separators.
    // Note: PMC calculation time may disregard the time required to calculate
    // the minimal separators of g (see actual_pmc_calc_time())
    time_t calc_time_ms;
    map<PMCAlg,time_t> calc_time_by_alg_pmc;
    time_t calc_time_trng;

    // Data.
    // Minimal separators of all subgraphs may also be stored.
    vector<NodeSetSet> ms;          // Minimal separators
    NodeSetSet pmc;                 // PMCs
    vector<ChordalGraph> trng;      // Triangulations

    // Refresh calculated data
    GraphStats& update_ms_count();
    GraphStats& update_ms_subgraph_count();
    GraphStats& update_pmc_count();
    GraphStats& update_trng_count();

    // Add the GraphStats extension to the given filename, if missing
    static string add_ext_if_missing(const string&);

    // Make this private.
    // The field is supposed to be the most recently updated file path
    GraphStats& set_text(const string&);

    // Stringify instance
    string str() const;

    // Helper methods for integrity().
    // The aux_path() method decides the actual path to the filename to be tested for
    // integrity: constructs default / adds extension, etc.
    // the aux() method does the actual work. Called from both const and non-const versions
    // of integrity()
    string integrity_aux_path(const string& path) const;
    bool integrity_aux(const string& path) const;

public:

    /**
     * Basic constructors.
     *
     * We need a default constructor for containers.
     *
     * Note that graph text should be a unique identifier for ease of
     * use with the Dataset class - when random graphs are given, use the
     * 'instance' field to differentiate between different samples of the
     * same G(n,p) space.
     *
     * To update the text field, dump the data to a unique filename. The filename
     * will become the identifying text of the graph.
     */
    GraphStats(const Graph& g = Graph());

    // Compare all fields. except the graph text
    bool equal_except_text(const GraphStats&) const;

    // Compare all fields
    bool operator==(const GraphStats&) const;
    bool operator!=(const GraphStats&) const;

    // Checks if the object is valid (coupled to file) by testing the text field
    bool text_valid() const;

    // Default output filename for given graphs
    static string get_default_filename_from_graph(const Graph&);

    // With no input, get_dump_filepath() returns get_text() if the text
    // is valid, or ./<default_graph_filename> if it's invalid.
    // With an input string prepend_dir, if the GS object is valid
    // the filename will be taken from the get_text() value (not the leading
    // path, mind you!) and the input string will be prepended as a directory.
    //
    // Note: get_dump_filepath("") == get_dump_filepath()
    string get_dump_filepath() const;
    string get_dump_filepath(const string& prepend_dir) const;

    // Set / unset limit / error flags
    GraphStats& set_reached_time_limit_ms();
    GraphStats& set_reached_time_limit_pmc(const PMCAlg&);
    GraphStats& set_reached_time_limit_pmc(const set<PMCAlg>&);
    GraphStats& set_reached_time_limit_trng();
    GraphStats& set_reached_count_limit_ms();
    GraphStats& set_reached_count_limit_trng();
    GraphStats& unset_reached_time_limit_ms();
    GraphStats& unset_reached_time_limit_pmc(const PMCAlg&);
    GraphStats& unset_reached_time_limit_trng();
    GraphStats& unset_reached_count_limit_ms();
    GraphStats& unset_reached_count_limit_trng();
    GraphStats& set_mem_error_ms();
    GraphStats& set_mem_error_pmc(const PMCAlg&);
    GraphStats& set_mem_error_trng();
    bool reached_time_limit() const;
    bool reached_time_limit_ms() const;
    bool reached_time_limit_pmc(const PMCAlg&) const;
    bool reached_time_limit_trng() const;
    bool reached_count_limit() const;
    bool reached_count_limit_ms() const;
    bool reached_count_limit_trng() const;
    bool mem_error_ms() const;
    bool mem_error_pmc(const PMCAlg&) const;
    bool mem_error_trng() const;

    // Compound tests. Check if there are any error flags on
    bool ms_no_errors() const;
    bool pmc_no_errors(const PMCAlg&) const;
    bool trng_no_errors() const;

    // Setters.
    GraphStats& set_ms_count(long);
    GraphStats& set_ms_subgraph_count(const vector<long>&);
    GraphStats& set_pmc_count(long);
    GraphStats& set_trng_count(long);
    GraphStats& set_ms(const NodeSetSet&);
    GraphStats& set_ms_subgraphs(const vector<NodeSetSet>&);
    GraphStats& set_pmc(const NodeSetSet&);
    GraphStats& set_trng(const vector<ChordalGraph>&);
    GraphStats& set_instance(int inst);
    GraphStats& set_ms_subgraphs(unsigned index, const NodeSetSet&);
    GraphStats& add_sep(const NodeSet&);   // Add a minimal separator (main graph)
    GraphStats& increment_ms_count();      // Adds 1 to the ms count
    GraphStats& add_trng(const ChordalGraph&);
    GraphStats& increment_trng_count();    // Adds 1 to the trng count
    GraphStats& set_ms_calc_time(time_t);
    GraphStats& set_pmc_calc_time(const PMCAlg&,time_t);
    GraphStats& set_pmc_calc_time(const set<PMCAlg>&,time_t);   // Same time all algs
    GraphStats& set_pmc_calc_time(const vector<PMCAlg>&,time_t);   // Same time all algs
    GraphStats& set_trng_calc_time(time_t);
    GraphStats& set_random();
    GraphStats& unset_random();
    GraphStats& set_p(double);

    // If the value calculated is invalid because of timeout / count limit,
    // return the value calculated (may be invalid!)
    bool has_nodes() const;
    Graph get_graph() const;
    unsigned get_n() const;
    int get_m() const;
    int get_instance() const;
    double get_p() const;
    double get_ratio() const;   // Returns edge ratio
    bool is_random() const;
    string get_text() const;
    long get_ms_count() const;
    vector<long> get_ms_subgraph_count() const;
    long get_pmc_count() const;
    long get_trng_count() const;
    time_t get_ms_calc_time() const;
    time_t get_pmc_calc_time(const PMCAlg&) const;
    time_t get_trng_calc_time() const;
    NodeSetSet get_ms() const;
    NodeSetSet get_subgraph_ms(unsigned index) const;
    long get_ms_subgraph_count(unsigned index) const;
    vector<NodeSetSet> get_ms_subgraphs() const;
    NodeSetSet get_pmc() const;
    vector<ChordalGraph> get_trng() const;

    // Print to stream
    friend ostream& operator<<(ostream&, const GraphStats&);

    // IO
    // The first two rows are meta data -
    // . The first row contains a single cell with the total number of nodes.
    // . The second row either contains the value of P for random graphs, or GRAPHSTATS_TXT_NOT_RANDOM
    //   to indicate the graph is non-random.
    // On each of the remaining rows are two cells, representing an edge (each cell contains a node).

    // dump() returns true <==> a file exists and - when read - is equal to the GS instance
    // dumped.
    bool dump(const string& path, bool force=false/*, bool skip_if_grp_type_exists=true*/);

    // Read instance from file.
    // If an unknown extension is encountered, use the GraphReader.
    // The suppress_errors is here to cater to the noise generated by calls to integrity()
    static GraphStats read(const string& path, bool suppress_errors = false);

    // Returns true <==> the path is to a file containing the same GS object as *this.
    // Uses read().
    // If no filename is given (result of utils__get_filename() is empty) and the GS
    // instance is invalid (not coupled to file), the default filename is assumed.
    // If integrity() returns true for some given input and text_valid() is false, then
    // the internal text will be updated to the given file (hence, this is a non-const
    // method).
    // The const version won't update an invalid text field, even if it should.
    bool integrity(const string& path = "") const;
    bool integrity(const string& path = "");

};

}

#endif // GRAPHSTATS_H_INCLUDED
