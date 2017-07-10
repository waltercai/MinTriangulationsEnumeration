#include "TriangulationEvaluator.h"

namespace tdenum {
	TriangulationEvaluator::~TriangulationEvaluator() {
		blockByID.clear();
		curBlock = NULL;
		curBlockBestPMC = NULL;
	}

	void TriangulationEvaluator::resizeByNumBlocks(int numBlocks) {
		// If we already started calculating blocks, don't touch the vectors!
		if (curBlockID > 0)
			return;
		blockByID.resize(numBlocks, NULL);
		blockCostByID.resize(numBlocks, maxValue());
	}

	void TriangulationEvaluator::startNewBlock(const Block& B) {
		if (curBlock != NULL) {
			finishedCurBlock();
			curBlockID++;
		}
		curBlock = &B;
		curBlockBestPMC = NULL;
		curBlockBestCost = maxValue();
	}
	
	void TriangulationEvaluator::finishedCurBlock() {
		blockByID[curBlockID] = curBlock;
		blockCostByID[curBlockID] = curBlockBestCost;
	}

	void TriangulationEvaluator::evalSaturatePMC(const NodeSet& pmc, const vector<int>& pmcBlockIDs) {
		float curCost = curBlockUpholdsConstraints(pmc, pmcBlockIDs)? 
			costSaturatePMC(pmc, pmcBlockIDs) : CONSTRAINT_VIOLATION;
		
		// If this PMC has a lower cost than previouse ones, save it as best 
		if (curCost < curBlockBestCost || curBlockBestPMC == NULL) {
			curBlockBestPMC = &pmc;
			curBlockBestCost = curCost;
		}
	}

	bool TriangulationEvaluator::curBlockUpholdsConstraints(const NodeSet& pmc, const vector<int>& pmcBlockIDs) {
		// If current blocks S is in the exclusion list, 
		// this block can't be part of a legal triangulation
		if (exclusionConsts.isMember(curBlock->S))
			return false;

		// Iterate once over PMCs sub blocks
		for (auto bID = pmcBlockIDs.begin(); bID != pmcBlockIDs.end(); bID++)
			// If the sub block already violates some constraints, 
			// then so does this block with this PMC filled
			if (blockCostByID[*bID] == CONSTRAINT_VIOLATION)
				return false;

		// Check inclusion constraints are upheld
		for (auto constraint = inclusionConsts.begin();
			constraint != inclusionConsts.end(); constraint++) {
			// If constraint is not contained in this block it is irrelevant
			if (!(curBlock->includesNodes(*constraint)))
				continue;

			// If constraint is contained in the current PMC
			if (includes(pmc.begin(), pmc.end(), constraint->begin(), constraint->end()))
				// It is upheld by filling in this PMC
				// (saturating a minimal separator of G makes it a minimal separator of H)
				continue;

			// If the constraint is contained in some block it has already been checked
			bool containedInBlock = false;
			// Check if the constraint is contained in some block
			for (auto bID = pmcBlockIDs.begin(); bID != pmcBlockIDs.end(); bID++)
				if (blockByID[*bID]->includesNodes(*constraint)) {
					containedInBlock = true;
					break;
				}

			// If the constraint is not contained in any block and not in the PMC,
			// it can't be a minimal separator in the result, meaning it is violated.
			if (!containedInBlock)
				return false;
		}
			
		// Check exclusion constraints are upheld
		for (auto constraint = exclusionConsts.begin();
			constraint != exclusionConsts.end(); constraint++) {
			// If constraint is contained in the current PMC
			if (includes(pmc.begin(), pmc.end(), constraint->begin(), constraint->end()))
				// It is violated by filling in this PMC
				// (saturating a minimal separator of G makes it a minimal separator of H)
				return false;

			// Otherwise we have one of three cases:
			// 1. It is not contained in the current block and therefore irrelevant
			// 2. It is contained in some sub block and therefore has already been checked.
			// 3. It is contained in the block but not in the PMC or another sub block,
			//    therefore it is not a minimal separator in H and is not violated.
			// For any of these options, we know the constraint is not violated.
		}

		// If we got here all constraints are upheld
		return true;
	}

	TriangulationEvaluator* TriangFillEvaluator::extendEvaluator(const NodeSetSet& newIncs, const NodeSetSet& newExcs) {
		TriangulationEvaluator* extended = new TriangFillEvaluator(originalGraph, inclusionConsts.unify(newIncs), exclusionConsts.unify(newExcs));
		extended->resizeByNumBlocks(blockByID.size());
		return extended;
	}

	void TriangFillEvaluator::startNewBlock(const Block& B) {
		TriangulationEvaluator::startNewBlock(B);
		curSFill = calcNodeSetFill(B.S);
	}

	float TriangFillEvaluator::costSaturatePMC(const NodeSet& pmc, const vector<int>& pmcBlockIDs) {
		// Calculate the fill of the block triangulation with this pmc
		// without filling in S (like in Bouchitte and Todinca paper)
		float curCost = calcNodeSetFill(pmc) - curSFill;
		for (auto i = pmcBlockIDs.begin(); i != pmcBlockIDs.end(); i++)
			curCost += blockCostByID[*i];

		return curCost;
	}

	int TriangFillEvaluator::calcNodeSetFill(const NodeSet& ns) {
		int fill = 0;
		for (auto n1 = ns.begin(); n1 != ns.end(); n1++)
			for (auto n2 = n1 + 1; n2 != ns.end(); n2++)
				if (!originalGraph.areNeighbors(*n1, *n2))
					fill++;
		return fill;
	}

	float TriangFillEvaluator::getOptimalCost(int numGraphComponents) {
		// The result fill is the sum of the fills of each main graph component
		// These should be the last numGraphComponents blocks processed.
		float answerFill = 0;
		for (int i = 1; i <= numGraphComponents; i++)
			answerFill += blockCostByID[blockCostByID.size() - i];
		return answerFill;
	}

	TriangulationEvaluator* TriangTreeWidthEvaluator::extendEvaluator(const NodeSetSet& newIncs, const NodeSetSet& newExcs) {
		TriangulationEvaluator* extended = new TriangTreeWidthEvaluator(originalGraph, inclusionConsts.unify(newIncs), exclusionConsts.unify(newExcs));
		extended->resizeByNumBlocks(blockByID.size());
		return extended;
	}

	float TriangTreeWidthEvaluator::costSaturatePMC(const NodeSet& pmc, const vector<int>& pmcBlockIDs) {
		// Calculate tree width of block triangulation with this pmc
		float curCost = pmc.size() - 1;
		for (auto i = pmcBlockIDs.begin(); i != pmcBlockIDs.end(); i++)
			if (blockCostByID[*i] > curCost)
				curCost = blockCostByID[*i];

		return curCost;
	}

	float TriangTreeWidthEvaluator::getOptimalCost(int numGraphComponents) {
		// The result width is the maximum between the widths of each main graph component
		// These should be the last numGraphComponents blocks processed.
		float answerWidth = blockCostByID[blockCostByID.size() - 1];
		for (int i = 2; i <= numGraphComponents; i++)
			if (blockCostByID[blockCostByID.size() - i] > answerWidth)
				answerWidth = blockCostByID[blockCostByID.size() - i];
		return answerWidth;
	}

}