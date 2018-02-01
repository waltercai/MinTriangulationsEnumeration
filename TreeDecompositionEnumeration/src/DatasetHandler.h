#ifndef DATASETHANDLER_H_INCLUDED
#define DATASETHANDLER_H_INCLUDED

#include "Dataset.h"
#include "DirectoryIterator.h"
#include "StatisticRequest.h"

namespace tdenum {

/**
 * This class is the main interface to the Dataset and the StatisticRequest
 * classes.
 *
 * Use this class to construct statistical analysis requests (including the
 * input graphs to analyze) and output to file.
 *
 * This class is designed to be edited: the methods simply construct specific
 * sets of data analysis (input graphs and desired output).
 */

/**
 * Start by defining macros for target directories and files.
 */

#define DORI_PROJECT_DATASET_DIR string(DATASET_DIR_BASE+"DoriProject"+string(1,SLASH))
#define DORI_PROJECT_DATASET_DIR_SMALLHARD (DORI_PROJECT_DATASET_DIR + string("SmallHard") + string(1,SLASH))
#define DORI_PROJECT_DATASET_DIR_SPARSE (DORI_PROJECT_DATASET_DIR + string("Sparse") + string(1,SLASH))
#define DORI_PROJECT_DATASET_DIR_DENSE (DORI_PROJECT_DATASET_DIR + string("Dense") + string(1,SLASH))
#define DORI_PROJECT_DATASET_DIR_BAYESIAN DATASET_NEW_DIR_BASE
#define DORI_PROJECT_RESULTS_DIR string(RESULT_DIR_BASE+"DoriProject"+string(1,SLASH))

class DatasetHandler {
private:
    /**
     * Runs a dataset defined by the vector of graphs given.
     *
     * The statistics requested are global (the same stats calculated for each graph) and
     * all algorithms are run.
     *
     * The output file will be composed of the dataset filename prefix and the dataset name.
     *
     * Verbose output is provided while calculating.
     */
    void run_dataset_globalstats_allalgs(const vector<GraphStats>& vgs,
                                         const StatisticRequest& sr,
                                         const string& graph_dir,
                                         const string& dataset_name,
                                         const string& dataset_filename_prefix);

public:

    int dori_project_races();

};

}

#endif // DATASETHANDLER_H_INCLUDED
