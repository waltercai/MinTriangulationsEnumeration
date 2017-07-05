#ifndef OPTIMALMINIMALTRIANGULATOR_H_
#define OPTIMALMINIMALTRIANGULATOR_H_

#include "Graph.h"
#include "ChordalGraph.h"
#include "DataStructures.h"
#include "BlockInfo.h"
#include "TriangulationEvaluator.h"

namespace tdenum {
	struct TriangulationResult {
		ChordalGraph triangulation;
		NodeSetSet minSeps;
		float cost;
		TriangulationEvaluator* eval;
		bool operator<(const TriangulationResult& rhs) const {
			return rhs.cost < this->cost;
		}
	};

	/**
	* Calculates an optimal minimal triangulation of the graph
	* According to extended Bouche and Toddinca alg.
	*/
	class OptimalMinimalTriangulator {
		const Graph& g;
		//NodeSetSet minSeps;
		//NodeSetSet pmcs;
		vector<BlockInfo*> allBlockInfos;
		int numMainBlocks;

		void calculateBlockInfos();
		SepToBlockMap calculateSepBlockInfos();
		void calculateMainBlockInfos();
	public:

		NodeSetSet minSeps;
		NodeSetSet pmcs;

		OptimalMinimalTriangulator(const Graph& g);
		
		int getNumBlocks() { return allBlockInfos.size(); }

		TriangulationResult triangulate(TriangulationEvaluator*);
	};

} /* namespace tdenum */

#endif /* OPTIMALMINIMALTRIANGULATOR_H_ */
