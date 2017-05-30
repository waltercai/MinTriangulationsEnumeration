#include "DatasetStatisticsGenerator.h"
#include "MinimalTriangulationsEnumerator.h"
#include "DataStructures.h"
#include "DirectoryIterator.h"
#include "Utils.h"
#include "TestUtils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
using std::ofstream;
using std::endl;
using std::ostringstream;
using std::setw;

namespace tdenum {

DatasetStatisticsGenerator::DatasetStatisticsGenerator(const string& outputfile, int flds) :
    outfilename(outputfile), fields(flds), max_text_len(10)/* Needs to be as long as "Graph text" */ {}

string DatasetStatisticsGenerator::str(unsigned int i, bool csv) const {

    // Input validation
    if (i>=g.size() || !valid[i]) {
        return "INVALID INPUT";
    }

    ostringstream oss;
    string delim(csv ? "," : "|");
    oss.setf(std::ios_base::left, std::ios_base::adjustfield);

    // Identifying text
    if (csv) {
        // Use quotes to escape commas and such, for CSV.
        // If the user sends opening quotes this won't work but
        // danger is my middle name
        oss << "\"" << text[i] << "\"";
    }
    else {
        oss << setw(max_text_len) << text[i];
    }

    // Fields
    if (fields & DSG_COMP_N) {
        oss << delim << setw(6) << n[i];
    }
    if (fields & DSG_COMP_M) {
        oss << delim << setw(6) << m[i];
    }
    if (fields & DSG_COMP_MS) {
        oss << delim << setw(18) << ms[i];
    }
    if (fields & DSG_COMP_PMC) {
        oss << delim << setw(8) << pmcs[i];
    }
    if (fields & DSG_COMP_TRNG) {
        oss << delim << setw(22) << triangs[i];
    }

    // That's it!
    return oss.str();
}
string DatasetStatisticsGenerator::str(bool csv) const {

    ostringstream oss;
    string delim(csv ? "," : "|");

    // Header
    oss << setw(max_text_len) << "Graph text";
    if (fields & DSG_COMP_N) {
        oss << delim << "Nodes ";
    }
    if (fields & DSG_COMP_M) {
        oss << delim << "Edges ";
    }
    if (fields & DSG_COMP_MS) {
        oss << delim << "Minimal separators";
    }
    if (fields & DSG_COMP_PMC) {
        oss << delim << "PMCs    ";
    }
    if (fields & DSG_COMP_TRNG) {
        oss << delim << "Minimal triangulations";
    }
    oss << endl;
    if (!csv) {
        oss << "=================================================================" << endl;
    }

    // Data
    for (unsigned int i=0; i<g.size(); ++i) {
        // Don't print invalid rows
        if (!valid[i]) continue;
        oss << str(i,csv) << endl;
    }

    // That's it!
    return oss.str();
}

void DatasetStatisticsGenerator::add_graph(const Graph& graph, const string& txt) {

    // Add vector elements
    g.push_back(graph);
    text.push_back(txt);
    valid.push_back(false);
    n.push_back(0);
    m.push_back(0);
    ms.push_back(0);
    pmcs.push_back(0);
    triangs.push_back(0);

    // Update max text length
    if (max_text_len < strlen(txt.c_str())) {
        max_text_len = strlen(txt.c_str());
    }
}
void DatasetStatisticsGenerator::add_graph(const string& filename, const string& text) {
    Graph g = GraphReader::read(filename);
    add_graph(g, text == "" ? filename : text);
}

void DatasetStatisticsGenerator::compute(bool verbose) {

    // Compute
    for (unsigned int i=0; i<g.size(); ++i) {

        // No need to re-compute anything
        if (valid[i]) continue;

        if (verbose) {
            cout << "Computing graph " << i+1 << "/" << g.size() << " (" << text[i] << ")... ";
        }
        time_t t = time(NULL);

        // Basics
        if (fields & DSG_COMP_N) {
            n[i] = g[i].getNumberOfNodes();
            if (verbose) cout << ".";
        }
        if (fields & DSG_COMP_M) {
            m[i] = g[i].getNumberOfEdges();
            if (verbose) cout << ".";
        }

        // Separators
        if (fields & DSG_COMP_MS) {
            ms[i] = 0;
            MinimalSeparatorsEnumerator mse(g[i], UNIFORM);
            while(mse.hasNext()) {
                ++ms[i];
                mse.next();
            }
            if (verbose) cout << ".";
        }

        // PMCs
        if (fields & DSG_COMP_PMC) {
            PMCEnumerator pmce(g[i]);
            NodeSetSet nss = pmce.get();
            pmcs[i] = nss.size();
            if (verbose) cout << ".";
        }

        // Triangulations
        if (fields & DSG_COMP_TRNG) {
            triangs[i] = 0;
            MinimalTriangulationsEnumerator enumerator(g[i], NONE, UNIFORM, SEPARATORS);
            while (enumerator.hasNext()) {
                ++triangs[i];
                enumerator.next();
            }
            if (verbose) cout << ".";
        }

        // That's it for this one!
        valid[i] = true;
        if (verbose) cout << " done (in " << time(NULL) - t << " seconds)." << endl;
    }

    // Output
    if (verbose) cout << "Dumping to file... ";
    ofstream outfile;
    outfile.open(outfilename, ios::out | ios::trunc);
    if (!outfile.good()) {
        TRACE(TRACE_LVL__ERROR, "Couldn't open file '" << outfilename << "'");
        return;
    }
    outfile << str(true);
    if (verbose) cout << "done." << endl;
}

void DatasetStatisticsGenerator::print() const {
    cout << str(false);
}

}



/*
const string DatasetStatisticsGenerator::header_str="Filename,Nodes,Edges,Minimal Separators,PMCs,Minimal triangulations\n";

DatasetStatisticsGenerator::DatasetStatisticsGenerator(const string& filename,
                                                       const string& outfile,
                                                       bool of) :
    infilename(filename), outfilename(outfile), valid(false), oldfile(of),
    n(0), m(0), ms(0), pmcs(0), triangs(0) {
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
    n(0), m(0), ms(0), pmcs(0), triangs(0) {
    outfilename = outfile;
    if (!of) {
        reset_file();
    }
}

bool DatasetStatisticsGenerator::is_valid() const {
    return valid;
}

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
            << pmcs << ","
            << triangs << "\n";
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
    ms = 0;
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

void DatasetStatisticsGenerator::compute_traingulations() {
    triangs = 0;
    MinimalTriangulationsEnumerator enumerator(g, NONE, UNIFORM, SEPARATORS);
    while (enumerator.hasNext()) {
        ++triangs;
    }
}

void DatasetStatisticsGenerator::get() {
    if (!is_valid()) {
        compute_nodes();
        compute_edges();
        compute_ms();
        compute_pmcs();
        compute_traingulations();
        valid = true;
    }
}

}*/






