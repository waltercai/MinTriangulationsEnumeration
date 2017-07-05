#ifndef GRAPH_H_
#define GRAPH_H_

#include <set>
#include <vector>
#include <iostream>
#include <string>
#include <iterator>
using namespace std;

namespace tdenum {

typedef int Node;
typedef vector<Node> NodeSet; // sorted vector of node names
typedef NodeSet MinimalSeparator;
typedef pair<MinimalSeparator, NodeSet> Block;

string str(const NodeSet&);
void print(const NodeSet&);


class Graph {
	int numberOfNodes;
	int numberOfEdges;
	vector< set<Node> > neighborSets;

	// If the graph is randomized, update these
	bool isRandomGraph;
	double p;

	bool isValidNode(Node v) const;
	vector< vector<Node> > getComponentsAux(vector<int> visitedList, int numberOfUnhandeledNodes) const;
	vector< Block* > getBlocksAux(vector<int> visitedList, int numberOfUnhandeledNodes) const;

public:
	// Constructs an empty graph
	Graph();
	// Constructs a graph with nodes and without edges
	Graph(int numberOfNodes);
	// Resets the graph (call constructor again)
	void reset(int numberOfNodes = 0);
	// Given a graph with n vertices and no edges, creates a random graph
	// from G(p,n).
	// Assumes the user has called srand()
	void randomize(double p);
	// Removes all but the first k nodes from the graph/
	void removeAllButFirstK(int k);
	// Randomly renames all nodes (this only affects the neighbor sets)
	void randomNodeRename();
	// Connects the given two nodes by a edge
	void addEdge(Node u, Node v);
	// Adds edges that will make that given node set a clique
	void addClique(const set<Node>& s);
	// Adds edges that will make that given node set a clique
	void addClique(const vector<Node>& s);
	// Adds edges that will make the given node sets cliques
	void saturateNodeSets(const set< set<Node> >& s);
	// Adds edges that will make the given node sets cliques
	void saturateNodeSets(const set< vector<Node> >& s);

	// Returns the nodes of the graph
	set<Node> getNodes() const;
	// Returns the nodes of the graph in a vector
	vector<Node> getNodesVector() const;
	// Returns the number of edges in the graph
	int getNumberOfEdges() const;
	// Returns the number of nodes in the graph
	int getNumberOfNodes() const;
	// Self explanatory
	bool isRandom() const;
	double getP() const;
	// Returns the degree of the node. Returns -1 on invalid node
	int d(Node i) const;
	// Returns the neighbors of the given node
	const set<Node>& getNeighbors(Node v) const;
	// Returns a vector the size of the number of nodes in the graph, stating
	// whether the index nodes are neighbors of the input node.
	vector<bool> getNeighborsMap(Node v) const;
	// Returns the neighbors of the given node set
	NodeSet getNeighbors(const vector<Node>& s) const;
	// Returns the neighbors of the given node set
	NodeSet getNeighbors(const set<Node>& s) const;
	// Returns whether there is an edge between the given two nodes
	bool areNeighbors(Node u, Node v) const;
	// Returns a map from the Nodes numbers to the number of the component they are in.
	// -1 if they are in the input set.
	vector<int> getComponentsMap(const vector<Node>& removedNodes) const;
	// Returns the set of components obtained by removing the given node set
	vector<NodeSet> getComponents(const set<Node>& removedNodes) const;
	// Returns the set of components obtained by removing the given node set
	vector<NodeSet> getComponents(const NodeSet& removedNodes) const;
	// Returns all the nodes reachable from the node after removing
	// the removedNodes from the graph
	set<Node> getComponent(Node v, const set<Node>& removedNodes);
	// Given a node set K, returns a subset K' of K that are connected
	// to some node in C.
	NodeSet getAdjacent(const NodeSet& C, const NodeSet& K) const;
	// Returns the set of blocks of the given node set
	vector<Block*> getBlocks(const set<Node>& removedNodes) const;
	vector<Block*> getBlocks(const NodeSet& removedNodes) const;
	// Prints the graph
	string str() const;
	void print() const;
	friend ostream& operator<<(ostream&, const Graph&);
};

} /* namespace tdenum */

#endif /* GRAPH_H_ */
