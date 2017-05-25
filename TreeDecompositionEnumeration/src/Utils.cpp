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

string Logger::filename;
ofstream Logger::file;
bool Logger::state;



}

