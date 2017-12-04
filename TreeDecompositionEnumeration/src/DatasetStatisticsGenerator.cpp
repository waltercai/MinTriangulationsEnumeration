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
using std::stoi;

namespace tdenum {

#define TRACE_INVALID_INDEX_STRING(_i) \
        TRACE(TRACE_LVL__ERROR, "Bad index " << _i << "! May be at most " << gs.size())
#define VALIDATE_GRAPH_INDEX_VOID_RET(_i) do { \
        if (_i > gs.size()) { \
            TRACE_INVALID_INDEX_STRING(_i); \
            return; \
        } \
    } while(0)
#define VALIDATE_GRAPH_INDEX_WITH_RET(_i,_ret) do { \
        if (_i > gs.size()) { \
            TRACE_INVALID_INDEX_STRING(_i); \
            return _ret; \
        } \
    } while(0)


/**
 * Note that we're using locks, so they need to be created and destroyed.
 *
 * Also, max_text_len needs to be at least the size of the string "Graph text", quotes included!
 */
DatasetStatisticsGenerator::DatasetStatisticsGenerator(const string& outputfile, int flds) :
                            outfilename(outputfile),
                            fields(flds),
                            has_random(false),
                            verbose(false),
                            allow_dump_flag(true),
                            allow_dsg_parallel(false),
                            allow_pmc_parallel(true),
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
                            pmc_alg(PMCAlg()),
                            graphs_computed(0),
                            max_text_len(utils__strlen(DSG_COL_STR_TXT)+2) // Allow for extra quotes
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

/**
 * Parallelization methods.
 *
 * Allowing both levels of parallelization may cause unwanted results
 * (e.g. I haven't tested it).
 */
void DatasetStatisticsGenerator::suppress_dsg_parallel() {
    allow_dsg_parallel = false;
}
void DatasetStatisticsGenerator::enable_dsg_parallel() {
    suppress_pmc_parallel();
    allow_dsg_parallel = true;
}
void DatasetStatisticsGenerator::suppress_pmc_parallel() {
    allow_pmc_parallel = false;
}
void DatasetStatisticsGenerator::enable_pmc_parallel() {
    suppress_dsg_parallel();
    allow_pmc_parallel = true;
}

void DatasetStatisticsGenerator::set_pmc_alg(const PMCAlg& alg) {
    pmc_alg = alg;
    if (alg.is_parallel()) {
        enable_pmc_parallel();
    }
}

void DatasetStatisticsGenerator::set_ms(const NodeSetSet& ms, time_t calc_time, unsigned index) {
    VALIDATE_GRAPH_INDEX_VOID_RET(index);
    --index;    // Zero-indexed now
    gs[index].set_ms(ms);
    gs[index].set_ms_calc_time(calc_time);
}
NodeSetSet DatasetStatisticsGenerator::get_ms(unsigned index) const {
    VALIDATE_GRAPH_INDEX_WITH_RET(index, NodeSetSet());
    --index;    // Zero-indexed now
    if (gs[index].valid(GRAPHSTATS_MS)) {
        return gs[index].get_ms();
    }
    TRACE(TRACE_LVL__WARNING, "Minimal separators are invalid, returning an empty container!");
    return NodeSetSet();
}

/**
 * Useful for stringifying output, in CSV format or human readable.
 */
string DatasetStatisticsGenerator::header(bool csv) const {

    ostringstream oss;
    string delim(csv ? "," : "|");

    // Header
    oss.setf(std::ios_base::left, std::ios_base::adjustfield);
    oss << setw(max_text_len) << DSG_COL_STR_TXT;
    if (GRAPHSTATS_TEST_N(fields)) {
        oss << delim << DSG_COL_STR_NODES;
    }
    if (GRAPHSTATS_TEST_M(fields)) {
        oss << delim << DSG_COL_STR_EDGES;
    }
    if (GRAPHSTATS_TEST_MS(fields)) {
        oss << delim << DSG_COL_STR_MSS;
    }
    if (GRAPHSTATS_TEST_PMC(fields)) {
        oss << delim << DSG_COL_STR_PMCS;
    }
    if (GRAPHSTATS_TEST_TRNG(fields)) {
        oss << delim << DSG_COL_STR_TRNG;
    }
    // Special columns
    if (has_random) {
        oss << delim << DSG_COL_STR_P << delim << DSG_COL_STR_RATIO;
    }
    if (GRAPHSTATS_TEST_MS(fields)) {
        oss << delim << DSG_COL_STR_MS_TIME;
    }
    if (GRAPHSTATS_TEST_PMC(fields)) {
        oss << delim << DSG_COL_STR_PMC_TIME;
    }
    if (GRAPHSTATS_TEST_TRNG(fields)) {
        oss << delim << DSG_COL_STR_TRNG_TIME;
    }
    if (has_time_limit()) {
        oss << delim << DSG_COL_STR_ERR_TIME;
    }
    if (has_count_limit()) {
        oss << delim << DSG_COL_STR_ERR_CNT;
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
    if (i>=gs.size()) {
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
        oss << "\"" << gs[i].get_text() << "\"";
    }
    else {
        oss << setw(max_text_len) << gs[i].get_text();
    }

    // Fields
    if (GRAPHSTATS_TEST_N(fields)) {
        oss << delim << setw(utils__strlen(DSG_COL_STR_NODES)) << gs[i].get_n();
    }
    if (GRAPHSTATS_TEST_M(fields)) {
        oss << delim << setw(utils__strlen(DSG_COL_STR_EDGES)) << gs[i].get_m();
    }
    if (GRAPHSTATS_TEST_MS(fields)) {
        oss << delim << setw(utils__strlen(DSG_COL_STR_MSS)) << gs[i].get_ms_count();
    }
    if (GRAPHSTATS_TEST_PMC(fields)) {
        oss << delim << setw(utils__strlen(DSG_COL_STR_PMCS)) << gs[i].get_pmc_count();
    }
    if (GRAPHSTATS_TEST_TRNG(fields)) {
        oss << delim << setw(utils__strlen(DSG_COL_STR_TRNG)) << gs[i].get_trng_count();
    }
    // Special columns
    if (has_random) {
        int N = gs[i].get_graph().getNumberOfNodes();
        int M = gs[i].get_graph().getNumberOfEdges();
        double rat = 2*M / (double(N*(N-1))); // |E|/(|V| choose 2)
        if (gs[i].get_graph().isRandom()) {
            oss << delim << setw(utils__strlen(DSG_COL_STR_P)) << gs[i].get_graph().getP();
            oss << delim << setw(utils__strlen(DSG_COL_STR_RATIO)) << rat;
        }
        else {
            oss << delim << setw(utils__strlen(DSG_COL_STR_P)) << "0";
            oss << delim << setw(utils__strlen(DSG_COL_STR_RATIO)) << rat;
        }
    }
    if (GRAPHSTATS_TEST_MS(fields)) {
        oss << delim << setw(utils__strlen(DSG_COL_STR_MS_TIME)) << utils__timestamp_to_hhmmss(gs[i].get_ms_calc_time());
    }
    if (GRAPHSTATS_TEST_PMC(fields)) {
        oss << delim << setw(utils__strlen(DSG_COL_STR_PMC_TIME)) << utils__timestamp_to_hhmmss(gs[i].get_pmc_calc_time());
    }
    if (GRAPHSTATS_TEST_TRNG(fields)) {
        oss << delim << setw(utils__strlen(DSG_COL_STR_TRNG_TIME)) << utils__timestamp_to_hhmmss(gs[i].get_trng_calc_time());
    }
    if (has_time_limit()) {
        string s;
        if (gs[i].reached_time_limit_ms() || gs[i].reached_time_limit_trng() || gs[i].reached_time_limit_pmc()) {
            if (gs[i].reached_time_limit_ms()) {
                s += string(DSG_ERROR_TEXT_MS);
            }
            if (gs[i].reached_time_limit_trng()) {
                s += (gs[i].reached_time_limit_ms() ? ";" : "") + string(DSG_ERROR_TEXT_TRNG);
            }
            if (gs[i].reached_time_limit_pmc()) {
                s += (gs[i].reached_time_limit_ms() || gs[i].reached_time_limit_trng() ? ";" : "") + string(DSG_ERROR_TEXT_PMC);
            }
        }
        else {
            s = " ";
        }
        oss << delim << setw(utils__strlen(DSG_COL_STR_ERR_TIME)) << s;
    }
    if (has_count_limit()) {
        string s;
        if (gs[i].reached_count_limit_ms() || gs[i].reached_count_limit_trng()) {
            if (gs[i].reached_count_limit_ms()) {
                s += string(DSG_ERROR_TEXT_MS);
            }
            if (gs[i].reached_count_limit_trng()) {
                s += (gs[i].reached_count_limit_ms() ? "," : "") + string(DSG_ERROR_TEXT_TRNG);
            }
        }
        else {
            s = " ";
        }
        oss << delim << setw(utils__strlen(DSG_COL_STR_ERR_CNT)) << s;
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
void DatasetStatisticsGenerator::dump_parallel_aux(const string& s, bool append) {
    if(allow_dump_flag) {
        omp_set_lock(&lock);
        utils__dump_string_to_file(outfilename, s, append);
        omp_unset_lock(&lock);
    }
}
void DatasetStatisticsGenerator::dump_line(unsigned int i) {
    dump_parallel_aux(str(i, true));
}
void DatasetStatisticsGenerator::dump_header() {
    dump_parallel_aux(header(true), false); // New file, if we're dumping the header
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
    oss << "N/M";
    if (GRAPHSTATS_TEST_MS(fields)) oss << "/MS";
    if (GRAPHSTATS_TEST_PMC(fields)) oss << "/PMCS";
    if (GRAPHSTATS_TEST_TRNG(fields)) oss << "/TRNG";
    for(unsigned int i=0; i<graphs_in_progress.size(); ++i) {
        unsigned int j = graphs_in_progress[i];
        oss << " | " << gs[j].get_n() << "/" << gs[j].get_m();
        if (GRAPHSTATS_TEST_MS(fields)) {
            oss << "/" << gs[j].get_ms_count() << (gs[j].reached_count_limit_ms() ? "+" : "") << (gs[j].reached_time_limit_ms() ? "t" : "");
        }
        if (GRAPHSTATS_TEST_PMC(fields)) oss << "/" << gs[j].get_pmc_count();
        if (GRAPHSTATS_TEST_TRNG(fields)) {
            oss << "/" << gs[j].get_trng_count() << (gs[j].reached_count_limit_trng() ? "+" : "") << (gs[j].reached_time_limit_trng() ? "t" : "");
        }
    }
    cout << utils__replace_string(oss.str());

    // Unlock
    omp_unset_lock(&lock);

}
void DatasetStatisticsGenerator::add_graph_to_progress(unsigned int i) {
    if (verbose) {
        omp_set_lock(&lock);
        cout << utils__replace_string();
        cout << utils__timestamp_to_fulldate(time(NULL)) << ": Calculating graph '" << gs[i].get_text() << "'" << endl;
        graphs_in_progress.push_back(i);
        omp_unset_lock(&lock);
        print_progress();
    }
}

/**
 * If the graph is given by a filename, GraphReader is used.
 */
void DatasetStatisticsGenerator::add_graph(const Graph& graph, const string& txt) {

    // Add graph statistic object
    gs.push_back(GraphStats(graph, txt));

    // Update max text length
    if (max_text_len < strlen(txt.c_str())+2) {
        max_text_len = strlen(txt.c_str())+2;
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
void DatasetStatisticsGenerator::add_random_graph(int n, double p, int instances) {
    for (auto graph_stat: GraphProducer().add_random(n, p, instances).get()) {
        add_graph(graph_stat.get_graph(), graph_stat.get_text());
    }
}
void DatasetStatisticsGenerator::add_graph(const string& filename, const string& text) {
    Graph g = GraphReader::read(filename);
    add_graph(g, text == "" ? filename : text);
}
void DatasetStatisticsGenerator::add_graphs(DirectoryIterator di) {
    for (auto graph_stat: GraphProducer().add_by_dir(di).get()) {
        add_graph(graph_stat.get_graph(), graph_stat.get_text());
    }
}
void DatasetStatisticsGenerator::add_graphs_dir(const string& dir,
                        const vector<string>& filters) {
    for (auto graph_stat: GraphProducer().add_by_dir(dir, filters).get()) {
        add_graph(graph_stat.get_graph(), graph_stat.get_text());
    }
}
void DatasetStatisticsGenerator::add_random_graphs(const vector<int>& n,
                        const vector<double>& p, bool mix_match) {
    for (auto graph_stat: GraphProducer().add_random(n, p, mix_match).get()) {
        add_graph(graph_stat.get_graph(), graph_stat.get_text());
    }
}
void DatasetStatisticsGenerator::add_random_graphs_pstep(const vector<int>& n,
                        double step,
                        int instances) {
    for (auto graph_stat: GraphProducer().add_random_pstep(n, step, instances).get()) {
        add_graph(graph_stat.get_graph(), graph_stat.get_text());
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
    add_graph_to_progress(i);

    // Basics
    gs[i].set_pmc_alg(pmc_alg);

    // Separators.
    // We may need the result for PMCs
    NodeSetSet min_seps;
    if (GRAPHSTATS_TEST_MS(fields) && !gs[i].ms_valid()) {
        gs[i].set_ms_count(0);
        MinimalSeparatorsEnumerator mse(gs[i].get_graph(), UNIFORM);
        t = time(NULL);
        try {
            while(mse.hasNext()) {
                min_seps.insert(mse.next());
                gs[i].set_ms_count(1+gs[i].get_ms_count());
                print_progress();
                if (has_ms_count_limit && gs[i].get_ms_count() > ms_count_limit) {
                    gs[i].set_reached_count_limit_ms();
                    break;
                }
                TRACE(TRACE_LVL__NOISE, "Time taken: " << utils__timestamp_to_hhmmss(difftime(time(NULL),t)) << ", "
                      << "limit is " << utils__timestamp_to_hhmmss(ms_time_limit));
                if (has_ms_time_limit && difftime(time(NULL),t) > ms_time_limit) {
                    gs[i].set_reached_time_limit_ms();
                    break;
                }
                mse.next();
            }
            gs[i].set_ms(min_seps);
        }
        catch(std::bad_alloc) {
            TRACE(TRACE_LVL__ERROR, "Caught std::bad_alloc while calculating separators! Setting MS 'valid' field to FALSE");
            gs[i].set_mem_error_ms();
        }
        gs[i].set_ms_calc_time(difftime(time(NULL),t));
        TRACE(TRACE_LVL__TEST, "Timestamp difference: " << gs[i].get_ms_calc_time());
        TRACE(TRACE_LVL__TEST, "Timestamp in hhmmss format: " << utils__timestamp_to_hhmmss(gs[i].get_ms_calc_time()));
        TRACE(TRACE_LVL__TEST,"SET MS COUNT TO " << gs[i].get_ms_count());
    }

    // PMCs.
    // The minimal separators are free, so no need to calculate
    // them separately.
    if (GRAPHSTATS_TEST_PMC(fields) && !gs[i].pmc_valid()) {
        PMCEnumerator pmce(gs[i].get_graph(), has_pmc_time_limit ? pmc_time_limit : 0);
        if (!allow_pmc_parallel) {
            pmc_alg.unset_parallel();
        }
        pmce.set_algorithm(pmc_alg);
        // Re-use the calculated minimal separators, if relevant
        if (!(gs[i].reached_count_limit_ms() || gs[i].reached_time_limit_ms())) {
            pmce.set_minimal_separators(min_seps);
        }
        t = time(NULL);
        try {
            TRACE(TRACE_LVL__NOISE, "Starting the PMC part...");
            pmce.get();
            gs[i].set_ms(pmce.get_ms());
            gs[i].set_pmc(pmce.get());
            if (pmce.is_out_of_time()) {
                gs[i].set_reached_time_limit_pmc();
            }
        }
        catch(std::bad_alloc) {
            TRACE(TRACE_LVL__ERROR, "Caught std::bad_alloc while calculating PMCs! Setting PMC 'valid' field to FALSE");
            gs[i].set_mem_error_pmc();
        }
        gs[i].set_pmc_calc_time(difftime(time(NULL),t));
        print_progress();
    }

    // Triangulations
    if (GRAPHSTATS_TEST_TRNG(fields) && !gs[i].trng_valid()) {
        vector<ChordalGraph> triangs;
        triangs.clear();
        gs[i].set_trng_count(0);
        MinimalTriangulationsEnumerator enumerator(gs[i].get_graph(), NONE, UNIFORM, SEPARATORS);
        t = time(NULL);
        try {
            while (enumerator.hasNext()) {
                gs[i].set_trng_count(1+gs[i].get_trng_count());
                print_progress();
                if (has_trng_count_limit && gs[i].get_trng_count() > trng_count_limit) {
                    gs[i].set_reached_count_limit_trng();
                    break;
                }
                if (has_trng_time_limit && difftime(time(NULL),t) > trng_time_limit) {
                    gs[i].set_reached_time_limit_trng();
                    break;
                }
                triangs.push_back(enumerator.next());
            }
            gs[i].set_trng(triangs);
        }
        catch(std::bad_alloc) {
            TRACE(TRACE_LVL__ERROR, "Caught std::bad_alloc while calculating triangulations! Setting triangulations 'valid' field to FALSE");
            gs[i].set_mem_error_trng();
        }
        gs[i].set_trng_calc_time(difftime(time(NULL),t));
    }

    // Printing and cleanup.
    // No need for any of this if verbose isn't true.
    if (verbose) {
        omp_set_lock(&lock);

        // Update the graphs in progress (while locked) and do some cleanup
        UTILS__REMOVE_FROM_VECTOR(graphs_in_progress, i);
        graphs_computed++;
        cout << utils__replace_string();   // Clear output

        // Get the time and output.
        // If any limits were encountered, be verbose!
        t = time(NULL);
        p = localtime(&t);
        strftime(s, 1000, "%c", p);
        cout << s << ": ";
        if (gs[i].reached_time_limit_ms() || gs[i].reached_time_limit_trng() || gs[i].reached_time_limit_pmc()) {
            cout << "OUT OF TIME on graph ";
        }
        else if (gs[i].reached_count_limit_ms() || gs[i].reached_count_limit_trng()) {
            cout << "HIT NUMERIC LIMIT on graph ";
        }
        else {
            cout << "Done computing graph ";
        }
        cout << graphs_computed << "/" << gs.size() << ","
                  << "'" << gs[i].get_text() << "'" << endl;

        // Unlock
        omp_unset_lock(&lock);
    }

}
void DatasetStatisticsGenerator::compute_by_graph_number_range(unsigned int first, unsigned int last, bool v) {

    // Validate input
    if (first < 1 || first > last || last > gs.size()) {
        TRACE(TRACE_LVL__ERROR, "Bad parameter: called compute_by_graph_number_range(first,last) "
             << "with first=" << first << " and last=" << last
             << ". Legal values require 1<=first<=last<=" << gs.size());
        return;
    }

    // Set the verbose member:
    verbose = v;

    // Start a new file
    dump_header();

    // Dump all data, calculate the missing data.
    // If possible, parallelize this
#pragma omp parallel for if(allow_dsg_parallel)
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

vector<GraphStats> DatasetStatisticsGenerator::read_stats(const string& filename) {

/**
    TODO:
    Different columns in the same row may require different stats objects -
    because different algorithms.
    It would be better to join them together somehow...
    Add optional fields in GraphStats objects to allow for different fields:
        - PMC calculation time
        - Algorithm used
        - Vector of MS set sizes for all subgraphs
    These extra vectors are ignored unless specified otherwise by method call.
    To make this simpler, make all data fields private and use setters/getters
    to make sure the correct data is read everywhere.
*/
    vector<GraphStats> data;

    // Open the file, read the header.
    ifstream file(filename);
    if (!file.good()) {
        TRACE(TRACE_LVL__ERROR, "Couldn't open file '" << filename << "'");
        return data;
    }
    string line;
    vector<string> headers;
    getline(file, line);
    utils__split(line, ',', std::back_inserter(headers));
    // First cell is useless
    headers.erase(headers.begin());

    // For every other line, add a graph stat object and populate using
    // the headers.
    while(file.good()) {
        vector<string> tokens;
        getline(file,line);
        utils__split(line, ',', std::back_inserter(tokens));
        GraphStats stats(GraphReader::read(tokens[0]), tokens[0]);
        for (unsigned int i=0; i<headers.size(); ++i) {
            double p; // Declare it here for switch scope reasons
            string token = tokens[i+1];
            switch(DSG_COL_STR_TO_INT_MAP.at(headers[i])) {
            case DSG_COL_NODES:
            case DSG_COL_EDGES:
                // Handled automatically
                break;
            case DSG_COL_MSS:
                stats.set_ms_count(stoi(token));
                break;
            case DSG_COL_PMCS:
                stats.set_pmc_count(stoi(token));
                break;
            case DSG_COL_TRNG:
                stats.set_trng_count(stoi(token));
                break;
            case DSG_COL_P:
                p = stod(token);
                // If p is greater than zero, this is a random graph.
                // Otherwise, this is a normal graph with nothing in the respective column.
                if (p >= EPSILON) {
                    stats.set_random();
                    stats.set_p(p);
                }
                break;
            case DSG_COL_RATIO:
                stats.refresh_graph_data(); // Don't read from file
                break;
            case DSG_COL_MS_TIME:
                stats.set_ms_calc_time(utils__hhmmss_to_timestamp(token));
                break;
            case DSG_COL_ERR_TIME:
                // This needs to be parsed so we can tell where the time error occurred
                if (token.find(DSG_ERROR_TEXT_MS) != string::npos) stats.set_reached_time_limit_ms();
                if (token.find(DSG_ERROR_TEXT_PMC) != string::npos) stats.set_reached_time_limit_pmc();
                if (token.find(DSG_ERROR_TEXT_TRNG) != string::npos) stats.set_reached_time_limit_trng();
                break;
            case DSG_COL_ERR_CNT:
                if (token.find(DSG_ERROR_TEXT_MS) != string::npos) stats.set_reached_count_limit_ms();
                if (token.find(DSG_ERROR_TEXT_TRNG) != string::npos) stats.set_reached_count_limit_trng();
                break;
            // Get times for different PMC algorithms
            #define X(_name, _mask) \
                case DSG_COL_##_name: \
                    stats.set_pmc_calc_time_by_alg(PMCAlg(_mask), utils__hhmmss_to_timestamp(token)); \
                    break;
            PMCALG_ALGORITHM_TABLE
            #undef X
            case DSG_COL_TXT:
            case DSG_COL_LAST:
            default:
                break;
            }
        }
        data.push_back(stats);
    }

    return data;
}

}



