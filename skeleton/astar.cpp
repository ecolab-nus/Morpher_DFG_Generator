#include <assert.h>
#include "edge.h"
#include "astar.h"
#include "dfg.h"
#include <queue>


int AStar::heuristic(CGRANode* a, CGRANode* b) {
//	assert(a->getT() == b->getT());
	return abs(a->getX() - b->getX()) + abs(a->getY() - b->getY()) + 16*((b->getT() - a->getT() + MII)%MII);
}

CGRANode* AStar::AStarSearch(std::map<CGRANode*,std::vector<CGRANode*> > graph,
						CGRANode* start,
						CGRANode* goal,
						std::map<CGRANode*,CGRANode*> *cameFrom,
						std::map<CGRANode*,int> *costSoFar) {
//	assert(start->getT() == goal->getT());
	std::priority_queue<CGRANodeWithCost, std::vector<CGRANodeWithCost>, LessThanCGRANodeWithCost> frontier;
	frontier.push(CGRANodeWithCost(start,0));

	cameFrom->clear();
	costSoFar->clear();

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

////	if(start->getT() == 9){
//		errs() << "ASTAR::current = " << "(" << current->getT() << ","
//			   	   	  	  	  	  	  	  	 << current->getY() << ","
//											 << current->getX() << ")\n";
////	}


		if(current == goal){
////			if(start->getT() == 9){
//			errs() << "ASTAR:: goal achieved!\n";
////			}
			break;
		}



//		std::map<CGRANode*,std::vector<CGRANode*> > graph = cgra->getCGRAEdges();
		for (int i = 0; i < graph[current].size(); ++i) {
			next = graph[current][i];

////			if(start->getT() == 9){
//			errs() << "ASTAR::next = " << "("    << next->getT() << ","
//				   	   	  	  	  	  	  	  	 << next->getY() << ","
//												 << next->getX() << ")\n";
////			}

			if(current->getT() == next->getT()){
				newCost = (*costSoFar)[current] + 1; //always graph.cost(current, next) = 1
			}else{
				newCost = (*costSoFar)[current] + 16*((next->getT() - current->getT() + MII)%MII);
			}

////			if(start->getT() == 9){
//			errs() << "ASTAR::newCost = " << newCost << "\n";
////			}

			if(costSoFar->find(next) != costSoFar->end()){
				if(newCost >= (*costSoFar)[next]){
////					if(start->getT() == 9){
//					errs() << "ASTAR::newCost is higher, abandoning\n";
////					}
					continue;
				}
			}
			(*costSoFar)[next] = newCost;
			priority = newCost + heuristic(goal,next);
////			if(start->getT() == 9){
//			errs() << "ASTAR::pusing to prqueue with priority" << priority << "\n";
////			}
			frontier.push(CGRANodeWithCost(next,priority));
			(*cameFrom)[next] = current;
		}
	}

	return current;
//	if(current != goal){
//		return false;
//	}
//	else{
////		errs() << "ASTAR:: goal achieved and returning!\n";
//		return true;
//	}
}

bool AStar::Route(std::vector<dfgNode*> parents,
				  std::vector<std::pair<CGRANode*, CGRANode*> > paths,
			      std::map<CGRANode*,std::vector<CGRANode*> >* cgraEdges,
				  std::vector<std::pair<CGRANode*,CGRANode*> > *pathsNotRouted) {

	CGRANode* start;
	CGRANode* goal;
	CGRANode* current;
	CGRANode* end;
	dfgNode* currParent;

	std::map<CGRANode*,CGRANode*> cameFrom;
	std::map<CGRANode*,int> costSoFar;

	struct pathWithCost{
		std::pair<CGRANode*, CGRANode*> path;
		int cost;
		dfgNode* parent;
		pathWithCost(std::pair<CGRANode*, CGRANode*> path,int cost,dfgNode* parent) : path(path), cost(cost), parent(parent){}
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
		end = AStarSearch(*cgraEdges,start,goal,&cameFrom, &costSoFar);
		if(end != goal){
//			return false;
			errs() << "SMARTRouteEst :: " << "Path is not routed = " << start->getName() << " to " << goal->getName() << "\n";
			errs() << "SMARTRouteEst :: " << "But Routed until : " << (end)->getName() << "\n";
			pathsNotRouted->push_back(paths[i]);

			cameFrom.clear();
			costSoFar.clear();

			continue;
		}

		cameFrom.clear();
		costSoFar.clear();
		pathsWithCost.push_back(pathWithCost(paths[i],costSoFar[goal],parents[i]));
	}

	cameFrom.clear();
	costSoFar.clear();

	std::sort(pathsWithCost.begin(),pathsWithCost.end(),LessThanPathWithCost());

	//Real Routing happens here

	for (int i = 0; i < pathsWithCost.size(); ++i) {
		start = pathsWithCost[i].path.first;
		goal = pathsWithCost[i].path.second;
		currParent = pathsWithCost[i].parent;

		*mappingOutFile << "start = (" << start->getT() << "," << start->getY() << "," << start->getX() << ")\n";
		*mappingOutFile << "goal = (" << goal->getT() << "," << goal->getY() << "," << goal->getX() << ")\n";

		end = AStarSearch(*cgraEdges,start,goal,&cameFrom, &costSoFar);
		if(end != goal){
//			for (int j = i; j < pathsWithCost.size(); ++j) {
			errs() << "SMARTRouteReal :: " << "Path is not routed = " << start->getName() << " to " << goal->getName() << "\n";
			errs() << "SMARTRouteReal :: " << "But Routed until : " << (end)->getName() << "\n";
			pathsNotRouted->push_back(pathsWithCost[i].path);
//			}
//			return false;
			continue;
		}

		int pathLength = 0;
		int SMARTpathLength = 0;
		int regConnections = 0;
		bool SMARTPathEnd = true;

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
//			if(cameFrom[current]->getT() == current->getT()){
//				assert(cameFrom[current]->getT() == current->getT());
				if(cgraEdges->find(cameFrom[current]) != cgraEdges->end()){
//					*mappingOutFile << (*cgraEdges)[cameFrom[current]].size();
//					*mappingOutFile << " R" << current << " ";
//					std::vector<CGRANode*> *vec = &(cgraEdges.find(cameFrom[current])->second);
//					std::vector<CGRANode*> *vec = &cgraEdges[cameFrom[current]];

					std::vector<CGRANode*>::iterator found = std::find((*cgraEdges)[cameFrom[current]].begin(), (*cgraEdges)[cameFrom[current]].end(), current);
					if(found != (*cgraEdges)[cameFrom[current]].end()){
						(*cgraEdges)[cameFrom[current]].erase(found);
					}

//					assert(std::find((*cgraEdges)[cameFrom[current]].begin(),(*cgraEdges)[cameFrom[current]].end(),current) == (*cgraEdges)[cameFrom[current]].end());

//					*mappingOutFile << (*cgraEdges)[cameFrom[current]].size();
				}
//			}

			//Logging - Begin
			if(current->getT() != cameFrom[current]->getT()){
				if(!SMARTPathEnd){
					SMARTPathEnd = true;
					if(SMARTPathHist.find(SMARTpathLength) == SMARTPathHist.end()){
						SMARTPathHist[SMARTpathLength] = 1;
						if(currParent->getNode()->getOpcode() == Instruction::Br){
							SMARTPredicatePathHist[SMARTpathLength] = 1;
						}

					}
					else{
						SMARTPathHist[SMARTpathLength]++;
						if(currParent->getNode()->getOpcode() == Instruction::Br){
							SMARTPredicatePathHist[SMARTpathLength]++;
						}
					}

					if(SMARTpathLength > maxSMARTPathLength){
						maxSMARTPathLength = SMARTpathLength;
					}
					SMARTpathLength = 0;
				}
				else{
					assert(SMARTpathLength == 0);
				}
				regConnections++;
			}
			else{
				SMARTPathEnd = false;
				SMARTpathLength++;
			}
			//Logging - End

			pathLength++;
			current = cameFrom[current];
		}

		//Logging - SMART Path Length
		if(SMARTpathLength != 0){
			if(SMARTPathHist.find(SMARTpathLength) == SMARTPathHist.end()){
				SMARTPathHist[SMARTpathLength] = 1;
				if(currParent->getNode()->getOpcode() == Instruction::Br){
					SMARTPredicatePathHist[SMARTpathLength] = 1;
				}
			}
			else{
				SMARTPathHist[SMARTpathLength]++;
				if(currParent->getNode()->getOpcode() == Instruction::Br){
					SMARTPredicatePathHist[SMARTpathLength]++;
				}
			}

			if(SMARTpathLength > maxSMARTPathLength){
				maxSMARTPathLength = SMARTpathLength;
			}
		}

		//Logging - Path Histogram
		if(PathHist.find(pathLength) == PathHist.end()){
			PathHist[pathLength] = 1;
			PathSMARTPathCount[pathLength] = regConnections;
		}
		else{
			PathHist[pathLength]++;
			PathSMARTPathCount[pathLength] += regConnections;
		}
		PathSMARTPaths[pathLength].push_back(regConnections);



		if(pathLength > maxPathLength){
			maxPathLength = pathLength;
		}

		*mappingOutFile << "(" << current->getT() << "," << current->getY() << "," << current->getX() << ")" << "\n";
//		*mappingOutFile << "\n";
	}

	return true;
}

bool AStar::EMSRoute(std::vector<dfgNode*> parents,
					 std::vector<std::pair<CGRANode*, CGRANode*> > paths,
					 std::map<CGRANode*, std::vector<CGRANode*> >* cgraEdges,
					 std::vector<std::pair<CGRANode*, CGRANode*> >* pathsNotRouted) {

	CGRANode* start;
	CGRANode* goal;
	CGRANode* current;
	CGRANode* end;
	CGRANode* TwoNodesBeforeCurrent;

	std::map<CGRANode*,CGRANode*> cameFrom;
	std::map<CGRANode*,int> costSoFar;

	struct pathWithCost{
		std::pair<CGRANode*, CGRANode*> path;
		int cost;
		dfgNode* parent;
		pathWithCost(){}
		pathWithCost(dfgNode* parent, std::pair<CGRANode*, CGRANode*> path,int cost) : parent(parent), path(path), cost(cost){}
	};

	struct LessThanPathWithCost{
	    bool operator()(pathWithCost const & p1, pathWithCost const & p2) {
	        // return "true" if "p1" is ordered before "p2", for example:
	        return p1.cost < p2.cost;
	    }
	};

	std::vector<pathWithCost> pathsWithCost;
	std::priority_queue<pathWithCost, std::vector<pathWithCost>, LessThanPathWithCost> pathsPrQueue;
	std::priority_queue<pathWithCost, std::vector<pathWithCost>, LessThanPathWithCost> pathsPrQueueTemp;

	pathsNotRouted->clear();

	//Route estimation
	for (int i = 0; i < paths.size(); ++i) {
		start = paths[i].first;
		goal = paths[i].second;
		end = AStarSearch(*cgraEdges,start,goal,&cameFrom, &costSoFar);
		if(end != goal){
//			return false;
			errs() << "EMSRoute :: " << "Init Route est, Path is not routed = " << start->getName() << " to " << goal->getName() << "\n";
			errs() << "EMSRoute :: " << "But Routed until : " << (end)->getName() << "\n";
			pathsNotRouted->push_back(paths[i]);
			return false;
//			continue;
		}

		pathsPrQueue.push(pathWithCost(parents[i], paths[i],costSoFar[goal]));
//		pathsWithCost.push_back(pathWithCost(parents[i], paths[i],costSoFar[goal]));
	}

//	std::sort(pathsWithCost.begin(),pathsWithCost.end(),LessThanPathWithCost());

	//Real Routing happens here

	pathWithCost currPath;

	*mappingOutFile << "Routing the set of paths....\n";
	while(!pathsPrQueue.empty()){
		currPath = pathsPrQueue.top();
		pathsPrQueue.pop();

		start = currPath.path.first;
		goal = currPath.path.second;

		if(AStarSearch(*cgraEdges,start,goal,&cameFrom, &costSoFar) != goal){
			errs() << "EMSRoute :: " << "Real Routing,Path is not routed = " << start->getName() << " to " << goal->getName() << "\n";
			pathsNotRouted->push_back(currPath.path);
			return false;
//			continue;
		}

		int pathLength = 0;
		current = goal;
		*mappingOutFile << "The Path : ";

		while(current!=start){
			pathLength++;
			*mappingOutFile << "(" << current->getT() << "," << current->getY() << "," << current->getX() << ")" << " <-- ";
			if(cameFrom.find(current) == cameFrom.end()){
				printf("ASTAR :: ROUTING FAILURE\n");

//				for (int j = i; j < pathsWithCost.size(); ++j) {
//					pathsNotRouted->push_back(currPath.path);
//				}

				pathsNotRouted->push_back(currPath.path);
				while(!pathsPrQueue.empty()){
					currPath = pathsPrQueue.top();
					pathsPrQueue.pop();
					errs() << "EMSRoute :: " << "Real Routing Fail,Path is not routed = " << start->getName() << " to " << goal->getName() << "\n";
					pathsNotRouted->push_back(currPath.path);
				}

				return false;
//				continue;
//				break;
			}

//				if(cgraEdges->find(cameFrom[current]) != cgraEdges->end()){
////					*mappingOutFile << (*cgraEdges)[cameFrom[current]].size();
////					*mappingOutFile << " R" << current << " ";
//
//					std::vector<CGRANode*>::iterator found = std::find((*cgraEdges)[cameFrom[current]].begin(), (*cgraEdges)[cameFrom[current]].end(), current);
//					if(found != (*cgraEdges)[cameFrom[current]].end()){
//						(*cgraEdges)[cameFrom[current]].erase(found);
//					}
////					assert(std::find((*cgraEdges)[cameFrom[current]].begin(),(*cgraEdges)[cameFrom[current]].end(),current) == (*cgraEdges)[cameFrom[current]].end());
////					*mappingOutFile << (*cgraEdges)[cameFrom[current]].size();
//				}

				//Remove all edges used by the routing location
//				if((cameFrom[current]->getX() == current->getX())&&
//				   (cameFrom[current]->getY() == current->getY())
//				   ){
//
//				}
//				else{
					std::map<CGRANode*, std::vector<CGRANode*> >::iterator cgraEdgeIter;
					for(cgraEdgeIter = cgraEdges->begin(); cgraEdgeIter != cgraEdges->end(); cgraEdgeIter++){


						if(cameFrom[current] != cgraEdgeIter->first){
							if( (cgraEdgeIter->first->getX() == current->getX())&&(cgraEdgeIter->first->getY() == current->getY()) ){
								continue;
							}
						}

					    std::vector<CGRANode*>::iterator found = std::find((*cgraEdges)[cgraEdgeIter->first].begin(), (*cgraEdges)[cgraEdgeIter->first].end(), current);
					    if(found != (*cgraEdges)[cgraEdgeIter->first].end()){
					    	*mappingOutFile << "DEBUG :: " << "removing edge=" << cgraEdgeIter->first->getName() << " to " << (*found)->getName();
					    	*mappingOutFile << " Number of edges=" << (*cgraEdges)[cgraEdgeIter->first].size() << " reduced to ";
						    (*cgraEdges)[cgraEdgeIter->first].erase(found);
						    *mappingOutFile << (*cgraEdges)[cgraEdgeIter->first].size() << "\n";
					    }
					}
//				}

			if(cameFrom[current] != start){
				TwoNodesBeforeCurrent = cameFrom[cameFrom[current]];
				if((TwoNodesBeforeCurrent->getY() == cameFrom[current]->getY())&&
				   (TwoNodesBeforeCurrent->getX() == cameFrom[current]->getX())&&
				   (cameFrom[current]->getY() == current->getY())&&
				   (cameFrom[current]->getX() == current->getX())){

				}
				else{
					cameFrom[current]->setMappedDFGNode(currPath.parent);
					currPath.parent->getRoutingLocs()->push_back(cameFrom[current]);
				}
			}
			current = cameFrom[current];
		}

		//Logging
		if(SMARTPathHist.find(pathLength) == SMARTPathHist.end()){
			SMARTPathHist[pathLength] = 1;
		}
		else{
			SMARTPathHist[pathLength]++;
		}

		if(pathLength > maxPathLength){
			maxPathLength = pathLength;
		}

		*mappingOutFile << "(" << current->getT() << "," << current->getY() << "," << current->getX() << ")" << "\n";

		//Re-calculate the shortest paths
		while(!pathsPrQueue.empty()){
			currPath = pathsPrQueue.top();
			pathsPrQueue.pop();
			pathsPrQueueTemp.push(currPath);
		}

		while(!pathsPrQueueTemp.empty()){
			currPath = pathsPrQueueTemp.top();
			pathsPrQueueTemp.pop();

			start = currPath.path.first;
			goal = currPath.path.second;
			if(AStarSearch(*cgraEdges,start,goal,&cameFrom, &costSoFar) != goal){
				errs() << "EMSRoute :: " << "Real Routing Recompute,Path is not routed = " << start->getName() << " to " << goal->getName() << "\n";
				pathsNotRouted->push_back(currPath.path);
				return false;
//				continue;
			}
			pathsPrQueue.push(pathWithCost(currPath.parent, currPath.path, costSoFar[goal]));
		}
	}
	return true;
}
//	for (int i = 0; i < pathsWithCost.size(); ++i) {
//		start = pathsWithCost[i].path.first;
//		goal = pathsWithCost[i].path.second;
//		if(!AStarSearch(*cgraEdges,start,goal,&cameFrom, &costSoFar)){
////			for (int j = i; j < pathsWithCost.size(); ++j) {
//			pathsNotRouted->push_back(pathsWithCost[i].path);
////			}
////			return false;
//			continue;
//		}
//		int pathLength = 0;
//		current = goal;
//		*mappingOutFile << "The Path : ";
//		while(current!=start){
//			pathLength++;
//			*mappingOutFile << "(" << current->getT() << "," << current->getY() << "," << current->getX() << ")" << " <-- ";
//			if(cameFrom.find(current) == cameFrom.end()){
//				printf("ASTAR :: ROUTING FAILURE\n");
//
//				for (int j = i; j < pathsWithCost.size(); ++j) {
//					pathsNotRouted->push_back(pathsWithCost[i].path);
//				}
////				return false;
////				continue;
//				break;
//			}
//
////			cgra->removeEdge(cameFrom[current],current);
////			if(cameFrom[current]->getT() == current->getT()){
////				assert(cameFrom[current]->getT() == current->getT());
//				if(cgraEdges->find(cameFrom[current]) != cgraEdges->end()){
////					*mappingOutFile << (*cgraEdges)[cameFrom[current]].size();
////					*mappingOutFile << " R" << current << " ";
////					std::vector<CGRANode*> *vec = &(cgraEdges.find(cameFrom[current])->second);
////					std::vector<CGRANode*> *vec = &cgraEdges[cameFrom[current]];
//
//					std::vector<CGRANode*>::iterator found = std::find((*cgraEdges)[cameFrom[current]].begin(), (*cgraEdges)[cameFrom[current]].end(), current);
//					if(found != (*cgraEdges)[cameFrom[current]].end()){
//						(*cgraEdges)[cameFrom[current]].erase(found);
//					}
//
//
////					assert(std::find((*cgraEdges)[cameFrom[current]].begin(),(*cgraEdges)[cameFrom[current]].end(),current) == (*cgraEdges)[cameFrom[current]].end());
//
////					*mappingOutFile << (*cgraEdges)[cameFrom[current]].size();
//				}
//
//				//Remove all edges used by the routing location
//				if((cameFrom[current]->getX() == current->getX())&&
//				   (cameFrom[current]->getY() == current->getY())
//				   ){
//
//				}
//				else{
//					std::map<CGRANode*, std::vector<CGRANode*> >::iterator cgraEdgeIter;
//					for(cgraEdgeIter = cgraEdges->begin(); cgraEdgeIter != cgraEdges->end(); cgraEdgeIter++){
//
//						if( (cgraEdgeIter->first->getX() == current->getX())&&(cgraEdgeIter->first->getY() == current->getY()) ){
//							continue;
//						}
//
//					    std::vector<CGRANode*>::iterator found = std::find(cgraEdges[cgraEdgeIter->first].begin(), cgraEdges[cgraEdgeIter->first].end(), current);
//					    if(found != cgraEdges[cgraEdgeIter->first].end()){
//						    (*cgraEdges)[cgraEdgeIter->first].erase(found);
//					    }
//
//					}
//				}
//
//
////			}
//
//			if( (current->getX() == cameFrom[current]->getX()) && (current->getY() == cameFrom[current]->getY()) ){
//
//			}
//			else{
//				current->setMappedDFGNode(pathsWithCost[i].parent);
//				pathsWithCost[i].parent->getRoutingLocs()->push_back(current);
//			}
//
//
//			current = cameFrom[current];
//		}
//		if(pathLength > maxPathLength){
//			maxPathLength = pathLength;
//		}
//
//		*mappingOutFile << "(" << current->getT() << "," << current->getY() << "," << current->getX() << ")" << "\n";
//		*mappingOutFile << "MAX SMART Path Length = " << maxPathLength << "\n";
////		*mappingOutFile << "\n";
//	}
//
//	return true;

//}
