CC=g++
CPPFLAGS=-std=c++11 -fopenmp -g
LDFLAGS=-fopenmp
OBJS=ChordalGraph.o Converter.o DatasetStatisticsGenerator.o \
 DataStructures.o DirectoryIterator.o Graph.o GraphProducer.o GraphReader.o \
 GraphStats.o IndSetExtBySeparators.o IndSetExtByTriangulation.o \
 IndSetScorerByTriangulation.o MinimalSeparatorsEnumerator.o \
 MinimalTriangulationsEnumerator.o MinimalTriangulator.o \
 PMCEnumerator.o PMCEnumeratorTester.o \
 PMCRacer.o SeparatorGraph.o SeparatorsScorer.o SubGraph.o Utils.o
OBJS_TRNG_ENUM=$(OBJS) MinTriangulationsEnumeration.o
OBJS_MAIN=$(OBJS) main.o
EXEC=main

$(EXEC): $(OBJS_MAIN)

clean:
	rm -rf $(OBJS)

BlockInfo.o: BlockInfo.cpp BlockInfo.h Graph.h SubGraph.h \
 IndependentSetExtender.h DataStructures.h
ChordalGraph.o: ChordalGraph.cpp ChordalGraph.h Graph.h DataStructures.h
Converter.o: Converter.cpp Converter.h Graph.h ChordalGraph.h \
 DataStructures.h
DatasetStatisticsGenerator.o: DatasetStatisticsGenerator.cpp \
 DatasetStatisticsGenerator.h Graph.h GraphReader.h GraphStats.h \
 MinimalSeparatorsEnumerator.h SeparatorsScorer.h DataStructures.h \
 PMCEnumerator.h SubGraph.h IndependentSetExtender.h DirectoryIterator.h \
 MinimalTriangulationsEnumerator.h SeparatorGraph.h \
 SuccinctGraphRepresentation.h IndSetExtByTriangulation.h \
 MinimalTriangulator.h ChordalGraph.h IndSetExtBySeparators.h \
 MaximalIndependentSetsEnumerator.h IndependentSetScorer.h \
 IndSetScorerByTriangulation.h GraphProducer.h Utils.h TestUtils.h
DataStructures.o: DataStructures.cpp DataStructures.h Graph.h
DirectoryIterator.o: DirectoryIterator.cpp TestUtils.h Utils.h \
 DirectoryIterator.h
Graph.o: Graph.cpp Graph.h DataStructures.h Utils.h
GraphProducer.o: GraphProducer.cpp GraphProducer.h Graph.h GraphReader.h \
 GraphStats.h DirectoryIterator.h Utils.h
GraphReader.o: GraphReader.cpp GraphReader.h Graph.h
GraphStats.o: GraphStats.cpp GraphStats.h Graph.h
IndSetExtBySeparators.o: IndSetExtBySeparators.cpp \
 IndSetExtBySeparators.h IndependentSetExtender.h Graph.h SubGraph.h
IndSetExtByTriangulation.o: IndSetExtByTriangulation.cpp \
 IndSetExtByTriangulation.h IndependentSetExtender.h \
 MinimalTriangulator.h ChordalGraph.h Graph.h Converter.h
IndSetScorerByTriangulation.o: IndSetScorerByTriangulation.cpp \
 IndSetScorerByTriangulation.h IndependentSetScorer.h Graph.h \
 ChordalGraph.h Converter.h
main.o: main.cpp DatasetStatisticsGenerator.h Graph.h GraphReader.h \
 GraphStats.h MinimalSeparatorsEnumerator.h SeparatorsScorer.h \
 DataStructures.h PMCEnumerator.h SubGraph.h IndependentSetExtender.h \
 DirectoryIterator.h GraphProducer.h Utils.h TestUtils.h \
 PMCEnumeratorTester.h ChordalGraph.h MinimalTriangulationsEnumerator.h \
 SeparatorGraph.h SuccinctGraphRepresentation.h \
 IndSetExtByTriangulation.h MinimalTriangulator.h IndSetExtBySeparators.h \
 MaximalIndependentSetsEnumerator.h IndependentSetScorer.h \
 IndSetScorerByTriangulation.h MinTriangulationsEnumeration.h \
 ResultsHandler.h PMCRacer.h
MinimalSeparatorsEnumerator.o: MinimalSeparatorsEnumerator.cpp \
 MinimalSeparatorsEnumerator.h Graph.h SeparatorsScorer.h \
 DataStructures.h
MinimalTriangulationsEnumerator.o: MinimalTriangulationsEnumerator.cpp \
 MinimalTriangulationsEnumerator.h Graph.h SeparatorGraph.h \
 MinimalSeparatorsEnumerator.h SeparatorsScorer.h DataStructures.h \
 SuccinctGraphRepresentation.h IndSetExtByTriangulation.h \
 IndependentSetExtender.h MinimalTriangulator.h ChordalGraph.h \
 IndSetExtBySeparators.h SubGraph.h MaximalIndependentSetsEnumerator.h \
 IndependentSetScorer.h IndSetScorerByTriangulation.h Converter.h
MinimalTriangulator.o: MinimalTriangulator.cpp MinimalTriangulator.h \
 ChordalGraph.h Graph.h DataStructures.h
MinTriangulationsEnumeration.o: MinTriangulationsEnumeration.cpp \
 GraphReader.h Graph.h MinimalTriangulationsEnumerator.h SeparatorGraph.h \
 MinimalSeparatorsEnumerator.h SeparatorsScorer.h DataStructures.h \
 SuccinctGraphRepresentation.h IndSetExtByTriangulation.h \
 IndependentSetExtender.h MinimalTriangulator.h ChordalGraph.h \
 IndSetExtBySeparators.h SubGraph.h MaximalIndependentSetsEnumerator.h \
 IndependentSetScorer.h IndSetScorerByTriangulation.h \
 MinTriangulationsEnumeration.h ResultsHandler.h
OptimalMinimalTriangulator.o: OptimalMinimalTriangulator.cpp \
 OptimalMinimalTriangulator.h Graph.h ChordalGraph.h DataStructures.h \
 BlockInfo.h SubGraph.h IndependentSetExtender.h TriangulationEvaluator.h \
 PMCEnumerator.h
PMCEnumerator.o: PMCEnumerator.cpp PMCEnumerator.h Graph.h SubGraph.h \
 IndependentSetExtender.h DataStructures.h MinimalSeparatorsEnumerator.h \
 SeparatorsScorer.h Utils.h
PMCEnumeratorTester.o: PMCEnumeratorTester.cpp PMCEnumeratorTester.h \
 TestUtils.h Utils.h Graph.h PMCEnumerator.h SubGraph.h \
 IndependentSetExtender.h DataStructures.h DirectoryIterator.h \
 GraphReader.h ChordalGraph.h MinimalTriangulationsEnumerator.h \
 SeparatorGraph.h MinimalSeparatorsEnumerator.h SeparatorsScorer.h \
 SuccinctGraphRepresentation.h IndSetExtByTriangulation.h \
 MinimalTriangulator.h IndSetExtBySeparators.h \
 MaximalIndependentSetsEnumerator.h IndependentSetScorer.h \
 IndSetScorerByTriangulation.h
PMCRacer.o: PMCRacer.cpp PMCEnumerator.h Graph.h SubGraph.h \
 IndependentSetExtender.h DataStructures.h PMCRacer.h \
 DatasetStatisticsGenerator.h GraphReader.h GraphStats.h \
 MinimalSeparatorsEnumerator.h SeparatorsScorer.h DirectoryIterator.h \
 Utils.h
RankedTriangulationEnumerator.o: RankedTriangulationEnumerator.cpp \
 RankedTriangulationEnumerator.h Graph.h DataStructures.h \
 TriangulationEvaluator.h SubGraph.h IndependentSetExtender.h \
 OptimalMinimalTriangulator.h ChordalGraph.h BlockInfo.h
SeparatorGraph.o: SeparatorGraph.cpp SeparatorGraph.h \
 MinimalSeparatorsEnumerator.h Graph.h SeparatorsScorer.h \
 DataStructures.h SuccinctGraphRepresentation.h
SeparatorsScorer.o: SeparatorsScorer.cpp SeparatorsScorer.h Graph.h
SubGraph.o: SubGraph.cpp SubGraph.h IndependentSetExtender.h Graph.h \
 DataStructures.h
TriangulationEvaluator.o: TriangulationEvaluator.cpp \
 TriangulationEvaluator.h Graph.h SubGraph.h IndependentSetExtender.h \
 DataStructures.h
Utils.o: Utils.cpp Utils.h

