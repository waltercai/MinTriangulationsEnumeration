#ifndef TRIANGULATIONEVALUATOR_H
#define TRIANGULATIONEVALUATOR_H
#include "Graph.h"
#include "SubGraph.h"
#include "DataStructures.h"
#include <set>

#define CONSTRAINT_VIOLATION HUGE_VALF

namespace tdenum {
	/**
	* TODO - write documentation
	*/
	class TriangulationEvaluator {
	protected:
		const Graph& originalGraph;

		vector<const Block*> blockByID;
		vector<float> blockCostByID;

		const Block* curBlock;
		NodeSet curBlockNodes;
		const NodeSet* curBlockBestPMC;
		float curBlockBestCost;

		const NodeSetSet& inclusionConsts;
		const NodeSetSet& exclusionConsts;

		bool curBlockUpholdsConstraints(const NodeSet&, const vector<int>&);

	public:
		TriangulationEvaluator(const Graph& G) :
			originalGraph(G), blockByID(), blockCostByID(),
			curBlock(NULL), curBlockNodes(),  curBlockBestPMC(NULL), 
			inclusionConsts(*(new NodeSetSet())), exclusionConsts(*(new NodeSetSet()))
			{ curBlockBestCost = maxValue(); }

		TriangulationEvaluator(const Graph& G, const NodeSetSet& incConsts, const NodeSetSet& excConsts) :
			originalGraph(G), blockByID(), blockCostByID(), 
			curBlock(NULL), curBlockNodes(), curBlockBestPMC(NULL) , 
			inclusionConsts(incConsts), exclusionConsts(excConsts) { curBlockBestCost = maxValue(); }
		
		virtual TriangulationEvaluator* extendEvaluator(const NodeSetSet&, const NodeSetSet&) = 0;

		const NodeSetSet& getInclusionConsts() { return inclusionConsts; }
		const NodeSetSet& getExclusionConsts() { return exclusionConsts; }

		virtual void finishedCurBlock();
		virtual void startNewBlock(const Block& B);
		virtual void startNewBlock(const Block& B, const SubGraph& GinducedB) { startNewBlock(B); }

		virtual void evalSaturatePMC(const NodeSet& pmc, const vector<int>& pmcBlockIDs);

		virtual const NodeSet* getBestPMC() { return curBlockBestPMC; }
		
		virtual float costSaturatePMC(const NodeSet& pmc, const vector<int>& pmcBlockIDs) = 0;
		virtual float maxValue() { return HUGE_VALF; };
		virtual float getOptimalCost(int) = 0;
	};

	class TriangFillEvaluator : public TriangulationEvaluator {
		int curSFill;
		
		int calcNodeSetFill(const NodeSet&);

	public:
		TriangFillEvaluator(const Graph& G) :
			TriangulationEvaluator(G), curSFill(-1) {}
		TriangFillEvaluator(const Graph& G, const NodeSetSet& incConsts, const NodeSetSet& excConsts) :
			TriangulationEvaluator(G, incConsts, excConsts), curSFill(-1) {}
		
		TriangulationEvaluator* extendEvaluator(const NodeSetSet&, const NodeSetSet&);

		void startNewBlock(const Block& B);
		float costSaturatePMC(const NodeSet& pmc, const vector<int>& pmcBlockIDs);
		float maxValue() { 
			return float(originalGraph.getNumberOfNodes() * (originalGraph.getNumberOfNodes() - 1)/2); 
		}
		float getOptimalCost(int);
	};

	class TriangTreeWidthEvaluator : public TriangulationEvaluator {
	public:
		TriangTreeWidthEvaluator(const Graph& G) : 
			TriangulationEvaluator(G) {}
		TriangTreeWidthEvaluator(const Graph& G, const NodeSetSet& incConsts, const NodeSetSet& excConsts) :
			TriangulationEvaluator(G, incConsts, excConsts) {}

		TriangulationEvaluator* extendEvaluator(const NodeSetSet&, const NodeSetSet&);

		float costSaturatePMC(const NodeSet& pmc, const vector<int>& pmcBlockIDs);
		float maxValue() { return float(originalGraph.getNumberOfNodes() - 1); }

		float getOptimalCost(int);
	};

} /* namespace tdenum */

#endif // TRIANGULATIONEVALUATOR_H
