#include "DatasetStatisticsGenerator.h"
#include "DataStructures.h"
#include "DirectoryIterator.h"
#include "Utils.h"
#include "TestUtils.h"
#include "PMCEnumeratorTester.h"
#include "GraphReader.h"
#include "ChordalGraph.h"
#include "MinimalTriangulationsEnumerator.h"
#include <iostream>
#include <fstream>

using namespace tdenum;

/**
 * Run the DatasetStatisticsGenerator and output files given the datasets NOT in
 * the "difficult" folder (so it won't take forever).
 */
/*
int main() {
    DirectoryIterator deadeasy_files(DATASET_DIR_BASE+DATASET_DIR_DEADEASY);
    DirectoryIterator easy_files(DATASET_DIR_BASE+DATASET_DIR_EASY);
    string dataset_filename;
    string output_filename = RESULT_DIR_BASE+"results.csv";
    // Clear the old results file
    ofstream out;
    out.open(output_filename, ios::out | ios::trunc);
    out << "Filename,Nodes,Edges,Separators,PMCs\n";
    out.close();
    while(deadeasy_files.next_file(&dataset_filename)) {
        DatasetStatisticsGenerator dsg(dataset_filename, output_filename, true);
        dsg.output_stats(true);
    }
    while(easy_files.next_file(&dataset_filename)) {
        DatasetStatisticsGenerator dsg(dataset_filename, output_filename, true);
        dsg.output_stats(true);
    }
    return 0;
}
*/

/**
 * Test the PMC enumerator
 */
int main() {
    Logger::start("log.txt", false);
    PMCEnumeratorTester p(false);
//    p.clearAll();
//    p.flag_crosscheck = true;
//    p.flag_triangleonstilts = true;
//    p.flag_independentsets = true;
    p.go();
    return 0;
}




