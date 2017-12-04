#ifndef GRAPHPRODUCER_H_INCLUDED
#define GRAPHPRODUCER_H_INCLUDED

#include "DirectoryIterator.h"
#include "Graph.h"
#include "GraphReader.h"
#include "GraphStats.h"
#include "Utils.h"
#include <vector>
using std::vector;

namespace tdenum {


/**
 * Class used to create graph statistic objects in batch jobs.
 *
 * Handles batch construction / reading / dumping of GraphStats objects.
 *
 * By default, dumping is done into the given output directory (required for
 * construction). However, if GraphStats instances loaded into a GraphProducer
 * already exist on disk (even if it's NOT IN THE OUTPUT DIR) they WILL NOT
 * be dumped again!
 */
class GraphProducer {
private:

    friend class GraphProducerTester;

    vector<GraphStats> graphs;

    // When adding a graph, print out info
    bool verbose;

    // When outputting to file, dump here
    string output_dir;

public:

    // The given directory is used to contain dumped files
    GraphProducer(const string& dir = ".", bool v = false);

    // Reset
    GraphProducer& reset(const string& dir = ".", bool v = false);

    // Add graphs directly, by filename (default text is the filename),
    // in batch by providing a directory iterator (text will be the filenames),
    // random graphs and batch random graphs (each with text defined by rand_txt()).
    GraphProducer& add(const GraphStats& gs);
    GraphProducer& add(const vector<GraphStats>& vgs);
    GraphProducer& add(const string& filepath);

    // Recursive search.
    GraphProducer& add(DirectoryIterator di);
    GraphProducer& add_dir(const string& dirpath);

    // Adds random graphs. The inputs are:
    // - A vector of graphs sizes (number of nodes)
    // - A vector of p values
    // If mix_match is false, for every i a graph will be sampled from
    // G(n[i],p[i]).
    // If mix_match is true, for every i and j a graph will be sampled
    // from G(n[i],p[j]). The resulting vector will be sorted by n, then
    // by p, then in ascending instance order.
    GraphProducer& add_random(int n, double p, int instances = 1);
    GraphProducer& add_random(const vector<int>& n,
                    const vector<double>& p,
                    bool mix_match = false,
                    int instances = 1);

    // Adds random graphs. For each graph size (number of nodes) n,
    // samples a graph from G(n,p) for all p from p=step to p=k*step where k*step<1.
    // Allow the user to control the number of sampled instances for each graph.
    GraphProducer& add_random_pstep(const vector<int>& n,
                          double step = 0.5,
                          int instances = 1);
    GraphProducer& add_random_pstep_range(const vector<int>& n,
                          double first = 0, double last = 1, double step = 0.5,
                          int instances = 1);

    // Returns the vector of graph statistic objects, or a vector of paths to filenames.
    // Behavior when the text field is invalid is undefined!
    // For now, graphs with no known source files are skipped.
    vector<GraphStats> get() const;
    vector<string> get_paths() const;

    // Dumps the graphs to files.
    //
    // If skip_graphs_from_files is true, and GS object that supports gs.integrity()==true
    // will NOT be dumped.
    // The reason we allow this is because sometimes graph files are very large, and there's
    // no point in dumping the same one in a different location; However, sometimes the user
    // would like all of the graphs in a given GraphProducer to appear in the same directory.
    // If this is the case, set skip_graphs_from_files to FALSE.
    GraphProducer& dump_graphs(bool skip_graphs_from_files = true);


};


}

#endif // GRAPHPRODUCER_H_INCLUDED
