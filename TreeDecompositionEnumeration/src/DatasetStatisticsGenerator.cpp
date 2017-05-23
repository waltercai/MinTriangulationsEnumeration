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

void DatasetStatisticsGenerator::init_outfilename(const string& name) {
    if (name == string("")) {
        outfilename = infilename + ".Stats.csv";
    }
    else {
        outfilename = name;
    }
}

DatasetStatisticsGenerator::DatasetStatisticsGenerator(const string& filename,
                                                       const string& outfile) :
    infilename(filename), outfilename(outfile), valid(false),
    n(0), m(0), ms(0), pmcs(0) {
    if (outfilename == string("")) {
        outfilename = infilename + ".Stats.csv";
    }
    g = GraphReader::read(filename);
}
DatasetStatisticsGenerator::DatasetStatisticsGenerator(const Graph& G,
                                                       const string& outfile) :
    g(G), outfilename(outfile), valid(false),
    n(0), m(0), ms(0), pmcs(0) {
    outfilename = outfile;
}

bool DatasetStatisticsGenerator::is_valid() const {
    return valid;
}

void DatasetStatisticsGenerator::output_stats(bool verbose) {

    ofstream outfile;

    // Make sure values are updated
    if (!is_valid()) {
        get(verbose);
    }

    // Output to file
    outfile.open(outfilename, ios::out | ios::trunc);
    outfile << "Nodes,Edges,Minimal Separators,Potential Maximal Cliques\n";
    outfile << n << "," << m << "," << ms << "," << pmcs;
    outfile.close();
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

using namespace tdenum;

/**
 * Run the generator and output files given the datasets NOT in
 * the "difficult" folder (so it won't take forever).
 */
#define DATASET_DIR_BASE "../Datasets/"
#define DATASET_DIR_DEADEASY DATASET_DIR_BASE "DeadEasy/"
#define DATASET_DIR_EASY DATASET_DIR_BASE "Easy/Random/"
int main() {
    DirectoryIterator deadeasy_files(DATASET_DIR_DEADEASY);
    DirectoryIterator easy_files(DATASET_DIR_EASY);
    string dataset_filename;
    while(deadeasy_files.next_file(&dataset_filename)) {
        DatasetStatisticsGenerator dsg(dataset_filename);
        dsg.output_stats(true);
    }
    while(easy_files.next_file(&dataset_filename)) {
        DatasetStatisticsGenerator dsg(dataset_filename);
        dsg.output_stats(true);
    }
    return 0;
}





