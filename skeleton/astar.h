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
	AStar(std::ofstream *mappingOutFile, int MII) : mappingOutFile(mappingOutFile), MII(MII){}
	int heuristic(CGRANode* a, CGRANode* b);
	bool AStarSearch(std::map<CGRANode*,std::vector<CGRANode*> > graph, CGRANode* start, CGRANode* goal, std::map<CGRANode*,CGRANode*> *cameFrom, std::map<CGRANode*,int> *costSoFar);
	bool Route(std::vector<std::pair<CGRANode*,CGRANode*> > paths, std::map<CGRANode*,std::vector<CGRANode*> > cgraEdges, std::vector<std::pair<CGRANode*,CGRANode*> > *pathsNotRouted);

};















#endif
