#ifndef DATASETSTATISTICSGENERATOR_H_INCLUDED
#define DATASETSTATISTICSGENERATOR_H_INCLUDED

#include "Graph.h"
#include "GraphReader.h"
#include "MinimalSeparatorsEnumerator.h"
#include "PMCEnumerator.h"
#include <string>
#include <vector>
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

    // The fields to be calculated.
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

    // Used for printing to console.
    unsigned int max_text_len;

    // Stringify a single result, in CSV format or printable.
    string str(unsigned int i, bool csv) const;

    // Stringify the data, in CSV format or printable format.
    string str(bool csv) const;

public:

    // Creates a new instance of the generator.
    // By ORing different flags the user may decide which fields to compute.
    DatasetStatisticsGenerator(const string& outputfile, int flds = DSG_COMP_ALL);

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
