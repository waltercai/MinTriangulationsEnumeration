/*
 * DataStructures.cpp
 *
 *  Created on: Oct 17, 2016
 *      Author: tmp
 */

#include "DataStructures.h"
#include <sstream>

namespace tdenum {

IncreasingWeightNodeQueue::IncreasingWeightNodeQueue(int numberOfNodes) :
		weight(numberOfNodes, 0) {
	vector< pair<int,Node> > temporaryQueue;
	for (Node v = 0; v<numberOfNodes; v++) {
		temporaryQueue.push_back(pair<int,Node> (0,v));
	}
	queue = set< pair<int,Node> >(temporaryQueue.begin(), temporaryQueue.end());
}

void IncreasingWeightNodeQueue::increaseWeight(Node v) {
	queue.erase(pair<int,Node> (weight[v],v));
	weight[v]++;
	queue.insert(pair<int,Node> (weight[v],v));
}

int IncreasingWeightNodeQueue::getWeight(Node v) {
	return weight[v];
}

bool IncreasingWeightNodeQueue::isEmpty() {
	return queue.empty();
}

Node IncreasingWeightNodeQueue::pop() {
	pair<int,Node> current = *queue.rbegin();
	Node v = current.second;
	queue.erase(current);
	return v;
}



bool WeightedNodeSetQueue::isEmpty() {
	return queue.empty();
}

bool WeightedNodeSetQueue::isMember(const vector<Node>& nodeVec, int weight) {
	return queue.find(make_pair(weight, nodeVec)) != queue.end();
}

void WeightedNodeSetQueue::insert(const vector<Node>& nodeVec, int weight) {
	queue.insert(make_pair(weight, nodeVec));
}

vector<Node> WeightedNodeSetQueue::pop() {
	vector<Node> nodeVec = queue.begin()->second;
	queue.erase(queue.begin());
	return nodeVec;
}



bool NodeSetSet::isMember(const vector<Node>& nodeVec) const {
	return sets.find(nodeVec) != sets.end();
}

string NodeSetSet::str() const {
    if (empty()) {
        return string("{}");
    }
    ostringstream oss;
    oss << "\{ ";
    for(auto it = begin(); it != end(); ++it) {
		if (it != begin())
			oss << ",";
		if (it->size() == 0) {
            oss << "\{}";
            continue;
        }
        // *it is now a NodeSet.
        // There's no way to print a NodeSet unless we know it's a vector...
        oss << "\{";
        for(unsigned int j = 0; j < it->size(); ++j) {
            oss << (*it)[j];
			if (j + 1 < it->size())
				oss << ",";
        }
        oss << "}";
    }
    oss << "}";
    return oss.str();
}

ostream& operator<<(ostream& os, const NodeSetSet& nss) {
    os << nss.str();
    return os;
}

void NodeSetSet::insert(const NodeSet& nodeVec) {
	sets.insert(nodeVec);
}
void NodeSetSet::remove(const NodeSet& nodeVec) {
    sets.erase(nodeVec);
}

NodeSetSet NodeSetSet::unify(const NodeSetSet& other) const {
	NodeSetSet result = other;
	for (auto ns = begin(); ns != end(); ns++)
		result.insert(*ns);
	return result;
}

bool NodeSetSet::operator==(const NodeSetSet& nss) const {
    return sets == nss.sets;
}
bool NodeSetSet::operator!=(const NodeSetSet& nss) const {
    return !(*this == nss);
}

unsigned int NodeSetSet::size() const {
    return sets.size();
}
bool NodeSetSet::empty() const {
    return sets.empty();
}
void NodeSetSet::clear() {
    sets.clear();
}

set<NodeSet>::iterator NodeSetSet::begin() const {
    return sets.begin();
}
set<NodeSet>::iterator NodeSetSet::end() const {
    return sets.end();
}
set<NodeSet>::iterator NodeSetSet::find(const NodeSet& nodeSet) const {
    return sets.find(nodeSet);
}



NodeSetProducer::NodeSetProducer(int sizeOfOriginalNodeSet) :
		isMember(sizeOfOriginalNodeSet, false), numMembers(0) {}

void NodeSetProducer::insert(Node v) {
	if (!isMember[v]) {
		isMember[v] = true;
		numMembers++;
	}
}

void NodeSetProducer::remove(Node v) {
	if (isMember[v]) {
		isMember[v] = false;
		numMembers--;
	}
}

NodeSet NodeSetProducer::produce() {
	NodeSet members(numMembers);
	int i;
	unsigned int node;
	for (i=0,node=0; i<numMembers && node<isMember.size(); node++) {
		if (isMember[node])
			members[i++] = node;
	}
	return members;
}

} /* namespace tdenum */
