#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <string.h>
#include <iostream>
#include <vector>
using std::ostream;
using std::vector;

namespace tdenum {

/**
 * Some OS hacks
 */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define SLASH '\\'
#else
#define SLASH '/'
#endif

/**
 * Printing
 */
#define __FILENAME__ (strrchr(__FILE__, SLASH) ? strrchr(__FILE__, SLASH) + 1 : __FILE__)

#define ASSERT_PRINT(_stream) \
    cout << __FILENAME__ << ":" << __FUNCTION__ << ":" << __LINE__ << ": " << _stream << "\n"

// Output vectors, in general
template<typename T>
ostream& operator<<(ostream& os, const vector<T>& v) {
    os << "{";
    for (auto i=v.begin(); i!=v.end(); ++i) {
        os << *i << ",";
    }
    if (v.size() > 0) {
        os << "\b"; // removing trailing space
    }
    os << "}";
    return os;
}

/**
 * For use in live code
 */
typedef enum _TRACE_LVL_CODES {
    TRACE_LVL__NONE,
    TRACE_LVL__ERROR,
    TRACE_LVL__WARNING,
    TRACE_LVL__DEBUG,
    TRACE_LVL__NOISE
} TRACE_LVL__CODES;
#define TRACE_LVL TRACE_LVL__WARNING
#define TRACE(_lvl, _stream) do { \
        if (_lvl <= TRACE_LVL) { \
            ASSERT_PRINT(_stream); \
        } \
    } while(0)


}

#endif // UTILS_H_INCLUDED
