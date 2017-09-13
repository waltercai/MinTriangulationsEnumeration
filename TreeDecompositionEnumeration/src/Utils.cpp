#include "Utils.h"

namespace tdenum {

unsigned utils__strlen(const string& s) {
    return s.length();
}
unsigned utils__strlen(const char* s) {
    return std::char_traits<char>::length(s);
}

void Logger::stop() {
    if (state) {
        file.close();
        state = false;
    }
}
void Logger::start(const string& f, bool append) {
    if (state) {
        stop();
    }
    filename = f;
    file.open(filename, std::ios::out | (append ? std::ios::app : std::ios::trunc));
    state = true;
}
void Logger::out(const ostringstream& os) {
    if (state) {
        file << os.str();
    }
}

string utils__replace_substr_with_substr(const string& str,
                                         const string& old_substr,
                                         const string& new_substr)
{
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

int utils_replace_string__last_strlen = UTILS__INVALID_STRING_LENGTH;
string utils__replace_string(const string& s) {
    if (utils_replace_string__last_strlen == UTILS__INVALID_STRING_LENGTH) {
        utils_replace_string__last_strlen = utils__strlen(s);
        return s;
    }
    else {
        string eraser(utils_replace_string__last_strlen, '\b');
        utils_replace_string__last_strlen = utils__strlen(s);
        return eraser+s;
    }
}
string utils__replace_string() {
    if (utils_replace_string__last_strlen == UTILS__INVALID_STRING_LENGTH) {
        return "";
    }
    string eraser(utils_replace_string__last_strlen, '\b');
    string spacer(utils_replace_string__last_strlen, ' ');
    utils_replace_string__last_strlen = UTILS__INVALID_STRING_LENGTH;
    return eraser+spacer+eraser;
}

string utils__secs_to_hhmmss(time_t t) {
    ostringstream oss;
    oss << setfill('0') << setw(2) << t/(60*60);
    oss << ":" << setfill('0') << setw(2) << (t/60)%60;
    oss << ":" << setfill('0') << setw(2) << t%60;
    return oss.str();
}
// Assume time_t stores seconds
string utils__timestamp_to_hhmmss(time_t t) {
    return utils__secs_to_hhmmss(t);
}
string utils__timestamp_to_fulldate(time_t t) {
    char buff[100];
    strftime(buff, 100, "%c", localtime(&t));
    return buff;
}
time_t utils__hhmmss_to_timestamp(const string& hhmmss) {
    if (utils__strlen(hhmmss) != 8) {
        TRACE(TRACE_LVL__ERROR, "Can't translate '" << hhmmss << "' into timestamp, must be a string of type 'HH:MM:SS'");
        return 0;
    }
    string hours = hhmmss.substr(0,2);
    string mins = hhmmss.substr(3,2);
    string secs = hhmmss.substr(6,2);
    return stoi(hours)*3600 + stoi(mins)*60 + stoi(secs);
}

void utils__dump_string_to_file(const string& filename, const string& str, bool append) {
    ofstream outfile;
    try {
        outfile.open(filename, ios::out | (append ? ios::app : ios::trunc));
        if (!outfile.good()) {
            TRACE(TRACE_LVL__ERROR, "Couldn't open file '" << filename << "'");
            return;
        }
        outfile << str;
        outfile.close();
    }
    catch(...) {
        TRACE(TRACE_LVL__ERROR, "Caught unknown exception");
    }
}

bool utils__file_exists(const string& file) {
    return (access(file.c_str(), F_OK) != -1);
}

string Logger::filename;
ofstream Logger::file;
bool Logger::state;



}

