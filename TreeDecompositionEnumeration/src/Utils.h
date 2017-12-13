#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <vector>
using std::cout;
using std::endl;
using std::ios;
using std::map;
using std::ofstream;
using std::ostream;
using std::ostringstream;
using std::pair;
using std::set;
using std::setw;
using std::string;
using std::setfill;
using std::vector;

namespace tdenum {

/**
 * Some OS hacks and basic defines
 */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define UTILS__WINDOWS_MODE
//#define SLASH '\\'
#else
#define UTILS__LINUX_MODE
//#define SLASH '/'
#endif

#define SLASH '/'

#define EPSILON ((double)0.0001)
#define UTILS__EQ_FLOAT(_x,_y) ((_x) + EPSILON >= (_y) && (_x) - EPSILON <= (_y))

extern unsigned UTILS__MAX_RECURSION_DEPTH;
const time_t UTILS__MAX_LONG = std::numeric_limits<long>::max();

/**
 * Logger class.
 *
 * Chops wood in Canada
 */
class Logger {
private:
    static string filename;
    static const string dirname;
    static bool state;
    static bool append;
public:
    static void stop();
    // Add an option to append to file, or truncate it.
    // Returns false if the out stream is invalid (error)
    static void start(const string& f, bool appnd = true);
    static bool out(const ostringstream&);
    static bool out(const string&);
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
void utils__insert_critical(const V& val, T& container) {
    #pragma omp critical
    {
        container.insert(val);
    }
}

// Sleep for x milliseconds.
void utils__sleep_msecs(int);

/**
 * String manipulation, printing and logging.
 *
 * To fork-output all ASSERT_PRINTS to file, call Logger::start(filename).
 */

// strlen() and std::to_string cause problems
unsigned utils__strlen(const string&);
unsigned utils__strlen(const char*);
bool utils__str_eq(const string& s1, const string& s2);
bool utils__str_empty(const string&);

// Strips ONE instance of the input character from the beginning and end of
// the input string:
// strip_char("avfa",'a')="vf"
// strip_char("asdgaf", 'a')="sdgaf"
// strip_char("", 'a')=""
// strip_char("a", 'a')=""
// strip_char("aa", 'a')=""
// strip_char("aaa", 'a')="a"
string utils__strip_char(const string& s, char c);

vector<string> utils__tokenize(const string& input, const vector<char>& delimiters);

// TO_STRING expects a stream, or something that can be output via a stream
#define UTILS__TO_STRING(_x) static_cast< std::ostringstream & >( \
        ( std::ostringstream()/* << std::dec*/ << _x ) ).str()

#define UTILS__FILENAME (strrchr(__FILE__, SLASH) ? strrchr(__FILE__, SLASH) + 1 : __FILE__)


// For example:
// utils__replace_substr_with_substr("abcabd", "b", "xx") == "axxcaxxc"
bool utils__has_substr(const string& str, const string& sbstr, unsigned from_index = 0);
string utils__replace_substr_with_substr(const string& str,
                                         const string& old_substr,
                                         const string& new_substr);

// utils__join({"a","b"},',') == "a,b"
string utils__join(const vector<string>& vs, char delim);

// Returns the input string (assumes no newline in the string), and
// if the function is called with the same output ID (can be any
// integer) prepends \b characters to erase the previous string.
// If called with no arguments, outputs a string that erases the
// previous output, returns the cursor to it's original location.
// Useful for printing progress.
#define UTILS__INVALID_STRING_LENGTH (-1)
string utils__replace_string(const string&);
string utils__replace_string();
#define UTILS__REPLACE_STREAM(_stream) \
        utils__replace_string(UTILS__TO_STRING(_stream))

/**
 * Time utilities
 */
// Convert time stamp (in seconds) to / from readable time
string utils__timestamp_to_hhmmss(time_t t);
string utils__timestamp_to_fulldate(time_t t);
time_t utils__hhmmss_to_timestamp(const string& hhmmss);

// Max time_t value
const time_t UTILS__MAX_TIME = std::numeric_limits<time_t>::max();

/**
 * Useful container wrappers / utilities
 */

// For vectors. Doesn't assume the vector is sorted
template<typename T>
bool utils__is_in_vector(const T& elem, const vector<T>& v) {
    return (std::find(v.begin(), v.end(), elem) != v.end());
}

// Remove an element / elements from a vector (by value).
// All elements supporting X == _elem will be deleted
template<typename T>
void utils__remove_from_vector(const T& elem, vector<T>& v_ref) {
    v_ref.erase(std::remove(v_ref.begin(), v_ref.end(), elem), v_ref.end());
}

// Add an element to the vector, unless it already exists
template<typename T>
void utils__push_back_unique(const T& elem, vector<T>& v_ref) {
    if (!utils__is_in_vector(elem, v_ref)) {
        v_ref.push_back(elem);
    }
}

// Assume an incremental type with a binary comparison operator
template <typename T>
vector<T> utils__vector_range(const T& first, const T& last) {
    vector<T> v;
    for(T element=first; element<=last; ++element) {
        v.push_back(element);
    }
    return v;
}

// For sets<> only
template<typename T>
bool utils__is_in_set(const T& elem, const set<T>& s) {
    return (s.find(elem) != s.end());
}

// Compares the (unsorted) elements of two vectors
template<typename T>
bool utils__same_vector_elements(const vector<T>& v1, const vector<T>& v2) {
    if (v1.size() != v2.size()) return false;
    for (auto e1: v1) {
        bool found = false;
        for (auto e2: v2) {
            if (e1 == e2) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

// Boolean, true <==> _x is an element of the container.
// Must be iterable and sorted!
// This is a macro because we don't know the container type
#define UTILS__IS_IN_SORTED_CONTAINER(_elem, _cont) \
        (bool(std::binary_search(_cont.begin(), _cont.end(), _elem)))

// Output vectors / sets / containers, in general
#define UTILS__AUTOPRINT_CONTAINER(_container_type) \
template<typename V> \
ostream& operator<<(ostream& os, const _container_type<V>& v) { \
    string delim = ";"; \
    os << "{"; \
    for (auto i=v.begin(); i!=v.end(); ++i) { \
        os << *i << delim; \
    } \
    if (v.size() > 0) { \
        os << "\b"; /*removing trailing delimiter*/ \
    } \
    os << "}"; \
    return os; \
}
UTILS__AUTOPRINT_CONTAINER(set)
UTILS__AUTOPRINT_CONTAINER(vector)

template<typename S, typename T>
ostream& operator<<(ostream& os, pair<S,T> p) { return (os << "<" << p.first << "," << p.second << ">"); }


// Convert vector to set
template<typename T>
set<T> utils__vector_to_set(const vector<T>& v) {
    return set<T>(v.begin(), v.end());
}
#define UTILS__CREATE_SET(_type, ...) utils__vector_to_set(vector<_type>({ __VA_ARGS__ }))

// Output map to stream
template<typename K, typename V>
ostream& operator<<(ostream& os, const map<K,V>& m)  {
    for (auto it=m.begin(); it != m.end(); ++it) {
        os << it->first << " --> " << it->second << endl;
    }
    return os;
}

#define UTILS__VECTOR_INTERSECTION(_A,_B,_res) do { \
        _res.clear(); \
        std::set_intersection((_A).begin(), (_A).end(), \
                              (_B).begin(), (_B).end(), \
                              std::back_inserter(_res)); \
    } while(0)

#define UTILS__VECTOR_UNION(_A,_B,_res) do { \
        _res.clear(); \
        std::set_union((_A).begin(), (_A).end(), \
                       (_B).begin(), (_B).end(), \
                       std::back_inserter(_res)); \
    } while(0)


/**
 * IO section.
 *
 * Functions will usually output warnings / errors when applicable
 * using the TRACE mechanism.
 */
extern unsigned UTILS__IO_RETRY_COUNT;
extern unsigned UTILS__IO_RETRY_TIMEOUT_MS;

// Some basics:
string utils__to_forward_slashes(const string&);    // Backslashes to forward slashes
string utils__get_dirname(const string&);
string utils__get_filename(const string&);
string utils__get_extension(const string&); // Includes the '.' character
string utils__merge_dir_basename(const string& dir, const string& base);

bool utils__file_exists(const string& file);
bool utils__dir_exists(const string& path);
bool utils__dir_is_empty(const string& path);   // Returns true if directory doesn't exist, as well
bool utils__mkdir(const string& path);          // Returns true on success, false otherwise

// Outputs all lines from a file.
string utils__read_file_as_string(const string& filename);
vector<string> utils__getlines(const string& filename);

// Writes / appends a string of output to a file (doesn't append, by default)
// Returns true on success.
bool utils__dump_string_to_file(const string& str, const string& filename, bool append = false);

// Returns a vector of rows split by columns (second field of the first
// row would be v[0][1]), given the source CSV file.
vector<vector<string> > utils__read_csv(const string& filename);
bool utils__dump_csv(const vector<vector<string> >& data, const string& filename, bool append = false);

// This one returns true <==> after the function is done, the directory
// indicated by the argument exists and is empty.
bool utils__delete_all_files_in_dir(const string& path, int max_depth = UTILS__MAX_RECURSION_DEPTH);

// Returns true if the directory is successfully deleted
bool utils__delete_dir(const string& path, int max_depth = UTILS__MAX_RECURSION_DEPTH);

// Easy deletions.
// Return true if the path is no longer an entry
bool utils__delete_file(const string& path);
bool utils__delete_empty_dir(const string& path);

/**
 * Tracing, for use in live code.
 *
 * All TRACE commands with trace levels at most TRACE_LVL will
 * print: if TRACE_LVL is defined as TRACE_LVL__WARNING, then
 * all traces of level ALWAYS, ERROR or WARNING will be printed.
 *
 * Never define TRACE_LVL as TRACE_LVL__OFF... kind of defeats
 * the purpose.
 *
 * In general, WARNING level is recommended.
 */
// Trace levels
typedef enum TRACE_LVL__CODES {
    TRACE_LVL__ALWAYS,  // Always print
    TRACE_LVL__ERROR,   // Errors
    TRACE_LVL__WARNING, // Warnings
    TRACE_LVL__TEST,    // Test mode
    TRACE_LVL__DEBUG,   // Debug info in live code
    TRACE_LVL__NOISE,   // General info in live code
    TRACE_LVL__OFF      // Never print (turn this trace off)
} TRACE_LVL__CODES;
#define TRACE_LVL__DEFAULT TRACE_LVL__ERROR

// Trace level handlers
extern TRACE_LVL__CODES TRACE_LVL;  // Init: TRACE_LVL__DEFAULT
extern TRACE_LVL__CODES _prev_trace_lvl;
#define TRACE_LVL_OVERRIDE(_lvl) \
    _prev_trace_lvl = TRACE_LVL; \
    TRACE_LVL = _lvl
#define TRACE_LVL_RESTORE() \
    TRACE_LVL = _prev_trace_lvl

// Tracing
#define TRACE(_lvl, _stream) TRACE_NL(_lvl,_stream)
#define TRACE_NO_NL(_lvl, _stream) do { \
        if (_lvl <= TRACE_LVL) { \
            UTILS__ASSERT_PRINT(_stream); \
        } \
    } while(0)
#define TRACE_NL(_lvl, _stream) do { \
        TRACE_NO_NL(_lvl, _stream); \
        if (_lvl <= TRACE_LVL) { \
            cout << endl; \
        } \
    } while(0)

// Actual printing
#define UTILS__ASSERT_PRINT_STREAM(_stream) utils__timestamp_to_fulldate(time(NULL)) \
                                     << ":" << utils__get_filename(UTILS__FILENAME) \
                                     << ":" << __FUNCTION__ \
                                     << ":" << __LINE__ \
                                     << ":::" << _stream

#define UTILS__ASSERT_PRINT(_stream) do { \
        ostringstream _oss; \
        _oss << UTILS__ASSERT_PRINT_STREAM(_stream); \
        cout << _oss.str(); \
        Logger::out(_oss); \
    } while(0)

#define UTILS__PRINT_IF(_bool, _stream) UTILS__PRINT_IF_NONL(_bool, _stream << endl)
#define UTILS__PRINT_IF_NONL(_bool, _stream) do { \
        if (_bool) { \
            cout << UTILS__ASSERT_PRINT_STREAM(_stream); \
        } \
    } while(0)

}

#endif // UTILS_H_INCLUDED
