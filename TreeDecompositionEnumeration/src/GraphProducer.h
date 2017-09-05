#ifndef GRAPHPRODUCER_H_INCLUDED
#define GRAPHPRODUCER_H_INCLUDED

#include "Graph.h"
#include "GraphReader.h"
#include "GraphStats.h"
#include "DirectoryIterator.h"
#include "Utils.h"
#include <vector>
using std::vector;

namespace tdenum {

/**
 * Class used to create graph statistic objects in batch jobs.
 */
class GraphProducer {
private:
    vector<GraphStats> graphs;

    // Textify a random graph
    string rand_str(unsigned n, double p, int instance) const;

    // When adding a graph, print out info
    bool verbose;
public:

    GraphProducer(bool v = false) : verbose(v) {}

    // Add graphs directly, by filename (default text is the filename),
    // in batch by providing a directory iterator (text will be the filenames),
    // random graphs and batch random graphs (each with text defined by rand_txt()).
    GraphProducer& add(const Graph& g, const string& text);
    GraphProducer& add(const string& filename, const string& txt = "");

    // Recursive search.
    // Allow user to send a directory iterator.
    // The second method scans the directory given by the user (recursively)
    // and adds all graphs found. Optionally add filters to the path strings
    // (graphs with at least one filter as a substring of the path won't be
    // added).
    GraphProducer& add_by_dir(DirectoryIterator di);
    GraphProducer& add_by_dir(const string& dir,
                    const vector<string>& filters = vector<string>());


    // Adds random graphs. The inputs are:
    // - A vector of graphs sizes (number of nodes)
    // - A vector of p values
    // If mix_match is false, for every i a graph will be sampled from
    // G(n[i],p[i]).
    // If mix_match is true, for every i and j a graph will be sampled
    // from G(n[i],p[j])
    GraphProducer& add_random(int n, double p, int instances = 1);
    GraphProducer& add_random(const vector<int>& n,
                    const vector<double>& p,
                    bool mix_match = false,
                    int instances = 1);

    // Adds random graphs. For each graph size (number of nodes) n,
    // samples a graph from G(n,k*step) for all k from k=1 to 1/step
    // (not including 1/step).
    // Allow the user to control the number of sampled instances for
    // each graph.
    GraphProducer& add_random_pstep(const vector<int>& n,
                          double step = 0.5,
                          int instances = 1);
    GraphProducer& add_random_pstep_range(const vector<int>& n,
                          double first = 0, double last = 1, double step = 0.5,
                          int instances = 1);

    // Generate graphs with 'interesting' run times for min seps / PMCs calculation.
    // Random graphs with 0.2<=p<=0.4 tend to yield large numbers of said objects.
    GraphProducer& hard_graphs(int instances = 1);

    // Returns the vector of graph statistic objects
    vector<GraphStats> get() const;

};


}

#endif // GRAPHPRODUCER_H_INCLUDED
