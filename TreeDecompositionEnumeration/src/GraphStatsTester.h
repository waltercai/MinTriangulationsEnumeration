#include "TestInterface.h"

namespace tdenum {

/**
 * Most of the functionality of GraphStats is better tested
 * via classes that use it (we won't be counting PMCs in this tester).
 *
 * Basic creation and destruction, IO integrity, correct flags set...
 */

#define GRAPHSTATSTESTER_TEST_TABLE \
    X(construction) \
    X(construct_random) \
    X(read_simple) \
    X(read_nonexisting) \
    X(read_different_extensions) \
    X(read_and_dump) \
    X(read_and_dump_different_file) \
    X(dump_without_ext) \
    X(dump_random) \
    X(stats_basic_and_io) \
    X(stats_random) \
    X(stats_errors_compound) \
    X(stats_errors_time) \
    X(stats_errors_count) \
    X(stats_errors_mem) \
    X(stats_object_count) \
    X(stats_objects) \
    X(stats_incremental) \
    X(stats_time_declarations) \
    X(integrity_no_path_default_doesnt_exist) \
    X(integrity_no_path_or_dir_default_exists) \
    X(integrity_no_filename_default_doesnt_exist) \
    X(integrity_no_filename_default_exists) \
    X(integrity_file_doesnt_exist) \
    X(integrity_file_exists) \
    X(read_grp) \
    X(read_grp_and_dump) \
    X(read_non_grp) \
    X(read_non_grp_and_dump) \
    X(construct_read_no_input) \
    X(construct_g_nonames) \
    X(construct_g_nonames_exists)

// General equality macro, so we can catch which field is bad
#define ASSERT_GS_EQ(_gs1,_gs2) \
    ASSERT_EQ(_gs1.g, _gs2.g); \
    ASSERT_EQ(_gs1.count_ms, _gs2.count_ms); \
    ASSERT_EQ(_gs1.count_pmc, _gs2.count_pmc); \
    ASSERT_EQ(_gs1.count_trng, _gs2.count_trng); \
    ASSERT_EQ(_gs1.reached_count_limit_flag_ms, _gs2.reached_count_limit_flag_ms); \
    ASSERT_EQ(_gs1.reached_count_limit_flag_trng, _gs2.reached_count_limit_flag_trng); \
    ASSERT_EQ(_gs1.reached_time_limit_flag_ms, _gs2.reached_time_limit_flag_ms); \
    ASSERT_EQ(_gs1.reached_time_limit_flag_trng, _gs2.reached_time_limit_flag_trng); \
    ASSERT_EQ(_gs1.mem_error_flag_ms, _gs2.mem_error_flag_ms); \
    ASSERT_EQ(_gs1.mem_error_flag_trng, _gs2.mem_error_flag_trng); \
    ASSERT_EQ(_gs1.calc_time_ms, _gs2.calc_time_ms); \
    ASSERT_EQ(_gs1.calc_time_trng, _gs2.calc_time_trng); \
    ASSERT_EQ(_gs1.ms, _gs2.ms); \
    ASSERT_EQ(_gs1.pmc, _gs2.pmc); \
    ASSERT_EQ(_gs1.trng, _gs2.trng); \
    ASSERT_EQ(_gs1.reached_time_limit_flag_pmc, _gs2.reached_time_limit_flag_pmc); \
    ASSERT_EQ(_gs1.mem_error_flag_pmc, _gs2.mem_error_flag_pmc); \
    ASSERT_EQ(_gs1.calc_time_by_alg_pmc, _gs2.calc_time_by_alg_pmc)
// The other way doesn't work
#define ASSERT_GS_NEQ(_gs1,_gs2) \
    ASSERT_NEQ(_gs1, _gs2)


class GraphStatsTester : public TestInterface {
private:

    // Out of the total amount of existing tests, set those in the given
    // range [first,last)
    static const int total_tests_defined;
    GraphStatsTester& set_range(int first, int last);

    #define X(test) bool flag_##test;
    GRAPHSTATSTESTER_TEST_TABLE
    #undef X
    GraphStatsTester& go();
public:
    GraphStatsTester();
    #define X(test) \
    GraphStatsTester& set_##test(); \
    GraphStatsTester& unset_##test(); \
    GraphStatsTester& set_only_##test(); \
    bool test() const;
    GRAPHSTATSTESTER_TEST_TABLE
    #undef X
    GraphStatsTester& set_all();
    GraphStatsTester& clear_all();
};

}
