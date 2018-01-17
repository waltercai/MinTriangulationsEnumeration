#include "DirectoryIterator.h"
#include "TestInterface.h"
#include "Utils.h"
#include <iostream>
#include <string>
#include <sys/stat.h>
using std::endl;
using std::string;

namespace tdenum {

/**
 * Init the directory stack with the main directory.
 *
 * If the directory failed to open, keep the stack empty so next_file
 * will always return false.
 */
DirectoryIterator::DirectoryIterator(const string& bd, bool v, unsigned int md) :
        base_dir(bd), verbose(v), max_depth(md) { init(); }
DirectoryIterator::~DirectoryIterator() {
    // Close all open directories
    while (!dir_ptr_stack.empty()) {
        DIR* d = dir_ptr_stack.top();
        dir_ptr_stack.pop();
        closedir(d);
    }
}

void DirectoryIterator::init() {
    DIR* d = opendir(base_dir.c_str());
    if (d != NULL) {
        dir_ptr_stack.push(d);
        name_stack.push(base_dir);
    }
    else if (verbose) {
        TRACE(TRACE_LVL__ERROR, "Error opening directory '" << base_dir << "'" << endl);
    }
}

DirectoryIterator& DirectoryIterator::reset(bool keep_skiplist) { return reset(base_dir, keep_skiplist); }
DirectoryIterator& DirectoryIterator::reset(const string& bd, bool keep_skiplist) {
    if (keep_skiplist) {
        vector<string> tmplist = skip_list;
        // Let the destructor do it's job
        *this = DirectoryIterator(bd);
        skip(tmplist);
    }
    else {
        *this = DirectoryIterator(bd);
    }
    return *this;
}

/**
 * Adds a string (or strings) to the skip list.
 */
DirectoryIterator& DirectoryIterator::skip(const string& s) {
    skip_list.push_back(s);
    return *this;
}
DirectoryIterator& DirectoryIterator::skip(const vector<string>& vs) {
    for (auto s: vs) {
        skip(s);
    }
    return *this;
}

/**
 * Find the next file.
 *
 * If the next dirent is a directory, open it and push it onto the stack,
 * and call next_file again (will now search the subdirectory).
 * If the next dirent is NULL, it's time to pop the stack.
 * If the stack is empty, we're done: return false and set the return string
 * to the empty string.
 */
bool DirectoryIterator::next_file(string& filename_ref) {
    // Basics:
    if (dir_ptr_stack.empty()) {
        filename_ref = "";
        return false;
    }

    // Read next:
    struct dirent *drnt;
    DIR* d = dir_ptr_stack.top();
    drnt = readdir(d);  // This advances the underlying iterator

    // If d is NULL, we're done with this subdirectory. Go up a level.
    if (drnt == NULL) {
        dir_ptr_stack.pop();
        name_stack.pop();
        closedir(d);    // We're done with this directory, close it.
        return next_file(filename_ref);
    }

    string filename = drnt->d_name;
    string fullname = utils__merge_dir_basename(name_stack.top(), filename);

    // If the directory given is "." or "..", ignore it.
    if (filename == "." || filename == "..") {
        return next_file(filename_ref);
    }

    // If the user requested to skip this, skip
    for (unsigned int i=0; i<skip_list.size(); ++i) {
        if (fullname.find(skip_list[i]) !=  string::npos) {
            return next_file(filename_ref);
        }
    }

    // If this is a directory, push it onto the stack and recurse:
    struct stat s;
    stat(fullname.c_str(), &s);
    if ((s.st_mode & S_IFDIR) && dir_ptr_stack.size() < max_depth) {
        // Add trailing slash
        fullname += string(1,SLASH);
        // Open the directory
        DIR* subdir = opendir(fullname.c_str());
        // If we failed to open the directory, try the next entry...
        if (subdir == NULL) {
            if (verbose) {
               TRACE(TRACE_LVL__ERROR, "Error opening '" << drnt->d_name << "', continuing..." << endl);
            }
            return next_file(filename_ref);
        }
        // Otherwise, push it onto the stack and recurse.
        dir_ptr_stack.push(subdir);
        name_stack.push(fullname);
        return next_file(filename_ref);
    }

    // If this is a file, hurrah! Return it
    else if (s.st_mode & S_IFREG) {
        filename_ref = fullname;   // Give the full name, relative to the working path
        return true;
    }

    // Uh oh.. we shouldn't be here
    TRACE(TRACE_LVL__ERROR, "Unknown error when opening "
                            "'" << filename << "'" << endl);
    filename_ref = "";
    return false;
}

int DirectoryIterator::file_count(bool from_this_point) {
    if (!from_this_point) {
        reset();
    }
    string dud;
    int cnt;
    for (cnt=0; next_file(dud); ++cnt) {
        TRACE(TRACE_LVL__DEBUG, "File " << cnt+1 << " is '" << dud << "'");
    }
    reset();
    return cnt;
}


}
