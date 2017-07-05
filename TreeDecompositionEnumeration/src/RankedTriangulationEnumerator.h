#ifndef RANKEDTRIANGULATIONSENUMERATOR_H_
#define RANKEDTRIANGULATIONSENUMERATOR_H_

#include "Graph.h"
#include "DataStructures.h"
#include "TriangulationEvaluator.h"
#include "OptimalMinimalTriangulator.h"
#include <vector>

//#include <chrono>
//#include <ctime>

namespace tdenum {

	/**
	* Enumerates the minimal triangulations of a graph in ranked order
	*/
	class RankedTriangulationsEnumerator {
		OptimalMinimalTriangulator triangulator;
		vector<TriangulationResult> resultQueue;

	public:
		// initialization
		RankedTriangulationsEnumerator(const Graph& G, TriangulationEvaluator* eval);
		// Checks whether there is another minimal triangulation
		bool hasNext();
		// Returns another minimal triangulation
		ChordalGraph next();
	};

} /* namespace tdenum */

#endif /* RANKEDTRIANGULATIONSENUMERATOR_H_ */
