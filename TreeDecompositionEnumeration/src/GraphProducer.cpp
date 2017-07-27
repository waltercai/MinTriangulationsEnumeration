#include "GraphProducer.h"
#include "GraphReader.h"
#include <string>
#include <sstream>

namespace tdenum {

string GraphProducer::rand_str(unsigned n, double p, int instance) const {
    ostringstream oss;
    oss << "G(" << n << ":" << p << "); instance " << instance;
    return oss.str();
}

// Add graphs directly, by filename (default text is the filename),
// in batch by providing a directory iterator (text will be the filenames),
// random graphs and batch random graphs (each with text defined by rand_txt()).
GraphProducer& GraphProducer::add(const Graph& g, const string& text) {
    graphs.push_back(GraphStats(g,text));
    return *this;
}
GraphProducer& GraphProducer::add(const string& filename, const string& txt) {
    Graph g = GraphReader::read(filename);
    add(g, txt == "" ? filename : txt);
    return *this;
}

// Recursive search.
// Allow user to send a directory iterator.
// The second method scans the directory given by the user (recursively)
// and adds all graphs found. Optionally add filters to the path strings
// (graphs with at least one filter as a substring of the path won't be
// added).
GraphProducer& GraphProducer::add_by_dir(DirectoryIterator di) {
    string dataset_filename;
    while(di.next_file(&dataset_filename)) {
        add(dataset_filename);
    }
    return *this;
}
GraphProducer& GraphProducer::add_by_dir(const string& dir,
                               const vector<string>& filters)
{
    DirectoryIterator di(dir);
    for (unsigned i=0; i<filters.size(); ++i) {
        di.skip(filters[i]);
    }
    add_by_dir(di);
    return *this;
}


// Adds random graphs. The inputs are:
// - A vector of graphs sizes (number of nodes)
// - A vector of p values
// If mix_match is false, for every i a graph will be sampled from
// G(n[i],p[i]).
// If mix_match is true, for every i and j a graph will be sampled
// from G(n[i],p[j])
GraphProducer& GraphProducer::add_random(unsigned n, double p, int instances) {
    for (int i=0; i<instances; ++i) {
        Graph g(n);
        g.randomize(p);
        add(g, rand_str(n,p,i+1));
    }
    return *this;
}
GraphProducer& GraphProducer::add_random(const vector<unsigned int>& n,
                               const vector<double>& p,
                               bool mix_match,
                               unsigned instances)
{
    if (!mix_match) {
        if (n.size() != p.size()) {
            cout << "Invalid arguments (" << n.size() << " graphs requested, "
                 << p.size() << " probabilities sent)" << endl;
            return *this;
        }
        for (unsigned i=0; i<n.size(); ++i) {
            add_random(n[i], p[i], instances);
        }
    }
    else {
        for (unsigned i=0; i<n.size(); ++i) {
            for (unsigned j=0; j<p.size(); ++j) {
                add_random(n[i], p[j], instances);
            }
        }
    }
    return *this;
}


// Adds random graphs. For each graph size (number of nodes) n,
// samples a graph from G(n,k*step) for all k from k=1 to 1/step
// (not including 1/step).
// Allow the user to control the number of sampled instances for
// each graph.
GraphProducer& GraphProducer::add_random_pstep(const vector<unsigned int>& n,
                                     double step,
                                     int instances)
{
    if (step >= 1 || step <= 0) {
        cout << "Step value must be 0<step<1 (is " << step << ")" << endl;
        return *this;
    }
    for (unsigned i=0; i<n.size(); ++i) {
        for (unsigned j=1; (double(j))*step < 1; ++j) {
            add_random(n[i], j*step, instances);
        }
    }
    return *this;
}

// Returns the vector of graph statistic objects
vector<GraphStats> GraphProducer::get() const {
    return graphs;
}

}
