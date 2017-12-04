#ifndef TESTINTERFACE_H_INCLUDED
#define TESTINTERFACE_H_INCLUDED

#include "Utils.h"
#include <iostream>
#include <string>
#include <unistd.h>
using std::cout;
using std::endl;
using std::string;

namespace tdenum {
/**
 * USAGE:
 *
 * - Derive from this class.
 * - When implementing, define methods that take no arguments and return
 *   a boolean success / failure value. Use the macro DO_TEST within
 *   the derived class go() method for the macro to work (uses member fields).
 * - See examples (there are at least 3 XXXTester source files).
 * - To run (selected) tests, call start()
 */


/**
 * Per-test macros
 */
#define ASSERT(_val) do { \
        if (!(_val)) { \
            TRACE(TRACE_LVL__ALWAYS, "'" << #_val << "' is FALSE"); \
            return false; \
        } \
    } while(0)

#define ASSERT_OR_ELSE_DELETE_FILE(_val,_filename) do { \
        if (!(_val)) { \
            TRACE(TRACE_LVL__ALWAYS, "'" << #_val << "' is FALSE"); \
            ASSERT(utils__delete_file(_filename)); \
            return false; \
        } \
    } while(0)

/**
 * Equalities
 */
#define ASSERT_ARENT_NEQ_TEXT(_x,_y) _x << " is equal to " << _y << "!"
#define ASSERT_ARENT_EQ_TEXT(_x,_y) _x << " isn't equal to " << _y << "!"

#define ASSERT_NEQ(_x,_y) do { \
        if (_x == _y) { \
            TRACE(TRACE_LVL__ALWAYS, ASSERT_ARENT_NEQ_TEXT(_x,_y)); \
            return false; \
        } \
    } while(0)

#define ASSERT_NEQ_FLOAT(_x,_y) do { \
        if (UTILS__EQ_FLOAT(_x,_y)) { \
            TRACE(TRACE_LVL__ALWAYS, ASSERT_ARENT_NEQ_TEXT(_x,_y)); \
            return false; \
        } \
    } while(0)

#define ASSERT_EQ(_x,_y) do { \
        if (!(_x == _y)) { \
            TRACE(TRACE_LVL__ALWAYS, ASSERT_ARENT_EQ_TEXT(_x,_y)); \
            return false; \
        } \
    } while(0)

#define ASSERT_EQ_FLOAT(_x,_y) do { \
        if (!UTILS__EQ_FLOAT(_x,_y)) { \
            TRACE(TRACE_LVL__ALWAYS, ASSERT_ARENT_EQ_TEXT(_x,_y)); \
            return false; \
        } \
    } while(0)

#define ASSERT_SAME_VEC_ELEMS(_v1,_v2) do { \
        auto tmp1 = _v1; \
        auto tmp2 = _v2; \
        while(!tmp1.empty()) { \
            auto e = tmp1.back(); \
            tmp1.pop_back(); \
            auto iter = find(tmp2.begin(), tmp2.end(), e); \
            if (iter == tmp2.end()) { \
                TRACE(TRACE_LVL__ALWAYS, "Element '" << e << "' is in " << #_v1 << ", but not in " << #_v2); \
                return false; \
            } \
            tmp2.erase(iter); \
        } \
        if (!tmp2.empty()) { \
            TRACE(TRACE_LVL__ALWAYS, "Vector " << #_v1 << " is strictly contained in " << #_v2); \
            return false; \
        } \
    } while(0)

#define ASSERT_DIFF_VEC_ELEMS(_v1,_v2) do { \
        if (utils__same_vector_elements(_v1,_v2)) { \
            TRACE(TRACE_LVL__ALWAYS, "Vectors " << #_v1 << "," << #_v2 << " are equal!" << endl << \
                                     #_v1 << " is: " << _v1 << endl << \
                                     #_v2 << " is: " << _v2); \
            return false; \
        } \
    } while(0)

/**
 * Order
 */
#define ASSERT_LESS(_x,_y) do { \
        if (!((_x) < (_y))) { \
            TRACE(TRACE_LVL__ALWAYS, _x << " is at least " << _y << "!"); \
            return false; \
        } \
    } while(0)

#define ASSERT_GREATER(_x,_y) do { \
        if (!((_x) > (_y))) { \
            TRACE(TRACE_LVL__ALWAYS, _x << " is at most " << _y << "!"); \
            return false; \
        } \
    } while(0)

#define ASSERT_LEQ(_x,_y) do { \
        if (!((_x) <= (_y))) { \
            TRACE(TRACE_LVL__ALWAYS, _x << " is greater than " << _y << "!"); \
            return false; \
        } \
    } while(0)

#define ASSERT_GEQ(_x,_y) do { \
        if (!((_x) >= (_y))) { \
            TRACE(TRACE_LVL__ALWAYS, _x << " is less than " << _y << "!"); \
            return false; \
        } \
    } while(0)

/**
 * String asserts
 */
#define ASSERT_SUBSTR(_sub, _str) do { \
        if (_str.find(_sub) == string::npos) { \
            TRACE(TRACE_LVL__ALWAYS, _sub << " isn't a substring of " << _str); \
            return false; \
        } \
    } while(0)

#define ASSERT_NOT_SUBSTR(_sub, _str) do { \
        if (_str.find(_sub) != string::npos) { \
            TRACE(TRACE_LVL__ALWAYS, _sub << " is a substring of " << _str); \
            return false; \
        } \
    } while(0)

/**
 * Other
 */
#define ASSERT_NO_THROW(_code) do { \
        try { \
            _code; \
        } \
        catch(...) { \
            TRACE(TRACE_LVL__ALWAYS, "Exception thrown on command '" << #_code << "'!"); \
            return false; \
        } \
    } while(0)

/**
 * Main tester macros
 */

// Designed for use with classes implementing TestInterface.
#define DO_TEST(_testfunc) do { \
        cout << utils__timestamp_to_fulldate(time(NULL)) <<  ": Running '" << #_testfunc << "'... "; \
        if (!((bool)(_testfunc()))) { \
            cout << "'" << #_testfunc << "' FAILED\n"; \
            failed = true; \
        } \
        else { \
            ++total_passed; \
            cout << "PASSED\n"; \
        } \
        ++total_tests; \
        utils__sleep_msecs(10); /*Helps IO tests pass(?), less filesystem races*/\
    } while(0)

/**
 * Public interface, to be inherited by all testing classes.
 */
class TestInterface {
protected:

    // The name of this specific tester
    const string tester_name;
    const string tmp_dir_name;  // Temporary directory for test files

    // A boolean failure flag
    bool failed;

    // If the 'failed' flag is valid, set this to true
    bool done;

    // Set this global flag to require the destructor to delete the temporary
    // testing directory
    static bool cleanup;

    // Test pass counter
    int total_passed;
    int total_tests;

    // Basic output
    /*virtual */const TestInterface& start_tester() const;
    /*virtual */const TestInterface& end_tester() const;

    // Actual work. User must define this.
    // Return a reference to the DERIVED class!
    virtual TestInterface& go() = 0;

public:

    // Return a reference to the DERIVED class when implementing!
    virtual TestInterface& set_all() = 0;     // Set all tests active
    virtual TestInterface& clear_all() = 0;   // Unset all tests

    TestInterface(const string& t_name);
    ~TestInterface();

    TestInterface& start();
    bool test_failed() const;
    bool test_passed() const;

    // May be useful in macros
    operator bool();
    operator bool() const;

    // Set or unset the cleanup bit
    static void set_cleanup();
    static void unset_cleanup();
};

}

#endif // TESTINTERFACE_H_INCLUDED
