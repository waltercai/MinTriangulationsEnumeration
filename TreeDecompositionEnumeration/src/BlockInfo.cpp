#include "BlockInfo.h"

namespace tdenum {

BlockInfo::BlockInfo(const Graph& setG, BlockPtr setB) :
	B(setB), GinducedB(setG, setB->nodes),locBySize(-1) { }

BlockInfo::BlockInfo(const SubGraph& setG, BlockPtr setB) :
	B(setB), GinducedB(setG, setB->nodes), locBySize(-1){}

int BlockInfo::blockSize() const {
	return B->nodes.size();
}

bool BlockInfo::isS(const MinimalSeparator& ms) {
	return B->S == ms;
}

bool BlockInfo::isC(const NodeSet& ns) {
	return B->C == ns;
}

void BlockInfo::updatePMCs(NodeSetSet& pmcList) {
	for (auto pmc = pmcList.begin(); pmc != pmcList.end(); pmc++)
		if (B->includesNodes(*pmc))
			pmcToBlocks[*pmc] = vector<int>();
}

void BlockInfo::updateLocation(int i, SepToBlockMap& SToB) {
	locBySize = i;

	// For each pmc associated with our current block, and its block list (pmcToB)
	for (auto pmcToB = pmcToBlocks.begin(); pmcToB != pmcToBlocks.end(); pmcToB++) {
		// Calculate all PMC blocks
		BlockVec pmcBlocks = GinducedB.getBlocksByMain(pmcToB->first);
		// For each block (pmcBlock) of the PMC
		for (auto pmcBlock = pmcBlocks.begin(); pmcBlock != pmcBlocks.end(); pmcBlock++)
			// For each BlockInfo associated  with pmcBlock's separator (sepBlockInfo)
			for (vector<BlockInfo*>::iterator sepBlockInfo = SToB[(*pmcBlock)->S].begin(); 
				sepBlockInfo != SToB[(*pmcBlock)->S].end(); sepBlockInfo++)
				// If sepBlockInfo.B is the same block as pmcBlock
				if ((*sepBlockInfo)->isC((*pmcBlock)->C)) {
					// Update its location in pmcToBlocks
					pmcToB->second.push_back((*sepBlockInfo)->locBySize);
					break;
				}
	}
}

}