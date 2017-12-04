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

    Logger::start("log.txt", false);
//    PMCEnumeratorTester pmcet(false);
//    pmcet.go();
    DatasetHandler dh;
    dh.dori_project_races();
    return 0;
}
