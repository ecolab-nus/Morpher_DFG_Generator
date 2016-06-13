#ifndef DFG_H
#define DFG_H

#include "edge.h"
#include "dfgnode.h"
#include "CGRANode.h"
#include "CGRA.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/DependenceAnalysis.h"

#include <iostream>
#include <fstream>

#define REGS_PER_NODE 4

class AStar;

using namespace llvm;

struct less_than_schIdx{
	inline bool operator()(dfgNode* node1, dfgNode* node2){
		return (node1->getSchIdx() < node2->getSchIdx());
	}
};

struct ScheduleOrder{
	inline bool operator()(dfgNode node1, dfgNode node2){

		int slack1 = node1.getALAPnumber() - node1.getASAPnumber();
		int slack2 = node2.getALAPnumber() - node2.getASAPnumber();

		if(node1.getASAPnumber() < node2.getASAPnumber()){
			return true;
		}
		else{
			return false;
		}

//		if(node1.getALAPnumber() < node2.getALAPnumber()){
//			return true;
//		}
//		else if(node1.getALAPnumber() > node2.getALAPnumber()){
//			return false;
//		}
//		else if(slack1 == slack2){ //node1.getALAPnumber() == node2.getALAPnumber()
//			return(node1.getASAPnumber() < node2.getASAPnumber());
//		}
//		else if(slack1 < slack2){
//			return true;
//		}
//		else{ // slack1 > slack2
//			return false;
//		}

	}
};

struct ValueComparer{
	inline bool operator()(const std::pair<Instruction*,int> a, const std::pair<Instruction*,int> b){
		if(a.second < b.second){
			return true;
		}
		else{
			return false;
		}
	}
};

struct CostComparer{
	inline bool operator()(const ConnectedCGRANode a, const ConnectedCGRANode b){
		if(a.cost < b.cost){
			return true;
		}
		else{
			return false;
		}
	}
};

struct phyLoc{
	int x;
	int y;
	int t;
};

struct nodeWithCost{
	dfgNode* node;
	CGRANode* cnode;
	int cost;
	int mappedRealTime;
	nodeWithCost(dfgNode* node, CGRANode* cnode, int cost, int mappedRealTime) : node(node), cnode(cnode), cost(cost), mappedRealTime(mappedRealTime){}
};

struct LessThanNodeWithCost{
    bool operator()(nodeWithCost const & p1, nodeWithCost const & p2) {
        // return "true" if "p1" is ordered before "p2", for example:
        return p1.cost < p2.cost;
    }
};

struct TreePath{
	std::vector<CGRANode*> sources;
	CGRANode* bestSource;
	int bestCost;
	CGRANode* dest;
	TreePath() : bestSource(NULL), dest(NULL), bestCost(-1){}
};

class DFG{
		private :
			std::vector<dfgNode> NodeList;
			std::ofstream xmlFile;
			std::vector<Edge> edgeList;
			int maxASAPLevel = -1;
			int maxRecDist = -1;
			std::map<const BasicBlock*,std::vector<const BasicBlock*>> BBSuccBasicBlocks;
			bool deadEndReached = false;
			std::string name;



			void renumber();
			void traverseBFS(dfgNode* node, int ASAPlevel);
			void traverseInvBFS(dfgNode* node, int ALAPlevel);

			std::vector<dfgNode*> schedule;
			void traverseCriticalPath(dfgNode* node, int level);

			CGRA* currCGRA;
			std::vector<ConnectedCGRANode> searchCandidates(CGRANode* mappedLoc, dfgNode* node, std::vector<std::pair<Instruction*,int>>* candidateNumbers);
			void eraseAlreadyMappedNodes(std::vector<ConnectedCGRANode>* candidates);
			void backTrack(int nodeSeq);

			std::vector<ConnectedCGRANode> ExpandCandidatesAddingRoutingNodes(std::vector<std::pair<Instruction*,int>>* candidateNumbers);
			std::vector<ConnectedCGRANode> getConnectedCGRANodes(dfgNode* node);

			int getConMatIdx(int t, int y, int x);

			bool MapMultiDestRec(std::map<dfgNode*,std::vector< std::pair<CGRANode*,int> > > *nodeDestMap,
					             std::map<CGRANode*,std::vector<dfgNode*> > *destNodeMap,
								 std::map<dfgNode*,std::vector< std::pair<CGRANode*,int> > >::iterator it,
								 std::map<CGRANode*,std::vector<CGRANode*> > cgraEdges,
								 int index);


		public :
			std::ofstream mappingOutFile;
			AStar* astar;

			DFG(std::string name) : name(name){}

			dfgNode* getEntryNode();

			std::vector<dfgNode> getNodes();
			std::vector<dfgNode>* getNodesPtr(){return &NodeList;}

			std::vector<Edge> getEdges();
			void InsertNode(Instruction* Node);

			void InsertNode(dfgNode Node);

			void InsertEdge(Edge e);

			dfgNode* findNode(Instruction* I);

			Edge* findEdge(Instruction* src, Instruction* dest);

			std::vector<dfgNode*> getRoots();

			std::vector<dfgNode*> getLeafs(BasicBlock* BB);
			std::vector<dfgNode*> getLeafs();

			void connectBB();

			void addMemDepEdges(MemoryDependenceAnalysis *MD);
			void addMemRecDepEdges(DependenceAnalysis *DA);
			void addMemRecDepEdgesNew(DependenceAnalysis *DA);

		    int removeEdge(Edge* e);
		    int removeNode(dfgNode* n);
			void removeAlloc();

			void traverseDFS(dfgNode* startNode, int dfsCount=0);


			void printXML(std::string fileName);

			void printInEdges(dfgNode* node, int depth = 0);

			void printOutEdges(dfgNode* node, int depth = 0);

			void printOP(dfgNode* node, int depth=0);

			void printOPs(int depth=0);

			void printDFGInfo();

			void printHeaderTag(std::string tagName, int depth=0);

			void printFooterTag(std::string tagName, int depth=0);

			void printEdges(int depth = 0);

			void printEdge(Edge* e, int depth = 0);

			void scheduleASAP();
			void scheduleALAP();

			void MapCGRA(int XDim, int YDim);
			void CreateSchList();

			std::vector<ConnectedCGRANode> FindCandidateCGRANodes(dfgNode* node);

			void MapCGRA_SMART(int XDim, int YDim, std::string mapfileName = "Mapping.log");
			void MapCGRA_SA(int XDim, int YDim, std::string mapfileName = "Mapping.log");
			bool MapMultiDest(std::map<dfgNode*,std::vector< std::pair<CGRANode*,int> > > *nodeDestMap, std::map<CGRANode*,std::vector<dfgNode*> > *destNodeMap);
			bool MapASAPLevel(int MII, int XDim, int YDim);
			int getAffinityCost(dfgNode* a, dfgNode* b);

			std::vector<std::vector<int> > getConMat();
			std::vector<std::vector<int> > selfMulConMat(std::vector<std::vector<int> > in);
			std::map<dfgNode*,std::vector<CGRANode*> > getPrimarySlots(std::vector<dfgNode*> nodes);

			std::vector<int> getIntersection(std::vector<std::vector<int> > vectors);
			int AddRoutingEdges(dfgNode* node);
			int AStarSP(CGRANode* src, CGRANode* dest, std::vector<CGRANode*>* path);
			int getDist(CGRANode* a, CGRANode*b);

			void setMaxRecDist (int d){maxRecDist = d;}
			int getMaxRecDist(){return maxRecDist;}

			void findMaxRecDist();

			void setBBSuccBasicBlocks(std::map<const BasicBlock*,std::vector<const BasicBlock*>> map){BBSuccBasicBlocks = map;}
			std::map<const BasicBlock*,std::vector<const BasicBlock*>> getBBSuccBasicBlocks(){return BBSuccBasicBlocks;}

			void MapCGRA_EMS(int XDim, int YDim, std::string mapfileName = "Mapping.log");
			bool MapCGRA_EMS_ASAPLevel(int MII,int XDim, int YDim);
			bool MAPCGRA_EMS_MultDest(std::map<dfgNode*,std::vector< std::pair<CGRANode*,int> > > *nodeDestMap,
								      std::map<CGRANode*,std::vector<dfgNode*> > *destNodeMap,
//									  std::map<dfgNode*,std::vector< std::pair<CGRANode*,int> > >::iterator it,
									  std::vector<nodeWithCost>::iterator it,
									  std::map<CGRANode*,std::vector<CGRANode*> > cgraEdges,
									  int index);

			TreePath createTreePath(dfgNode* parent, CGRANode* dest);
			void clearMapping();

			CGRA* getCGRA(){return currCGRA;}

			int findUtilTreeRoutingLocs(CGRANode* cnode, dfgNode* currNode);
			void printOutSMARTRoutes();
			int convertToPhyLoc(int t, int y, int x);
			int convertToPhyLoc(int y, int x);


	};


#endif
