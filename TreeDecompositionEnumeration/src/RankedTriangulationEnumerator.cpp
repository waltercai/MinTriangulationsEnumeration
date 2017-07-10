#include "RankedTriangulationEnumerator.h"
#include <algorithm>

namespace tdenum {

	RankedTriangulationsEnumerator::RankedTriangulationsEnumerator(const Graph& G, TriangulationEvaluator* eval) :
		triangulator(G), resultQueue() {
		eval->resizeByNumBlocks(triangulator.getNumBlocks());
		resultQueue.push_back(triangulator.triangulate(eval));
	}

	bool RankedTriangulationsEnumerator::hasNext() {
		return resultQueue.size() > 0;
	}

	ChordalGraph RankedTriangulationsEnumerator::next() {
		// Remove first element from queue
		pop_heap(resultQueue.begin(), resultQueue.end());
		TriangulationResult next = resultQueue.back();
		resultQueue.pop_back();

		// Split this elements into new elements and evaluate them.
		NodeSetSet newInclusions;

		for (auto sep = next.minSeps.begin(); sep != next.minSeps.end(); sep++) {
			// If separator in Inclusion set
			if (next.eval->getInclusionConsts().isMember(*sep))
				continue;
			
			// Create evaluator with new constraint set 
			NodeSetSet newExclusions;
			newExclusions.insert(*sep);
			TriangulationEvaluator* newEval = next.eval->extendEvaluator(newInclusions, newExclusions);

			// Push result of the new constraint set into queue if it is legal
			TriangulationResult& newResult = triangulator.triangulate(newEval);
			if (newResult.cost != CONSTRAINT_VIOLATION) {
				resultQueue.push_back(newResult);
				//cout << "Pushed cost is " << resultQueue.back().cost << endl;
				push_heap(resultQueue.begin(), resultQueue.end());
			}

			newInclusions.insert(*sep);
		}

		return next.triangulation;
	}
}