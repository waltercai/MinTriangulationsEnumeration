/*
 * This class was written by Vladimir Zacharov.
 * It is only used by IndSetExtBySeparators, which provides a way of
 * extending an independent set of non-crossing minimal separators.
 */

#ifndef SRC_SUBGRAPH_H_
#define SRC_SUBGRAPH_H_

#include "IndependentSetExtender.h"
#include "Graph.h"
#include <queue>
#include <algorithm>
#include <list>
#include <map>

namespace tdenum {
	typedef vector<Node> Edge;
	class SubGraph : public Graph {
		const Graph& mainGraph;
		vector<int> nodeMapToMainGraph;
		map<Node, Node> nodeMapFromMainGraph;

		set<MinimalSeparator> seps;

	public:

		SubGraph(const SubGraph& fatherGraph, NodeSet nodeSetInFatherGraph);

		// first graph init
		SubGraph(const Graph& mainGraph);

		SubGraph(const SubGraph& fatherGraph, NodeSet nodeSetInFatherGraph,
			const set<MinimalSeparator>& seps);

		SubGraph(const Graph& mainGraph, const set<MinimalSeparator>& seps);

		set<MinimalSeparator>& getSeps();

		// edge set that represents the sub graph in main graph
		// for testing
		set<Edge> createEdgeSet();

		void setSeps(set<MinimalSeparator>& seps);
		
		int getMainNumberOfNodes() const;

		vector<int>& getNodeMapToMainGraph();

		// Given a NodeSet of nodes from the main graph, returns the set
		// of blocks of the given node set in the subgraph
		vector<Block*> getBlocksByMain(const NodeSet& removedNodes) const;

		set<MinimalSeparator> createNewSepGroup(NodeSet& subNodesInFather,
			const MinimalSeparator& excludeSep,
			const set<MinimalSeparator>& sepsInFatherGraph);

		void print() const;

	};
}


#endif /* SRC_SUBGRAPH_H_ */
