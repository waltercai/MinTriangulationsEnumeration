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
    ostringstream oss;
    for(auto it = begin(); it != end(); ++it) {
        if (it->size() == 0) {
            oss << "\{}\n";
            continue;
        }
        // *it is now a NodeSet.
        // There's no way to print a NodeSet unless we know it's a vector...
        oss << "\{";
        for(unsigned int j = 0; j < it->size(); ++j) {
            oss << (*it)[j] << ",";
        }
        oss << "\b}\n";
    }
    return oss.str();
}

void NodeSetSet::insert(const vector<Node>& nodeVec) {
	sets.insert(nodeVec);
}

bool NodeSetSet::operator==(const NodeSetSet& nss) const {
    return sets == nss.sets;
}
bool NodeSetSet::operator!=(const NodeSetSet& nss) const {
    return !(*this == nss);
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
		isMember(sizeOfOriginalNodeSet, false){}

void NodeSetProducer::insert(Node v) {
	isMember[v] = true;
}

void NodeSetProducer::remove(Node v) {
	isMember[v] = false;
}

NodeSet NodeSetProducer::produce() {
	NodeSet members;
	for (unsigned int i=0; i<isMember.size(); i++) {
		if (isMember[i]) {
			members.push_back(i);
		}
	}
	return members;
}

} /* namespace tdenum */
