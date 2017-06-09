#ifndef DATASETSTATISTICSGENERATOR_H_INCLUDED
#define DATASETSTATISTICSGENERATOR_H_INCLUDED

#include "Graph.h"
#include "GraphReader.h"
#include "MinimalSeparatorsEnumerator.h"
#include "PMCEnumerator.h"
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

#define DEFAULT_OUTPUT_DIR "/"
#define DSG_COMP_N 1
#define DSG_COMP_M 2
#define DSG_COMP_MS 4
#define DSG_COMP_PMC 8
#define DSG_COMP_TRNG 16
#define DSG_COMP_ALL (-1) // All bits are 1

class DatasetStatisticsGenerator {
private:

    // Single output file, where the data will be dumped.
    const string outfilename;

    // The fields to be calculated (ORed flags).
    const int fields;

    // For every i, the following vectors store the data of graph i.
    vector<Graph> g;
    vector<string> text;
    vector<int> n;
    vector<int> m;
    vector<int> ms;
    vector<int> pmcs;
    vector<int> triangs;
    // Are the (n,m,ms,pmcs,triangs) fields valid for graph i?
    vector<bool> valid;

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
    void print_progress(bool verbose);

    // Used to compute a single graph.
    // Optionally print verbose output.
    void compute(unsigned int i, bool verbose=false);

public:

    // Creates a new instance of the generator.
    // By ORing different flags the user may decide which fields to compute.
    DatasetStatisticsGenerator(const string& outputfile, int flds = DSG_COMP_ALL);
    ~DatasetStatisticsGenerator();

    // Add graphs.
    // The user may either send an input filename to read the data
    // from, or a graph instance.
    // In each case, text to be printed alongside the graph's computed
    // data may be sent as input (to identify the graph). This is a
    // required parameter if a Graph is sent, otherwise the default is
    // the filename.
    void add_graph(const Graph& g, const string& text);
    void add_graph(const string& filename, const string& text = "");

    // Computes the desired fields and outputs to file.
    // Optionally, output progress to console.
    void compute(bool verbose = false);

    // Prints data to console (only valid data).
    void print() const;

};

}

#endif // DATASETSTATISTICSGENERATOR_H_INCLUDED
