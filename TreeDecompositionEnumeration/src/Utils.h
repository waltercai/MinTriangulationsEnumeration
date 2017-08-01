#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <string.h>
#include <string>
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
using std::cout;

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
 * Logger class.
 *
 * Chops wood in Canada
 */
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

#define UTILS__SET_LOGGING(_filename) do { \
        if (_LOG_FLAG) { \
            _LOG_FILE.close(); \
        } \
        _LOG_FLAG = true; \
        _LOG_FILE_NAME = _filename; \

/**
 * Synchronization tools
 */

// Insert the item into the container, where the container
// itself may be a shared OMP resource.
template<typename T, typename V>
void insert_critical(const V& val, T& container) {
    #pragma omp critical
    {
        container.insert(val);
    }
}

/**
 * String manipulation, printing and logging.
 *
 * To fork-output all ASSERT_PRINTS to file, call Logger::start(filename).
 */

// strlen() and std::to_string cause problems
unsigned utils_strlen(const string&);
unsigned utils_strlen(const char*);

// TO_STRING expects a stream, or something that can be output via a stream
#define UTILS__TO_STRING(_x) static_cast< std::ostringstream & >( \
        ( std::ostringstream()/* << std::dec*/ << _x ) ).str()

#define UTILS__FILENAME (strrchr(__FILE__, SLASH) ? strrchr(__FILE__, SLASH) + 1 : __FILE__)

#define UTILS__ASSERT_PRINT(_stream) do { \
        ostringstream _oss; \
        _oss << UTILS__FILENAME << ":" << __FUNCTION__ << ":" << __LINE__ << ": " << _stream << "\n"; \
        cout << _oss.str(); \
        Logger::out(_oss); \
    } while(0)

#define UTILS__PRINT_IF(_bool, _stream) do { \
        if (_bool) { \
            cout << _stream; \
        } \
    } while(0)


// Returns the input string (assumes no newline in the string), and
// if the function is called with the same output ID (can be any
// integer) prepends \b characters to erase the previous string.
// If called with no arguments, outputs a string that erases the
// previous output, returns the cursor to it's original location
// and resets the internal ID.
// Useful for printing progress.
#define UTILS__REPLACE_STRING_INVALID_ID (-1)
string utils_replace_string(const string&, int id);
string utils_replace_string();
#define UTILS__REPLACE_STREAM(_stream, _id) \
        utils_replace_string(UTILS__TO_STRING(_stream), _id)

// Convert time in seconds to hh:mm:ss
string secs_to_hhmmss(time_t t);

// Convert time stamp (in seconds) to readable time
string timestamp_to_hhmmss(time_t t);
string timestamp_to_fulldate(time_t t);

// Writes / appends a string of output to a file (doesn't append, by default)
void dump_string_to_file(const string& filename, const string& str, bool append = false);

/**
 * Useful container wrappers.
 *
 * Removal, search, printing, set operations..
 */
// Remove an element from a vector (by value).
#define UTILS__REMOVE_FROM_VECTOR(_v, _elmt) \
    _v.erase(std::remove(_v.begin(), _v.end(), _elmt), _v.end())

// Boolean, true <==> _x is an element of the container.
// Must be iterable and sorted!
#define UTILS__IS_IN_CONTAINER(_x, _cont) \
        std::binary_search(_cont.begin(), _cont.end(), _x)
//    (std::find(_cont.begin(), _cont.end(), _x) == _cont.end())


// Output vectors / sets, in general
#define UTILS__AUTOPRINT_CONTAINER(_container_type) \
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
UTILS__AUTOPRINT_CONTAINER(set)
UTILS__AUTOPRINT_CONTAINER(vector)

// Set intersection and union.
#define UTILS__SET_INTERSECTION(_A,_B,_res) do { \
        std::set_intersection(_A->begin(), _A->end(), \
                              _B.begin(), _B.end(), \
                              std::back_inserter(_res)); \
    } while(0)

#define UTILS__SET_UNION(_A,_B,_res) do { \
        std::set_union(_A.begin(), _A.end(), \
                       _B.begin(), _B.end(), \
                       std::back_inserter(_res)); \
    } while(0)


/**
 * For use in live code.
 *
 * All TRACE commands with trace levels at most TRACE_LVL will
 * print: if TRACE_LVL is defined as TRACE_LVL__WARNING, then
 * all traces of level NONE, ERROR or WARNING will be printed.
 *
 * Never define TRACE_LVL as TRACE_LVL__OFF... kind of defeats
 * the purpose.
 */
typedef enum _TRACE_LVL_CODES {
    TRACE_LVL__NONE,
    TRACE_LVL__ERROR,
    TRACE_LVL__WARNING,
    TRACE_LVL__TEST,
    TRACE_LVL__DEBUG,
    TRACE_LVL__NOISE,
    TRACE_LVL__OFF
} TRACE_LVL__CODES;
#define TRACE_LVL TRACE_LVL__WARNING
#define TRACE(_lvl, _stream) do { \
        if (_lvl <= TRACE_LVL) { \
            UTILS__ASSERT_PRINT(_stream); \
        } \
    } while(0)


}

#endif // UTILS_H_INCLUDED
