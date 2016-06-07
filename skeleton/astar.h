#ifndef ASTAR_H
#define ASTAR_H


#include "CGRANode.h"
#include "CGRA.h"

#include <iostream>
#include <fstream>

struct CGRANodeWithCost{
	CGRANode* Cnode;
	int cost;
	CGRANodeWithCost(CGRANode* Cnode, int cost) : Cnode(Cnode),  cost(cost){}
};

struct LessThanCGRANodeWithCost {
    bool operator()(CGRANodeWithCost const & p1, CGRANodeWithCost const & p2) {
        // return "true" if "p1" is ordered before "p2", for example:
        return p1.cost > p2.cost;
    }
};


class AStar{

private:
	std::ofstream *mappingOutFile;
	int MII;



public:
	AStar(std::ofstream *mappingOutFile, int MII) : mappingOutFile(mappingOutFile), MII(MII), maxPathLength(0), maxSMARTPathLength(0){}
	int heuristic(CGRANode* a, CGRANode* b);
	CGRANode* AStarSearch(std::map<CGRANode*,
			         std::vector<CGRANode*> > graph,
					 CGRANode* start,
					 CGRANode* goal,
					 std::map<CGRANode*,CGRANode*> *cameFrom,
					 std::map<CGRANode*,int> *costSoFar);
	bool Route(std::vector<dfgNode*> parents,
			   std::vector<std::pair<CGRANode*,CGRANode*> > paths,
			   std::map<CGRANode*,std::vector<CGRANode*> >* cgraEdges,
			   std::vector<std::pair<CGRANode*,CGRANode*> > *pathsNotRouted);

	bool EMSRoute(std::vector<dfgNode*> parents,
				  std::vector<std::pair<CGRANode*,CGRANode*> > paths,
			   	  std::map<CGRANode*,std::vector<CGRANode*> >* cgraEdges,
				  std::vector<std::pair<CGRANode*,CGRANode*> > *pathsNotRouted);

	//Logging
	std::map<int,int> SMARTPathHist;
	std::map<int,int> SMARTPredicatePathHist;
	std::map<int,int> PathHist;
	std::map<int,std::vector<int>> PathSMARTPaths;
	std::map<int,int> PathSMARTPathCount;

	int maxPathLength;
	int maxSMARTPathLength;

};















#endif
