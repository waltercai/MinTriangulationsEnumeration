#include <string>
#include <iostream>
#include "Graph.h"
#include "ResultsHandler.h"
using std::string;
using std::ofstream;


namespace tdenum {


class InputFile {
	string name;
	string wholename;
	string innerContainingFolder;
	string outerContainingFolder;
public:
	InputFile(string path);
	string getName() const;
	string getPath() const;
	string getField() const;
	string getType() const;
};

void printSummaryHeader(ofstream& summaryOutput);

void printSummary(ofstream& summaryOutput, InputFile& input, Graph& graph, bool timeLimitExceeded,
		double time, string algorithm, int separators, ResultsHandler& results);


}

