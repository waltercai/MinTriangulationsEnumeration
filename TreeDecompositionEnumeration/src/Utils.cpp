#include "Utils.h"

namespace tdenum {


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

string secs_to_hhmmss(time_t t) {
    ostringstream oss;
    oss << setfill('0') << setw(2) << t/(60*60);
    oss << ":" << setfill('0') << setw(2) << (t/60)%60;
    oss << ":" << setfill('0') << setw(2) << t%60;
    return oss.str();
}

string timestamp_to_hhmmss(time_t t) {
    char buff[20];
    strftime(buff, 20, "%H:%M:%S", localtime(&t));
    return buff;
}

void dump_string_to_file(const string& filename, const string& str, bool append) {
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

