#ifndef ASTAR_H
#define ASTAR_H


#include "CGRANode.h"
#include "CGRA.h"

#include <iostream>
#include <fstream>

struct CGRANodeWithCost{
	CGRANode* Cnode;
	int cost;
	Port port;
	CGRANodeWithCost(CGRANode* Cnode, int cost, Port port) : Cnode(Cnode),  cost(cost), port(port){}
};

struct LessThanCGRANodeWithCost {
    bool operator()(CGRANodeWithCost const & p1, CGRANodeWithCost const & p2) {
        // return "true" if "p1" is ordered before "p2", for example:
        return p1.cost > p2.cost;
    }
};

class DFG;

class AStar{

private:
	std::ofstream *mappingOutFile;
	int MII;
	DFG* currDFG;


public:
	AStar(std::ofstream *mappingOutFile, int MII, DFG* currDFG) : mappingOutFile(mappingOutFile),
																  MII(MII),
																  currDFG(currDFG),
																  maxPathLength(0),
																  maxSMARTPathLength(0){}
	int heuristic(CGRANode* a, CGRANode* b);
	CGRANode* AStarSearch(	std::map<CGRANode*,std::vector<CGRAEdge> > graph,
							CGRANode* start,
							Port startPort,
							CGRANode* goal,
							std::map<std::pair<CGRANode*,Port>,std::pair<CGRANode*,Port>> *cameFrom,
							std::map<CGRANode*,int> *costSoFar,
							Port* endPort);

	CGRANode* AStarSearchEMS(std::map<CGRANode*, std::vector<CGRAEdge> > graph,
							 CGRANode* start,
							 CGRANode* goal,
							 std::map<std::pair<CGRANode*,Port>, std::pair<CGRANode*,Port>>* cameFrom,
							 std::map<CGRANode*, int>* costSoFar,
							 Port* endPort);


	bool Route(dfgNode* currNode,
			   std::vector<dfgNode*> parents,
//			   std::vector<std::pair<CGRANode*,CGRANode*> > paths,
//			   std::vector<TreePath> treePaths,
			   std::vector<CGRANode*>* dests,
			   std::map<CGRANode*,std::vector<CGRAEdge> >* cgraEdges,
			   std::vector<std::pair<CGRANode*,CGRANode*> > *pathsNotRouted,
			   CGRANode** chosenDest,
			   bool* deadEndReached = NULL);

//	bool EMSRoute(std::vector<dfgNode*> parents,
//				  std::vector<std::pair<CGRANode*,CGRANode*> > paths,
//			   	  std::map<CGRANode*,std::vector<CGRANode*> >* cgraEdges,
//				  std::vector<std::pair<CGRANode*,CGRANode*> > *pathsNotRouted);

	bool EMSRoute(dfgNode* currNode,
			   std::vector<dfgNode*> parents,
//			   std::vector<std::pair<CGRANode*,CGRANode*> > paths,
//			   std::vector<TreePath> treePaths,
			   std::vector<CGRANode*> dests,
			   std::map<CGRANode*,std::vector<CGRAEdge> >* cgraEdges,
			   std::vector<std::pair<CGRANode*,CGRANode*> > *pathsNotRouted,
			   bool* deadEndReached = NULL);


	bool CameFromVerify(std::map<std::pair<CGRANode*,Port>,std::pair<CGRANode*,Port>> *cameFrom);

	//Logging
	std::map<int,int> SMARTPathHist;
	std::map<int,int> SMARTPredicatePathHist;
	std::map<int,int> PathHist;
	std::map<int,std::vector<int>> PathSMARTPaths;
	std::map<int,int> PathSMARTPathCount;

	int maxPathLength;
	int maxSMARTPathLength;
	std::map<int,std::vector<dfgNode*>> ASAPLevelNodeMap;

	bool reportDeadEnd(CGRANode* end, Port endPort, dfgNode* currNode, std::map<CGRANode*,std::vector<CGRAEdge> >* cgraEdges);

};















#endif
