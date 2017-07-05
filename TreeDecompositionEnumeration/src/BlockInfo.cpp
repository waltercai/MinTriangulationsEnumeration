#include "BlockInfo.h"

namespace tdenum {

BlockInfo::BlockInfo(const Graph& setG, Block* setB) :
	B(setB), GinducedB(setG, getBlockNodes(setG.getNumberOfNodes(), *setB)),locBySize(-1) { }

BlockInfo::BlockInfo(const SubGraph& setG, Block* setB) :
	B(setB), GinducedB(setG, getBlockNodes(setG.getNumberOfNodes(), *setB)), locBySize(-1){}

int BlockInfo::blockSize() const {
	return B->first.size() + B->second.size();
}

bool BlockInfo::isS(MinimalSeparator& ms) {
	return B->first == ms;
}

bool BlockInfo::isC(NodeSet& ns) {
	return B->second == ns;
}

void BlockInfo::updatePMCs(NodeSetSet& pmcList) {
	NodeSet bNodes = getBlockNodes(GinducedB.getMainNumberOfNodes(), *B);
	for (auto pmc = pmcList.begin(); pmc != pmcList.end(); pmc++)
		if (includes(bNodes.begin(), bNodes.end(),
			pmc->begin(), pmc->end()))
			pmcToBlocks[*pmc] = vector<int>();
}

void BlockInfo::updateLocation(int i, SepToBlockMap& SToB) {
	locBySize = i;

	// For each pmc associated with our current block, and its block list (pmcToB)
	for (auto pmcToB = pmcToBlocks.begin(); pmcToB != pmcToBlocks.end(); pmcToB++) {
		// Calculate all PMC blocks
		vector<Block*> pmcBlocks = GinducedB.getBlocksByMain(pmcToB->first);
		// For each block (pmcBlock) of the PMC
		for (auto pmcBlock = pmcBlocks.begin(); pmcBlock != pmcBlocks.end(); pmcBlock++)
			// For each BlockInfo associated  with pmcBlock's separator (sepBlockInfo)
			for (vector<BlockInfo*>::iterator sepBlockInfo = SToB[(*pmcBlock)->first].begin(); 
				sepBlockInfo != SToB[(*pmcBlock)->first].end(); sepBlockInfo++)
				// If sepBlockInfo.B is the same block as pmcBlock
				if ((*sepBlockInfo)->isC((*pmcBlock)->second)) {
					// Update its location in pmcToBlocks
					pmcToB->second.push_back((*sepBlockInfo)->locBySize);
					break;
				}
	}
}

}