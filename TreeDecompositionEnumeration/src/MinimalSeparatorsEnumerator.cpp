#include "MinimalSeparatorsEnumerator.h"
#include <ctime>

namespace tdenum {

/*
 * Initialization
 */
MinimalSeparatorsEnumerator::MinimalSeparatorsEnumerator(const Graph& g, SeparatorsScoringCriterion c) :
	graph(g), scorer(g,c) {
	// Initialize separatorsNotReturned according to the initialization phase
	for (Node v = 0; v < g.getNumberOfNodes(); v++) {
		set<Node> vAndNeighbors = graph.getNeighbors(v);
		vAndNeighbors.insert(v);
		BlockVec blocks = graph.getBlocks(vAndNeighbors);
		for (auto it=blocks.begin(); it!=blocks.end(); ++it) {
			if ((*it)->S.size() > 0) {
				int score = scorer.scoreSeparator((*it)->S);
				separatorsToExtend.insert((*it)->S, score);
			}
		}
	}
}

/*
 * Outputs whether there is a separator not yet returned.
 */
bool MinimalSeparatorsEnumerator::hasNext() {
	return !separatorsToExtend.isEmpty();
}

/*
 * Input: minimal separator.
 * If this separator was not found before, it is inserted to the set of not
 * returned separators.
 */
void MinimalSeparatorsEnumerator::minimalSeparatorFound(const MinimalSeparator& s) {
	if (s.size() > 0 && !separatorsExtended.isMember(s)) {
		int score = scorer.scoreSeparator(s);
		separatorsToExtend.insert(s, score);
	}
}

/*
 * Processes a separator that was not yet processed, transfers it to the list of
 * returned separators, and returns it.
 */
MinimalSeparator MinimalSeparatorsEnumerator::next() {
	// Verify that there is another separator
	if (!hasNext()) {
		return MinimalSeparator();
	}
	// Choose separator and transfer to list of returned
	MinimalSeparator s = separatorsToExtend.pop();
	separatorsExtended.insert(s);
	// Process separator according to the generation phase
	for (MinimalSeparator::iterator i = s.begin(); i != s.end(); ++i) {
		Node x = *i;
		set<Node> xNeighborsAndS = graph.getNeighbors(x);
		xNeighborsAndS.insert(s.begin(),s.end());
		BlockVec blocks = graph.getBlocks(xNeighborsAndS);
		for (auto j = blocks.begin(); j != blocks.end(); ++j) {
			minimalSeparatorFound((*j)->S);
		}
	}
	return s;
}

bool MinimalSeparatorsEnumerator::getAll(NodeSetSet& out, time_t limit) {
    time_t t = time(NULL);
    while (hasNext()) {
        out.insert(next());
        if (limit > 0 && difftime(time(NULL),t) > limit) {
            out = NodeSetSet();
            return false;
        }
    }
    return true;
}

} /* namespace tdenum */
