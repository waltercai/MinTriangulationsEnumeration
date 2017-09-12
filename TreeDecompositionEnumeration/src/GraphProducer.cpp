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
string GraphProducer::rand_filename(unsigned n, double p, int instance) const {
    string filename = GRAPHPRODUCER_RANDFILE_EXPR;
    filename = utils__replace_substr_with_substr(filename, GRAPHPRODUCER_RANDFILE_TOKEN_N, UTILS__TO_STRING(n));
    filename = utils__replace_substr_with_substr(filename, GRAPHPRODUCER_RANDFILE_TOKEN_P, UTILS__TO_STRING(p));
    filename = utils__replace_substr_with_substr(filename, GRAPHPRODUCER_RANDFILE_TOKEN_INST, UTILS__TO_STRING(instance));
    return filename;
}

// Add graphs directly, by filename (default text is the filename),
// in batch by providing a directory iterator (text will be the filenames),
// random graphs and batch random graphs (each with text defined by rand_txt()).
GraphProducer& GraphProducer::add(const Graph& g,
                                  const string& text,
                                  bool is_random,
                                  double p,
                                  int instance,
                                  bool from_file) {
    if (verbose) {
        cout << "Adding graph '" << text << "'..." << endl;
    }
    graphs.push_back(GraphStats(g,text,is_random,p,instance,from_file));
    return *this;
}
GraphProducer& GraphProducer::add(const string& filename, const string& txt) {
    Graph g = GraphReader::read(filename);
    add(g,
        txt == "" ? filename : txt,
        false,
        0,
        1,
        true);
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
GraphProducer& GraphProducer::add_random(int n, double p, int instances) {
    for (int i=0; i<instances; ++i) {
        Graph g(n);
        g.randomize(p);
        add(g, rand_str(n,p,i+1), true, p, i+1);
    }
    return *this;
}
GraphProducer& GraphProducer::add_random(const vector<int>& n,
                               const vector<double>& p,
                               bool mix_match,
                               int instances)
{
    if (!mix_match) {
        if (n.size() != p.size()) {
            TRACE(TRACE_LVL__ERROR, "Invalid arguments (" << n.size() << " graphs requested, "
                 << p.size() << " probabilities sent)");
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
GraphProducer& GraphProducer::add_random_pstep_range(const vector<int>& n,
                                     double first, double last, double step,
                                     int instances)
{
    if (step >= 1-EPSILON || step <= EPSILON) {
        TRACE(TRACE_LVL__ERROR, "Step value must be 0<step<1 (is " << step << ")");
        return *this;
    }
    if (first >= 1-EPSILON || last <= EPSILON) {
        TRACE(TRACE_LVL__ERROR, "Bounds must make sense (got " << first << "<=p<=" << last << ")");
        return *this;
    }
    // We can allow these
    if (first <= EPSILON) {
        first = step;
    }
    if (last >= 1-EPSILON) {
        last = 1-EPSILON;
    }
    for (unsigned i=0; i<n.size(); ++i) {
        for (unsigned j=0; first+(double(j))*step < last; ++j) {
            add_random(n[i], first+((double)j)*step, instances);
        }
    }
    return *this;
}
GraphProducer& GraphProducer::add_random_pstep(const vector<int>& n,
                                     double step,
                                     int instances)
{
    return add_random_pstep_range(n, step, 1, step, instances);
}

GraphProducer& GraphProducer::hard_graphs(int instances) {
    // p=0.2~0.4
    for (int n: utils__vector_range(25,35)) {
        add_random(n,0.2,instances);
        add_random(n,0.3,instances);
        add_random(n,0.4,instances);
    }
    // p=0.1, 0.6~0.9
    for (int n: utils__vector_range(50,60)) {
        add_random(n,0.1,instances);
        add_random(n,0.6,instances);
        add_random(n,0.7,instances);
        add_random(n,0.8,instances);
        add_random(n,0.9,instances);
    }
    return *this;
}

// Returns the vector of graph statistic objects
vector<GraphStats> GraphProducer::get() const {
    return graphs;
}

GraphProducer& GraphProducer::dump_graphs(bool skip_graphs_from_files) {
    for (GraphStats gs: graphs) {
        if (skip_graphs_from_files && gs.from_file) {
            continue;
        }
        string filename = gs.is_random ? output_dir + rand_filename(gs.n, gs.p, gs.instance) : gs.text;
        GraphReader::dump(gs.g, filename);
    }
    return *this;
}

}
