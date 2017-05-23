#include "TestUtils.h"
#include "Utils.h"
#include "DirectoryIterator.h"
#include <iostream>
#include <string>
using std::endl;
using std::string;

namespace tdenum {

/**
 * Init the directory stack with the main directory.
 *
 * If the directory failed to open, keep the stack empty so next_file
 * will always return false.
 */
DirectoryIterator::DirectoryIterator(const string& base_dir, bool v, unsigned int md) : verbose(v), max_depth(md) {
    DIR* d = opendir(base_dir.c_str());
    if (d != NULL) {
        dir_ptr_stack.push(d);
        name_stack.push(base_dir);
    }
    else if (verbose) {
        TRACE(TRACE_LVL__ERROR, "Error opening directory '" << base_dir << "'" << endl);
    }
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
bool DirectoryIterator::next_file(string* filename_ptr) {
    // Basics:
    if (filename_ptr == NULL) {
        return false;
    }
    if (dir_ptr_stack.empty()) {
        *filename_ptr = "";
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
        return next_file(filename_ptr);
    }

    string filename = drnt->d_name;
    string fullname = name_stack.top() + string(1,SLASH) + string(drnt->d_name);

    // If the directory given is "." or "..", ignore it.
    if (filename == "." || filename == "..") {
        return next_file(filename_ptr);
    }

    // If this is a directory, push it onto the stack and recurse:
    if (drnt->d_type == DT_DIR && dir_ptr_stack.size() < max_depth) {
        DIR* subdir = opendir(fullname.c_str());
        // If we failed to open the directory, try the next entry...
        if (subdir == NULL) {
            if (verbose) {
               TRACE(TRACE_LVL__ERROR, "Error opening '" << drnt->d_name << "', continuing..." << endl);
            }
            return next_file(filename_ptr);
        }
        // Otherwise, push it onto the stack and recurse.
        dir_ptr_stack.push(subdir);
        name_stack.push(fullname);
        return next_file(filename_ptr);
    }

    // If this is a file, hurrah! Return it
    else if (drnt->d_type == DT_REG) {
        *filename_ptr = fullname;   // Give the full name, relative to the working path
        return true;
    }

    // Uh oh.. we shouldn't be here
    if (verbose) {
        TRACE(TRACE_LVL__ERROR, "Unknown error" << endl);
    }
    *filename_ptr = "";
    return false;
}


}
