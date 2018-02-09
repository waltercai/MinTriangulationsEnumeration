//#include "DatasetStatisticsGenerator.h"
#include "DatasetHandler.h"
#include "DirectoryIterator.h"
#include "Graph.h"
#include "GraphProducer.h"
#include "GraphStats.h"
#include "PMCAlg.h"
#include "PMCEnumeratorTester.h"
#include "PMCRacer.h"
#include "Utils.h"

using namespace tdenum;

int main(int argc, char *argv[]) {
    utils__timestamp_to_fulldate()
    Logger::start(string("log_") + utils__now_to_fulldate_filename() + ".txt", false);
//    PMCEnumeratorTester pmcet(false);
//    pmcet.go();
    DatasetHandler dh;
    dh.dori_project_races();
    return 0;
}
