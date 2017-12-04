#include "DirectoryIterator.h"
#include "PMCEnumerator.h"
#include "PMCRacer.h"
#include "PMCRacerTester.h"

namespace tdenum {

PMCRacerTester& PMCRacerTester::go() {
    unset_crosscheck_insanity();
    #define X(test) if (flag_##test) {DO_TEST(test);}
    PMCRACERTESTER_TEST_TABLE
    #undef X
    return *this;
}
PMCRacerTester::PMCRacerTester() :
    TestInterface("PMCRacer Tester")
    #define X(test) , flag_##test(true)
    PMCRACERTESTER_TEST_TABLE
    #undef X
    {}

// Setters / getters
#define X(test) \
PMCRacerTester& PMCRacerTester::set_##test() { flag_##test = true; return *this; } \
PMCRacerTester& PMCRacerTester::unset_##test() { flag_##test = false; return *this; } \
PMCRacerTester& PMCRacerTester::set_only_##test() { clear_all(); return set_##test(); }
PMCRACERTESTER_TEST_TABLE
#undef X

PMCRacerTester& PMCRacerTester::set_all() {
    #define X(test) set_##test();
    PMCRACERTESTER_TEST_TABLE
    #undef X
    return *this;
}
PMCRacerTester& PMCRacerTester::clear_all() {
    #define X(test) unset_##test();
    PMCRACERTESTER_TEST_TABLE
    #undef X
    return *this;
}

bool crosscheck_aux(vector<GraphStats>& vgs) {

    StatisticRequest sr = StatisticRequest()
        .set_all_algs_to_pmc_race()
        .set_pmc()
        .set_count_pmc();

    // Calculate PMCs with the enumerator
    for (unsigned i=0; i<vgs.size(); ++i) {
        TRACE(TRACE_LVL__OFF, "Precalculating graph #" << (i+1) << "...");
        PMCEnumerator pmce(vgs[i].get_graph());
        pmce.set_algorithm(PMCALG_ENUM_DESCENDING_REVERSE_MS);
        vgs[i].set_pmc(pmce.get(/*sr*/));
        TRACE(TRACE_LVL__OFF, "Got " << vgs[i].get_pmc() << endl);
    }


    // Crosscheck with all algorithms
    PMCRacer pmcr("", false);
    pmcr.set_debug();
    pmcr.add(vgs);
    ASSERT(pmcr.go(sr));
    vector<GraphStats> out = pmcr.get_stats();
    ASSERT_EQ(out.size(), vgs.size());
    for (unsigned i=0; i<vgs.size(); ++i) {
        ASSERT_EQ(vgs[i].get_graph(), out[i].get_graph());
        ASSERT_EQ(vgs[i].get_pmc(), out[i].get_pmc());
    }

    return true;
}
bool PMCRacerTester::crosscheck_sanity() const {

    vector<GraphStats> vgs;
    for (unsigned i=0; i<10; ++i) {
        vgs.push_back(GraphStats(Graph(5).randomize(0.3)));
        vgs.push_back(GraphStats(Graph(10).randomize(0.3)));
    }

    return crosscheck_aux(vgs);
}
bool PMCRacerTester::crosscheck_insanity() const {

    vector<string> paths({
            DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_RANDOM_70+"30.csv",
            DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_RANDOM_70+"40.csv",
            DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_RANDOM_70+"50.csv",
            DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_RANDOM_70+"60.csv",
            DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_RANDOM_70+"70.csv"
        });

    vector<GraphStats> vgs;
    for (unsigned i=0; i<paths.size(); ++i) {
        ASSERT(utils__file_exists(paths[i]));
        vgs.push_back(GraphStats::read(paths[i]));
    }

    return crosscheck_aux(vgs);
}
bool PMCRacerTester::validate_accurate_times_basic() const {
    GraphStats gs = GraphStats::read(DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_RANDOM_70+"40.csv");
    StatisticRequest sr = StatisticRequest().set_single_pmc_alg(PMCALG_ENUM_NORMAL).set_pmc().set_count_pmc();
    PMCEnumerator pmce(gs.get_graph());
    pmce.set_algorithm(PMCALG_ENUM_NORMAL);
    time_t start_time = time(NULL);
    NodeSetSet pmcs = pmce.get(/*sr*/);
    time_t expected_pmc_time = difftime(time(NULL), start_time);
    TRACE(TRACE_LVL__ALWAYS, "PMCEnumerator time: " << utils__timestamp_to_hhmmss(expected_pmc_time));
    PMCRacer pmcr("",false);
    pmcr.add(gs);
    pmcr.go(sr);
    time_t racer_time = pmcr.get_stats()[0].get_pmc_calc_time(PMCALG_ENUM_NORMAL);
    TRACE(TRACE_LVL__ALWAYS, "PMCRacer time: " << utils__timestamp_to_hhmmss(racer_time));

    ASSERT_GEQ(1.2*expected_pmc_time, racer_time);
    ASSERT_LEQ(0.8*expected_pmc_time, racer_time);

    return true;
}
bool PMCRacerTester::validate_accurate_times_basic_twoalgs() const {
    GraphStats gs = GraphStats::read(DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_RANDOM_70+"40.csv");
    StatisticRequest sr = StatisticRequest().set_single_pmc_alg(PMCALG_ENUM_NORMAL)
                                            .add_alg_to_pmc_race(PMCALG_ENUM_REVERSE_MS)
                                            .set_pmc()
                                            .set_count_pmc();
    ASSERT_GEQ(sr.get_active_pmc_algs().size(), 2);
    map<PMCAlg, time_t> pmce_times;
    for (PMCAlg alg: sr.get_active_pmc_algs_vector()) {
        PMCEnumerator pmce(gs.get_graph());
        pmce.set_algorithm(alg);
        time_t start_time = time(NULL);
        NodeSetSet pmcs = pmce.get(/*sr*/);
        pmce_times[alg] = difftime(time(NULL), start_time);
        TRACE(TRACE_LVL__ALWAYS, "PMCEnumerator time: " << utils__timestamp_to_hhmmss(pmce_times.at(alg)) << " for alg " << alg.str());
    }
    PMCRacer pmcr("",false);
    pmcr.add(gs);
    pmcr.go(sr);
    GraphStats out = pmcr.get_stats()[0];
    map<PMCAlg, time_t> racer_times;
    for (PMCAlg alg: sr.get_active_pmc_algs_vector()) {
        racer_times[alg] = out.get_pmc_calc_time(alg);
        TRACE(TRACE_LVL__ALWAYS, "PMCRacer time: " << utils__timestamp_to_hhmmss(racer_times[alg]) << " for alg " << alg.str());
    }


    for (PMCAlg alg: sr.get_active_pmc_algs_vector()) {
        ASSERT_GEQ(1.1*pmce_times[alg], racer_times[alg]);
        ASSERT_LEQ(0.9*pmce_times[alg], racer_times[alg]);
    }

    return true;
}
bool PMCRacerTester::validate_accurate_times() const {

    /**
     * Take hard graphs, run with specific algorithms using the PMCEnumerator
     * and then with the PMCRacer (use the batch-algorithm mode of the PMCRacer).
     *
     * Make sure the resulting times (as long as they are more than 10 seconds, or
     * some threshold) deviate by no more then 10%.
     */
    const int threshold = 20;
    const double allowed_deviation = 0.2; // 10% is too low..
    time_t test_start_time = time(NULL);

    vector<string> paths({
            DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_RANDOM_70+"40.csv",
            DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_RANDOM_70+"50.csv",
            DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_RANDOM_70+"60.csv",
        });

    vector<GraphStats> vgs;
    for (unsigned i=0; i<paths.size(); ++i) {
        ASSERT(utils__file_exists(paths[i]));
        vgs.push_back(GraphStats::read(paths[i]));
    }

    // Calculate PMCs with the enumerator
    map<pair<unsigned,PMCAlg>, time_t> calc_times;
    for (unsigned i=0; i<vgs.size(); ++i) {
        TRACE(TRACE_LVL__ALWAYS, "Precalculating graph #" << (i+1) << "...");
        for (PMCAlg alg: PMCAlg::get_all()) {
            TRACE(TRACE_LVL__OFF, "Algorithm: " << alg.str());
            /*StatisticRequest sr = StatisticRequest()
                .set_single_pmc_alg(alg)
                .set_pmc()
                .set_count_pmc();*/
            PMCEnumerator pmce(vgs[i].get_graph());
            pmce.set_algorithm(alg);
            time_t start_time = time(NULL);
            pmce.get(/*sr*/);
            time_t calc_time = difftime(time(NULL), start_time);
            calc_times[pair<unsigned,PMCAlg>(i,alg)] = calc_time;
            TRACE(TRACE_LVL__ALWAYS, "Got a time of "
                << utils__timestamp_to_hhmmss(calc_time)
                << " with algorithm " << alg.str());
        }
        // Filter out those graphs that have quick times..
        bool cont = false;
        for (PMCAlg alg: PMCAlg::get_all()) {
            TRACE(TRACE_LVL__ALWAYS, "Checking skip level for algorithm " << alg.str());
            pair<unsigned,PMCAlg> k(i,alg);
            if (calc_times.at(k) < threshold) {
                TRACE(TRACE_LVL__WARNING, "Graph #" << (i+1) << " is too easy "
                        << "(a time of " << utils__timestamp_to_hhmmss(calc_times.at(k))
                        << " with algorithm " << alg.str() << ", skipping...");
                cont = true;
                break;
            }
        }
        if (cont) {
            continue;
        }
        // Crosscheck with all algorithms
        TRACE(TRACE_LVL__ALWAYS, "Starting cross-check with the PMCR, should be done at ~"
              << utils__timestamp_to_hhmmss(time(NULL) + difftime(time(NULL),test_start_time))
              << "...");
        StatisticRequest sr = StatisticRequest()
            .set_all_algs_to_pmc_race()
            .set_pmc()
            .set_count_pmc();
        PMCRacer pmcr("", false);
        pmcr.set_debug();
        pmcr.add(vgs[i]);
        ASSERT(pmcr.go(sr));
        ASSERT_EQ(pmcr.get_stats().size(), 1);
        GraphStats out = pmcr.get_stats()[0];
        TRACE(TRACE_LVL__ALWAYS, "Done. got the following times:");
        for (PMCAlg alg: PMCAlg::get_all()) {
            TRACE(TRACE_LVL__ALWAYS, utils__timestamp_to_hhmmss(out.get_pmc_calc_time(alg)) << " for algorithm " << alg.str());
        }
        ASSERT_EQ(vgs[i].get_graph(), out.get_graph());
        for (PMCAlg alg: PMCAlg::get_all()) {
            time_t pmce_time = calc_times.at(pair<unsigned,PMCAlg>(i,alg));
            TRACE(TRACE_LVL__ALWAYS, "Do " << utils__timestamp_to_hhmmss(pmce_time)
                  << " and " << utils__timestamp_to_hhmmss(out.get_pmc_calc_time(alg))
                  << " deviate at most " << (int)(allowed_deviation*100) << "\%?");
            ASSERT_LEQ((double(1)-allowed_deviation)*calc_times.at(pair<unsigned,PMCAlg>(i,alg)), out.get_pmc_calc_time(alg));
            ASSERT_GEQ((double(1)+allowed_deviation)*calc_times.at(pair<unsigned,PMCAlg>(i,alg)), out.get_pmc_calc_time(alg));
        }
    }

    return true;
}
bool PMCRacerTester::validate_accurate_times_batch_mode() const {

    /**
     * Take hard graphs, run with specific algorithms using the PMCEnumerator
     * and then with the PMCRacer (use the batch-algorithm mode of the PMCRacer).
     *
     * Make sure the resulting times (as long as they are more than 10 seconds, or
     * some threshold) deviate by no more then 10%.
     */
    const int threshold = 60;
    const double deviance = 0.1;

    vector<string> paths({
            DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_RANDOM_70+"40.csv",
            DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_RANDOM_70+"50.csv",
            DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_RANDOM_70+"60.csv",
        });

    vector<GraphStats> vgs;
    for (unsigned i=0; i<paths.size(); ++i) {
        ASSERT(utils__file_exists(paths[i]));
        vgs.push_back(GraphStats::read(paths[i]));
    }

    // Calculate PMCs with the enumerator
    map<pair<unsigned,PMCAlg>, time_t> calc_times;
    for (unsigned i=0; i<vgs.size(); ++i) {
        TRACE(TRACE_LVL__ALWAYS, "Precalculating graph #" << (i+1) << "...");
        for (PMCAlg alg: PMCAlg::get_all()) {
            TRACE(TRACE_LVL__OFF, "Algorithm: " << alg.str());
            /*StatisticRequest sr = StatisticRequest()
                .set_single_pmc_alg(alg)
                .set_pmc()
                .set_count_pmc();*/
            PMCEnumerator pmce(vgs[i].get_graph());
            pmce.set_algorithm(alg);
            time_t start_time = time(NULL);
            pmce.get(/*sr*/);
            time_t calc_time = difftime(time(NULL), start_time);
            calc_times[pair<unsigned,PMCAlg>(i,alg)] = calc_time;
            TRACE(TRACE_LVL__ALWAYS, "Got a time of "
                << utils__timestamp_to_hhmmss(calc_time)
                << " with algorithm " << alg.str());
        }
    }

    // Filter out those graphs that have quick times..
    vector<unsigned> skip_list;
    for (unsigned i=0; i<vgs.size(); ++i) {
        TRACE(TRACE_LVL__ALWAYS, "Checking skip level for graph #" << (i+1));
        for (PMCAlg alg: PMCAlg::get_all()) {
            TRACE(TRACE_LVL__ALWAYS, "Checking skip level for algorithm " << alg.str());
            pair<unsigned,PMCAlg> k(i,alg);
            if (calc_times.at(k) < threshold) {
                skip_list.push_back(i);
                break;
            }
        }
    }

    // Crosscheck with all algorithms
    StatisticRequest sr = StatisticRequest()
        .set_all_algs_to_pmc_race()
        .set_pmc()
        .set_count_pmc();
    PMCRacer pmcr("", false);
    pmcr.set_debug();
    pmcr.add(vgs);
    ASSERT(pmcr.go(sr));
    vector<GraphStats> out = pmcr.get_stats();
    ASSERT_EQ(out.size(), vgs.size());
    for (unsigned i=0; i<vgs.size(); ++i) {
        if (utils__is_in_vector(i,skip_list)) {
            continue;
        }
        ASSERT_EQ(vgs[i].get_graph(), out[i].get_graph());
        for (PMCAlg alg: PMCAlg::get_all()) {
            ASSERT_LEQ((double(1)-deviance)*calc_times.at(pair<unsigned,PMCAlg>(i,alg)), out[i].get_pmc_calc_time(alg));
            ASSERT_GEQ((double(1)+deviance)*calc_times.at(pair<unsigned,PMCAlg>(i,alg)), out[i].get_pmc_calc_time(alg));
        }
    }

    return true;
}


}
