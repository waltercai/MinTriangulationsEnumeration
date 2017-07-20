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
    int ms;
    int pmcs;
    int triangs;
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
    // The amount of time required to calculate the minimal separators.
    string ms_calc_time;

    /**
     * Basic constructor
     */
    GraphStats(const Graph&, const string&);

};

}
