/*
 * clusternode.cpp
 *
 *  Created on: Apr 21, 2021
 *      Author: dmd
 */
#include "clusternode.h"

clusterNode::clusterNode(int idx){
	this->idx = idx;
}

//clusterNode::clusterNode(){
//
//}


void clusterNode::addDfgNode(dfgNode* node){
	dfgnodes_in_cluster.push_back(node);
}

void clusterNode::addChildClusterNode(clusterNode* clnode){
	ChildClusterNodes.push_back(clnode);
}

void clusterNode::addParentClusterNode(clusterNode* clnode){
	ParentClusterNodes.push_back(clnode);
}

int clusterNode::getClusterID(){return idx;}

void clusterNode::printDFGnodes(){
	for(dfgNode* node : dfgnodes_in_cluster){
		std::cout << "DFG node id: " << node->getIdx() << "\n";
	}
}

void clusterNode::setTileID(int tileid){
	tile_id= tileid;
}
