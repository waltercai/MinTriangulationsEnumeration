#ifndef MINIMALSEPARATORSENUMERATOR_H_
#define MINIMALSEPARATORSENUMERATOR_H_

#include "Graph.h"
#include "SeparatorsScorer.h"
#include "DataStructures.h"

namespace tdenum {

/**
 * Enumerates the minimal separators of a graph with polynomial delay.
 *
 * Implementing the paper:
 * Generating all the minimal separators of a graph.
 * A. Berry, J. P. Bordat, and O. Cogis, 1999.
 */
class MinimalSeparatorsEnumerator {
	Graph graph;
	SeparatorsScorer scorer;
	WeightedNodeSetQueue separatorsToExtend;
	NodeSetSet separatorsExtended;
	void minimalSeparatorFound(const MinimalSeparator& s);
public:
	// Initialization
	MinimalSeparatorsEnumerator(const Graph& g, SeparatorsScoringCriterion c);
	// Checks whether there is another minimal separator
	bool hasNext();
	// Returns another minimal separator
	MinimalSeparator next();
	// Returns all minimal separators as a NodeSetSet.
	// If a non-zero time limit is given and the algorithm takes too long,
	// returns false and sets the output to an empty NodeSetSet.
	bool getAll(NodeSetSet& out, time_t limit = 0);
};

} /* namespace tdenum */

#endif /* MINIMALSEPARATORSENUMERATOR_H_ */
