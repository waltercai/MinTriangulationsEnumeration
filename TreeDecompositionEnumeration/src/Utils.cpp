#include "DirectoryIterator.h"
#include "Utils.h"
#include <cstdlib>
#include <dirent.h>
#include <fstream>
#include <regex>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef UTILS__WINDOWS_MODE
#include <io.h>
#include <Shlwapi.h>
#include <Windows.h>
#endif

using std::ifstream;
using std::regex;
using std::regex_match;

namespace tdenum {

unsigned UTILS__MAX_RECURSION_DEPTH = 20;
unsigned UTILS__IO_RETRY_COUNT = 3;
unsigned UTILS__IO_RETRY_TIMEOUT_MS = 10;

//TRACE_LVL__CODES TRACE_LVL = TRACE_LVL__DEFAULT;
TRACE_LVL__CODES TRACE_LVL = TRACE_LVL__WARNING;
TRACE_LVL__CODES _prev_trace_lvl = TRACE_LVL;

const string Logger::dirname = "logs";
void Logger::stop() { state = false; }
void Logger::start(const string& f, bool append) {
    if (!utils__mkdir(Logger::dirname)) {
        cout << "FATAL ERROR: Couldn't create log directory!";
        exit(1);
    }
    filename = Logger::dirname + "/" + f;
    if (!append) {
        utils__delete_file(f);
    }
    state = true;
}
bool Logger::out(const ostringstream& os) { return out(os.str()); }
bool Logger::out(const string& str) {
    /**
     * Can't use TRACE() in the logger!
     *
     * As such, utility functions like utils__dump_string_to_file shouldn't be used
     */
    if (state) {
        ofstream outfile;
        try {
            outfile.open(filename, ios::out | ios::app);
            if (!outfile.good()) {
                cout << UTILS__ASSERT_PRINT_STREAM("FATAL: Couldn't open file '" << filename << "' to dump '" << str << "'") << endl;
                return false;
            }
            outfile << str;
            outfile.close();
        }
        catch(...) {
            cout << UTILS__ASSERT_PRINT_STREAM("Caught unknown exception") << endl;
            return false;
        }
        return true;
    }
    return false;
}

void utils__sleep_msecs(int x) {
    if (x<0) return;
#ifdef UTILS__WINDOWS_MODE
    Sleep(x);
#else // UTILS__WINDOWS_MODE
    usleep(x*1000);
#endif
}

bool utils__has_substr(const string& str, const string& sbstr, unsigned from_index) {
    // The only case an empty string has a substring is if the substring is empty as well.
    if (utils__str_empty(str)) {
        return (from_index == 0 && utils__str_empty(sbstr));
    }
    // If from_index is equal to the length of the string, the only substring is the empty
    // string.
    else if (from_index == utils__strlen(str)) {
        return utils__str_empty(sbstr);
    }
    else if (from_index > utils__strlen(str)) {
        TRACE(TRACE_LVL__ERROR, "Invalid index " << from_index << ": valid values are between 0 and " << utils__strlen(str));
        return false;
    }
    return (string::npos != str.find(sbstr,from_index));
}
string utils__replace_substr_with_substr(const string& str,
                                         const string& old_substr,
                                         const string& new_substr)
{
    // Some sanity
    if (utils__str_empty(str)) {
        return "";
    }
    else if (utils__str_empty(old_substr)) {
        return str;
    }

    // Go
    string ret = str;
    size_t old_len = utils__strlen(old_substr);
    size_t index = 0;
    while(true) {
        index = str.find(old_substr, index);
        if (index == string::npos) {
            break;
        }
        string first_half = ret.substr(0, index);
        string second_half = ret.substr(index + old_len);
        ret = first_half + new_substr + second_half;
        index += old_len;   // Search from new position
    }
    return ret;
}
string utils__join(const vector<string>& vs, char delim) {
    if (vs.empty())
        return "";
    string ret = vs[0];
    for (unsigned i = 1; i<vs.size(); ++i) {
        ret += delim + vs[i];
    }
    return ret;
}

unsigned utils__strlen(const string& s) { return s.length(); }
unsigned utils__strlen(const char* s) { return std::char_traits<char>::length(s); }
bool utils__str_eq(const string& s1, const string& s2) { return s1==s2; }
bool utils__str_empty(const string& s) { return utils__str_eq(s,""); }

string utils__strip_char(const string& s, char c) {
    int len = utils__strlen(s);
    if (len == 0)
        return s;
    if (len == 1)
        return s[0] == c ? "" : s;
    string ret = s;
    if (ret[len-1] == c)
        ret.erase(len-1);
    if (ret[0] == c)
        ret.erase(0,1);
    return ret;
}

vector<string> utils__tokenize(const string& text,  const vector<char>& delimiters) {
    vector<string> tokens;

    size_t start = 0, end = 0;
    while (true) {
        size_t next_delim = string::npos;
        for (char sep: delimiters) {
            size_t tmp = text.find(sep, start);
            if (tmp == string::npos) {
                continue;
            }
            if (next_delim == string::npos  || next_delim > tmp) {
                next_delim = tmp;
            }
        }
        if (next_delim == string::npos) {
            break;
        }
        // Found next delimiter.
        end = next_delim;
        tokens.push_back(text.substr(start, end - start));
        start = end + 1;
    }
    tokens.push_back(text.substr(start));
    return tokens;
}
int utils__replace_string_last_strlen = UTILS__INVALID_STRING_LENGTH;
string utils__replace_string(const string& s) {
    if (utils__replace_string_last_strlen == UTILS__INVALID_STRING_LENGTH) {
        utils__replace_string_last_strlen = utils__strlen(s);
        return s;
    }
    else {
        string eraser(utils__replace_string_last_strlen, '\b');
        utils__replace_string_last_strlen = utils__strlen(s);
        return eraser+s;
    }
}
string utils__replace_string() {
    if (utils__replace_string_last_strlen == UTILS__INVALID_STRING_LENGTH) {
        return "";
    }
    string eraser(utils__replace_string_last_strlen, '\b');
    string spacer(utils__replace_string_last_strlen, ' ');
    utils__replace_string_last_strlen = UTILS__INVALID_STRING_LENGTH;
    return eraser+spacer+eraser;
}

// Assume time_t stores seconds
string utils__timestamp_to_hhmmss(time_t t) {
    const time_t max_input = 99*3600+59*60+59;
    if (t < 0) {
        TRACE(TRACE_LVL__ERROR, "Negative time value given (" << t << ")");
        return "++:++:++";
    }
    if (t > max_input) {
        TRACE(TRACE_LVL__WARNING, "Amount of seconds larger than " << max_input
                            << " sent (" << t << ")");
        return "++:++:++";
    }
    ostringstream oss;
    oss << setfill('0') << setw(2) << t/(60*60);
    oss << ":" << setfill('0') << setw(2) << (t/60)%60;
    oss << ":" << setfill('0') << setw(2) << t%60;
    return oss.str();
}
string utils__timestamp_to_fulldate(time_t t) {
    char buff[100];
    strftime(buff, 100, "%c", localtime(&t));
    return buff;
}
string utils__now_to_fulldate_filename() {
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer,sizeof(buffer),"%d_%m_%Y__%H_%M_%S",timeinfo);
    return string(buffer);
}
time_t utils__hhmmss_to_timestamp(const string& hhmmss) {
    string regex_str = "^[0-9][0-9]:[0-5][0-9]:[0-5][0-9]$";
    if (!regex_match(hhmmss,regex(regex_str))) {
        TRACE(TRACE_LVL__ERROR, "Can't translate '" << hhmmss << "' into timestamp, "
                            << "must match the regex '" << regex_str << "'");
        return 0;
    }
    string hours = hhmmss.substr(0,2);
    string mins = hhmmss.substr(3,2);
    string secs = hhmmss.substr(6,2);
    return stoi(hours)*3600 + stoi(mins)*60 + stoi(secs);
}

size_t utils__basename_index(const string& path) {
    size_t index = utils__replace_substr_with_substr(path,"\\","/").find_last_of("/");
    return  (index == string::npos ? 0 : index+1);
}
string utils__to_forward_slashes(const string& path) { return utils__replace_substr_with_substr(path,"\\","/"); }
string utils__get_dirname(const string& s) {
    size_t index = utils__basename_index(s);
    return index == 0 ? string(".") : s.substr(0,index-1);
}
string utils__get_filename(const string& s) { return s.substr(utils__basename_index(s)); }
string utils__get_extension(const string& s) {
    // Don't look for any '.'s in the full path, only in the base name.
    string base = utils__get_filename(s);
    // Find last '.'
    size_t index = base.find_last_of(".");
    if (index == string::npos) {
        return "";
    }
    return base.substr(index+1);   // No '.'
}
string utils__merge_dir_basename(const string& dir, const string& base) {
    string leading_path = dir;
    char trailing = leading_path.back();
    if (trailing != '/' && trailing != '\\') {
        leading_path += '/';
    }
    return leading_path + string(base);
}

bool utils__file_exists(const string& file) {
    if (utils__dir_exists(file)) {
        TRACE(TRACE_LVL__OFF, "'" << file << "' is a directory! Returning FALSE");
        return false;
    }
    if (access(file.c_str(), F_OK) != -1) {
        TRACE(TRACE_LVL__OFF, "'" << file << "' is a file");
        return true;
    }
    return false;
}
bool utils__dir_exists(const string& path) {
    TRACE(TRACE_LVL__OFF, "In with path=" << path);
    string path_cpy = path;
    if (path_cpy.back() == '/' || path_cpy.back() == '\\') {
        path_cpy.pop_back();
    }
    struct stat st;
    int ret = stat(path_cpy.c_str(), &st);
    if (ret != 0) {
        if (errno != ENOENT) {  // ENOENT = No entry. No dir exists.
            TRACE(TRACE_LVL__ERROR, "Cannot access '" << path_cpy << "' for IO, stat() returned " << ret << " and errno is " << errno);
        }
        TRACE(TRACE_LVL__OFF,"No such dir '" << path_cpy << "', ret=" << ret << " and errno=" << errno);
        return false;
    }
    TRACE(TRACE_LVL__OFF, "The path '" << path_cpy << "' is " << (S_ISDIR(st.st_mode) ? "" : "not ") << "a directory");
    return S_ISDIR(st.st_mode);
}
bool utils__dir_is_empty(const string& path) {
    int n = 0;
    struct dirent *d;
    DIR *dir = opendir(path.c_str());
    if (dir == NULL) {  //Not a directory or doesn't exist
        return true;
    }
    // Count files, not including ./ and ../
    for (d = readdir(dir); d != NULL; d = readdir(dir)) {
        if(++n > 2) {
            break;
        }
    }
    closedir(dir);

    return (n <= 2);
}
bool utils__mkdir(const string& path) {
    // If we're already done, return true
    if (utils__dir_exists(path)) {
        return true;
    }
    // If the containing dir doesn't exist, return in error!
    string parent_dir = utils__get_dirname(path);
    if (parent_dir != "" && !utils__dir_exists(parent_dir)) {
        TRACE(TRACE_LVL__ERROR, "Cannot create new directory, parent dir '" << parent_dir << "' doesn't exist");
        return false;
    }
#ifdef UTILS__WINDOWS_MODE
    int mkdir_ret = _mkdir(path.c_str());
#else
    int mkdir_ret = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
#endif
    if (mkdir_ret == 0) {
        TRACE(TRACE_LVL__NOISE, "Created directory '" << path << "'");
        return true;
    }
    TRACE(TRACE_LVL__WARNING, "Couldn't create directory '" << path <<
                                "', return value is " << mkdir_ret << " and " <<
                                "errno=" << errno);
    return false;
}

string utils__read_file_as_string(const string& filename) {
    ifstream input(filename);
    if (!input.good()) {
        TRACE(TRACE_LVL__ERROR, "Cannot read file '" << filename << "'");
        return "";
    }
    string output = "";
    getline(input,output);
    for(string line; getline(input, line);) {
        output += "\n"+line;
    }
    input.close();
    return output;
}
vector<string> utils__getlines(const string& filename) { return utils__tokenize(utils__read_file_as_string(filename),{'\n'}); }
bool utils__dump_string_to_file(const string& str, const string& filename, bool append) {
    ofstream outfile;
    try {
        string dirname = utils__get_dirname(filename);
        if (!utils__mkdir(dirname)) {
            TRACE(TRACE_LVL__ERROR, "Couldn't create directory '" << dirname << "', cannot dump string to '" << filename << "'");
        }
        outfile.open(filename, ios::out | (append ? ios::app : ios::trunc));
        for (unsigned i=0; i<UTILS__IO_RETRY_COUNT && !outfile.good(); ++i) {
            utils__sleep_msecs(UTILS__IO_RETRY_TIMEOUT_MS);
            outfile.open(filename, ios::out | (append ? ios::app : ios::trunc));
        }
        if (!outfile.good()) {
            TRACE(TRACE_LVL__ERROR, "Couldn't open file '" << filename << "' to dump '" << str << "'");
            return false;
        }
        outfile << str;
        outfile.flush();
        outfile.close();
        if (!outfile.good()) {
            TRACE(TRACE_LVL__ERROR, "Something went wrong writing '" << str << "' to '" << filename << "'");
            return false;
        }
    }
    catch(...) {
        TRACE(TRACE_LVL__ERROR, "Caught unknown exception");
        if (outfile.is_open()) {
            outfile.flush();
            outfile.close();
        }
        return false;
    }

    TRACE(TRACE_LVL__NOISE, "File '" << filename << "' now contains '" << utils__read_file_as_string(filename) << "'");

    return true;
}

// Returns a vector of rows split by columns (second field of the first
// row would be v[0][1]), given the source CSV file.
vector<vector<string> > utils__read_csv(const string& filename) {
    vector<string> rows = utils__getlines(filename);
    vector<vector<string> > csv;
    for (string row: rows) {
        csv.push_back(utils__tokenize(row, {','}));
    }
    return csv;
}
bool utils__dump_csv(const vector<vector<string> >& data, const string& filename, bool append) {
    string dump_str = "";
    string delim = ",";
    for (vector<string> row: data) {
        for (string cell: row) {
            dump_str += cell + delim;
        }
        // Remove last comma
        dump_str.resize(utils__strlen(dump_str)-1);
        dump_str += "\n";
    }
    return utils__dump_string_to_file(dump_str, filename, append);
}

#define CLOSE_AND_RET(_bool) \
    closedir(d); \
    return (_bool)
bool utils__delete_all_files_in_dir(const string& path, int max_depth) {
    if (!utils__delete_dir(path, max_depth)) {
        return false;
    }
    return utils__mkdir(path);
}
bool utils__delete_dir(const string& path, int max_depth) {
    // This is a recursive method. Set a maximal depth
    if (max_depth <= 0) {
        TRACE(TRACE_LVL__ERROR, "Max depth of recursion (" << max_depth << ") reached, unable to delete all files");
        return false;
    }
    // Sanity
    if (!utils__dir_exists(path)) {
        return true;   // Dir doesn't exist
    }

    // Iterate and delete.
    // DO NOT use the DirectoryIterator - use recursion
    DIR* d = opendir(path.c_str());
    if (d == NULL) {
        TRACE(TRACE_LVL__ERROR, "Cannot open '" << path << "' for deletion");
        CLOSE_AND_RET(utils__dir_is_empty(path));
    }
    for (struct dirent *drnt = readdir(d); drnt != NULL; drnt = readdir(d)) {
        string name = drnt->d_name;
        string fullpath = utils__merge_dir_basename(path, name);
        if (name == "." || name == "..") {
            continue;  // Ignore these two
        }
        // If this is a directory, recurse.
        // Propagate failure
        if (utils__dir_exists(fullpath)) {
            if (!utils__delete_dir(fullpath,max_depth-1)) {
                CLOSE_AND_RET(false);
            }
        }
        // Otherwise, this is a file. Delete it
        else if (!utils__file_exists(fullpath)) {
            TRACE(TRACE_LVL__ERROR, "Uh oh... '" << fullpath << "' isn't a file and isn't a directory...");
            CLOSE_AND_RET(false);
        }
        else if (!utils__delete_file(fullpath)) {
            TRACE(TRACE_LVL__ERROR, "Couldn't delete file '" << fullpath << "'");
            CLOSE_AND_RET(false);
        }
        // Successfully deleted... whatever it was
        TRACE(TRACE_LVL__NOISE, "Deleted '" << fullpath << "'");
    }
    closedir(d);

    // Current directory
    if (!utils__dir_is_empty(path)) {
        TRACE(TRACE_LVL__ERROR, "Failed to delete the files in '" << path << "'");
        return false;
    }
    else if (!utils__delete_empty_dir(path)) {
        TRACE(TRACE_LVL__ERROR, "Couldn't delete empty directory '" << path << "'");
        return false;
    }
    return true;
}
bool delete_empty_dir_or_file_aux(const string& path, bool is_dir) {

    // Sanity
    if (is_dir) {
        if (!utils__dir_exists(path)) {
            return true;    // Nothing to do
        }
        if (!utils__dir_is_empty(path)) {
            return false;
        }
    }
    else if (!utils__file_exists(path)) {
        TRACE(TRACE_LVL__WARNING, "File '" << path << "' doesn't exist");
        return true;    // Already 'deleted'
    }

    // Try to delete
    bool success = false;
    int ret = 0;
    for (unsigned i=0; i<UTILS__IO_RETRY_COUNT; ++i) {
        ret = is_dir ? rmdir(path.c_str()) : remove(path.c_str());
        if (ret == 0) {
            success = true;
            break;
        }
        utils__sleep_msecs(UTILS__IO_RETRY_TIMEOUT_MS); // Before next retry
    }

    // Return
    if (!success) {
        TRACE(TRACE_LVL__WARNING, "Couldn't delete " << (is_dir ? "(empty) dir" : "file") << " '" << path << "', "
                                << (is_dir ? "rmdir()" : "remove()") << " returned " << ret << ", errno is " << errno);
    }
    else {
        TRACE(TRACE_LVL__NOISE, "Successfully deleted " << (is_dir ? "dir" : "file") << " '" << path << "'");
    }
    return success;
}
bool utils__delete_file(const string& path) { return delete_empty_dir_or_file_aux(path, false); }
bool utils__delete_empty_dir(const string& path) { return delete_empty_dir_or_file_aux(path, true); }

string Logger::filename;
bool Logger::state;
bool Logger::append;

}

