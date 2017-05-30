#include "DatasetStatisticsGenerator.h"
#include "DataStructures.h"
#include "DirectoryIterator.h"
#include "Utils.h"
#include "TestUtils.h"
#include <iostream>
#include <fstream>
using std::ofstream;
using std::endl;

namespace tdenum {


const string DatasetStatisticsGenerator::header_str="Filename,Nodes,Edges,Minimal Separators,PMCs\n";


DatasetStatisticsGenerator::DatasetStatisticsGenerator(const string& filename,
                                                       const string& outfile,
                                                       bool of) :
    infilename(filename), outfilename(outfile), valid(false), oldfile(of),
    n(0), m(0), ms(0), pmcs(0) {
    if (outfilename == string("")) {
        string basename = infilename.substr(infilename.rfind(string(1,SLASH))+1);
        outfilename = DEFAULT_OUTPUT_DIR + basename + ".Stats.csv";
    }
    g = GraphReader::read(filename);
    TRACE(TRACE_LVL__DEBUG, "Created stats generator, output name will be:" << endl
                            << outfilename << endl);
    if (!of) {
        reset_file();
    }
}
DatasetStatisticsGenerator::DatasetStatisticsGenerator(const Graph& G,
                                                       const string& outfile,
                                                       bool of) :
    g(G), outfilename(outfile), valid(false), oldfile(of),
    n(0), m(0), ms(0), pmcs(0) {
    outfilename = outfile;
    if (!of) {
        reset_file();
    }
}

bool DatasetStatisticsGenerator::is_valid() const {
    return valid;
}

/**
 * Outputs CSV format to file.
 */
void DatasetStatisticsGenerator::output_stats(const string& filename_text) {

    ofstream outfile;

    // Make sure values are updated
    if (!is_valid()) {
        get();
    }

    // Output to file
    outfile.open(outfilename, ios::out | ios::app);
    if (!outfile.good()) {
        TRACE(TRACE_LVL__ERROR, "Couldn't open file '" << outfilename << "'");
        return;
    }
    outfile << (filename_text == "" ? infilename : filename_text) << ","
            << n << ","
            << m << ","
            << ms << ","
            << pmcs << "\n";
}

void DatasetStatisticsGenerator::output_header() const {
    ofstream outfile;
    outfile.open(outfilename, ios::out | ios::trunc);
    outfile << DatasetStatisticsGenerator::header_str;
    return;
}

void DatasetStatisticsGenerator::reset_file(const string& filename) {
    if (filename != "") {
        outfilename = filename;
    }
    output_header();
}

void DatasetStatisticsGenerator::reset_graph(const Graph& G) {
    g = G;
    valid = false;
}
void DatasetStatisticsGenerator::reset_graph(const string& infile) {
    g = GraphReader::read(infile);
    valid = false;
}

void DatasetStatisticsGenerator::compute_nodes() {
    n = g.getNumberOfNodes();
}

void DatasetStatisticsGenerator::compute_edges() {
    m = g.getNumberOfEdges();
}

void DatasetStatisticsGenerator::compute_ms() {
    MinimalSeparatorsEnumerator mse(g, UNIFORM);
    while(mse.hasNext()) {
        ++ms;
        mse.next();
    }
}

void DatasetStatisticsGenerator::compute_pmcs() {
    PMCEnumerator pmce(g);
    NodeSetSet nss = pmce.get();
    pmcs = nss.size();
}

void DatasetStatisticsGenerator::get(bool verbose) {
    if (!is_valid()) {
        compute_nodes();
        compute_edges();
        compute_ms();
        compute_pmcs();
        valid = true;
    }
    if (verbose) {
        cout << "Nodes: " << n << endl;
        cout << "Edges: " << m << endl;
        cout << "Minimal separators: " << ms << endl;
        cout << "Potential maximal cliques: " << pmcs << endl;
    }
}

}






