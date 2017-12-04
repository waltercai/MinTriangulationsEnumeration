#ifndef DATASETTESTER_H_INCLUDED
#define DATASETTESTER_H_INCLUDED

#include "TestInterface.h"

namespace tdenum {


#define DATASETTESTER_TEST_TABLE \
    X(find_by_text) \
    X(validate_unique_text) \
    X(load_empty_dir) \
    X(calc_trng_correct_order) \
    X(calc_ms_varying_statreqs) \
    X(calc_pmc_varying_statreqs) \
    X(calc_trng_varying_statreqs) \
    X(stupid_GS_dump_no_preexisting_flag_true) \
    X(stupid_GS_dump_no_preexisting_flag_false) \
    X(stupid_GS_dump_with_preexisting_flag_true) \
    X(stupid_GS_dump_with_preexisting_flag_false) \
    X(construct_via_load_no_statfile_exists) \
    X(construct_via_load_statfile_exists) \
    X(construct_via_load_missing_sources) \
    X(construct_via_GS_paths_all_valid) \
    X(construct_via_GS_paths_some_invalid) \
    X(basic_calc_same_statreq) \
    X(permute_stat_request_fields_check_output_columns) \
    X(column_indexes_correct) \
    X(several_pmc_algorithms_check_times_and_errors_per_alg) \
    X(generate_count_and_time_errors_test_text_and_gs_fields) \
    X(load_stat_files_same_statreq) \
    X(load_stat_files_diverse_statreq) \
    X(load_stat_files_diverse_statreq_diverse_valid_cells) \
    X(validate_stat_load_before_calc) \
    X(validate_stat_load_updates_statreq_same_statreq) \
    X(validate_stat_load_updates_statreq_diverse_statreq) \
    X(validate_unavailable_stat_update_after_stat_load) \
    X(column_sanity) \
    X(diverse_column_output)

class DatasetTester : public TestInterface {

    // Flags, setter / getters and actual tests.
    #define X(test) \
    private: \
        bool flag_##test; \
        bool test() const; \
    public: \
        DatasetTester& set_##test(); \
        DatasetTester& unset_##test(); \
        DatasetTester& set_only_##test();
    DATASETTESTER_TEST_TABLE
    #undef X

private:
    DatasetTester& go();

public:

    DatasetTester();

    DatasetTester& set_all();
    DatasetTester& clear_all();
};

}

#endif // DATASETTESTER_H_INCLUDED
