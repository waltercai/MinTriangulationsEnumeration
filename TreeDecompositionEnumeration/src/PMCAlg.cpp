#include "PMCAlg.h"
#include "Utils.h"
#include <sstream>

namespace tdenum {

PMCAlg::PMCAlg(PMCALG_ENUM a) : alg(a) {}
PMCAlg::PMCAlg(int alg_int) : PMCAlg(static_cast<PMCALG_ENUM>(alg_int)) {}

bool PMCAlg::is_ascending() const { return PMCALG_IS_ASCENDING(alg); }
bool PMCAlg::is_descending() const { return PMCALG_IS_DESCENDING(alg); }
bool PMCAlg::is_sorted() const { return is_ascending() || is_descending(); }
bool PMCAlg::is_random_node_rename() const { return PMCALG_IS_RANDOM_RENAME(alg); }
bool PMCAlg::is_node_rename() const { return is_random_node_rename() || is_sorted(); }
bool PMCAlg::is_reverse() const { return PMCALG_IS_REVERSE(alg); }
bool PMCAlg::is_parallel() const { return PMCALG_IS_PARALLEL_OMV(alg); }
bool PMCAlg::is_normal() const { return !alg; }    // The algorithm is normal <==> alg==0

PMCAlg::operator int() const { return (int)alg; }
PMCAlg& PMCAlg::operator++() { return (*this = PMCAlg((int(*this))+1)); }
PMCAlg PMCAlg::operator++(int) {
    PMCAlg ret = *this;
    ++(*this);
    return ret;
}

int PMCAlg::first() {
    return 0;
}
int PMCAlg::last() {
    return PMCALG_ENUM_LAST;
}
int PMCAlg::total_algs() { return last(); }
const vector<PMCAlg> PMCAlg::get_all(bool csv_format) {
    vector<PMCAlg> v;
    if (!csv_format) {
        for (int i=first(); i<last(); ++i) {
            v.push_back(PMCAlg(i));
        }
    }
    else {
        #define Y(ID) v.push_back(PMCAlg(PMCALG_ENUM_##ID));
        PMCALG_ALGORITHM_TABLE
        #undef Y
    }
    return v;
}
const set<PMCAlg> PMCAlg::get_all_set(bool csv_format) {
    vector<PMCAlg> v = get_all(csv_format);
    return set<PMCAlg>(v.begin(), v.end());
}

string PMCAlg::str() const { return PMCALG_NUM_TO_STR_MAP.at(alg); }

}
