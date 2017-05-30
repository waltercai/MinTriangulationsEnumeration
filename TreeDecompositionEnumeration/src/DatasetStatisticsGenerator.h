#ifndef DATASETSTATISTICSGENERATOR_H_INCLUDED
#define DATASETSTATISTICSGENERATOR_H_INCLUDED

#include "Graph.h"
#include "GraphReader.h"
#include "MinimalSeparatorsEnumerator.h"
#include "PMCEnumerator.h"
#include <string>

namespace tdenum {

/**
 * A class responsible for taking input datasets (utilizing
 * the GraphReader class) and generating CSV files with:
 * - Number of nodes
 * - Number of edges
 * - Number of minimal separators
 * - Number of PMCs
 *
 * In the future (if required), add functionality to actually list
 * the separators and PMCs themselves.
 *
 * If the output filename isn't given explicitly, make sure the directory
 * defined by DEFAULT_OUTPUT_DIR exists and can be reached relatively from
 * the point of code execution.
 */

#define DEFAULT_OUTPUT_DIR "../Results/"

class DatasetStatisticsGenerator {
private:
    Graph g;
    string infilename;
    string outfilename;
    // Nodes, edges, minimal separators and PMCs (total).
    // Add a flag to check if the values are up-to-date
    bool valid;
    // If true, output is appended to file
    bool oldfile;
    int n;
    int m;
    int ms;
    int pmcs;
    // Prints data to screen
    void print_stats() const;
    // The methods used to update each field value.
    void compute_nodes();
    void compute_edges();
    void compute_ms();
    void compute_pmcs();
    static const string header_str;
public:
    // The input is the filename of a dataset, and an optional output filename
    // for the generated statistics.
    // One can construct this class from an existing graph or from in input file
    // using GraphReader.
    // If of is set to true, the output is appended to the given output file.
    // Otherwise, a header is printed out.
    DatasetStatisticsGenerator(const Graph&,
                               const string& output,
                               bool of = true);
    DatasetStatisticsGenerator(const string& filename,
                               const string& output="",
                               bool of = true);
    // Generates and outputs statistics.
    // If verbose is set to true, the results will also be printed to the console.
    void get(bool verbose=false);
    // Outputs the data to file. Calls get() if need be.
    // Sometimes the input isn't from a file, so the filename_text is
    // what will be printed instead under the "Filename" column.
    void output_stats(const string& filename_text = "");
    // Outputs the header line to the file
    void output_header() const;
    // Returns true iff the computation required has already been done.
    bool is_valid() const;
    // Empties the file (useful to reset at the start of code).
    // CALLS output_header()!
    void reset_file(const string& filename="");
    // Starts to gather statistics for a new input graph
    void reset_graph(const Graph& g);
    void reset_graph(const string& infile);
};

}

#endif // DATASETSTATISTICSGENERATOR_H_INCLUDED
