#ifndef DATASET_H_INCLUDED
#define DATASET_H_INCLUDED

//#include "DatasetStatisticsGenerator.h"
#include "DirectoryIterator.h"
#include "Graph.h"
#include "GraphProducer.h"
#include "GraphStats.h"
#include "PMCAlg.h"
#include "PMCEnumeratorTester.h"
#include "PMCRacer.h"
#include "StatisticRequest.h"
#include "Utils.h"
using std::pair;

namespace tdenum {

/**
 * This class is used to calculate graph statistics in bulk.
 *
 * Allows the user to load several graphs from file, re-use pre-calculated statistics
 * and calculated per-graph statistics to file.
 *
 * Requires use of StatisticRequest objects and GraphStats objects.
 *
 * See constructor(s) for usage.
 *
 * The dumped CSV file should contain columns from DATASET_COL_TABLE, in order defined.
 * Note that there isn't necessarily one column per column entry in the table. See examples
 * or implementation for details.
 */
// The number of rows of the CSV file the header requires
#define DATASET_HEADER_ROWS (1)
// Invalid graph index
#define DATASET_INVALID_INDEX (-1)

/**
 * For examples on usage of the macros defined bellow, take a look at the str() method,
 * or this class's testing class DatasetTester
 */
// The possible columns appearing in the output CSV table.
// Note that PMC algorithms also have columns (calculation time per algorithm)
#define DATASET_COL_TABLE \
    X(TXT, "Graph text") \
    X(NODES, "Nodes ") \
    X(EDGES, "Edges ") \
    X(MSS, "Minimal separators") \
    X(PMCS, "PMCs    ") \
    X(TRNG, "Minimal triangulations") \
    X(P, "P value") \
    X(RATIO, "Edge ratio") \
    X(MS_TIME, "MS calculation time") \
    X(TRNG_TIME, "Triangulation calculation time") \
    X(ERR_TIME, "Time errors") \
    X(ERR_CNT, "Count errors") \
    PMCALG_ALGORITHM_TABLE // Y macros Y(ID,bitmask)

// Define the columns as constants DATASET_COL_XXX.
typedef enum _dataset_columns {
#define X(ID,_) DATASET_COL_NUM_##ID,
#define Y(ID) DATASET_COL_NUM_TIME_##ID, \
                DATASET_COL_NUM_ERR_##ID,
    DATASET_COL_TABLE
#undef X
#undef Y
    DATASET_COL_TOTAL
} DatasetColumnIndex;

// Total non-PMCAlg related columns
#define X(...) +1
#define Y(...)
const int DATASET_COL_TOTAL_NONPMCALG = 0 /*+1+1+1...*/ DATASET_COL_TABLE;
#undef X
#undef Y

// Define the column text for each column. As before, define extra columns for
// PMC algorithms.
#define X(ID,_str) const string DATASET_COL_STR_##ID = UTILS__TO_STRING(_str);
#define Y(ID) const string DATASET_COL_STR_##ID = PMCALG_NUM_TO_STR_MAP.at(PMCALG_ENUM_##ID) + "_TIME"; \
                   const string DATASET_COL_STR_ERR_##ID = PMCALG_NUM_TO_STR_MAP.at(PMCALG_ENUM_##ID) + "_ERRORS";
DATASET_COL_TABLE
#undef X
#undef Y

// Define maps between the text and the enum constants. As before, define extra
// columns for PMC algorithms.
const map<string, DatasetColumnIndex> DATASET_COL_STR_TO_INT_MAP {
#define X(ID,_) {DATASET_COL_STR_##ID, DATASET_COL_NUM_##ID},
#define Y(ID) {DATASET_COL_STR_##ID, DATASET_COL_NUM_TIME_##ID}, \
                   {DATASET_COL_STR_ERR_##ID, DATASET_COL_NUM_ERR_##ID},
    DATASET_COL_TABLE
#undef X
#undef Y
};
const map<DatasetColumnIndex, string> DATASET_COL_INT_TO_STR_MAP {
#define X(ID,_) {DATASET_COL_NUM_##ID, DATASET_COL_STR_##ID},
#define Y(ID) {DATASET_COL_NUM_TIME_##ID, DATASET_COL_STR_##ID}, \
                   {DATASET_COL_NUM_ERR_##ID, DATASET_COL_STR_ERR_##ID},
    DATASET_COL_TABLE
#undef X
#undef Y
};

// Map algorithms to indexes of the columns displaying the time/errors for the algorithm
const map<PMCAlg,int> DATASET_COL_ALG_TO_TIME_INT {
#define Y(ID) {PMCALG_ENUM_##ID, DATASET_COL_NUM_TIME_##ID},
    PMCALG_ALGORITHM_TABLE
#undef Y
};
const map<PMCAlg,int> DATASET_COL_ALG_TO_ERR_INT {
#define Y(ID) {PMCALG_ENUM_##ID, DATASET_COL_NUM_ERR_##ID},
    PMCALG_ALGORITHM_TABLE
#undef Y
};

// Define a stringified header for output CSV files
#define X(ID,_) (DATASET_COL_STR_##ID + ",") +
#define Y(ID) (DATASET_COL_STR_##ID + "," + \
                    DATASET_COL_STR_ERR_##ID + ",") +
// Erase the extra comma at the end
const string DATASET_CSV_HEADER = string(DATASET_COL_TABLE/*+*/"").erase(utils__strlen(DATASET_COL_TABLE "")-1);
#undef X
#undef Y

// Some basic text
#define DATASET_COL_CONTENT_ERR_MS (string("MSs"))
#define DATASET_COL_CONTENT_ERR_PMC (string("PMCs"))
#define DATASET_COL_CONTENT_ERR_TRNG (string("Triangulations"))

#define DATASET_COL_CONTENT_TIME_ERR (string("Out of time"))
#define DATASET_COL_CONTENT_MEM_ERR (string("Memory error"))

#define DATASET_COL_CONTENT_DATA_UNAVAILABLE (string("Data unavailable"))

typedef pair<GraphStats,StatisticRequest> GraphReq;

class Dataset {
private:

    // Testing class
    friend class DatasetTester;

    // Internals
    vector<GraphReq> dataset; // The dataset itself
    string file_path;                                   // Path to file containing statistics / dump target
    bool verbose;                                       // If true, progress reports will be displayed

    // These print out CSV strings for output.
    // An entry will use the StatisticRequest object to determine which
    // fields have valid (printable) values, and which don't
    string header() const;
    string str(const GraphStats& gs, const StatisticRequest& sr) const;
    string str() const;

    // Used for mid-calculation progress reports.
    const Dataset& update_progress(const StatisticRequest& sr, GraphStats& gs) const;
    const Dataset& end_progress_report() const;

    // Given graph text, find the index in the dataset with a graph with the
    // same text.
    // If no such graph exists, -1 is returned.
    int graph_index_by_text(const string& txt) const;

    // Returns true <==> the path points to an existing file
    bool file_exists() const;

    // Returns true <==> all GraphStats object in the dataset have unique file paths
    bool unique_graph_text() const;

    // Clears the Dataset. DOES NOT touch the file path
    Dataset& clear();

    // Reads the given file path & searches for all Graph objects that exist
    // on file.
    // All existing graphs are read into the dataset field (with default
    // StatisticRequests)
    Dataset& load();

    // Updates (upon request):
    //  - Memory errors
    //  - Time errors
    //  - Count errors
    //  - MSs
    //  - Calculation time
    //  - Total MSs
    // If both test_calc_ms() and test_calc_count_ms() return false, nothing
    // is calculated!
    Dataset& calc_ms(GraphStats& gs, const StatisticRequest& sr);

    // Updates (upon request):
    //  - PMCs
    //  - PMC count
    //  - MSs
    //  - MS count
    //  - MS count for all subgraphs
    //  - MS for all subgraphs
    //  - PMC calculation times per algorithm set
    //  - Memory and time errors for each algorithm set
    Dataset& calc_pmc(GraphStats& gs, const StatisticRequest& sr);

    // Updates (upon request):
    // - Triangulations
    // - Triangulation count
    // - Time + count + memory errors for the above
    Dataset& calc_trng(GraphStats& gs, const StatisticRequest& sr);

public:

    // Public static versions of methods described above
    static int graph_index_by_text(const vector<GraphStats>& vgs, const string& txt);
    static bool unique_graph_text(const vector<GraphStats>&);

    // Given a GraphStats vector, stupidly dump a basic statistics file.
    // Using the basic file, load() should be able to re-read all GraphStats
    // and read the graphs from the paths in the text column.
    // If a previous version of the file exists, the file will not be dumped
    // unless the _force_ flag is set.
    // Returns true if there now exists a file by the name of file_path, where
    // calling load() produces the same graphs (not necessarily the same
    // statistics!) as the input GS vector.
    static bool dump_barren_stat_file(const string& target_path,
                                      const vector<GraphStats>& vgs,
                                      bool force = false);

    // Prefer to construct the class via loading an existing file.
    // To do so:
    // 1. Use the constructor with only a file path string as it's input (may be used
    //    with the output of dump_barren_stat_file()).
    //    The constructor will load all GraphStats objects into the internal fields.
    //    If an invalid graph text field is found (or the file is unreadable), the specific
    //    graph won't be loaded into the dataset.
    // 2. Construct a StatisticRequest for each graph in the dataset, and set it per graph
    //    using set_request(), or use the same request for all graphs with set_all_requests().
    // 3. Call load_stats() to re-read the input file for statistics
    //    already calculated.
    // 4. Call calc() to calculate all remaining requested data.
    // Every constructor initializes the StatisticRequest objects to their default value.
    Dataset(const string& path);

    // If constructed with paths to GraphStats files as input, NO STATISTICS FILE IS READ.
    // Instead, the 'path' string will be used for output only!
    // Every GS object that isn't found on disk or fails to load is discarded, so remember to
    // dump the GS instances first using the GraphProducer or GraphStats classes!
    // If the vector of graph paths isn't unique, duplicates will be discarded.
    Dataset(const string& path, const vector<string>& graph_paths);

    // Reset the Dataset (constructs a new one with the given input)
    Dataset& reset(const string& path);
    Dataset& reset(const string& path, const vector<string>& graphs);

    // Described above.
    // Sets the StatisticRequests for the dataset.
    Dataset& set_request(unsigned index, const StatisticRequest& sr);
    Dataset& set_all_requests(const StatisticRequest& sr);

    // Sets verbose output on/off
    Dataset& set_verbose();
    Dataset& unset_verbose();

    // Get the output path
    string get_file_path() const;

    // Get the GraphStats vector or the StatisticRequest vector
    vector<GraphStats> get_vector_gs() const;
    vector<StatisticRequest> get_vector_sr() const;

    // Described above.
    // After updating StatisticRequests using the above method(s), call this
    // to re-read the given statistics file.
    // If no file existed when the Dataset instance was constructed (or it was
    // invalid), this does nothing.
    Dataset& load_stats();

    // The main calculating method.
    // After finished calling the above methods, call this before dumping.
    Dataset& calc();

    // Dumps the statistics file.
    // The number and order of columns in all output statistic files should
    // be the same (if the same version of the Dataset class was used for
    // output).
    // Returns true on successful dump.
    bool dump() const;

    // Ignores the verbosity and file path, takes GraphStats and StatisticRequests into account
    bool operator==(const Dataset&) const;

    // Simple output
    friend ostream& operator<<(ostream&, const Dataset&);

};

}

#endif // DATASET_H_INCLUDED
