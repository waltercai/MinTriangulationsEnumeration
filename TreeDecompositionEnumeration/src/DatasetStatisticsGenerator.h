#ifndef DATASETSTATISTICSGENERATOR_H_INCLUDED
#define DATASETSTATISTICSGENERATOR_H_INCLUDED

#include "Graph.h"
#include "GraphReader.h"
#include "GraphStats.h"
#include "MinimalSeparatorsEnumerator.h"
#include "PMCEnumerator.h"
#include "PMCAlg.h"
#include "DirectoryIterator.h"
#include <map>
#include <string>
#include <vector>
#include <omp.h>
using std::vector;
using std::map;

namespace tdenum {

/**
 * A class responsible for taking input datasets (utilizing
 * the GraphReader class) and generating statistics:
 * - Number of nodes
 * - Number of edges
 * - Number of minimal separators
 * - Number of PMCs
 * - Number of minimal triangulations
 * - Number of minimal separators of each subgraph of G
 *
 * By default, the results are output to CSV file (optionally, print
 * formatted statistics to console with the 'verbose' flag). If the DSG
 * is only being use to generate statistics for use in user code, output
 * to file may be suppressed by calling suppress_dump().
 *
 * To use:
 * 1. Construct the class with the name of the desired output file and
 *    desired statistics to be calculated. Defaults to DSG_COMP_ALL. The
 *    'fields' value is a bitmap, so by calling:
 *    > DatasetStatisticsGenerator("out.csv", DSG_COMP_ALL ^ DSG_COMP_TRNG)
 *    the output statistics will be everything except triangulations.
 * 2. Add graphs. Several utility functions exist for adding graphs in
 *    singles / batch with relative ease (recursive file search with filters,
 *    batch random graph insertion..).
 * 3. (OPTIONAL) Specify which PMC algorithm should be used (if calculating
 *    PMCs).
 * 4. If no file output is desired, call suppress_dump(). If the computation
 *    must be sequential (statistics for performance are needed, for example),
 *    call suppress_async().
 * 5. Call compute().
 *
 * If the output filename isn't given explicitly, make sure the directory
 * defined by DEFAULT_OUTPUT_DIR exists and can be reached relatively from
 * the point of code execution.
 *
 * To print results to the console, call print(), and to get the
 * statistics vector call get_stats().
 */

#define DEFAULT_OUTPUT_DIR "./"


/**
 * Text to be displayed in the CSV file header.
 * Map names to numeric identifiers for ease of use.
 */
#define DSG_ERROR_TEXT_MS "MS"
#define DSG_ERROR_TEXT_PMC "PMC"
#define DSG_ERROR_TEXT_TRNG "TRNG"

#define DSG_COL_TABLE \
    X(TXT, "Graph text") \
    X(NODES, "Nodes ") \
    X(EDGES, "Edges ") \
    X(MSS, "Minimal separators") \
    X(PMCS, "PMCs    ") \
    X(TRNG, "Minimal triangulations") \
    X(P, "P value") \
    X(RATIO, "Edge ratio") \
    X(MS_TIME, "MS calculation time") \
    X(ERR_TIME, "Time errors") \
    X(ERR_CNT, "Count errors")

#define X(ID,_) DSG_COL_##ID,
typedef enum _dsg_columns {
    DSG_COL_TABLE
    DSG_COL_LAST
} dsg_columns;
#undef X

#define X(ID,str) const string DSG_COL_STR_##ID = str;
DSG_COL_TABLE
#undef X

#define X(ID,str) {str, DSG_COL_##ID},
const map<string, dsg_columns> DSG_COL_STR_TO_INT_MAP {
    DSG_COL_TABLE
};
#undef X

#define X(ID,str) {DSG_COL_##ID, str},
const map<dsg_columns, string> DSG_COL_INT_TO_STR_MAP {
    DSG_COL_TABLE
};
#undef X

/*
#define DSG_COL_STR_TXT "Graph text"
#define DSG_COL_STR_NODES "Nodes "
#define DSG_COL_STR_EDGES "Edges "
#define DSG_COL_STR_MSS "Minimal separators"
#define DSG_COL_STR_PMCS "PMCs    "
#define DSG_COL_STR_TRNG "Minimal triangulations"
#define DSG_COL_STR_P "P value"
#define DSG_COL_STR_RATIO "Edge ratio"
#define DSG_COL_STR_MS_TIME "MS calculation time"
#define DSG_COL_STR_ERR_TIME "Time errors"
#define DSG_COL_STR_CNT_TIME "Count errors"
*/


class DatasetStatisticsGenerator {
private:

    // Single output file, where the data will be dumped.
    string outfilename;

    // The fields to be calculated (ORed flags).
    int fields;

    // If at least one of the added graphs is randomized, set this
    // to true so we know to output an extra column for p.
    bool has_random;

    // If this is set to true, thread processes will output progress
    // during compute().
    // If set to false, this may greatly improve performance: the lock
    // used for syncing threads is only relevant when they need to
    // output progress.
    bool verbose;

    // If allow_dump_flag is set to true, file output will be created.
    bool allow_dump_flag;

    // If allow_parallel is set to true, parallelize the job.
    // dsg_parallel computes different graphs asynchronously while
    // pmc_parallel utilizes the parallel version of the PMC algorithm.
    bool allow_dsg_parallel;
    bool allow_pmc_parallel;

    // If true, every graph added will cause a line to be printed to
    // console (with graph text).
    bool show_graphs;

    // Time / count limits (if exceeded, processing is stopped and moves
    // on to the next graph).
    bool has_ms_time_limit;
    bool has_pmc_time_limit;
    bool has_trng_time_limit;
    bool has_ms_count_limit;
    bool has_trng_count_limit;
    time_t ms_time_limit;
    time_t pmc_time_limit;
    time_t trng_time_limit;
    long ms_count_limit;
    long trng_count_limit;

    // The algorithm to be used when calculating PMCs
    PMCAlg pmc_alg;

    // For every i, the following vectors store the data of graph i.
    // Different threads access these at different indexes, so there
    // should be no need to lock any of them.
    vector<GraphStats> gs;

    // While computing a graph's stats, push it's index here so
    // print_progress will output the current computation status.
    // Also, add a flag indicating whether or not progress is
    // currently printed.
    vector<unsigned int> graphs_in_progress;
    unsigned int graphs_computed;

    // Used for printing to console (width of first column).
    unsigned int max_text_len;

    // When verbose computation output is requested, it's better
    // to lock a lock before printing, otherwise we'll get garbage.
    omp_lock_t lock;

    // Useful for printing
    bool has_time_limit() const;
    bool has_count_limit() const;

    // Return a header string
    string header(bool csv) const;

    // Stringify all data / single result, in CSV format or printable.
    string str(unsigned int i, bool csv) const;
    string str(bool csv) const;

    // Dump result line to file, or open a new file and output a header.
    // Can't be const, we need to lock the lock here
    void dump_line(unsigned int i);
    void dump_header();
    void dump_parallel_aux(const string& s, bool append = true);

    // If verbose computation is enabled, use this to print progress
    // to the screen.
    void print_progress();
    void add_graph_to_progress(unsigned int i);

    // Used to compute a single graph.
    // Optionally print verbose output.
    void compute(unsigned int i);

public:

    // Creates a new instance of the generator.
    // By ORing different flags the user may decide which fields to compute.
    // Optionally send a directory iterator to input graphs from.
    // If no file name is sent, no output is dumped.
    DatasetStatisticsGenerator(const string& outputfile, int flds = GRAPHSTATS_ALL);
    DatasetStatisticsGenerator(const string& outputfile,
                               DirectoryIterator di,
                               int flds = GRAPHSTATS_ALL);
    DatasetStatisticsGenerator(int flds = GRAPHSTATS_ALL);
    ~DatasetStatisticsGenerator();

    // If set to true, whenever a graph is added - print the text.
    void show_added_graphs();
    void dont_show_added_graphs();

    // Resets the DSG (doesn't change output filename, allows change
    // of fields to be calculated).
    void reset(int flds = GRAPHSTATS_ALL);

    // Sets time/count limits or disables them
    void disable_all_limits();
    void disable_all_count_limits();
    void disable_all_time_limits();
    void set_ms_time_limit(time_t);
    void set_trng_time_limit(time_t);
    void set_pmc_time_limit(time_t);
    void set_ms_count_limit(unsigned long);
    void set_trng_count_limit(unsigned long);

    // Sets the 'valid' flag of all graphs to 'false' (to force
    // recalculation).
    void force_recalc();

    // Changes filename (doesn't call reset()), or prevents output to file
    // altogether.
    void change_outfile(const string&);
    void suppress_dump();
    void allow_dump(const string& filename);

    // Enables / disables parallel computation of graphs.
    // Note: only one active parallelization level available at
    // this time (allow_pmc_parallel() calls suppress_dsg_parallel())
    void suppress_dsg_parallel();
    void enable_dsg_parallel();
    void suppress_pmc_parallel();
    void enable_pmc_parallel();

    // Choose the PMC algorithm
    void set_pmc_alg(const PMCAlg&);

    // If the minimal separators were already calculated, the user may
    // set them.
    // This will not be validated internally!
    // The index is 1-based (as opposed to zero based) because the numbers
    // printed to screen are 1-indexed.
    void set_ms(const NodeSetSet& ms, time_t calc_time, unsigned index);
    NodeSetSet get_ms(unsigned index) const;

    // Add graphs.
    // The user may either send an input filename to read the data
    // from, or a graph instance.
    // In each case, text to be printed alongside the graph's computed
    // data may be sent as input (to identify the graph). This is a
    // required parameter if a Graph is sent, otherwise the default is
    // the filename.
    void add_graph(const Graph& g, const string& text);
    void add_graph(const string& filename, const string& text = "");

    // Recursive search.
    // Allow user to send a directory iterator.
    // The second method scans the directory given by the user (recursively)
    // and adds all graphs found. Optionally add filters to the path strings
    // (graphs with at least one filter as a substring of the path won't be
    // added).
    void add_graphs(DirectoryIterator di);
    void add_graphs_dir(const string& dir,
                        const vector<string>& filters = vector<string>());


    // Adds random graphs to the DSG.
    // The inputs are:
    // - A vector of graphs sizes (number of nodes)
    // - A vector of p values
    // If mix_match is false, for every i a graph will be sampled from
    // G(n[i],p[i]).
    // If mix_match is true, for every i and j a graph will be sampled
    // from G(n[i],p[j])
    void add_random_graph(int n, double p, int instances = 1);
    void add_random_graphs(const vector<int>& n,
                           const vector<double>& p,
                           bool mix_match = false);

    // Adds random graphs. For each graph size (number of nodes) n,
    // samples a graph from G(n,k*step) for all k from k=1 to 1/step
    // (not including 1/step).
    // Allow the user to control the number of sampled instances for
    // each graph.
    void add_random_graphs_pstep(const vector<int>& n,
                                 double step = 0.5,
                                 int instances = 3);

    // Scans the directory given by the user (recursively) and
    // adds all graphs found.
    // Optionally add filters to the path strings (graphs with
    // at least one filter as a substring won't be added).
    // Computes the desired fields and outputs to file.
    // Optionally, output progress to console.
    void compute(bool verbose = false);

    // Computes only the graph specified by the given number (1-indexed).
    // This is useful when show_graphs is true and the user wishes to run
    // the computation only on a specific graph added.
    // This is 1-indexed, s.t. if "Adding graph X/100: 'good_graph'" is
    // printed and the user wishes to run the graph good_graph only, call
    // compute_by_graph_number(X).
    void compute_by_graph_number(unsigned int i, bool verbose = false);
    void compute_by_graph_number_range(unsigned int first, unsigned int last, bool verbose = false);

    // Returns the statistics, as they are (no computation is done).
    vector<GraphStats> get_stats() const;

    // Returns the number of graphs in the DSG
    unsigned get_total_graphs() const;

    // Prints data to console (only valid data).
    void print() const;

    // Reads graph statistics from a CSV file.
    // Assumes the actual graph isn't encoded in the CSV (no actual
    // graph is stored).
    // Designed to work with the dumped CSV output generated in the DSG
    // when computing.
    static vector<GraphStats> read_stats(const string& filename);

};

}

#endif // DATASETSTATISTICSGENERATOR_H_INCLUDED
