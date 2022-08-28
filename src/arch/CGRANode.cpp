#include <morpherdfggen/arch/CGRANode.h>

#include <morpherdfggen/arch/CGRA.h>

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

CGRANode::CGRANode(int x, int y, int t, CGRA* ParentCGRA) {
	setX(x);
	setY(y);
	setT(t);
	Parent = ParentCGRA;
}

int CGRANode::getT() {
	return t;
}

std::string CGRANode::getNameWithOutTime() {
	return "(" + std::to_string(y) + "," + std::to_string(x) + ")";
}

std::vector<ConnectedCGRANode> CGRANode::getConnectedNodes() {
	return connectedNodes;
}

std::string CGRANode::getName() {
	return "(" + std::to_string(t) + "," + std::to_string(y) + "," + std::to_string(x) + ")";
}

std::string CGRANode::getNameSp() {
	return "(" + std::to_string(t) + "/" + std::to_string(y) + "/" + std::to_string(x) + ")";
}

bool CGRANode::equals(int tt, int yy, int xx) {
	if((tt == this->t) && (yy == this->y) && (xx == this->x)){
		return true;
	}
	else{
		return false;
	}
}

bool CGRANode::isCorner() {
	if((x==0)&&(y==0)){
		return true;
	}
	if((x==0)&&(y==Parent->getYdim()-1)){
		return true;
	}
	if((x==Parent->getXdim()-1)&&(y==0)){
		return true;
	}
	if((x==Parent->getXdim()-1)&&(y==Parent->getYdim()-1)){
		return true;
	}
	return false;
}

bool CGRANode::isBoundary() {
	if((x==0)||(x==Parent->getXdim()-1)){
		return true;
	}
	if((y==0)||(y==Parent->getYdim()-1)){
		return true;
	}
	return false;
}

bool CGRANode::isMiddle() {
	return !isBoundary();
}
