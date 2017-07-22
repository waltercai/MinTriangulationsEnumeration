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
    outfilename(out), has_time_limit(limit > 0), time_limit(limit),
    dsg(out+"_DSG_OUTFILE.csv", DSG_COMP_N | DSG_COMP_M | DSG_COMP_PMC) {}

void PMCRacer::set_time_limit(time_t t) {
    has_time_limit = true;
    time_limit = t; // Allow user to set zero time... dummy
}
void PMCRacer::remove_time_limit() {
    has_time_limit = false;
    time_limit = 0;
}

void PMCRacer::go(bool verbose) {

    // Fix anything the user may have broken
    dsg.suppress_dump();
    dsg.suppress_async();
    dsg.disable_all_limits();
    if (has_time_limit) {
        dsg.set_pmc_time_limit(time_limit);
    }

    // Run all algorithms
    for (int alg = PMCEnumerator::ALG_NORMAL; alg<PMCEnumerator::ALG_LAST; ++alg) {
        if (verbose) {
            cout << "Calculating PMCs using algorithm " << alg+1 << "/" << PMCEnumerator::ALG_LAST << endl;
        }
        dsg.set_pmc_alg(PMCEnumerator::Alg(alg));
        dsg.force_recalc();
        dsg.compute(verbose);
        alg_gs[alg] = dsg.get_stats();
    }

    // Output data
    ostringstream oss;
    oss << "Comparison of " << PMCEnumerator::ALG_LAST << " algorithms." << endl;
    oss << "Time limit set per graph: " << secs_to_hhmmss(time_limit) << endl;
    oss << "Graph,N,M,PMCs";
    for (int alg = PMCEnumerator::ALG_NORMAL; alg<PMCEnumerator::ALG_LAST; ++alg) {
        oss << "," << PMCEnumerator::get_alg_name(PMCEnumerator::Alg(alg));
    }
    oss << endl;
    for (unsigned i=0; i<dsg.get_total_graphs(); ++i) {
        oss.str("");
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
    }
    append_string_to_file(outfilename, oss.str());
    if (verbose) {
        cout << oss.str();
    }

}

}
