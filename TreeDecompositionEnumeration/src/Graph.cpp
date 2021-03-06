#include "Graph.h"
#include "DataStructures.h"
#include "Utils.h"
#include <queue>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <sstream>
#include <algorithm>

namespace tdenum {

Graph::Graph() : Graph(0) {}
Graph::Graph(int numNodes) :
            numberOfNodes(numNodes),
            numberOfEdges(0),
            neighborSets(numberOfNodes),
            isRandomGraph(false),
            p(-1),
            instance(1),
            newToOldNames(numNodes)
{
    for (int i=0; i<numNodes; ++i) {
        newToOldNames[i] = i;
    }
}

Graph& Graph::reset(int n) {
    *this = Graph(n);
    return *this;
}

bool Graph::nodeSanity() const {
    TRACE(TRACE_LVL__WARNING, "Called nodeSanity(), this will take " << numberOfNodes*(numberOfNodes-1)/2 << " steps!");
    for (Node u=0; u<numberOfNodes; ++u) {
        for (Node v=0; v<numberOfNodes; ++v) {
            if (areNeighbors(v,u) != areNeighbors(u,v)) {
                return false;
            }
        }
    }
    return true;
}
#define ASSERT_NODESANE_OR_RET(_ret) do { \
        if (!nodeSanity()) { \
            TRACE(TRACE_LVL__ERROR, "The following graph isn't node-sane:" << endl << *this); \
            return _ret; \
        } \
    } while(0)


Graph& Graph::derandomize() { isRandomGraph = false; return *this; }
Graph& Graph::randomize(double pr, int inst) {

    // Remove all edges
    for (int i=0; i<numberOfNodes; ++i) {
        neighborSets[i].clear();
    }

    NodeSet nodes = getNodesVector();
    double d;
    TRACE(TRACE_LVL__NOISE, "In, p=" << pr);
    double rand_num = pr * RAND_MAX;
    for (int i=0; i<numberOfNodes; ++i) {
        for (int j=i+1; j<numberOfNodes; ++j) {
            d = rand();
            TRACE(TRACE_LVL__NOISE, "Got d=" << (double)d/RAND_MAX);
            if (d <= rand_num) {
                TRACE(TRACE_LVL__NOISE, "Adding edge..");
                addEdge(nodes[i],nodes[j]);
            }
        }
    }

    // I need to know if I'm a random graph!
    declareRandom(pr, inst);

    return *this;
}
Graph& Graph::declareRandom(double pr, int inst) {
    isRandomGraph = true;
    p = pr;
    instance = inst;
    return *this;
}
bool Graph::isRandom() const { return isRandomGraph; }
double Graph::getP() const {
    if (!isRandom()) {
        TRACE(TRACE_LVL__WARNING, "Called getP() on non-random graph!");
    }
    return p;
}
int Graph::getInstance() const { return instance; }

Graph& Graph::removeAllButFirstK(int k) {
    // Remove respective edges from neighbor sets.
    // While doing so, recalculate the number of edges using the formula:
    // sum_v(d(v))=2E
    int E = 0;
    for (int u=0; u<k; ++u) {
        for (int v=k; v<numberOfNodes; ++v) {
            neighborSets[u].erase(v);
        }
        E += neighborSets[u].size();
    }
    // Update the number of nodes and edges
    if (E%2) {
        TRACE(TRACE_LVL__ERROR, "WTF");
    }
    numberOfNodes = k;
    numberOfEdges = E/2;
    return *this;
}

void Graph::composeNewToOld(vector<Node> oldToNew) {
    // newToOldNames now maps from the old names j(i) to i,
    // we want to map them from  oldToNew[j(i)] to i.
    if (oldToNew.size() != newToOldNames.size()) {
        TRACE(TRACE_LVL__ERROR, "ERROR: cannot map " << newToOldNames << " using " << oldToNew << ", size mismatch");
        return;
    }
    TRACE(TRACE_LVL__TEST, "Composing. newToOldNames is " << newToOldNames
                        << ", and the oldToNew vector is " << oldToNew);
    vector<Node> newMap(newToOldNames.size());
    for (unsigned i=0; i<newMap.size(); ++i) {
        newMap[oldToNew[i]] = newToOldNames[i]; // Tested by GraphTester, I swear to god
    }
    newToOldNames = newMap;
    TRACE(TRACE_LVL__TEST, "Result of the composition: " << newToOldNames);
}

// Assumes input map is 1-1 and onto {0,1,...,m.size()-1}
vector<Node> Graph::inverse_map(const vector<Node>& m) const {
    vector<Node> inv(m.size());
    for (unsigned i=0; i<m.size(); ++i) {
        inv[m[i]] = i;
    }
    return inv;
}
vector<Node> Graph::nodeRenameAux(const vector<Node>& oldToNew) {
    vector< set<Node> > newNeighbors(numberOfNodes);
    for (Node u=0; u<numberOfNodes; ++u) {
        for (auto v: neighborSets[u]) {
            newNeighbors[oldToNew[u]].insert(oldToNew[v]);
        }
    }
    neighborSets = newNeighbors;
    composeNewToOld(oldToNew);
    return oldToNew;
}
vector<Node> Graph::randomNodeRename() {
    // Shuffle node names:
    vector<Node> newToOld = getNodesVector();
    std::random_shuffle(newToOld.begin(), newToOld.end());
    // Use f as a mapping between nodes to build the new
    // neighbor sets.
    return nodeRenameAux(inverse_map(newToOld)); // oldToNew
}
vector<Node> Graph::sortNodesByDegree(bool ascending) {
    vector<Node> newToOld = getNodesVector();
    TRACE(TRACE_LVL__DEBUG, "Nodes vector before sort: " << newToOld);
    std::sort(newToOld.begin(), newToOld.end(), NodeCompare(neighborSets, ascending));
    TRACE(TRACE_LVL__DEBUG, "Nodes vector after sort: " << newToOld);
    return nodeRenameAux(inverse_map(newToOld));
}

Node Graph::getOriginalName(Node v) const {
    return getOriginalNames(NodeSet({v}))[0];
}
Node Graph::getNewName(Node v) const {
    return getNewNames(NodeSet({v}))[0];
}
NodeSetSet Graph::getNamesAux(const NodeSetSet& nss, bool get_new) const {
    NodeSetSet original;
    for (auto ns: nss) {
        original.insert(get_new ? getNewNames(ns) : getOriginalNames(ns));
    }
    return original;
}
vector<Node> Graph::getNamesAux(const vector<Node>& nodes, bool get_new) const {
    vector<Node> original;
    vector<Node> nameMap = get_new ? inverse_map(newToOldNames) : newToOldNames;
    for (auto u: nodes) {
        original.push_back(nameMap[u]);
    }
    std::sort(original.begin(), original.end());
    return original;
}
vector<Node> Graph::getOriginalNames(const vector<Node>& nodes) const {
    return getNamesAux(nodes, false);
}
NodeSetSet Graph::getOriginalNames(const NodeSetSet& nss) const {
    return getNamesAux(nss, false);
}
vector<Node> Graph::getNewNames(const vector<Node>& nodes) const {
    return getNamesAux(nodes, true);
}
NodeSetSet Graph::getNewNames(const NodeSetSet& nss) const {
    return getNamesAux(nss, true);
}
Graph& Graph::forgetOriginalNames() {
    for(unsigned i=0; i<newToOldNames.size(); ++i) {
        newToOldNames[i] = i;
    }
    return *this;
}

Graph& Graph::addClique(const set<Node>& newClique) {
	for (set<Node>::iterator i = newClique.begin(); i != newClique.end(); ++i) {
		Node v = *i;
		for (set<Node>::iterator j = newClique.begin(); j != newClique.end(); ++j) {
			Node u = *j;
			if (u < v) {
				addEdge(u, v);
			}
		}
	}
    return *this;
}

Graph& Graph::addClique(const vector<Node>& newClique) {
	for (vector<Node>::const_iterator i = newClique.begin(); i != newClique.end(); ++i) {
		Node v = *i;
		for (vector<Node>::const_iterator j = newClique.begin(); j != newClique.end(); ++j) {
			Node u = *j;
			if (u < v) {
				addEdge(u, v);
			}
		}
	}
    return *this;
}

Graph& Graph::addEdge(Node u, Node v) {
	if (!isValidNode(u) || !isValidNode(v) || neighborSets[u].count(v)>0) {
		return *this;
	}
	neighborSets[u].insert(v);
	neighborSets[v].insert(u);
	numberOfEdges++;
    return *this;
}

Graph& Graph::saturateNodeSets(const set< set<Node> >& s) {
	for (set< set<Node> >::iterator i = s.begin(); i != s.end(); ++i) {
		addClique(*i);
	}
    return *this;
}

// Adds edges that will make the given node sets cliques
Graph& Graph::saturateNodeSets(const set< vector<Node> >& s) {
	for (set< vector<Node> >::iterator i = s.begin(); i != s.end(); ++i) {
		addClique(*i);
	}
    return *this;
}

bool Graph::isValidNode(Node v) const {
	if (v<0 || v>=numberOfNodes) {
		TRACE(TRACE_LVL__ERROR, "Invalid node '" << v << "'");
		return false;
	}
	return true;
}


/*
 * Returns the set of nodes in the graph
 */
set<Node> Graph::getNodes() const {
	set<Node> nodes;
	for (Node i=0; i<numberOfNodes; i++) {
		nodes.insert(i);
	}
	return nodes;
}

/*
 * Returns the vector of nodes in the graph
 */
vector<Node> Graph::getNodesVector() const {
	vector<Node> nodes;
	for (Node i=0; i<numberOfNodes; i++) {
		nodes.insert(nodes.end(), i);
	}
	return nodes;
}

/*
 * Returns the number of edges in the graph
 */
int Graph::getNumberOfEdges() const {
	return numberOfEdges;
}

/*
 * Returns the number of nodes in the graph
 */
int Graph::getNumberOfNodes() const {
	return numberOfNodes;
}

double Graph::getEdgeRatio() const {
    if (getNumberOfNodes() <= 1) {
        return 0;
    }
    double N = (double)getNumberOfNodes();
    double M = (double)getNumberOfEdges();
    return 2*M / (N*(N-1));
}

int Graph::d(Node v) const {
    if (!isValidNode(v)) {
        cout << "Error: requesting degree of invalid node" << endl;
        return -1;
    }
    return neighborSets[v].size();
}
/*
 * Returns the set of neighbors of the given node
 */
const set<Node>& Graph::getNeighbors(Node v) const {
	if (!isValidNode(v)) {
		cout << "Error: Requesting access to invalid node" << endl;
		return neighborSets[0];
	}
	return neighborSets[v];
}

vector<bool> Graph::getNeighborsMap(Node v) const {
	vector<bool> result(numberOfNodes, false);
	for (set<Node>::iterator j = neighborSets[v].begin(); j != neighborSets[v].end(); ++j) {
		result[*j] = true;
	}
	return result;
}

/*
 * Returns the set of neighbors of nodes in the given node set without returning
 * nodes that are in the input node set
 */
NodeSet Graph::getNeighbors(const set<Node>& inputSet) const {
	NodeSetProducer neighborsProducer(numberOfNodes);
	for (set<Node>::const_iterator i = inputSet.begin(); i != inputSet.end(); ++i) {
		Node v = *i;
		if (!isValidNode(v)) {
			return NodeSet();
		}
		for (set<Node>::iterator j = neighborSets[v].begin(); j != neighborSets[v].end(); ++j) {
			neighborsProducer.insert(*j);
		}
	}
	for (set<Node>::const_iterator i = inputSet.begin(); i != inputSet.end(); ++i) {
		neighborsProducer.remove(*i);
	}
	return neighborsProducer.produce();
}

NodeSet Graph::getNeighbors(const vector<Node>& inputSet) const {
	NodeSetProducer neighborsProducer(numberOfNodes);
	for (vector<Node>::const_iterator i = inputSet.begin(); i != inputSet.end(); ++i) {
		Node v = *i;
		if (!isValidNode(v)) {
			return NodeSet();
		}
		for (set<Node>::iterator j = neighborSets[v].begin(); j != neighborSets[v].end(); ++j) {
			neighborsProducer.insert(*j);
		}
	}
	for (vector<Node>::const_iterator i = inputSet.begin(); i != inputSet.end(); ++i) {
		neighborsProducer.remove(*i);
	}
	return neighborsProducer.produce();
}

bool Graph::areNeighbors(Node u, Node v) const {
	return neighborSets[u].find(v) != neighborSets[u].end();
}

vector<NodeSet> Graph::getComponents(const set<Node>& removedNodes) const {
	vector<int> visitedList(numberOfNodes, 0);
	for (set<Node>::iterator i = removedNodes.begin(); i != removedNodes.end(); ++i) {
		Node v = *i;
		if (!isValidNode(v)) {
			return vector<NodeSet>();
		}
		visitedList[v] = -1;
	}
	int numberOfUnhandeledNodes = numberOfNodes - removedNodes.size();
	return getComponentsAux(visitedList, numberOfUnhandeledNodes);
}

vector<NodeSet> Graph::getComponents(const NodeSet& removedNodes) const {
	vector<int> visitedList(numberOfNodes, 0);
	for (Node v : removedNodes) {
		if (!isValidNode(v)) {
			return vector<NodeSet>();
		}
		visitedList[v] = -1;
	}
	int numberOfUnhandeledNodes = numberOfNodes - removedNodes.size();
	return getComponentsAux(visitedList, numberOfUnhandeledNodes);
}

vector<NodeSet> Graph::getComponentsAux(vector<int> visitedList, int numberOfUnhandeledNodes) const {
	vector<NodeSet> components;
	// Finds a new component in each iteration
	while (numberOfUnhandeledNodes > 0) {
		queue<Node> bfsQueue;
		NodeSetProducer componentProducer(visitedList.size());
		// Initialize the queue to contain a node not handled
		for (Node i=0; i<numberOfNodes; i++) {
			if (visitedList[i]==0) {
				bfsQueue.push(i);
				visitedList[i]=1;
				componentProducer.insert(i);
				numberOfUnhandeledNodes--;
				break;
			}
		}
		// BFS through the component
		while (!bfsQueue.empty()) {
			Node v = bfsQueue.front();
			bfsQueue.pop();
			for (set<Node>::iterator i = neighborSets[v].begin();
					i != neighborSets[v].end(); ++i) {
				Node u = *i;
				if (visitedList[u]==0) {
					bfsQueue.push(u);
					visitedList[u]=1;
					componentProducer.insert(u);
					numberOfUnhandeledNodes--;
				}
			}
		}
		components.push_back(componentProducer.produce());
	}
	return components;
}

bool Graph::isFullComponent(const vector<Node>& C, const vector<Node>& S) const {
    for (auto s: S) {
        bool has_neighbor = false;
        for (auto c: C) {
            if (neighborSets[s].find(c) != neighborSets[s].end()) {
                has_neighbor = true;
                break;
            }
        }
        if (!has_neighbor) {
            return false;
        }
    }
    return true;
}

vector<int> Graph::getComponentsMap(const vector<Node>& removedNodes) const {
	vector<int> visitedList(numberOfNodes, 0);
	for (vector<Node>::const_iterator i = removedNodes.begin(); i != removedNodes.end(); ++i) {
		Node v = *i;
		if (!isValidNode(v)) {
			return vector<int>();
		}
		visitedList[v] = -1;
	}
	int numberOfUnhandeledNodes = numberOfNodes - removedNodes.size();
	int currentComponent = 1;
	while (numberOfUnhandeledNodes > 0) {
			queue<Node> bfsQueue;
			// Initialize the queue to contain a node not handled
			for (Node i=0; i<numberOfNodes; i++) {
				if (visitedList[i]==0) {
					bfsQueue.push(i);
					visitedList[i]=currentComponent;
					numberOfUnhandeledNodes--;
					break;
				}
			}
			// BFS through the component
			while (!bfsQueue.empty()) {
				Node v = bfsQueue.front();
				bfsQueue.pop();
				for (set<Node>::iterator i = neighborSets[v].begin();
						i != neighborSets[v].end(); ++i) {
					Node u = *i;
					if (visitedList[u]==0) {
						bfsQueue.push(u);
						visitedList[u]=currentComponent;
						numberOfUnhandeledNodes--;
					}
				}
			}
			currentComponent++;
		}
	return visitedList;
}

// Returns the nodes reachable from v after removing removedNodes.
// Uses a BFS from v, where nodes in removedNodes are not processed.
set<Node> Graph::getComponent(Node v, const set<Node>& removedNodes) {
	queue<Node> q;
	vector<bool> insertedNodes = vector<bool>(numberOfNodes);
	set<Node> component;

	// Mark removedNodes as inserted to avoid processing them
	for (Node removed : removedNodes) {
		insertedNodes[removed] = true;
	}

	// Initialize the BFS with v
	component.insert(v);
	q.push(v);
	insertedNodes[v] = true;
	// BFS through the component
	while (!q.empty()) {
		v = q.front();
		q.pop();
		const set<Node>& neighbors = getNeighbors(v);
		for (Node neighbor : neighbors) {
			if (!insertedNodes[neighbor]) {
				q.push(neighbor);
				insertedNodes[neighbor] = true;
				component.insert(neighbor);
			}
		}
	}
	return component;
}


NodeSet Graph::getAdjacent(const NodeSet& C, const NodeSet& K) const {
    NodeSet K2;
    auto adjacent = getNeighbors(C);
    std::set_intersection(adjacent.begin(), adjacent.end(),
                        K.begin(), K.end(),
                        std::back_inserter(K2));
    return K2;
}

BlockVec Graph::getBlocks(const set<Node>& removedNodes) const {
	vector<int> visitedList(numberOfNodes, 0);
	for (set<Node>::iterator i = removedNodes.begin(); i != removedNodes.end(); ++i) {
		Node v = *i;
		if (!isValidNode(v)) {
			return BlockVec();
		}
		visitedList[v] = -1;
	}
	int numberOfUnhandeledNodes = numberOfNodes - removedNodes.size();
	return getBlocksAux(visitedList, numberOfUnhandeledNodes);
}

BlockVec Graph::getBlocks(const NodeSet& removedNodes) const {
	vector<int> visitedList(numberOfNodes, 0);
	for (Node v : removedNodes) {
		if (!isValidNode(v)) {
			return BlockVec();
		}
		visitedList[v] = -1;
	}
	int numberOfUnhandeledNodes = numberOfNodes - removedNodes.size();
	return getBlocksAux(visitedList, numberOfUnhandeledNodes);
}

BlockVec Graph::getBlocksAux(vector<int> visitedList, int numberOfUnhandeledNodes) const {
	BlockVec blocks;
	// Finds a new component in each iteration
	Node unhandeledID = 0;
	while (numberOfUnhandeledNodes > 0) {
		queue<Node> bfsQueue;
		NodeSetProducer sepProducer(visitedList.size()),
						compProducer(visitedList.size());
		// Initialize the queue to contain a node not handled
		for (; unhandeledID<numberOfNodes; unhandeledID++) {
			if (visitedList[unhandeledID] == 0) {
				bfsQueue.push(unhandeledID);
				visitedList[unhandeledID] = 1;
				compProducer.insert(unhandeledID);
				numberOfUnhandeledNodes--;
				unhandeledID++;
				break;
			}
		}
		// BFS through the component
		while (!bfsQueue.empty()) {
			Node v = bfsQueue.front();
			bfsQueue.pop();
			for (set<Node>::iterator it = neighborSets[v].begin();
				it != neighborSets[v].end(); ++it) {
				Node u = *it;
				if (visitedList[u] == 0) {
					bfsQueue.push(u);
					visitedList[u] = 1;
					compProducer.insert(u);
					numberOfUnhandeledNodes--;
				}
				else if (visitedList[u] == -1) {
					sepProducer.insert(u);
				}
			}
		}
		blocks.push_back(BlockPtr(new Block(sepProducer.produce(), compProducer.produce(),getNumberOfNodes())));
	}
	return blocks;
}
string Graph::str() const {
    ostringstream oss;
	for (Node v=0; v<getNumberOfNodes(); v++) {
        auto neighbors = getNeighbors(v);
		oss << v << " has neighbors: {";
		for (set<Node>::iterator jt = neighbors.begin(); jt!=neighbors.end(); ++jt) {
			oss << *jt << ",";
		}
        if (neighbors.size() > 0) {
            oss << "\b"; // Remove trailing space
        }
		oss << "}" << endl;
	}
	return oss.str();
}
void Graph::print() const { cout << *this; }
ostream& operator<<(ostream& os, const Graph& g) { return (os << g.str()); }

const NodeSet Block::getNodeSetUnion(const NodeSet& sep, const NodeSet& comp) {
	NodeSet result(sep.size() + comp.size());
	std::set_union(sep.begin(), sep.end(),
		comp.begin(), comp.end(),
		result.begin());
	return result;
}

const vector<bool> Block::getFullNodeVector(const NodeSet& nodes, int numNodes) {
	vector<bool> result(numNodes, false);
	for (auto n = nodes.begin(); n != nodes.end(); n++)
		result[*n] = true;
	return result;
}

bool Block::includesNodes(const NodeSet& toCheck) const {
	for (auto n = toCheck.begin(); n != toCheck.end(); n++)
		if (!fullNodes[*n])
			return false;
	return true;
}



Graph::NodeCompare::NodeCompare(const vector< set<Node> >& neighborSets, bool ascending) :
        ns(neighborSets),
        asc(ascending) {}
bool Graph::NodeCompare::operator()(Node a, Node b) const {
    return asc ?
        ns[a].size() < ns[b].size() :
        ns[a].size() > ns[b].size();
}
bool Graph::operator==(const Graph& g) const {
    return neighborSets == g.neighborSets; // Good enough
}
bool Graph::operator!=(const Graph& g) const {
    return !(*this == g);
}

} /* namespace tdenum */

