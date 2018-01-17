CC=g++
CPPFLAGS=-std=c++11 -fopenmp -g
LDFLAGS=-fopenmp
OBJS=BlockInfo.o ChordalGraph.o Converter.o Dataset.o DatasetHandler.o \
 DataStructures.o DirectoryIterator.o Graph.o GraphProducer.o GraphReader.o \
 GraphStats.o IndSetExtBySeparators.o IndSetExtByTriangulation.o \
 IndSetScorerByTriangulation.o MinimalSeparatorsEnumerator.o \
 MinimalTriangulationsEnumerator.o MinimalTriangulator.o \
 OptimalMinimalTriangulator.o PMCAlg.o PMCEnumerator.o PMCRacer.o \
 RankedTriangulationEnumerator.o SeparatorGraph.o SeparatorsScorer.o \
 StatisticRequest.o SubGraph.o TriangulationEvaluator.o Utils.o
OBJS_TRNG_ENUM=$(OBJS) MinTriangulationsEnumeration.o
OBJS_TESTER=$(OBJS) DatasetTester.o GraphProducerTester.o GraphStatsTester.o \
 GraphTester.o PMCEnumeratorTester.o PMCRacerTester.o StatisticRequestTester.o \
 Tester.o TestInterface.o UtilsTester.o
OBJS_MAIN=$(OBJS) $(EXEC).o
EXEC=dori_main
EXEC_TEST=tester

$(EXEC): $(OBJS_MAIN)

$(EXEC_TEST): $(OBJS_TESTER)
	$(CC) $(CPPFLAGS) $(LDFLAGS) -o $(EXEC_TEST) $(OBJS_TESTER)

clean:
	rm -f $(OBJS) $(OBJS_MAIN) $(OBJS_TESTER) $(OBJS_TRNG_ENUM) $(EXEC) $(EXEC_TEST)

BlockInfo.o: BlockInfo.cpp BlockInfo.h Graph.h DataStructures.h \
 SubGraph.h IndependentSetExtender.h
ChordalGraph.o: ChordalGraph.cpp ChordalGraph.h Graph.h DataStructures.h
Converter.o: Converter.cpp Converter.h Graph.h DataStructures.h \
 ChordalGraph.h
Dataset.o: Dataset.cpp Dataset.h DirectoryIterator.h Graph.h \
 DataStructures.h GraphProducer.h GraphReader.h GraphStats.h \
 ChordalGraph.h PMCAlg.h StatisticRequest.h Utils.h PMCEnumeratorTester.h \
 PMCEnumerator.h SubGraph.h IndependentSetExtender.h TestInterface.h \
 PMCRacer.h MinimalSeparatorsEnumerator.h SeparatorsScorer.h \
 MinimalTriangulationsEnumerator.h SeparatorGraph.h \
 SuccinctGraphRepresentation.h IndSetExtByTriangulation.h \
 MinimalTriangulator.h IndSetExtBySeparators.h \
 MaximalIndependentSetsEnumerator.h IndependentSetScorer.h \
 IndSetScorerByTriangulation.h
DatasetHandler.o: DatasetHandler.cpp DatasetHandler.h Dataset.h \
 DirectoryIterator.h Graph.h DataStructures.h GraphProducer.h \
 GraphReader.h GraphStats.h ChordalGraph.h PMCAlg.h StatisticRequest.h \
 Utils.h PMCEnumeratorTester.h PMCEnumerator.h SubGraph.h \
 IndependentSetExtender.h TestInterface.h PMCRacer.h
DatasetTester.o: DatasetTester.cpp Dataset.h DirectoryIterator.h Graph.h \
 DataStructures.h GraphProducer.h GraphReader.h GraphStats.h \
 ChordalGraph.h PMCAlg.h StatisticRequest.h Utils.h PMCEnumeratorTester.h \
 PMCEnumerator.h SubGraph.h IndependentSetExtender.h TestInterface.h \
 PMCRacer.h DatasetTester.h MinimalSeparatorsEnumerator.h \
 SeparatorsScorer.h MinimalTriangulationsEnumerator.h SeparatorGraph.h \
 SuccinctGraphRepresentation.h IndSetExtByTriangulation.h \
 MinimalTriangulator.h IndSetExtBySeparators.h \
 MaximalIndependentSetsEnumerator.h IndependentSetScorer.h \
 IndSetScorerByTriangulation.h GraphStatsTester.h \
 StatisticRequestTester.h
DataStructures.o: DataStructures.cpp DataStructures.h
DirectoryIterator.o: DirectoryIterator.cpp DirectoryIterator.h \
 TestInterface.h Utils.h
dori_main.o: dori_main.cpp DatasetHandler.h Dataset.h DirectoryIterator.h \
 Graph.h DataStructures.h GraphProducer.h GraphReader.h GraphStats.h \
 ChordalGraph.h PMCAlg.h StatisticRequest.h Utils.h PMCEnumeratorTester.h \
 PMCEnumerator.h SubGraph.h IndependentSetExtender.h TestInterface.h \
 PMCRacer.h
Graph.o: Graph.cpp Graph.h DataStructures.h Utils.h
GraphProducer.o: GraphProducer.cpp GraphProducer.h DirectoryIterator.h \
 Graph.h DataStructures.h GraphReader.h GraphStats.h ChordalGraph.h \
 PMCAlg.h StatisticRequest.h Utils.h
GraphProducerTester.o: GraphProducerTester.cpp Graph.h DataStructures.h \
 GraphProducer.h DirectoryIterator.h GraphReader.h GraphStats.h \
 ChordalGraph.h PMCAlg.h StatisticRequest.h Utils.h GraphProducerTester.h \
 TestInterface.h GraphStatsTester.h
GraphReader.o: GraphReader.cpp GraphReader.h Graph.h DataStructures.h \
 Utils.h
GraphStats.o: GraphStats.cpp Graph.h DataStructures.h GraphReader.h \
 GraphStats.h ChordalGraph.h PMCAlg.h StatisticRequest.h Utils.h
GraphStatsTester.o: GraphStatsTester.cpp GraphReader.h Graph.h \
 DataStructures.h GraphStats.h ChordalGraph.h PMCAlg.h StatisticRequest.h \
 Utils.h GraphStatsTester.h TestInterface.h
GraphTester.o: GraphTester.cpp GraphTester.h Graph.h DataStructures.h \
 TestInterface.h Utils.h
IndSetExtBySeparators.o: IndSetExtBySeparators.cpp \
 IndSetExtBySeparators.h IndependentSetExtender.h Graph.h \
 DataStructures.h SubGraph.h
IndSetExtByTriangulation.o: IndSetExtByTriangulation.cpp \
 IndSetExtByTriangulation.h IndependentSetExtender.h \
 MinimalTriangulator.h ChordalGraph.h Graph.h DataStructures.h \
 Converter.h
IndSetScorerByTriangulation.o: IndSetScorerByTriangulation.cpp \
 IndSetScorerByTriangulation.h IndependentSetScorer.h Graph.h \
 DataStructures.h ChordalGraph.h Converter.h
main.o: main.cpp ChordalGraph.h Graph.h DataStructures.h \
 DirectoryIterator.h GraphProducer.h GraphReader.h GraphStats.h PMCAlg.h \
 StatisticRequest.h Utils.h GraphTester.h TestInterface.h \
 MinimalTriangulationsEnumerator.h SeparatorGraph.h \
 MinimalSeparatorsEnumerator.h SeparatorsScorer.h \
 SuccinctGraphRepresentation.h IndSetExtByTriangulation.h \
 IndependentSetExtender.h MinimalTriangulator.h IndSetExtBySeparators.h \
 SubGraph.h MaximalIndependentSetsEnumerator.h IndependentSetScorer.h \
 IndSetScorerByTriangulation.h MinTriangulationsEnumeration.h \
 ResultsHandler.h PMCEnumeratorTester.h PMCEnumerator.h PMCRacer.h
MinimalSeparatorsEnumerator.o: MinimalSeparatorsEnumerator.cpp \
 MinimalSeparatorsEnumerator.h Graph.h DataStructures.h \
 SeparatorsScorer.h
MinimalTriangulationsEnumerator.o: MinimalTriangulationsEnumerator.cpp \
 MinimalTriangulationsEnumerator.h Graph.h DataStructures.h \
 SeparatorGraph.h MinimalSeparatorsEnumerator.h SeparatorsScorer.h \
 SuccinctGraphRepresentation.h IndSetExtByTriangulation.h \
 IndependentSetExtender.h MinimalTriangulator.h ChordalGraph.h \
 IndSetExtBySeparators.h SubGraph.h MaximalIndependentSetsEnumerator.h \
 IndependentSetScorer.h IndSetScorerByTriangulation.h Converter.h
MinimalTriangulator.o: MinimalTriangulator.cpp MinimalTriangulator.h \
 ChordalGraph.h Graph.h DataStructures.h
MinTriangulationsEnumeration.o: MinTriangulationsEnumeration.cpp \
 GraphReader.h Graph.h DataStructures.h MinimalTriangulationsEnumerator.h \
 SeparatorGraph.h MinimalSeparatorsEnumerator.h SeparatorsScorer.h \
 SuccinctGraphRepresentation.h IndSetExtByTriangulation.h \
 IndependentSetExtender.h MinimalTriangulator.h ChordalGraph.h \
 IndSetExtBySeparators.h SubGraph.h MaximalIndependentSetsEnumerator.h \
 IndependentSetScorer.h IndSetScorerByTriangulation.h \
 MinTriangulationsEnumeration.h ResultsHandler.h
OptimalMinimalTriangulator.o: OptimalMinimalTriangulator.cpp \
 OptimalMinimalTriangulator.h Graph.h DataStructures.h ChordalGraph.h \
 BlockInfo.h SubGraph.h IndependentSetExtender.h TriangulationEvaluator.h \
 PMCEnumerator.h PMCAlg.h StatisticRequest.h Utils.h
PMCAlg.o: PMCAlg.cpp PMCAlg.h Utils.h
PMCEnumerator.o: PMCEnumerator.cpp PMCEnumerator.h DataStructures.h \
 Graph.h PMCAlg.h StatisticRequest.h Utils.h SubGraph.h \
 IndependentSetExtender.h MinimalSeparatorsEnumerator.h \
 SeparatorsScorer.h
PMCEnumeratorTester.o: PMCEnumeratorTester.cpp ChordalGraph.h Graph.h \
 DataStructures.h GraphProducer.h DirectoryIterator.h GraphReader.h \
 GraphStats.h PMCAlg.h StatisticRequest.h Utils.h \
 MinimalTriangulationsEnumerator.h SeparatorGraph.h \
 MinimalSeparatorsEnumerator.h SeparatorsScorer.h \
 SuccinctGraphRepresentation.h IndSetExtByTriangulation.h \
 IndependentSetExtender.h MinimalTriangulator.h IndSetExtBySeparators.h \
 SubGraph.h MaximalIndependentSetsEnumerator.h IndependentSetScorer.h \
 IndSetScorerByTriangulation.h PMCEnumeratorTester.h PMCEnumerator.h \
 TestInterface.h
PMCRacer.o: PMCRacer.cpp MinimalSeparatorsEnumerator.h Graph.h \
 DataStructures.h SeparatorsScorer.h PMCEnumerator.h PMCAlg.h \
 StatisticRequest.h Utils.h SubGraph.h IndependentSetExtender.h \
 PMCRacer.h GraphStats.h ChordalGraph.h
PMCRacerTester.o: PMCRacerTester.cpp DirectoryIterator.h PMCEnumerator.h \
 DataStructures.h Graph.h PMCAlg.h StatisticRequest.h Utils.h SubGraph.h \
 IndependentSetExtender.h PMCRacer.h GraphStats.h ChordalGraph.h \
 PMCRacerTester.h TestInterface.h
RankedTriangulationEnumerator.o: RankedTriangulationEnumerator.cpp \
 RankedTriangulationEnumerator.h Graph.h DataStructures.h \
 TriangulationEvaluator.h SubGraph.h IndependentSetExtender.h \
 OptimalMinimalTriangulator.h ChordalGraph.h BlockInfo.h
SeparatorGraph.o: SeparatorGraph.cpp SeparatorGraph.h \
 MinimalSeparatorsEnumerator.h Graph.h DataStructures.h \
 SeparatorsScorer.h SuccinctGraphRepresentation.h
SeparatorsScorer.o: SeparatorsScorer.cpp SeparatorsScorer.h Graph.h \
 DataStructures.h
StatisticRequest.o: StatisticRequest.cpp StatisticRequest.h PMCAlg.h \
 Utils.h
StatisticRequestTester.o: StatisticRequestTester.cpp StatisticRequest.h \
 PMCAlg.h Utils.h StatisticRequestTester.h TestInterface.h
SubGraph.o: SubGraph.cpp SubGraph.h IndependentSetExtender.h Graph.h \
 DataStructures.h
Tester.o: Tester.cpp PMCEnumerator.h DataStructures.h Graph.h PMCAlg.h \
 StatisticRequest.h Utils.h SubGraph.h IndependentSetExtender.h Tester.h \
 DatasetTester.h TestInterface.h GraphProducerTester.h GraphProducer.h \
 DirectoryIterator.h GraphReader.h GraphStats.h ChordalGraph.h \
 GraphStatsTester.h GraphTester.h PMCEnumeratorTester.h PMCRacerTester.h \
 StatisticRequestTester.h UtilsTester.h
TestInterface.o: TestInterface.cpp TestInterface.h Utils.h
TriangulationEvaluator.o: TriangulationEvaluator.cpp \
 TriangulationEvaluator.h Graph.h DataStructures.h SubGraph.h \
 IndependentSetExtender.h
Utils.o: Utils.cpp DirectoryIterator.h Utils.h
UtilsTester.o: UtilsTester.cpp UtilsTester.h TestInterface.h Utils.h
