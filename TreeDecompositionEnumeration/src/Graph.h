#ifndef GRAPH_H_
#define GRAPH_H_

#include "DataStructures.h"
#include <set>
#include <vector>
#include <iostream>
#include <string>
#include <iterator>
#include <memory>
using namespace std;

namespace tdenum {

/**
 * When using node renaming methods (sorting, random rename..) note
 * that graph structures (PMCs, separators) also have their names changed!
 *
 * To translate back / forth, use getOriginalName or getNewName methods
 * provided.
 */
class Graph {

    // Allow the tester access
    friend class GraphTester;

	int numberOfNodes;
	int numberOfEdges;
	vector< set<Node> > neighborSets;

	// If the graph is randomized, update these
	bool isRandomGraph;
	double p;
	int instance;

	// If node names were permuted, keep a mapping from new node
	// names to old ones.
	// Useful when taking node sets processed after node rename, when we want
	// the names of the nodes in the original graph.
	vector<Node> newToOldNames;
	void composeNewToOld(vector<Node> oldToNew);

	bool isValidNode(Node v) const;
	vector< vector<Node> > getComponentsAux(vector<int> visitedList, int numberOfUnhandeledNodes) const;
	BlockVec getBlocksAux(vector<int> visitedList, int numberOfUnhandeledNodes) const;

	// Used by node renaming / sorting methods.
	// nodeRenameAux returns a mapping from old names to new.
	vector<Node> inverse_map(const vector<Node>&) const;
	vector<Node> nodeRenameAux(const vector<Node>& mapping);
	vector<Node> getNamesAux(const vector<Node>&, bool get_new) const;
	NodeSetSet getNamesAux(const NodeSetSet&, bool get_new) const;
    class NodeCompare {
        vector< set<Node> > ns;
        bool asc;
    public:
        NodeCompare(const vector< set<Node> >&, bool ascending);
        bool operator()(Node a, Node b) const;
    };

    // Returns true <==> (u is v's neighbor <==> v is u's neighbor)
    bool nodeSanity() const;

public:
	// Constructs an empty graph
	Graph();
	// Constructs a graph with nodes and without edges
	Graph(int numberOfNodes);
	// Resets the graph (call constructor again)
	Graph& reset(int numberOfNodes = 0);
	// Removes all but the first k nodes from the graph/
	Graph& removeAllButFirstK(int k);

	// Given a graph with n vertices and no edges, creates a random graph
	// from G(p,n).
	// Assumes the user has called srand()
	Graph& derandomize();   // Declares the graph non-random
	Graph& randomize(double pr, int inst = 1);
	Graph& declareRandom(double pr, int inst = 1); // Indicate the graph was constructed as a random graph manually
	// Used to rename nodes (sort them) randomly or by degree.
	// Returns the mapping from old names to new.
	vector<Node> randomNodeRename();
	vector<Node> sortNodesByDegree(bool ascending);
	// Returns the original node names (in a sorted vector) given the new names,
	// or the new names (sorted) given the old.
	Node getOriginalName(Node v) const;
	Node getNewName(Node v) const;
	vector<Node> getOriginalNames(const vector<Node>&) const;
	NodeSetSet getOriginalNames(const NodeSetSet&) const;
	vector<Node> getNewNames(const vector<Node>&) const;
	NodeSetSet getNewNames(const NodeSetSet&) const;
	// If this is called, the current node names are treated as
	// the original node names
	Graph& forgetOriginalNames();

	// Connects the given two nodes by a edge
	Graph& addEdge(Node u, Node v);
	// Adds edges that will make that given node set a clique
	Graph& addClique(const set<Node>& s);
	// Adds edges that will make that given node set a clique
	Graph& addClique(const vector<Node>& s);
	// Adds edges that will make the given node sets cliques
	Graph& saturateNodeSets(const set< set<Node> >& s);
	// Adds edges that will make the given node sets cliques
	Graph& saturateNodeSets(const set< vector<Node> >& s);

	// Returns the nodes of the graph
	set<Node> getNodes() const;
	// Returns the nodes of the graph in a vector
	vector<Node> getNodesVector() const;
	// Returns the number of edges in the graph
	int getNumberOfEdges() const;
	// Returns the number of nodes in the graph
	int getNumberOfNodes() const;
	// Returns m/(n choose 2), or 0 if n<=1
	double getEdgeRatio() const;
	// Self explanatory.
	// getP() alerts user if called when isRandom() is false
	bool isRandom() const;
	double getP() const;
	int getInstance() const;
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
	vector<NodeSet> getComponents(const NodeSet& removedNodes) const;
	// Returns true <=> all nodes in C are neighbors of some element in S.
	// Note: assumes C is a connected component, and DOES NOT enforce it.
	bool isFullComponent(const vector<Node>& C, const vector<Node>& S) const;
	// Returns all the nodes reachable from the node after removing
	// the removedNodes from the graph
	set<Node> getComponent(Node v, const set<Node>& removedNodes);
	// Given a node set K, returns a subset K' of K that are connected
	// to some node in C.
	NodeSet getAdjacent(const NodeSet& C, const NodeSet& K) const;
	// Returns the set of blocks of the given node set
	BlockVec getBlocks(const set<Node>& removedNodes) const;
	BlockVec getBlocks(const NodeSet& removedNodes) const;
	// Prints the graph
	string str() const;
	void print() const;
	friend ostream& operator<<(ostream&, const Graph&);
	bool operator==(const Graph&) const;
	bool operator!=(const Graph&) const;
};



} /* namespace tdenum */

#endif /* GRAPH_H_ */
