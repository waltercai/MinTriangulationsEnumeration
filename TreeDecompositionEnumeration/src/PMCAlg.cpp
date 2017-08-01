#include "PMCAlg.h"
#include "Utils.h"
#include <sstream>

namespace tdenum {

#define PMC_ALG_BASENAME string("PMC_ALG")

#define INT_IS_REVERSE(x) (x%16 >= 8)
#define INT_IS_PARALLEL(x) (x%8 >= 4)
#define INT_IS_DESCENDING(x) (x%4 == 3)
#define INT_IS_ASCENDING(x) (x%4 == 2)
#define INT_IS_RANDOMRENAME(x) (x%4 == 1)

#define INT_MAKE_REVERSE(x) do {(x += INT_IS_REVERSE(x) ? 0 : 8);} while(0)
#define INT_MAKE_PARALLEL(x) do {(x += INT_IS_PARALLEL(x) ? 0 : 4);} while(0)
#define INT_MAKE_DESCENDING(x) do {(x += INT_IS_DESCENDING(x) ? 0 : 3);} while(0)
#define INT_MAKE_ASCENDING(x) do {(x += INT_IS_ASCENDING(x) ? 0 : 2);} while(0)
#define INT_MAKE_RANDOMRENAME(x) do {(x += INT_IS_RANDOMRENAME(x) ? 0 : 1);} while(0)

PMCAlg::PMCAlg(bool reverse_ms, bool parallel, bool descending, bool ascending, bool random_rename) {
    // Reverse
    alg = 0;
    if (reverse_ms) {
        alg |= PMCAlg::reverse_ms_mask;
    }
    // Prioritize descending > ascending > random
    if (descending) {
        alg |= PMCAlg::descending_deg_mask;
    }
    else if (ascending) {
        alg |= PMCAlg::ascending_deg_mask;
    }
    else if (random_rename) {
        alg |= PMCAlg::random_rename_mask;
    }
    if ((descending && ascending) || (descending && random_rename) || (ascending && random_rename)) {
        TRACE(TRACE_LVL__WARNING, "Bad arguments given, more than one rename scheme selected");
    }
    // Parallel
    if (parallel) {
        alg |= PMCAlg::parallel_omv_mask;
    }
}
PMCAlg::PMCAlg(int alg_int) : // See operator int()
        PMCAlg(INT_IS_REVERSE(alg_int),
               INT_IS_PARALLEL(alg_int),
               INT_IS_DESCENDING(alg_int),
               INT_IS_ASCENDING(alg_int),
               INT_IS_RANDOMRENAME(alg_int)) {}

void PMCAlg::set_reverse() {
    alg |= PMCAlg::reverse_ms_mask;
}
void PMCAlg::unset_reverse() {
    alg &= (PMCAlg::all_mask ^ PMCAlg::reverse_ms_mask);
}
void PMCAlg::set_ascending() {
    alg |= PMCAlg::ascending_deg_mask;
}
void PMCAlg::set_descending() {
    alg |= PMCAlg::descending_deg_mask;
}
void PMCAlg::set_random_rename() {
    alg |= PMCAlg::random_rename_mask;
}
void PMCAlg::unset_node_rename() {
    alg &= (PMCAlg::all_mask ^ (PMCAlg::ascending_deg_mask | PMCAlg::descending_deg_mask | PMCAlg::random_rename_mask));
}
void PMCAlg::set_parallel() {
    alg |= PMCAlg::parallel_omv_mask;
}
void PMCAlg::unset_parallel() {
    alg &= (PMCAlg::all_mask ^ PMCAlg::parallel_omv_mask);
}

bool PMCAlg::is_ascending() const {
    return alg & PMCAlg::ascending_deg_mask;
}
bool PMCAlg::is_descending() const {
    return alg & PMCAlg::descending_deg_mask;
}
bool PMCAlg::is_sorted() const {
    return is_ascending() || is_descending();
}
bool PMCAlg::is_random_node_rename() const {
    return alg & PMCAlg::random_rename_mask;
}
bool PMCAlg::is_node_rename() const {
    return is_random_node_rename() || is_sorted();
}
bool PMCAlg::is_reverse() const {
    return alg & PMCAlg::reverse_ms_mask;
}
bool PMCAlg::is_parallel() const {
    return alg & PMCAlg::parallel_omv_mask;
}
bool PMCAlg::is_normal() const {
    return !alg;    // The algorithm is normal <==> alg==0
}

PMCAlg::operator int() const {
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
        return PMCAlg::all_mask;
    }
    return alg_id;
}

int PMCAlg::first() {
    return 0;
}
int PMCAlg::last() {
    return 16;
}

string PMCAlg::str() const {
    if (is_normal()) {
        return PMC_ALG_BASENAME + "_NORMAL";
    }
    std::ostringstream oss;
    oss << PMC_ALG_BASENAME;
    if (is_reverse()) {
        oss << "_REVERSEMS";
    }
    if (is_parallel()) {
        oss << "_PARALLEL";
    }
    if (is_ascending()) {
        oss << "_ASCENDINGDEG";
    }
    if (is_descending()) {
        oss << "_DESCENDINGDEG";
    }
    if (is_random_node_rename()) {
        oss << "_RANDOMRENAME";
    }
    return oss.str();
}

}
