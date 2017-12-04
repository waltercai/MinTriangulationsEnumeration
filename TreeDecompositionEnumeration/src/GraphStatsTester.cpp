#include "GraphReader.h"
#include "GraphStats.h"
#include "GraphStatsTester.h"
#include "Utils.h"
using std::cin;

namespace tdenum {

#define X(test) +1
const int GraphStatsTester::total_tests_defined = (0 GRAPHSTATSTESTER_TEST_TABLE);
#undef X
GraphStatsTester& GraphStatsTester::set_range(int first, int last) {
    if (last > total_tests_defined) {
        last = total_tests_defined;
    }
    clear_all();
    int counter = -1;
    #define X(test) \
    ++counter; \
    if (counter >= last) { \
        return *this; \
    } \
    else if (counter >= first) { \
        TRACE(TRACE_LVL__TEST, "Setting test '" << #test << "'"); \
        set_##test(); \
    }
    GRAPHSTATSTESTER_TEST_TABLE
    #undef X
    return *this;
}

/**
 * General class implementation
 */
GraphStatsTester& GraphStatsTester::go() {
    //set_range(0, 32);
    #define X(test) if (flag_##test) {DO_TEST(test);}
    GRAPHSTATSTESTER_TEST_TABLE
    #undef X
    return *this;
}
GraphStatsTester::GraphStatsTester() :
    TestInterface("GraphStats Tester")
    #define X(test) , flag_##test(true)
    GRAPHSTATSTESTER_TEST_TABLE
    #undef X
    {}

// Setters / getters
#define X(test) \
GraphStatsTester& GraphStatsTester::set_##test() { flag_##test = true; return *this; } \
GraphStatsTester& GraphStatsTester::unset_##test() { flag_##test = false; return *this; } \
GraphStatsTester& GraphStatsTester::set_only_##test() { clear_all(); return set_##test(); }
GRAPHSTATSTESTER_TEST_TABLE
#undef X

GraphStatsTester& GraphStatsTester::set_all() {
    #define X(test) set_##test();
    GRAPHSTATSTESTER_TEST_TABLE
    #undef X
    return *this;
}
GraphStatsTester& GraphStatsTester::clear_all() {
    #define X(test) unset_##test();
    GRAPHSTATSTESTER_TEST_TABLE
    #undef X
    return *this;
}


/**
 * Macros
 */
#define WAIT_FOR_CHAR() do { \
        char _tmpc; \
        std::cin >> _tmpc; \
    } while(0)


// IO Macros
#define PATH_TO_DIFFERENT_GRAPH_FILE_DIR "Datasets/Difficult/BN/Grids"
#define PATH_TO_DIFFERENT_GRAPH_FILE_BASE "grid10x10.f5.wrap.uai"
#define PATH_TO_DIFFERENT_GRAPH_FILE (string(PATH_TO_DIFFERENT_GRAPH_FILE_DIR)+ string("/")+ string(PATH_TO_DIFFERENT_GRAPH_FILE_BASE))
#define GENERATE_GRP_GRAPHS() \
    Graph g0 = Graph(3); \
    Graph g1 = Graph(5).addEdge(4,3).addEdge(0,2).addEdge(0,1); \
    Graph g2 = Graph(5).randomize(0.52,2); \
    Graph g3 = GraphReader::read(PATH_TO_DIFFERENT_GRAPH_FILE); \
    string filename0 = tmp_dir_name+"/three_graph"; \
    string filename1 = tmp_dir_name+string("/five_graph.")+GRAPHSTATS_FILE_EXTENSION; \
    string filename2 = tmp_dir_name+"/"+GraphStats::get_default_filename_from_graph(g2); \
    string filename3 = tmp_dir_name+"/different_graph"; \
    string file0 = string("3,")+GRAPHSTATS_METADATA_TXT_NOT_RANDOM+",1"; \
    string file1 = string("5,")+GRAPHSTATS_METADATA_TXT_NOT_RANDOM+",1" \
                + string("\n0,1\n0,2\n3,4"); \
    string file2 = string("5,0.52,2") \
                + (g2.areNeighbors(0,1) ? string("\n0,1") : string("")) \
                + (g2.areNeighbors(0,2) ? string("\n0,2") : string("")) \
                + (g2.areNeighbors(0,3) ? string("\n0,3") : string("")) \
                + (g2.areNeighbors(0,4) ? string("\n0,4") : string("")) \
                + (g2.areNeighbors(1,2) ? string("\n1,2") : string("")) \
                + (g2.areNeighbors(1,3) ? string("\n1,3") : string("")) \
                + (g2.areNeighbors(1,4) ? string("\n1,4") : string("")) \
                + (g2.areNeighbors(2,3) ? string("\n2,3") : string("")) \
                + (g2.areNeighbors(2,4) ? string("\n2,4") : string("")) \
                + (g2.areNeighbors(3,4) ? string("\n3,4") : string(""))
#define GENERATE_AND_DUMP_GRP_GRAPHS() \
    GENERATE_GRP_GRAPHS(); \
    do { \
        if (!utils__file_exists(filename0)) { \
            utils__dump_string_to_file(file0,filename0); \
        } \
        if (!utils__file_exists(filename1)) { \
            utils__dump_string_to_file(file1,filename1); \
        } \
        if (!utils__file_exists(filename2)) { \
            utils__dump_string_to_file(file2,filename2); \
        } \
    } while(0)
#define CLEANUP_GRP_GRAPHS() \
    utils__delete_file(filename0); \
    utils__delete_file(filename1); \
    utils__delete_file(filename2); \
    utils__delete_file(filename3)

// Objects construction (for stat_ tests)
#define INIT_GNP(_n,_p,_inst) \
    double pr = _p; \
    Graph rand = Graph(_n).randomize(pr,_inst); \
    GraphStats gs(rand)
#define INIT_OBJECTS() \
    NodeSet pmc1 = {0,2,4}; \
    NodeSet pmc2 = {1,2,3}; \
    NodeSet ms0 = {0}; \
    NodeSet ms1 = {3,4}; \
    NodeSet ms2 = {0,1}; \
    ChordalGraph trng0(Graph(5).addEdge(1,2).addEdge(2,3)); \
    ChordalGraph trng1(Graph(5)); \
    ChordalGraph trng2(Graph(5).addEdge(0,1))

/**
 * Tests
 */
bool GraphStatsTester::construction() const {
    ASSERT_NO_THROW(GraphStats gs1);
    ASSERT(!utils__file_exists(GraphStats::get_default_filename_from_graph(Graph())));
    ASSERT_NO_THROW(GraphStats gs2(Graph(1)));
    GraphStats gs1;
    GraphStats gs2(Graph(1));
    ASSERT(!utils__file_exists(GraphStats::get_default_filename_from_graph(Graph())));
    // Validate flags and stuff
    ASSERT_EQ(gs1.get_graph(), Graph());
    ASSERT(!gs1.is_random());
    ASSERT(!utils__file_exists(GraphStats::get_default_filename_from_graph(Graph())));
    ASSERT(!gs1.integrity());
    ASSERT_EQ(gs2.get_graph(), Graph(1));
    ASSERT(!gs2.is_random());
    ASSERT(!gs2.integrity());
    return true;
}
bool GraphStatsTester::construct_random() const {
    Graph rand1(10), rand2(10);
    rand1.randomize(0.5, 1);
    rand2.randomize(0.5, 2);
    // Construct with default text to test it's generation
    GraphStats gs1(rand1);
    GraphStats gs2(rand2);

    // Test
    ASSERT(gs1.is_random());
    ASSERT(!gs1.integrity());
    ASSERT_EQ(gs1.get_graph(), rand1);
    ASSERT_EQ(gs1.get_instance(), 1);

    ASSERT(gs2.is_random());
    ASSERT(!gs2.integrity());
    ASSERT_EQ(gs2.get_graph(), rand2);
    ASSERT_EQ(gs2.get_instance(), 2);

    return true;
}
bool GraphStatsTester::read_simple() const {

    Graph g = Graph(5).addEdge(0,1).addEdge(2,3).addEdge(0,3);
    string file_contents = string("5,")+GRAPHSTATS_METADATA_TXT_NOT_RANDOM+",1\n0,1\n0,3\n2,3";
    string filename = tmp_dir_name+"/read_simple."+GRAPHSTATS_FILE_EXTENSION;
    ASSERT(utils__dump_string_to_file(file_contents, filename));
    ASSERT(utils__file_exists(filename));

    GraphStats gs;
    ASSERT_NO_THROW(gs = GraphStats::read(filename));
    ASSERT_EQ(gs.get_text(), filename);
    ASSERT(gs.text_valid());
    ASSERT(gs.integrity());
    ASSERT(!gs.integrity("silly_name"));
    ASSERT_GS_EQ(gs, GraphStats(g).set_text(filename)); // Text should be the only difference

    // Cleanup
    ASSERT(utils__delete_file(filename));
    return true;
}
bool GraphStatsTester::read_nonexisting() const {

    GraphStats gs(Graph(5));
    TRACE_LVL_OVERRIDE(TRACE_LVL__ALWAYS);
    ASSERT_NO_THROW(gs = GraphStats::read(tmp_dir_name+"/read_nonexisting"));    // Doesn't exist on disk
    TRACE_LVL_RESTORE();
    ASSERT(!gs.text_valid());
    ASSERT(!gs.integrity());
    ASSERT(!gs.integrity(tmp_dir_name+"/read_nonexisting"));
    ASSERT_EQ(gs.get_graph(), Graph()); // Old graph overridden
    ASSERT_GS_EQ(gs, GraphStats());
    ASSERT_EQ(gs.get_text(), GRAPHSTATS_NO_FILE_KNOWN);

    return true;
}
bool GraphStatsTester::read_different_extensions() const {

    string file_contents = string("5\n")+GRAPHSTATS_METADATA_TXT_NOT_RANDOM+"\n0,1\n0,3\n2,3";
    string filename = tmp_dir_name+"/read_different_extensions";  // No ext
    ASSERT(utils__dump_string_to_file(file_contents, filename));
    ASSERT(utils__file_exists(filename));

    // First, a foreign file
    GraphStats foreign_gs = GraphStats::read(PATH_TO_DIFFERENT_GRAPH_FILE);
    ASSERT(foreign_gs.text_valid());
    ASSERT_EQ(foreign_gs.get_text(), PATH_TO_DIFFERENT_GRAPH_FILE);
    ASSERT(foreign_gs.integrity());
    ASSERT(foreign_gs.integrity(PATH_TO_DIFFERENT_GRAPH_FILE));
    ASSERT_GS_EQ(foreign_gs, GraphStats(GraphReader::read(PATH_TO_DIFFERENT_GRAPH_FILE)));

    // Next, a file without the correct extension
    GraphStats gs;
    TRACE_LVL_OVERRIDE(TRACE_LVL__ALWAYS);
    ASSERT_NO_THROW(gs = GraphStats::read(filename));
    TRACE_LVL_RESTORE();
    ASSERT(!gs.text_valid());
    ASSERT(!gs.integrity());
    TRACE_LVL_OVERRIDE(TRACE_LVL__ALWAYS);
    ASSERT(!gs.integrity(filename));  // GraphReader should complain
    TRACE_LVL_RESTORE();
    ASSERT(!gs.integrity(PATH_TO_DIFFERENT_GRAPH_FILE));
    ASSERT_GS_EQ(gs, GraphStats());
    ASSERT_EQ(gs.get_text(), GRAPHSTATS_NO_FILE_KNOWN);

    // Cleanup
    ASSERT(utils__delete_file(filename));
    return true;
}
bool GraphStatsTester::read_and_dump() const {

    Graph g = Graph(5).addEdge(0,1).addEdge(2,3).addEdge(0,3);
    string file_contents = string("5,")+GRAPHSTATS_METADATA_TXT_NOT_RANDOM+",1\n0,1\n0,3\n2,3";
    string filename = tmp_dir_name+"/read_and_dump."+GRAPHSTATS_FILE_EXTENSION;
    ASSERT(utils__dump_string_to_file(file_contents, filename));
    ASSERT(utils__file_exists(filename));

    GraphStats gs = GraphStats::read(filename);
    ASSERT(gs.text_valid());
    ASSERT(gs.integrity());
    ASSERT(gs.integrity(filename));
    ASSERT_EQ(gs.get_text(), filename);
    ASSERT(utils__delete_file(filename));
    ASSERT(!utils__file_exists(filename));
    // Don't check validity... leave it undefined
    ASSERT(!gs.integrity());
    ASSERT(!gs.integrity(filename));
    ASSERT_NO_THROW(gs.dump(filename));
    ASSERT(utils__file_exists(filename));
    ASSERT(gs.integrity());
    ASSERT(gs.integrity(filename));
    ASSERT_GS_EQ(gs, GraphStats::read(filename));
    ASSERT(gs.text_valid());
    ASSERT_EQ(gs.get_text(), filename);

    // Cleanup
    ASSERT(utils__delete_file(filename));
    return true;
}
bool GraphStatsTester::read_and_dump_different_file() const {

    Graph g = Graph(5).addEdge(0,1).addEdge(2,3).addEdge(0,3);
    string file_contents = string("5,")+GRAPHSTATS_METADATA_TXT_NOT_RANDOM+",1\n0,1\n0,3\n2,3";
    string filename = tmp_dir_name+"/read_and_dump_different_file."+GRAPHSTATS_FILE_EXTENSION;
    string filename_new = tmp_dir_name+"/read_and_dump_different_file_new."+GRAPHSTATS_FILE_EXTENSION;
    ASSERT(utils__dump_string_to_file(file_contents, filename));
    ASSERT(utils__file_exists(filename));
    ASSERT(!utils__file_exists(filename_new));

    GraphStats gs = GraphStats::read(filename);
    ASSERT(gs.text_valid());
    ASSERT(gs.integrity());
    ASSERT(gs.integrity(filename));
    ASSERT_EQ(gs.get_text(), filename);
    ASSERT_EQ(gs.get_graph(), g);
    ASSERT(!utils__file_exists(filename_new));
    ASSERT_NO_THROW(gs.dump(filename_new));
    ASSERT(utils__file_exists(filename_new));
    ASSERT(gs.integrity());
    ASSERT(gs.integrity(filename)); // Old file still exists
    ASSERT(gs.integrity(filename_new));
    ASSERT(gs.text_valid());
    ASSERT_GS_EQ(gs, GraphStats::read(filename_new));   // Filename should be internally changed
    ASSERT_EQ(gs.get_text(), filename_new);

    // Cleanup and some more tests
    ASSERT(utils__delete_file(filename));
    ASSERT(gs.integrity());
    ASSERT(!gs.integrity(filename));
    ASSERT(gs.integrity(filename_new));
    ASSERT(utils__delete_file(filename_new));
    ASSERT(!gs.integrity());    // New file is also missing!
    ASSERT(!gs.integrity(filename));
    ASSERT(!gs.integrity(filename_new));

    return true;
}
bool GraphStatsTester::dump_without_ext() const {

    Graph g = Graph(5).addEdge(0,1).addEdge(2,3).addEdge(0,3);
    string file_contents = string("5\n")+GRAPHSTATS_METADATA_TXT_NOT_RANDOM+"\n0,1\n0,3\n2,3";
    string filename = tmp_dir_name+"/dump_without_ext";
    string filename_fixed = filename+"."+GRAPHSTATS_FILE_EXTENSION;

    GraphStats gs(g);
    ASSERT(!gs.text_valid());
    ASSERT(!gs.integrity());
    ASSERT(!utils__file_exists(filename_fixed));
    ASSERT_NO_THROW(gs.dump(filename));
    ASSERT(utils__file_exists(filename_fixed));
    ASSERT(gs.text_valid());
    ASSERT(gs.integrity());
    ASSERT(gs.integrity(filename_fixed));
    ASSERT_EQ(gs.get_graph(), g);
    ASSERT_GS_EQ(gs, GraphStats::read(filename_fixed));
    ASSERT_EQ(gs.get_text(), filename_fixed);

    return true;
}
bool GraphStatsTester::dump_random() const {

    GENERATE_GRP_GRAPHS();
    ASSERT(!utils__file_exists(filename2));
    ASSERT(utils__dump_string_to_file(file2, filename2));
    ASSERT(utils__file_exists(filename2));

    GraphStats gs(g2);
    ASSERT_EQ(gs.get_graph(), GraphStats::read(filename2).get_graph());
    ASSERT(!gs.text_valid());
    ASSERT(!gs.integrity());
    ASSERT(!utils__file_exists(filename1));
    ASSERT_NO_THROW(gs.dump(filename1));
    ASSERT(utils__file_exists(filename1));
    ASSERT(gs.text_valid());
    ASSERT(gs.integrity());
    ASSERT(gs.integrity(filename1));
    ASSERT(gs.integrity(filename2));    // Should also be true
    ASSERT(gs.text_valid());
    ASSERT_EQ(gs.get_text(), filename1);
    ASSERT_GS_EQ(gs.set_text(filename2), GraphStats::read(filename2));  // Completely equal, apart from filename

    // Cleanup
    ASSERT(utils__delete_file(filename1));
    ASSERT(utils__delete_file(filename2));
    return true;
}
bool GraphStatsTester::stats_basic_and_io() const {
    INIT_GNP(5,0.5,1);

    ASSERT_EQ(gs.get_n(), (unsigned)5);
    ASSERT_EQ(gs.get_m(), rand.getNumberOfEdges());
    ASSERT_EQ_FLOAT(gs.get_p(), pr);
    ASSERT_EQ_FLOAT(gs.get_ratio(), rand.getEdgeRatio());
    ASSERT(!gs.integrity());
    ASSERT(gs.is_random());
    ASSERT_EQ(gs.get_instance(), 1);
    gs.set_instance(2);
    ASSERT_EQ(gs.get_instance(), 2);

    return true;
}
bool GraphStatsTester::stats_random() const {
    INIT_GNP(5,0.5,1);

    ASSERT(gs.is_random());
    gs.set_instance(2);
    ASSERT_EQ_FLOAT(gs.get_p(), pr);
    ASSERT_EQ(gs.get_instance(), 2);
    gs.unset_random();
    ASSERT(!gs.is_random());
    ASSERT_EQ_FLOAT(gs.get_p(), pr);    // Don't lose the data!
    ASSERT_EQ(gs.get_instance(), 2);
    gs.set_random();
    ASSERT(gs.is_random());
    ASSERT_EQ_FLOAT(gs.get_p(), pr);    // Don't lose the data!
    ASSERT_EQ(gs.get_instance(), 2);
    ASSERT_EQ(rand, gs.get_graph());
    gs.unset_random();
    ASSERT(!gs.is_random());
    gs.set_p(0.75);
    ASSERT(gs.is_random());             // Setting P should set the random flag
    ASSERT_EQ_FLOAT(gs.get_p(), 0.75);  // Don't lose the data!
    ASSERT_EQ(gs.get_instance(), 2);
    ASSERT_EQ(gs.get_graph(), rand);

    return true;
}
bool GraphStatsTester::stats_errors_compound() const {
    INIT_GNP(5,0.5,1);

    #define ASSERT_PMC_NO_ERRORS(_gs) ASSERT((_gs).pmc_no_errors())

    // No errors
    ASSERT_NO_THROW(ASSERT(gs.ms_no_errors()));
    ASSERT_NO_THROW(ASSERT_PMC_NO_ERRORS(gs));
    ASSERT_NO_THROW(ASSERT(gs.trng_no_errors()));

    // This is the ugliest piece of code I've ever seen
    gs.set_reached_time_limit_ms();
    ASSERT_PMC_NO_ERRORS(gs);
    ASSERT(gs.trng_no_errors());
    ASSERT(!gs.ms_no_errors());
    gs.unset_reached_time_limit_ms();
    ASSERT_PMC_NO_ERRORS(gs);
    ASSERT(gs.trng_no_errors());
    ASSERT(gs.ms_no_errors());
    gs.set_reached_time_limit_ms();
    ASSERT_PMC_NO_ERRORS(gs);
    ASSERT(gs.trng_no_errors());
    gs.set_reached_count_limit_ms();
    ASSERT_PMC_NO_ERRORS(gs);
    ASSERT(gs.trng_no_errors());
    ASSERT(!gs.ms_no_errors());
    gs.unset_reached_time_limit_ms();
    ASSERT_PMC_NO_ERRORS(gs);
    ASSERT(gs.trng_no_errors());
    ASSERT(!gs.ms_no_errors());
    gs.unset_reached_count_limit_ms();
    ASSERT_PMC_NO_ERRORS(gs);
    ASSERT(gs.trng_no_errors());
    ASSERT(gs.ms_no_errors());
    gs.set_mem_error_ms();
    ASSERT_PMC_NO_ERRORS(gs);
    ASSERT(gs.trng_no_errors());
    ASSERT(!gs.ms_no_errors());
    gs.set_reached_time_limit_ms();
    ASSERT(!gs.ms_no_errors());
    gs.set_reached_count_limit_ms();
    ASSERT(!gs.ms_no_errors());
    gs.unset_reached_time_limit_ms();
    ASSERT(!gs.ms_no_errors());
    gs.unset_reached_count_limit_ms();
    ASSERT(!gs.ms_no_errors());

    // Let's test a little less this time
    ASSERT_NO_THROW(gs.set_mem_error_pmc(PMCAlg::first()));
    ASSERT_NO_THROW(ASSERT(!gs.pmc_no_errors(PMCAlg::first())));
    ASSERT_NO_THROW(ASSERT(gs.pmc_no_errors(PMCAlg::first()+1)));
    ASSERT_NO_THROW(ASSERT(gs.pmc_no_errors(PMCAlg::last()-1)));
    ASSERT_NO_THROW(gs.set_reached_time_limit_pmc(PMCAlg::first()+1));
    ASSERT_NO_THROW(ASSERT(!gs.pmc_no_errors(PMCAlg::first())));
    ASSERT_NO_THROW(ASSERT(!gs.pmc_no_errors(PMCAlg::first()+1)));
    ASSERT_NO_THROW(ASSERT(gs.pmc_no_errors(PMCAlg::last()-1)));
    ASSERT_NO_THROW(gs.unset_reached_time_limit_pmc(PMCAlg::first()+1));
    ASSERT_NO_THROW(ASSERT(!gs.pmc_no_errors(PMCAlg::first())));
    ASSERT_NO_THROW(ASSERT(gs.pmc_no_errors(PMCAlg::first()+1)));
    ASSERT_NO_THROW(ASSERT(gs.pmc_no_errors(PMCAlg::last()-1)));

    // Even less please
    gs.set_reached_count_limit_trng();
    ASSERT(!gs.trng_no_errors());
    gs.set_mem_error_trng();
    gs.set_reached_time_limit_trng();
    ASSERT(!gs.trng_no_errors());
    gs.unset_reached_count_limit_trng();
    gs.unset_reached_time_limit_trng();
    ASSERT(!gs.trng_no_errors());

    return true;
}
bool GraphStatsTester::stats_errors_time() const {
    INIT_GNP(5,0.5,1);

    ASSERT(!gs.reached_time_limit_ms());
    gs.set_reached_time_limit_ms();                                                     // Set MS time limit
    ASSERT(gs.reached_time_limit_ms());
    ASSERT(!gs.reached_time_limit_pmc(PMCAlg::first()));
    ASSERT(!gs.reached_time_limit_pmc(PMCAlg::first()+1));
    gs.set_reached_time_limit_pmc(PMCAlg::first());                                     // Set PMC time limit for first()
    ASSERT(gs.reached_time_limit_ms());
    ASSERT(gs.reached_time_limit_pmc(PMCAlg::first()));
    ASSERT(!gs.reached_time_limit_pmc(PMCAlg::first()+1));
    gs.set_reached_time_limit_pmc(set<PMCAlg>({PMCAlg::first(), PMCAlg::first()+2}));   // Set PMC time limit for first()+2
    ASSERT(gs.reached_time_limit_pmc(PMCAlg::first()));
    ASSERT(!gs.reached_time_limit_pmc(PMCAlg::first()+1));
    ASSERT(gs.reached_time_limit_pmc(PMCAlg::first()+2));
    gs.unset_reached_time_limit_ms();                                                   // Unset MS time limit
    ASSERT(!gs.reached_time_limit_ms());
    gs.unset_reached_time_limit_pmc(PMCAlg::first());                                   // Unset PMC time limit for first()
    ASSERT(!gs.reached_time_limit_pmc(PMCAlg::first()));
    ASSERT(!gs.reached_time_limit_pmc(PMCAlg::first()+1));
    ASSERT(gs.reached_time_limit_pmc(PMCAlg::first()+2));
    ASSERT(!gs.reached_time_limit_trng());
    gs.set_reached_time_limit_trng();                                                   // Set TRNG time limit
    ASSERT(gs.reached_time_limit_trng());

    return true;
}
bool GraphStatsTester::stats_errors_count() const {
    INIT_GNP(5,0.5,1);

    ASSERT(!gs.reached_count_limit_ms());
    gs.set_reached_count_limit_ms();
    ASSERT(gs.reached_count_limit_ms());
    ASSERT(!gs.reached_count_limit_trng());
    gs.set_reached_count_limit_trng();
    ASSERT(gs.reached_count_limit_trng());
    ASSERT(gs.reached_count_limit_ms());
    gs.unset_reached_count_limit_ms();
    ASSERT(!gs.reached_count_limit_ms());
    ASSERT(gs.reached_count_limit_trng());
    gs.unset_reached_count_limit_trng();
    ASSERT(!gs.reached_count_limit_trng());

    return true;
}
bool GraphStatsTester::stats_errors_mem() const {
    INIT_GNP(5,0.5,1);

    ASSERT(!gs.mem_error_ms());
    gs.set_mem_error_ms();
    ASSERT(gs.mem_error_ms());
    ASSERT(!gs.mem_error_pmc(PMCAlg::first()));
    ASSERT(!gs.mem_error_pmc(PMCAlg::first()+1));
    gs.set_mem_error_pmc(PMCAlg::first()+1);
    ASSERT(!gs.mem_error_pmc(PMCAlg::first()));
    ASSERT(gs.mem_error_pmc(PMCAlg::first()+1));
    ASSERT(!gs.mem_error_trng());
    gs.set_mem_error_trng();
    ASSERT(gs.mem_error_trng());

    return true;
}
bool GraphStatsTester::stats_object_count() const {
    ASSERT_NO_THROW(INIT_GNP(5,0.5,1));
    INIT_GNP(5,0.5,1);

    ASSERT_NO_THROW(gs.set_ms_count(100));
    ASSERT_NO_THROW(ASSERT_EQ(gs.get_ms_count(),100));
    ASSERT_NO_THROW(ASSERT_EQ(gs.get_ms_subgraph_count(), vector<long>({0,0,0,0,100})));
    TRACE_LVL_OVERRIDE(TRACE_LVL__ALWAYS);
    ASSERT_NO_THROW(gs.set_ms_subgraph_count({2,3,4}));  // Wrong number of vector elements! Nothing updated
    TRACE_LVL_RESTORE();
    ASSERT_NO_THROW(ASSERT_EQ(gs.get_ms_subgraph_count(), vector<long>({0,0,0,0,100})));
    ASSERT_NO_THROW(gs.set_ms_subgraph_count({2,3,4,5,6}));
    ASSERT_NO_THROW(ASSERT_EQ(gs.get_ms_subgraph_count(), vector<long>({2,3,4,5,6})));
    ASSERT_NO_THROW(ASSERT_EQ(gs.get_ms_count(), 6));
    for (int i=0; i<5; ++i) {
        ASSERT_NO_THROW(ASSERT_EQ(gs.get_ms_subgraph_count(i), 2+i));
    }
    ASSERT_NO_THROW(gs.set_pmc_count(50));
    ASSERT_NO_THROW(ASSERT_EQ(gs.get_pmc_count(), 50));
    ASSERT_NO_THROW(gs.set_trng_count(500));
    ASSERT_NO_THROW(ASSERT_EQ(gs.get_trng_count(), 500));

    return true;
}
bool GraphStatsTester::stats_objects() const {
    INIT_GNP(5,0.5,1);
    ASSERT_NO_THROW(INIT_OBJECTS());
    INIT_OBJECTS();

    // Try to fool the GS
    gs.set_ms_count(100);
    gs.set_ms_subgraph_count({2,3,4,5,6});
    gs.set_pmc_count(50);
    gs.set_trng_count(500);

    // Go
    ASSERT_NO_THROW(gs.set_ms(NodeSetSet(set<NodeSet>({ms1,ms2}))));    // Set MS[last] {ms1,ms2}
    ASSERT_NO_THROW(ASSERT_EQ(NodeSetSet({ms1,ms2}), gs.get_ms()));
    ASSERT_NO_THROW(ASSERT_EQ(gs.get_ms_count(), 2));
    ASSERT_NO_THROW(gs.set_ms_subgraphs(vector<NodeSetSet>({NodeSetSet(),NodeSetSet(),NodeSetSet(),set<NodeSet>({ms1}),set<NodeSet>({ms2})})));
    ASSERT_NO_THROW(ASSERT_EQ(gs.get_ms_count(), 1));                   // Set MS=[nill,nill,nill,{ms1},{ms2}]
    ASSERT_NO_THROW(ASSERT_EQ(gs.get_ms_subgraph_count(), vector<long>({0,0,0,1,1})));
    ASSERT_NO_THROW(ASSERT_EQ(NodeSetSet({ms2}), gs.get_ms()));
    ASSERT_NO_THROW(ASSERT_EQ(NodeSetSet({ms2}), gs.get_subgraph_ms(4)));
    ASSERT_NO_THROW(ASSERT_EQ(NodeSetSet({ms1}), gs.get_subgraph_ms(3)));
    ASSERT_NO_THROW(ASSERT_EQ(NodeSetSet(), gs.get_subgraph_ms(2)));
    ASSERT_NO_THROW(ASSERT_EQ(NodeSetSet(), gs.get_subgraph_ms(1)));
    ASSERT_NO_THROW(ASSERT_EQ(NodeSetSet(), gs.get_subgraph_ms(0)));
    ASSERT_NO_THROW(gs.set_ms_subgraphs(1,NodeSetSet({ms0})));           // Set MS=[nill,{ms0},nill,{ms1},{ms2}]
    ASSERT_NO_THROW(ASSERT_EQ(gs.get_ms_subgraph_count(), vector<long>({0,1,0,1,1})));
    ASSERT_NO_THROW(ASSERT_EQ(NodeSetSet({ms2}), gs.get_ms()));
    ASSERT_NO_THROW(ASSERT_EQ(NodeSetSet({ms2}), gs.get_subgraph_ms(4)));
    ASSERT_NO_THROW(ASSERT_EQ(NodeSetSet({ms1}), gs.get_subgraph_ms(3)));
    ASSERT_NO_THROW(ASSERT_EQ(NodeSetSet(), gs.get_subgraph_ms(2)));
    ASSERT_NO_THROW(ASSERT_EQ(NodeSetSet({ms0}), gs.get_subgraph_ms(1)));
    ASSERT_NO_THROW(ASSERT_EQ(NodeSetSet(), gs.get_subgraph_ms(0)));
    ASSERT_NO_THROW(ASSERT_EQ(gs.get_ms_subgraphs(), vector<NodeSetSet>({
                                    NodeSetSet(),
                                    NodeSetSet(set<NodeSet>({ms0})),
                                    NodeSetSet(),
                                    NodeSetSet(set<NodeSet>({ms1})),
                                    NodeSetSet(set<NodeSet>({ms2}))
                                    })));
    ASSERT_NO_THROW(gs.set_pmc(NodeSetSet({pmc1,pmc2})));               // Set PMC {pmc1,pmc2}
    ASSERT_NO_THROW(ASSERT_EQ(NodeSetSet({pmc1,pmc2}), gs.get_pmc()));
    ASSERT_NO_THROW(gs.set_trng({trng1,trng2}));                        // Set TRNG {trng1,trng2}
    ASSERT_NO_THROW(ASSERT_EQ(vector<ChordalGraph>({trng1,trng2}), gs.get_trng()));

    return true;
}
bool GraphStatsTester::stats_incremental() const {
    INIT_GNP(5,0.5,1);
    INIT_OBJECTS();
    gs.set_pmc(NodeSetSet({pmc1,pmc2}));
    gs.set_trng(vector<ChordalGraph>({trng1,trng2}));
    gs.set_ms_subgraphs(vector<NodeSetSet>({
                        NodeSetSet(),
                        NodeSetSet({ms0}),
                        NodeSetSet(),
                        NodeSetSet({ms1}),
                        NodeSetSet({ms2})
                        }));

    gs.add_sep(ms0);                                                // Set MS[last] {ms2,ms0}
    ASSERT_EQ(NodeSetSet({ms2,ms0}), gs.get_ms());
    ASSERT_EQ(gs.get_ms_count(), 2);
    gs.increment_ms_count();                                        // Set MS_COUNT[last] 3
    ASSERT_EQ(NodeSetSet({ms2,ms0}), gs.get_ms());
    ASSERT_EQ(gs.get_ms_count(), 3);
    gs.add_trng(trng0);                                             // Set TRNG {trng1,trng2,trng0}
    ASSERT_EQ(vector<ChordalGraph>({trng1,trng2,trng0}), gs.get_trng());
    ASSERT_EQ(gs.get_trng_count(), 3);
    gs.increment_trng_count();                                        // Set TRNG_COUNT 4
    ASSERT_EQ(gs.get_trng_count(), 4);
    ASSERT_EQ(vector<ChordalGraph>({trng1,trng2,trng0}), gs.get_trng());
//    ASSERT_EQ(gs.get_trng_count(), 3?4);    // Undefined. get_trng() may choose to update the count

    return true;
}
bool GraphStatsTester::stats_time_declarations() const {
    INIT_GNP(5,0.5,1);

    gs.set_ms_calc_time(1);                                 // Set MS time 1
    ASSERT_EQ(gs.get_ms_calc_time(),1);
    gs.set_trng_calc_time(1);                               // Set TRNG time 1
    ASSERT_EQ(gs.get_trng_calc_time(),1);
    gs.set_pmc_calc_time(PMCAlg::first(), 2);               // Set PMC time[first] 2
    ASSERT_EQ(gs.get_pmc_calc_time(PMCAlg::first()),2);
    gs.set_pmc_calc_time(PMCAlg::first()+1, 3);             // Set PMC time[first+1] 3
    ASSERT_EQ(gs.get_pmc_calc_time(PMCAlg::first()),2);
    ASSERT_EQ(gs.get_pmc_calc_time(PMCAlg::first()+1),3);
    gs.set_pmc_calc_time(set<PMCAlg>({PMCAlg::first()+1,PMCAlg::first()+2}),4);
    ASSERT_EQ(gs.get_pmc_calc_time(PMCAlg::first()),2);     // Set PMC time[first,+1,+2]=2,4,4
    ASSERT_EQ(gs.get_pmc_calc_time(PMCAlg::first()+1),4);
    ASSERT_EQ(gs.get_pmc_calc_time(PMCAlg::first()+2),4);
    gs.set_pmc_calc_time(vector<PMCAlg>({PMCAlg::first()+2,PMCAlg::first()+3}),5);
    ASSERT_EQ(gs.get_pmc_calc_time(PMCAlg::first()),2);     // Set PMC time[first,+1,+2,+3]=2,4,5,5
    ASSERT_EQ(gs.get_pmc_calc_time(PMCAlg::first()+1),4);
    ASSERT_EQ(gs.get_pmc_calc_time(PMCAlg::first()+2),5);
    ASSERT_EQ(gs.get_pmc_calc_time(PMCAlg::first()+3),5);

    return true;
}
bool GraphStatsTester::integrity_no_path_default_doesnt_exist() const {
    GraphStats gs;
    ASSERT(!utils__file_exists(GraphStats::get_default_filename_from_graph(Graph())));
    ASSERT(!gs.integrity());
    return true;
}
bool GraphStatsTester::integrity_no_path_or_dir_default_exists() const {
    Graph g = Graph(2).addEdge(1,0);
    string filename = GraphStats::get_default_filename_from_graph(g);
    string file_bad = string("2,")+GRAPHSTATS_METADATA_TXT_NOT_RANDOM+",1";
    string file_good = file_bad + "\n0,1";
    ASSERT(utils__delete_file(filename));   // Make sure it doesn't exist
    ASSERT(utils__dump_string_to_file("",filename));
    ASSERT(utils__file_exists(filename));

    GraphStats gs(g);
    ASSERT_OR_ELSE_DELETE_FILE(!gs.integrity(), filename);
    ASSERT_OR_ELSE_DELETE_FILE(!gs.integrity(filename), filename);
    ASSERT_OR_ELSE_DELETE_FILE(utils__dump_string_to_file(file_bad,filename), filename);
    ASSERT_OR_ELSE_DELETE_FILE(utils__read_file_as_string(filename) == file_bad, filename);
    ASSERT_OR_ELSE_DELETE_FILE(!gs.integrity(), filename);
    ASSERT_OR_ELSE_DELETE_FILE(!gs.integrity(filename), filename);
    ASSERT_OR_ELSE_DELETE_FILE(utils__dump_string_to_file(file_good,filename), filename);
    ASSERT_OR_ELSE_DELETE_FILE(utils__read_file_as_string(filename) == file_good, filename);
    ASSERT_OR_ELSE_DELETE_FILE(!gs.text_valid(), filename);
    ASSERT_OR_ELSE_DELETE_FILE(gs.integrity(), filename);         // GS constructed from graph, but default file exists and is consistent
    ASSERT_OR_ELSE_DELETE_FILE(gs.text_valid(), filename);
    ASSERT_OR_ELSE_DELETE_FILE(gs.integrity(filename), filename); // This should now work

    // Cleanup
    ASSERT(utils__delete_file(filename));

    return true;
}
bool GraphStatsTester::integrity_no_filename_default_doesnt_exist() const {
    Graph g = Graph(2).addEdge(1,0);
    GraphStats gs(g);
    string filepath = tmp_dir_name + "/" + GraphStats::get_default_filename_from_graph(g);

    ASSERT(!gs.integrity());
    ASSERT(!gs.integrity(tmp_dir_name));
    ASSERT(!gs.integrity());

    return true;
}
bool GraphStatsTester::integrity_no_filename_default_exists() const {
    Graph g = Graph(2).addEdge(1,0);
    GraphStats gs(g);
    string filepath = tmp_dir_name + "/" + GraphStats::get_default_filename_from_graph(g);
    string file_bad = string("2,")+GRAPHSTATS_METADATA_TXT_NOT_RANDOM+",1";
    string file_good = file_bad + "\n0,1";

    ASSERT(utils__dump_string_to_file("",filepath));
    ASSERT(utils__file_exists(filepath));
    ASSERT(!gs.integrity());
    ASSERT(!gs.integrity(tmp_dir_name+"/"));
    ASSERT(!gs.integrity());
    ASSERT(utils__dump_string_to_file(filepath,filepath));
    ASSERT_EQ(utils__read_file_as_string(filepath), filepath);
    ASSERT(!gs.integrity());
    ASSERT(!gs.integrity(tmp_dir_name+"/"));
    ASSERT(utils__dump_string_to_file(file_good,filepath));
    ASSERT_EQ(utils__read_file_as_string(filepath), file_good);
    ASSERT(!gs.integrity());        // GS constructed from graph
    ASSERT(gs.integrity(tmp_dir_name+"/")); // This should now work
    ASSERT(gs.integrity());         // We've found the file!

    // Cleanup
    ASSERT(utils__delete_file(filepath));
    return true;
}
bool GraphStatsTester::integrity_file_doesnt_exist() const {
    Graph g = Graph(2).addEdge(1,0);
    GraphStats gs(g);
    string filepath = tmp_dir_name + "/integrity_file_doesnt_exist";

    ASSERT(!gs.integrity());
    ASSERT(!gs.integrity(filepath));
    ASSERT(!gs.integrity());
    return true;
}
bool GraphStatsTester::integrity_file_exists() const {
    Graph g = Graph(2).addEdge(1,0);
    GraphStats gs(g);
    string filedir = tmp_dir_name+"/";
    string filepath = filedir + "integrity_file_exists";
    string filepath_with_ext = filepath + "." + GRAPHSTATS_FILE_EXTENSION;
    string file_bad = string("2,")+GRAPHSTATS_METADATA_TXT_NOT_RANDOM+",1";
    string file_good = file_bad + "\n0,1";

    ASSERT(utils__dir_exists(tmp_dir_name));
    ASSERT(utils__dir_exists(filedir));

    ASSERT(utils__dump_string_to_file("",filepath));
    ASSERT(utils__file_exists(filepath));
    ASSERT(!gs.integrity());
    ASSERT(!gs.integrity(filedir));
    TRACE_LVL_OVERRIDE(TRACE_LVL__ALWAYS);
    ASSERT(!gs.integrity(filepath));
    TRACE_LVL_RESTORE();
    ASSERT(!gs.integrity());
    ASSERT(utils__dump_string_to_file(file_bad,filepath));
    ASSERT_EQ(utils__read_file_as_string(filepath), file_bad);
    ASSERT(!gs.integrity());
    ASSERT(!gs.integrity(filedir));
    TRACE_LVL_OVERRIDE(TRACE_LVL__ALWAYS);
    ASSERT(!gs.integrity(filepath));
    TRACE_LVL_RESTORE();
    ASSERT(utils__dump_string_to_file(file_good,filepath));
    ASSERT_EQ(utils__read_file_as_string(filepath), file_good);
    ASSERT(!gs.integrity());        // GS constructed from graph
    ASSERT(!gs.integrity(filedir));
    TRACE_LVL_OVERRIDE(TRACE_LVL__ALWAYS);
    ASSERT(!gs.integrity(filepath)); // The file exists but has the wrong extension!
    TRACE_LVL_RESTORE();
    ASSERT(utils__dump_string_to_file(file_good,filepath_with_ext));
    ASSERT_EQ(utils__read_file_as_string(filepath_with_ext), file_good);
    ASSERT(!gs.integrity(filedir));  // Still no
    TRACE_LVL_OVERRIDE(TRACE_LVL__ALWAYS);
    ASSERT(!gs.integrity(filepath)); // The file exists but has the wrong extension!
    TRACE_LVL_RESTORE();
    ASSERT(gs.integrity(filepath_with_ext)); // NOW it should work
    ASSERT(gs.integrity());         // We've found the file, the previous failure doesn't matter!

    // Cleanup
    ASSERT(utils__delete_file(filepath));
    ASSERT(utils__delete_file(filepath_with_ext));
    return true;
}
bool GraphStatsTester::read_grp() const {
    GENERATE_AND_DUMP_GRP_GRAPHS();

    GraphStats gs = GraphStats::read(filename1);
    ASSERT(gs.text_valid());
    ASSERT(gs.integrity());
    ASSERT_EQ(gs.get_text(), filename1);
    ASSERT_EQ(gs.get_graph(), g1);

    return true;
}
bool GraphStatsTester::read_grp_and_dump() const {
    GENERATE_AND_DUMP_GRP_GRAPHS();

    GraphStats gs1 = GraphStats::read(filename1);
    ASSERT(gs1.integrity());
    ASSERT(gs1.integrity(filename1));
    ASSERT_EQ(gs1.get_text(), filename1);
    utils__delete_file(filename1);
    ASSERT(!utils__file_exists(filename1));
    ASSERT(!gs1.integrity());
    ASSERT(!gs1.integrity(filename1));
    // text should now be in undefined state
    ASSERT_NO_THROW(gs1.dump(filename1));
    ASSERT(gs1.integrity());
    ASSERT(gs1.integrity(filename1));
    GraphStats gs2 = GraphStats::read(filename1);
    ASSERT_GS_EQ(gs1,gs2);

    return true;
}
bool GraphStatsTester::read_non_grp() const {
    GENERATE_AND_DUMP_GRP_GRAPHS();

    GraphStats gs = GraphStats::read(PATH_TO_DIFFERENT_GRAPH_FILE);
    ASSERT(gs.integrity());
    ASSERT(gs.integrity(PATH_TO_DIFFERENT_GRAPH_FILE));
    ASSERT_EQ(gs.get_text(), PATH_TO_DIFFERENT_GRAPH_FILE);
    return true;
}
bool GraphStatsTester::read_non_grp_and_dump() const {
    GENERATE_AND_DUMP_GRP_GRAPHS();

    GraphStats gs1 = GraphStats::read(PATH_TO_DIFFERENT_GRAPH_FILE);
    ASSERT_EQ(gs1.get_graph(), g3);
    ASSERT(gs1.integrity());
    ASSERT(gs1.integrity(PATH_TO_DIFFERENT_GRAPH_FILE));
    ASSERT(!utils__file_exists(filename3));
    ASSERT(!gs1.integrity(filename3));
    ASSERT(gs1.integrity());
    ASSERT_NO_THROW(gs1.dump(filename3));
    ASSERT(gs1.integrity());
    ASSERT(gs1.integrity(filename3));
    ASSERT(gs1.integrity(PATH_TO_DIFFERENT_GRAPH_FILE));
    ASSERT_EQ(gs1.get_text(), filename3+"."+GRAPHSTATS_FILE_EXTENSION);
    GraphStats gs2 = GraphStats::read(filename3+"."+GRAPHSTATS_FILE_EXTENSION);
    ASSERT_GS_EQ(gs1,gs2);
    return true;
}
bool GraphStatsTester::construct_read_no_input() const {
    GraphStats gs;
    string default_name = GraphStats::get_default_filename_from_graph(Graph());
    string custom_name = "yolo";
    string g0_file = string("0,")+GRAPHSTATS_METADATA_TXT_NOT_RANDOM+",1";   // Contents of GraphStats(Graph()).dump()

    // Missing input
    // Nothing given:
    gs = GraphStats();
    ASSERT(!gs.integrity());     // File shouldn't exist at this point
    ASSERT(utils__dump_string_to_file(g0_file, default_name));
    gs = GraphStats();
    ASSERT_OR_ELSE_DELETE_FILE(!gs.text_valid(), default_name);
    ASSERT_OR_ELSE_DELETE_FILE(gs.integrity(), default_name);     // File exists, it should be found
    ASSERT_OR_ELSE_DELETE_FILE(gs.text_valid(), default_name);
    ASSERT_OR_ELSE_DELETE_FILE(gs.get_text() == string("./")+default_name, default_name);
    ASSERT_OR_ELSE_DELETE_FILE(gs.integrity(default_name), default_name); // Here's the file!
    ASSERT_OR_ELSE_DELETE_FILE(gs.get_text() == string("./")+default_name, default_name);
    ASSERT_OR_ELSE_DELETE_FILE(gs.integrity(), default_name);     // File exists, it should be found
    ASSERT_OR_ELSE_DELETE_FILE(gs.text_valid(), default_name);

    ASSERT(utils__delete_file(default_name));
    ASSERT(!gs.integrity());            // File no longer exists
    ASSERT(!gs.integrity(default_name));
    return true;
}
bool GraphStatsTester::construct_g_nonames() const {
    GraphStats gs = GraphStats(Graph(5));
    string file_contents = string("5,") + GRAPHSTATS_METADATA_TXT_NOT_RANDOM + ",1";
    string expected_filepath = string("./") + tmp_dir_name + "/" + GraphStats::get_default_filename_from_graph(Graph(5));
    ASSERT(utils__delete_file(expected_filepath));  // Make sure it's not already here
    ASSERT_EQ(gs.get_text(), GRAPHSTATS_NO_FILE_KNOWN);
    ASSERT(!gs.integrity());
    ASSERT_NO_THROW(gs.dump(expected_filepath));
    ASSERT(gs.integrity());
    ASSERT_EQ(utils__read_file_as_string(expected_filepath), file_contents);
    // Do it again, make sure it _still_ isn't from file
    ASSERT(utils__file_exists(expected_filepath));
    gs = GraphStats(Graph(5).addEdge(0,1));
    ASSERT_EQ(gs.get_text(), GRAPHSTATS_NO_FILE_KNOWN);
    ASSERT(!gs.integrity());
    ASSERT_EQ(utils__read_file_as_string(expected_filepath), file_contents);
    ASSERT_NO_THROW(gs.dump(expected_filepath));
    ASSERT(gs.integrity());
    ASSERT_EQ(utils__read_file_as_string(expected_filepath), file_contents+"\n0,1");
    ASSERT(utils__delete_file(expected_filepath));

    return true;
}
bool GraphStatsTester::construct_g_nonames_exists() const {
    GraphStats gs = GraphStats(Graph(5));
    string file_contents = string("5,") + GRAPHSTATS_METADATA_TXT_NOT_RANDOM + ",1";
    string expected_filepath = string("./") + GraphStats::get_default_filename_from_graph(Graph(5));
    ASSERT(utils__delete_file(expected_filepath));  // Make sure it's not already here
    ASSERT_EQ(gs.get_text(), GRAPHSTATS_NO_FILE_KNOWN);
    ASSERT(!gs.integrity());

    ASSERT(utils__dump_string_to_file(file_contents, expected_filepath));
    ASSERT_OR_ELSE_DELETE_FILE(utils__read_file_as_string(expected_filepath) == file_contents, expected_filepath);
    ASSERT_OR_ELSE_DELETE_FILE(GraphStats(Graph(5)).integrity(), expected_filepath); // Should find it anyway
    ASSERT_OR_ELSE_DELETE_FILE(GraphStats(Graph(5)).integrity(expected_filepath), expected_filepath);
    ASSERT_OR_ELSE_DELETE_FILE(gs.integrity(), expected_filepath);

    ASSERT_OR_ELSE_DELETE_FILE(utils__delete_file(expected_filepath), expected_filepath);  // Make sure it's not already here
    ASSERT_OR_ELSE_DELETE_FILE(!gs.integrity(), expected_filepath);
    ASSERT_OR_ELSE_DELETE_FILE(!gs.integrity(expected_filepath), expected_filepath);
    ASSERT_NO_THROW(ASSERT_OR_ELSE_DELETE_FILE(gs.dump(expected_filepath), expected_filepath));
    ASSERT_OR_ELSE_DELETE_FILE(gs.integrity(), expected_filepath);
    ASSERT_OR_ELSE_DELETE_FILE(gs.integrity(expected_filepath), expected_filepath);
    ASSERT_OR_ELSE_DELETE_FILE(utils__read_file_as_string(expected_filepath) == file_contents, expected_filepath);

    // Cleanup
    ASSERT(utils__delete_file(expected_filepath));
    return true;
}

}
