#ifndef PMCALG_H_INCLUDED
#define PMCALG_H_INCLUDED

#include <map>
#include <set>
#include <string>
#include <vector>
using std::map;
using std::set;
using std::string;
using std::vector;

namespace tdenum {

// Define these as Y macros, as they are used in conjunction with
// X macros in Dataset.h
#define PMCALG_ALGORITHM_TABLE \
    Y(NORMAL) \
    Y(REVERSE_MS) \
    Y(ASCENDING) \
    Y(ASCENDING_REVERSE_MS) \
    Y(DESCENDING) \
    Y(DESCENDING_REVERSE_MS) \
    Y(RANDOM_RENAME) \
    Y(RANDOM_RENAME_REVERSE_MS) \
    Y(PARALLEL_OMV) \
    Y(PARALLEL_OMV_REVERSE_MS) \
    Y(PARALLEL_OMV_ASCENDING) \
    Y(PARALLEL_OMV_ASCENDING_REVERSE_MS) \
    Y(PARALLEL_OMV_DESCENDING) \
    Y(PARALLEL_OMV_DESCENDING_REVERSE_MS) \
    Y(PARALLEL_OMV_RANDOM_RENAME) \
    Y(PARALLEL_OMV_RANDOM_RENAME_REVERSE_MS)

// Basic const values
typedef enum _PMCALG_ENUM {
    #define Y(ID) PMCALG_ENUM_##ID,
    PMCALG_ALGORITHM_TABLE
    #undef Y
    PMCALG_ENUM_LAST
} PMCALG_ENUM;

#define Y(ID) {PMCALG_ENUM_##ID, "PMCALG_" #ID},
const map<PMCALG_ENUM,string> PMCALG_NUM_TO_STR_MAP = { PMCALG_ALGORITHM_TABLE };
#undef Y

// Bitmask set/clear/test
#define PMCALG_IS_REVERSE(num) ( \
    num == PMCALG_ENUM_REVERSE_MS || \
    num == PMCALG_ENUM_ASCENDING_REVERSE_MS || \
    num == PMCALG_ENUM_DESCENDING_REVERSE_MS || \
    num == PMCALG_ENUM_RANDOM_RENAME_REVERSE_MS || \
    num == PMCALG_ENUM_PARALLEL_OMV_REVERSE_MS || \
    num == PMCALG_ENUM_PARALLEL_OMV_ASCENDING_REVERSE_MS || \
    num == PMCALG_ENUM_PARALLEL_OMV_DESCENDING_REVERSE_MS || \
    num == PMCALG_ENUM_PARALLEL_OMV_RANDOM_RENAME_REVERSE_MS \
    )
#define PMCALG_IS_ASCENDING(num) ( \
    num == PMCALG_ENUM_ASCENDING_REVERSE_MS || \
    num == PMCALG_ENUM_PARALLEL_OMV_ASCENDING_REVERSE_MS || \
    num == PMCALG_ENUM_ASCENDING || \
    num == PMCALG_ENUM_PARALLEL_OMV_ASCENDING \
    )
#define PMCALG_IS_DESCENDING(num) ( \
    num == PMCALG_ENUM_DESCENDING_REVERSE_MS || \
    num == PMCALG_ENUM_PARALLEL_OMV_DESCENDING_REVERSE_MS || \
    num == PMCALG_ENUM_DESCENDING || \
    num == PMCALG_ENUM_PARALLEL_OMV_DESCENDING \
    )
#define PMCALG_IS_RANDOM_RENAME(num) ( \
    num == PMCALG_ENUM_RANDOM_RENAME || \
    num == PMCALG_ENUM_RANDOM_RENAME_REVERSE_MS || \
    num == PMCALG_ENUM_PARALLEL_OMV_RANDOM_RENAME || \
    num == PMCALG_ENUM_PARALLEL_OMV_RANDOM_RENAME_REVERSE_MS \
    )
#define PMCALG_IS_PARALLEL_OMV(num) ( \
    num == PMCALG_ENUM_PARALLEL_OMV || \
    num == PMCALG_ENUM_PARALLEL_OMV_REVERSE_MS || \
    num == PMCALG_ENUM_PARALLEL_OMV_ASCENDING || \
    num == PMCALG_ENUM_PARALLEL_OMV_ASCENDING_REVERSE_MS || \
    num == PMCALG_ENUM_PARALLEL_OMV_DESCENDING || \
    num == PMCALG_ENUM_PARALLEL_OMV_DESCENDING_REVERSE_MS || \
    num == PMCALG_ENUM_PARALLEL_OMV_RANDOM_RENAME || \
    num == PMCALG_ENUM_PARALLEL_OMV_RANDOM_RENAME_REVERSE_MS \
    )
/*
#define PMCALG_SET_REVERSE(_mask) do { (_mask |= PMCALG_MASK_REVERSE); } while(0)
#define PMCALG_SET_ASCENDING(_mask) do { (_mask |= PMCALG_MASK_ASCENDING); } while(0)
#define PMCALG_SET_DESCENDING(_mask) do { (_mask |= PMCALG_MASK_DESCENDING); } while(0)
#define PMCALG_SET_RANDOM_RENAME(_mask) do { (_mask |= PMCALG_MASK_RANDOM_RENAME); } while(0)
#define PMCALG_SET_PARALLEL_OMV(_mask) do { (_mask |= PMCALG_MASK_PARALLEL_OMV); } while(0)

#define PMCALG_UNSET_REVERSE(_mask) do { (_mask &= (PMCALG_MASK_ALL ^ PMCALG_MASK_REVERSE)); } while(0)
#define PMCALG_UNSET_ASCENDING(_mask) do { (_mask &= (PMCALG_MASK_ALL ^ PMCALG_MASK_ASCENDING)); } while(0)
#define PMCALG_UNSET_DESCENDING(_mask) do { (_mask &= (PMCALG_MASK_ALL ^ PMCALG_MASK_DESCENDING)); } while(0)
#define PMCALG_UNSET_RANDOM_RENAME(_mask) do { (_mask &= (PMCALG_MASK_ALL ^ PMCALG_MASK_RANDOM_RENAME)); } while(0)
#define PMCALG_UNSET_PARALLEL_OMV(_mask) do { (_mask &= (PMCALG_MASK_ALL ^ PMCALG_MASK_PARALLEL_OMV)); } while(0)
*/
/**
 * Useful utility (algorithm set:
 */
#define PMCALG_CREATE_SET(...) UTILS__CREATE_SET(PMCAlg, __VA_ARGS__ )

/**
 * Algorithms used by the PMC enumerator.
 */
class PMCAlg {
private:

    // The actual algorithm
    PMCALG_ENUM alg;

public:

    // If more than one of the node-rename modes is true, prefer descending, then ascending,
    // then random.
    // Print a warning anyway.
    // Also allow construction via the integer returned by operator int() (NOT by bitmask!)
/*    PMCAlg(bool reverse_ms = false,
           bool parallel = false,
           bool descending = false,
           bool ascending = false,
           bool random_rename = false);*/
    PMCAlg(PMCALG_ENUM a = static_cast<PMCALG_ENUM>(0));
    PMCAlg(int int_alg);
//    static PMCAlg from_bitmask(int mask);

/*    // Edit the algorithm
    void set_reverse();
    void unset_reverse();
    void set_ascending();
    void set_descending();
    void set_random_rename();
    void unset_node_rename();
    void set_parallel();
    void unset_parallel();
*/
    // Query the algorithms
    bool is_ascending() const;
    bool is_descending() const;
    bool is_sorted() const; // One of the above two
    bool is_random_node_rename() const;
    bool is_node_rename() const; // One of the three
    bool is_reverse() const;
    bool is_parallel() const;
    bool is_normal() const; // No bits set

    // Allow conversion to an integer value between 0 and n-1, where n is the
    // total number of possible algorithms.
    // To allow iteration, add static methods indicating first and last values
    // (return 0 and n, respectively).
    operator int() const;
    PMCAlg& operator++();
    PMCAlg operator++(int);
    static int first();
    static int last();

    // If csv_format is set to true, the returned vector is the same one as used
    // by the Dataset class for CSV output.
    static const vector<PMCAlg> get_all(bool csv_format=false);
    static const set<PMCAlg> get_all_set(bool csv_format=false);
    static int total_algs();

    // Translate to string (human-readable)
    string str() const;

};

}

#endif // PMCALG_H_INCLUDED
