#include "PMCAlg.h"
#include "Utils.h"
#include <sstream>

namespace tdenum {

//#define PMC_ALG_BASENAME string("PMC_ALG")
/*
// Numeric representation of each algorithm is done internally,
// s.t. the user can iterate over all algorithms from PMCAlg::first()
// to PMCAlg::last().
#define INT_IS_REVERSE(x) (x%16 >= 8)
#define INT_IS_PARALLEL(x) (x%8 >= 4)
#define INT_IS_DESCENDING(x) (x%4 == 3)
#define INT_IS_ASCENDING(x) (x%4 == 2)
#define INT_IS_RANDOMRENAME(x) (x%4 == 1)

#define INT_MAKE_REVERSE(x) do {(x += INT_IS_REVERSE(x) ? 0 : 8);} while(0)
#define INT_MAKE_PARALLEL(x) do {(x += INT_IS_PARALLEL(x) ? 0 : 4);} while(0)
#define INT_MAKE_DESCENDING(x) do {(x += INT_IS_DESCENDING(x) ? 0 : 3);} while(0)
#define INT_MAKE_ASCENDING(x) do {(x += INT_IS_ASCENDING(x) ? 0 : 2);} while(0)
#define INT_MAKE_RANDOMRENAME(x) do {(x += INT_IS_RANDOMRENAME(x) ? 0 : 1);} while(0)*/
/*
PMCAlg::PMCAlg(bool reverse_ms, bool parallel, bool descending, bool ascending, bool random_rename) {
    // Reverse
    alg = 0;
    if (reverse_ms) {
        PMCALG_SET_REVERSE(alg);
    }
    // Prioritize descending > ascending > random
    if (descending) {
        PMCALG_SET_DESCENDING(alg);
    }
    else if (ascending) {
        PMCALG_SET_ASCENDING(alg);
    }
    else if (random_rename) {
        PMCALG_SET_RANDOM_RENAME(alg);
    }
    if ((descending && ascending) || (descending && random_rename) || (ascending && random_rename)) {
        TRACE(TRACE_LVL__WARNING, "Bad arguments given, more than one rename scheme selected");
    }
    // Parallel
    if (parallel) {
        PMCALG_SET_PARALLEL_OMV(alg);
    }
}*/
PMCAlg::PMCAlg(PMCALG_ENUM a) : alg(a) {}
PMCAlg::PMCAlg(int alg_int) : PMCAlg(static_cast<PMCALG_ENUM>(alg_int)) {}
        /*PMCAlg(INT_IS_REVERSE(alg_int),
               INT_IS_PARALLEL(alg_int),
               INT_IS_DESCENDING(alg_int),
               INT_IS_ASCENDING(alg_int),
               INT_IS_RANDOMRENAME(alg_int))
               {} */

/*PMCAlg PMCAlg::from_bitmask(int mask) {
    return PMCAlg(PMCALG_IS_REVERSE(mask),
                  PMCALG_IS_PARALLEL_OMV(mask),
                  PMCALG_IS_DESCENDING(mask),
                  PMCALG_IS_ASCENDING(mask),
                  PMCALG_IS_RANDOM_RENAME(mask));
}*/

/*
void PMCAlg::set_reverse() { PMCALG_SET_REVERSE(alg); }
void PMCAlg::set_ascending() { PMCALG_SET_ASCENDING(alg); }
void PMCAlg::set_descending() { PMCALG_SET_DESCENDING(alg); }
void PMCAlg::set_random_rename() { PMCALG_SET_RANDOM_RENAME(alg); }
void PMCAlg::set_parallel() { PMCALG_SET_PARALLEL_OMV(alg); }

void PMCAlg::unset_reverse() { PMCALG_UNSET_REVERSE(alg); }
void PMCAlg::unset_node_rename() {
    PMCALG_UNSET_ASCENDING(alg);
    PMCALG_UNSET_DESCENDING(alg);
    PMCALG_UNSET_RANDOM_RENAME(alg);
}
void PMCAlg::unset_parallel() { PMCALG_UNSET_PARALLEL_OMV(alg); }
*/

bool PMCAlg::is_ascending() const { return PMCALG_IS_ASCENDING(alg); }
bool PMCAlg::is_descending() const { return PMCALG_IS_DESCENDING(alg); }
bool PMCAlg::is_sorted() const { return is_ascending() || is_descending(); }
bool PMCAlg::is_random_node_rename() const { return PMCALG_IS_RANDOM_RENAME(alg); }
bool PMCAlg::is_node_rename() const { return is_random_node_rename() || is_sorted(); }
bool PMCAlg::is_reverse() const { return PMCALG_IS_REVERSE(alg); }
bool PMCAlg::is_parallel() const { return PMCALG_IS_PARALLEL_OMV(alg); }
bool PMCAlg::is_normal() const { return !alg; }    // The algorithm is normal <==> alg==0

PMCAlg::operator int() const { return (int)alg; }
/*
    // We have 3 categories of independent algorithm properties.
    // The first two are binary (parallel / reverse), and the third
    // has 4 options (normal, descending, ascending, random).
    // A total of 16 algorithms.
    int alg_id = 0;
    if (is_reverse()) {
        INT_MAKE_REVERSE(alg_id);
    }
    if (is_parallel()) {
        INT_MAKE_PARALLEL(alg_id);
    }
    if (is_descending()) {
        INT_MAKE_DESCENDING(alg_id);
    }
    if (is_ascending()) {
        INT_MAKE_ASCENDING(alg_id);
    }
    if (is_random_node_rename()) {
        INT_MAKE_RANDOMRENAME(alg_id);
    }
    if (alg_id >= PMCAlg::last()) {
        TRACE(TRACE_LVL__WARNING, "Bad underlying algorithm!");
        return PMCALG_MASK_ALL;
    }
    return alg_id;
}*/
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
