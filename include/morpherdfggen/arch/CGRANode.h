#ifndef CGRANODE_H
#define CGRANODE_H

#include <morpherdfggen/common/dfgnode.h>
#include <morpherdfggen/common/edge.h>

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
enum PEType{MEM,ALU};

class CGRA;

using namespace llvm;

class CGRANode{
		private :
			std::vector<ConnectedCGRANode> connectedNodes;
			dfgNode* mappedDFGNode = NULL;
			int x;
			int y;
			int t;

			int probCost = 0;
			dfgNode* routingDFGNode = NULL;
			CGRA* Parent;
			PEType peType = ALU;


		public :
			int originalEdgesSize = 0;
			CGRANode(int x, int y, int t, CGRA* ParentCGRA);
			void addConnectedNode(CGRANode* node, int cost, std::string name = "unknown");
			void sortConnectedNodes();

			void setMappedDFGNode (dfgNode* node);
			dfgNode* getmappedDFGNode();

			void setX(int x);
			void setY(int y);
			void setT(int t);
			void setProbCost(int cost){probCost = cost;}
			void setRoutingNode(dfgNode* src){routingDFGNode = src;}

			int getX();
			int getY();
			int getT();
			int getProbCost(){return probCost;}
			dfgNode* getRoutingNode(){return routingDFGNode;}
			std::string getName();
			std::string getNameSp();
			std::string getNameWithOutTime();
			bool equals(int tt, int yy, int xx);
			bool isCorner();
			bool isBoundary(); //Corner is a subset of boundary
			bool isMiddle();

			std::vector<ConnectedCGRANode> getConnectedNodes();

			//RegAllocation - this will initially consists of edges allocation
			std::map<CGRANode*,std::vector<Edge*> > regAllocation;

			std::map<Port,std::vector<Port> > InOutPortMap;

			PEType getPEType(){return peType;}
			void setPEType(PEType pt){peType = pt;}


	};


#endif
