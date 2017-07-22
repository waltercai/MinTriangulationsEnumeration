#ifndef GRAPHSTATS_H_INCLUDED
#define GRAPHSTATS_H_INCLUDED

#include "Graph.h"
#include <string>

using std::string;

namespace tdenum {

class GraphStats {
public:
    Graph g;
    string text;
    int n;
    int m;
    long ms;
    long pmcs;
    long triangs;
    // Are the (n,m,ms,pmcs,triangs) fields valid for graph i?
    // Note: if the DSG didn't request PMCs, then even if valid
    // is true the PMCs should be zero.
    bool valid;
    // If the limit was overreached, set the relevant flag.
    bool ms_count_limit;
    bool ms_time_limit;
    bool trng_count_limit;
    bool trng_time_limit;
    bool pmc_time_limit;
    // The amount of time required for calculation the minimal separators.
    time_t ms_calc_time;
    time_t pmc_calc_time;
    time_t trng_calc_time;

    /**
     * Basic constructor
     */
    GraphStats(const Graph&, const string&);

};

}

#endif // GRAPHSTATS_H_INCLUDED
