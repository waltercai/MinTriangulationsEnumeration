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
#include <unistd.h>
using std::ostringstream;
using std::cin;
namespace tdenum {

typedef enum {
    MAIN_TMP,
    MAIN_PMC_TEST,
    MAIN_STATISTIC_GEN,
    MAIN_RANDOM_STATS,
    MAIN_DIFFICULT_STATS,
    MAIN_QUICK_STATS,
    MAIN_QUICK_STATS_WITH_PMCS,
    MAIN_FINE_GRAIN_P,
    MAIN_FINE_GRAIN_AND_QUICK,
    MAIN_ALL_BAYESIAN,
    MAIN_INTERACTIVE,
    MAIN_LAST   // Keep this one last
} MainType;

class Main {
private:

    /**
     * Just a sandbox main
     */
    int tmp() const {
        string certain_file = DATASET_DIR_BASE+DATASET_DIR_DIFFICULT
                                +"BN"+string(1,SLASH)
                                +"Grids"+string(1,SLASH)
                                +"grid10x10.f10.uai";
        DatasetStatisticsGenerator dsg(RESULT_DIR_BASE+"TempResults.csv",
                                       DSG_COMP_ALL ^ (DSG_COMP_TRNG));
        dsg.add_graph(certain_file);
        dsg.compute(true);
        dsg.print();
        return 0;
    }

    /**
     * Run the DatasetStatisticsGenerator and output files given the datasets NOT in
     * the "difficult" folder (so it won't take forever).
     */
    void stat_gen_aux(DatasetStatisticsGenerator& dsg) const {
        DirectoryIterator deadeasy_files(DATASET_DIR_BASE+DATASET_DIR_DEADEASY);
        DirectoryIterator easy_files(DATASET_DIR_BASE+DATASET_DIR_EASY);
        dsg.add_graphs(deadeasy_files);
        dsg.add_graphs(easy_files);
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
        DatasetStatisticsGenerator dsg(RESULT_DIR_BASE+"RandomResults.csv",
                        DSG_COMP_ALL ^ (DSG_COMP_TRNG/* | DSG_COMP_PMC*/)); // Everything except triangulations
        vector<int> n = {20,30,40,50};
        vector<vector<double> > p;
        for (unsigned int i=0; i < n.size(); ++i) {
            p.push_back({0.3,0.5,0.7});
        }
        random_stats_aux(dsg, n, p, 3);
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
        DatasetStatisticsGenerator dsg(RESULT_DIR_BASE+"DifficultResults.csv",
                                       difficult_files,
                                       DSG_COMP_ALL ^ (DSG_COMP_TRNG));
        dsg.compute(true);
        dsg.print();
        return 0;
    }

    /**
     * Gather ONLY the minimal separators on:
     * - Graphs in easy+deadeasy
     * - Random graphs with p=70% edges and up to 50 nodes
     */
    void quick_graphs_aux(DatasetStatisticsGenerator& dsg) const {
        stat_gen_aux(dsg);
        vector<vector<double> > p = {{0.7},{0.7},{0.7},{0.7}};
        random_stats_aux(dsg, {20,30,40,50}, p, 10);
    }
    int quick_graphs(bool with_pmcs) const {
        // Don't calculate PMCs or triangulations
        DirectoryIterator difficult_files(DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_BN);
        difficult_files.skip("Grid");
        DatasetStatisticsGenerator dsg(RESULT_DIR_BASE+"QuickResults.csv",
                                       difficult_files,
                                       with_pmcs ?
                                           DSG_COMP_ALL ^ DSG_COMP_TRNG :
                                           DSG_COMP_ALL ^ (DSG_COMP_TRNG | DSG_COMP_PMC));
        quick_graphs_aux(dsg);
        dsg.compute(true);
        dsg.print();
        return 0;
    }

    /**
     * Random graphs with p={1/n,2/n,...,(n-1)/n,1} (where n is the number of nodes).
     */
    void fine_grained_probability_aux(DatasetStatisticsGenerator& dsg) const {
        vector<int> n = {20, 30, 50, 70};
        vector<vector<double> > p(n.size());
        for (unsigned int i=0; i<n.size(); ++i)
            for (int j=1; j<=n[i]; ++j)
                p[i].push_back(j/(double(n[i])));
        random_stats_aux(dsg, n, p, 3);
    }
    int fine_grained_probability() const {
        DatasetStatisticsGenerator dsg(RESULT_DIR_BASE+"FineGrainedRandom.csv",
                        DSG_COMP_ALL ^ (DSG_COMP_TRNG | DSG_COMP_PMC));
        fine_grained_probability_aux(dsg);
        dsg.compute(true);
        dsg.print();
        return 0;
    }

    /**
     * Both fine grained and quick graphs.
     */
    int fine_grained_and_quick() const {
        DatasetStatisticsGenerator dsg(RESULT_DIR_BASE+"FineGrainedAndQuick.csv",
                        DSG_COMP_ALL ^ (DSG_COMP_TRNG | DSG_COMP_PMC));
        fine_grained_probability_aux(dsg);
        quick_graphs_aux(dsg);
        dsg.compute(true);
        dsg.print();
        return 0;
    }

    /**
     * All graphs from http://www.cs.huji.ac.il/project/PASCAL/showNet.php
     */
    int all_bayesian() const {
        DirectoryIterator di(DATASET_NEW_DIR_BASE);
        di.skip("evid"); // Skip the evidence files
        DatasetStatisticsGenerator dsg(RESULT_DIR_BASE+"AllBayesian.csv",
                                       di,
                                       DSG_COMP_ALL ^ (DSG_COMP_TRNG));
        dsg.compute(true);
        dsg.print();
        return 0;
    }

    /**
     * Small utility function for [y/n] user input.
     */
    bool get_answer() const {
        char answer;
        cin >> answer;
        while(answer != 'y' && answer != 'n') {
            cout << "Please enter 'y' or 'n': ";
            cin >> answer;
        }
        return answer == 'y';
    }
    int interactive() const {

        // Create the DSG
        int dsg_flags = DSG_COMP_ALL;
        string filename;
        string default_filename = "Interactive.csv";
        cout << "Before entering input graphs, please answer the following:" << endl;
        cout << "Count minimal separators? [y/n]: ";
        if (!get_answer()) {
            dsg_flags ^= DSG_COMP_MS;
        }
        cout << "Count PMCs? [y/n]: ";
        if (!get_answer()) {
            dsg_flags ^= DSG_COMP_PMC;
        }
        cout << "Count minimal triangulations? [y/n]: ";
        if (!get_answer()) {
            dsg_flags ^= DSG_COMP_TRNG;
        }
        cout << "Desired output filename (leave blank for default value of '" << default_filename << "'): ";
        cin >> filename;
        if (filename == string("")) {
            filename = default_filename;
        }
        DatasetStatisticsGenerator dsg(RESULT_DIR_BASE+filename, dsg_flags);

        // Handle random graphs:
        unsigned int randoms;
        cout << "Enter number of random graphs: ";
        cin >> randoms;
        cout << "Enter " << randoms << " lines, where each line contains two numbers separated by a space:" << endl
             << "\ti/" << randoms << ": N P" << endl
             << "where N is the number of nodes, and 0<=P<=1 is the edge probability:" << endl;
        for (unsigned int i=0; i<randoms; ++i) {
            int n;
            double p;
            cout << "\t" << i+1 << "/" << randoms << ": ";
            cin >> n >> p;
            Graph g(n);
            g.randomize(p);
            ostringstream oss;
            oss << "Random_" << i+1 << "_oo_" << randoms;
            dsg.add_graph(g, oss.str());
        }

        // Handle input files
        cout << "Any graphs given by file? [y/n]: ";
        if (get_answer()) {
            cout << "I can either scan directories recursively, or add"
                    " graphs by path." << endl;
            cout << "If recursive scan is selected, the user may add filters to the "
                    "recursive search (substrings which, if found, cause the path to "
                    "be skipped)." << endl;
            cout << "Scan recursively? [y/n]: ";

            // Recursive scan:
            if (get_answer()) {
                unsigned int root_dirs;
                cout << "How many root directories? ";
                cin >> root_dirs;
                for (unsigned int i=0; i<root_dirs; ++i) {
                    string root_dir;
                    cout << "Enter root directory #" << i+1 << ":" << endl;
                    cin >> root_dir;
                    DirectoryIterator di(root_dir);
                    cout << "Apply filters to subdirectories? [y/n]: ";
                    if (get_answer()) {
                        unsigned int total_filters;
                        cout << "How many filters? ";
                        cin >> total_filters;
                        cout << "Enter filters (separated by whitespace):" << endl;
                        for (unsigned int j=0; j<total_filters; ++j) {
                            string filter;
                            cin >> filter;
                            di.skip(filter);
                        }
                    }
                    dsg.add_graphs(di);
                }
            }
            else { // Specific files
                unsigned int total_files;
                cout << "How many files? ";
                cin >> total_files;
                cout << "Enter " << total_files << " files, separated by whitespace." << endl
                     << "Remember to escape chars:" << endl;
                for (unsigned int i=0; i<total_files; ++i) {
                    string filename;
                    cin >> filename;
                    dsg.add_graph(filename);
                }
            }
        }

        // That's it! Compute and output.
        cout << "Ready! Enter any character to start... ";
        char tmp;
        cin >> tmp;
        dsg.compute(true);
        dsg.print();
        return 0;
    }

    // Store the return value
    int return_val;
    // The main function to run
    int main_type;
public:

    // Go!
    Main(MainType mt = MAIN_PMC_TEST, int argc = 1, char* argv[] = NULL) :
                                        return_val(-1), main_type(mt) {
        try {
            switch(main_type) {
            case MAIN_TMP:
                return_val = tmp();
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
            case MAIN_INTERACTIVE:
                return_val = interactive();
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

    /**
     * Returns the help text as a string.
     */
    static string help() {
        return "USAGE: Executable may require write permissions (or the output won't be saved):\n"
                "\tmain [(-i|-d|-n <FUNCTION_NUM>)]\n"
                "Parameters:\n"
                "\t-i\tInteractive mode.\n"
                "\t-d\tDefault action (whatever's pre-coded).\n"
                "\t-n\tRequires additional numeric argument, representing the specific program to be run.\n"
                "Note that if MAIN_MIN_TRIANG_ENUM mode is chosen, the user must supply the arguments expected "
                "by triang_enum(argc,argv).\n";
    }
};

}

using namespace tdenum;

int main(int argc, char* argv[]) {
    srand(time(NULL)); // For random graphs
    int opt, main_type;
    while ((opt = getopt(argc,argv,"idn:")) != EOF) switch(opt) {
        case 'd':
            break;
        case 'n':
            main_type = atoi(optarg);
            if (main_type < 0 || main_type >= MAIN_LAST) {
                cout << "Bad parameter given to n: must be between 0 and " << MAIN_LAST-1
                     << ", n is " << main_type << endl;
                return -1;
            }
            else {
                return Main(MainType(main_type)).get();
            }
            break;
        case 'i':
            return Main(MAIN_INTERACTIVE).get();
            break;
        case '?':
            cout << "Bad parameters / missing argument." << endl << Main::help();
            break;
        default: cout<<endl; abort();
    }
    // No parameters. Use default
    return Main().get();
}








