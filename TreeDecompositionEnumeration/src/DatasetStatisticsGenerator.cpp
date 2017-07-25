#include "DatasetStatisticsGenerator.h"
#include "MinimalTriangulationsEnumerator.h"
#include "DataStructures.h"
#include "GraphProducer.h"
#include "DirectoryIterator.h"
#include "Utils.h"
#include "TestUtils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <math.h>
#include <string>
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
                            outfilename(outputfile),
                            fields(flds),
                            has_random(false),
                            verbose(false),
                            allow_dump_flag(true),
                            allow_parallel(true),
                            show_graphs(true),
                            has_ms_time_limit(false),
                            has_pmc_time_limit(false),
                            has_trng_time_limit(false),
                            has_ms_count_limit(false),
                            has_trng_count_limit(false),
                            ms_time_limit(GRAPHSTATS_MS_TIME_LIMIT),
                            pmc_time_limit(GRAPHSTATS_PMC_TIME_LIMIT),
                            trng_time_limit(GRAPHSTATS_TRNG_TIME_LIMIT),
                            ms_count_limit(GRAPHSTATS_MS_COUNT_LIMIT),
                            trng_count_limit(GRAPHSTATS_TRNG_COUNT_LIMIT),
                            pmc_alg(PMCEnumerator::ALG_NORMAL),
                            graphs_computed(0),
                            max_text_len(STRLEN(DSG_COL_TXT))
{
    // Locking mechanism
    omp_init_lock(&lock);

    // If PMCs are to be calculated, we need the minimal separators
    // anyway. We'll re-use the result, this doesn't slow us down.
    if (GRAPHSTATS_TEST_PMC(fields)) {
        GRAPHSTATS_SET_MS(fields);
    }
}
DatasetStatisticsGenerator::DatasetStatisticsGenerator(const string& outputfile,
                                                       DirectoryIterator di,
                                                       int flds) :
                            DatasetStatisticsGenerator(outputfile, flds)
{
    // Input all graphs using the directory iterator
    add_graphs(di);
}
DatasetStatisticsGenerator::DatasetStatisticsGenerator(int flds) :
                            DatasetStatisticsGenerator("", flds)
{
    suppress_dump();
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

void DatasetStatisticsGenerator::reset(int flds) {
    *this = DatasetStatisticsGenerator(outfilename, flds);
}

void DatasetStatisticsGenerator::disable_all_limits() {
    disable_all_count_limits();
    disable_all_time_limits();
}
void DatasetStatisticsGenerator::disable_all_count_limits() {
    has_ms_count_limit = false;
    has_trng_count_limit = false;
}
void DatasetStatisticsGenerator::disable_all_time_limits() {
    has_ms_time_limit = false;
    has_pmc_time_limit = false;
    has_trng_time_limit = false;
}
void DatasetStatisticsGenerator::set_ms_time_limit(time_t t) {
    ms_time_limit = t;
    has_ms_time_limit = true;
}
void DatasetStatisticsGenerator::set_trng_time_limit(time_t t) {
    trng_time_limit = t;
    has_trng_time_limit = true;
}
void DatasetStatisticsGenerator::set_pmc_time_limit(time_t t) {
    pmc_time_limit = t;
    has_pmc_time_limit = true;
}
void DatasetStatisticsGenerator::set_ms_count_limit(unsigned long x) {
    ms_count_limit = x;
    has_ms_count_limit = true;
}
void DatasetStatisticsGenerator::set_trng_count_limit(unsigned long x) {
    trng_count_limit = x;
    has_trng_count_limit = true;
}
bool DatasetStatisticsGenerator::has_time_limit() const {
    return (has_ms_time_limit || has_pmc_time_limit || has_trng_time_limit);
}
bool DatasetStatisticsGenerator::has_count_limit() const {
    return (has_ms_count_limit || has_trng_count_limit);
}

void DatasetStatisticsGenerator::force_recalc() {
    for (unsigned i=0; i<gs.size(); ++i) {
        gs[i].set_all_invalid();
    }
    graphs_computed = 0;
}

void DatasetStatisticsGenerator::change_outfile(const string& name) {
    outfilename = name;
}
void DatasetStatisticsGenerator::suppress_dump() {
    allow_dump_flag = false;
}
void DatasetStatisticsGenerator::allow_dump(const string& name) {
    change_outfile(name);
    allow_dump_flag = true;
}

void DatasetStatisticsGenerator::suppress_async() {
    allow_parallel = false;
}
void DatasetStatisticsGenerator::allow_async() {
    allow_parallel = true;
}

void DatasetStatisticsGenerator::set_pmc_alg(PMCEnumerator::Alg alg) {
    pmc_alg = alg;
}

/**
 * Useful for stringifying output, in CSV format or human readable.
 */
string DatasetStatisticsGenerator::header(bool csv) const {

    ostringstream oss;
    string delim(csv ? "," : "|");

    // Header
    oss.setf(std::ios_base::left, std::ios_base::adjustfield);
    oss << setw(max_text_len) << DSG_COL_TXT;
    if (GRAPHSTATS_TEST_N(fields)) {
        oss << delim << DSG_COL_NODES;
    }
    if (GRAPHSTATS_TEST_M(fields)) {
        oss << delim << DSG_COL_EDGES;
    }
    if (GRAPHSTATS_TEST_MS(fields)) {
        oss << delim << DSG_COL_MSS;
    }
    if (GRAPHSTATS_TEST_PMC(fields)) {
        oss << delim << DSG_COL_PMCS;
    }
    if (GRAPHSTATS_TEST_TRNG(fields)) {
        oss << delim << DSG_COL_TRNG;
    }
    // Special columns
    if (has_random) {
        oss << delim << DSG_COL_P << delim << DSG_COL_RATIO;
    }
    if (GRAPHSTATS_TEST_MS(fields)) {
        oss << delim << DSG_COL_MS_TIME;
    }
    if (has_time_limit()) {
        oss << delim << DSG_COL_ERR_TIME;
    }
    if (has_count_limit()) {
        oss << delim << DSG_COL_CNT_TIME;
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
    if (i>=gs.size() || !gs[i].valid(fields)) {
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
        oss << "\"" << gs[i].text << "\"";
    }
    else {
        oss << setw(max_text_len) << gs[i].text;
    }

    // Fields
    if (GRAPHSTATS_TEST_N(fields)) {
        oss << delim << setw(STRLEN(DSG_COL_NODES)) << gs[i].n;
    }
    if (GRAPHSTATS_TEST_M(fields)) {
        oss << delim << setw(STRLEN(DSG_COL_EDGES)) << gs[i].m;
    }
    if (GRAPHSTATS_TEST_MS(fields)) {
        oss << delim << setw(STRLEN(DSG_COL_MSS)) << gs[i].ms;
    }
    if (GRAPHSTATS_TEST_PMC(fields)) {
        oss << delim << setw(STRLEN(DSG_COL_PMCS)) << gs[i].pmcs;
    }
    if (GRAPHSTATS_TEST_TRNG(fields)) {
        oss << delim << setw(STRLEN(DSG_COL_TRNG)) << gs[i].triangs;
    }
    // Special columns
    if (has_random) {
        if (gs[i].g.isRandom()) {
            oss << delim << setw(STRLEN(DSG_COL_P)) << gs[i].g.getP();
            int N = gs[i].g.getNumberOfNodes();
            int M = gs[i].g.getNumberOfEdges();
            oss << delim << setw(STRLEN(DSG_COL_RATIO)) << 2*M / (double(N*(N-1))); // |E|/(|V| choose 2)
        }
        else {
            oss << delim << setw(STRLEN(DSG_COL_P)) << " ";
            oss << delim << setw(STRLEN(DSG_COL_RATIO)) << " ";
        }
    }
    // MS are calculated in one go if we're calculating PMCs,
    // no point in printing MS calculation time
    if (GRAPHSTATS_TEST_MS(fields) && !GRAPHSTATS_TEST_PMC(fields)) {
        oss << delim << setw(STRLEN(DSG_COL_MS_TIME)) << gs[i].ms_calc_time;
    }
    if (has_time_limit()) {
        string s;
        if (gs[i].ms_time_limit || gs[i].trng_time_limit || gs[i].pmc_time_limit) {
            if (gs[i].ms_time_limit) {
                s += string("MS");
            }
            if (gs[i].trng_time_limit) {
                s += (gs[i].ms_time_limit ? "," : "") + string("TRNG");
            }
            if (gs[i].pmc_time_limit) {
                s += (gs[i].ms_time_limit || gs[i].trng_time_limit ? "," : "") + string("PMC");
            }
        }
        else {
            s = " ";
        }
        oss << delim << setw(STRLEN(DSG_COL_ERR_TIME)) << s;
    }
    if (has_count_limit()) {
        string s;
        if (gs[i].ms_count_limit || gs[i].trng_count_limit) {
            if (gs[i].ms_count_limit) {
                s += string("MS");
            }
            if (gs[i].trng_count_limit) {
                s += (gs[i].ms_count_limit ? "," : "") + string("TRNG");
            }
        }
        else {
            s = " ";
        }
        oss << delim << setw(STRLEN(DSG_COL_CNT_TIME)) << s;
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
    for (unsigned int i=0; i<gs.size(); ++i) {
        // Don't print invalid rows
        if (!gs[i].valid(fields)) continue;
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
void DatasetStatisticsGenerator::dump_parallel_aux(const string& s) {
    if(allow_dump_flag) {
        omp_set_lock(&lock);
        dump_string_to_file(outfilename, s, true);
        omp_unset_lock(&lock);
    }
}
void DatasetStatisticsGenerator::dump_line(unsigned int i) {
    dump_parallel_aux(str(i, true));
}
void DatasetStatisticsGenerator::dump_header() {
    dump_parallel_aux(header(true));
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
    if (GRAPHSTATS_TEST_MS(fields)) oss << "/MS";
    if (GRAPHSTATS_TEST_PMC(fields)) oss << "/PMCS";
    if (GRAPHSTATS_TEST_TRNG(fields)) oss << "/TRNG";
    for(unsigned int i=0; i<graphs_in_progress.size(); ++i) {
        unsigned int j = graphs_in_progress[i];
        oss << " | " << gs[j].n << "/" << gs[j].m;
        if (GRAPHSTATS_TEST_MS(fields)) {
            oss << "/" << gs[j].ms << (gs[j].ms_count_limit ? "+" : "") << (gs[j].ms_time_limit ? "t" : "");
        }
        if (GRAPHSTATS_TEST_PMC(fields)) oss << "/" << gs[j].pmcs;
        if (GRAPHSTATS_TEST_TRNG(fields)) {
            oss << "/" << gs[j].triangs << (gs[j].trng_count_limit ? "+" : "") << (gs[j].trng_time_limit ? "t" : "");
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

    // Add graph statistic object
    gs.push_back(GraphStats(graph, txt));

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
        cout << "Added graph " << gs.size() << ": '" << txt << "'" << endl;
    }

}
void DatasetStatisticsGenerator::add_random_graph(unsigned int n, double p, int instances) {
    for (auto graph_stat: GraphProducer().add_random(n, p, instances).get()) {
        add_graph(graph_stat.g, graph_stat.text);
    }
}
void DatasetStatisticsGenerator::add_graph(const string& filename, const string& text) {
    Graph g = GraphReader::read(filename);
    add_graph(g, text == "" ? filename : text);
}
void DatasetStatisticsGenerator::add_graphs(DirectoryIterator di) {
    for (auto graph_stat: GraphProducer().add_by_dir(di).get()) {
        add_graph(graph_stat.g, graph_stat.text);
    }
}
void DatasetStatisticsGenerator::add_graphs_dir(const string& dir,
                        const vector<string>& filters) {
    for (auto graph_stat: GraphProducer().add_by_dir(dir, filters).get()) {
        add_graph(graph_stat.g, graph_stat.text);
    }
}
void DatasetStatisticsGenerator::add_random_graphs(const vector<unsigned int>& n,
                        const vector<double>& p, bool mix_match) {
    for (auto graph_stat: GraphProducer().add_random(n, p, mix_match).get()) {
        add_graph(graph_stat.g, graph_stat.text);
    }
}
void DatasetStatisticsGenerator::add_random_graphs_pstep(const vector<unsigned int>& n,
                                                         double step,
                                                         int instances) {
    for (auto graph_stat: GraphProducer().add_random_pstep(n, step, instances).get()) {
        add_graph(graph_stat.g, graph_stat.text);
    }
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
    if (GRAPHSTATS_TEST_N(fields)) {
        gs[i].n = gs[i].g.getNumberOfNodes();
    }
    if (GRAPHSTATS_TEST_M(fields)) {
        gs[i].m = gs[i].g.getNumberOfEdges();
    }

    // Separators.
    // We may need the result for PMCs
    NodeSetSet min_seps;
    if (GRAPHSTATS_TEST_MS(fields)) {
        gs[i].ms = 0;
        MinimalSeparatorsEnumerator mse(gs[i].g, UNIFORM);
        t = time(NULL);
        while(mse.hasNext()) {
            min_seps.insert(mse.next());
            ++gs[i].ms;
            print_progress();
            if (has_ms_count_limit && gs[i].ms > ms_count_limit) {
                gs[i].ms_count_limit = true;
                break;
            }
            if (has_ms_time_limit && time(NULL)-t > ms_time_limit) {
                gs[i].ms_time_limit = true;
                break;
            }
            mse.next();
        }
        gs[i].ms_calc_time = time(NULL)-t;
        gs[i].ms_valid = !(gs[i].ms_time_limit || gs[i].ms_count_limit);
    }

    // PMCs.
    // The minimal separators are free, so no need to calculate
    // them separately.
    if (GRAPHSTATS_TEST_PMC(fields)) {
        PMCEnumerator pmce(gs[i].g, has_pmc_time_limit ? pmc_time_limit : 0);
        pmce.set_algorithm(pmc_alg);
        // Re-use the calculated minimal separators, if relevant
        if (!(gs[i].ms_count_limit || gs[i].ms_time_limit)) {
            pmce.set_minimal_separators(min_seps);
        }
        t = time(NULL);
        pmce.get();
        gs[i].pmc_calc_time = time(NULL) - t;
        gs[i].pmcs = pmce.get().size();
        gs[i].ms = pmce.get_ms().size(); // Redundant? Not expensive, though..
        if (pmce.is_out_of_time()) {
            gs[i].pmc_time_limit = true;
        }
        print_progress();
        gs[i].pmc_valid = !pmce.is_out_of_time();
    }

    // Triangulations
    if (GRAPHSTATS_TEST_TRNG(fields)) {
        gs[i].triangs = 0;
        MinimalTriangulationsEnumerator enumerator(gs[i].g, NONE, UNIFORM, SEPARATORS);
        t = time(NULL);
        while (enumerator.hasNext()) {
            ++gs[i].triangs;
            print_progress();
            if (has_trng_count_limit && gs[i].triangs > trng_count_limit) {
                gs[i].trng_count_limit = true;
                break;
            }
            if (has_trng_time_limit && time(NULL)-t > trng_time_limit) {
                gs[i].trng_time_limit = true;
                break;
            }
            enumerator.next();
        }
        gs[i].trng_calc_time = time(NULL)-t;
        gs[i].trng_valid = !(gs[i].trng_time_limit || gs[i].trng_count_limit);
    }

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
        if (gs[i].ms_time_limit || gs[i].trng_time_limit || gs[i].pmc_time_limit) {
            cout << "OUT OF TIME on graph ";
        }
        else if (gs[i].ms_count_limit || gs[i].trng_count_limit) {
            cout << "HIT NUMERIC LIMIT on graph ";
        }
        else {
            cout << "Done computing graph ";
        }
        cout << graphs_computed << "/" << gs.size() << ","
                  << "'" << gs[i].text << "'" << endl;

        // Unlock
        omp_unset_lock(&lock);
    }

}
void DatasetStatisticsGenerator::compute_by_graph_number_range(unsigned int first, unsigned int last, bool v) {

    // Validate input
    if (first < 1 || first > last || last > gs.size()) {
        cout << "Bad parameter: called compute_by_graph_number_range(first,last) "
             << "with first=" << first << " and last=" << last
             << ". Legal values require 1<=first<=last<=" << gs.size() << endl;
        return;
    }

    // Set the verbose member:
    verbose = v;

    // Start a new file
    dump_header();

    // Dump all data, calculate the missing data.
    // If possible, parallelize this
#pragma omp parallel for if(allow_parallel)
    for (unsigned int i=first-1; i<last; ++i) {
        if (!gs[i].valid(fields)) {
            compute(i);
        }
        dump_line(i);
    }
}
void DatasetStatisticsGenerator::compute(bool v) {
    compute_by_graph_number_range(1, gs.size(), v);
}
void DatasetStatisticsGenerator::compute_by_graph_number(unsigned int i, bool v) {
    if (i<1 || i>gs.size()) {
        cout << "Bad parameter: called compute_by_graph_number(i) with i=" << i
             << ". Legal values of i are between 1 and " << gs.size() << endl;
        return;
    }
    compute_by_graph_number_range(i,i,v);
}

vector<GraphStats> DatasetStatisticsGenerator::get_stats() const {
    return gs;
}

unsigned DatasetStatisticsGenerator::get_total_graphs() const {
    return gs.size();
}

/**
 * Stringify currently computed data and output to console in human readable
 * format.
 */
void DatasetStatisticsGenerator::print() const {
    cout << str(false);
}

}



