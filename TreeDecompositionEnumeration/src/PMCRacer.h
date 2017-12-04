#ifndef PMCRACER_H_INCLUDED
#define PMCRACER_H_INCLUDED

//#include "DatasetStatisticsGenerator.h"
#include "GraphStats.h"
#include "PMCEnumerator.h"
#include "StatisticRequest.h"
#include <map>
#include <set>
using std::map;
using std::set;

namespace tdenum {

/**
 * The purpose of this class is to compare the performance of different
 * PMC enumeration algorithms.
 *
 * Output is (by default) to a CSV file.
 *
 * The method used is a simple sequential execution: for each algorithm
 * requested, the PMCs are computed for each graph in the dataset. After
 * all graph's PMCs are computed using algorithm 1, algorithm 2 is selected
 * and the computation is repeated... etc.
 *
 * Running in parallel would be a bad idea in this case (no real way to
 * measure).
 */
class PMCRacer {
private:

    // The output file name
    string outfilename;
    bool dump_to_file_when_done;

    // The algorithms to use
    vector<PMCAlg> algs;

    // Input graph statistics objects.
    vector<GraphStats> gs;

    // Helper functions
    string stringify_header() const;
    string stringify_result(unsigned i) const;

public:

    // Construct the PMCR with an output filename and an optional time limit
    // per graph.
    PMCRacer(const string& out, bool dump = true);

    // Adds graphs to race, given the graph statistics object.
    void add(const GraphStats&);
    void add(const vector<GraphStats>&);

    // Add / remove algorithms
    void add_alg(const PMCAlg&);
    void add_algs(const vector<PMCAlg>&);
    void add_algs(const set<PMCAlg>&);
    void remove_alg(const PMCAlg&);
    void remove_algs(const vector<PMCAlg>&);
    void remove_algs(const set<PMCAlg>&);
    void clear_algs();
    void add_all_algs();

    // Race! Optionally, print stuff to console.
    // If append_results is set to true, results will be appended
    // as new rows in the CSV file.
    // The StatisticRequest object will contain the required algorithms.
    void go(const StatisticRequest& sr, bool verbose = false/*, bool append_results = false*/);

    // Return the vector of GraphStats
    vector<GraphStats> get_stats() const;

};

}

#endif // PMCRACER_H_INCLUDED
