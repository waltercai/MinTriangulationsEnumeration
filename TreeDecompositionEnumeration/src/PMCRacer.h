#ifndef PMCRACER_H_INCLUDED
#define PMCRACER_H_INCLUDED

#include "DatasetStatisticsGenerator.h"
#include "GraphStats.h"
#include "PMCEnumerator.h"

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

    // Time limit for calculation
    bool has_time_limit;
    time_t time_limit;

    // The graph statistics for each algorithm.
    vector<GraphStats> alg_gs[PMCEnumerator::ALG_LAST];

    // Input graph statistics objects.
    // May have redundant data, but nothing too bad... we need graphs
    // and graph text.
    vector<GraphStats> gs;

    // Helper functions
    string stringify_header() const;
    string stringify_result(unsigned i) const;

public:

    // Construct the PMCR with an output filename and an optional time limit
    // per graph.
    PMCRacer(const string& out, time_t limit = 0);

    // Adds graphs to race, given the graph statistics object.
    void add(const Graph&, const string&);
    void add(const vector<GraphStats>&);

    // If calculating PMCs takes too long, stop and move on.
    void set_time_limit(time_t);
    void remove_time_limit();

    // Race! Optionally, print stuff to console
    void go(bool verbose = false);

};

}

#endif // PMCRACER_H_INCLUDED
