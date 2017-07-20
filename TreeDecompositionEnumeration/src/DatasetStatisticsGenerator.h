#ifndef DATASETSTATISTICSGENERATOR_H_INCLUDED
#define DATASETSTATISTICSGENERATOR_H_INCLUDED

#include "Graph.h"
#include "GraphReader.h"
#include "MinimalSeparatorsEnumerator.h"
#include "PMCEnumerator.h"
#include "DirectoryIterator.h"
#include <string>
#include <vector>
#include <omp.h>
using std::vector;

namespace tdenum {

/**
 * A class responsible for taking input datasets (utilizing
 * the GraphReader class) and generating CSV files with:
 * - Number of nodes
 * - Number of edges
 * - Number of minimal separators
 * - Number of PMCs
 * - Number of minimal triangulations
 *
 * To use, construct the class with the name of the output file, add
 * graphs and then call compute() (or compute(true) to see progress).
 * To print results to the console, call print().
 *
 * In the future (if required), add functionality to actually list
 * the separators and PMCs themselves.
 *
 * If the output filename isn't given explicitly, make sure the directory
 * defined by DEFAULT_OUTPUT_DIR exists and can be reached relatively from
 * the point of code execution.
 */

#define DEFAULT_OUTPUT_DIR "./"
#define DSG_COMP_N 1
#define DSG_COMP_M 2
#define DSG_COMP_MS 4
#define DSG_COMP_PMC 8
#define DSG_COMP_TRNG 16
#define DSG_COMP_ALL (-1) // All bits are 1

/**
 * If calculations take too long, limit the number of
 * minimal separators / triangulations calculated (PMCs
 * are calculated all-or-nothing anyway), and / or the amount
 * of time (in seconds) required.
 *
 * To run with no limit, set DSG_<MS,TRNG>_<COUNT,TIME>_LIMIT
 * to DSG_NO_LIMIT.
 *
 * The output statistic will contain a "t" for out-of-time, or
 * a "+" for hitting DSG_COUNT limits.
 *
 * The CSV file WILL NOT reflect
 */
#define DSG_NO_LIMIT (-1)
#define DSG_MS_COUNT_LIMIT (500000)
#define DSG_TRNG_COUNT_LIMIT DSG_MS_COUNT_LIMIT
#define DSG_MS_TIME_LIMIT (20*60) // Ten minutes
#define DSG_TRNG_TIME_LIMIT DSG_MS_TIME_LIMIT
#define DSG_PMC_TIME_LIMIT (5*60)

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

    // If true, every graph added will cause a line to be printed to
    // console (with graph text).
    bool show_graphs;

    // For every i, the following vectors store the data of graph i.
    // Different threads access these at different indexes, so there
    // should be no need to lock any of them.
    vector<Graph> g;
    vector<string> text;
    vector<int> n;
    vector<int> m;
    vector<int> ms;
    vector<int> pmcs;
    vector<int> triangs;
    // Are the (n,m,ms,pmcs,triangs) fields valid for graph i?
    vector<bool> valid;
    // If the limit was overreached, set the relevant flag.
    vector<bool> ms_count_limit;
    vector<bool> ms_time_limit;
    vector<bool> trng_count_limit;
    vector<bool> trng_time_limit;
    vector<bool> pmc_time_limit;
    // The amount of time required to calculate the minimal separators.
    vector<string> ms_calc_time;

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

    // Return a header string
    string header(bool csv) const;

    // Stringify all data / single result, in CSV format or printable.
    string str(unsigned int i, bool csv) const;
    string str(bool csv) const;

    // Dump result line to file, or open a new file and output a header.
    // Can't be const, we need to lock the lock here
    void dump_line(unsigned int i);
    void dump_header();

    // If verbose computation is enabled, use this to print progress
    // to the screen.
    void print_progress();

    // Used to compute a single graph.
    // Optionally print verbose output.
    void compute(unsigned int i);

public:

    // Creates a new instance of the generator.
    // By ORing different flags the user may decide which fields to compute.
    // Optionally send a directory iterator to input graphs from.
    DatasetStatisticsGenerator(const string& outputfile = "DSG_OUT.csv", int flds = DSG_COMP_ALL);
    DatasetStatisticsGenerator(const string& outputfile,
                               DirectoryIterator di,
                               int flds = DSG_COMP_ALL);
    ~DatasetStatisticsGenerator();

    // If set to true, whenever a graph is added - print the text.
    void show_added_graphs();
    void dont_show_added_graphs();

    // Resets the DSG (doesn't change output filename
    void reset();

    // Changes filename (doesn't call reset())
    void change_outfile(const string&);

    // Add graphs.
    // The user may either send an input filename to read the data
    // from, or a graph instance.
    // In each case, text to be printed alongside the graph's computed
    // data may be sent as input (to identify the graph). This is a
    // required parameter if a Graph is sent, otherwise the default is
    // the filename.
    void add_graph(const Graph& g, const string& text);
    void add_random_graph(unsigned int n, double p, int instances = 1);
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
    void add_random_graphs(const vector<unsigned int>& n,
                           const vector<double>& p,
                           bool mix_match = false);

    // Adds random graphs. For each graph size (number of nodes) n,
    // samples a graph from G(n,k*step) for all k from k=1 to 1/step
    // (not including 1/step).
    // Allow the user to control the number of sampled instances for
    // each graph.
    void add_random_graphs_pstep(const vector<unsigned int>& n,
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

    // Prints data to console (only valid data).
    void print() const;

};

}

#endif // DATASETSTATISTICSGENERATOR_H_INCLUDED
