#include "PMCEnumerator.h"
#include "PMCRacer.h"
#include "Utils.h"
#include <string>
#include <sstream>
using std::endl;
using std::string;
using std::ostringstream;

namespace tdenum {


PMCRacer::PMCRacer(const string& out, time_t limit) :
    outfilename(out),
    has_time_limit(limit > 0),
    time_limit(limit),
    alg_gs(PMCEnumerator::ALG_LAST),
    gs(0) {}

string PMCRacer::stringify_header() const {
    ostringstream oss;
    oss << "Comparison of " << PMCEnumerator::ALG_LAST << " algorithms." << endl;
    oss << "Time limit set per graph: " << secs_to_hhmmss(time_limit) << endl;
    oss << "Graph,N,M,PMCs";
    for (int alg = PMCEnumerator::ALG_NORMAL; alg<PMCEnumerator::ALG_LAST; ++alg) {
        oss << "," << PMCEnumerator::get_alg_name(PMCEnumerator::Alg(alg));
    }
    oss << endl;
    return oss.str();
}
string PMCRacer::stringify_result(unsigned i) const {
    ostringstream oss;
    if (i<0 || i >= alg_gs[0].size()) {
        oss << "INVALID GRAPH ID " << i << " GIVEN, must be between 0 and " << alg_gs[0].size() << endl;
        cout << oss.str();
        return oss.str();
    }
    oss << alg_gs[0][i].text << ","
        << alg_gs[0][i].n << ","
        << alg_gs[0][i].m << ","
        << alg_gs[0][i].pmc_count;
    for (int alg = PMCEnumerator::ALG_NORMAL; alg<PMCEnumerator::ALG_LAST; ++alg) {
        if (alg_gs[alg][i].pmc_calc_time > alg_gs[alg][i].pmc_time_limit) {
            oss << "," << secs_to_hhmmss(alg_gs[alg][i].get_pmc_time_limit());
        }
        else {
            oss << "," << secs_to_hhmmss(alg_gs[alg][i].pmc_calc_time);
        }
    }
    oss << endl;
    return oss.str();
}

void PMCRacer::add(const Graph& g, const string& txt) {
    gs.push_back(GraphStats(g,txt));
}
void PMCRacer::add(const vector<GraphStats>& in_stats) {
    gs.insert(gs.end(), in_stats.begin(), in_stats.end());
}


void PMCRacer::set_time_limit(time_t t) {
    has_time_limit = true;
    time_limit = t; // Allow user to set zero time... dummy
}
void PMCRacer::remove_time_limit() {
    has_time_limit = false;
    time_limit = 0;
}

void PMCRacer::go(bool verbose, bool append_results) {

    // Open a new file, dump the header
    if (!append_results) {
        dump_string_to_file(outfilename, stringify_header());
    }

    TRACE(TRACE_LVL__OFF, "Current status of alg_gs: " << alg_gs);

    // For each graph:
    for (unsigned i=0; i<gs.size(); ++i) {
        PRINT_IF(verbose, "=== Racing graph " << i+1 << "/" << gs.size()
                       << ": '" << gs[i].text << "'" << endl);
        PRINT_IF(verbose, "Start time: " << timestamp_to_fulldate(time(NULL)) << endl);

        // Use all algorithms on the graph.
        // To save time, since all algorithms require the calculation of all minimal
        // separators, start by calculating them in advance. Add the time required to
        // the total time.
        // To enforce the time limit, calculate the separators one by one.
        time_t start_time = time(NULL);
        bool time_limit_exceeded = false;
        MinimalSeparatorsEnumerator mse(gs[i].g, UNIFORM);
        NodeSetSet min_seps;
        while(mse.hasNext()) {
            min_seps.insert(mse.next());
            if (time(NULL)-start_time > time_limit) {
                time_limit_exceeded = true;
                break;
            }
        }
        // If the initial MS calculation took too long, all algorithms would be
        // too long.
        // Update the graph stats objects and continue on to the new graph.
        if (time_limit_exceeded) {
            PRINT_IF(verbose, "Out of time in initial MS calculation, moving on to the next graph." << endl);
            gs[i].set_pmc_time_limit(time_limit);
            gs[i].pmc_calc_time = time_limit+1;
            gs[i].ms = min_seps;
            for (int alg=0; alg<PMCEnumerator::ALG_LAST; ++alg) {
                alg_gs[alg].push_back(gs[i]);
            }
            continue;
        }

        // Keep calculating. Now, the remaining time can be used by each algorithm
        // separately.
        time_t ms_calc_time = time(NULL) - start_time;
        PRINT_IF(verbose, "MS calc time: " << secs_to_hhmmss(ms_calc_time) << endl);

        // Use a random order of the algorithms, in case
        // cache hits affect results.
        vector<int> algorithms(int(PMCEnumerator::ALG_LAST));
        for (unsigned j=0; j<algorithms.size(); ++j) {
            algorithms[j] = j;
        }
        std::random_shuffle(algorithms.begin(), algorithms.end());
        PRINT_IF(verbose, "Iterating over algorithms in the following order: " << algorithms << endl);

        for (unsigned alg_index=0; alg_index<algorithms.size(); ++alg_index) {
            int alg = algorithms[alg_index];
            PRINT_IF(verbose,"New iteration, alg = " << PMCEnumerator::get_alg_name(alg) << endl);
            DatasetStatisticsGenerator dsg(GRAPHSTATS_N | GRAPHSTATS_M | GRAPHSTATS_PMC);
            dsg.set_pmc_alg(PMCEnumerator::Alg(alg));
            dsg.dont_show_added_graphs();
            dsg.add_graph(gs[i].g,gs[i].text);
            dsg.disable_all_limits();
            if (has_time_limit) {
                dsg.set_pmc_time_limit(time_limit - ms_calc_time);
            }
            // Add the minimal separators
            dsg.set_ms(min_seps, ms_calc_time, 1);
            TRACE(TRACE_LVL__OFF,"Algorithm " << PMCEnumerator::get_alg_name(alg) << ".. " << endl);
            dsg.compute(verbose);
            TRACE(TRACE_LVL__OFF,"done. Getting stats..." << endl);
            // Get stats, add the time
            GraphStats stats = dsg.get_stats()[0];
            stats.set_pmc_time_limit(time_limit);   // Was initialized to time_limit - ms_calc_time by the DSG
            stats.pmc_calc_time += ms_calc_time;
            TRACE(TRACE_LVL__OFF, "Got stats, pushing into alg_gs[" << alg
                            << "], which currently contains " << alg_gs << endl);
            TRACE(TRACE_LVL__OFF, "alg_gs[" << alg << "] is " << alg_gs[alg] << endl);
            alg_gs[alg].push_back(stats);
        }

        // Output the result to file
        dump_string_to_file(outfilename, stringify_result(i), append_results);
        PRINT_IF(verbose,"Dumped string #" << i+1 << "/" << gs.size() << ":" << endl << stringify_result(i));
    }


}

}
