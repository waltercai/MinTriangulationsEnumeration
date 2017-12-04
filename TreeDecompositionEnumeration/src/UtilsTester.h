#ifndef UTILSTESTER_H_INCLUDED
#define UTILSTESTER_H_INCLUDED

#include "TestInterface.h"
#include "Utils.h"

namespace tdenum {

#define UTILSTESTER_TEST_TABLE \
    X(epsilon_float_cmp) \
    X(io) \
    X(recursive_depth_io) \
    X(strip) \
    X(str_tokenize) \
    X(join) \
    X(get_all_lines) \
    X(str_to_file) \
    X(csv_io) \
    X(logger_io) \
    X(str_length) \
    X(str_eq) \
    X(str_empty) \
    X(to_str) \
    X(substr_functions) \
    X(time_functions) \
    X(in_cont_in_set) \
    X(same_vector_elements) \
    X(remove_vec) \
    X(push_back_unique) \
    X(vec_range) \
    X(set_union_intersection) \
    X(path_string_decomposition)

class UtilsTester : public TestInterface {
private:
    #define X(test) bool flag_##test;
    UTILSTESTER_TEST_TABLE
    #undef X
    UtilsTester& go() {
        //set_only_join();
        #define X(test) \
        if (flag_##test) {DO_TEST(test);}
        UTILSTESTER_TEST_TABLE
        #undef X
        return *this;
    }
public:
    UtilsTester() :
        TestInterface("Utils Tester")
        #define X(test) , flag_##test(true)
        UTILSTESTER_TEST_TABLE
        #undef X
        {};
    #define X(test) \
    UtilsTester& set_##test() { flag_##test = true; return *this; } \
    UtilsTester& unset_##test() { flag_##test = false; return *this; } \
    UtilsTester& set_only_##test() { clear_all(); return set_##test(); } \
    bool test() const;
    UTILSTESTER_TEST_TABLE
    #undef X
    UtilsTester& set_all() {
        #define X(test) set_##test();
        UTILSTESTER_TEST_TABLE
        #undef X
        return *this;
    }
    UtilsTester& clear_all() {
        #define X(test) unset_##test();
        UTILSTESTER_TEST_TABLE
        #undef X
        return *this;
    }

};

}

#endif // UTILSTESTER_H_INCLUDED
