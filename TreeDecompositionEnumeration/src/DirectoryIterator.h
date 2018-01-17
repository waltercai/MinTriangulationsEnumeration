#ifndef DIRECTORYITERATOR_H_INCLUDED
#define DIRECTORYITERATOR_H_INCLUDED

// DO NOT include Utils.h, Utils includes this header
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
#define DATASET_DIR_DEADEASY string("DeadEasy"+string(1,SLASH))
#define DATASET_DIR_EASY string("Easy"+string(1,SLASH)+ "Random"+string(1,SLASH))
#define DATASET_DIR_DIFFICULT string("Difficult"+string(1,SLASH))
#define DATASET_DIR_DIFFICULT_RANDOM_30 string(DATASET_DIR_DIFFICULT+"Random"+string(1,SLASH)+"Probability30percent"+string(1,SLASH))
#define DATASET_DIR_DIFFICULT_RANDOM_50 string(DATASET_DIR_DIFFICULT+"Random"+string(1,SLASH)+"Probability50percent"+string(1,SLASH))
#define DATASET_DIR_DIFFICULT_RANDOM_70 string(DATASET_DIR_DIFFICULT+"Random"+string(1,SLASH)+"Probability70percent"+string(1,SLASH))
#define DATASET_DIR_DIFFICULT_BN string(DATASET_DIR_DIFFICULT+"BN"+string(1,SLASH))
#define DATASET_DIR_DIFFICULT_BN_OBJDET string(DATASET_DIR_DIFFICULT_BN+"ObjectDetection"+string(1,SLASH))

#define DUMMY_FILENAME "dummy.txt"

namespace tdenum {

/**
 * Utility class used to traverse files under a directory (recursively).
 */
class DirectoryIterator {
private:
    string base_dir;              // The original base directory
    stack<DIR*> dir_ptr_stack;    // Use a stack to keep track of recursive deepening
    stack<string> name_stack;     // The names of the directories in the stack
    vector<string> skip_list;     // If some s in skip[] is a substring of the current filename, skip it
    bool verbose;                 // If true, prints errors
    unsigned int max_depth;       // Maximal depth of subdirectory recursion

    void init();                  // Sets the first base directory on the ptr_stack + name_stack

    // Closes all opened directories, empties the dir stack
    DirectoryIterator& close_and_empty_dir_stack();

public:

    // Init with the base directory, verbosity (print errors) and maximum depth of
    // directory tree.
    DirectoryIterator(const string& base_dir,
                      bool verbose = true,
                      unsigned int md = 100);
    ~DirectoryIterator();


    // Resets the Directory iterator with the same original base dir.
    DirectoryIterator& reset(bool keep_skiplist = true);

    // Reconstructs the directory iterator with a new base dir
    DirectoryIterator& reset(const string& bd, bool keep_skiplist = true);

    // Add a substring(s) to skip
    DirectoryIterator& skip(const string& s);
    DirectoryIterator& skip(const vector<string>& vs);

    // Fetches the name of the next file under the directory.
    // Returns true <==> such a file exists (we aren't at the last file).
    bool next_file(string& p_str);

    // Counts the total number of files found (counts calls to next_file()).
    // If from_this_point is set, the method won't first call reset().
    // This method calls reset() at the end anyway (internal iteration is reset).
    int file_count(bool from_this_point = false);
};

}

#endif // DIRECTORYITERATOR_H_INCLUDED
