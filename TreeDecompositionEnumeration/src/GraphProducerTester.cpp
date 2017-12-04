#include "Graph.h"
#include "GraphProducer.h"
#include "GraphProducerTester.h"
#include "GraphStatsTester.h" // Need this for ASSERT_GS_EQ/NEQ

namespace tdenum {

GraphProducerTester& GraphProducerTester::go() {
    //set_only_add_all_types_of_GSs_test_dump();
    #define X(test) if (flag_##test) {DO_TEST(test);}
    GRAPHPRODUCERTESTER_TEST_TABLE
    #undef X
    return *this;
}

GraphProducerTester::GraphProducerTester() :
    TestInterface("GraphProducer Tester")
    #define X(test) , flag_##test(true)
    GRAPHPRODUCERTESTER_TEST_TABLE
    #undef X
    {}

#define X(test) \
GraphProducerTester& GraphProducerTester::set_##test() { flag_##test = true; return *this; } \
GraphProducerTester& GraphProducerTester::unset_##test() { flag_##test = false; return *this; } \
GraphProducerTester& GraphProducerTester::set_only_##test() { clear_all(); return set_##test(); }
GRAPHPRODUCERTESTER_TEST_TABLE
#undef X

GraphProducerTester& GraphProducerTester::set_all() {
    #define X(test) set_##test();
    GRAPHPRODUCERTESTER_TEST_TABLE
    #undef X
    return *this;
}
GraphProducerTester& GraphProducerTester::clear_all() {
    #define X(test) unset_##test();
    GRAPHPRODUCERTESTER_TEST_TABLE
    #undef X
    return *this;
}

/**
 * Compound ASSERT wrappers
 */
#define ASSERT_GS_VEC_EQ_AUX(_vgs1,_vgs2,_except_text) do { \
        vector<GraphStats> tmp1 = _vgs1; \
        vector<GraphStats> tmp2 = _vgs2; \
        ASSERT_EQ(tmp1.size(),tmp2.size()); \
        for (unsigned i=0; i<tmp1.size(); ++i) { \
            if (_except_text) { \
                tmp1[i].set_text(tmp2[i].get_text()); \
            } \
            ASSERT_GS_EQ(tmp1[i],tmp2[i]); \
        } \
    } while(0)

#define ASSERT_GS_VEC_EQ_EXCEPT_TEXT(_vgs1, _vgs2) ASSERT_GS_VEC_EQ_AUX(_vgs1, _vgs2, true)
#define ASSERT_GS_VEC_EQ(_vgs1, _vgs2) ASSERT_GS_VEC_EQ_AUX(_vgs1, _vgs2, false)

/**
 * Some setup macros
 */
#define INIT_BASICS(_gp_varname) \
    Graph g1; \
    Graph g2 = Graph(5); \
    Graph g3 = Graph(6).addEdge(2,0).addEdge(1,0); \
    Graph g4 = Graph(5).randomize(0.5,2); \
    GraphStats gs1(g1); \
    GraphStats gs2(g2); \
    GraphStats gs3(g3); \
    GraphStats gs4(g4); \
    string filename1 = tmp_dir_name+"/"+GraphStats::get_default_filename_from_graph(g1); \
    string filename2 = tmp_dir_name+"/"+GraphStats::get_default_filename_from_graph(g2); \
    string filename3 = tmp_dir_name+"/"+GraphStats::get_default_filename_from_graph(g3); \
    string filename4 = tmp_dir_name+"/"+GraphStats::get_default_filename_from_graph(g4); \
    string file1 = string("0,")+GRAPHSTATS_METADATA_TXT_NOT_RANDOM+",1"; \
    string file2 = string("5,")+GRAPHSTATS_METADATA_TXT_NOT_RANDOM+",1"; \
    string file3 = string("6,")+GRAPHSTATS_METADATA_TXT_NOT_RANDOM+",1" \
                   +"\n0,1" \
                   +"\n0,2"; \
    string file4 = string("5,0.5,2") \
        +(g4.areNeighbors(0,1) ? "\n0,1" : "") \
        +(g4.areNeighbors(0,2) ? "\n0,2" : "") \
        +(g4.areNeighbors(0,3) ? "\n0,3" : "") \
        +(g4.areNeighbors(0,4) ? "\n0,4" : "") \
        +(g4.areNeighbors(1,2) ? "\n1,2" : "") \
        +(g4.areNeighbors(1,3) ? "\n1,3" : "") \
        +(g4.areNeighbors(1,4) ? "\n1,4" : "") \
        +(g4.areNeighbors(2,3) ? "\n2,3" : "") \
        +(g4.areNeighbors(2,4) ? "\n2,4" : "") \
        +(g4.areNeighbors(3,4) ? "\n3,4" : ""); \
    vector<GraphStats> vgs = {gs1,gs2,gs3,gs4}; \

#define DUMP_BASICS() \
    ASSERT(utils__dump_string_to_file(file1,filename1)); \
    ASSERT(utils__dump_string_to_file(file2,filename2)); \
    ASSERT(utils__dump_string_to_file(file3,filename3)); \
    ASSERT(utils__dump_string_to_file(file4,filename4))

#define CLEANUP_BASICS() \
    ASSERT(utils__delete_file(filename1)); \
    ASSERT(utils__delete_file(filename2)); \
    ASSERT(utils__delete_file(filename3)); \
    ASSERT(utils__delete_file(filename4))

/**
 * Tests
 */
bool GraphProducerTester::construction_no_dir() const {
    ASSERT_NO_THROW(GraphProducer gp);
    GraphProducer gp;
    ASSERT_NO_THROW(gp.dump_graphs(true));
    ASSERT_NO_THROW(gp.get());
    ASSERT_EQ(gp.get(), vector<GraphStats>());
    ASSERT_NO_THROW(gp.dump_graphs(false));
    ASSERT_NO_THROW(gp.get());
    ASSERT_EQ(gp.get(), vector<GraphStats>());
    return true;
}
bool GraphProducerTester::construction_dir_given() const {
    ASSERT_NO_THROW(GraphProducer gp(tmp_dir_name));
    GraphProducer gp(tmp_dir_name);
    ASSERT_NO_THROW(gp.dump_graphs(true));
    ASSERT_NO_THROW(gp.get());
    ASSERT_EQ(gp.get(), vector<GraphStats>());
    ASSERT_NO_THROW(gp.dump_graphs(false));
    ASSERT_NO_THROW(gp.get());
    ASSERT_EQ(gp.get(), vector<GraphStats>());
    return true;
}
bool GraphProducerTester::reset_no_dir_from_no_dir() const {
    GraphProducer gp;
    ASSERT_NO_THROW(gp.reset());
    ASSERT_NO_THROW(gp.dump_graphs(true));
    ASSERT_NO_THROW(gp.get());
    ASSERT_EQ(gp.get(), vector<GraphStats>());
    ASSERT_NO_THROW(gp.dump_graphs(false));
    ASSERT_NO_THROW(gp.get());
    ASSERT_EQ(gp.get(), vector<GraphStats>());
    return true;
}
bool GraphProducerTester::reset_no_dir_from_dir_given() const {
    GraphProducer gp(tmp_dir_name);
    ASSERT_NO_THROW(gp.reset());
    ASSERT_NO_THROW(gp.dump_graphs(true));
    ASSERT_NO_THROW(gp.get());
    ASSERT_EQ(gp.get(), vector<GraphStats>());
    ASSERT_NO_THROW(gp.dump_graphs(false));
    ASSERT_NO_THROW(gp.get());
    ASSERT_EQ(gp.get(), vector<GraphStats>());
    return true;
}
bool GraphProducerTester::reset_dir_given_from_no_dir() const {
    GraphProducer gp;
    ASSERT_NO_THROW(gp.reset(tmp_dir_name));
    ASSERT_NO_THROW(gp.dump_graphs(true));
    ASSERT_NO_THROW(gp.get());
    ASSERT_EQ(gp.get(), vector<GraphStats>());
    ASSERT_NO_THROW(gp.dump_graphs(false));
    ASSERT_NO_THROW(gp.get());
    ASSERT_EQ(gp.get(), vector<GraphStats>());
    return true;
}
bool GraphProducerTester::reset_dir_given_from_dir_given() const {
    GraphProducer gp(tmp_dir_name);
    ASSERT_NO_THROW(gp.reset(tmp_dir_name));
    ASSERT_NO_THROW(gp.dump_graphs(true));
    ASSERT_NO_THROW(gp.get());
    ASSERT_EQ(gp.get(), vector<GraphStats>());
    ASSERT_NO_THROW(gp.dump_graphs(false));
    ASSERT_NO_THROW(gp.get());
    ASSERT_EQ(gp.get(), vector<GraphStats>());
    return true;
}
bool GraphProducerTester::add_gs() const {
    INIT_BASICS();
    GraphProducer gp(tmp_dir_name);

    ASSERT_NO_THROW(gp.add(gs2));
    ASSERT_NO_THROW(gp.dump_graphs());
    ASSERT(utils__file_exists(filename2));
    ASSERT(gp.get()[0].equal_except_text(gs2));
    ASSERT_NEQ(gp.get(), vector<GraphStats>({gs2}));
    gs2.set_text(filename2);
    ASSERT_EQ(gp.get(), vector<GraphStats>({gs2}));
    ASSERT(gs2.text_valid());
    ASSERT(gs2.integrity());    // The graph text should be valid! This should work
    // gs should be from file now.
    // Switch dirs, and make sure no new graph is dumped
    // in dump(true) and a graph is dumped for dump(false)
    string inner_dir = tmp_dir_name+"/tmp_subdir";
    string new_filename2 = inner_dir + "/" + utils__get_filename(filename2);
    ASSERT(utils__mkdir(inner_dir));
    ASSERT_NO_THROW(gp.reset(inner_dir));
    ASSERT_NO_THROW(gp.add(gs2));
    ASSERT_EQ(gp.get(), vector<GraphStats>({gs2}));
    ASSERT(gp.get()[0].integrity());                // gs2 should still point to it's file
    ASSERT_EQ(gp.get()[0].get_text(),filename2);    // gs2 should still point to it's file
    ASSERT_NO_THROW(gp.dump_graphs(true));
    ASSERT(utils__dir_is_empty(inner_dir));         // Shouldn't have been dumped, gs.get_text() contains a valid path
    ASSERT_NO_THROW(gp.dump_graphs(false));
    ASSERT(gs2.equal_except_text(gp.get()[0]));
    ASSERT(!utils__dir_is_empty(inner_dir));
    ASSERT(gs2.integrity(inner_dir+"/"));           // Should find the default filename
    ASSERT(gs2.integrity(tmp_dir_name+"/"));        // Should find the default filename
    ASSERT(gs2.integrity(new_filename2));

    // Cleanup
    CLEANUP_BASICS();
    ASSERT(utils__delete_file(new_filename2));
    ASSERT(utils__delete_file(filename2));
    ASSERT(utils__delete_dir(inner_dir));
    return true;
}
bool GraphProducerTester::add_gs_vec() const {
    INIT_BASICS();
    GraphProducer gp(tmp_dir_name);

    ASSERT_NO_THROW(gp.add(vgs));
    ASSERT_EQ(gp.get(), vgs);

    ASSERT(!utils__file_exists(filename1));
    ASSERT(!utils__file_exists(filename2));
    ASSERT(!utils__file_exists(filename3));
    ASSERT(!utils__file_exists(filename4));

    ASSERT_NO_THROW(gp.dump_graphs());

    ASSERT(utils__file_exists(filename1));
    ASSERT(utils__file_exists(filename2));
    ASSERT(utils__file_exists(filename3));
    ASSERT(utils__file_exists(filename4));

    ASSERT(gs1.integrity(filename1));
    ASSERT(gs2.integrity(filename2));
    ASSERT(gs3.integrity(filename3));
    ASSERT(gs4.integrity(filename4));

    CLEANUP_BASICS();
    return true;
}
bool GraphProducerTester::add_by_existing_filepath() const {
    INIT_BASICS();
    GraphProducer gp(tmp_dir_name);

    ASSERT(utils__dump_string_to_file(file1,filename1));
    ASSERT(utils__dump_string_to_file(file2,filename2));
    ASSERT(utils__dump_string_to_file(file3,filename3));
    ASSERT(utils__dump_string_to_file(file4,filename4));
    ASSERT_NO_THROW(gp.add(filename1));
    ASSERT_NO_THROW(gp.add(filename2));
    ASSERT_NO_THROW(gp.add(filename3));
    ASSERT_NO_THROW(gp.add(filename4));
    ASSERT_GS_VEC_EQ_EXCEPT_TEXT(gp.get(), vgs);
    ASSERT(gp.get()[0].integrity());    // Should be automatic after being read
    ASSERT(gp.get()[1].integrity());
    ASSERT(gp.get()[2].integrity());
    ASSERT(gp.get()[3].integrity());

    CLEANUP_BASICS();
    return true;
}
bool GraphProducerTester::add_by_non_existing_filepath() const {
    INIT_BASICS();
    GraphProducer gp(tmp_dir_name);

    TRACE_LVL_OVERRIDE(TRACE_LVL__ALWAYS);
    ASSERT_NO_THROW(gp.add(filename1));
    ASSERT_NO_THROW(gp.add(filename2));
    ASSERT_NO_THROW(gp.add(filename3));
    ASSERT_NO_THROW(gp.add(filename4));
    TRACE_LVL_RESTORE();

    ASSERT_EQ(gp.get().size(), 0);  // Should contain nothing!

    CLEANUP_BASICS();
    return true;
}
bool GraphProducerTester::add_nonexistent_dir() const {

    GraphProducer gp;
    string fake_dir = tmp_dir_name+"/fake_dir";
    TRACE_LVL_OVERRIDE(TRACE_LVL__ALWAYS);
    ASSERT(!utils__dir_exists(fake_dir));
    ASSERT_NO_THROW(gp.add(DirectoryIterator(fake_dir)));
    TRACE_LVL_RESTORE();

    ASSERT_EQ(gp.get().size(), 0);  // Should contain nothing!

    return true;
}
bool GraphProducerTester::add_empty_dir() const {

    GraphProducer gp;
    string empty_dir = tmp_dir_name+"/empty_dir";
    ASSERT(utils__mkdir(empty_dir));
    ASSERT(utils__dir_exists(empty_dir));
    ASSERT(utils__dir_is_empty(empty_dir));
    ASSERT_NO_THROW(gp.add(DirectoryIterator(empty_dir)));

    ASSERT_EQ(gp.get().size(), 0);  // Should contain nothing!

    // Cleanup
    ASSERT(utils__delete_dir(empty_dir));
    return true;
}
bool GraphProducerTester::add_dir_only_grp() const {
    INIT_BASICS();
    DUMP_BASICS();
    GraphProducer gp(tmp_dir_name);

    ASSERT_NO_THROW(gp.add(DirectoryIterator(tmp_dir_name)));
    ASSERT_EQ(gp.get().size(), 4);
    ASSERT_NEQ(gp.get(), vgs);  // The text fields should be different
    ASSERT(gp.get()[0].integrity());
    ASSERT(gp.get()[1].integrity());
    ASSERT(gp.get()[2].integrity());
    ASSERT(gp.get()[3].integrity());

    // Now, change the text values by force and test equality of the vector
    ASSERT(!vgs[0].integrity());
    ASSERT(!vgs[1].integrity());
    ASSERT(!vgs[2].integrity());
    ASSERT(!vgs[3].integrity());
    ASSERT(vgs[0].integrity(filename1));
    ASSERT(vgs[1].integrity(filename2));
    ASSERT(vgs[2].integrity(filename3));
    ASSERT(vgs[3].integrity(filename4));
    ASSERT(vgs[0].integrity());
    ASSERT(vgs[1].integrity());
    ASSERT(vgs[2].integrity());
    ASSERT(vgs[3].integrity());
    ASSERT_SAME_VEC_ELEMS(gp.get(), vgs);  // The text fields should be equal now

    CLEANUP_BASICS();
    return true;
}
bool GraphProducerTester::add_dir_multiple_types() const {
    INIT_BASICS();
    DUMP_BASICS();
    GraphProducer gp(tmp_dir_name);

    // Add a dummy file (no known extension) and a non-grp file.
    Graph g5 = Graph(4).addClique(vector<Node>({0,1,2})).addClique(vector<Node>({0,3}));
    string filename5 = tmp_dir_name + "/non_grp_file.csv";
    string file5 = "0,1,2\n0,3"; // I think..
    GraphStats gs5(g5);
    vgs.push_back(gs5);

    string filename6 = tmp_dir_name + "/dummy_file.garblewarble";

    utils__dump_string_to_file(file5, filename5);
    utils__dump_string_to_file("", filename6);

    ASSERT(utils__file_exists(filename5));
    ASSERT(utils__file_exists(filename6));

    TRACE_LVL_OVERRIDE(TRACE_LVL__ALWAYS);
    ASSERT_NO_THROW(gp.add(DirectoryIterator(tmp_dir_name)));
    TRACE_LVL_RESTORE();
    ASSERT_EQ(gp.get().size(), 5);
    ASSERT_NEQ(gp.get(), vgs);      // Missing text fields
    ASSERT(gp.get()[0].integrity());
    ASSERT(gp.get()[1].integrity());
    ASSERT(gp.get()[2].integrity());
    ASSERT(gp.get()[3].integrity());
    ASSERT(gp.get()[4].integrity());    // CSV type graph should also be OK

    ASSERT_EQ(gp.get()[4].get_graph(), g5);

    // Fix the vector, assert equality
    ASSERT(vgs[0].integrity(filename1));
    ASSERT(vgs[1].integrity(filename2));
    ASSERT(vgs[2].integrity(filename3));
    ASSERT(vgs[3].integrity(filename4));
    ASSERT(vgs[4].integrity(filename5));
    ASSERT_SAME_VEC_ELEMS(gp.get(), vgs);

    // Cleanup
    ASSERT(utils__delete_file(filename6));
    ASSERT(utils__delete_file(filename5));
    CLEANUP_BASICS();
    return true;
}
bool GraphProducerTester::add_random_basic() const {

    // Create
    GraphProducer gp(tmp_dir_name);
    double p = 0.5;
    int n = 5;
    unsigned instances = 3;
    vector<GraphStats> vgs;
    ASSERT_NO_THROW(vgs = gp.add_random(n,p,instances).get());
    unsigned expected = instances;
    ASSERT_EQ(vgs.size(), expected);

    // Helper vectors
    vector<Graph> g(expected);
    vector<string> filename(expected);

    // Everything in a loop
    for (unsigned i=0; i<expected; ++i) {

        // Make sure the coast is clear
        ASSERT(!utils__file_exists(filename[i]));

        // Populate vector
        g[i] = vgs[i].get_graph();
        filename[i] = tmp_dir_name+"/"+GraphStats::get_default_filename_from_graph(g[i]);

        // Validate parameters
        ASSERT(vgs[i].is_random());
        ASSERT(!vgs[i].integrity());
        ASSERT_EQ(vgs[i].get_n(), (unsigned)n);
        ASSERT_EQ_FLOAT(vgs[i].get_p(), p);
        ASSERT_EQ(vgs[i].get_instance(), (int)i+1);

        // No guarantees for filenames, just make sure they're different:
        for (unsigned j=0; j<i; ++j) {
            ASSERT_NEQ(filename[i], filename[j]);
        }
    }

    // Dump
    ASSERT_NO_THROW(gp.dump_graphs());
    vector<GraphStats> vgs_out = gp.get();

    // Continue
    for (unsigned i=0; i<expected; ++i) {

        ASSERT(utils__file_exists(filename[i]));

        // When they are dumped - even without calling integrity() - the GS objects
        // should have updated file paths
        ASSERT(vgs_out[i].text_valid());
        ASSERT_EQ(vgs_out[i].get_text(), filename[i]);

        // Now, test integrity
        ASSERT(vgs_out[i].integrity());
        ASSERT(vgs_out[i].integrity(filename[i]));
        ASSERT(!vgs[i].text_valid());  // Shouldn't be valid, old vector

        // Make sure the files can be read and are consistent
        GraphStats gs_in = GraphStats::read(filename[i]);
        ASSERT_EQ(gs_in.get_graph(), g[i]);
        // Make them equal to the previous GS objects by force, and check equality
        ASSERT(!vgs[i].text_valid());  // Shouldn't be valid yet
        ASSERT(gs_in.text_valid());
        ASSERT_GS_NEQ(gs_in,vgs[i]);
        vgs[i].set_text(filename[i]);
        ASSERT_GS_EQ(gs_in,vgs[i]);

    }

    return true;
}
bool GraphProducerTester::add_random_no_mixmatch() const {

    // Create
    GraphProducer gp(tmp_dir_name);
    vector<double> p = {0.3,0.4,0.5};
    vector<int> n = {4,5};
    unsigned instances = 3;
    vector<GraphStats> vgs;
    TRACE_LVL_OVERRIDE(TRACE_LVL__ALWAYS);
    ASSERT_NO_THROW(vgs = gp.add_random(n,p,false,instances).get());    // Bad input
    TRACE_LVL_RESTORE();
    ASSERT_EQ(vgs.size(), 0);                                           // .. so nothing is done
    n.push_back(6);
    ASSERT_EQ(n.size(),p.size());   // Sanity
    unsigned expected = instances * n.size();
    ASSERT_NO_THROW(vgs = gp.add_random(n,p,false,instances).get());    // Good input
    ASSERT_EQ(vgs.size(), expected);

    // Helper vectors
    vector<Graph> g(expected);
    vector<string> filename(expected);

    // Everything in a loop.
    // Start by stat validation, for ease of looping: this isn't implementation-defined,
    // the header file specifically defines the order these graphs are generated
    unsigned current_index = 0;
    for (unsigned i=0; i<n.size(); ++i) {
        for (unsigned k=1; k<=instances; ++k) {
            ASSERT_EQ(vgs[current_index].get_n(), (unsigned)n[i]);
            ASSERT_EQ_FLOAT(vgs[current_index].get_p(), p[i]);
            ASSERT_EQ(vgs[current_index].get_instance(), (int)k);
            ++current_index;
        }
    }

    // Next:
    for (unsigned i=0; i<expected; ++i) {

        // Make sure the coast is clear
        ASSERT(!utils__file_exists(filename[i]));

        // Populate vector
        g[i] = vgs[i].get_graph();
        filename[i] = tmp_dir_name+"/"+GraphStats::get_default_filename_from_graph(g[i]);

        // Validate parameters
        ASSERT(vgs[i].is_random());
        ASSERT(!vgs[i].integrity());

        // No guarantees for filenames, just make sure they're different:
        for (unsigned j=0; j<i; ++j) {
            ASSERT_NEQ(filename[i], filename[j]);
        }
    }

    // Dump
    ASSERT_NO_THROW(gp.dump_graphs());
    vector<GraphStats> vgs_out = gp.get();

    // Continue
    for (unsigned i=0; i<expected; ++i) {

        ASSERT(utils__file_exists(filename[i]));

        // When they are dumped - even without calling integrity() - the GS objects
        // should have updated file paths
        ASSERT(vgs_out[i].text_valid());
        ASSERT_EQ(vgs_out[i].get_text(), filename[i]);

        // Now, test integrity
        ASSERT(vgs_out[i].integrity());
        ASSERT(vgs_out[i].integrity(filename[i]));
        ASSERT(!vgs[i].text_valid());  // Shouldn't be valid, old vector

        // Make sure the files can be read and are consistent
        GraphStats gs_in = GraphStats::read(filename[i]);
        ASSERT_EQ(gs_in.get_graph(), g[i]);
        // Make them equal to the previous GS objects by force, and check equality
        ASSERT(!vgs[i].text_valid());  // Shouldn't be valid yet
        ASSERT(gs_in.text_valid());
        ASSERT_GS_NEQ(gs_in,vgs[i]);
        vgs[i].set_text(filename[i]);
        ASSERT_GS_EQ(gs_in,vgs[i]);

    }

    return true;
}
bool GraphProducerTester::add_random_mixmatch() const {

    // Create
    GraphProducer gp(tmp_dir_name);
    vector<double> p = {0.3,0.4,0.5};
    vector<int> n = {4,5};
    unsigned instances = 3;
    unsigned expected = instances * n.size() * p.size();    // Mix&Match (n,p)
    vector<GraphStats> vgs;
    ASSERT_NO_THROW(vgs = gp.add_random(n,p,true,instances).get());    // Bad input
    ASSERT_EQ(vgs.size(), expected);

    // Helper vectors
    vector<Graph> g(expected);
    vector<string> filename(expected);

    // Everything in a loop.
    // Start by stat validation, for ease of looping: this isn't implementation-defined,
    // the header file specifically defines the order these graphs are generated
    unsigned current_index = 0;
    for (unsigned i=0; i<n.size(); ++i) {
        for (unsigned j=0; j<p.size(); ++j) {
            for (unsigned k=1; k<=instances; ++k) {
                ASSERT_EQ(vgs[current_index].get_n(), (unsigned)n[i]);
                ASSERT_EQ_FLOAT(vgs[current_index].get_p(), p[j]);
                ASSERT_EQ(vgs[current_index].get_instance(), (int)k);
                ++current_index;
            }
        }
    }

    // Next:
    for (unsigned i=0; i<expected; ++i) {

        // Make sure the coast is clear
        ASSERT(!utils__file_exists(filename[i]));

        // Populate vector
        g[i] = vgs[i].get_graph();
        filename[i] = tmp_dir_name+"/"+GraphStats::get_default_filename_from_graph(g[i]);

        // Validate parameters
        ASSERT(vgs[i].is_random());
        ASSERT(!vgs[i].integrity());

        // No guarantees for filenames, just make sure they're different:
        for (unsigned j=0; j<i; ++j) {
            ASSERT_NEQ(filename[i], filename[j]);
        }
    }

    // Dump
    ASSERT_NO_THROW(gp.dump_graphs());
    vector<GraphStats> vgs_out = gp.get();

    // Continue
    for (unsigned i=0; i<expected; ++i) {

        ASSERT(utils__file_exists(filename[i]));

        // When they are dumped - even without calling integrity() - the GS objects
        // should have updated file paths
        ASSERT(vgs_out[i].text_valid());
        ASSERT_EQ(vgs_out[i].get_text(), filename[i]);

        // Now, test integrity
        ASSERT(vgs_out[i].integrity());
        ASSERT(vgs_out[i].integrity(filename[i]));
        ASSERT(!vgs[i].text_valid());  // Shouldn't be valid, old vector

        // Make sure the files can be read and are consistent
        GraphStats gs_in = GraphStats::read(filename[i]);
        ASSERT_EQ(gs_in.get_graph(), g[i]);
        // Make them equal to the previous GS objects by force, and check equality
        ASSERT(!vgs[i].text_valid());  // Shouldn't be valid yet
        ASSERT(gs_in.text_valid());
        ASSERT_GS_NEQ(gs_in,vgs[i]);
        vgs[i].set_text(filename[i]);
        ASSERT_GS_EQ(gs_in,vgs[i]);

    }

    return true;
}
bool GraphProducerTester::add_random_pstep() const {

    // Create
    GraphProducer gp(tmp_dir_name);
    double step = 0.15;
    ASSERT_EQ((int)(1/step), 6);    // Manually check value, for now: 0.15,0.3,0.45,0.6,0.75,0.9
    vector<int> n = {4,5,6};
    unsigned instances = 3;
    unsigned expected = instances * n.size() * ((int)(1/step));
    vector<GraphStats> vgs;
    ASSERT_NO_THROW(vgs = gp.add_random_pstep(n,step,instances).get());    // Bad input
    ASSERT_EQ(vgs.size(), expected);

    // Helper vectors
    vector<Graph> g(expected);
    vector<string> filename(expected);

    // Everything in a loop.
    // Start by stat validation, for ease of looping: this isn't implementation-defined,
    // the header file specifically defines the order these graphs are generated
    unsigned current_index = 0;
    for (unsigned i=0; i<n.size(); ++i) {
        for (double p=step; p<1; p += step) {
            for (unsigned k=1; k<=instances; ++k) {
                ASSERT_EQ(vgs[current_index].get_n(), (unsigned)n[i]);
                ASSERT_EQ_FLOAT(vgs[current_index].get_p(), p);
                ASSERT_EQ(vgs[current_index].get_instance(), (int)k);
                ++current_index;
            }
        }
    }

    // Next:
    for (unsigned i=0; i<expected; ++i) {

        // Make sure the coast is clear
        ASSERT(!utils__file_exists(filename[i]));

        // Populate vector
        g[i] = vgs[i].get_graph();
        filename[i] = tmp_dir_name+"/"+GraphStats::get_default_filename_from_graph(g[i]);

        // Validate parameters
        ASSERT(vgs[i].is_random());
        ASSERT(!vgs[i].integrity());

        // No guarantees for filenames, just make sure they're different:
        for (unsigned j=0; j<i; ++j) {
            ASSERT_NEQ(filename[i], filename[j]);
        }
    }

    // Dump
    ASSERT_NO_THROW(gp.dump_graphs());
    vector<GraphStats> vgs_out = gp.get();

    // Continue
    for (unsigned i=0; i<expected; ++i) {

        ASSERT(utils__file_exists(filename[i]));

        // When they are dumped - even without calling integrity() - the GS objects
        // should have updated file paths
        ASSERT(vgs_out[i].text_valid());
        ASSERT_EQ(vgs_out[i].get_text(), filename[i]);

        // Now, test integrity
        ASSERT(vgs_out[i].integrity());
        ASSERT(vgs_out[i].integrity(filename[i]));
        ASSERT(!vgs[i].text_valid());  // Shouldn't be valid, old vector

        // Make sure the files can be read and are consistent
        GraphStats gs_in = GraphStats::read(filename[i]);
        ASSERT_EQ(gs_in.get_graph(), g[i]);
        // Make them equal to the previous GS objects by force, and check equality
        ASSERT(!vgs[i].text_valid());  // Shouldn't be valid yet
        ASSERT(gs_in.text_valid());
        ASSERT_GS_NEQ(gs_in,vgs[i]);
        vgs[i].set_text(filename[i]);
        ASSERT_GS_EQ(gs_in,vgs[i]);

    }

    return true;
}
bool GraphProducerTester::add_random_pstep_range() const {

    // Create
    GraphProducer gp(tmp_dir_name);
    double first = 0.05, last = 0.29, step = 0.05;
    vector<double> expected_p = {0.05,0.1,0.15,0.2,0.25};
    vector<int> n = {4,5,6};
    unsigned instances = 3;
    unsigned expected = instances * n.size() * expected_p.size();
    vector<GraphStats> vgs;
    ASSERT_NO_THROW(vgs = gp.add_random_pstep_range(n,first,last,step,instances).get());    // Bad input
    ASSERT_EQ(vgs.size(), expected);

    // Helper vectors
    vector<Graph> g(expected);
    vector<string> filename(expected);

    // Everything in a loop.
    // Start by stat validation, for ease of looping: this isn't implementation-defined,
    // the header file specifically defines the order these graphs are generated
    unsigned current_index = 0;
    for (unsigned i=0; i<n.size(); ++i) {
        for (unsigned j=0; j<expected_p.size(); ++j) {
            for (unsigned k=1; k<=instances; ++k) {
                ASSERT_EQ(vgs[current_index].get_n(), (unsigned)n[i]);
                ASSERT_EQ_FLOAT(vgs[current_index].get_p(), expected_p[j]);
                ASSERT_EQ(vgs[current_index].get_instance(), (int)k);
                ++current_index;
            }
        }
    }

    // Next:
    for (unsigned i=0; i<expected; ++i) {

        // Make sure the coast is clear
        ASSERT(!utils__file_exists(filename[i]));

        // Populate vector
        g[i] = vgs[i].get_graph();
        filename[i] = tmp_dir_name+"/"+GraphStats::get_default_filename_from_graph(g[i]);

        // Validate parameters
        ASSERT(vgs[i].is_random());
        ASSERT(!vgs[i].integrity());

        // No guarantees for filenames, just make sure they're different:
        for (unsigned j=0; j<i; ++j) {
            ASSERT_NEQ(filename[i], filename[j]);
        }
    }

    // Dump
    ASSERT_NO_THROW(gp.dump_graphs());
    vector<GraphStats> vgs_out = gp.get();

    // Continue
    for (unsigned i=0; i<expected; ++i) {

        ASSERT(utils__file_exists(filename[i]));

        // When they are dumped - even without calling integrity() - the GS objects
        // should have updated file paths
        ASSERT(vgs_out[i].text_valid());
        ASSERT_EQ(vgs_out[i].get_text(), filename[i]);

        // Now, test integrity
        ASSERT(vgs_out[i].integrity());
        ASSERT(vgs_out[i].integrity(filename[i]));
        ASSERT(!vgs[i].text_valid());  // Shouldn't be valid, old vector

        // Make sure the files can be read and are consistent
        GraphStats gs_in = GraphStats::read(filename[i]);
        ASSERT_EQ(gs_in.get_graph(), g[i]);
        // Make them equal to the previous GS objects by force, and check equality
        ASSERT(!vgs[i].text_valid());  // Shouldn't be valid yet
        ASSERT(gs_in.text_valid());
        ASSERT_GS_NEQ(gs_in,vgs[i]);
        vgs[i].set_text(filename[i]);
        ASSERT_GS_EQ(gs_in,vgs[i]);

    }

    return true;
}
void GraphProducerTester::add_some_GSs(GraphProducer& gp) const {
    INIT_BASICS();
    // CAREFUL: We're relying on default graph names and gs4 is a random graph!
    // Make sure no other (5,0.5) graphs are added separately!
    gp.add(gs1)
      .add_random(1,0.5,1)
      .add(gs2)
      .add_random({2,3},{0.1,0.9},false,3)
      .add(gs3)
      .add_random({4,6},{0.2},true,4)
      .add(gs4);
}
void GraphProducerTester::add_lots_of_GSs(GraphProducer& gp) const {
    INIT_BASICS();
    // CAREFUL: We're relying on default graph names and gs4 is a random graph!
    // Make sure no other (5,0.5) graphs are added separately!
    gp.add(gs1)
      .add_random(1,0.5,2)
      .add(gs2)
      .add_random({2,3},{0.1,0.9},false,3)
      .add(gs3)
      .add_random({4,6},{0.2,0.8,0.4},true,4)
      .add(gs4)
      .add_random_pstep({7,8,9},0.3,5)
      .add_random_pstep_range({10,11,12,13},0.2,0.8,0.15,6);
}
bool GraphProducerTester::add_all_types_of_GSs_test_dump() const {

    // Create a complex GP and dump into a clean directory
    string subdir = tmp_dir_name+"/all_types";
    ASSERT(utils__mkdir(subdir));
    GraphProducer gp1(subdir);
    add_some_GSs(gp1);
    ASSERT_NO_THROW(gp1.dump_graphs());

    // Get the GS objects from the GP, and read from the directory in a different
    // instance. Compare resulting objects.
    vector<GraphStats> vgs1_in = gp1.get();
    vector<GraphStats> vgs1_out = GraphProducer().add(DirectoryIterator(subdir)).get();

    // They may have different orders, the iteration through the DI object is OS-defined
    ASSERT_SAME_VEC_ELEMS(vgs1_in, vgs1_out);

    // Don't validate all names... just make sure they're here
    ASSERT_EQ(gp1.get().size(), gp1.get_paths().size());

    // Go again. This time, force dump and make sure we get DIFFERENT graphs (the random
    // graphs should change)
    GraphProducer gp2(subdir);
    add_some_GSs(gp2);
    ASSERT_NO_THROW(gp2.dump_graphs(true));
    vector<GraphStats> vgs2_in = gp2.get();
    vector<GraphStats> vgs2_out = GraphProducer().add(DirectoryIterator(subdir)).get();

    ASSERT_SAME_VEC_ELEMS(vgs2_in, vgs2_out);
    ASSERT_DIFF_VEC_ELEMS(vgs2_in, vgs1_out);
    ASSERT_EQ(gp2.get().size(), gp2.get_paths().size());

    // Now, do the same but with MOAR graphs

    // Create a complex GP and dump into a clean directory
    ASSERT(utils__delete_all_files_in_dir(subdir));
    GraphProducer gp3(subdir);
    add_lots_of_GSs(gp3);
    ASSERT_NO_THROW(gp3.dump_graphs());

    // Get the GS objects from the GP, and read from the directory in a different
    // instance. Compare resulting objects.
    vector<GraphStats> vgs3_in = gp3.get();
    vector<GraphStats> vgs3_out = GraphProducer().add(DirectoryIterator(subdir)).get();

    // They may have different orders, the iteration through the DI object is OS-defined
    ASSERT_SAME_VEC_ELEMS(vgs3_in, vgs3_out);
    ASSERT_EQ(gp3.get().size(), gp3.get_paths().size());

    // Go again. This time, force dump and make sure we get DIFFERENT graphs (the random
    // graphs should change)
    GraphProducer gp4(subdir);
    add_lots_of_GSs(gp4);
    ASSERT_NO_THROW(gp4.dump_graphs(true));
    vector<GraphStats> vgs4_in = gp4.get();
    vector<GraphStats> vgs4_out = GraphProducer().add(DirectoryIterator(subdir)).get();

    ASSERT_SAME_VEC_ELEMS(vgs4_in, vgs4_out);
    ASSERT_DIFF_VEC_ELEMS(vgs4_in, vgs3_out);
    ASSERT_EQ(gp4.get().size(), gp4.get_paths().size());

    return true;
}

}
