#include "MinimalSeparatorsEnumerator.h"
#include "PMCEnumerator.h"
#include "PMCRacer.h"
#include "Utils.h"
#include <string>
#include <sstream>
using std::endl;
using std::string;
using std::ostringstream;

namespace tdenum {


PMCRacer::PMCRacer(const string& out, bool dump) :
    outfilename(out),
    dump_to_file_when_done(dump),
    debug(false),
    gs(0)
    {}

//void PMCRacer::add(const Graph& g, const string& txt) { gs.push_back(GraphStats(g,txt)); }
void PMCRacer::add(const GraphStats& in_stats) { gs.push_back(in_stats); }
void PMCRacer::add(const vector<GraphStats>& in_stats) { for (auto g: in_stats) { add(g); } }

vector<GraphStats> PMCRacer::get_stats() const { return gs; }
/*
void PMCRacer::add_alg(const PMCAlg& a) { utils__push_back_unique(a, algs); }
void PMCRacer::add_algs(const vector<PMCAlg>& va) { for (PMCAlg a: va) add_alg(a); }
void PMCRacer::add_algs(const set<PMCAlg>& sa) { for (PMCAlg a: sa) add_alg(a); }
void PMCRacer::remove_alg(const PMCAlg& a) { utils__remove_from_vector(a, algs); }
void PMCRacer::remove_algs(const vector<PMCAlg>& va) { for (PMCAlg a: va) remove_alg(a); }
void PMCRacer::remove_algs(const set<PMCAlg>& sa) { for (PMCAlg a: sa) remove_alg(a); }
void PMCRacer::clear_algs() { algs.clear(); }
void PMCRacer::add_all_algs() { algs = PMCAlg::get_all(); }
*/
void PMCRacer::set_debug() { debug = true; }
void PMCRacer::unset_debug() { debug = false; }

bool PMCRacer::go(const StatisticRequest& sr, bool verbose) {

    // Open a new file, dump the header

    // Set the algorithms used
    vector<PMCAlg> algs = sr.get_active_pmc_algs_vector();

    // For each graph:
    for (unsigned i=0; i<gs.size(); ++i) {
        UTILS__PRINT_IF(verbose, "=== Racing graph " << i+1 << "/" << gs.size()
                       << ": '" << gs[i].get_text() << "'" << endl);
        UTILS__PRINT_IF(verbose, "Start time: " << utils__timestamp_to_fulldate(time(NULL)) << endl);

        // Use all algorithms on the graph.
        // To save time, since all algorithms require the calculation of all minimal
        // separators, start by calculating them in advance. Add the time required to
        // the total time.
        // To enforce the time limit, calculate the separators one by one.
        time_t start_time = time(NULL);
        bool time_limit_exceeded = false;
        MinimalSeparatorsEnumerator mse(gs[i].get_graph(), UNIFORM);
        NodeSetSet min_seps;
        time_t ms_calc_time;
        try {
            while(mse.hasNext()) {
                min_seps.insert(mse.next());
                ms_calc_time = difftime(time(NULL),start_time);
                if (sr.test_time_limit_pmc() && difftime(time(NULL),start_time) > sr.get_time_limit_pmc()) {
                    time_limit_exceeded = true;
                    break;
                }
            }
        }
        catch(std::bad_alloc) {
            TRACE(TRACE_LVL__ERROR, "Out of memory calculating minimal separators...");
            gs[i].set_mem_error_ms();
            continue;
        }

        // Keep calculating. Now, the remaining time can be used by each algorithm
        // separately.
        UTILS__PRINT_IF(verbose, "MS calc time: " << utils__timestamp_to_hhmmss(ms_calc_time) << endl);
        // time_remaining may be garbage if sr.test_time_limit_pmc() == false
        time_t time_remaining_for_pmcs = difftime(sr.get_time_limit_pmc(),ms_calc_time);

        // If the initial MS calculation took too long, all algorithms would be
        // too long.
        // Update the graph stats objects and continue on to the new graph.
        if (time_limit_exceeded) {
            UTILS__PRINT_IF(verbose, "Out of time in initial MS calculation, moving on to the next graph." << endl);
            gs[i].set_reached_time_limit_pmc(utils__vector_to_set(algs));
            gs[i].set_pmc_calc_time(algs,sr.get_time_limit_pmc()+1);
            continue;
        }

        // Update data, if requested
        if (sr.test_ms()) {
            TRACE(TRACE_LVL__TEST, "Setting ms to " << min_seps);
            gs[i].set_ms(min_seps);
        }
        else {
            // Update MS count anyway
            TRACE(TRACE_LVL__TEST, "Setting ms-count to " << min_seps.size());
            gs[i].set_ms_count(min_seps.size());
        }

        // Use a random order of the algorithms, in case
        // cache hits affect results.
        vector<int> algorithm_shuffle(algs.size());
        for (unsigned j=0; j<algorithm_shuffle.size(); ++j) {
            algorithm_shuffle[j] = j;
        }
        std::random_shuffle(algorithm_shuffle.begin(), algorithm_shuffle.end());
        UTILS__PRINT_IF(verbose, "Iterating over algorithm_shuffle in the following order: " << algorithm_shuffle << endl);

        for (unsigned alg_index=0; alg_index<algorithm_shuffle.size(); ++alg_index) {
            PMCAlg alg = algs[algorithm_shuffle[alg_index]];
            UTILS__PRINT_IF(verbose,"New iteration, alg = " << alg.str() << endl);

            // Calculate PMCs
            PMCEnumerator pmce(gs[i].get_graph());
            TRACE(TRACE_LVL__TEST, "Set algorithm for the PMCE");
            pmce.set_algorithm(alg);
            NodeSetSet pmcs;
            time_t inner_start_time = time(NULL);
            try {
                if (gs[i].get_graph().getNumberOfNodes() > 0) {
                    pmce.set_minimal_separators(min_seps);
                }
                if (sr.test_time_limit_pmc() && time_remaining_for_pmcs < sr.get_time_limit_pmc()) {
                    // Set error state
                    TRACE(TRACE_LVL__WARNING, "Out of time at an unusual place... used "
                                            << ms_calc_time << "/" << sr.get_time_limit_pmc() << " seconds");
                    gs[i].set_reached_time_limit_pmc(alg);
                }
                else {
                    if (sr.test_time_limit_pmc()) {
                        pmce.set_time_limit(time_remaining_for_pmcs);
                    }
                    pmcs = pmce.get(/*sr*/);
                }
            }
            catch (std::bad_alloc) {
                TRACE(TRACE_LVL__ERROR, "Out of memory during PMC calculation");
                gs[i].set_mem_error_pmc(alg);
                continue;
            }
            time_t pmc_calc_time_without_ms = difftime(time(NULL), inner_start_time);
            time_t pmc_calc_time_with_ms = pmc_calc_time_without_ms + ms_calc_time;
            TRACE(TRACE_LVL__TEST, "Ran PMCE, total (including MS) is " << utils__timestamp_to_hhmmss(pmc_calc_time_with_ms));

            // Error checking
            gs[i].set_pmc_calc_time(alg, pmc_calc_time_with_ms);
            if (gs[i].reached_time_limit_pmc(alg) ||
                        pmce.is_out_of_time() ||
                        (sr.test_time_limit_pmc() && gs[i].get_pmc_calc_time(alg) > sr.get_time_limit_pmc())) {
                gs[i].set_reached_time_limit_pmc(alg);  // This may be the first time we set this
                UTILS__PRINT_IF(verbose, "Time limit reached in algorithm " << alg.str());
                continue;
            }
            TRACE(TRACE_LVL__TEST, "Not out of time");

            // Update data, if requested
            if (sr.test_count_pmc()) {
                if (debug && alg_index>0 && gs[i].get_pmc_count() != (long)pmcs.size()) {
                    TRACE(TRACE_LVL__ERROR, "Inconsistent PMC count! Already set "
                                << gs[i].get_pmc_count() << ", but counted " << pmcs.size()
                                << " using algorithm " << alg.str());
                    return false;
                }
                TRACE(TRACE_LVL__TEST, "Setting pmc_count to " << pmcs.size());
                gs[i].set_pmc_count(pmcs.size());
            }
            if (sr.test_pmc()) {
                if (debug && alg_index>0 && gs[i].get_pmc() != pmcs) {
                    TRACE(TRACE_LVL__ERROR, "Inconsistent PMC set!" << endl
                                << "Previously set:" << endl << gs[i].get_pmc() << endl
                                << "Now calculated:" << endl << pmcs << endl
                                << "Using algorithm " << alg.str());
                    return false;
                }
                TRACE(TRACE_LVL__TEST, "Setting pmcs to " << pmcs);
                gs[i].set_pmc(pmcs);
            }
            if (sr.test_ms_subgraphs()) {
                if (debug && alg_index>0 && gs[i].get_ms_subgraphs() != pmce.get_ms_subgraphs()) {
                    TRACE(TRACE_LVL__ERROR, "Inconsistent MS subgraphs!" << endl
                                << "Previously set:" << endl << gs[i].get_ms_subgraphs() << endl
                                << "Now calculated:" << endl << pmce.get_ms_subgraphs() << endl
                                << "Using algorithm " << alg.str());
                    return false;
                }
                TRACE(TRACE_LVL__TEST, "Setting ms_subgraphs to " << pmce.get_ms_subgraphs());
                gs[i].set_ms_subgraphs(pmce.get_ms_subgraphs());
            }
            if (sr.test_ms_subgraph_count()) {
                if (debug && alg_index>0 && gs[i].get_ms_subgraph_count() != pmce.get_ms_count_subgraphs()) {
                    TRACE(TRACE_LVL__ERROR, "Inconsistent MS subgraph count!" << endl
                                << "Previously set:" << endl << gs[i].get_ms_subgraph_count() << endl
                                << "Now calculated:" << endl << pmce.get_ms_count_subgraphs() << endl
                                << "Using algorithm " << alg.str());
                    return false;
                }
                TRACE(TRACE_LVL__TEST, "Setting ms_count_subgraphs to " << pmce.get_ms_count_subgraphs());
                gs[i].set_ms_subgraph_count(pmce.get_ms_count_subgraphs());
            }
            TRACE(TRACE_LVL__TEST, "Done with iteration (alg=" << alg.str() << ")");
        }

    }

    // We're OK!
    return true;
}



}
