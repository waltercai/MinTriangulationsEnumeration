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
    if (utils_replace_string__last_strlen = UTILS__INVALID_STRING_LENGTH) {
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

string timestamp_aux(time_t t, bool fulldate) {
    char buff[100];
    strftime(buff, 100, fulldate ? "%c" : "%H:%M:%S", localtime(&t));
    return buff;
}
string utils__timestamp_to_hhmmss(time_t t) {
    return timestamp_aux(t, false);
}
string utils__timestamp_to_fulldate(time_t t) {
    return timestamp_aux(t, true);
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

string Logger::filename;
ofstream Logger::file;
bool Logger::state;



}

