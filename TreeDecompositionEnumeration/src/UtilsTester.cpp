#include "UtilsTester.h"
#include <algorithm>
using std::min;

namespace tdenum {

bool UtilsTester::epsilon_float_cmp() const {
    ASSERT_EQ_FLOAT(EPSILON/5, 0);  // Should be very small
    ASSERT_EQ_FLOAT(0.5,0.5);
    ASSERT_EQ_FLOAT(0.9,0.9);
    ASSERT_NEQ_FLOAT(0.8,0.9);
    ASSERT_NEQ_FLOAT(0.89,0.9);
    return true;
}
bool UtilsTester::io() const {
    // General
    ASSERT(utils__dir_exists(tmp_dir_name));
    ASSERT(utils__dir_is_empty(tmp_dir_name));
    // Strings
    string file = tmp_dir_name+"/io_output";
    ASSERT(utils__dump_string_to_file("yo",file));
    ASSERT_EQ(utils__read_file_as_string(file), "yo");
    // Directories
    string dir = tmp_dir_name+"/io_dir";
    // Empty:
    ASSERT(utils__mkdir(dir));
    ASSERT(utils__dir_is_empty(dir));
    ASSERT(utils__dir_exists(dir));
    ASSERT(utils__delete_all_files_in_dir(dir));    // Should be true, the dir is empty
    ASSERT(utils__delete_dir(dir));
    ASSERT(utils__dir_is_empty(dir));               // Required behavior
    ASSERT(!utils__dir_exists(dir));
    // Full:
    file = dir+"/io_inner_file";
    ASSERT(utils__mkdir(dir));
    ASSERT(utils__dump_string_to_file("yo",file));
    ASSERT(!utils__dir_is_empty(dir));
    ASSERT(utils__dir_exists(dir));
    ASSERT(utils__delete_all_files_in_dir(dir));
    ASSERT(utils__dir_is_empty(dir));
    ASSERT(utils__dir_exists(dir));
    ASSERT(utils__delete_dir(dir));
    ASSERT(utils__dir_is_empty(dir));
    ASSERT(!utils__dir_exists(dir));

    return true;
}
bool recursive_depth_io_mkdirs_aux(const string& path, int depth_remaining) {
    if (depth_remaining <= 0) {
        return true;
    }
    ASSERT(utils__mkdir(path+"/left"));
    ASSERT(utils__mkdir(path+"/right"));
    if (!recursive_depth_io_mkdirs_aux(path+"/left", depth_remaining-1) ||
        !recursive_depth_io_mkdirs_aux(path+"/right", depth_remaining-1)) {
        return false;
    }
    return true;
}
bool UtilsTester::recursive_depth_io() const {
    unsigned test_depth = 3;
    // Create lots of dirs (binary tree of depth 5, or less if the depth is less)
    // and make sure they are successfully deleted
    string tmpdir = tmp_dir_name+"/recursive_depth_io";
    ASSERT(UTILS__MAX_RECURSION_DEPTH >= test_depth);
    ASSERT(utils__mkdir(tmpdir));
    ASSERT(recursive_depth_io_mkdirs_aux(tmpdir, test_depth));
    ASSERT(utils__dir_exists(tmpdir));
    ASSERT(utils__dir_exists(tmpdir+"/left/right/left"));
    ASSERT(utils__delete_dir(tmpdir));
    ASSERT(!utils__dir_exists(tmpdir));
    ASSERT(utils__mkdir(tmpdir));
    ASSERT(utils__dir_exists(tmpdir));
    ASSERT(recursive_depth_io_mkdirs_aux(tmpdir, test_depth));
    ASSERT(utils__dir_exists(tmpdir));
    ASSERT(utils__dir_exists(tmpdir+"/left/right/left"));
    ASSERT(utils__delete_all_files_in_dir(tmpdir));
    ASSERT(utils__dir_exists(tmpdir));
    ASSERT(utils__dir_is_empty(tmpdir));
    ASSERT(utils__delete_dir(tmpdir));
    ASSERT(!utils__dir_exists(tmpdir));

    // Make sure we can't delete more than the depth defined
    unsigned original_max_depth = UTILS__MAX_RECURSION_DEPTH;
    UTILS__MAX_RECURSION_DEPTH = test_depth-1;
    ASSERT(utils__mkdir(tmpdir));
    ASSERT(utils__dir_exists(tmpdir));
    ASSERT(recursive_depth_io_mkdirs_aux(tmpdir, test_depth));
    ASSERT(utils__dir_exists(tmpdir));
    ASSERT(utils__dir_exists(tmpdir+"/left/right/left"));
    TRACE_LVL_OVERRIDE(TRACE_LVL__ALWAYS);
    ASSERT(!utils__delete_dir(tmpdir));
    TRACE_LVL_RESTORE();
    ASSERT(utils__dir_exists(tmpdir));
    ASSERT(utils__dir_exists(tmpdir+"/left/right/left"));
    TRACE_LVL_OVERRIDE(TRACE_LVL__ALWAYS);
    ASSERT(!utils__delete_all_files_in_dir(tmpdir));
    TRACE_LVL_RESTORE();
    ASSERT(utils__dir_exists(tmpdir));
    ASSERT(utils__dir_exists(tmpdir+"/left/right/left"));
    UTILS__MAX_RECURSION_DEPTH = original_max_depth;
    ASSERT(utils__delete_all_files_in_dir(tmpdir)); // Sometimes fails, don't know why
    ASSERT(utils__dir_exists(tmpdir));
    ASSERT(utils__dir_is_empty(tmpdir));

    // Cleanup
    ASSERT(utils__delete_dir(tmpdir));
    ASSERT(!utils__dir_exists(tmpdir));

    return true;
}
bool UtilsTester::strip() const {
    ASSERT_EQ("",utils__strip_char("",'a'));
    ASSERT_EQ("",utils__strip_char("a",'a'));
    ASSERT_EQ("",utils__strip_char("aa",'a'));
    ASSERT_EQ("a",utils__strip_char("aaa",'a'));
    ASSERT_EQ("abb",utils__strip_char("aabb",'a'));
    ASSERT_EQ("bba",utils__strip_char("bbaa",'a'));
    ASSERT_EQ("bb",utils__strip_char("bb",'a'));
    return true;
}
bool UtilsTester::str_tokenize() const {
    string str1 = "A normal sentence, separated by spaces";
    vector<string> exp1 = {"A","normal","sentence,","separated","by","spaces"};
    string str2 = "This time, separate by_several_-types---_ _of delimiters";
    vector<string> exp2 = {"This","time,","separate","by","several","","types","","","","","","of","delimiters"};
    ASSERT_EQ(exp1,utils__tokenize(str1,{' '}));
    ASSERT_EQ(exp2,utils__tokenize(str2,{' ','_','-'}));
    ASSERT_EQ(exp2,utils__tokenize(str2,{' ',' ','_',' ','-'})); // Give duplicates
    ASSERT_EQ(exp2,utils__tokenize(str2,{'/',' ','_','-'}));     // Nonexistent chars
    ASSERT_EQ(exp2,utils__tokenize(str2,{' ','/','_','-'}));
    ASSERT_EQ(exp2,utils__tokenize(str2,{'/',' ','_','/','-',' '}));
    return true;
}
bool UtilsTester::join() const {

    ASSERT_EQ(utils__join(vector<string>(), ' '), "");
    ASSERT_EQ(utils__join(vector<string>({"a"}), ' '), "a");
    ASSERT_EQ(utils__join(vector<string>({"a","b"}), ' '), "a b");
    ASSERT_EQ(utils__join(vector<string>({"a","b"}), ','), "a,b");
    ASSERT_EQ(utils__join(vector<string>({"0.3 ","  b "}), ';'), "0.3 ;  b ");

    return true;
}
bool UtilsTester::get_all_lines() const {
    // Setup
    string file_oneline = tmp_dir_name+"/get_lines_oneline";
    string file_twolines = tmp_dir_name+"/get_lines_twolines";
    string file_mix = tmp_dir_name+"/get_lines_mix";
    ostringstream oss;
    oss << "heyoo";
    string str_oneline = oss.str();
    oss.clear();
    oss << "hey" << "\n" << "yoo";
    string str_twolines = oss.str();
    oss.clear();
    oss << "h" << endl
        << "e\n"
        << "y\r\n"
        << "o\n\r"
        << "o";
    string str_mix = oss.str();

    // Asserts
    ASSERT(utils__dump_string_to_file(str_oneline,file_oneline));
    ASSERT_EQ(str_oneline,utils__read_file_as_string(file_oneline));
    ASSERT(utils__dump_string_to_file(str_twolines,file_twolines));
    ASSERT_EQ(str_twolines,utils__read_file_as_string(file_twolines));
    ASSERT(utils__dump_string_to_file(str_mix,file_mix));
    ASSERT_EQ(str_mix,utils__read_file_as_string(file_mix));
    return true;
}
bool UtilsTester::str_to_file() const {
    string file = tmp_dir_name+"/str_to_file";
    string str = "asdjejrkgnf\n\n\aegfersalf\nsfkldjgdf32b5 y6 s4 t_\ts43t@#$%^&* dsfg";
    ASSERT(utils__dump_string_to_file(str,file));
    ASSERT_EQ(str,utils__read_file_as_string(file));
    return true;
}
bool UtilsTester::csv_io() const {
    // Basic
    string tmp_filename_base = tmp_dir_name+"/csv_io";
    string csv_str = "a,b\nc,d";
    vector<vector<string> > csv_vec = {{"a","b"},{"c","d"}};
    ASSERT(utils__dump_string_to_file(csv_str,tmp_filename_base+"0"));
    ASSERT_EQ(utils__read_csv(tmp_filename_base+"0"), csv_vec);
    ASSERT(utils__dump_csv(csv_vec,tmp_filename_base+"1"));
    ASSERT_EQ(utils__read_csv(tmp_filename_base+"1"), csv_vec);
    ASSERT_EQ(utils__read_file_as_string(tmp_filename_base+"1"), csv_str);
    // Advanced
    csv_str = "a,b,c\nd\n\nasdf__sdf+=\n\nxy,,,sdfg";
    csv_vec = {{"a","b","c"},{"d"},{""},{"asdf__sdf+="},{""},{"xy","","","sdfg"}};
    ASSERT(utils__dump_string_to_file(csv_str,tmp_filename_base+"0"));
    ASSERT_EQ(utils__read_csv(tmp_filename_base+"0"), csv_vec);
    ASSERT(utils__dump_csv(csv_vec,tmp_filename_base+"1"));
    ASSERT_EQ(utils__read_csv(tmp_filename_base+"1"), csv_vec);
    ASSERT_EQ(utils__read_file_as_string(tmp_filename_base+"1"), csv_str);

    return true;
}
bool UtilsTester::logger_io() const {
    const string file = tmp_dir_name+"/logger_io_output";
    ASSERT_NO_THROW(Logger::stop());
    ASSERT_NO_THROW(Logger::start(file));
    ASSERT(Logger::out("yo"));
    ASSERT_NO_THROW(Logger::stop());
    ASSERT(!Logger::out("mama"));
    ASSERT_EQ(utils__read_file_as_string(file), "yo");
    ASSERT_NO_THROW(Logger::start(file));
    ASSERT(Logger::out("mama"));
    ASSERT_EQ(utils__read_file_as_string(file), "yomama");
    ASSERT_NO_THROW(Logger::start(file, false));
    ASSERT(Logger::out("whoops"));
    ASSERT_EQ(utils__read_file_as_string(file), "whoops");
    ASSERT_NO_THROW(Logger::stop());    // Otherwise, TRACE()ing will raise errors - the dir will be deleted

    return true;
}
bool UtilsTester::str_length() const {
    // Normal
    string str0 = "sdger\nsdfg";
    ASSERT_EQ(utils__strlen(str0),10);

    // Null terminated?
    string str1 = "asdf\0dafg";
    ASSERT_EQ(utils__strlen(str1),4);

    // Differences in types
    const char* c_str2 = "asdjejrkgnf\n\n\aegfersalf\nsfkldjgdf32b5 y6 s4 t_\ts43t@#$%^&* dsfg";
    string str2 = string(c_str2);
    ASSERT_EQ(utils__strlen(c_str2), utils__strlen(str2));
    return true;
}
bool UtilsTester::str_eq() const {
    // Pretty dumb
    ASSERT(utils__str_eq("Asdf","Asdf"));
    ASSERT(!utils__str_eq("asdf","Asdf"));
    return true;
}
bool UtilsTester::str_empty() const {

    ASSERT(utils__str_empty(""));
    ASSERT(utils__str_empty(string("")));
    ASSERT(utils__str_empty("\0"));
    ASSERT(utils__str_empty(string("\0")));
    ASSERT(!utils__str_empty(" "));
    ASSERT(!utils__str_empty("\t"));
    ASSERT(!utils__str_empty("\n"));

    return true;
}
bool UtilsTester::to_str() const {
    ASSERT_EQ(UTILS__TO_STRING(3),"3");
    ASSERT_EQ(UTILS__TO_STRING((int)3),"3");
    ASSERT_EQ(UTILS__TO_STRING((unsigned)3),"3");
    ASSERT_EQ(UTILS__TO_STRING(3.010),"3.01");
    ASSERT_EQ(UTILS__TO_STRING("3asdf"),"3asdf");
    ASSERT_EQ(UTILS__TO_STRING(string("3asdf")),"3asdf");
    return true;
}
bool UtilsTester::substr_functions() const {

    // Substring existence
    TRACE_LVL_OVERRIDE(TRACE_LVL__ALWAYS);
    ASSERT_NO_THROW(utils__has_substr("","",1));
    ASSERT(!utils__has_substr("","",1));
    TRACE_LVL_RESTORE();
    ASSERT_NO_THROW(ASSERT(utils__has_substr("","")));
    ASSERT_NO_THROW(ASSERT(!utils__has_substr(""," ")));
    ASSERT_NO_THROW(ASSERT(utils__has_substr(" "," ")));
    ASSERT_NO_THROW(ASSERT(utils__has_substr(" ","")));
    ASSERT(utils__has_substr("abcabc","abc"));
    ASSERT(utils__has_substr("abcabc","abc",1));
    ASSERT(utils__has_substr("abcabc","abc",2));
    ASSERT(utils__has_substr("abcabc","abc",3));
    ASSERT(!utils__has_substr("abcabc","abc",4));

    // Replacement
    ASSERT_NO_THROW(utils__replace_substr_with_substr("","",""));
    ASSERT_NO_THROW(utils__replace_substr_with_substr("","a","aa"));    // Substring longer than string
    ASSERT_EQ("b",utils__replace_substr_with_substr("a","a","b"));
    ASSERT_EQ("a",utils__replace_substr_with_substr("a","b","b"));
    ASSERT_EQ("a",utils__replace_substr_with_substr("a","b","c"));
    ASSERT_EQ("bb_b",utils__replace_substr_with_substr("aa_a","a","b"));
    ASSERT_EQ("bb_b",utils__replace_substr_with_substr("ba_a","a","b"));
    ASSERT_EQ("bb_b",utils__replace_substr_with_substr("ab_a","a","b"));
    ASSERT_EQ("b_a",utils__replace_substr_with_substr("aa_a","aa","b"));
    ASSERT_EQ("bbbb_a",utils__replace_substr_with_substr("aa_a","aa","bbbb"));

    return true;
}
bool UtilsTester::time_functions() const {

    // Secs to hhmmss
    TRACE_LVL_OVERRIDE(TRACE_LVL__ALWAYS);
    ASSERT_NO_THROW(utils__timestamp_to_hhmmss(100*3600));
    ASSERT_NO_THROW(utils__timestamp_to_hhmmss(-1));
    TRACE_LVL_RESTORE();
    ASSERT_EQ("00:00:00", utils__timestamp_to_hhmmss(0));
    ASSERT_EQ("00:00:01", utils__timestamp_to_hhmmss(1));
    ASSERT_EQ("00:01:00", utils__timestamp_to_hhmmss(60));
    ASSERT_EQ("00:01:01", utils__timestamp_to_hhmmss(61));
    ASSERT_EQ("02:00:00", utils__timestamp_to_hhmmss(2*3600));
    ASSERT_EQ("02:01:01", utils__timestamp_to_hhmmss(2*3600+60+1));
    ASSERT_EQ("99:59:59", utils__timestamp_to_hhmmss(99*3600+59*60+59));

    // hhmmss to timestamp
    TRACE_LVL_OVERRIDE(TRACE_LVL__ALWAYS);
    ASSERT_NO_THROW(utils__hhmmss_to_timestamp("adher4tfa"));
    ASSERT_NO_THROW(utils__hhmmss_to_timestamp(""));
    ASSERT_NO_THROW(utils__hhmmss_to_timestamp("99:99:99"));
    ASSERT_NO_THROW(utils__hhmmss_to_timestamp("01:-1:20"));
    TRACE_LVL_RESTORE();
    ASSERT_EQ(utils__hhmmss_to_timestamp("00:00:00"), 0);
    ASSERT_EQ(utils__hhmmss_to_timestamp("00:00:01"), (1));
    ASSERT_EQ(utils__hhmmss_to_timestamp("00:01:00"), (60));
    ASSERT_EQ(utils__hhmmss_to_timestamp("00:01:01"), (61));
    ASSERT_EQ(utils__hhmmss_to_timestamp("02:00:00"), (2*3600));
    ASSERT_EQ(utils__hhmmss_to_timestamp("02:01:01"), (2*3600+60+1));
    ASSERT_EQ(utils__hhmmss_to_timestamp("99:59:59"), (99*3600+59*60+59));

    // Just sanity for these
    ASSERT_NO_THROW(utils__timestamp_to_fulldate(0));
    ASSERT_NO_THROW(utils__timestamp_to_fulldate(1));
    ASSERT_NO_THROW(utils__timestamp_to_fulldate(60));
    ASSERT_NO_THROW(utils__timestamp_to_fulldate(61));
    ASSERT_NO_THROW(utils__timestamp_to_fulldate(2*3600));
    ASSERT_NO_THROW(utils__timestamp_to_fulldate(2*3600+60+1));
    ASSERT_NO_THROW(utils__timestamp_to_fulldate(99*3600+59*60+59));

    return true;
}
bool UtilsTester::in_cont_in_set() const {
    // Setup
    set<int> s1={0,3,4,6,2};    // Will be sorted
    vector<int> v1={43345,4563,6537,4,5,52,34,521347,324,657,568,0};
    std::sort(v1.begin(), v1.end());

    // Vector
    ASSERT(UTILS__IS_IN_SORTED_CONTAINER(43345,v1));
    ASSERT(UTILS__IS_IN_SORTED_CONTAINER(4563,v1));
    ASSERT(UTILS__IS_IN_SORTED_CONTAINER(6537,v1));
    ASSERT(UTILS__IS_IN_SORTED_CONTAINER(4,v1));
    ASSERT(UTILS__IS_IN_SORTED_CONTAINER(5,v1));
    ASSERT(UTILS__IS_IN_SORTED_CONTAINER(52,v1));
    ASSERT(UTILS__IS_IN_SORTED_CONTAINER(34,v1));
    ASSERT(UTILS__IS_IN_SORTED_CONTAINER(521347,v1));
    ASSERT(UTILS__IS_IN_SORTED_CONTAINER(324,v1));
    ASSERT(UTILS__IS_IN_SORTED_CONTAINER(657,v1));
    ASSERT(UTILS__IS_IN_SORTED_CONTAINER(568,v1));
    ASSERT(UTILS__IS_IN_SORTED_CONTAINER(0,v1));
    ASSERT(!UTILS__IS_IN_SORTED_CONTAINER(1,v1));
    ASSERT(!UTILS__IS_IN_SORTED_CONTAINER(-1,v1));
    ASSERT(!UTILS__IS_IN_SORTED_CONTAINER(10,v1));
    ASSERT(!UTILS__IS_IN_SORTED_CONTAINER(15345,v1));

    // Set
    ASSERT(UTILS__IS_IN_SORTED_CONTAINER(0,s1));
    ASSERT(UTILS__IS_IN_SORTED_CONTAINER(3,s1));
    ASSERT(UTILS__IS_IN_SORTED_CONTAINER(4,s1));
    ASSERT(UTILS__IS_IN_SORTED_CONTAINER(6,s1));
    ASSERT(UTILS__IS_IN_SORTED_CONTAINER(2,s1));
    ASSERT(!UTILS__IS_IN_SORTED_CONTAINER(1,s1));

    return true;
}
bool UtilsTester::same_vector_elements() const {

    // Sanity
    ASSERT(utils__same_vector_elements(vector<char>(),vector<char>()));
    ASSERT(!utils__same_vector_elements({1},vector<int>()));
    ASSERT(!utils__same_vector_elements(vector<int>(),{1}));
    ASSERT(utils__same_vector_elements(vector<int>({1}),{1}));

    // Inclusion
    ASSERT(!utils__same_vector_elements(vector<int>({1,5,3,4}),vector<int>({1,5,3,4,6})));
    ASSERT(!utils__same_vector_elements(vector<int>({1,5,3,4}),vector<int>({1,5,6,3,4})));
    ASSERT(!utils__same_vector_elements(vector<int>({1,5,3,4}),vector<int>({6,1,5,3,4})));
    ASSERT(!utils__same_vector_elements(vector<int>({1,5,3,4,6}),vector<int>({1,5,3,4})));
    ASSERT(!utils__same_vector_elements(vector<int>({1,5,6,3,4}),vector<int>({1,5,3,4})));
    ASSERT(!utils__same_vector_elements(vector<int>({6,1,5,3,4}),vector<int>({1,5,3,4})));

    // Duplicate values
    ASSERT(!utils__same_vector_elements(vector<int>({1,1}),vector<int>({1})));
    ASSERT(!utils__same_vector_elements(vector<int>({1,1}),vector<int>({1,1,1})));
    ASSERT(utils__same_vector_elements(vector<int>({1,1,1}),vector<int>({1,1,1})));
    ASSERT(!utils__same_vector_elements(vector<int>({1,1,2,2}),vector<int>({1,1,1,2,2})));
    ASSERT(utils__same_vector_elements(vector<int>({1,1,2,2}),vector<int>({1,2,1,2})));
    ASSERT(utils__same_vector_elements(vector<int>({1,1,2,2}),vector<int>({2,1,2,1})));
    ASSERT(utils__same_vector_elements(vector<int>({1,2,1,2}),vector<int>({2,2,1,1})));
    ASSERT(utils__same_vector_elements(vector<int>({2,1,2,1}),vector<int>({2,2,1,1})));

    // Simple vector
    vector<int> v1 = {2,3,4,6};
    vector<int> v2 = v1;
    for (int i=0; i<4*3*2; ++i) {
        ASSERT(utils__same_vector_elements(v1,v2));
        next_permutation(v1.begin(), v1.end());
    }

    return true;
}
bool UtilsTester::remove_vec() const {
    vector<int> v1={4,8,7,2,6,4,3,0,1};
    vector<int> v2={4,8,4,8,4,4,8,8,1};

    ASSERT_NO_THROW(utils__remove_from_vector(3,v1));
    ASSERT_EQ(v1, vector<int>({4,8,7,2,6,4,0,1}));
    ASSERT_NO_THROW(utils__remove_from_vector(4,v1));
    ASSERT_EQ(v1, vector<int>({8,7,2,6,0,1}));          // Remove all occurrences
    ASSERT_NO_THROW(utils__remove_from_vector(3,v1));
    ASSERT_EQ(v1, vector<int>({8,7,2,6,0,1}));
    ASSERT_NO_THROW(utils__remove_from_vector(4,v1));   // Just do nothing
    ASSERT_NO_THROW(utils__remove_from_vector(4,v2));
    ASSERT_EQ(v2, vector<int>({8,8,8,8,1}));
    ASSERT_NO_THROW(utils__remove_from_vector(4,v2));
    ASSERT_EQ(v2, vector<int>({8,8,8,8,1}));
    ASSERT_NO_THROW(utils__remove_from_vector(8,v2));
    ASSERT_EQ(v2, vector<int>({1}));
    ASSERT_NO_THROW(utils__remove_from_vector(1,v2));
    ASSERT_EQ(v2, vector<int>());
    ASSERT_NO_THROW(utils__remove_from_vector(8,v2));
    ASSERT_EQ(v2, vector<int>());

    return true;
}
bool UtilsTester::push_back_unique() const {

    vector<int> v = {0,4,5,2,3,1};
    ASSERT_NO_THROW(utils__push_back_unique(0,v));
    ASSERT_EQ(v,vector<int>({0,4,5,2,3,1}));
    ASSERT_NO_THROW(utils__push_back_unique(-1,v));
    ASSERT_EQ(v,vector<int>({0,4,5,2,3,1,-1}));
    ASSERT_NO_THROW(utils__push_back_unique(-1,v));
    ASSERT_EQ(v,vector<int>({0,4,5,2,3,1,-1}));
    ASSERT_NO_THROW(utils__push_back_unique(3,v));
    ASSERT_EQ(v,vector<int>({0,4,5,2,3,1,-1}));

    return true;
}
bool UtilsTester::vec_range() const {

    ASSERT_EQ(utils__vector_range(3,6),vector<int>({3,4,5,6}));
    ASSERT_EQ(utils__vector_range(-1,6),vector<int>({-1,0,1,2,3,4,5,6}));
    ASSERT_EQ(utils__vector_range('a','d'),vector<char>({'a','b','c','d'}));

    return true;
}
bool UtilsTester::set_union_intersection() const {

    vector<int> out;

    // Use with vectors and sets.
    // Vectors must be pre-sorted.
    vector<int> v1={0,2,4}, backup_v1=v1;
    vector<int> v2={1,2,5}, backup_v2=v2;
    vector<int> v3={0,3,6}, backup_v3=v3;

    // Compound assert
    #define ASSERT_SET_OP_RESULT(_vA,_vB,_is_union,_result) \
        if (_is_union) { \
            ASSERT_NO_THROW(UTILS__VECTOR_UNION(_vA, _vB, out)); \
        } \
        else { \
            ASSERT_NO_THROW(UTILS__VECTOR_INTERSECTION(_vA, _vB, out)); \
        } \
        ASSERT_EQ(_vA, backup_##_vA); \
        ASSERT_EQ(_vB, backup_##_vB); \
        ASSERT_EQ(out, _result)

    // Sanity
    ASSERT_SET_OP_RESULT(v1,v1,true,v1);
    ASSERT_SET_OP_RESULT(v1,v1,false,v1);
    ASSERT_NO_THROW(UTILS__VECTOR_INTERSECTION(vector<int>(),v1,out));
    ASSERT(out.empty());
    ASSERT_EQ(v1,backup_v1);

    ASSERT_SET_OP_RESULT(v1,v2,true,vector<int>({0,1,2,4,5}));
    ASSERT_SET_OP_RESULT(v1,v3,true,vector<int>({0,2,3,4,6}));
    ASSERT_SET_OP_RESULT(v2,v3,true,vector<int>({0,1,2,3,5,6}));

    ASSERT_SET_OP_RESULT(v1,v2,false,vector<int>({2}));
    ASSERT_SET_OP_RESULT(v1,v3,false,vector<int>({0}));
    ASSERT_SET_OP_RESULT(v2,v3,false,vector<int>());

    // Non trivial intersection
    vector<int> v4={0,2,3,5}, backup_v4=v4;
    vector<int> v5={0,3,5,6}, backup_v5=v5;
    vector<int> v6={-1,0,2,3,4,5,6}, backup_v6=v6;

    ASSERT_SET_OP_RESULT(v4,v5,false,vector<int>({0,3,5}));
    ASSERT_SET_OP_RESULT(v4,v6,false,v4);
    ASSERT_SET_OP_RESULT(v5,v6,false,v5);
    ASSERT_SET_OP_RESULT(v4,v5,true,vector<int>({0,2,3,5,6}));
    ASSERT_SET_OP_RESULT(v4,v6,true,v6);
    ASSERT_SET_OP_RESULT(v5,v6,true,v6);

    return true;
}
bool UtilsTester::path_string_decomposition() const {

    ASSERT_EQ(utils__to_forward_slashes("a/b\\c/d\\e\\"),"a/b/c/d/e/");

    // Compound assert
    #define ASSERT_PATH_STRINGS(_src, _dir,_base,_ext) \
        ASSERT_NO_THROW(utils__get_dirname(_src)); \
        ASSERT_NO_THROW(utils__get_filename(_src)); \
        ASSERT_NO_THROW(utils__get_extension(_src)); \
        ASSERT_NO_THROW(utils__merge_dir_basename(_dir,_base)) ;\
        ASSERT_EQ(utils__get_dirname(_src),_dir); \
        ASSERT_EQ(utils__get_filename(_src),_base); \
        ASSERT_EQ(utils__get_extension(_src),_ext); \
        ASSERT_EQ(utils__merge_dir_basename(utils__to_forward_slashes(_dir),_base),utils__to_forward_slashes(_src))

    ASSERT_PATH_STRINGS("abc/defg.hij","abc","defg.hij","hij");
    ASSERT_PATH_STRINGS("abc/defg","abc","defg","");
    ASSERT_PATH_STRINGS("abc/.","abc",".","");
    ASSERT_PATH_STRINGS("./defg",".","defg","");
    ASSERT_PATH_STRINGS("./.",".",".","");
    ASSERT_PATH_STRINGS("abc/defg.hij/","abc/defg.hij","","");
    ASSERT_PATH_STRINGS("abc/defg/","abc/defg","","");
    ASSERT_PATH_STRINGS("abc/./","abc/.","","");
    ASSERT_PATH_STRINGS("./defg/","./defg","","");
    ASSERT_PATH_STRINGS("././","./.","","");
    ASSERT_PATH_STRINGS("a/b\\c/d\\e/","a/b\\c/d\\e","","");
    ASSERT_PATH_STRINGS("a/b\\c/d\\e\\","a/b\\c/d\\e","","");

    return true;
}

}
