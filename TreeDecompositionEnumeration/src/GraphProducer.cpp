//#include "DatasetStatisticsGenerator.h"
#include "GraphProducer.h"
#include "GraphReader.h"
#include <string>
#include <sstream>

namespace tdenum {

GraphProducer::GraphProducer(const string& dir, bool v) :
            verbose(v),
            output_dir(utils__str_empty(dir) ? string(".") : dir)
            {}
GraphProducer& GraphProducer::reset(const string& dir, bool v) { return (*this = GraphProducer(dir,v)); }


// Add graphs directly, by filename (default text is the filename),
// in batch by providing a directory iterator (text will be the filenames),
// random graphs and batch random graphs (each with text defined by rand_txt()).
GraphProducer& GraphProducer::add(const GraphStats& gs) {
    graphs.push_back(gs);
    return *this;
}
GraphProducer& GraphProducer::add(const vector<GraphStats>& vgs) {
    for (auto gs: vgs) {
        add(gs);
    }
    return *this;
}
GraphProducer& GraphProducer::add(const string& filepath) {
    GraphStats gs = GraphStats::read(filepath);
    if (!gs.integrity()) {
        TRACE(TRACE_LVL__ERROR, "Error reading '" << filepath << "', no GS object added");
        if (utils__dir_exists(filepath)) {
            TRACE(TRACE_LVL__ERROR, "Called add('" << filepath << "'), but the path points to a directory... " <<
                                    "Did you mean to call add_dir()?");
        }
        return *this;
    }
    return add(gs);
}

// Recursive search.
// Allow user to send a directory iterator.
// The second method scans the directory given by the user (recursively)
// and adds all graphs found. Optionally add filters to the path strings
// (graphs with at least one filter as a substring of the path won't be
// added).
GraphProducer& GraphProducer::add(DirectoryIterator di) {
    string dataset_filename;
    while(di.next_file(dataset_filename)) {
        add(dataset_filename);
    }
    return *this;
}
GraphProducer& GraphProducer::add_dir(const string& dirpath) { return add(DirectoryIterator(dirpath)); }


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
        g.randomize(p, i+1);
        GraphStats gs(g);
        add(gs);
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

vector<GraphStats> GraphProducer::get() const { return graphs; }
vector<string> GraphProducer::get_paths() const {
    vector<string> paths;
    for (const GraphStats& gs: graphs) {
        if (gs.text_valid()) {
            paths.push_back(gs.get_text());
        }
    }
    return paths;
}

GraphProducer& GraphProducer::dump_graphs(bool skip_graphs_from_files) {
    // Iterate using NON CONST references to make sure the text fields are updated
    // in the GS objects.
    // Each call to dump() may update the GS internal data
    for (GraphStats& gs: graphs) {
        string out_file = gs.get_dump_filepath(output_dir);
        if (skip_graphs_from_files && (gs.integrity(out_file) || gs.integrity())) {
            TRACE(TRACE_LVL__TEST, "File '" << out_file << "' exists and is consistent, skipping...");
            continue;
        }
        else {
            TRACE(TRACE_LVL__TEST, "Dumping graph '" << gs << "'");
            if (!gs.dump(out_file)) {
                TRACE(TRACE_LVL__ERROR, "Couldn't dump graph to '" << out_file << "'... should be:" << endl << gs);
            }
        }
    }
    return *this;
}

}
