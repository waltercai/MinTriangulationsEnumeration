#include "DatasetHandler.h"
#include "TestInterface.h"

#define RESULTS_SUFFIX 0 // Change this to change the suffix of the target filename

namespace tdenum {

void DatasetHandler::run_dataset_globalstats_allalgs(const vector<GraphStats>& vgs,
                 const StatisticRequest& sr,
                 const string& graph_dir,
                 const string& dataset_name,
                 const string& dataset_filename_prefix) {
    vector<string> paths;
    TRACE(TRACE_LVL__ALWAYS, "In. Dumping graph files...");
    for (GraphStats gs: vgs) {
        string graph_name = gs.get_text();
        TRACE(TRACE_LVL__ALWAYS, "Got text '" << graph_name << "'. Is it valid?");
        if (!gs.text_valid()) {
            if (!gs.is_random()) {
                TRACE(TRACE_LVL__ERROR, "ERROR: Input graph doesn't have a valid name, but isn't random!\nSkipping dataset...");
                return;
            }
            graph_name = GraphStats::get_default_filename_from_graph(gs.get_graph());
            TRACE(TRACE_LVL__ALWAYS, "NO. Got new text: '" << graph_name << "'");
        }
        paths.push_back(utils__merge_dir_basename(graph_dir, graph_name));
        TRACE(TRACE_LVL__ALWAYS, "Dumping graph '" << paths.back() << "'");
        if (!gs.dump(paths.back())) {
            TRACE(TRACE_LVL__ERROR, "Failed to dump graph '" << paths.back() << "'! Aborting dataset...");
            return;
        }
    }
    TRACE(TRACE_LVL__ALWAYS, "Running DI from '" << graph_dir << "'");
    int total_files = DirectoryIterator(graph_dir).file_count();
    TRACE(TRACE_LVL__ALWAYS, "Post-production. Now, " << graph_dir << " contains " << total_files << " files.");

    Dataset ds(DORI_PROJECT_RESULTS_DIR+"/"+dataset_filename_prefix+UTILS__TO_STRING(RESULTS_SUFFIX)+".csv", paths);
    ds.set_all_requests(sr);
    TRACE(TRACE_LVL__ALWAYS, "Starting with " << dataset_name << " graphs.");
    TRACE(TRACE_LVL__ALWAYS, "Reading from " << graph_dir << ", "
                            << paths.size() << " files found.");
    TRACE(TRACE_LVL__ALWAYS, "Calculating (this will take a while...)");
    ds.calc();
    TRACE(TRACE_LVL__ALWAYS, "Done. Dumping results to " << ds.get_file_path() << ", replacing previous contents");
    ds.dump();
}

int DatasetHandler::dori_project_races() {

    // Sanity
    ASSERT(utils__mkdir(DORI_PROJECT_RESULTS_DIR));

    // Start by constructing the request.
    // We want a lot of data here, but the same data for each dataset which
    // makes things a little easier.
    StatisticRequest sr;
    sr.reset()
      .set_all_algs_to_pmc_race()
      .set_count_ms()
      .set_count_pmc()
      .set_time_limit_ms(5*60)        // Give MSs five minutes - if it takes more, then computing PMCs will take forever
      .set_time_limit_pmc(20*60)      // 20 minutes for PMCs
      .set_count_limit_ms(500000);    // More than enough. Memory may run out even with this limit

    // Four datasets in this run.
    TRACE(TRACE_LVL__ALWAYS, "=====START=====");

    // Small & hard (see the produce() command for details)
    vector<GraphStats> vgs_smallhard = GraphProducer(DORI_PROJECT_DATASET_DIR_SMALLHARD,true)
            .add_random_pstep_range(utils__vector_range(25,30),0.2,0.45,0.1,10)
            .get();
    run_dataset_globalstats_allalgs(vgs_smallhard, sr, DORI_PROJECT_DATASET_DIR_SMALLHARD, "small hard", "SmallHard_");

    // Sparse graphs
    vector<GraphStats> vgs_sparse = GraphProducer(DORI_PROJECT_DATASET_DIR_SPARSE,true)
            .add_dir(DORI_PROJECT_DATASET_DIR_SPARSE)
            .add_random(utils__vector_range(50,60),{0.05},true,10)
            .get();
    run_dataset_globalstats_allalgs(vgs_sparse, sr, DORI_PROJECT_DATASET_DIR_SPARSE, "sparse", "Sparse_");

    // Dense graphs
    vector<GraphStats> vgs_dense = GraphProducer(DORI_PROJECT_DATASET_DIR_DENSE,true)
            .add_dir(DORI_PROJECT_DATASET_DIR_DENSE)
            .add_random_pstep_range(utils__vector_range(50,60),0.7,0.95,0.1,10)
            .get();
    run_dataset_globalstats_allalgs(vgs_dense, sr, DORI_PROJECT_DATASET_DIR_DENSE, "dense", "Dense_");

    // Bayesian graphs
    vector<GraphStats> vgs_bayesian = GraphProducer(DORI_PROJECT_DATASET_DIR_BAYESIAN,true)
            .add(DirectoryIterator(DORI_PROJECT_DATASET_DIR_BAYESIAN).skip("evid"))
            .get();
    run_dataset_globalstats_allalgs(vgs_bayesian, sr, DORI_PROJECT_DATASET_DIR_BAYESIAN, "bayesian", "Bayesian_");

    // Done.
    TRACE(TRACE_LVL__ALWAYS, "=====END=====");
    return 0;
}


} // namespace tdenum
