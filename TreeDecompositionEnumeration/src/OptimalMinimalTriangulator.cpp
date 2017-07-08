
#include "OptimalMinimalTriangulator.h"
#include "PMCEnumerator.h"
#include <queue>

namespace tdenum {

	OptimalMinimalTriangulator::OptimalMinimalTriangulator(const Graph& triangG) :
		g(triangG), allBlockInfos() {

		// Calculate PMCs
		PMCEnumerator pmcEnum(g);
		pmcs = pmcEnum.get();
		minSeps = pmcEnum.get_ms();

		cout << "Finished Calculating PMCs, num PMCS: " << pmcs.size() << ", num MinSeps: " << minSeps.size() << endl;

		calculateBlockInfos();
		cout << "Initialization finished! "<< allBlockInfos.size() << " Blocks!" << endl;
	}

	void OptimalMinimalTriangulator::calculateBlockInfos() {
		// Add all graph blocks to allBlockInfos
		SepToBlockMap sepsToBlocks = calculateSepBlockInfos();

		// sort blocks by size
		sort(allBlockInfos.begin(), allBlockInfos.end(),
			[](BlockInfo* b1, BlockInfo* b2) {
			return (b1->blockSize() < b2->blockSize());
		});

		// Add main component blocks at end of allBlockInfos
		calculateMainBlockInfos();

		// update block locations
		for (unsigned int i = 0; i < allBlockInfos.size(); i++)
			allBlockInfos[i]->updateLocation(i, sepsToBlocks);
	}

	SepToBlockMap OptimalMinimalTriangulator::calculateSepBlockInfos() {
		SepToBlockMap sepsToBlocks;
		for (auto sep = minSeps.begin(); sep != minSeps.end(); sep++) {
			// Find all PMCs which sep is a subset of
			NodeSetSet pmcsContainingSep;
			for (auto pmc = pmcs.begin(); pmc != pmcs.end(); pmc++) {
				if (includes(pmc->begin(), pmc->end(),
					sep->begin(), sep->end()))
					pmcsContainingSep.insert(*pmc);
			}
			// Find all full blocks of sep
			BlockVec sepBlocks = g.getBlocks(*sep);
			sepsToBlocks[*sep] = vector<BlockInfo*>();
			for (auto b = sepBlocks.begin(); b != sepBlocks.end(); b++)
				// Save only full blocks
				if ((*b)->S.size() == sep->size()) {
					BlockInfo* bInfo = new BlockInfo(g, *b);
					bInfo->updatePMCs(pmcsContainingSep);
					allBlockInfos.push_back(bInfo);
					sepsToBlocks[*sep].push_back(bInfo);
				}
		}
		return sepsToBlocks;
	}

	void OptimalMinimalTriangulator::calculateMainBlockInfos() {
		// Add a "block" representing the whole graph so it will be optimized as well
		MinimalSeparator* emptySep = new MinimalSeparator();
		vector<NodeSet> gComps = g.getComponents(*emptySep);

		for (auto comp = gComps.begin(); comp != gComps.end(); comp++) {
			BlockInfo* compBlockInfo = new BlockInfo(g, BlockPtr(new Block(*emptySep, *comp, g.getNumberOfNodes())));
			compBlockInfo->updatePMCs(pmcs);
			allBlockInfos.push_back(compBlockInfo);
		}

		numMainBlocks = gComps.size();
	}

	TriangulationResult OptimalMinimalTriangulator::triangulate(TriangulationEvaluator* eval) {
		// Calculate Omega(S,C) for each block
		vector<const NodeSet*> bestBlockPMCs;
		for (auto bInfo = allBlockInfos.begin(); bInfo != allBlockInfos.end(); bInfo++) {
			eval->startNewBlock((*bInfo)->B);
			for (auto pmcToB = (*bInfo)->pmcToBlocks.begin();
				pmcToB != (*bInfo)->pmcToBlocks.end(); pmcToB++)
				eval->evalSaturatePMC(pmcToB->first, pmcToB->second);
			bestBlockPMCs.push_back(eval->getBestPMC());
		}
		// Make sure eval saves the last block
		eval->finishedCurBlock();

		// Build a chordal graph by filling in the optimal PMCs
		ChordalGraph triang(g);
		NodeSetSet triangSeps;
		queue<int> bestBlockIDs;

		//Start with PMC of last blocks - the ones representing connected components of the graph
		for (int i = 1; i <= numMainBlocks; i++)
			bestBlockIDs.push(allBlockInfos.size() - i);

		while (bestBlockIDs.size() > 0) {
			// Pop the next chosen block for processing
			int curBlockID = bestBlockIDs.front();
			bestBlockIDs.pop();

			const NodeSet* curBlockPMC = bestBlockPMCs[curBlockID];
			triang.addClique(*curBlockPMC);
			triangSeps.insert(allBlockInfos[curBlockID]->B->S);

			// Add this PMCs blocks to be processed
			BlockInfo* curBlock = allBlockInfos[curBlockID];
			for (auto subBlock = curBlock->pmcToBlocks[*curBlockPMC].begin();
				subBlock != curBlock->pmcToBlocks[*curBlockPMC].end(); subBlock++)
				bestBlockIDs.push(*subBlock);
		}

		TriangulationResult res;
		res.triangulation = triang;
		res.minSeps = triangSeps;
		res.cost = eval->getOptimalCost(numMainBlocks);
		res.eval = eval;
		return res;
	}
}
