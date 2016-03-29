#ifndef CGRANODE_H
#define CGRANODE_H

#include "edge.h"
#include "dfgnode.h"

class CGRANode;

struct ConnectedCGRANode{
	CGRANode* node;
	int cost;
	std::string name;

	ConnectedCGRANode(CGRANode* n, int c, std::string str="unknown") : node(n), cost(c), name(str){};
};

struct less_than_cost{
	inline bool operator()(const ConnectedCGRANode& node1, const ConnectedCGRANode& node2){
		return (node1.cost < node2.cost);
	}
};



using namespace llvm;

class CGRANode{
		private :
			std::vector<ConnectedCGRANode> connectedNodes;
			dfgNode* mappedDFGNode = NULL;
			int x;
			int y;
			int t;

		public :
			CGRANode(int x, int y, int t);
			void addConnectedNode(CGRANode* node, int cost, std::string name = "unknown");
			void sortConnectedNodes();

			void setMappedDFGNode (dfgNode* node);
			dfgNode* getmappedDFGNode();

			void setX(int x);
			void setY(int y);
			void setT(int t);

			int getX();
			int getY();
			int getT();

			std::vector<ConnectedCGRANode> getConnectedNodes();


	};


#endif
