#include "DatasetStatisticsGenerator.h"
#include "DirectoryIterator.h"
#include "Graph.h"
#include "GraphProducer.h"
#include "GraphStats.h"
#include "PMCAlg.h"
#include "PMCEnumeratorTester.h"
#include "PMCRacer.h"
#include "Utils.h"

namespace tdenum {

#define DORI_PROJECT_DATASET_DIR_SMALLHARD (DORI_PROJECT_DATASET_DIR + string("SmallHard") + string(1,SLASH))
#define DORI_PROJECT_DATASET_DIR_SPARSE (DORI_PROJECT_DATASET_DIR + string("Sparse") + string(1,SLASH))
#define DORI_PROJECT_DATASET_DIR_DENSE (DORI_PROJECT_DATASET_DIR + string("Dense") + string(1,SLASH))
#define DORI_PROJECT_DATASET_DIR_BAYESIAN DATASET_NEW_DIR_BASE

class DoriMain {
private:
    // Calculates the data required (fast) and populates data fields.
    void calc_stats_aux(vector<GraphStats>& dataset, const char* filename) {
        if (utils__file_exists(filename)) {
            dataset = GraphProducer(true).import(filename).get();
            return;
        }
        DatasetStatisticsGenerator dsg(DORI_PROJECT_RESULTS_DIR+filename, GRAPHSTATS_ALL ^ (GRAPHSTATS_TRNG));
        dsg.set_pmc_alg(PMCAlg(true, false, true)); // As fast as possible, synced
        dsg.set_ms_time_limit(max_seconds); // So we see time taken in the results
        dsg.set_pmc_time_limit(max_seconds);
        for (GraphStats gs: dataset) {
            dsg.add_graph(gs.get_graph(), gs.get_text());
        }
        dsg.compute(true);
        dsg.print();
        dataset = dsg.get_stats();
    }

    // Races the graphs and outputs results
    void race_graphs_aux(vector<GraphStats>& dataset, const char* filename) {
        PMCRacer pmcr(DORI_PROJECT_RESULTS_DIR+filename, max_seconds);
        pmcr.add(dataset);
        pmcr.go(true, false);
    }

public:

    // Some constants.
    // 'instances' must be defined before the dataset fields for proper construction
    const int instances = 10;
    const int max_seconds = 10; // Temporary
    const char* small_hard_stats_filename = "SmallHardStats.csv";
    const char* sparse_stats_filename = "SparseStats.csv";
    const char* dense_stats_filename = "DenseStats.csv";
    const char* bayesian_stats_filename = "BayesianStats.csv";
    const char* small_hard_race_filename = "SmallHardRace.csv";
    const char* sparse_race_filename = "SparseRace.csv";
    const char* dense_race_filename = "DenseRace.csv";
    const char* bayesian_race_filename = "BayesianRace.csv";

    // The datasets & data
    vector<GraphStats> small_hard_stats;
    vector<GraphStats> sparse_stats;
    vector<GraphStats> dense_stats;
    vector<GraphStats> bayesian_stats;

    // Used to create random graphs and save them to disk.
    // We want consistent datasets for future study
    DoriMain& produce_datasets() {
        GraphProducer(DORI_PROJECT_DATASET_DIR_SMALLHARD, true)
            .add_random_pstep_range(utils__vector_range(25,35),0.2,0.45,0.1,instances)
            .dump_graphs();
        GraphProducer(DORI_PROJECT_DATASET_DIR_SPARSE, true)
            .add_random(utils__vector_range(50,60),{0.05},true,instances)
            .dump_graphs();
        GraphProducer(DORI_PROJECT_DATASET_DIR_DENSE, true)
            .add_random_pstep_range(utils__vector_range(50,60),0.7,0.95,0.1,instances)
            .dump_graphs();
        return *this;
    }

    // Assumes the datasets exist, and loads them.
    DoriMain& load_datasets() {
        small_hard_stats = GraphProducer(true).add_by_dir(DirectoryIterator(DORI_PROJECT_DATASET_DIR_SMALLHARD).skip(DUMMY_FILENAME)).get();
        sparse_stats = GraphProducer(true).add_by_dir(DirectoryIterator(DORI_PROJECT_DATASET_DIR_SPARSE).skip(DUMMY_FILENAME)).get();
        dense_stats = GraphProducer(true).add_by_dir(DirectoryIterator(DORI_PROJECT_DATASET_DIR_DENSE).skip(DUMMY_FILENAME)).get();
        bayesian_stats = GraphProducer(true).add_by_dir(DirectoryIterator(DORI_PROJECT_DATASET_DIR_BAYESIAN).skip(DUMMY_FILENAME).skip("evid")).get();
        return *this;
    }

    /**
     * Statistics generation.
     */
    DoriMain& calc_stats() {
//        calc_stats_aux(small_hard_stats, small_hard_stats_filename);
//        calc_stats_aux(sparse_stats, sparse_stats_filename);
//        calc_stats_aux(dense_stats, dense_stats_filename);
        calc_stats_aux(bayesian_stats, bayesian_stats_filename);
        return *this;
    }

    /**
     * After we have the basic stats, race the graphs (check calculation times)
     */
    DoriMain& race_graphs() {
        race_graphs_aux(small_hard_stats, small_hard_race_filename);
        race_graphs_aux(sparse_stats, sparse_race_filename);
        race_graphs_aux(dense_stats, dense_race_filename);
        race_graphs_aux(bayesian_stats, bayesian_race_filename);
        return *this;
    }


};

}


using namespace tdenum;

int main(int argc, char *argv[]) {
    Logger::start("log.txt", false);
//    PMCEnumeratorTester pmcet(false);
//    pmcet.go();
    DoriMain dm;
//    dm.produce_datasets();
    dm.load_datasets().calc_stats().race_graphs();
    return 0;
}
