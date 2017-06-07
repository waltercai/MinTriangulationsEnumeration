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
#include <math.h>
using std::ofstream;
using std::endl;
using std::ostringstream;
using std::setw;

namespace tdenum {

DatasetStatisticsGenerator::DatasetStatisticsGenerator(const string& outputfile, int flds) :
    outfilename(outputfile), fields(flds), max_text_len(10)/* Needs to be as long as "Graph text" */ {}

string DatasetStatisticsGenerator::header(bool csv) const {

    ostringstream oss;
    string delim(csv ? "," : "|");

    // Header
    oss.setf(std::ios_base::left, std::ios_base::adjustfield);
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
        oss << string(max_text_len+7+7+14+9+23, '=') << endl;
    }

    return oss.str();
}
string DatasetStatisticsGenerator::str(unsigned int i, bool csv) const {

    // Input validation
    if (i>=g.size() || !valid[i]) {
        return "INVALID INPUT\n";
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
    oss << endl;

    // That's it!
    return oss.str();
}
string DatasetStatisticsGenerator::str(bool csv) const {

    ostringstream oss;
    string delim(csv ? "," : "|");

    // Header
    oss << header(csv);

    // Data
    for (unsigned int i=0; i<g.size(); ++i) {
        // Don't print invalid rows
        if (!valid[i]) continue;
        oss << str(i,csv);
    }

    // That's it!
    return oss.str();
}


void DatasetStatisticsGenerator::dump_line(unsigned int i) const {
    ofstream outfile;
    outfile.open(outfilename, ios::out | ios::app);
    if (!outfile.good()) {
        TRACE(TRACE_LVL__ERROR, "Couldn't open file '" << outfilename << "'");
        return;
    }
    outfile << str(i, true);
}
void DatasetStatisticsGenerator::dump_header() const {
    ofstream outfile;
    outfile.open(outfilename, ios::out | ios::trunc);
    if (!outfile.good()) {
        TRACE(TRACE_LVL__ERROR, "Couldn't open file '" << outfilename << "'");
        return;
    }
    outfile << header(true);
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

    // Start a new file
    dump_header();

    // Dump all data, calculate the missing data
    for (unsigned int i=0; i<g.size(); ++i) {

        // No need to re-compute anything
        if (!valid[i]) {

            if (verbose) {
                char s[1000];
                time_t t = time(NULL);
                struct tm * p = localtime(&t);
                strftime(s, 1000, "%c", p);
                cout << s << ": Computing graph " << i+1 << "/" << g.size() << " (" << text[i] << ")..." << endl;
            }

            // Basics
            if (fields & DSG_COMP_N) {
                n[i] = g[i].getNumberOfNodes();
            }
            if (fields & DSG_COMP_M) {
                m[i] = g[i].getNumberOfEdges();
            }

            // Separators
            if (fields & DSG_COMP_MS) {
                ms[i] = 0;
                MinimalSeparatorsEnumerator mse(g[i], UNIFORM);
                while(mse.hasNext()) {
                    ++ms[i];
                    if (verbose)
                        cout << "\r " << n[i] << " | " << m[i] << " | " << ms[i] << " | ? | ? | ";
                    mse.next();
                }
            }

            // PMCs
            if (fields & DSG_COMP_PMC) {
                PMCEnumerator pmce(g[i]);
                NodeSetSet nss = pmce.get();
                pmcs[i] = nss.size();
                if (verbose)
                    cout << "\r " << n[i] << " | " << m[i] << " | " << ms[i] << " | " << pmcs[i] << " | ? | ";
            }

            // Triangulations
            if (fields & DSG_COMP_TRNG) {
                triangs[i] = 0;
                MinimalTriangulationsEnumerator enumerator(g[i], NONE, UNIFORM, SEPARATORS);
                while (enumerator.hasNext()) {
                    ++triangs[i];
                    if (verbose)
                        cout << "\r " << n[i] << " | " << m[i] << " | " << ms[i] << " | " << pmcs[i] << " | " << triangs[i];
                    enumerator.next();
                }
            }
        }

        // That's it for this one! Output to file
        valid[i] = true;
        if (verbose)
            cout << "Done, writing to file..." << endl;
        dump_line(i);
    }

    // Done
    if (verbose) cout << "done." << endl;
}

void DatasetStatisticsGenerator::print() const {
    cout << str(false);
}

}



