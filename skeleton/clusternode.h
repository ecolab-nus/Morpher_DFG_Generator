/*
 * clusternode.h
 *
 *  Created on: Apr 21, 2021
 *      Author: dmd
 */

#ifndef SKELETON_CLUSTERNODE_H_
#define SKELETON_CLUSTERNODE_H_


#include "edge.h"
#include "dfgnode.h"

using namespace llvm;


class clusterNode{
private:
	int idx;
	int tile_id;
	std::vector<clusterNode*> ChildClusterNodes;
	std::vector<clusterNode*> ParentClusterNodes;


	std::vector<dfgNode*> dfgnodes_in_cluster;
public:
	void addDfgNode(dfgNode* node);
	void addChildClusterNode(clusterNode* node);
	void addParentClusterNode(clusterNode* node);

	std::vector<clusterNode*> getParentClusterNodes(){return ParentClusterNodes;};
	std::vector<clusterNode*> getChildClusterNodes(){return ChildClusterNodes;};
	int getClusterID();//{return idx;}
	int getTileID(){return tile_id;};
	void setTileID(int tile_id);
	clusterNode(int idx);
	void printDFGnodes();

};



#endif /* SKELETON_CLUSTERNODE_H_ */
