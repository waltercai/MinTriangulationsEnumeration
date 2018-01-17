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
            base_dir(bd),
            verbose(v),
            max_depth(md) {
    TRACE(TRACE_LVL__DEBUG, "In DI ctor, base dir is '" << bd << "'");
    init();
}
DirectoryIterator::~DirectoryIterator() {
    // Close all open directories
    while (!dir_ptr_stack.empty()) {
        DIR* d = dir_ptr_stack.top();
        dir_ptr_stack.pop();
        closedir(d);
    }
}

void DirectoryIterator::init() {
    TRACE(TRACE_LVL__DEBUG, "Opening directory '" << base_dir << "'...");
    DIR* d = opendir(base_dir.c_str());
    if (d != NULL) {
        TRACE(TRACE_LVL__DEBUG, "Success!");
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
    TRACE(TRACE_LVL__DEBUG, "In with filename_ref = '" << filename_ref << "'");
    if (dir_ptr_stack.empty()) {
        filename_ref = "";
        return false;
    }

    // Read next:
    TRACE(TRACE_LVL__DEBUG, "Pointer stack not empty. Reading new dirent");
    struct dirent *drnt;
    DIR* d = dir_ptr_stack.top();
    drnt = readdir(d);  // This advances the underlying iterator

    // If d is NULL, we're done with this subdirectory. Go up a level.
    if (drnt == NULL) {
        TRACE(TRACE_LVL__DEBUG, "Got empty dirent. Popping pointer stack, iterating with filename_ref = '" << filename_ref << "'");
        dir_ptr_stack.pop();
        name_stack.pop();
        closedir(d);    // We're done with this directory, close it.
        return next_file(filename_ref);
    }

    string filename = drnt->d_name;
    string fullname = utils__merge_dir_basename(name_stack.top(), filename);
    TRACE(TRACE_LVL__DEBUG, "Dirent has filename '" << filename << "' and dirname '" << fullname << "'");

    // If the directory given is "." or "..", ignore it.
    if (filename == "." || filename == "..") {
        return next_file(filename_ref);
    }

    // If the user requested to skip this, skip
    for (unsigned int i=0; i<skip_list.size(); ++i) {
        if (fullname.find(skip_list[i]) !=  string::npos) {
            TRACE(TRACE_LVL__DEBUG, "Skip! Iterating with filename_ref='" << filename_ref << "'");
            return next_file(filename_ref);
        }
    }

    // If this is a directory, push it onto the stack and recurse:
    TRACE(TRACE_LVL__DEBUG, "Is directory?");
    struct stat s;
    stat(fullname.c_str(), &s);
    if ((s.st_mode & S_IFDIR) && dir_ptr_stack.size() < max_depth) {
        // Add trailing slash
        TRACE(TRACE_LVL__DEBUG, "After stat(), is a directory. Go deeper (stack size is "
                        << dir_ptr_stack.size() << ", max depth is " << max_depth << ")...");
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
        TRACE(TRACE_LVL__DEBUG, "Recursion with top of stack = '" << fullname << "'");
        return next_file(filename_ref);
    }

    // If this is a file, hurrah! Return it
    else if (s.st_mode & S_IFREG) {
        TRACE(TRACE_LVL__DEBUG, "Nope. Updating filename_ref to '" << fullname << "', returning true");
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
    TRACE(TRACE_LVL__DEBUG, "In");
    if (!from_this_point) {
        reset();
    }
    TRACE(TRACE_LVL__DEBUG, "Starting loop...");
    string dud;
    int cnt;
    for (cnt=0; next_file(dud); ++cnt) {
        TRACE(TRACE_LVL__DEBUG, "File " << cnt+1 << " is '" << dud << "'");
    }
    reset();
    return cnt;
}


}
