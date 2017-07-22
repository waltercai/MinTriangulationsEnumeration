#ifndef TESTUTILS_H_INCLUDED
#define TESTUTILS_H_INCLUDED

#include <iostream>
#include "Utils.h"
using std::cout;

/**
 * Per-test macros
 */
#define ASSERT(_testname) do { \
        if (!(_testname)) { \
            TRACE(TRACE_LVL__TEST, "'" << #_testname << "' is FALSE"); \
            return false; \
        } \
    } while(0)

#define ASSERT_EQUAL(_x,_y) do { \
        if (!(_x == _y)) { \
            TRACE(TRACE_LVL__TEST, "'" << #_x << "' isn't equal to '" << #_y << "'!"); \
            return false; \
        } \
    } while(0)

/**
 * Main tester macros
 */
#define START_TESTS() \
    int _passcounter=0, _totalcounter=0; \
    cout << "STARTING TESTS\n";

#define END_TESTS() \
    cout << "DONE. " << _passcounter << "/" << _totalcounter << " passed\n"

#define DO_TEST(_testfunc) do { \
        cout << "Running '" << #_testfunc << "'... "; \
        if (!(_testfunc())) { \
            cout << "'" << #_testfunc << "' FAILED\n"; \
        } \
        else { \
            ++_passcounter; \
            cout << "PASSED\n"; \
        } \
        ++_totalcounter; \
    }while(0)


#endif // TESTUTILS_H_INCLUDED
