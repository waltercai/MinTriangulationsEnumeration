#include "Dataset.h"
#include "MinimalSeparatorsEnumerator.h"
#include "MinimalTriangulationsEnumerator.h"
#include "PMCRacer.h"
#include "StatisticRequest.h"

namespace tdenum {

#define HAS_MEM_ERR(_str) (utils__has_substr(_str, DATASET_COL_CONTENT_MEM_ERR))
#define HAS_TIME_ERR(_str) (utils__has_substr(_str, DATASET_COL_CONTENT_TIME_ERR))
string Dataset::header() const { return DATASET_CSV_HEADER; }
string Dataset::str(const GraphStats& gs, const StatisticRequest& sr) const {

    vector<string> cells(DATASET_COL_TOTAL);
    cells[DATASET_COL_NUM_TXT] = string("\"") + gs.get_text() + "\"";
    cells[DATASET_COL_NUM_NODES] = UTILS__TO_STRING(gs.get_n());
    cells[DATASET_COL_NUM_EDGES] = UTILS__TO_STRING(gs.get_m());
    cells[DATASET_COL_NUM_MSS] = (sr.test_count_ms() && gs.ms_no_errors() ? UTILS__TO_STRING(gs.get_ms_count()) : DATASET_COL_CONTENT_DATA_UNAVAILABLE);
    cells[DATASET_COL_NUM_PMCS] = (sr.test_count_pmc() && gs.pmc_no_errors() ? UTILS__TO_STRING(gs.get_pmc_count()) : DATASET_COL_CONTENT_DATA_UNAVAILABLE);
    cells[DATASET_COL_NUM_TRNG] = (sr.test_count_trng() && gs.trng_no_errors() ? UTILS__TO_STRING(gs.get_trng_count()) : DATASET_COL_CONTENT_DATA_UNAVAILABLE);
    // Special columns
    cells[DATASET_COL_NUM_P] = (gs.is_random() ? UTILS__TO_STRING(gs.get_p()) : DATASET_COL_CONTENT_DATA_UNAVAILABLE);
    int N = gs.get_n();
    int M = gs.get_m();
    cells[DATASET_COL_NUM_RATIO] = UTILS__TO_STRING(2*M / (double(N*(N-1)))); // |E|/(|V| choose 2)
    cells[DATASET_COL_NUM_MS_TIME] = (sr.test_has_ms_calculation() && gs.ms_no_errors() ?
            utils__timestamp_to_hhmmss(gs.get_ms_calc_time()) : DATASET_COL_CONTENT_DATA_UNAVAILABLE);
    cells[DATASET_COL_NUM_TRNG_TIME] = (sr.test_has_trng_calculation() && gs.trng_no_errors() ?
            utils__timestamp_to_hhmmss(gs.get_trng_calc_time()) : DATASET_COL_CONTENT_DATA_UNAVAILABLE);
    // Error reporting
    cells[DATASET_COL_NUM_ERR_TIME] =
        (sr.test_has_ms_calculation() && gs.reached_time_limit_ms() ? DATASET_COL_CONTENT_ERR_MS+" " : string("")) +
        (sr.test_has_trng_calculation() && gs.reached_time_limit_trng() ? DATASET_COL_CONTENT_ERR_TRNG : string(""));
    cells[DATASET_COL_NUM_ERR_CNT] =
        (sr.test_has_ms_calculation() && gs.reached_count_limit_ms() ? DATASET_COL_CONTENT_ERR_MS+" " : string("")) +
        (sr.test_has_trng_calculation() && gs.reached_count_limit_trng() ? DATASET_COL_CONTENT_ERR_TRNG : string(""));
    for (PMCAlg alg: PMCAlg::get_all(true)) {
        // Make sure this algorithm was used
        if (!sr.is_active_alg(alg)) {
            cells[DATASET_COL_ALG_TO_TIME_INT.at(alg)] = DATASET_COL_CONTENT_DATA_UNAVAILABLE;
            cells[DATASET_COL_ALG_TO_ERR_INT.at(alg)] = DATASET_COL_CONTENT_DATA_UNAVAILABLE;
            continue;
        }
        cells[DATASET_COL_ALG_TO_TIME_INT.at(alg)] = utils__timestamp_to_hhmmss(gs.get_pmc_calc_time(alg));
        cells[DATASET_COL_ALG_TO_ERR_INT.at(alg)] =
            (gs.reached_time_limit_pmc(alg) ? DATASET_COL_CONTENT_TIME_ERR+" " : string("")) +
            (gs.mem_error_pmc(alg) ? DATASET_COL_CONTENT_MEM_ERR : string(""));
    }

    // CSVify
    return utils__join(cells, ',');
/*OLD CODE
    ostringstream oss;
    string delim(",");
    oss.setf(std::ios_base::left, std::ios_base::adjustfield);

    // Identifying text
    // Use quotes to escape commas and such, for CSV.
    // If the user sends opening quotes this won't work but
    // danger is my middle name
    oss << "\"" << gs.get_text() << "\"";

    // Fields
    oss << delim << gs.get_n();
    oss << delim << gs.get_m();
    oss << delim << (sr.test_count_ms() ? UTILS__TO_STRING(gs.get_ms_count()) : DATASET_COL_CONTENT_DATA_UNAVAILABLE);
    oss << delim << (sr.test_count_pmc() ? UTILS__TO_STRING(gs.get_pmc_count()) : DATASET_COL_CONTENT_DATA_UNAVAILABLE);
    oss << delim << (sr.test_count_trng() ? UTILS__TO_STRING(gs.get_trng_count()) : DATASET_COL_CONTENT_DATA_UNAVAILABLE);
    // Special columns
    oss << delim << (gs.is_random() ? UTILS__TO_STRING(gs.get_p()) : DATASET_COL_CONTENT_DATA_UNAVAILABLE);
    int N = gs.get_n();
    int M = gs.get_m();
    double rat = 2*M / (double(N*(N-1))); // |E|/(|V| choose 2)
    oss << delim << rat;
    // Time for each calculation.
    // If the resulting data is invalid, report that the data is unavailable.
    oss << delim << (sr.test_has_ms_calculation() ?
            utils__timestamp_to_hhmmss(gs.get_ms_calc_time()) : DATASET_COL_CONTENT_DATA_UNAVAILABLE);
    oss << delim << (sr.test_has_trng_calculation() ?
            utils__timestamp_to_hhmmss(gs.get_trng_calc_time()) : DATASET_COL_CONTENT_DATA_UNAVAILABLE);
    // Error reporting
    oss << delim;
    if (gs.reached_time_limit_ms()) {
        oss << string(DATASET_COL_CONTENT_ERR_MS) << " ";
    }
    if (gs.reached_time_limit_trng()) {
        oss << string(DATASET_COL_CONTENT_ERR_TRNG);
    }
    oss << delim;
    if (gs.reached_count_limit_ms()) {
        oss << string(DATASET_COL_CONTENT_ERR_MS) << " ";
    }
    if (gs.reached_count_limit_trng()) {
        oss << string(DATASET_COL_CONTENT_ERR_TRNG);
    }
    // Add per-algorithm data.
    // To comply with the header() method (as of now), each algorithm consists
    // of a column representing time taken and a column for errors.
    // See Dataset.h for definition of the header string.
    // The PMCALG_ALGORITHM_TABLE defines macros Y(ID,bitmask).
    // Hacky, but it works
    vector<PMCAlg> col_algs = PMCAlg::get_all(true);
    for (unsigned i=0; i<col_algs.size(); ++i) {
        PMCAlg alg=col_algs[i];
        // Make sure this algorithm was used
        if (!sr.is_active_alg(alg)) {
            oss << delim << DATASET_COL_CONTENT_DATA_UNAVAILABLE << delim << DATASET_COL_CONTENT_DATA_UNAVAILABLE;
            continue;
        }
        oss << delim << utils__timestamp_to_hhmmss(gs.get_pmc_calc_time(alg));
        oss << delim;
        if (gs.reached_time_limit_pmc(alg)) {
            oss << DATASET_COL_CONTENT_TIME_ERR << " ";
        }
        if (gs.mem_error_pmc(alg)) {
            oss << DATASET_COL_CONTENT_MEM_ERR;
        }
    }

    // Done
    return oss.str();*/
}
string Dataset::str() const {
    string out;
    out = header();
    for (auto request: dataset) {
        out += "\n"+str(request.first, request.second);
    }
    return out;
}

const Dataset& Dataset::update_progress(const StatisticRequest& sr, GraphStats& gs) const {
    if (!verbose) {
        return *this;
    }
    ostringstream oss;
    oss << gs.get_n() << "/" << gs.get_m();
    if (sr.test_count_ms()) oss << "/" << gs.get_ms_count();
    if (sr.test_count_pmc()) oss << "/" << gs.get_pmc_count();
    if (sr.test_count_trng()) oss << "/" << gs.get_trng_count();
    cout << utils__replace_string(oss.str());
    return *this;
}
const Dataset& Dataset::end_progress_report() const {
    if (verbose) {
        cout << endl;
    }
    return *this;
}

int Dataset::graph_index_by_text(const vector<GraphStats>& vgs, const string& txt) {
    for (unsigned i=0; i<vgs.size(); ++i) {
        if (txt == vgs[i].get_text()) {
            return i;
        }
    }
    return DATASET_INVALID_INDEX;
}
int Dataset::graph_index_by_text(const string& txt) const { return graph_index_by_text(get_vector_gs(), txt); }

bool Dataset::file_exists() const { return utils__file_exists(file_path); }
bool Dataset::unique_graph_text(const vector<GraphStats>& vgs) {
    for (unsigned i=0; i<vgs.size(); ++i) {
        for (unsigned j=i+1; j<vgs.size(); ++j) {
            if (utils__str_eq(vgs[i].get_text(), vgs[j].get_text())) {
                return false;
            }
        }
    }
    return true;
}
bool Dataset::unique_graph_text() const { return unique_graph_text(get_vector_gs()); }

Dataset& Dataset::clear() {
    dataset.clear();
    return *this;
}

Dataset& Dataset::load() {
    // Start by clearing the existing dataset
    dataset.clear();
    // Sanity
    if (!file_exists()) {
        return *this;
    }
    auto cells = utils__read_csv(file_path);
    // Expect a header row, and then the filename in column DATASET_COL_NUM_TXT
    for (unsigned i=DATASET_HEADER_ROWS; i<cells.size(); ++i) {
        if (DATASET_COL_NUM_TXT >= cells[i].size()) {
            TRACE(TRACE_LVL__ERROR, "Broken file '" << file_path << "', no text column found in row number " << i);
            continue;
        }
        string graph_path = utils__strip_char(cells[i][DATASET_COL_NUM_TXT], '"');
        GraphStats gs = GraphStats::read(graph_path);
        if (gs.integrity(graph_path)) {   // If read() was successful
            dataset.push_back(GraphReq(gs,StatisticRequest()));
        }
        else {
            TRACE(TRACE_LVL__ERROR, "Tried reading graph '" << graph_path << "', read() failed");
        }
    }
    return *this;
}

#define CALC_SANITY(calc_type) do { \
        if (!sr.test_has_##calc_type()) { \
            TRACE(TRACE_LVL__NOISE, "Nothing to calculate..."); \
            return *this; \
        } \
    } while(0)
Dataset& Dataset::calc_ms(GraphStats& gs, const StatisticRequest& sr) {

    CALC_SANITY(ms_calculation);

    TRACE(TRACE_LVL__TEST, "In with graph=\n" << gs.get_graph());

    // Init
    NodeSetSet min_seps;
    MinimalSeparatorsEnumerator mse(gs.get_graph(), UNIFORM);
    time_t t = time(NULL);
    min_seps.clear();
    long ms_count = 0;
    gs.set_ms_count(ms_count);
    gs.set_ms(NodeSetSet());

    // Catch memory errors
    try {
        TRACE(TRACE_LVL__TEST, "In try\{} block with the following graph:\n" << gs.get_graph());
        time_t start_time = time(NULL);
        while(mse.hasNext()) {
            // No need to store the separators if they weren't requested.
            // We do need to advance the MS enumerator, though.
            auto next = mse.next();
            ++ms_count;
            time_t time_taken = difftime(time(NULL), start_time);

            // Set count AFTER setting (possible) separators, as the above updates the count automatically.
            if (sr.test_ms()) {
                gs.add_sep(next);
            }
            gs.set_ms_count(ms_count);

            // Set time
            gs.set_ms_calc_time(time_taken);

            // Update progress
            update_progress(sr,gs);
            // Limit tests
            if (sr.test_count_limit_ms() && gs.get_ms_count() > sr.get_count_limit_ms()) {
                TRACE(TRACE_LVL__WARNING, "Reached count limit in MS calculation for graph '"
                            << gs.get_text() << "' (counted to "
                            << gs.get_ms_count() << ", limited to "
                            << sr.get_count_limit_ms() << ")");
                gs.set_reached_count_limit_ms();
                break;
            }
            if (sr.test_time_limit_ms() && gs.get_ms_calc_time() > sr.get_time_limit_ms()) {
                TRACE(TRACE_LVL__WARNING, "Reached time limit in MS calculation for graph '"
                            << gs.get_text() << "' (used "
                            << gs.get_ms_calc_time() << " seconds, limited to "
                            << sr.get_time_limit_ms() << ")");
                gs.set_reached_time_limit_ms();
                break;
            }
        }
    }
    catch(std::bad_alloc) {
        TRACE(TRACE_LVL__ERROR, "Caught std::bad_alloc while calculating separators! Setting MS 'valid' field to FALSE");
        gs.set_mem_error_ms();
    }
    // That's it.
    end_progress_report();
    gs.set_ms_calc_time(difftime(time(NULL),t));// If there are no separators, this is never updated
    TRACE(TRACE_LVL__TEST, "Timestamp difference: " << gs.get_ms_calc_time());
    TRACE(TRACE_LVL__TEST, "Timestamp in hhmmss format: " << utils__timestamp_to_hhmmss(gs.get_ms_calc_time()));
    TRACE(TRACE_LVL__TEST,"SET MS COUNT TO " << gs.get_ms_count());
    return *this;
}
Dataset& Dataset::calc_pmc(GraphStats& gs, const StatisticRequest& sr) {

    CALC_SANITY(pmc_calculation);

    PMCRacer pmcr("tmp", false);
    pmcr.add(gs);
    TRACE(TRACE_LVL__TEST, "In");
    pmcr.go(sr, TRACE_LVL >= TRACE_LVL__WARNING);
    TRACE(TRACE_LVL__TEST, "Done with race!");

    GraphStats tmp_gs = pmcr.get_stats()[0];
    for (PMCAlg alg: sr.get_active_pmc_algs()) {
        // If the total equals the limit, we can't really tell what happened...
        if (sr.test_time_limit_pmc() && tmp_gs.get_pmc_calc_time(alg) == sr.get_time_limit_pmc()) {
            continue;
        }
        // If we're over the limit and the PMCRacer didn't detect it, it's bad!
        if (sr.test_time_limit_pmc() && (tmp_gs.get_pmc_calc_time(alg) > sr.get_time_limit_pmc()) != tmp_gs.reached_time_limit_pmc(alg)) {
            TRACE(TRACE_LVL__ERROR, "The GS objects says '" << (tmp_gs.reached_time_limit_pmc(alg) ? "" : "NOT ") << "OOT' "
                                    << "running algorithm " << alg.str() << ", when in reality: "
                                    << tmp_gs.get_pmc_calc_time(alg) << " seconds required, "
                                    << sr.get_time_limit_pmc() << " allowed!");
        }
    }

    if (pmcr.get_stats().size() != 1) {
        TRACE(TRACE_LVL__ERROR, "PMCRacer returned statistics for "
              << pmcr.get_stats().size() << " graphs instead of one!");
        return *this;
    }
    GraphStats out_gs = pmcr.get_stats()[0];
    TRACE(TRACE_LVL__TEST, "Looping over algs...");
    for (auto alg: sr.get_active_pmc_algs()) {
        if (out_gs.pmc_no_errors(alg)) {
            TRACE(TRACE_LVL__TEST, "No errors");
            // Set the ms-count anyway
            TRACE(TRACE_LVL__TEST, "Setting MS-count to " << out_gs.get_ms_count());
            gs.set_ms_count(out_gs.get_ms_count());
            if (sr.test_pmc()) {
                TRACE(TRACE_LVL__TEST, "Setting PMCs to " << out_gs.get_pmc());
                gs.set_pmc(out_gs.get_pmc());
            }
            if (sr.test_count_pmc()) {
                TRACE(TRACE_LVL__TEST, "Setting PMC count to " << out_gs.get_pmc_count());
                gs.set_pmc_count(out_gs.get_pmc_count());
            }
            // This should also be calculated only once:
            if (sr.test_ms_subgraphs()) {
                TRACE(TRACE_LVL__TEST, "Setting MS subgraphs to " << out_gs.get_ms_subgraphs());
                gs.set_ms_subgraphs(out_gs.get_ms_subgraphs());
            }
            if (sr.test_ms_subgraph_count()) {
                TRACE(TRACE_LVL__TEST, "Setting MS subgraphs to " << out_gs.get_ms_subgraphs());
                gs.set_ms_subgraph_count(out_gs.get_ms_subgraph_count());
            }
            gs.set_pmc_calc_time(alg, out_gs.get_pmc_calc_time(alg));
        }
        else {
            // Some error occurred
            if (out_gs.reached_time_limit_pmc(alg)) {
                gs.set_reached_time_limit_pmc(alg);
            }
            if (out_gs.mem_error_pmc(alg)) {
                gs.set_mem_error_pmc(alg);
            }
        }
    }
    // If we're here, the calculated data is valid
    return *this;
}
Dataset& Dataset::calc_trng(GraphStats& gs, const StatisticRequest& sr) {

    CALC_SANITY(trng_calculation);

    // Init
    time_t start_time = time(NULL);
    MinimalTriangulationsEnumerator mte(gs.get_graph(), NONE, UNIFORM, MCS_M);
    long trng_count = 0;
    gs.set_trng_count(trng_count);
    gs.set_trng(vector<ChordalGraph>());

    // Catch memory errors
    try {
        while (mte.hasNext()) {
            // Calculate
            ++trng_count;
            ChordalGraph triangulation = mte.next();
            time_t time_taken = difftime(time(NULL), start_time);
            // Update fields
            gs.set_trng_calc_time(time_taken);
            if (sr.test_trng()) {
                gs.add_trng(triangulation);
            }
            // Set count AFTER setting (possible) triangulations, as the above updates the count automatically.
            gs.set_trng_count(trng_count);
            TRACE(TRACE_LVL__TEST, "Set trng count to " << trng_count);
            // Check for problems
            if (sr.test_count_limit_trng() && sr.get_count_limit_trng() < trng_count) {
                TRACE(TRACE_LVL__WARNING, "Reached count limit in TRNG calculation for graph '"
                                << gs.get_text() << "' (counted to "
                                << trng_count << ", limited to "
                                << sr.get_count_limit_trng() << ")");
                gs.set_reached_count_limit_trng();
                return *this;
            }
            if (sr.test_time_limit_trng() && sr.get_time_limit_trng() < time_taken) {
                TRACE(TRACE_LVL__WARNING, "Reached time limit in TRNG calculation for graph '"
                                << gs.get_text() << "' (took "
                                << time_taken << " seconds, limited to "
                                << sr.get_time_limit_trng() << ")");
                gs.set_reached_time_limit_trng();
                return *this;
            }
        }
    }
    catch (std::bad_alloc) {
        TRACE(TRACE_LVL__ERROR, "Memory error when calculating triangulations!");
        gs.set_mem_error_trng();
        return *this;
    }
    // If we're here, the calculated data is valid
    TRACE(TRACE_LVL__TEST, "Done, trng count is finally " << trng_count);
	return *this;
}
Dataset& Dataset::calc() {
    // Calculate!
    // Use the .second field of the request to send the StatReq object.
    // Remember to use references, or it won't write the data!
    for (GraphReq& request: dataset) { // pair<GraphStats,StatisticRequest>
        calc_ms(request.first, request.second);
        calc_pmc(request.first, request.second);
        calc_trng(request.first, request.second);
    }
    return *this;
}

bool Dataset::dump_barren_stat_file(const string& target_path,
                                    const vector<GraphStats>& vgs,
                                    bool force)
{

    // Sanity
    if (!unique_graph_text(vgs)) {
        TRACE(TRACE_LVL__ERROR, "Non-unique filenames detected! Dumping vector:" << endl << vgs);
        return false;
    }

    // If a file exists, just check integrity of each file.
    if (!force && utils__file_exists(target_path)) {
        Dataset ds(target_path);
        auto vgs_in = ds.get_vector_gs();
        // Size test
        if (vgs.size() != vgs_in.size()) {
            TRACE(TRACE_LVL__ERROR, "Input file '" << target_path << "' has the wrong number of graphs!" <<
                                    " Expected " << vgs.size() << " and got " << vgs_in.size() << " instead");
            return false;
        }
        for (auto gs: vgs_in) {
            int index = graph_index_by_text(vgs, gs.get_text());
            if (index == -1) {
                TRACE(TRACE_LVL__ERROR, "Existing file contains graph '" << gs.get_text() << "', " <<
                                        "no such graph name found in GS vector:" << endl << vgs);
                return false;
            }
            if (!vgs[index].integrity(gs.get_text())) {
                TRACE(TRACE_LVL__ERROR, "Graph '" << gs.get_text() << "' from file isn't equal to the given graph!" << endl <<
                                        "On file:" << endl << gs.get_graph() << endl <<
                                        "In vector:" << endl << vgs[index].get_graph());
                return false;
            }
        }
        // Integrity!
        return true;
    }
    else {
        // Otherwise, just dump it.
        // Abuse the private access and use dump().
        // Also note that if an existing file is loaded, we need to clear the Dataset first!
        Dataset ds = Dataset(target_path);
        ds.clear();
        for (auto gs: vgs) {
            ds.dataset.push_back(GraphReq(gs,StatisticRequest()));
        }
        return ds.dump();
    }
}

Dataset::Dataset(const string& path) :
                file_path(path),
                verbose(true)
{
    load();
    if (!unique_graph_text()) {
        TRACE(TRACE_LVL__ERROR, "Graph file paths aren't unique in '" << file_path << "'");
    }
}
Dataset::Dataset(const string& path, const vector<string>& graph_paths) :
                file_path(path),
                verbose(true)
{
    for (unsigned i=0; i<graph_paths.size(); ++i) {
        // Make sure the text is unique
        bool is_unique = true;
        for (unsigned j=0; j<dataset.size(); ++j) {
            if (utils__str_eq(graph_paths[i], dataset[j].first.get_text())) {
                TRACE(TRACE_LVL__ERROR, "Duplicate graph text '" << graph_paths[i] << "', ignoring...");
                is_unique = false;
                break;
            }
        }
        if (!is_unique) {
            continue;
        }
        // It is! Read it from file and validate input
        GraphStats gs = GraphStats::read(graph_paths[i]);
        if (!gs.integrity(graph_paths[i])) {
            TRACE(TRACE_LVL__ERROR, "Graph '" << graph_paths[i] << "' is invalid (read error)!"
                                    " Dumping resulting GS instance:" << endl << gs);
            continue;
        }
        // Done. Add it
        dataset.push_back(GraphReq(gs,StatisticRequest()));
    }

}

Dataset& Dataset::reset(const string& path) { return (*this = Dataset(path)); }
Dataset& Dataset::reset(const string& path, const vector<string>& graph_paths) { return (*this = Dataset(path, graph_paths)); }

Dataset& Dataset::set_request(unsigned index, const StatisticRequest& sr) {
    if (index < 0 || index >= dataset.size()) {
        TRACE(TRACE_LVL__ERROR,"Invalid index " << index << " sent, must be between 0 and " << dataset.size()-1);
        return *this;
    }
    dataset[index].second = sr;
    return *this;
}
Dataset& Dataset::set_all_requests(const StatisticRequest& sr) {
    for (unsigned i=0; i<dataset.size(); ++i) {
        set_request(i, sr);
    }
    return *this;
}
Dataset& Dataset::set_verbose() { verbose = true; return *this; }
Dataset& Dataset::unset_verbose() { verbose = false; return *this; }

string Dataset::get_file_path() const { return file_path; }

vector<GraphStats> Dataset::get_vector_gs() const {
    vector<GraphStats> vgs;
    std::transform(dataset.begin(),
                   dataset.end(),
                   std::back_inserter(vgs),
                   [](const GraphReq& p) { return p.first; });
    return vgs;
}
vector<StatisticRequest> Dataset::get_vector_sr() const {
    vector<StatisticRequest> vsr;
    std::transform(dataset.begin(),
                   dataset.end(),
                   std::back_inserter(vsr),
                   [](const GraphReq& p) { return p.second; });
    return vsr;
}

Dataset& Dataset::load_stats() {
    // Read the CSV file
    auto table = utils__read_csv(file_path);

    // Iterate over all lines except the header
    for (unsigned i=DATASET_HEADER_ROWS; i<table.size(); ++i) {
        vector<string> row = table[i];

        // Sanity
        if (row.size() < DATASET_COL_TOTAL) {
            TRACE(TRACE_LVL__ERROR, "Expected " << DATASET_COL_TOTAL << " columns, got " << row.size() << " in row " << i+1);
            continue;
        }

        // Find the graph
        string txt_col = row[DATASET_COL_NUM_TXT];
        int index = graph_index_by_text(txt_col);
        if (index == -1) {  // Only read valid graphs! load() should already have loaded this
            TRACE(TRACE_LVL__ERROR, "Graph '" << txt_col << "' can't be found! No stats loaded");
            continue;
        }

        // Found it! Use references for ease of editing
        GraphStats& gs = dataset[index].first;
        StatisticRequest& sr = dataset[index].second;

        // Now, update the stats, and the request (any updated data need no longer be requested).
        // Counters:
        if (sr.test_count_ms() && row[DATASET_COL_NUM_MSS] != DATASET_COL_CONTENT_DATA_UNAVAILABLE) {
            gs.set_ms_count(stoi(row[DATASET_COL_NUM_MSS]));
            sr.unset_count_ms();
        }
        if (sr.test_count_pmc() && row[DATASET_COL_NUM_PMCS] != DATASET_COL_CONTENT_DATA_UNAVAILABLE) {
            gs.set_pmc_count(stoi(row[DATASET_COL_NUM_PMCS]));
            sr.unset_count_pmc();
        }
        if (sr.test_count_trng() && row[DATASET_COL_NUM_TRNG] != DATASET_COL_CONTENT_DATA_UNAVAILABLE) {
            gs.set_trng_count(stoi(row[DATASET_COL_NUM_TRNG]));
            sr.unset_count_trng();
        }
        // P value (ratio is calculated via graph):
        if (row[DATASET_COL_NUM_P] != DATASET_COL_CONTENT_DATA_UNAVAILABLE) {
            gs.set_p(stod(row[DATASET_COL_NUM_P]));
        }
        // Time taken for MSs or triangulations:
        if (row[DATASET_COL_NUM_MS_TIME] != DATASET_COL_CONTENT_DATA_UNAVAILABLE) {
            gs.set_ms_calc_time(utils__hhmmss_to_timestamp(row[DATASET_COL_NUM_MS_TIME]));
        }
        if (row[DATASET_COL_NUM_TRNG_TIME] != DATASET_COL_CONTENT_DATA_UNAVAILABLE) {
            gs.set_trng_calc_time(utils__hhmmss_to_timestamp(row[DATASET_COL_NUM_TRNG_TIME]));
        }
        // Algorithm specifics:
        for (PMCAlg alg: PMCAlg::get_all(true)) {
            string time_cell = row[DATASET_COL_ALG_TO_TIME_INT.at(alg)];
            string err_cell = row[DATASET_COL_ALG_TO_ERR_INT.at(alg)];
            if (sr.is_active_alg(alg) && time_cell != DATASET_COL_CONTENT_DATA_UNAVAILABLE) {
                gs.set_pmc_calc_time(alg,utils__hhmmss_to_timestamp(time_cell));
                if (HAS_TIME_ERR(err_cell)) {
                    gs.set_reached_time_limit_pmc(alg);
                }
                if (HAS_MEM_ERR(err_cell)) {
                    gs.set_mem_error_pmc(alg);
                }
                sr.remove_alg_from_pmc_race(alg);
            }
        }
    }

    // Return
    return *this;
}
bool Dataset::dump() const {
    if (!utils__dump_string_to_file(str(), file_path)) {
        TRACE(TRACE_LVL__ERROR, "Couldn't dump data to '" << file_path << "'");
        return false;
    }
    return true;
}

bool Dataset::operator==(const Dataset& ds) const { return dataset == ds.dataset; }

ostream& operator<<(ostream& os, const Dataset& ds) { return (os << ds.dataset); }

}
