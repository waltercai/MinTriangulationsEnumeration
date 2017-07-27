#include "DatasetStatisticsGenerator.h"
#include "DataStructures.h"
#include "Graph.h"
#include "GraphProducer.h"
#include "DirectoryIterator.h"
#include "Utils.h"
#include "TestUtils.h"
#include "PMCEnumeratorTester.h"
#include "GraphTester.h"
#include "GraphReader.h"
#include "ChordalGraph.h"
#include "MinimalTriangulationsEnumerator.h"
#include "MinTriangulationsEnumeration.h"
#include "ResultsHandler.h"
#include "PMCRacer.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include <string>
#include <sstream>
#include <time.h>
#include <random>
#include <unistd.h>
using std::ostringstream;
using std::cin;
namespace tdenum {

typedef enum {
    MAIN_TMP,
    MAIN_PMC_TEST,
    MAIN_GRAPH_TEST,
    MAIN_STATISTIC_GEN,
    MAIN_RANDOM_STATS,
    MAIN_DIFFICULT_STATS,
    MAIN_QUICK_STATS,
    MAIN_QUICK_STATS_WITH_PMCS,
    MAIN_FINE_GRAIN_P,
    MAIN_FINE_GRAIN_AND_QUICK,
    MAIN_ALL_BAYESIAN,
    MAIN_PMC_RACE,
    MAIN_LAST   // Keep this one last
} MainType;


/**
 * Use this class to run specialized tests.
 * Designed to work with the DatasetStatisticsGenerator.
 */
class Main {
public:

private:

    /**
     * Placeholder for temporary stuff
     */
    int tmp() const {
        Graph g(2);
        cout << "Started with graph:" << endl << g;
        g.randomNodeRename();
        cout << "After randomization:" << endl << g;
        g.sortNodesByDegree(true);
        cout << "Ascending sort:" << endl << g;
        g.sortNodesByDegree(true);
        cout << "Descending sort:" << endl << g;
        return 0;
    }

    /**
     * Run the DatasetStatisticsGenerator and output files given the datasets NOT in
     * the "difficult" folder (so it won't take forever).
     */
    int stat_gen() const {
        DatasetStatisticsGenerator dsg(RESULT_DIR_BASE+"EasyResults.csv");
        dsg.add_graphs_dir(DATASET_DIR_BASE+DATASET_DIR_DEADEASY);
        dsg.add_graphs(DATASET_DIR_BASE+DATASET_DIR_EASY);
        dsg.compute(true);
        dsg.print();
        return 0;
    }

    /**
     * Test the PMC enumerator.
     *
     * This relies on an older version of DatasetStatisticsGenerator... see
     * random_stats for a better implementation.
     */
    int pmc_test() const {
        Logger::start("log.txt", false);
        PMCEnumeratorTester p(false);
//        p.clearAll();
//        p.flag_triangleonstilts = true;
        p.go();
        return 0;
    }

    /**
     * Test new graph features.
     * Not comprehensive, only created this class after Graph.h was
     * already done.
     */
    int graph_test() const {
        Logger::start("log.txt", false);
        GraphTester gt(false);
        gt.go();
        return 0;
    }

    int random_stats() const {
        // No need to output nodes, the graph name is enough
        DatasetStatisticsGenerator dsg(RESULT_DIR_BASE+"RandomResults.csv",
                        GRAPHSTATS_ALL ^ (GRAPHSTATS_TRNG/* | DSG_COMP_PMC*/)); // Everything except triangulations
        dsg.add_random_graphs({20,30,40,50},{0.3,0.5,0.7},true);
        dsg.compute(true);
        dsg.print();
        return 0;
    }

    /**
     * Generates stats (MS's and PMCs, not triangulations) using the difficult
     * input graphs.
     */
    int difficult_graphs() const {
        DatasetStatisticsGenerator dsg(RESULT_DIR_BASE+"DifficultResults.csv",
                                       DATASET_DIR_BASE+DATASET_DIR_DIFFICULT,
                                       GRAPHSTATS_ALL ^ (GRAPHSTATS_TRNG));
        dsg.compute(true);
        dsg.print();
        return 0;
    }

    /**
     * Gather ONLY the minimal separators on:
     * - Graphs in easy+deadeasy
     * - Random graphs with p=70% edges and up to 50 nodes
     */
    int quick_graphs(bool with_pmcs) const {
        // Don't calculate PMCs or triangulations
        DatasetStatisticsGenerator dsg(RESULT_DIR_BASE+"QuickResults.csv",
                                       with_pmcs ?
                                           GRAPHSTATS_ALL ^ GRAPHSTATS_TRNG :
                                           GRAPHSTATS_ALL ^ (GRAPHSTATS_TRNG | GRAPHSTATS_PMC));
        dsg.add_graphs_dir(DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_BN, {"Grid"});
        dsg.add_graphs_dir(DATASET_DIR_BASE+DATASET_DIR_DEADEASY);
        dsg.add_graphs(DATASET_DIR_BASE+DATASET_DIR_EASY);
        dsg.add_random_graphs({20,30,40,50},{0.7},true);
//        dsg.suppress_async();
//        dsg.compute_by_graph_number(80, true);
        dsg.compute(true);
        dsg.print();
        return 0;
    }

    /**
     * Random graphs with p={1/n,2/n,...,(n-1)/n,1} (where n is the number of nodes).
     */
    int fine_grained_probability() const {
        DatasetStatisticsGenerator dsg(RESULT_DIR_BASE+"FineGrainedRandom.csv",
                        GRAPHSTATS_ALL ^ (GRAPHSTATS_TRNG | GRAPHSTATS_PMC));
        dsg.add_random_graphs_pstep({20,30,50,70}, 0.5, 3);
        dsg.add_random_graphs({20,30,40,50},{0.3,0.5,0.7},true);
        dsg.compute(true);
        dsg.print();
        return 0;
    }

    /**
     * Both fine grained and quick graphs.
     */
    int fine_grained_and_quick() const {
        DatasetStatisticsGenerator dsg(RESULT_DIR_BASE+"FineGrainedAndQuick.csv",
                        GRAPHSTATS_ALL ^ (GRAPHSTATS_TRNG | GRAPHSTATS_PMC));
        dsg.add_random_graphs_pstep({20,30,50,70}, 0.5, 3);
        dsg.add_graphs_dir(DATASET_DIR_BASE+DATASET_DIR_DEADEASY);
        dsg.add_graphs(DATASET_DIR_BASE+DATASET_DIR_EASY);
        dsg.compute(true);
        dsg.print();
        return 0;
    }

    /**
     * All graphs from http://www.cs.huji.ac.il/project/PASCAL/showNet.php
     */
    int all_bayesian() const {
        DatasetStatisticsGenerator dsg(RESULT_DIR_BASE+"AllBayesian.csv",
                                       GRAPHSTATS_ALL ^ (GRAPHSTATS_TRNG));
        dsg.add_graphs_dir(DATASET_NEW_DIR_BASE, {"evid"});
        dsg.compute(true);
        dsg.print();
        return 0;
    }

    /**
     * Run the PMCRacer
     */
    int pmc_race() const {
        // Allow 20 minutes per graph
        PMCRacer pmcr(RESULT_DIR_BASE+"PMCRace.csv", 20*60);
        pmcr.add(GraphProducer().add_random({20,30,40},{0.3,0.5,0.7},true, 3).get());
        pmcr.add(GraphProducer().add_by_dir(DATASET_DIR_BASE+DATASET_DIR_DEADEASY).get());
        pmcr.add(GraphProducer().add_by_dir(DATASET_DIR_BASE+DATASET_DIR_EASY).get());
//        pmcr.add(GraphProducer().add_by_dir(DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_BN, {"Grid"}).get());
        pmcr.go(true);
        return 0;
    }

    // Store the return value
    int return_val;
    // The main function to run
    int main_type;
public:

    // Go!
    Main(MainType mt = MAIN_PMC_RACE, int argc = 1, char* argv[] = NULL) :
                                        return_val(-1), main_type(mt) {
        try {
            switch(main_type) {
            case MAIN_TMP:
                return_val = tmp();
                break;
            case MAIN_PMC_TEST:
                return_val = pmc_test();
                break;
            case MAIN_GRAPH_TEST:
                return_val = graph_test();
                break;
            case MAIN_STATISTIC_GEN:
                return_val = stat_gen();
                break;
            case MAIN_RANDOM_STATS:
                return_val = random_stats();
                break;
            case MAIN_DIFFICULT_STATS:
                return_val = difficult_graphs();
                break;
            case MAIN_QUICK_STATS:
                return_val = quick_graphs(false);
                break;
            case MAIN_QUICK_STATS_WITH_PMCS:
                return_val = quick_graphs(true);
                break;
            case MAIN_FINE_GRAIN_P:
                return_val = fine_grained_probability();
                break;
            case MAIN_FINE_GRAIN_AND_QUICK:
                return_val = fine_grained_and_quick();
                break;
            case MAIN_ALL_BAYESIAN:
                return_val = all_bayesian();
                break;
            case MAIN_PMC_RACE:
                return_val = pmc_race();
                break;
            default: break;
            }
        } catch (const std::exception& ex) {
            ASSERT_PRINT("Caught exception:" << endl << ex.what());
        } catch (const std::string& s) {
            ASSERT_PRINT("Caught exception string:" << endl << s);
        } catch(...) {
            ASSERT_PRINT("Whoops... unknown exception" << endl);
        }
    }

    // Retrieve the return value
    int get() {
        return return_val;
    }
};

}

using namespace tdenum;

int main(int argc, char* argv[]) {
    srand(time(NULL)); // For random graphs
    return Main().get();
}








