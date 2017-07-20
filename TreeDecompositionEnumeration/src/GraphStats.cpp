#include "GraphStats.h"
#include "Graph.h"

namespace tdenum {

GraphStats::GraphStats(const Graph& graph, const string& s) :
                       g(graph), text(s), ms(0), pmcs(0), triangs(0),
                       valid(false), ms_count_limit(false), ms_time_limit(false),
                       trng_count_limit(false), trng_time_limit(false),
                       pmc_time_limit(false) {
    n = g.getNumberOfNodes();
    m = g.getNumberOfEdges();
}

}
