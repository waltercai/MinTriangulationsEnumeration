#include "DatasetStatisticsGenerator.h"
#include "DataStructures.h"
#include "DirectoryIterator.h"
#include "Utils.h"
#include "TestUtils.h"
#include "PMCEnumeratorTester.h"
#include "GraphReader.h"
#include "ChordalGraph.h"
#include "MinimalTriangulationsEnumerator.h"
#include "MinTriangulationsEnumeration.h"
#include "ResultsHandler.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include <string>
#include <sstream>
#include <time.h>
#include <random>
using std::ostringstream;
namespace tdenum {

typedef enum {
    MAIN_TMP,
    MAIN_MIN_TRIANG_ENUM,
    MAIN_PMC_TEST,
    MAIN_STATISTIC_GEN,
    MAIN_RANDOM_STATS,
    MAIN_DIFFICULT_STATS,
    MAIN_QUICK_STATS,
    MAIN_FINE_GRAIN_P
} MainType;

class Main {
private:

    /**
     * Just a sandbox main
     */
    int tmp() const {

        return 0;
    }

    /**
     * First parameter is the graph file path. Second is timeout in seconds.
     * Third is the order of extending triangulations. Options are: width, fill,
     * difference, sepsize, none.
     * Fourth is the order of extending minimal separators. Options are: size
     * (ascending), fill or none.
     */
    int triang_enum(int argc, char* argv[]) const {
        // Parse input graph file
        if (argc < 2) {
            cout << "No graph file specified" << endl;
            return 0;
        }
        InputFile inputFile(argv[1]);
        Graph g = GraphReader::read(inputFile.getPath());

        // Define default parameters
        bool isTimeLimited = false;
        int timeLimitInSeconds = -1;
        WhenToPrint print = NEVER;
        string algorithm = "";
        TriangulationAlgorithm heuristic = MCS_M;
        TriangulationScoringCriterion triangulationsOrder = NONE;
        SeparatorsScoringCriterion separatorsOrder = UNIFORM;

        // Replace parameter specified
        for (int i=2; i<argc; i++) {
            string argument = argv[i];
            string flagName = argument.substr(0, argument.find_last_of("="));
            string flagValue = argument.substr(argument.find_last_of("=")+1);
            if (flagName == "time_limit") {
                timeLimitInSeconds = atoi(flagValue.c_str());
                if (timeLimitInSeconds >= 0) {
                    isTimeLimited = true;
                }
            } else if (flagName == "print") {
                if (flagValue == "all") {
                    print = ALWAYS;
                } else if (flagValue == "none") {
                    print = NEVER;
                } else if (flagValue == "improved") {
                    print = IF_IMPROVED;
                }
            } else if (flagName == "alg") {
                algorithm = algorithm + "." + flagValue;
                if (flagValue == "mcs") {
                    heuristic = MCS_M;
                } else if (flagValue == "degree") {
                    heuristic = MIN_DEGREE_LB_TRIANG;
                } else if (flagValue == "fill") {
                    heuristic = MIN_FILL_LB_TRIANG;
                }  else if (flagValue == "initialDegree") {
                    heuristic = INITIAL_DEGREE_LB_TRIANG;
                } else if (flagValue == "initialFill") {
                    heuristic = INITIAL_FILL_LB_TRIANG;
                } else if (flagValue == "lb") {
                    heuristic = LB_TRIANG;
                } else if (flagValue == "combined") {
                    heuristic = COMBINED;
                } else if (flagValue == "separators") {
                    heuristic = SEPARATORS;
                } else {
                    cout << "Triangulation algorithm not recognized" << endl;
                    return 0;
                }
            } else if (flagName == "t_order") {
                algorithm = algorithm + "." + flagValue;
                if (flagValue == "fill") {
                    triangulationsOrder = FILL;
                } else if (flagValue == "width") {
                    triangulationsOrder = WIDTH;
                } else if (flagValue == "difference") {
                    triangulationsOrder = DIFFERENECE;
                } else if (flagValue == "sepsize") {
                    triangulationsOrder = MAX_SEP_SIZE;
                } else if (flagValue == "none") {
                    triangulationsOrder = NONE;
                } else {
                    cout << "Triangulation scoring criterion not recognized" << endl;
                    return 0;
                }
            } else if (flagName == "s_order") {
                algorithm = algorithm + "." + flagValue;
                if (flagValue == "size") {
                    separatorsOrder = ASCENDING_SIZE;
                } else if (flagValue == "none") {
                    separatorsOrder = UNIFORM;
                } else if (flagValue == "fill") {
                    separatorsOrder = FILL_EDGES;
                } else {
                    cout << "Seperators scoring criterion not recognized" << endl;
                    return 0;
                }
            }
        }

        // Open output files
        ofstream detailedOutput;
        if (print != NEVER) {
            algorithm = algorithm != "" ? algorithm : "mcs";
            string outputFileName = inputFile.getField() + "." + inputFile.getType()
                    + "." + inputFile.getName() + "." + algorithm + ".csv";
            detailedOutput.open(outputFileName.c_str());
        }
        string summaryFileName = "summary.csv";
        ofstream summaryOutput;
        if (!ifstream(summaryFileName.c_str()).good()) { // summary file doesn't exist
            summaryOutput.open(summaryFileName.c_str());
            printSummaryHeader(summaryOutput);
        } else  {
            summaryOutput.open(summaryFileName.c_str(), ios::app);
        }

        // Initialize variables
        cout << setprecision(2);
        cout << "Starting enumeration for " << inputFile.getField() << "\\"
                << inputFile.getType() << "\\" << inputFile.getName() << endl;
        clock_t startTime = clock();
        ResultsHandler results(g, detailedOutput, print);
        bool timeLimitExceeded = false;

        // Generate the results and print details if asked for
        MinimalTriangulationsEnumerator enumerator(g, triangulationsOrder, separatorsOrder, heuristic);
        while (enumerator.hasNext()) {
            ChordalGraph triangulation = enumerator.next();
            results.newResult(triangulation);
            double totalTimeInSeconds = double(clock() - startTime) / CLOCKS_PER_SEC;
            if (isTimeLimited && totalTimeInSeconds >= timeLimitInSeconds) {
                timeLimitExceeded = true;
                break;
            }
        }
        if (print != NEVER) {
            detailedOutput.close();
        }

        // Print summary to file
        double totalTimeInSeconds = double(clock() - startTime) / CLOCKS_PER_SEC;
        int separators = enumerator.getNumberOfMinimalSeperatorsGenerated();
        printSummary(summaryOutput, inputFile, g, timeLimitExceeded,
                totalTimeInSeconds, algorithm, separators, results);
        summaryOutput.close();

        // Print summary to standard output
        if (timeLimitExceeded) {
            cout << "Time limit reached." << endl;
        } else {
            cout << "All minimal triangulations were generated!" << endl;
        }
        results.printReadableSummary(cout);
        cout << "The graph has " << g.getNumberOfNodes() << " nodes and " << g.getNumberOfEdges() << " edges. ";
        cout << separators << " minimal separators were generated in the process." << endl;

        return 0;
	}

    /**
     * Run the DatasetStatisticsGenerator and output files given the datasets NOT in
     * the "difficult" folder (so it won't take forever).
     */
    void stat_gen_aux(DatasetStatisticsGenerator& dsg) const {
        DirectoryIterator deadeasy_files(DATASET_DIR_BASE+DATASET_DIR_DEADEASY);
        DirectoryIterator easy_files(DATASET_DIR_BASE+DATASET_DIR_EASY);
        string dataset_filename;
        while(deadeasy_files.next_file(&dataset_filename)) {
            dsg.add_graph(dataset_filename);
        }
        while(easy_files.next_file(&dataset_filename)) {
            dsg.add_graph(dataset_filename);
        }
    }
    int stat_gen() const {
        DatasetStatisticsGenerator dsg(RESULT_DIR_BASE+"EasyResults.csv");
        stat_gen_aux(dsg);
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
//        p.flag_noamsgraphs = true;
        p.go();
        return 0;
    }

    /**
     * Generate random graphs from G(n,p) (where p[i] is a vector of
     * probabilities for graphs of size n[i]). For each value of n
     * and p, create and compute stats for 'instances' instances.
     */
    void random_stats_aux(DatasetStatisticsGenerator& dsg,
                          const vector<int>& n,
                          const vector<vector<double> >& p,
                          int instances) const {
        for (unsigned int i=0; i<n.size(); ++i) {
            for (unsigned int j=0; j<p[i].size(); ++j) {
                for (int k=0; k<instances; ++k) {
                    ostringstream s;
                    s << "G(" << n[i] << ":" << p[i][j] << "); " << k+1 << "/" << instances;
                    Graph g(n[i]);
                    g.randomize(p[i][j]);
                    dsg.add_graph(g, s.str());
                }
            }
        }
    }
    int random_stats() const {
        // No need to output nodes, the graph name is enough
        DatasetStatisticsGenerator dsg("RandomResults.csv",
                        DSG_COMP_ALL ^ DSG_COMP_TRNG); // Everything except triangulations
        vector<vector<double> > p = {{0.3,0.5,0.7},{0.3,0.5,0.7},{0.3,0.5,0.7},{0.3,0.5,0.7}};
        random_stats_aux(dsg, {20,30,40,50}, p, 3);
        dsg.compute(true);
        dsg.print();
        return 0;
    }

    /**
     * Generates stats (MS's and PMCs, not triangulations) using the difficult
     * input graphs.
     */
    int difficult_graphs() const {
        DirectoryIterator difficult_files(DATASET_DIR_BASE+DATASET_DIR_DIFFICULT);
        string dataset_filename;
        string output_filename = RESULT_DIR_BASE+"DifficultResults.csv";
        DatasetStatisticsGenerator dsg(output_filename);
        while(difficult_files.next_file(&dataset_filename)) {
            dsg.add_graph(dataset_filename);
        }
        dsg.compute(true);
        dsg.print();
        return 0;
    }

    /**
     * Gather ONLY the minimal separators on:
     * - Graphs in easy+deadeasy
     * - Random graphs with p=70% edges and up to 50 nodes
     */
    int quick_graphs() const {
        // Don't calculate PMCs or triangulations
        DatasetStatisticsGenerator dsg(RESULT_DIR_BASE+"QuickResults.csv",
                        DSG_COMP_ALL ^ (DSG_COMP_TRNG | DSG_COMP_PMC));
        stat_gen_aux(dsg);
        vector<vector<double> > p = {{0.7},{0.7},{0.7},{0.7}};
        random_stats_aux(dsg, {20,30,40,50}, p, 10);
        DirectoryIterator difficult_files(DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_BN);
        difficult_files.skip("Grid");
        string dataset_filename;
        while(difficult_files.next_file(&dataset_filename)) {
            dsg.add_graph(dataset_filename);
        }
        dsg.compute(true);
        dsg.print();
        return 0;
    }

    /**
     * Random graphs with p={1/n,2/n,...,(n-1)/n,1} (where n is the number of nodes).
     */
    int fine_grained_probability() const {
        DatasetStatisticsGenerator dsg(RESULT_DIR_BASE+"FineGrainedRandom.csv",
                        DSG_COMP_ALL ^ (DSG_COMP_TRNG | DSG_COMP_PMC));
        vector<int> n = {20, 30, 50, 70};
        vector<vector<double> > p(n.size());
        for (unsigned int i=0; i<n.size(); ++i)
            for (int j=1; j<=n[i]; ++j)
                p[i].push_back(j/(double(n[i])));
        random_stats_aux(dsg, n, p, 3);
        dsg.compute(true);
        dsg.print();
        return 0;
    }



    int return_val;
public:
    // Run the specified program
    Main(MainType mt, int argc=0, char* argv[]=NULL) : return_val(-1) {
        switch(mt) {
        case MAIN_TMP:
            return_val = tmp();
            break;
        case MAIN_MIN_TRIANG_ENUM:
            return_val = triang_enum(argc, argv);
            break;
        case MAIN_PMC_TEST:
            return_val = pmc_test();
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
            return_val = quick_graphs();
            break;
        case MAIN_FINE_GRAIN_P:
            return_val = fine_grained_probability();
            break;
        }
    }
    // Output the return value
    int get() const {
        return return_val;
    }
};

}

using namespace tdenum;

int main(int argc, char* argv[]) {
    srand(time(NULL)); // For random graphs
    return Main(MAIN_FINE_GRAIN_P).get();
}






