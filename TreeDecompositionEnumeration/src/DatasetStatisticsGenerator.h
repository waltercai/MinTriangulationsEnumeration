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
    bool append;
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
public:
    // The input is the filename of a dataset, and an optional output filename
    // for the generated statistics.
    // One can construct this class from an existing graph or from in input file
    // using GraphReader.
    // If append is set to true, the output is appended to the given output file.
    DatasetStatisticsGenerator(const Graph&,
                               const string& output,
                               bool append = true);
    DatasetStatisticsGenerator(const string& filename,
                               const string& output="",
                               bool append = true);
    // Generates and outputs statistics.
    // If verbose is set to true, the results will also be printed to the console.
    void get(bool verbose=true);
    // Outputs the data to file. Calls get() if need be. As with get, a verbose flag
    // is provided.
    void output_stats(bool verbose=true);
    // Returns true iff the computation required has already been done.
    bool is_valid() const;
};

}

#endif // DATASETSTATISTICSGENERATOR_H_INCLUDED
