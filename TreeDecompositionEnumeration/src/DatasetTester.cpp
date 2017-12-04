#include "Dataset.h"
#include "DatasetTester.h"
#include "MinimalSeparatorsEnumerator.h"
#include "MinimalTriangulationsEnumerator.h"
#include "StatisticRequestTester.h"
#include <unistd.h>
#include <iostream>
using std::cout;
using std::endl;

namespace tdenum {

DatasetTester::DatasetTester() :
    TestInterface("Dataset Tester")
    #define X(test) , flag_##test(true)
    DATASETTESTER_TEST_TABLE
    #undef X
    {}

#define X(test) \
DatasetTester& DatasetTester::set_##test() { flag_##test = true; return *this; } \
DatasetTester& DatasetTester::unset_##test() { flag_##test = false; return *this; } \
DatasetTester& DatasetTester::set_only_##test() { clear_all(); return set_##test(); }
DATASETTESTER_TEST_TABLE
#undef X

DatasetTester& DatasetTester::set_all() {
    #define X(test) set_##test();
    DATASETTESTER_TEST_TABLE
    #undef X
    return *this;
}
DatasetTester& DatasetTester::clear_all() {
    #define X(test) unset_##test();
    DATASETTESTER_TEST_TABLE
    #undef X
    return *this;
}
DatasetTester& DatasetTester::go() {
    set_only_several_pmc_algorithms_check_times_and_errors_per_alg();
    #define X(test) if (flag_##test) {DO_TEST(test);}
    DATASETTESTER_TEST_TABLE
    #undef X
    return *this;
}

/**
 * Useful macros
 */
#define TO_DCI(_num) (static_cast<DatasetColumnIndex>(_num))

#define DO_QUIETLY(_) \
    TRACE_LVL_OVERRIDE(TRACE_LVL__ALWAYS); \
    _; \
    TRACE_LVL_RESTORE()

#define CALC_QUIETLY() \
    DO_QUIETLY(ASSERT_NO_THROW(ds.unset_verbose().calc().set_verbose()));

#define DUMP_GRAPHSTATS() do { \
    for (unsigned i=0; i<total_graphs; ++i) { \
        DUMP_GRAPHSTATS_BY_ID(i); \
    } } while(0)

#define DUMP_GRAPHSTATS_BY_ID(id) \
    ASSERT(gs[id].dump(filename[id]))

#define INIT_GRAPH_META(id) \
    MinimalSeparatorsEnumerator(g[id], UNIFORM).getAll(ms[id]); \
    ms_count[id] = ms[id].size(); \
    pmce.push_back(PMCEnumerator(g[id])); \
    pmc[id] = pmce[id].get(sr_all_pmc); \
    pmc_count[id] = pmc[id].size(); \
    ms_sub[id] = pmce[id].get_ms_subgraphs(); \
    ms_sub_count[id] = pmce[id].get_ms_count_subgraphs(); \
    trng[id] = MinimalTriangulationsEnumerator(g[id], NONE, UNIFORM, SEPARATORS).getAll(); \
    trng_count[id] = trng[id].size(); \
    filename[id] = tmp_dir_name+(string("/graph")+ UTILS__TO_STRING(id)+ ".grp"); \
    gs[id] = GraphStats(g[id]); \
    DUMP_GRAPHSTATS_BY_ID(id); \
    gp.add(filename[id])

#define INIT_GRAPH_META_DUMP(id) \
    INIT_GRAPH_META(id); \
    DUMP_GRAPHSTATS_BY_ID(id)

#define INIT() \
    GraphProducer gp = GraphProducer(tmp_dir_name); \
    GENERATE_STATREQS(); \
    double p3 = 0.5, p5 = 0.7, p7 = 0.3; \
    vector<Graph> g; \
    g.push_back(Graph(2)); \
    g.push_back(Graph(4).addEdge(0,1).addEdge(3,2).addEdge(1,2)); \
    g.push_back(Graph(5).randomize(p3)); \
    g.push_back(Graph(4).addEdge(0,1).addEdge(1,2).addEdge(2,3).addEdge(3,0)); \
    g.push_back(Graph(7).randomize(p5)); \
    g.push_back(Graph(6).addEdge(0,2).addEdge(2,4)); \
    g.push_back(Graph(10).randomize(p7)); \
    g.push_back(Graph()); \
    unsigned total_graphs = g.size(); \
    vector<NodeSetSet> ms(total_graphs); \
    vector<vector<NodeSetSet> > ms_sub(total_graphs); \
    vector<vector<long> > ms_sub_count(total_graphs); \
    vector<NodeSetSet> pmc(total_graphs); \
    vector<vector<ChordalGraph> > trng(total_graphs); \
    vector<long> ms_count(total_graphs); \
    vector<long> pmc_count(total_graphs); \
    vector<long> trng_count(total_graphs); \
    vector<PMCEnumerator> pmce;/*No default constructor*/ \
    vector<string> filename(total_graphs); \
    vector<GraphStats> gs(total_graphs); \
    for (unsigned i=0; i<total_graphs; ++i) { \
        INIT_GRAPH_META(i); \
    } \
    ASSERT_EQ(total_graphs, pmce.size()); \
    ASSERT_EQ(0/*No connected nodes to separate*/, ms[0].size()); \
    ASSERT_EQ(2/*Two inner nodes*/, ms[1].size()); \
    ASSERT_EQ(2/*Two node pairs*/, ms[3].size()); \
    ASSERT_EQ(1/*Node 2*/, ms[5].size()); \
    ASSERT_EQ(2/*Two isolated nodes*/, pmc[0].size()); \
    ASSERT_EQ(3/*Three pairs*/, pmc[1].size()); \
    ASSERT_EQ(4/*Four triangles, from two different triangulations*/, pmc[3].size()); \
    ASSERT_EQ(2+3/*Two pairs plus 3 remaining isolated nodes*/, pmc[5].size()); \
    ASSERT_EQ(vector<ChordalGraph>({ChordalGraph(g[0])}), trng[0]); \
    ASSERT_EQ(vector<ChordalGraph>({ChordalGraph(g[1])}), trng[1]); \
    ASSERT_EQ(vector<ChordalGraph>({ChordalGraph(g[5])}), trng[5]); \
    ASSERT_EQ(2, trng[3].size()); \
    ASSERT_EQ(gp.get().size(), total_graphs); \
    string dataset_filename = tmp_dir_name+"/dataset_out.csv"

#define INIT_DATASET() \
    INIT(); \
    ASSERT(Dataset::dump_barren_stat_file(dataset_filename, gp.get())); \
    Dataset ds = Dataset(dataset_filename); \
    ASSERT_EQ(ds.get_vector_gs().size(), total_graphs)

#define CLEANUP_BY_ID(id) \
    ASSERT(utils__delete_file(filename[id]))

#define CLEANUP_DATASET() \
    ASSERT(utils__delete_file(dataset_filename)); \
    CLEANUP()

#define CLEANUP() do { \
    for (unsigned i=0; i<total_graphs; ++i) { \
        CLEANUP_BY_ID(i); \
    } } while(0)

#define RESET_DATASET() \
    CLEANUP_DATASET(); \
    DUMP_GRAPHSTATS(); \
    ASSERT_EQ(gp.get(), gs); \
    ASSERT(Dataset::dump_barren_stat_file(dataset_filename, gp.get())); \
    ds.reset(dataset_filename); \
    ASSERT_EQ(ds.get_vector_gs().size(), total_graphs)

/**
 * TESTS!
 */
bool DatasetTester::find_by_text() const {
    INIT_DATASET();

    int i;
    auto vgs = gp.get();
    ASSERT_NO_THROW(i = Dataset::graph_index_by_text(vgs, "i dont exist"));
    ASSERT(i < 0 || i >= (int)ds.get_vector_gs().size());
    ASSERT_NO_THROW(i = ds.graph_index_by_text("i dont exist"));
    ASSERT(i < 0 || i >= (int)ds.get_vector_gs().size());
    ASSERT_EQ(vgs[0].get_text(), filename[0]);
    for (int i=0; i<(int)total_graphs; ++i) {
        ASSERT_EQ(i, Dataset::graph_index_by_text(vgs, filename[i]));
        ASSERT_EQ(i, ds.graph_index_by_text(filename[i]));
    }

    CLEANUP_DATASET();
    return true;
}
bool DatasetTester::validate_unique_text() const {
    INIT_DATASET();

    ASSERT(ds.unique_graph_text());

    CLEANUP_DATASET();
    return true;
}
bool DatasetTester::load_empty_dir() const {

    ASSERT(utils__dir_is_empty(tmp_dir_name));
    string file = tmp_dir_name+"/dataset_out.csv";
    Dataset::dump_barren_stat_file(file, vector<GraphStats>());
    ASSERT(utils__file_exists(file));
    Dataset ds(file);
    ASSERT_EQ(0, ds.get_vector_gs().size());

    ASSERT(utils__delete_file(file));
    return true;
}
bool DatasetTester::calc_trng_correct_order() const {
    INIT_DATASET();

    ASSERT_NO_THROW(ds.set_request(ds.graph_index_by_text(filename[2]), sr_trng));
    CALC_QUIETLY();
    ASSERT_EQ(ds.get_vector_gs()[2].get_trng(), trng[2]);

    CLEANUP_DATASET();
    return true;
}
bool DatasetTester::calc_ms_varying_statreqs() const {
    INIT_DATASET();

    vector<StatisticRequest> vsr({
        sr_all_ms, sr_all_ms, sr_ms, sr_count_ms, sr_count_ms, StatisticRequest(), sr_count_pmc, sr_all_pmc
    });
    ASSERT_EQ(vsr.size(), total_graphs);
    for (unsigned i=0; i<total_graphs; ++i) {
        ASSERT_NO_THROW(ds.set_request(ds.graph_index_by_text(filename[i]), vsr[i]));
    }
    CALC_QUIETLY();
    vector<GraphStats> vgs = ds.get_vector_gs();
    ASSERT_EQ(vgs.size(), total_graphs);

    for (unsigned i=0; i<total_graphs; ++i) {
        StatisticRequest sr = vsr[i];
        ASSERT_EQ(vgs[i].get_graph(), g[i]);
        // Make sure non-ms data wasn't calculated
        if (!sr.test_has_pmc_calculation()) {
            ASSERT_EQ(0, vgs[i].get_pmc_count());
        }
        if (!sr.test_has_trng_calculation()) {
            ASSERT_EQ(0, vgs[i].get_trng_count());
        }
        vector<long> tmp_ms_count(gs[i].get_n(),0);
        if((sr.test_has_pmc_calculation() || sr.test_has_ms_calculation()) && gs[i].get_n() > 0) {
            tmp_ms_count.back() = ms_count[i];
        }
        ASSERT_EQ(tmp_ms_count, vgs[i].get_ms_subgraph_count());
        // Make sure MS data was calculated
        ASSERT_EQ(vgs[i].get_ms_count(), ((sr.test_has_pmc_calculation() || sr.test_has_ms_calculation()) ? (int)ms_count[i] : 0));
        ASSERT_EQ(vgs[i].get_ms(), (sr.test_ms() ? ms[i] : NodeSetSet()));
    }

    // Now, do the same, but use calc_ms instead
    CLEANUP_DATASET();
    RESET_DATASET();
    vgs = ds.get_vector_gs();
    ASSERT_EQ(vgs.size(), total_graphs);

    for (unsigned i=0; i<total_graphs; ++i) {
        StatisticRequest sr = vsr[i];
        DO_QUIETLY(ASSERT_NO_THROW(ds.unset_verbose().calc_ms(vgs[i],sr).set_verbose()));
        ASSERT_EQ(0, vgs[i].get_pmc_count());
        ASSERT_EQ(0, vgs[i].get_trng_count());
        vector<long> tmp_ms_count(g[i].getNumberOfNodes(),0);
        if(sr.test_has_ms_calculation() && g[i].getNumberOfNodes() > 0) {
            tmp_ms_count.back() = ms_count[i];
        }
        ASSERT_EQ(tmp_ms_count, vgs[i].get_ms_subgraph_count());
        /* Data that IS calculated: */
        ASSERT_EQ(vgs[i].get_ms_count(), (sr.test_has_ms_calculation() ? (int)ms[i].size() : 0));
        ASSERT_EQ(vgs[i].get_ms(), (sr.test_ms() ? ms[i] : NodeSetSet()));
    }

    CLEANUP_DATASET();
    return true;
}
bool DatasetTester::calc_pmc_varying_statreqs() const {
    INIT_DATASET();

    vector<StatisticRequest> vsr({
        sr_all_pmc, sr_all_pmc, sr_pmc, sr_count_pmc, sr_count_pmc, StatisticRequest(), sr_count_ms, sr_all_ms
    });
    ASSERT_EQ(vsr.size(), total_graphs);
    for (unsigned i=0; i<total_graphs; ++i) {
        ASSERT_NO_THROW(ds.set_request(ds.graph_index_by_text(filename[i]), vsr[i]));
    }
    CALC_QUIETLY();
    vector<GraphStats> vgs = ds.get_vector_gs();
    ASSERT_EQ(vgs.size(), total_graphs);

    for (unsigned i=0; i<total_graphs; ++i) {
        StatisticRequest sr = vsr[i];
        ASSERT_EQ(vgs[i].get_graph(), g[i]);
         // Make sure non-pmc data wasn't calculated.
        // Note that MS-count is calculated when PMCs are, as a by-product
        if (!sr.test_has_ms_calculation() && !sr.test_has_pmc_calculation()) {
            ASSERT_EQ(0, vgs[i].get_ms_count());
        }
        if (!sr.test_has_trng_calculation()) {
            ASSERT_EQ(0, vgs[i].get_trng_count());
        }
        // Make sure PMC data was calculated
        ASSERT_EQ(vgs[i].get_pmc_count(), (sr.test_has_pmc_calculation() ? (int)pmc_count[i] : 0));
        ASSERT_EQ(vgs[i].get_pmc(), (sr.test_pmc() ? pmc[i] : NodeSetSet()));
        auto tmp_ms_subgraph_count = vector<long>(vgs[i].get_n(), 0);
        if (vgs[i].get_n() > 0 && (sr.test_count_ms() || sr.test_has_pmc_calculation())) {
            tmp_ms_subgraph_count.back() = ms_count[i];
        }
        ASSERT_EQ(vgs[i].get_ms_subgraph_count(), (sr.test_ms_subgraph_count() ? ms_sub_count[i] : tmp_ms_subgraph_count));
        vector<NodeSetSet> tmp_ms_sub(vgs[i].get_n(), NodeSetSet());
        if (gs[i].get_n() > 0 && sr.test_ms()) {
            tmp_ms_sub.back() = ms[i];
        }
        ASSERT_EQ(vgs[i].get_ms_subgraphs(), (sr.test_ms_subgraphs() ? ms_sub[i] : tmp_ms_sub));
    }

    // Now, do the same, but use calc_pmc instead.
    // Note that calculations done in PMC mode but not defined as a PMC calculation (ms-count, for example)
    // won't be calculated when calling calc_pmc()!
    CLEANUP_DATASET();
    RESET_DATASET();
    vgs = ds.get_vector_gs();
    ASSERT_EQ(vgs.size(), total_graphs);
    ASSERT_EQ(gp.get(), vgs);

    for (unsigned i=0; i<total_graphs; ++i) {
        StatisticRequest sr = vsr[i];
        auto tmp_ms_subgraph_count = vector<long>(vgs[i].get_n(), 0);
        /*No calculation is done if it isn't part of the PMC calculations. Hence,
          if test_has_pmc_calculation() is false, reset the subcount vector*/
        if (sr.test_has_pmc_calculation() && gs[i].get_n() > 0) {
            tmp_ms_subgraph_count.back() = ms_count[i];
        }
        ASSERT_NO_THROW(ds.unset_verbose().calc_pmc(vgs[i],sr).set_verbose());
        /* Data that isn't calculated: */
        ASSERT_EQ(0, vgs[i].get_trng_count());
        /* Data that IS calculated: */
        ASSERT_EQ(vgs[i].get_ms_count(), (sr.test_has_pmc_calculation()  ? (int)ms_count[i] : 0));
        ASSERT_EQ(vgs[i].get_pmc_count(), (sr.test_has_pmc_calculation() ? (int)pmc_count[i] : 0));
        ASSERT_EQ(vgs[i].get_pmc(), (sr.test_pmc() ? pmc[i] : NodeSetSet()));
        ASSERT_EQ(vgs[i].get_ms_subgraph_count(), (sr.test_ms_subgraph_count() || sr.test_ms_subgraphs() ? ms_sub_count[i] : tmp_ms_subgraph_count));
        vector<NodeSetSet> tmp_ms_sub(vgs[i].get_n(), NodeSetSet());
        if (gs[i].get_n() > 0 && sr.test_ms()) {
            tmp_ms_sub.back() = ms[i];
        }
        ASSERT_EQ(vgs[i].get_ms_subgraphs(), (sr.test_ms_subgraphs() ? ms_sub[i] : tmp_ms_sub));
    }

    CLEANUP_DATASET();
    return true;
}
bool DatasetTester::calc_trng_varying_statreqs() const {
    INIT_DATASET();

    vector<StatisticRequest> vsr({
        sr_all_trng, sr_all_trng, sr_trng, sr_count_trng, sr_count_trng, StatisticRequest(), sr_count_pmc, sr_all_pmc
    });
    ASSERT_EQ(vsr.size(), total_graphs);
    for (unsigned i=0; i<total_graphs; ++i) {
        ASSERT_NO_THROW(ds.set_request(ds.graph_index_by_text(filename[i]), vsr[i]));
    }
    CALC_QUIETLY();
    vector<GraphStats> vgs = ds.get_vector_gs();
    ASSERT_EQ(vgs.size(), total_graphs);

    for (unsigned i=0; i<total_graphs; ++i) {
        ASSERT_EQ(vgs[i].get_graph(), g[i]);
        StatisticRequest sr = vsr[i];
        // Make sure irrelevant data wasn't calculated
        if (!sr.test_has_pmc_calculation()) {
            ASSERT_EQ(0, vgs[i].get_pmc_count());
        }
        if (!sr.test_has_ms_calculation() && !sr.test_has_pmc_calculation()) {
            ASSERT_EQ(0, vgs[i].get_ms_count());
        }
        vector<long> tmp_ms_count(gs[i].get_n(),0);
        if((sr.test_has_pmc_calculation() || sr.test_has_ms_calculation()) && gs[i].get_n() > 0) {
            tmp_ms_count.back() = ms_count[i];
        }
        ASSERT_EQ(tmp_ms_count, vgs[i].get_ms_subgraph_count());
        // Make sure TRNG data was calculated
        ASSERT_EQ(vgs[i].get_trng_count(), (sr.test_has_trng_calculation() ? trng_count[i] : 0));
        ASSERT_SAME_VEC_ELEMS(vgs[i].get_trng(), (sr.test_trng() ? trng[i] : vector<ChordalGraph>()));
    }

    // Now, do the same, but use calc_trng instead
    CLEANUP_DATASET();
    RESET_DATASET();
    vgs = ds.get_vector_gs();
    ASSERT_EQ(vgs.size(), total_graphs);

    for (unsigned i=0; i<total_graphs; ++i) {
        StatisticRequest sr = vsr[i];
        vector<long> tmp_ms_count(gs[i].get_n(),0);
        // Data that isn't calculated.
        // Note that ms_count won't be calculated when calling calc_trng()
        ASSERT_NO_THROW(ds.unset_verbose().calc_trng(vgs[i],sr).set_verbose());
        ASSERT_EQ(0, vgs[i].get_pmc_count());
        ASSERT_EQ(0, vgs[i].get_ms_count());
        ASSERT_EQ(tmp_ms_count, vgs[i].get_ms_subgraph_count());
        // Data that IS calculated:
        ASSERT_EQ(vgs[i].get_trng_count(), (sr.test_has_trng_calculation() ? (int)trng_count[i] : 0));
        ASSERT_SAME_VEC_ELEMS(vgs[i].get_trng(), (sr.test_trng() ? trng[i] : vector<ChordalGraph>()));
    }

    CLEANUP_DATASET();
    return true;
}
bool DatasetTester::stupid_GS_dump_no_preexisting_flag_true() const {
    INIT();

    vector<GraphStats> vgs0({gs[1],gs[3]});
    ASSERT(!utils__file_exists(dataset_filename));
    Dataset ds = Dataset(dataset_filename);
    ASSERT(!utils__file_exists(dataset_filename));
    ASSERT_NO_THROW(ds.dump_barren_stat_file(dataset_filename, vgs0, true));
    ASSERT(utils__file_exists(dataset_filename));
    ASSERT_EQ(ds.get_vector_gs(), vector<GraphStats>());
    ASSERT_NO_THROW(ds.load());
    ASSERT_EQ(ds.get_vector_gs(), vgs0);

    CLEANUP_DATASET();
    return true;
}
bool DatasetTester::stupid_GS_dump_no_preexisting_flag_false() const {
    INIT();

    vector<GraphStats> vgs0({gs[1],gs[3]});
    ASSERT(!utils__file_exists(dataset_filename));
    Dataset ds = Dataset(dataset_filename);
    ASSERT(!utils__file_exists(dataset_filename));
    ASSERT_NO_THROW(ds.dump_barren_stat_file(dataset_filename, vgs0, false));
    ASSERT(utils__file_exists(dataset_filename));
    ASSERT_EQ(ds.get_vector_gs(), vector<GraphStats>());
    ASSERT_NO_THROW(ds.load());
    ASSERT_EQ(ds.get_vector_gs(), vgs0);

    CLEANUP_DATASET();
    return true;
}
bool DatasetTester::stupid_GS_dump_with_preexisting_flag_true() const {
    INIT_DATASET();

    vector<GraphStats> vgs0({gs[1],gs[3]});
    ASSERT(utils__file_exists(dataset_filename));
    Dataset ds2 = Dataset(dataset_filename);
    ASSERT(utils__file_exists(dataset_filename));
    ASSERT_NO_THROW(ds2.dump_barren_stat_file(dataset_filename, vgs0, true));
    ASSERT(utils__file_exists(dataset_filename));
    ASSERT_EQ(ds2.get_vector_gs(), ds.get_vector_gs()); // The file existed, we should have loaded the old file
    ASSERT_NO_THROW(ds2.load());
    ASSERT_EQ(ds2.get_vector_gs(), vgs0);

    CLEANUP_DATASET();
    return true;
}
bool DatasetTester::stupid_GS_dump_with_preexisting_flag_false() const {
    INIT_DATASET();

    vector<GraphStats> vgs0({gs[1],gs[3]});
    vector<GraphStats> vgs1 = ds.get_vector_gs();
    ASSERT(utils__file_exists(dataset_filename));
    Dataset ds2 = Dataset(dataset_filename);
    ASSERT(utils__file_exists(dataset_filename));
    DO_QUIETLY(ASSERT_NO_THROW(ds2.dump_barren_stat_file(dataset_filename, vgs0, false)));
    ASSERT(utils__file_exists(dataset_filename));
    ASSERT_NEQ(ds2.get_vector_gs(), vgs0); // Not overwritten!
    ASSERT_EQ(ds2.get_vector_gs(), vgs1);
    ASSERT_NO_THROW(ds.load()); // Use original Dataset instance, make sure it's consistent
    ASSERT_EQ(ds.get_vector_gs(), vgs1);

    CLEANUP_DATASET();
    return true;
}
bool DatasetTester::construct_via_load_no_statfile_exists() const {
    string nonexistent = tmp_dir_name+"/i_dont_exist.csv";
    ASSERT_NO_THROW(Dataset ds(nonexistent));
    Dataset ds(nonexistent);
    ASSERT_EQ(ds.get_vector_gs(),vector<GraphStats>());
    ASSERT(!utils__file_exists(nonexistent));
    return true;
}
bool DatasetTester::construct_via_load_statfile_exists() const {
    INIT();
    ASSERT(Dataset::dump_barren_stat_file(dataset_filename, gp.get()));
    Dataset ds(dataset_filename);
    ASSERT_EQ(ds.get_vector_gs().size(), total_graphs);
    CLEANUP_DATASET();
    return true;
}
bool DatasetTester::construct_via_load_missing_sources() const {
    INIT();
    ASSERT(Dataset::dump_barren_stat_file(dataset_filename, gp.get()));
    ASSERT(utils__delete_file(filename[1]));
    ASSERT(utils__delete_file(filename[3]));
    DO_QUIETLY(Dataset ds(dataset_filename));
    ASSERT_EQ(ds.get_vector_gs(), vector<GraphStats>({gs[0],gs[2],gs[4],gs[5],gs[6],gs[7]}));
    CLEANUP_DATASET();
    return true;
}
bool DatasetTester::construct_via_GS_paths_all_valid() const {
    INIT();

    ASSERT(!utils__file_exists(dataset_filename));
    Dataset ds(dataset_filename, vector<string>({
            filename[0], filename[2], filename[4]
        }));
    ASSERT_EQ(ds.get_vector_gs(), vector<GraphStats>({gs[0],gs[2],gs[4]}));

    ASSERT(!utils__file_exists(dataset_filename));
    ASSERT(ds.dump());
    ASSERT(utils__file_exists(dataset_filename));
    ASSERT_EQ(Dataset(dataset_filename), ds);

    CLEANUP_DATASET();
    return true;
}
bool DatasetTester::construct_via_GS_paths_some_invalid() const {
    INIT();

    ASSERT(!utils__file_exists(dataset_filename));
    DO_QUIETLY(Dataset ds(dataset_filename, vector<string>({
            filename[0], filename[2], filename[2]+"whoops", filename[4]
        })));
    ASSERT_EQ(ds.get_vector_gs(), vector<GraphStats>({gs[0],gs[2],gs[4]}));

    ASSERT(!utils__file_exists(dataset_filename));
    ASSERT(ds.dump());
    ASSERT(utils__file_exists(dataset_filename));
    ASSERT_EQ(Dataset(dataset_filename), ds);

    CLEANUP_DATASET();
    return true;
}
bool DatasetTester::basic_calc_same_statreq() const {
    INIT_DATASET();

    StatisticRequest sr = StatisticRequest().set_single_pmc_alg().set_ms_subgraphs().set_trng();
    ASSERT(sr.test_ms_subgraphs());
    ASSERT(sr.test_trng());
    ASSERT(sr.test_has_pmc_calculation());
    ASSERT_NO_THROW(ds.set_all_requests(sr));
    CALC_QUIETLY();

    // Validate data
    auto vgs = ds.get_vector_gs();
    ASSERT_EQ(vgs.size(), total_graphs);

    for (unsigned i=0; i<total_graphs; ++i) {
        ASSERT_SAME_VEC_ELEMS(vgs[i].get_trng(), trng[i]);
        ASSERT_SAME_VEC_ELEMS(vgs[i].get_ms_subgraphs(), ms_sub[i]);
        ASSERT_EQ(vgs[i].get_ms(), ms[i]);
        ASSERT_EQ(vgs[i].get_ms_count(), ms_count[i]);
        ASSERT_EQ(vgs[i].get_trng_count(), trng_count[i]);
    }

    CLEANUP_DATASET();
    return true;
}
bool DatasetTester::permute_stat_request_fields_check_output_columns() const {
    INIT_DATASET();

    // Calculate, read and validate the stats
    StatisticRequest sr_ms_subgraphs_and_trng = StatisticRequest(sr_ms_subgraphs).set_trng();
    ds.set_request(0, sr_ms);
    ds.set_request(1, sr_pmc);
    ds.set_request(2, sr_trng);
    ds.set_request(3, sr_ms_subgraphs_and_trng);
    ds.set_request(4, StatisticRequest());
    ds.set_request(5, sr_count_ms);
    ds.set_request(6, sr_trng);
    ds.set_request(7, sr_ms_subgraphs_and_trng);
    ASSERT_EQ(ds.dataset, vector<GraphReq>({
            GraphReq(gs[0], sr_ms),
            GraphReq(gs[1], sr_pmc),
            GraphReq(gs[2], sr_trng),
            GraphReq(gs[3], sr_ms_subgraphs_and_trng),
            GraphReq(gs[4], StatisticRequest()),
            GraphReq(gs[5], sr_count_ms),
            GraphReq(gs[6], sr_trng),
            GraphReq(gs[7], sr_ms_subgraphs_and_trng)
        }));
    CALC_QUIETLY();
    ds.dump();
    Dataset ds_fromfile(dataset_filename);

    // Note that only numeric stats can be loaded from file, and currently
    ASSERT_EQ(ds.dataset.size(), ds_fromfile.dataset.size());
    for (unsigned i=0; i<total_graphs; ++i) {
        GraphStats gs = ds.dataset[i].first;
        StatisticRequest sr = ds.dataset[i].second;
        ASSERT_EQ(gs.get_ms_count(), ((sr.test_has_ms_calculation() || sr.test_has_pmc_calculation()) ? ms_count[i] : 0));
        ASSERT_EQ(gs.get_pmc_count(), ((sr.test_count_pmc() || sr.test_pmc()) ? pmc_count[i] : 0));
        ASSERT_EQ(gs.get_trng_count(), (sr.test_has_trng_calculation() ? trng_count[i] : 0));
    }

    CLEANUP_DATASET();
    return true;
}
bool DatasetTester::column_indexes_correct() const {
    /**
     * Dump an empty dataset file, read the columns and make sure the PMC algorithms names
     * are substrings of the column contents.
     */
    INIT_DATASET();

    // Sanity
    ASSERT_GEQ(DATASET_COL_TOTAL, DATASET_COL_TOTAL_NONPMCALG);
    ASSERT_EQ(DATASET_COL_TOTAL, DATASET_COL_INT_TO_STR_MAP.size());
    ASSERT_EQ(DATASET_COL_TOTAL, DATASET_COL_STR_TO_INT_MAP.size());
    ASSERT_EQ(DATASET_COL_TOTAL, DATASET_COL_TOTAL_NONPMCALG + 2*PMCAlg::total_algs());
    ASSERT_EQ(PMCAlg::total_algs(), (int)DATASET_COL_ALG_TO_TIME_INT.size());
    ASSERT_EQ(PMCAlg::total_algs(), (int)DATASET_COL_ALG_TO_ERR_INT.size());

    // Get the actual header string
    vector<vector<string> > csv = utils__read_csv(dataset_filename);
    ASSERT(!csv.empty());
    vector<string> header = csv[0];
    ASSERT_EQ(header.size(), DATASET_COL_TOTAL);

    // Manually check all non-PMCAlg column headers
    ASSERT_GEQ(DATASET_COL_TOTAL_NONPMCALG, 12);
    ASSERT_EQ(header[0], DATASET_COL_STR_TXT);
    ASSERT_EQ(header[1], DATASET_COL_STR_NODES);
    ASSERT_EQ(header[2], DATASET_COL_STR_EDGES);
    ASSERT_EQ(header[3], DATASET_COL_STR_MSS);
    ASSERT_EQ(header[4], DATASET_COL_STR_PMCS);
    ASSERT_EQ(header[5], DATASET_COL_STR_TRNG);
    ASSERT_EQ(header[6], DATASET_COL_STR_P);
    ASSERT_EQ(header[7], DATASET_COL_STR_RATIO);
    ASSERT_EQ(header[8], DATASET_COL_STR_MS_TIME);
    ASSERT_EQ(header[9], DATASET_COL_STR_TRNG_TIME);
    ASSERT_EQ(header[10], DATASET_COL_STR_ERR_TIME);
    ASSERT_EQ(header[11], DATASET_COL_STR_ERR_CNT);

    // Non-PMCAlg related column tests:
    for (int i=0; i<DATASET_COL_TOTAL_NONPMCALG; ++i) {
        ASSERT_EQ(header[i], DATASET_COL_INT_TO_STR_MAP.at(DatasetColumnIndex(i)));
        ASSERT_EQ(DatasetColumnIndex(i), DATASET_COL_STR_TO_INT_MAP.at(header[i]));
    }

    // Algorithm related columns:
    for (int i=DATASET_COL_TOTAL_NONPMCALG; i<DATASET_COL_TOTAL; ++i) {
        int alg_index = (i-DATASET_COL_TOTAL_NONPMCALG)/2;
        PMCAlg alg = PMCAlg(static_cast<PMCALG_ENUM>(alg_index));
        // Make sure the actual PMCAlg name appears in the column header:
        ASSERT_SUBSTR(alg.str(), header[i]);
        string expected = (i-DATASET_COL_TOTAL_NONPMCALG)%2 ?
            DATASET_COL_INT_TO_STR_MAP.at(DatasetColumnIndex(DATASET_COL_ALG_TO_ERR_INT.at(alg))) : // Error column
            DATASET_COL_INT_TO_STR_MAP.at(DatasetColumnIndex(DATASET_COL_ALG_TO_TIME_INT.at(alg))); // Time column
        ASSERT_EQ(header[i], expected);
    }

    // Manually check algorithm columns
    ASSERT_EQ(DATASET_COL_STR_NORMAL, header[DATASET_COL_TOTAL_NONPMCALG+0]);
    ASSERT_EQ(DATASET_COL_STR_NORMAL,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_TIME_INT.at(PMCAlg(PMCALG_ENUM_NORMAL)))));
    ASSERT_EQ(DATASET_COL_STR_REVERSE_MS, header[DATASET_COL_TOTAL_NONPMCALG+2]);
    ASSERT_EQ(DATASET_COL_STR_REVERSE_MS,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_TIME_INT.at(PMCAlg(PMCALG_ENUM_REVERSE_MS)))));
    ASSERT_EQ(DATASET_COL_STR_ASCENDING, header[DATASET_COL_TOTAL_NONPMCALG+4]);
    ASSERT_EQ(DATASET_COL_STR_ASCENDING,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_TIME_INT.at(PMCAlg(PMCALG_ENUM_ASCENDING)))));
    ASSERT_EQ(DATASET_COL_STR_ASCENDING_REVERSE_MS, header[DATASET_COL_TOTAL_NONPMCALG+6]);
    ASSERT_EQ(DATASET_COL_STR_ASCENDING_REVERSE_MS,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_TIME_INT.at(PMCAlg(PMCALG_ENUM_ASCENDING_REVERSE_MS)))));
    ASSERT_EQ(DATASET_COL_STR_DESCENDING, header[DATASET_COL_TOTAL_NONPMCALG+8]);
    ASSERT_EQ(DATASET_COL_STR_DESCENDING,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_TIME_INT.at(PMCAlg(PMCALG_ENUM_DESCENDING)))));
    ASSERT_EQ(DATASET_COL_STR_DESCENDING_REVERSE_MS, header[DATASET_COL_TOTAL_NONPMCALG+10]);
    ASSERT_EQ(DATASET_COL_STR_DESCENDING_REVERSE_MS,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_TIME_INT.at(PMCAlg(PMCALG_ENUM_DESCENDING_REVERSE_MS)))));
    ASSERT_EQ(DATASET_COL_STR_RANDOM_RENAME, header[DATASET_COL_TOTAL_NONPMCALG+12]);
    ASSERT_EQ(DATASET_COL_STR_RANDOM_RENAME,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_TIME_INT.at(PMCAlg(PMCALG_ENUM_RANDOM_RENAME)))));
    ASSERT_EQ(DATASET_COL_STR_RANDOM_RENAME_REVERSE_MS, header[DATASET_COL_TOTAL_NONPMCALG+14]);
    ASSERT_EQ(DATASET_COL_STR_RANDOM_RENAME_REVERSE_MS,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_TIME_INT.at(PMCAlg(PMCALG_ENUM_RANDOM_RENAME_REVERSE_MS)))));
    ASSERT_EQ(DATASET_COL_STR_PARALLEL_OMV, header[DATASET_COL_TOTAL_NONPMCALG+16]);
    ASSERT_EQ(DATASET_COL_STR_PARALLEL_OMV,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_TIME_INT.at(PMCAlg(PMCALG_ENUM_PARALLEL_OMV)))));
    ASSERT_EQ(DATASET_COL_STR_PARALLEL_OMV_REVERSE_MS, header[DATASET_COL_TOTAL_NONPMCALG+18]);
    ASSERT_EQ(DATASET_COL_STR_PARALLEL_OMV_REVERSE_MS,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_TIME_INT.at(PMCAlg(PMCALG_ENUM_PARALLEL_OMV_REVERSE_MS)))));
    ASSERT_EQ(DATASET_COL_STR_PARALLEL_OMV_ASCENDING, header[DATASET_COL_TOTAL_NONPMCALG+20]);
    ASSERT_EQ(DATASET_COL_STR_PARALLEL_OMV_ASCENDING,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_TIME_INT.at(PMCAlg(PMCALG_ENUM_PARALLEL_OMV_ASCENDING)))));
    ASSERT_EQ(DATASET_COL_STR_PARALLEL_OMV_ASCENDING_REVERSE_MS, header[DATASET_COL_TOTAL_NONPMCALG+22]);
    ASSERT_EQ(DATASET_COL_STR_PARALLEL_OMV_ASCENDING_REVERSE_MS,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_TIME_INT.at(PMCAlg(PMCALG_ENUM_PARALLEL_OMV_ASCENDING_REVERSE_MS)))));
    ASSERT_EQ(DATASET_COL_STR_PARALLEL_OMV_DESCENDING, header[DATASET_COL_TOTAL_NONPMCALG+24]);
    ASSERT_EQ(DATASET_COL_STR_PARALLEL_OMV_DESCENDING,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_TIME_INT.at(PMCAlg(PMCALG_ENUM_PARALLEL_OMV_DESCENDING)))));
    ASSERT_EQ(DATASET_COL_STR_PARALLEL_OMV_DESCENDING_REVERSE_MS, header[DATASET_COL_TOTAL_NONPMCALG+26]);
    ASSERT_EQ(DATASET_COL_STR_PARALLEL_OMV_DESCENDING_REVERSE_MS,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_TIME_INT.at(PMCAlg(PMCALG_ENUM_PARALLEL_OMV_DESCENDING_REVERSE_MS)))));
    ASSERT_EQ(DATASET_COL_STR_PARALLEL_OMV_RANDOM_RENAME, header[DATASET_COL_TOTAL_NONPMCALG+28]);
    ASSERT_EQ(DATASET_COL_STR_PARALLEL_OMV_RANDOM_RENAME,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_TIME_INT.at(PMCAlg(PMCALG_ENUM_PARALLEL_OMV_RANDOM_RENAME)))));
    ASSERT_EQ(DATASET_COL_STR_PARALLEL_OMV_RANDOM_RENAME_REVERSE_MS, header[DATASET_COL_TOTAL_NONPMCALG+30]);
    ASSERT_EQ(DATASET_COL_STR_PARALLEL_OMV_RANDOM_RENAME_REVERSE_MS,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_TIME_INT.at(PMCAlg(PMCALG_ENUM_PARALLEL_OMV_RANDOM_RENAME_REVERSE_MS)))));

    ASSERT_EQ(DATASET_COL_STR_ERR_NORMAL, header[DATASET_COL_TOTAL_NONPMCALG+1]);
    ASSERT_EQ(DATASET_COL_STR_ERR_NORMAL,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_ERR_INT.at(PMCAlg(PMCALG_ENUM_NORMAL)))));
    ASSERT_EQ(DATASET_COL_STR_ERR_REVERSE_MS, header[DATASET_COL_TOTAL_NONPMCALG+3]);
    ASSERT_EQ(DATASET_COL_STR_ERR_REVERSE_MS,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_ERR_INT.at(PMCAlg(PMCALG_ENUM_REVERSE_MS)))));
    ASSERT_EQ(DATASET_COL_STR_ERR_ASCENDING, header[DATASET_COL_TOTAL_NONPMCALG+5]);
    ASSERT_EQ(DATASET_COL_STR_ERR_ASCENDING,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_ERR_INT.at(PMCAlg(PMCALG_ENUM_ASCENDING)))));
    ASSERT_EQ(DATASET_COL_STR_ERR_ASCENDING_REVERSE_MS, header[DATASET_COL_TOTAL_NONPMCALG+7]);
    ASSERT_EQ(DATASET_COL_STR_ERR_ASCENDING_REVERSE_MS,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_ERR_INT.at(PMCAlg(PMCALG_ENUM_ASCENDING_REVERSE_MS)))));
    ASSERT_EQ(DATASET_COL_STR_ERR_DESCENDING, header[DATASET_COL_TOTAL_NONPMCALG+9]);
    ASSERT_EQ(DATASET_COL_STR_ERR_DESCENDING,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_ERR_INT.at(PMCAlg(PMCALG_ENUM_DESCENDING)))));
    ASSERT_EQ(DATASET_COL_STR_ERR_DESCENDING_REVERSE_MS, header[DATASET_COL_TOTAL_NONPMCALG+11]);
    ASSERT_EQ(DATASET_COL_STR_ERR_DESCENDING_REVERSE_MS,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_ERR_INT.at(PMCAlg(PMCALG_ENUM_DESCENDING_REVERSE_MS)))));
    ASSERT_EQ(DATASET_COL_STR_ERR_RANDOM_RENAME, header[DATASET_COL_TOTAL_NONPMCALG+13]);
    ASSERT_EQ(DATASET_COL_STR_ERR_RANDOM_RENAME,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_ERR_INT.at(PMCAlg(PMCALG_ENUM_RANDOM_RENAME)))));
    ASSERT_EQ(DATASET_COL_STR_ERR_RANDOM_RENAME_REVERSE_MS, header[DATASET_COL_TOTAL_NONPMCALG+15]);
    ASSERT_EQ(DATASET_COL_STR_ERR_RANDOM_RENAME_REVERSE_MS,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_ERR_INT.at(PMCAlg(PMCALG_ENUM_RANDOM_RENAME_REVERSE_MS)))));
    ASSERT_EQ(DATASET_COL_STR_ERR_PARALLEL_OMV, header[DATASET_COL_TOTAL_NONPMCALG+17]);
    ASSERT_EQ(DATASET_COL_STR_ERR_PARALLEL_OMV,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_ERR_INT.at(PMCAlg(PMCALG_ENUM_PARALLEL_OMV)))));
    ASSERT_EQ(DATASET_COL_STR_ERR_PARALLEL_OMV_REVERSE_MS, header[DATASET_COL_TOTAL_NONPMCALG+19]);
    ASSERT_EQ(DATASET_COL_STR_ERR_PARALLEL_OMV_REVERSE_MS,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_ERR_INT.at(PMCAlg(PMCALG_ENUM_PARALLEL_OMV_REVERSE_MS)))));
    ASSERT_EQ(DATASET_COL_STR_ERR_PARALLEL_OMV_ASCENDING, header[DATASET_COL_TOTAL_NONPMCALG+21]);
    ASSERT_EQ(DATASET_COL_STR_ERR_PARALLEL_OMV_ASCENDING,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_ERR_INT.at(PMCAlg(PMCALG_ENUM_PARALLEL_OMV_ASCENDING)))));
    ASSERT_EQ(DATASET_COL_STR_ERR_PARALLEL_OMV_ASCENDING_REVERSE_MS, header[DATASET_COL_TOTAL_NONPMCALG+23]);
    ASSERT_EQ(DATASET_COL_STR_ERR_PARALLEL_OMV_ASCENDING_REVERSE_MS,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_ERR_INT.at(PMCAlg(PMCALG_ENUM_PARALLEL_OMV_ASCENDING_REVERSE_MS)))));
    ASSERT_EQ(DATASET_COL_STR_ERR_PARALLEL_OMV_DESCENDING, header[DATASET_COL_TOTAL_NONPMCALG+25]);
    ASSERT_EQ(DATASET_COL_STR_ERR_PARALLEL_OMV_DESCENDING,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_ERR_INT.at(PMCAlg(PMCALG_ENUM_PARALLEL_OMV_DESCENDING)))));
    ASSERT_EQ(DATASET_COL_STR_ERR_PARALLEL_OMV_DESCENDING_REVERSE_MS, header[DATASET_COL_TOTAL_NONPMCALG+27]);
    ASSERT_EQ(DATASET_COL_STR_ERR_PARALLEL_OMV_DESCENDING_REVERSE_MS,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_ERR_INT.at(PMCAlg(PMCALG_ENUM_PARALLEL_OMV_DESCENDING_REVERSE_MS)))));
    ASSERT_EQ(DATASET_COL_STR_ERR_PARALLEL_OMV_RANDOM_RENAME, header[DATASET_COL_TOTAL_NONPMCALG+29]);
    ASSERT_EQ(DATASET_COL_STR_ERR_PARALLEL_OMV_RANDOM_RENAME,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_ERR_INT.at(PMCAlg(PMCALG_ENUM_PARALLEL_OMV_RANDOM_RENAME)))));
    ASSERT_EQ(DATASET_COL_STR_ERR_PARALLEL_OMV_RANDOM_RENAME_REVERSE_MS, header[DATASET_COL_TOTAL_NONPMCALG+31]);
    ASSERT_EQ(DATASET_COL_STR_ERR_PARALLEL_OMV_RANDOM_RENAME_REVERSE_MS,
              DATASET_COL_INT_TO_STR_MAP.at(TO_DCI(DATASET_COL_ALG_TO_ERR_INT.at(PMCAlg(PMCALG_ENUM_PARALLEL_OMV_RANDOM_RENAME_REVERSE_MS)))));

    // Done
    CLEANUP_DATASET();
    return true;
}
bool DatasetTester::several_pmc_algorithms_check_times_and_errors_per_alg() const {

    INIT();
    int time_limit = 3;

    // Load large graphs, each with its own time limit and PMC algorithms.
    // Add some small graphs, so we can see some completion.
    TODO: Choose harder graphs! these take 1~5 seconds to compute...
    const vector<string> graph_paths({DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_BN_OBJDET+"deer_rescaled_0034.K20.F1.5.model.uai",
                                      DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_BN_OBJDET+"deer_rescaled_0034.K10.F2.model.uai",
                                      filename[0],
                                      DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_BN_OBJDET+"deer_rescaled_0034.K15.F1.5.model.uai",
                                      filename[1],
                                      DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_BN_OBJDET+"deer_rescaled_1002.K15.F1.75.model.uai",
                                      filename[3],
                                      DATASET_DIR_BASE+DATASET_DIR_DIFFICULT_BN_OBJDET+"deer_rescaled_1002.K20.F1.5.model.uai"});

    // Set diverse PMC algorithms
    vector<set<PMCAlg> > alg_sets({PMCALG_CREATE_SET(PMCALG_ENUM_DESCENDING,PMCALG_ENUM_ASCENDING_REVERSE_MS),
                                   PMCALG_CREATE_SET(PMCALG_ENUM_NORMAL,PMCALG_ENUM_DESCENDING,PMCALG_ENUM_ASCENDING_REVERSE_MS),
                                   PMCALG_CREATE_SET(PMCALG_ENUM_NORMAL,PMCALG_ENUM_DESCENDING,PMCALG_ENUM_ASCENDING_REVERSE_MS,PMCALG_ENUM_PARALLEL_OMV_ASCENDING),
                                   PMCALG_CREATE_SET(PMCALG_ENUM_DESCENDING,PMCALG_ENUM_ASCENDING_REVERSE_MS),
                                   PMCALG_CREATE_SET(PMCALG_ENUM_DESCENDING),
                                   PMCALG_CREATE_SET(PMCALG_ENUM_NORMAL,PMCALG_ENUM_DESCENDING,PMCALG_ENUM_ASCENDING_REVERSE_MS),
                                   PMCALG_CREATE_SET(PMCALG_ENUM_NORMAL,PMCALG_ENUM_PARALLEL_OMV_DESCENDING,PMCALG_ENUM_PARALLEL_OMV_DESCENDING_REVERSE_MS),
                                   PMCALG_CREATE_SET(PMCALG_ENUM_NORMAL,PMCALG_ENUM_DESCENDING,PMCALG_ENUM_ASCENDING_REVERSE_MS),
                                  });
    // Sanity
    ASSERT_EQ(alg_sets.size(), graph_paths.size());

    // Count how many total PMC enumeration runs there will be
    int total_runs = 0;
    for (auto alg_set: alg_sets) {
        total_runs += alg_set.size();
    }

    // Sanity.
    ASSERT_EQ(alg_sets.size(), graph_paths.size());
    for (unsigned i=0; i<graph_paths.size(); ++i) {
        ASSERT(utils__file_exists(graph_paths[i]));
    }

    // Set the expected error state for each algorithm for each graph.
    map<pair<unsigned,PMCAlg>, bool> expected_error;

    // Indicate this should take some time.
    // Two runs (one for error marking, one for testing), running at most time_limit seconds.
    cout << "\b\b\b\b (should take at most ~" << 2*total_runs*time_limit << " seconds) ";

    // Make sure the actual calculation time of each of the hard graphs is actually long.
    // Take note of graphs that are 'easy' and add the indexes to the relevant set.
    for (unsigned i=0; i<graph_paths.size(); ++i) {
        GraphStats gs = GraphStats::read(graph_paths[i]);
        for (PMCAlg alg: alg_sets[i]) {
            cout << endl << "Running PMCE on graph #" << (i+1) << ", algorithm " << alg.str() << "... ";
            PMCEnumerator tmp_pmce = PMCEnumerator(gs.get_graph(), time_limit);
            tmp_pmce.set_algorithm(alg);
            tmp_pmce.get(StatisticRequest().set_single_pmc_alg(alg).set_count_pmc());
            expected_error[pair<unsigned,PMCAlg>(i,alg)] = tmp_pmce.is_out_of_time();
            cout << (tmp_pmce.is_out_of_time() ? "TIME ERROR" : "no time error");
        }
    }

    // Construct a dataset with the graph, set requests.
    // Allow five seconds per graph - should be enough for the small graphs
    Dataset ds = Dataset(dataset_filename, graph_paths);
    ASSERT_EQ(ds.dataset.size(), graph_paths.size());
    for (unsigned i=0; i<graph_paths.size(); ++i) {
        ds.set_request(i, StatisticRequest().add_algs_to_pmc_race(alg_sets[i]).set_count_pmc().set_time_limit_pmc(time_limit));
    }

    // Calculate, read the columns of the output file and make sure the OOT errors exist in the right places
    ASSERT(!utils__file_exists(dataset_filename));
    ASSERT_NO_THROW(ds.calc().dump());
    ASSERT(utils__file_exists(dataset_filename));
    vector<vector<string> > csv_contents = utils__read_csv(dataset_filename);

    // Make sure the total number of rows is the number of graphs minus the header row(s)
    ASSERT_EQ(ds.dataset.size(), csv_contents.size() - DATASET_HEADER_ROWS);

    // Make sure the number of columns is DATASET_COL_TOTAL on each row
    for (auto row: csv_contents) {
        ASSERT_EQ(row.size(), DATASET_COL_TOTAL);
    }

    // Validate column contents
    ASSERT_EQ(csv_contents.size(), 1+graph_paths.size());   // 1 metadata line, and one line for each graph
    for (unsigned i=0; i<graph_paths.size(); ++i) {
        int row_index = i+1;
        for (PMCAlg alg=PMCAlg::first(); alg<PMCAlg::last(); ++alg) {
            //cout << endl << "Validating graph #" << row_index << ", algorithm " << alg.str();
            string err_column_contents = utils__strip_char(csv_contents[row_index][DATASET_COL_ALG_TO_ERR_INT.at(alg)], ' ');
            string time_column_contents = utils__strip_char(csv_contents[row_index][DATASET_COL_ALG_TO_TIME_INT.at(alg)], ' ');
            if (!utils__is_in_set(alg, alg_sets[i])) {
                //cout << ". Should have no data...";
                ASSERT_EQ(time_column_contents, DATASET_COL_CONTENT_DATA_UNAVAILABLE);
                ASSERT_EQ(err_column_contents, DATASET_COL_CONTENT_DATA_UNAVAILABLE);
            }
            else {
                bool time_err;
                //cout << ", " << (expected_error.at(pair<unsigned,PMCAlg>(i,alg)) ? "reached timeout" : "completed successfully");
                ASSERT_NO_THROW(time_err = expected_error.at(pair<unsigned,PMCAlg>(i,alg)));
                cout << endl << "Checking graph #" << row_index << ", algorithm " << alg.str() << "...";
                ASSERT_NEQ(time_column_contents, DATASET_COL_CONTENT_DATA_UNAVAILABLE);
                ASSERT_EQ(err_column_contents, (time_err ? DATASET_COL_CONTENT_TIME_ERR : ""));
            }
        }
    }

    CLEANUP_DATASET();
    return true;
}
bool DatasetTester::generate_count_and_time_errors_test_text_and_gs_fields() const {
    INIT_DATASET();


    CLEANUP_DATASET();
    return true;
}
bool DatasetTester::load_stat_files_same_statreq() const {

    return true;
}
bool DatasetTester::load_stat_files_diverse_statreq() const {

    return true;
}
bool DatasetTester::load_stat_files_diverse_statreq_diverse_valid_cells() const {

    return true;
}
bool DatasetTester::validate_stat_load_before_calc() const {

    return true;
}
bool DatasetTester::validate_stat_load_updates_statreq_same_statreq() const {

    return true;
}
bool DatasetTester::validate_stat_load_updates_statreq_diverse_statreq() const {

    return true;
}
bool DatasetTester::validate_unavailable_stat_update_after_stat_load() const {

    return true;
}
bool DatasetTester::column_sanity() const {

    return true;
}
bool DatasetTester::diverse_column_output() const {

    return true;
}

}
