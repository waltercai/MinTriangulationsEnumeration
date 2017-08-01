#ifndef PMCALG_H_INCLUDED
#define PMCALG_H_INCLUDED

#include <string>
using std::string;

namespace tdenum {

/**
 * Algorithms used by the PMC enumerator.
 */
class PMCAlg {
private:
    // Different algorithm strains are allowed.
    // The all_mask value doubles as an invalid algorithm value.
    static const int all_mask = -1;            // (all bits are 1)
    static const int reverse_ms_mask = 1;      // Noam & Dori's heuristic
    static const int ascending_deg_mask = 2*reverse_ms_mask;     // These three are
    static const int descending_deg_mask = 2*ascending_deg_mask; // mutually
    static const int random_rename_mask = 2*descending_deg_mask; // exclusive
    static const int parallel_omv_mask = 2*random_rename_mask;   // Async version of one_more_vertex

    // The actual algorithm
    int alg;

public:

    // If more than one of the node-rename modes is true, prefer descending, then ascending,
    // then random.
    // Print a warning anyway.
    // Also allow construction via the integer returned by operator int() (NOT by bitmask!)
    PMCAlg(bool reverse_ms = false,
           bool parallel = false,
           bool descending = false,
           bool ascending = false,
           bool random_rename = false);
    PMCAlg(int int_alg);

    // Edit the algorithm
    void set_reverse();
    void unset_reverse();
    void set_ascending();
    void set_descending();
    void set_random_rename();
    void unset_node_rename();
    void set_parallel();
    void unset_parallel();

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
    static int first();
    static int last();

    // Translate to string (human-readable)
    string str() const;

};

}

#endif // PMCALG_H_INCLUDED
