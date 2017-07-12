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

/**
 * Note that we're using locks, so they need to be created and destroyed.
 *
 * Also, max_text_len needs to be at least the size of the string "Graph text"
 */
DatasetStatisticsGenerator::DatasetStatisticsGenerator(const string& outputfile, int flds) :
                            outfilename(outputfile), fields(flds), has_random(false),
                            show_graphs(true), graphs_computed(0), max_text_len(10) {
    // Locking mechanism
    omp_init_lock(&lock);

    // If PMCs are to be calculated, we need the minimal separators
    // anyway. We'll re-use the result, this doesn't slow us down.
    if (fields & DSG_COMP_PMC) {
        fields |= DSG_COMP_MS;
    }
}
DatasetStatisticsGenerator::~DatasetStatisticsGenerator() {
    omp_destroy_lock(&lock);
}

void DatasetStatisticsGenerator::show_added_graphs() {
    show_graphs = true;
}
void DatasetStatisticsGenerator::dont_show_added_graphs() {
    show_graphs = false;
}


/**
 * Useful for stringifying output, in CSV format or human readable.
 */
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
    // Special columns
    if (has_random) {
        oss << delim << "P value" << delim << "Edge ratio";
    }
    if (fields & DSG_COMP_MS) {
        oss << delim << "MS calculation time";
    }
    if (DSG_MS_TIME_LIMIT != DSG_NO_LIMIT || DSG_TRNG_TIME_LIMIT != DSG_NO_LIMIT || DSG_PMC_TIME_LIMIT != DSG_NO_LIMIT) {
        oss << delim << "Time errors";
    }
    if (DSG_MS_COUNT_LIMIT != DSG_NO_LIMIT || DSG_TRNG_COUNT_LIMIT != DSG_NO_LIMIT) {
        oss << delim << "Count errors";
    }
    oss << endl;
    if (!csv) {
        // Minus one, for the end-line
        oss << string(strlen(oss.str().c_str())-1, '=') << endl;
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
    // Special columns
    if (has_random) {
        if (g[i].isRandom()) {
            oss << delim << setw(7) << g[i].getP();
            int N = g[i].getNumberOfNodes();
            int M = g[i].getNumberOfEdges();
            oss << delim << setw(10) << 2*M / (double(N*(N-1))); // |E|/(|V| choose 2)
        }
        else {
            oss << delim << setw(7) << " ";
            oss << delim << setw(10) << " ";
        }
    }
    // MS are calculated in one go if we're calculating PMCs,
    // no point in printing MS calculation time
    if ((fields & DSG_COMP_MS) && !(fields & DSG_COMP_PMC)) {
        oss << delim << setw(19) << ms_calc_time[i];
    }
    if (DSG_MS_TIME_LIMIT != DSG_NO_LIMIT || DSG_TRNG_TIME_LIMIT != DSG_NO_LIMIT || DSG_PMC_TIME_LIMIT != DSG_NO_LIMIT) {
        string s;
        if (ms_time_limit[i] || trng_time_limit[i] || pmc_time_limit[i]) {
            if (ms_time_limit[i]) {
                s += string("MS");
            }
            if (trng_time_limit[i]) {
                s += (ms_time_limit[i] ? "," : "") + string("TRNG");
            }
            if (pmc_time_limit[i]) {
                s += (ms_time_limit[i] || trng_time_limit[i] ? "," : "") + string("PMC");
            }
        }
        else {
            s = " ";
        }
        oss << delim << setw(11) << s;
    }
    if (DSG_MS_COUNT_LIMIT != DSG_NO_LIMIT || DSG_TRNG_COUNT_LIMIT != DSG_NO_LIMIT) {
        string s;
        if (ms_count_limit[i] || trng_count_limit[i]) {
            if (ms_count_limit[i]) {
                s += string("MS");
            }
            if (trng_count_limit[i]) {
                s += (ms_count_limit[i] ? "," : "") + string("TRNG");
            }
        }
        else {
            s = " ";
        }
        oss << delim << setw(12) << s;
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

/**
 * When dumping to file, it's always in CSV format.
 * Note that dump_line() is called by compute() which may be running
 * in several parallel instances, so be thread safe!
 */
void DatasetStatisticsGenerator::dump_line(unsigned int i) {
    ofstream outfile;
    omp_set_lock(&lock);
    outfile.open(outfilename, ios::out | ios::app);
    if (!outfile.good()) {
        TRACE(TRACE_LVL__ERROR, "Couldn't open file '" << outfilename << "'");
        omp_unset_lock(&lock);
        return;
    }
    outfile << str(i, true);
    omp_unset_lock(&lock);
}
void DatasetStatisticsGenerator::dump_header() {
    ofstream outfile;
    omp_set_lock(&lock);
    outfile.open(outfilename, ios::out | ios::trunc);
    if (!outfile.good()) {
        TRACE(TRACE_LVL__ERROR, "Couldn't open file '" << outfilename << "'");
        omp_unset_lock(&lock);
        return;
    }
    outfile << header(true);
    omp_unset_lock(&lock);
}

/**
 * Useful when running batch jobs with multiple processors.
 * This may be called by multiple threads, so the overhead here
 * is for non-garbled output.
 */
void DatasetStatisticsGenerator::print_progress()  {

    // Duh.
    if (!verbose) return;

    // This needs to be locked...
    omp_set_lock(&lock);

    // Construct and print the string
    ostringstream oss;
    oss << "\rN/M";
    if (fields & DSG_COMP_MS) oss << "/MS";
    if (fields & DSG_COMP_PMC) oss << "/PMCS";
    if (fields & DSG_COMP_TRNG) oss << "/TRNG";
    for(unsigned int i=0; i<graphs_in_progress.size(); ++i) {
        unsigned int j = graphs_in_progress[i];
        oss << " | " << n[j] << "/" << m[j];
        if (fields & DSG_COMP_MS) {
            oss << "/" << ms[j] << (ms_count_limit[j] ? "+" : "") << (ms_time_limit[j] ? "t" : "");
        }
        if (fields & DSG_COMP_PMC) oss << "/" << pmcs[j];
        if (fields & DSG_COMP_TRNG) {
            oss << "/" << triangs[j] << (trng_count_limit[j] ? "+" : "") << (trng_time_limit[j] ? "t" : "");
        }
    }
    cout << oss.str();

    // Unlock
    omp_unset_lock(&lock);

}

/**
 * If the graph is given by a filename, GraphReader is used.
 */
void DatasetStatisticsGenerator::add_graph(const Graph& graph, const string& txt) {

    // Add vector elements
    g.push_back(graph);
    text.push_back(txt);
    valid.push_back(false);
    ms_count_limit.push_back(false);
    ms_time_limit.push_back(false);
    trng_count_limit.push_back(false);
    trng_time_limit.push_back(false);
    pmc_time_limit.push_back(false);
    ms_calc_time.push_back(string("0"));
    n.push_back(0);
    m.push_back(0);
    ms.push_back(0);
    pmcs.push_back(0);
    triangs.push_back(0);

    // Update max text length
    if (max_text_len < strlen(txt.c_str())) {
        max_text_len = strlen(txt.c_str());
    }

    // If this is a random graph, inform the class
    if (graph.isRandom()) {
        has_random = true;
    }

    // If show_graphs is true, print the text
    if (show_graphs) {
        cout << "Added graph " << n.size() << ": '" << txt << "'" << endl;
    }

}
void DatasetStatisticsGenerator::add_graph(const string& filename, const string& text) {
    Graph g = GraphReader::read(filename);
    add_graph(g, text == "" ? filename : text);
}

/**
 * Computes the fields requested by the user for all graphs / a specific graph.
 * Calling compute() (all graph mode) is parallelized on supporting platforms,
 * so be cautious when editing compute(i)!
 */
void DatasetStatisticsGenerator::compute(unsigned int i) {

    // For printing the time
    char s[1000];
    time_t t;
    struct tm * p;

    // Add this graph to the  list of 'in progress' graphs.
    // This is a shared resource, so lock it.
    // If verbose is set to false, no need to keep track of the
    // graphs in progress.
    if (verbose) {
        omp_set_lock(&lock);
        graphs_in_progress.push_back(i);
        omp_unset_lock(&lock);
    }

    // Basics
    if (fields & DSG_COMP_N) {
        n[i] = g[i].getNumberOfNodes();
    }
    if (fields & DSG_COMP_M) {
        m[i] = g[i].getNumberOfEdges();
    }

    // Separators.
    // We may need the result for PMCs
    NodeSetSet min_seps;
    if (fields & DSG_COMP_MS) {
        ms[i] = 0;
        MinimalSeparatorsEnumerator mse(g[i], UNIFORM);
        t = time(NULL);
        while(mse.hasNext()) {
            min_seps.insert(mse.next());
            ++ms[i];
            print_progress();
            if (DSG_MS_COUNT_LIMIT != DSG_NO_LIMIT && ms[i] > DSG_MS_COUNT_LIMIT) {
                ms_count_limit[i] = true;
                break;
            }
            if (DSG_MS_TIME_LIMIT != DSG_NO_LIMIT && time(NULL)-t > DSG_MS_TIME_LIMIT) {
                ms_time_limit[i] = true;
                break;
            }
            mse.next();
        }
        ms_calc_time[i] = secs_to_hhmmss(time(NULL)-t);
    }

    // PMCs.
    // The minimal separators are free, so no need to calculate
    // them separately.
    if (fields & DSG_COMP_PMC) {
        PMCEnumerator pmce(g[i], DSG_PMC_TIME_LIMIT == DSG_NO_LIMIT ? 0 : DSG_PMC_TIME_LIMIT);
        // Re-use the calculated minimal separators, if relevant
        if (!(ms_count_limit[i] || ms_time_limit[i])) {
            pmce.set_minimal_separators(min_seps);
        }
        pmce.get();
        pmcs[i] = pmce.get().size();
        ms[i] = pmce.get_ms().size();
        if (pmce.is_out_of_time()) {
            pmc_time_limit[i] = true;
        }
        print_progress();
    }

    // Triangulations
    if (fields & DSG_COMP_TRNG) {
        triangs[i] = 0;
        MinimalTriangulationsEnumerator enumerator(g[i], NONE, UNIFORM, SEPARATORS);
        if (DSG_MS_TIME_LIMIT != DSG_NO_LIMIT) {
            t = time(NULL);
        }
        while (enumerator.hasNext()) {
            ++triangs[i];
            print_progress();
            if (DSG_TRNG_COUNT_LIMIT != DSG_NO_LIMIT && triangs[i] > DSG_TRNG_COUNT_LIMIT) {
                trng_count_limit[i] = true;
                break;
            }
            if (DSG_TRNG_TIME_LIMIT != DSG_NO_LIMIT && time(NULL)-t > DSG_TRNG_TIME_LIMIT) {
                trng_time_limit[i] = true;
                break;
            }
            enumerator.next();
        }
    }

    // That's it for this one!
    valid[i] = true;

    // Printing and cleanup.
    // No need for any of this if verbose isn't true.
    if (verbose) {
        omp_set_lock(&lock);

        // Update the graphs in progress (while locked)
        REMOVE_FROM_VECTOR(graphs_in_progress, i);
        graphs_computed++;

        // Print:
        // We may need to clear the line (if progress was printed)..
        cout << "\r" << string(50+max_text_len, ' ') << "\r";

        // Get the time and output.
        // If any limits were encountered, be verbose!
        t = time(NULL);
        p = localtime(&t);
        strftime(s, 1000, "%c", p);
        cout << s << ": ";
        if (ms_time_limit[i] || trng_time_limit[i] || pmc_time_limit[i]) {
            cout << "OUT OF TIME on graph ";
        }
        else if (ms_count_limit[i] || trng_count_limit[i]) {
            cout << "HIT NUMERIC LIMIT on graph ";
        }
        else {
            cout << "Done computing graph ";
        }
        cout << graphs_computed+1 << "/" << g.size() << ","
                  << "'" <<text[i] << "'" << endl;

        // Unlock
        omp_unset_lock(&lock);
    }

}
void DatasetStatisticsGenerator::compute_by_graph_number_range(unsigned int first, unsigned int last, bool v) {

    // Validate input
    if (first < 1 || first > last || last > n.size()) {
        cout << "Bad parameter: called compute_by_graph_number_range(first,last) "
             << "with first=" << first << " and last=" << last
             << ". Legal values require 1<=first<=last<=" << n.size() << endl;
        return;
    }

    // Set the verbose member:
    verbose = v;

    // Start a new file
    dump_header();

    // Dump all data, calculate the missing data.
    // If possible, parallelize this
#pragma omp parallel for
    for (unsigned int i=first-1; i<last; ++i) {
        if (!valid[i]) {
            compute(i);
        }
        dump_line(i);
    }
}
void DatasetStatisticsGenerator::compute(bool v) {
    compute_by_graph_number_range(1, n.size(), v);
}
void DatasetStatisticsGenerator::compute_by_graph_number(unsigned int i, bool v) {
    if (i<1 || i>n.size()) {
        cout << "Bad parameter: called compute_by_graph_number(i) with i=" << i
             << ". Legal values of i are between 1 and " << n.size() << endl;
        return;
    }
    compute_by_graph_number_range(i,i,v);
}



/**
 * Stringify currently computed data and output to console in human readable
 * format.
 */
void DatasetStatisticsGenerator::print() const {
    cout << str(false);
}

}



