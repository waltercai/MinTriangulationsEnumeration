#include "DatasetHandler.h"

namespace tdenum {

void DatasetHandler::dori_project_races() {

    const int generation = 0;   // Change this to change the suffix
    Dataset dataset(DORI_PROJECT_RESULTS_DIR+"/SmallHard_"+UTILS__TO_STRING(generation));
    StatisticRequest sr;

    // Start by constructing the request.
    // We want a lot of data here, but the same data for each dataset which
    // makes things a little easier.
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
            .add_random_pstep_range(utils__vector_range(25,35),0.2,0.45,0.1,10)
            .get();
    vector<string> paths_smallhard;
    for (auto gs: vgs_smallhard) {
        paths_smallhard.push_back(DORI_PROJECT_DATASET_DIR_SMALLHARD+"/"+gs.get_text());
        gs.dump(paths_smallhard.back());
    }
    TRACE(TRACE_LVL__ALWAYS, "Post-production. Now, " << DORI_PROJECT_DATASET_DIR_SMALLHARD << " contains "
                            << DirectoryIterator(DORI_PROJECT_DATASET_DIR_SMALLHARD).file_count()
                            << " files.");

    Dataset dataset_smallhard(DORI_PROJECT_RESULTS_DIR+"/SmallHard_"+UTILS__TO_STRING(generation), paths_smallhard);
    dataset_smallhard.set_all_requests(sr);
    TRACE(TRACE_LVL__ALWAYS, "Starting with small hard graphs.");
    TRACE(TRACE_LVL__ALWAYS, "Reading from " << DORI_PROJECT_DATASET_DIR_SMALLHARD << ", "
                            << paths_smallhard.size() << " files found.");
    TRACE(TRACE_LVL__ALWAYS, "Calculating (this will take a while...)");
    dataset_smallhard.calc();
    TRACE(TRACE_LVL__ALWAYS, "Done. Dumping results to " << dataset.get_file_path() << ", replacing previous contents");
    dataset.dump();




/*

    Dataset dataset;
    StatisticRequest sr;
    const int generation = 0;   // Change this to change the suffix

    // Start by constructing the request.
    // We want a lot of data here, but the same data for each dataset which
    // makes things a little easier.
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
    dataset.set_input_dir(DORI_PROJECT_DATASET_DIR_SMALLHARD)
            .set_output_dir(DORI_PROJECT_RESULTS_DIR)
            .set_basename("SmallHard")
            .set_suffix(UTILS__TO_STRING(generation));
    TRACE(TRACE_LVL__ALWAYS, "Starting with small hard graphs.");
    TRACE(TRACE_LVL__ALWAYS, "Reading from " << DORI_PROJECT_DATASET_DIR_SMALLHARD << ", "
                            << DirectoryIterator(DORI_PROJECT_DATASET_DIR_SMALLHARD).file_count()
                            << " files found.");
    TRACE(TRACE_LVL__ALWAYS, "Loading graphs (first time requires production as well)...");
    dataset.produce(GraphProducer(true).add_random_pstep_range(utils__vector_range(25,35),0.2,0.45,0.1,10).get());
    TRACE(TRACE_LVL__ALWAYS, "Post-production. Now, " << DORI_PROJECT_DATASET_DIR_SMALLHARD << " contains "
                            << DirectoryIterator(DORI_PROJECT_DATASET_DIR_SMALLHARD).file_count()
                            << " files.");
    TRACE(TRACE_LVL__ALWAYS, "Calculating (this will take a while...)");
    dataset.calc(sr);
    TRACE(TRACE_LVL__ALWAYS, "Done. Dumping results to " << dataset.file_path() << ", replacing previous contents");
    dataset.dump(sr);

    // Sparse graphs
    dataset.reset()
            .set_input_dir(DORI_PROJECT_DATASET_DIR_SPARSE)
            .set_output_dir(DORI_PROJECT_RESULTS_DIR)
            .set_basename("Sparse")
            .set_suffix(UTILS__TO_STRING(generation));
    TRACE(TRACE_LVL__ALWAYS, "Sparse graphs...");
    TRACE(TRACE_LVL__ALWAYS, "Reading from " << DORI_PROJECT_DATASET_DIR_SPARSE << ", "
                            << DirectoryIterator(DORI_PROJECT_DATASET_DIR_SPARSE).file_count()
                            << " files found.");
    TRACE(TRACE_LVL__ALWAYS, "Loading graphs (first time requires production as well)...");
    dataset.produce(GraphProducer(true).add_random(utils__vector_range(50,60),{0.05},true,10).get());
    TRACE(TRACE_LVL__ALWAYS, "Post-production. Now, " << DORI_PROJECT_DATASET_DIR_DENSE << " contains "
                            << DirectoryIterator(DORI_PROJECT_DATASET_DIR_DENSE).file_count()
                            << " files.");
    TRACE(TRACE_LVL__ALWAYS, "Calculating (this will take a while...)");
    dataset.calc(sr);
    TRACE(TRACE_LVL__ALWAYS, "Done. Dumping results to " << dataset.file_path() << ", replacing previous contents");
    dataset.dump(sr);

    // Dense graphs
    dataset.reset()
            .set_input_dir(DORI_PROJECT_DATASET_DIR_DENSE)
            .set_output_dir(DORI_PROJECT_RESULTS_DIR)
            .set_basename("Dense")
            .set_suffix(UTILS__TO_STRING(generation));
    TRACE(TRACE_LVL__ALWAYS, "Dense graphs...");
    TRACE(TRACE_LVL__ALWAYS, "Reading from " << DORI_PROJECT_DATASET_DIR_DENSE << ", "
                            << DirectoryIterator(DORI_PROJECT_DATASET_DIR_DENSE).file_count()
                            << " files found.");
    TRACE(TRACE_LVL__ALWAYS, "Loading graphs (first time requires production as well)...");
    dataset.produce(GraphProducer(true).add_random_pstep_range(utils__vector_range(50,60),0.7,0.95,0.1,10).get());
    TRACE(TRACE_LVL__ALWAYS, "Post-production. Now, " << DORI_PROJECT_DATASET_DIR_DENSE << " contains "
                            << DirectoryIterator(DORI_PROJECT_DATASET_DIR_DENSE).file_count()
                            << " files.");
    TRACE(TRACE_LVL__ALWAYS, "Calculating (this will take a while...)");
    dataset.calc(sr);
    TRACE(TRACE_LVL__ALWAYS, "Done. Dumping results to " << dataset.file_path() << ", replacing previous contents");
    dataset.dump(sr);

    // Bayesian graphs
    dataset.reset()
            .set_input_dir(DORI_PROJECT_DATASET_DIR_BAYESIAN)
            .set_output_dir(DORI_PROJECT_RESULTS_DIR)
            .set_basename("Bayesian")
            .set_suffix(UTILS__TO_STRING(generation))
            .set_filters({"evid"}); // Files in the Bayesian directories to be ignored
    TRACE(TRACE_LVL__ALWAYS, "Bayesian graphs...");
    TRACE(TRACE_LVL__ALWAYS, "Reading from " << DORI_PROJECT_DATASET_DIR_BAYESIAN << ", "
                            << DirectoryIterator(DORI_PROJECT_DATASET_DIR_BAYESIAN).file_count()
                            << " files found.");
    TRACE(TRACE_LVL__ALWAYS, "Loading graphs...");
    dataset.load();
    TRACE(TRACE_LVL__ALWAYS, "Done. Calculating (this will take a while...)");
    dataset.calc(sr);
    TRACE(TRACE_LVL__ALWAYS, "Done. Dumping results to " << dataset.file_path() << ", replacing previous contents");
    dataset.dump(sr);

    // Done.
    TRACE(TRACE_LVL__ALWAYS, "=====END=====");*/
}

}
