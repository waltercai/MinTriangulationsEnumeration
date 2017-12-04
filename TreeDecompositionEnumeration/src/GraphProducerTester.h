#ifndef GRAPHPRODUCERTESTER_H_INCLUDED
#define GRAPHPRODUCERTESTER_H_INCLUDED

#include "TestInterface.h"
#include "GraphProducer.h"

namespace tdenum {

// ALL THESE TESTS SHOULD TEST:
// * get()
// * dump() (make sure existing files aren't dumped)
#define GRAPHPRODUCERTESTER_TEST_TABLE \
    X(construction_no_dir) \
    X(construction_dir_given) \
    X(reset_no_dir_from_no_dir) \
    X(reset_no_dir_from_dir_given) \
    X(reset_dir_given_from_no_dir) \
    X(reset_dir_given_from_dir_given) \
    X(add_gs) \
    X(add_gs_vec) \
    X(add_by_existing_filepath) \
    X(add_by_non_existing_filepath) \
    X(add_nonexistent_dir) \
    X(add_empty_dir) \
    X(add_dir_only_grp)        /* Make sure GraphStats objects have valid text and integrity()*/ \
    X(add_dir_multiple_types)  /* Make sure GraphStats objects have valid text and integrity()*/ \
    X(add_random_basic)        /* Make sure no integrity*/\
    X(add_random_no_mixmatch) \
    X(add_random_mixmatch) \
    X(add_random_pstep) \
    X(add_random_pstep_range) \
    X(add_all_types_of_GSs_test_dump)


class GraphProducerTester : public TestInterface {
private:

    // Aux methods
    void add_some_GSs(GraphProducer&) const;
    void add_lots_of_GSs(GraphProducer&) const;

    #define X(test) bool flag_##test;
    GRAPHPRODUCERTESTER_TEST_TABLE
    #undef X
    GraphProducerTester& go();
public:
    GraphProducerTester();
    #define X(test) \
    GraphProducerTester& set_##test(); \
    GraphProducerTester& unset_##test(); \
    GraphProducerTester& set_only_##test(); \
    bool test() const;
    GRAPHPRODUCERTESTER_TEST_TABLE
    #undef X
    GraphProducerTester& set_all();
    GraphProducerTester& clear_all();
};

}

#endif // GRAPHPRODUCERTESTER_H_INCLUDED
