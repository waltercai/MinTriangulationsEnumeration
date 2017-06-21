#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <string.h>
#include <iostream>
#include <vector>
#include <set>
#include <fstream>
#include <sstream>
#include <iomanip>
using std::ostream;
using std::vector;
using std::set;
using std::string;
using std::ofstream;
using std::ostringstream;
using std::ios;
using std::setw;
using std::setfill;

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
 * Printing and logging.
 *
 * To fork-output all ASSERT_PRINTS to file, call Logger::start(filename).
 */
#define __FILENAME__ (strrchr(__FILE__, SLASH) ? strrchr(__FILE__, SLASH) + 1 : __FILE__)

#define ASSERT_PRINT(_stream) do { \
        ostringstream _oss; \
        _oss << __FILENAME__ << ":" << __FUNCTION__ << ":" << __LINE__ << ": " << _stream << "\n"; \
        cout << _oss.str(); \
        Logger::out(_oss); \
    } while(0)

class Logger {
private:
    static string filename;
    static ofstream file;
    static bool state;
public:
    static void stop();
    // Add an option to append to file, or truncate it.
    static void start(const string& f, bool append = true);
    static void out(const ostringstream& os);
};

#define SET_LOGGING(_filename) do { \
        if (_LOG_FLAG) { \
            _LOG_FILE.close(); \
        } \
        _LOG_FLAG = true; \
        _LOG_FILE_NAME = _filename; \


// Convert time in seconds to hh:mm:ss
string secs_to_hhmmss(time_t t);

/**
 * Useful container wrappers
 */
// Remove an element from a vector (by value).
#define REMOVE_FROM_VECTOR(_v, _elmt) \
    _v.erase(std::remove(_v.begin(), _v.end(), _elmt), _v.end())


// Output vectors / sets, in general
#define AUTOPRINT_CONTAINER(_container_type) \
template<typename V> \
ostream& operator<<(ostream& os, const _container_type<V>& v) { \
    os << "{"; \
    for (auto i=v.begin(); i!=v.end(); ++i) { \
        os << *i << ","; \
    } \
    if (v.size() > 0) { \
        os << "\b"; /*removing trailing space*/ \
    } \
    os << "}"; \
    return os; \
}
AUTOPRINT_CONTAINER(set)
AUTOPRINT_CONTAINER(vector)


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
