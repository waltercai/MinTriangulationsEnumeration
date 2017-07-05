/*
 * This class was written by Vladimir Zacharov.
 * It is only used by IndSetExtBySeparators, which provides a way of
 * extending an independent set of non-crossing minimal separators.
 */


#include "SubGraph.h"
#include "DataStructures.h"

namespace tdenum {
	// complexity doesnt matter. used only for debug.
	set<Edge> SubGraph::createEdgeSet() {

		set<Edge> edgeSet;

		vector<int>& nodeIndsInMain = getNodeMapToMainGraph();

		int n = getNumberOfNodes();

		for (int nis = 0; nis < n; nis++) {

			for (int ois = nis + 1; ois < n; ois++) {

				if (areNeighbors(nis, ois)) {
					Edge e(2);
					// get index of the node in main graph

					e[0] = nodeIndsInMain[nis];
					e[1] = nodeIndsInMain[ois];
					if (e[0] > e[1]) {
						int t = e[0];
						e[0] = e[1];
						e[1] = t;
					}

					edgeSet.insert(e);
				}
			}
		}

		return edgeSet;

	}

	set<MinimalSeparator> SubGraph::createNewSepGroup(NodeSet& subNodesInFather,
		const MinimalSeparator& excludeSep,
		const set<MinimalSeparator>& sepsInFather) {
		set<MinimalSeparator> sepsOfSub;

		// nodeInSubNodes will have a true in node index if the node index
		// is in subNodesInFather => the node is in V(c`) group
		vector<bool> nodeInSubNodes = vector<bool>(getNumberOfNodes());
		for (unsigned int i = 0; i < subNodesInFather.size(); i++) {
			nodeInSubNodes[subNodesInFather[i]] = true;
		}

		// mapping of a father node index to sub node index
		vector<int> fatherNodesMapToSub = vector<int>(getNumberOfNodes());
		for (unsigned int i = 0; i < subNodesInFather.size(); i++) {
			fatherNodesMapToSub[subNodesInFather[i]] = i;
		}

		for (const MinimalSeparator& sepInFather : sepsInFather) {

			// if sep == excludeSep => skip
			if (sepInFather == excludeSep) {
				continue;
			}

			// check that all nodes are in V(c`)
			bool allNodesInSubNodes = true;
			for (const Node& nodeInSep : sepInFather) {

				if (!nodeInSubNodes[nodeInSep]) {
					allNodesInSubNodes = false;
					break;
				}
			}

			if (allNodesInSubNodes) {
				// sepInFather is contained V(c`)
				// transform nodes in father to nodes in sub

				MinimalSeparator sepInSub = sepInFather;
				for (unsigned int i = 0; i < sepInSub.size(); i++) {
					sepInSub[i] = fatherNodesMapToSub[sepInFather[i]];
				}

				// insert to set
				sepsOfSub.insert(sepInSub);
			}
		}

		return sepsOfSub;
	}

	set<MinimalSeparator>& SubGraph::getSeps() {
		return seps;
	}

	void SubGraph::setSeps(set<MinimalSeparator>& seps) {
		this->seps = seps;
	}

	SubGraph::SubGraph(const Graph& mainGraph) :
		Graph(mainGraph), mainGraph(mainGraph) {

		nodeMapToMainGraph = vector<int>(mainGraph.getNumberOfNodes());
		nodeMapFromMainGraph = map<Node, Node>();
		for (unsigned int i = 0; i < nodeMapToMainGraph.size(); i++) {
			nodeMapToMainGraph[i] = i;
			nodeMapFromMainGraph[i] = i;
		}
	}

	SubGraph::SubGraph(const SubGraph& fatherGraph, NodeSet nodeSetInFatherGraph) :
		Graph(nodeSetInFatherGraph.size()), mainGraph(fatherGraph.mainGraph) {

		// father in main
		const vector<int>& fatherNodesMapInMain = fatherGraph.nodeMapToMainGraph;

		// node set in father
		NodeSet& nodeSetInFather = nodeSetInFatherGraph;

		nodeMapToMainGraph = vector<int>(nodeSetInFather.size());
		nodeMapFromMainGraph = map<Node, Node>();

		// sub nodes in main
		vector<int>& subNodesInMain = nodeMapToMainGraph;

		vector<int> fatherNodesInSub = vector<int>(fatherGraph.getNumberOfNodes());

		for (unsigned int i = 0; i < fatherNodesInSub.size(); i++) {
			fatherNodesInSub[i] = -1;
		}

		// going through the sub nodes
		for (unsigned int nodeInSub = 0; nodeInSub < nodeSetInFather.size(); nodeInSub++) {

			Node nodeInFather = nodeSetInFather[nodeInSub];

			int nodeInMain = fatherNodesMapInMain[nodeInFather];

			// saves for each node in sub its node index in the main graph
			subNodesInMain[nodeInSub] = nodeInMain;
			nodeMapFromMainGraph[nodeInMain] = nodeInSub;

			// saves for each node in father its nodes in sub if the node
			// is in nodeSetInFatherGraph, otherwise the value in the index will
			// be negative 1
			fatherNodesInSub[nodeInFather] = nodeInSub;
		}

		for (unsigned int nodeInSub = 0; nodeInSub < nodeSetInFather.size(); nodeInSub++) {

			Node nodeInFather = nodeSetInFather[nodeInSub];

			fatherNodesInSub[nodeInFather] = -1;

			const set<Node>& neighbors = fatherGraph.getNeighbors(nodeInFather);

			for (Node neighborInFather : neighbors) {

				// if the neighbor for nodeInSub exists in nodeSetInFatherGraph
				// we add an edge between them

				if (fatherNodesInSub[neighborInFather] >= 0) {
					addEdge(nodeInSub, fatherNodesInSub[neighborInFather]);

				}

			}

		}

	}

	SubGraph::SubGraph(const SubGraph& fatherGraph, NodeSet nodeSetInFatherGraph,
		const set<MinimalSeparator>& seps) :
		SubGraph(fatherGraph, nodeSetInFatherGraph) {

		this->seps = seps;
	}

	SubGraph::SubGraph(const Graph& mainGraph, const set<MinimalSeparator>& seps) :
		SubGraph(mainGraph) {

		this->seps = seps;
	}

	int SubGraph::getMainNumberOfNodes() const {
		return mainGraph.getNumberOfNodes();
	}

	vector<int>& SubGraph::getNodeMapToMainGraph() {

		return nodeMapToMainGraph;
	}

	vector<Block*> SubGraph::getBlocksByMain(const NodeSet& removedNodesMain) const {
		// Translate removedNodes to SubGraph numbers
		NodeSetProducer removedNodesSub(getNumberOfNodes());
		for (auto n = removedNodesMain.begin(); n != removedNodesMain.end(); n++)
			removedNodesSub.insert(nodeMapFromMainGraph.at(*n));

		// Calculate SubGraph blocks
		vector<Block*> subBlocks = getBlocks(removedNodesSub.produce());

		// Translate blocks to Main indexes
		vector<Block*> mainBlocks(subBlocks.size());
		for (int i = 0; i < subBlocks.size(); i++) {
		//for (auto b = subBlocks.begin(); b != subBlocks.end(); b++) {
			NodeSetProducer mainS(mainGraph.getNumberOfNodes()), mainC(mainGraph.getNumberOfNodes());
			for (auto n = subBlocks[i]->S.begin(); n != subBlocks[i]->S.end(); n++)
				mainS.insert(nodeMapToMainGraph.at(*n));
			for (auto n = subBlocks[i]->C.begin(); n != subBlocks[i]->C.end(); n++)
				mainC.insert(nodeMapToMainGraph.at(*n));
			mainBlocks[i] = new Block(mainS.produce(), mainC.produce());
		}
		return mainBlocks;
	}

	void SubGraph::print() const {
		Graph::print();

		for (unsigned int i = 0; i < nodeMapToMainGraph.size(); i++) {

			cout << "[ " << i << " - " << nodeMapToMainGraph[i] << " ]" << endl;
		}
	}
}


