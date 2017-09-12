#ifndef DIRECTORYITERATOR_H_INCLUDED
#define DIRECTORYITERATOR_H_INCLUDED

#include <string>
#include <dirent.h>
#include <stack>
#include <vector>
using std::stack;
using std::string;
using std::vector;

/**
 * Some useful directory names
 */
#define DATASET_DIR_BASE string("Datasets"+string(1,SLASH))
#define DATASET_NEW_DIR_BASE string("NewDatasets"+string(1,SLASH))
#define RESULT_DIR_BASE string("Results"+string(1,SLASH))
#define DORI_PROJECT_RESULTS_DIR string(RESULT_DIR_BASE+"DoriProject"+string(1,SLASH))
#define DORI_PROJECT_DATASET_DIR string(DATASET_DIR_BASE+"DoriProject"+string(1,SLASH))
#define DATASET_DIR_DEADEASY string("DeadEasy"+string(1,SLASH))
#define DATASET_DIR_EASY string("Easy"+string(1,SLASH)+ "Random"+string(1,SLASH))
#define DATASET_DIR_DIFFICULT string("Difficult"+string(1,SLASH))
#define DATASET_DIR_DIFFICULT_BN string(DATASET_DIR_DIFFICULT+"BN"+string(1,SLASH))

#define DUMMY_FILENAME "dummy.txt"

namespace tdenum {

/**
 * Utility class used to traverse files under a directory (recursively).
 */
class DirectoryIterator {
private:
    stack<DIR*> dir_ptr_stack;    // Use a stack to keep track of recursive deepening
    stack<string> name_stack;     // The names of the directories in the stack
    vector<string> skip_list;     // If some s in skip[] is a substring of the current filename, skip it
    bool verbose;                 // If true, prints errors
    unsigned int max_depth;       // Maximal depth of subdirectory recursion
public:

    // Init with the base directory, verbosity (print errors) and maximum depth of
    // directory tree.
    DirectoryIterator(const string& base_dir,
                      bool verbose = true,
                      unsigned int md = 100);

    // Add a substring to skip
    DirectoryIterator& skip(const string& s);

    // Fetches the name of the next file under the directory.
    // Returns true <==> such a file exists (we aren't at the last file).
    bool next_file(string* str);
};

}

#endif // DIRECTORYITERATOR_H_INCLUDED
