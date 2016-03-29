#include "CGRANode.h"

void CGRANode::addConnectedNode(CGRANode* node, int cost, std::string name) {
	ConnectedCGRANode temp(node,cost,name);
	connectedNodes.push_back(temp);
}

void CGRANode::setMappedDFGNode(dfgNode* node) {
	mappedDFGNode = node;
}

dfgNode* CGRANode::getmappedDFGNode() {
	return mappedDFGNode;
}

void CGRANode::sortConnectedNodes() {
	std::sort(connectedNodes.begin(),connectedNodes.end(),less_than_cost());
}

void CGRANode::setX(int x) {
	this->x = x;
}

void CGRANode::setY(int y) {
	this->y = y;
}

void CGRANode::setT(int t) {
	this->t = t;
}

int CGRANode::getX() {
	return x;
}

int CGRANode::getY() {
	return y;
}

CGRANode::CGRANode(int x, int y, int t) {
	setX(x);
	setY(y);
	setT(t);
}

int CGRANode::getT() {
	return t;
}

std::vector<ConnectedCGRANode> CGRANode::getConnectedNodes() {
	return connectedNodes;
}
