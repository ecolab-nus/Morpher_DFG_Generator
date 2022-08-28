#include <morpherdfggen/common/edge.h>

#include <morpherdfggen/common/dfgnode.h>

int Edge::getID(){
	return ID;
}

void Edge::setID (int id){
	this->ID = id;
}

void Edge::setName (std::string Name){
	this->name = Name;
}

std::string Edge::getName(){
	return name;
//	std::string ret = std::to_string(src->getIdx()) + "to" + std::to_string(dest->getIdx());
}

void Edge::setSrc(dfgNode* Src){
	this->src = Src;
}

dfgNode* Edge::getSrc(){
	return src;
}

void Edge::setDest(dfgNode* Dest){
	this->dest = Dest;
}

dfgNode* Edge::getDest(){
	return dest;
}

void Edge::setType(int Type) {
	type = Type;
}

int Edge::getType() {
	return type;
}
