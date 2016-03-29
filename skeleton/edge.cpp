#include "edge.h"

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
}

void Edge::setSrc(Instruction* Src){
	this->src = Src;
}

Instruction* Edge::getSrc(){
	return src;
}

void Edge::setDest(Instruction* Dest){
	this->dest = Dest;
}

Instruction* Edge::getDest(){
	return dest;
}

void Edge::setType(int Type) {
	type = Type;
}

int Edge::getType() {
	return type;
}
