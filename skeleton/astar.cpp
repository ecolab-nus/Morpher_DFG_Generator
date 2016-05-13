#include <assert.h>
#include "edge.h"
#include "astar.h"
#include "dfg.h"
#include <queue>


int AStar::heuristic(CGRANode* a, CGRANode* b) {
	assert(a->getT() == b->getT());
	return abs(a->getX() - b->getX()) + abs(a->getY() - b->getY());
}

bool AStar::AStarSearch(std::map<CGRANode*,std::vector<CGRANode*> > graph, CGRANode* start, CGRANode* goal, std::map<CGRANode*,CGRANode*> *cameFrom, std::map<CGRANode*,int> *costSoFar) {
	assert(start->getT() == goal->getT());
	std::priority_queue<CGRANodeWithCost, std::vector<CGRANodeWithCost>, LessThanCGRANodeWithCost> frontier;

	frontier.push(CGRANodeWithCost(start,0));
	(*cameFrom)[start] = NULL;
	(*costSoFar)[start] = 0;

	CGRANode* current;
	CGRANode* next;

	int newCost = 0;
	int priority = 0;

	while(!frontier.empty()){
		current = frontier.top().Cnode;
		frontier.pop();

//		errs() << "ASTAR::frontier_size = " << frontier.size() << "\n";

//		errs() << "ASTAR::current = " << "(" << current->getT() << ","
//			   	   	  	  	  	  	  	  	 << current->getY() << ","
//											 << current->getX() << ")\n";

		if(current == goal){
//			errs() << "ASTAR:: goal achieved!\n";
			break;
		}



//		std::map<CGRANode*,std::vector<CGRANode*> > graph = cgra->getCGRAEdges();
		for (int i = 0; i < graph[current].size(); ++i) {
			next = graph[current][i];

//			errs() << "ASTAR::next = " << "("    << next->getT() << ","
//				   	   	  	  	  	  	  	  	 << next->getY() << ","
//												 << next->getX() << ")\n";

			newCost = (*costSoFar)[current] + 1; //always graph.cost(current, next) = 1

//			errs() << "ASTAR::newCost = " << newCost << "\n";

			if(costSoFar->find(next) != costSoFar->end()){
				if(newCost >= (*costSoFar)[next]){
//					errs() << "ASTAR::newCost is higher, abandoning\n";
					continue;
				}
			}
			(*costSoFar)[next] = newCost;
			priority = newCost + heuristic(goal,next);
//			errs() << "ASTAR::pusing to prqueue with priority" << priority << "\n";
			frontier.push(CGRANodeWithCost(next,priority));
			(*cameFrom)[next] = current;
		}
	}

	if(current != goal){
		return false;
	}
	else{
//		errs() << "ASTAR:: goal achieved and returning!\n";
		return true;
	}
}

bool AStar::Route(std::vector<std::pair<CGRANode*, CGRANode*> > paths,
		std::map<CGRANode*,std::vector<CGRANode*> > cgraEdges, std::vector<std::pair<CGRANode*,CGRANode*> > *pathsNotRouted) {

	CGRANode* start;
	CGRANode* goal;
	CGRANode* current;

	std::map<CGRANode*,CGRANode*> cameFrom;
	std::map<CGRANode*,int> costSoFar;

	struct pathWithCost{
		std::pair<CGRANode*, CGRANode*> path;
		int cost;
		pathWithCost(std::pair<CGRANode*, CGRANode*> path,int cost) : path(path), cost(cost){}
	};

	struct LessThanPathWithCost{
	    bool operator()(pathWithCost const & p1, pathWithCost const & p2) {
	        // return "true" if "p1" is ordered before "p2", for example:
	        return p1.cost < p2.cost;
	    }
	};

	std::vector<pathWithCost> pathsWithCost;

	pathsNotRouted->clear();

	//Route estimation
	for (int i = 0; i < paths.size(); ++i) {
		start = paths[i].first;
		goal = paths[i].second;
		if(!AStarSearch(cgraEdges,start,goal,&cameFrom, &costSoFar)){
//			return false;
			pathsNotRouted->push_back(paths[i]);

			cameFrom.clear();
			costSoFar.clear();

			continue;
		}

		cameFrom.clear();
		costSoFar.clear();
		pathsWithCost.push_back(pathWithCost(paths[i],costSoFar[goal]));
	}

	cameFrom.clear();
	costSoFar.clear();

	std::sort(pathsWithCost.begin(),pathsWithCost.end(),LessThanPathWithCost());

	//Real Routing happens here

	for (int i = 0; i < pathsWithCost.size(); ++i) {
		start = pathsWithCost[i].path.first;
		goal = pathsWithCost[i].path.second;
		if(!AStarSearch(cgraEdges,start,goal,&cameFrom, &costSoFar)){
//			for (int j = i; j < pathsWithCost.size(); ++j) {
			pathsNotRouted->push_back(pathsWithCost[i].path);
//			}
//			return false;
			continue;
		}
		current = goal;
		*mappingOutFile << "The Path : ";
		while(current!=start){
			*mappingOutFile << "(" << current->getT() << "," << current->getY() << "," << current->getX() << ")" << " <-- ";
			if(cameFrom.find(current) == cameFrom.end()){
				printf("ASTAR :: ROUTING FAILURE\n");

				for (int j = i; j < pathsWithCost.size(); ++j) {
					pathsNotRouted->push_back(pathsWithCost[i].path);
				}
//				return false;
//				continue;
				break;
			}

//			cgra->removeEdge(cameFrom[current],current);
			assert(cameFrom[current]->getT() == current->getT());
			if(cgraEdges.find(cameFrom[current]) != cgraEdges.end()){
				std::vector<CGRANode*> *vec = &(cgraEdges.find(cameFrom[current])->second);
				vec->erase(std::remove(vec->begin(), vec->end(), current), vec->end());
			}

			current = cameFrom[current];
		}
		*mappingOutFile << "(" << current->getT() << "," << current->getY() << "," << current->getX() << ")" << "\n";
//		*mappingOutFile << "\n";
	}

	return true;
}
