#include "PMCEnumerator.h"
#include "PMCRacer.h"
#include "Utils.h"
#include <string>
#include <sstream>
using std::endl;
using std::string;
using std::ostringstream;

namespace tdenum {

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
        << alg_gs[0][i].pmcs;
    for (int alg = PMCEnumerator::ALG_NORMAL; alg<PMCEnumerator::ALG_LAST; ++alg) {
        if (alg_gs[alg][i].pmc_time_limit) {
            oss << ",OOT";
        }
        else {
            oss << "," << secs_to_hhmmss(alg_gs[alg][i].pmc_calc_time);
        }
    }
    oss << endl;
    return oss.str();
}

PMCRacer::PMCRacer(const string& out, time_t limit) :
    outfilename(out),
    has_time_limit(limit > 0),
    time_limit(limit),
    gs(0) {}

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

void PMCRacer::go(bool verbose) {

    // Open a new file, dump the header
    dump_string_to_file(outfilename, stringify_header());

    // For each graph:
    for (unsigned i=0; i<gs.size(); ++i) {
        if (verbose) {
            cout << "Racing graph number " << i+1 << "/" << gs.size() << ":" << endl;
        }

        // Use all algorithms on the graph
        for (int alg = PMCEnumerator::ALG_NORMAL; alg<PMCEnumerator::ALG_LAST; ++alg) {
            DatasetStatisticsGenerator dsg("DUMMY_FILE", DSG_COMP_N | DSG_COMP_M | DSG_COMP_PMC);
            dsg.dont_show_added_graphs();
            dsg.add_graph(gs[i].g,gs[i].text);
            dsg.suppress_dump();
            dsg.set_pmc_alg(PMCEnumerator::Alg(alg));
            dsg.disable_all_limits();
            if (has_time_limit) {
                dsg.set_pmc_time_limit(time_limit);
            }
            if (verbose) {
                cout << "Algorithm " << PMCEnumerator::get_alg_name(PMCEnumerator::Alg(alg)) << ".. " << endl;
            }
            dsg.compute(verbose);
            if (verbose) {
                cout << "done. ";
            }
            alg_gs[alg].push_back(dsg.get_stats()[0]);
        }
        if (verbose) {
            cout << endl;
        }

        // Output the result to file
        dump_string_to_file(outfilename, stringify_result(i), true);
        if (verbose) {
            cout << "Dumped the following string:" << endl << stringify_result(i);
        }
    }


}

}
