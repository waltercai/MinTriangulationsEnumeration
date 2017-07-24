#include "GraphStats.h"
#include "Graph.h"

namespace tdenum {

GraphStats::GraphStats() : GraphStats(Graph(), "") {}
GraphStats::GraphStats(const Graph& graph, const string& s) :
                       g(graph),
                       text(s),
                       n(g.getNumberOfNodes()),
                       m(g.getNumberOfEdges()),
                       ms(0),
                       pmcs(0),
                       triangs(0),
                       valid(false),
                       ms_count_limit(false),
                       ms_time_limit(false),
                       trng_count_limit(false),
                       trng_time_limit(false),
                       pmc_time_limit(false),
                       ms_calc_time(0),
                       pmc_calc_time(0)
{}

}
