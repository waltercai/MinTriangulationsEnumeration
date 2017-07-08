#ifndef BLOCKINFO_H
#define BLOCKINFO_H

#include "Graph.h"
#include "SubGraph.h"
#include "DataStructures.h"
//#include "TriangulationEvaluator.h"
#include <vector>
#include <map>

namespace tdenum {
	/**
	* TODO - write documentation
	*/
	class BlockInfo {
		friend class OptimalMinimalTriangulator;

		// The current block (S,C)
		const BlockPtr B;
		// SubGraph of the original graph, induced by block nodes
		const SubGraph GinducedB;
		// Location of block in list of graph blocks
		int locBySize;
		// Map each PMC such that S subset PMC subseteq S,C for this block
		// To the location of each of its blocks in the block list
		map<NodeSet, vector<int>> pmcToBlocks;

	public:
		typedef map<MinimalSeparator, vector<BlockInfo*>> SepToBlockMap;

		BlockInfo(const Graph& originalG, BlockPtr setB);
		BlockInfo(const SubGraph& originalG, BlockPtr setB);

		int blockSize() const;
		int getLocation() { return locBySize; }

		bool isS(const MinimalSeparator&);
		bool isC(const NodeSet&);

		// Insert only relevant PMCs into pmcToBlocks
		void updatePMCs(NodeSetSet& pmc_list);

		// Update my location and location of each PMC's blocks
		void updateLocation(int i, SepToBlockMap& SToB);

		//void evaluateBlock(TriangulationEvaluator&);
	};

	typedef map<MinimalSeparator, vector<BlockInfo*>> SepToBlockMap;

} /* namespace tdenum */


#endif // BLOCKINFO_H
