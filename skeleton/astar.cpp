#include <assert.h>
#include "edge.h"
#include "astar.h"
#include "dfg.h"
#include <queue>


int AStar::heuristic(CGRANode* a, CGRANode* b) {
//	assert(a->getT() == b->getT());
	return abs(a->getX() - b->getX()) + abs(a->getY() - b->getY()) + 16*((b->getT() - a->getT() + MII)%MII);
}

CGRANode* AStar::AStarSearch(std::map<CGRANode*,std::vector<CGRAEdge> > graph,
						CGRANode* start,
						CGRANode* goal,
						std::map<std::pair<CGRANode*,Port>,std::pair<CGRANode*,Port>> *cameFrom,
						std::map<CGRANode*,int> *costSoFar,
						Port* endPort) {
//	assert(start->getT() == goal->getT());


	std::priority_queue<CGRANodeWithCost, std::vector<CGRANodeWithCost>, LessThanCGRANodeWithCost> frontier;
	std::vector<CGRAEdge> tempCGRAEdges;
	frontier.push(CGRANodeWithCost(start,0,TILE));

	cameFrom->clear();
	costSoFar->clear();

//	(*cameFrom)[std::make_pair(start,TILE)] = NULL;
	(*costSoFar)[start] = 0;

	CGRANode* current;
	Port currentPort;
	CGRANode* next;
	Port nextPort;

	int newCost = 0;
	int priority = 0;

	while(!frontier.empty()){
		current = frontier.top().Cnode;
		currentPort = frontier.top().port;
		frontier.pop();

		if(current == goal){
			break;
		}

		CameFromVerify(cameFrom);
		tempCGRAEdges = currDFG->getCGRA()->findCGRAEdges(current,currentPort,&graph);
		for (int i = 0; i < tempCGRAEdges.size(); ++i) {
			next = tempCGRAEdges[i].Dst;
			nextPort = tempCGRAEdges[i].DstPort;

			if(current->getT() == next->getT()){
				newCost = (*costSoFar)[current] + 1; //always graph.cost(current, next) = 1
			}else{
				newCost = (*costSoFar)[current] + 16*((next->getT() - current->getT() + MII)%MII);
			}

			if(costSoFar->find(next) != costSoFar->end()){
				if(newCost >= (*costSoFar)[next]){
					continue;
				}
			}
			(*costSoFar)[next] = newCost;
			priority = newCost + heuristic(goal,next);
			frontier.push(CGRANodeWithCost(next,priority,nextPort));
			(*cameFrom)[std::make_pair(next,nextPort)] = std::make_pair(current,currentPort);
		}
	}

	*endPort = currentPort;
	return current;
}

bool AStar::Route(dfgNode* currNode,
				  std::vector<dfgNode*> parents,
//				  std::vector<std::pair<CGRANode*, CGRANode*> > paths,
//				  std::vector<TreePath> treePaths,
				  std::vector<CGRANode*> dests,
			      std::map<CGRANode*,std::vector<CGRAEdge> >* cgraEdges,
				  std::vector<std::pair<CGRANode*,CGRANode*> > *pathsNotRouted,
				  bool* deadEndReached) {

	CGRANode* start;
	CGRANode* goal;

	CGRANode* current;
	Port currPort;
	std::pair<CGRANode*,Port> currNodePortPair;

	CGRANode* end;
	Port endPort;

	dfgNode* currParent;
	std::vector<TreePath> treePaths;

	std::map<std::pair<CGRANode*,Port>,std::pair<CGRANode*,Port> > cameFrom;
	std::map<std::pair<CGRANode*,Port>,std::pair<CGRANode*,Port> >::iterator cameFromIt;
	bool cameFromSearch = false;

	std::map<CGRANode*,int> costSoFar;
	bool routingComplete = false;

	std::vector<CGRAEdge> tempCGRAEdges;

	struct pathWithCost{
		std::pair<CGRANode*, CGRANode*> path;
		int cost;
		dfgNode* parent;
		pathWithCost(std::pair<CGRANode*, CGRANode*> path,int cost,dfgNode* parent) : path(path), cost(cost), parent(parent){}
	};

	struct treePathWithCost{
		TreePath tp;
		int cost;
		dfgNode* parent;
		treePathWithCost(TreePath tp,int cost,dfgNode* parent) : tp(tp), cost(cost) ,parent(parent){}
	};

	struct LessThanPathWithCost{
	    bool operator()(pathWithCost const & p1, pathWithCost const & p2) {
	        // return "true" if "p1" is ordered before "p2", for example:
	        return p1.cost < p2.cost;
	    }
	};

	struct LessThanTreePathWithCost{
	    bool operator()(treePathWithCost const & p1, treePathWithCost const & p2) {
	        // return "true" if "p1" is ordered before "p2", for example:
	        return p1.cost < p2.cost;
	    }
	};

	std::vector<pathWithCost> pathsWithCost;
	std::vector<treePathWithCost> treePathsWithCost;
	bool localDeadEndReached = false;


	pathsNotRouted->clear();

	//Clearing all the routing associated with current node on previous routing attempts
	for (int i = 0; i < currNode->getAncestors().size(); ++i) {
		currParent = currNode->getAncestors()[i];
		(*currNode->getTreeBasedRoutingLocs())[currParent].clear();
		(*currNode->getTreeBasedGoalLocs())[currParent].clear();
	}

	//Creating TreePaths Data Structure
	for (int i = 0; i < dests.size(); ++i) {
		treePaths.push_back(currDFG->createTreePath(parents[i],dests[i]));
	}

	//Route estimation
	for (int i = 0; i < treePaths.size(); ++i) {

		//Sorting TreePath Paths
		for (int j = 0; j < treePaths[i].sources.size(); ++j) {
			start = treePaths[i].sources[j];
			start = currDFG->getCGRA()->getCGRANode(start->getT(),start->getY(),start->getX());
			goal = treePaths[i].dest;
			goal = currDFG->getCGRA()->getCGRANode(goal->getT(),goal->getY(),goal->getX());

//			if(start != currDFG->getCGRA()->getCGRANode(start->getT(),start->getY(),start->getX())){
//				start = currDFG->getCGRA()->getCGRANode(start->getT(),start->getY(),start->getX());
//			}

//			assert(start == currDFG->getCGRA()->getCGRANode(start->getT(),start->getY(),start->getX()));
//			assert(goal == currDFG->getCGRA()->getCGRANode(goal->getT(),goal->getY(),goal->getX()));

			end = AStarSearch(*cgraEdges,start,goal,&cameFrom, &costSoFar,&endPort);
			if(end != goal){
				continue;
			}
			pathsWithCost.push_back(pathWithCost(std::make_pair(start,goal),costSoFar[goal],parents[i]));
		}

		std::sort(pathsWithCost.begin(),pathsWithCost.end(),LessThanPathWithCost());

		if(pathsWithCost.size() == 0){
			errs() << "SMARTRouteEst :: " << "Path is not routed = " << start->getName() << " to " << goal->getName() << "\n";
			errs() << "SMARTRouteEst :: " << "But Routed until : " << (end)->getName() << "\n";
			localDeadEndReached = reportDeadEnd(end,endPort,currNode,cgraEdges);
			if(deadEndReached != NULL){
				*deadEndReached = localDeadEndReached;
			}
			pathsNotRouted->push_back(std::make_pair(start,goal));
			return false;
//			continue;
		}
		else{
			treePaths[i].bestSource = pathsWithCost[0].path.first;
			treePaths[i].bestCost = pathsWithCost[0].cost;
		}

//		start = paths[i].first;
//		goal = paths[i].second;
//		end = AStarSearch(*cgraEdges,start,goal,&cameFrom, &costSoFar);
//		if(end != goal){
////			return false;
//			errs() << "SMARTRouteEst :: " << "Path is not routed = " << start->getName() << " to " << goal->getName() << "\n";
//			errs() << "SMARTRouteEst :: " << "But Routed until : " << (end)->getName() << "\n";
//			pathsNotRouted->push_back(paths[i]);
//
//			cameFrom.clear();
//			costSoFar.clear();
//
//			continue;
//		}

		treePathsWithCost.push_back(treePathWithCost(treePaths[i],treePaths[i].bestCost,parents[i]));
//		pathsWithCost.push_back(pathWithCost(std::make_pair(treePaths[i].bestSource,treePaths[i].dest),,parents[i]));
	}

	cameFrom.clear();
	costSoFar.clear();

	std::sort(treePathsWithCost.begin(),treePathsWithCost.end(),LessThanTreePathWithCost());

	//Real Routing happens here

	for (int i = 0; i < treePathsWithCost.size(); ++i) {
		goal = treePathsWithCost[i].tp.dest;
		goal = currDFG->getCGRA()->getCGRANode(goal->getT(),goal->getY(),goal->getX());
		currParent = treePathsWithCost[i].parent;
		errs() << "Nodes cleared for NodeIdx=" << currNode->getIdx() << ", ParentIdx=" << currParent->getIdx() << "\n";


		assert(treePathsWithCost[i].tp.sources.size() != 0);

		//Check for the shortest-routable path
		pathsWithCost.clear();
		for (int j = 0; j < treePathsWithCost[i].tp.sources.size(); ++j) {
			start = treePathsWithCost[i].tp.sources[j];
			start = currDFG->getCGRA()->getCGRANode(start->getT(),start->getY(),start->getX());
			errs() << "AStar Search starts...\n";
			end = AStarSearch(*cgraEdges,start,goal,&cameFrom, &costSoFar,&endPort);
			errs() << "AStar Search ends...\n";
			if(end != goal){
				*mappingOutFile << "Routing Failed : " << start->getName() << "->" << goal->getName() << ",only routed to " << end->getName() << std::endl;
				continue;
			}
			pathsWithCost.push_back(pathWithCost(std::make_pair(start,goal),costSoFar[goal],parents[i]));
		}

		std::sort(pathsWithCost.begin(),pathsWithCost.end(),LessThanPathWithCost());

		//Order the routable paths in the order into the treePaths data structure
		treePaths[i].sources.clear();
		for (int j = 0; j < pathsWithCost.size(); ++j) {
			treePaths[i].sources.push_back(pathsWithCost[j].path.first);
		}

		if(treePaths[i].sources.size() == 0){
			*mappingOutFile << "routing not completed as it only routed until : " << end->getName() << std::endl;
			pathsNotRouted->push_back(std::make_pair(start,goal));
			localDeadEndReached = reportDeadEnd(end,endPort,currNode,cgraEdges);
			if(deadEndReached != NULL){
				*deadEndReached = localDeadEndReached;
			}
			return false;
		}

		std::pair<dfgNode*,dfgNode*> currSourcePath;
		//start routing
		routingComplete = false;
		for (int j = 0; j < treePaths[i].sources.size(); ++j) {
			start = treePaths[i].sources[j];
			start = currDFG->getCGRA()->getCGRANode(start->getT(),start->getY(),start->getX());
			goal = treePaths[i].dest;
			goal = currDFG->getCGRA()->getCGRANode(goal->getT(),goal->getY(),goal->getX());
			currParent = parents[i];
			currSourcePath = treePaths[i].sourcePaths[start];

			*mappingOutFile << "start = (" << start->getT() << "," << start->getY() << "," << start->getX() << ")\n";
			*mappingOutFile << "goal = (" << goal->getT() << "," << goal->getY() << "," << goal->getX() << ")\n";

			end = AStarSearch(*cgraEdges,start,goal,&cameFrom, &costSoFar,&endPort);
//			CameFromVerify(&cameFrom);


			if(end != goal){
				errs() << "SMARTRouteReal :: " << "Path is not routed = " << start->getName() << " to " << goal->getName() << "\n";
				errs() << "SMARTRouteReal :: " << "But Routed until : " << (end)->getName() << "\n";
				continue;
			}
			else{
				routingComplete = true;
				break;
			}
		}


		if(!routingComplete){
			*mappingOutFile << "routing not completed as it only routed until : " << end->getName() << std::endl;
			pathsNotRouted->push_back(std::make_pair(start,goal));
			return false;
		}

		int pathLength = 0;
		int SMARTpathLength = 0;
		int regConnections = 0;
		bool SMARTPathEnd = true;
		(*currNode->getTreeBasedRoutingLocs())[currParent].clear();
		(*currNode->getTreeBasedGoalLocs())[currParent].clear();

		current = goal;

		(*currNode->getTreeBasedGoalLocs())[currParent].push_back(goal);
		if(goal->equals(4,3,0)){
			errs() << "%% Pusing Goal ::";
			errs() << ", nodeIdx=" << currNode->getIdx();
			errs() << ", nodeASAPLevel=" << currNode->getASAPnumber();
			errs() << ", cgraEdgeSize=" << (*cgraEdges)[goal].size();
			errs() << ", util = " << currDFG->findUtilTreeRoutingLocs(goal,currNode);
			errs() << "\n";
		}

		(*currNode->getSourceRoutingPath())[currParent] = currSourcePath;

		if(cameFrom.size() > 0){
			cameFromSearch = false;
			for(cameFromIt = cameFrom.begin(); cameFromIt != cameFrom.end(); ++cameFromIt){
				if(cameFromIt->second.first == start){
					cameFromSearch = true;
					break;
				}
			}
			assert(cameFromSearch);
		}

		*mappingOutFile << "The Path : ";
		while(current!=start){
			*mappingOutFile << "(" << current->getT() << "," << current->getY() << "," << current->getX() << ")" << " <-- ";

//			assert(cameFrom.find(current) != cameFrom.end());
			cameFromSearch = false;
			for(cameFromIt = cameFrom.begin(); cameFromIt != cameFrom.end(); cameFromIt++){
				if(cameFromIt->first.first == current){
					currPort = cameFromIt->first.second;
					cameFromSearch = true;
					break;
				}
			}
			assert(cameFromSearch);
			currNodePortPair = std::make_pair(current,currPort);

			//Adding Routing Locs to form a multicast tree in the future
//			if(cameFrom[current] != start){

				//TODO :: //change this to record the port also.
				assert(current == currDFG->getCGRA()->getCGRANode(current->getT(),current->getY(),current->getX()));
				(*currNode->getTreeBasedRoutingLocs())[currParent].push_back(current);

//			}


//			assert(cgraEdges->find(cameFrom[current]) != cgraEdges->end());

//			CameFromVerify(&cameFrom);
			tempCGRAEdges = currDFG->getCGRA()->findCGRAEdges(cameFrom[currNodePortPair].first,cameFrom[currNodePortPair].second,cgraEdges);
			assert(tempCGRAEdges.size() != 0);

//			std::vector<CGRANode*>::iterator found = std::find((*cgraEdges)[cameFrom[current]].begin(), (*cgraEdges)[cameFrom[current]].end(), current);
			for (int j = 0; j < tempCGRAEdges.size(); ++j) {
				if(tempCGRAEdges[j].Dst == current){
					tempCGRAEdges[j].mappedDFGEdge = currDFG->findEdge(currParent,currNode);
					break;
				}
			}


			//Logging - Begin
			if(current->getT() != cameFrom[currNodePortPair].first->getT()){
				if(!SMARTPathEnd){
					SMARTPathEnd = true;
					if(SMARTPathHist.find(SMARTpathLength) == SMARTPathHist.end()){
						SMARTPathHist[SMARTpathLength] = 1;
						if(currParent->getNode() != NULL){
							if(currParent->getNode()->getOpcode() == Instruction::Br){
								SMARTPredicatePathHist[SMARTpathLength] = 1;
							}
						}

					}
					else{
						SMARTPathHist[SMARTpathLength]++;
						if(currParent->getNode() != NULL){
							if(currParent->getNode()->getOpcode() == Instruction::Br){
								SMARTPredicatePathHist[SMARTpathLength]++;
							}
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
			current = cameFrom[currNodePortPair].first;
		}

		//Logging - SMART Path Length
		if(SMARTpathLength != 0){
			if(SMARTPathHist.find(SMARTpathLength) == SMARTPathHist.end()){
				SMARTPathHist[SMARTpathLength] = 1;
				if(currParent->getNode() != NULL){
					if(currParent->getNode()->getOpcode() == Instruction::Br){
						SMARTPredicatePathHist[SMARTpathLength] = 1;
					}
				}
			}
			else{
				SMARTPathHist[SMARTpathLength]++;
				if(currParent->getNode() != NULL){
					if(currParent->getNode()->getOpcode() == Instruction::Br){
						SMARTPredicatePathHist[SMARTpathLength]++;
					}
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
	}
	errs() << "AStar search done...\n";
	return true;
}


//	for (int i = 0; i < pathsWithCost.size(); ++i) {
//		start = pathsWithCost[i].path.first;
//		goal = pathsWithCost[i].path.second;
//		currParent = pathsWithCost[i].parent;
//		(*currNode->getTreeBasedRoutingLocs())[currParent].clear();
//
//		*mappingOutFile << "start = (" << start->getT() << "," << start->getY() << "," << start->getX() << ")\n";
//		*mappingOutFile << "goal = (" << goal->getT() << "," << goal->getY() << "," << goal->getX() << ")\n";
//
//		end = AStarSearch(*cgraEdges,start,goal,&cameFrom, &costSoFar);
//		if(end != goal){
////			for (int j = i; j < pathsWithCost.size(); ++j) {
//			errs() << "SMARTRouteReal :: " << "Path is not routed = " << start->getName() << " to " << goal->getName() << "\n";
//			errs() << "SMARTRouteReal :: " << "But Routed until : " << (end)->getName() << "\n";
//			pathsNotRouted->push_back(pathsWithCost[i].path);
////			}
////			return false;
//			continue;
//		}
//
//		int pathLength = 0;
//		int SMARTpathLength = 0;
//		int regConnections = 0;
//		bool SMARTPathEnd = true;
//
//		current = goal;
//		*mappingOutFile << "The Path : ";
//		while(current!=start){
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
//			//Adding Routing Locs to form a multicast tree in the future
//			if(cameFrom[current] != start){
//				(*currNode->getTreeBasedRoutingLocs())[currParent].push_back(cameFrom[current]);
//			}
//
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
////					assert(std::find((*cgraEdges)[cameFrom[current]].begin(),(*cgraEdges)[cameFrom[current]].end(),current) == (*cgraEdges)[cameFrom[current]].end());
//
////					*mappingOutFile << (*cgraEdges)[cameFrom[current]].size();
//				}
////			}
//
//			//Logging - Begin
//			if(current->getT() != cameFrom[current]->getT()){
//				if(!SMARTPathEnd){
//					SMARTPathEnd = true;
//					if(SMARTPathHist.find(SMARTpathLength) == SMARTPathHist.end()){
//						SMARTPathHist[SMARTpathLength] = 1;
//						if(currParent->getNode()->getOpcode() == Instruction::Br){
//							SMARTPredicatePathHist[SMARTpathLength] = 1;
//						}
//
//					}
//					else{
//						SMARTPathHist[SMARTpathLength]++;
//						if(currParent->getNode()->getOpcode() == Instruction::Br){
//							SMARTPredicatePathHist[SMARTpathLength]++;
//						}
//					}
//
//					if(SMARTpathLength > maxSMARTPathLength){
//						maxSMARTPathLength = SMARTpathLength;
//					}
//					SMARTpathLength = 0;
//				}
//				else{
//					assert(SMARTpathLength == 0);
//				}
//				regConnections++;
//			}
//			else{
//				SMARTPathEnd = false;
//				SMARTpathLength++;
//			}
//			//Logging - End
//
//			pathLength++;
//			current = cameFrom[current];
//		}
//
//		//Logging - SMART Path Length
//		if(SMARTpathLength != 0){
//			if(SMARTPathHist.find(SMARTpathLength) == SMARTPathHist.end()){
//				SMARTPathHist[SMARTpathLength] = 1;
//				if(currParent->getNode()->getOpcode() == Instruction::Br){
//					SMARTPredicatePathHist[SMARTpathLength] = 1;
//				}
//			}
//			else{
//				SMARTPathHist[SMARTpathLength]++;
//				if(currParent->getNode()->getOpcode() == Instruction::Br){
//					SMARTPredicatePathHist[SMARTpathLength]++;
//				}
//			}
//
//			if(SMARTpathLength > maxSMARTPathLength){
//				maxSMARTPathLength = SMARTpathLength;
//			}
//		}
//
//		//Logging - Path Histogram
//		if(PathHist.find(pathLength) == PathHist.end()){
//			PathHist[pathLength] = 1;
//			PathSMARTPathCount[pathLength] = regConnections;
//		}
//		else{
//			PathHist[pathLength]++;
//			PathSMARTPathCount[pathLength] += regConnections;
//		}
//		PathSMARTPaths[pathLength].push_back(regConnections);
//
//
//
//		if(pathLength > maxPathLength){
//			maxPathLength = pathLength;
//		}
//
//		*mappingOutFile << "(" << current->getT() << "," << current->getY() << "," << current->getX() << ")" << "\n";
////		*mappingOutFile << "\n";
//	}
//
//	return true;
//}

bool AStar::EMSRoute(dfgNode* currNode,
		   	   	   	 std::vector<dfgNode*> parents,
//			   std::vector<std::pair<CGRANode*,CGRANode*> > paths,
//			   std::vector<TreePath> treePaths,
					 std::vector<CGRANode*> dests,
					 std::map<CGRANode*,std::vector<CGRAEdge> >* cgraEdges,
					 std::vector<std::pair<CGRANode*,CGRANode*> > *pathsNotRouted,
					 bool* deadEndReached) {

	CGRANode* start;
	CGRANode* goal;

	CGRANode* current;
	Port currPort;
	CGRANode* end;
	Port endPort;
	std::pair<CGRANode*,Port> currNodePortPair;
	std::vector<CGRAEdge> tempCGRAEdges;

	CGRANode* TwoNodesBeforeCurrent;

	dfgNode* currParent;
	std::vector<TreePath> treePaths;

//	std::map<CGRANode*,CGRANode*> cameFrom;
	std::map<std::pair<CGRANode*,Port>,std::pair<CGRANode*,Port> > cameFrom;
	std::map<std::pair<CGRANode*,Port>,std::pair<CGRANode*,Port> >::iterator cameFromIt;
	bool cameFromSearch = false;


	std::map<CGRANode*,int> costSoFar;
	bool routingComplete = false;
	bool localDeadEndReached = false;

	struct pathWithCost{
		std::pair<CGRANode*, CGRANode*> path;
		int cost;
		dfgNode* parent;
		pathWithCost(){}
		pathWithCost(dfgNode* parent, std::pair<CGRANode*, CGRANode*> path,int cost) : parent(parent), path(path), cost(cost){}
	};

	struct treePathWithCost{
		TreePath tp;
		int cost;
		dfgNode* parent;
		treePathWithCost(){}
		treePathWithCost(TreePath tp, int cost, dfgNode* parent) : tp(tp), cost(cost), parent(parent){}
	};

	struct LessThanPathWithCost{
	    bool operator()(pathWithCost const & p1, pathWithCost const & p2) {
	        // return "true" if "p1" is ordered before "p2", for example:
	        return p1.cost < p2.cost;
	    }
	};

	struct LessThanTreePathWithCost{
		bool operator()(treePathWithCost const & p1, treePathWithCost const & p2){
			return p1.cost < p2.cost;
		}
	};

	std::vector<pathWithCost> pathsWithCost;
	std::priority_queue<treePathWithCost, std::vector<treePathWithCost>, LessThanTreePathWithCost> pathsPrQueue;
	std::priority_queue<treePathWithCost, std::vector<treePathWithCost>, LessThanTreePathWithCost> pathsPrQueueTemp;

	pathsNotRouted->clear();

	//Clearing all the routing associated with current node on previous routing attempts
	for (int i = 0; i < currNode->getAncestors().size(); ++i) {
		currParent = currNode->getAncestors()[i];
		(*currNode->getTreeBasedRoutingLocs())[currParent].clear();
		(*currNode->getTreeBasedGoalLocs())[currParent].clear();
	}

	//Creating TreePaths Data Structure
	for (int i = 0; i < dests.size(); ++i) {
		treePaths.push_back(currDFG->createTreePath(parents[i],dests[i]));
	}

	//Route estimation
		for (int i = 0; i < treePaths.size(); ++i) {

			//Sorting TreePath Paths
			for (int j = 0; j < treePaths[i].sources.size(); ++j) {
				start = treePaths[i].sources[j];
				goal = treePaths[i].dest;
				end = AStarSearchEMS(*cgraEdges,start,goal,&cameFrom, &costSoFar, &endPort);
				if(end != goal){
					continue;
				}
				pathsWithCost.push_back(pathWithCost(parents[i],std::make_pair(start,goal),costSoFar[goal]));
			}

			std::sort(pathsWithCost.begin(),pathsWithCost.end(),LessThanPathWithCost());

			if(pathsWithCost.size() == 0){
				errs() << "SMARTRouteEst :: " << "Path is not routed = " << start->getName() << " to " << goal->getName() << "\n";
				errs() << "SMARTRouteEst :: " << "But Routed until : " << (end)->getName() << "\n";
				localDeadEndReached = reportDeadEnd(end, endPort,currNode,cgraEdges);
				if(deadEndReached != NULL){
					*deadEndReached = localDeadEndReached;
				}
				pathsNotRouted->push_back(std::make_pair(start,goal));
				return false;
	//			continue;
			}
			else{
				treePaths[i].bestSource = pathsWithCost[0].path.first;
				treePaths[i].bestCost = pathsWithCost[0].cost;
			}

	//		start = paths[i].first;
	//		goal = paths[i].second;
	//		end = AStarSearch(*cgraEdges,start,goal,&cameFrom, &costSoFar);
	//		if(end != goal){
	////			return false;
	//			errs() << "SMARTRouteEst :: " << "Path is not routed = " << start->getName() << " to " << goal->getName() << "\n";
	//			errs() << "SMARTRouteEst :: " << "But Routed until : " << (end)->getName() << "\n";
	//			pathsNotRouted->push_back(paths[i]);
	//
	//			cameFrom.clear();
	//			costSoFar.clear();
	//
	//			continue;
	//		}

			pathsPrQueue.push(treePathWithCost(treePaths[i],treePaths[i].bestCost,parents[i]));
	//		pathsWithCost.push_back(pathWithCost(std::make_pair(treePaths[i].bestSource,treePaths[i].dest),,parents[i]));
		}

//	//Route estimation
//	for (int i = 0; i < treePaths.size(); ++i) {
//		start = paths[i].first;
//		goal = paths[i].second;
//		end = AStarSearch(*cgraEdges,start,goal,&cameFrom, &costSoFar);
//		if(end != goal){
////			return false;
//			errs() << "EMSRoute :: " << "Init Route est, Path is not routed = " << start->getName() << " to " << goal->getName() << "\n";
//			errs() << "EMSRoute :: " << "But Routed until : " << (end)->getName() << "\n";
//			pathsNotRouted->push_back(paths[i]);
//			return false;
////			continue;
//		}
//
//		pathsPrQueue.push(pathWithCost(parents[i], paths[i],costSoFar[goal]));
////		pathsWithCost.push_back(pathWithCost(parents[i], paths[i],costSoFar[goal]));
//	}

//	std::sort(pathsWithCost.begin(),pathsWithCost.end(),LessThanPathWithCost());

	//Real Routing happens here

	treePathWithCost currPath;

	*mappingOutFile << "Routing the set of paths....\n";
	while(!pathsPrQueue.empty()){
		currPath = pathsPrQueue.top();
		pathsPrQueue.pop();

		goal = currPath.tp.dest;
		currParent = currPath.parent;
		errs() << "Nodes cleared for NodeIdx=" << currNode->getIdx() << ", ParentIdx=" << currParent->getIdx() << "\n";


		assert(currPath.tp.sources.size() != 0);

		//Check for the shortest-routable path
		pathsWithCost.clear();
		for (int j = 0; j < currPath.tp.sources.size(); ++j) {
			start = currPath.tp.sources[j];
			end = AStarSearchEMS(*cgraEdges,start,goal,&cameFrom, &costSoFar, &endPort);
			if(end != goal){
				*mappingOutFile << "Routing Failed : " << start->getName() << "->" << goal->getName() << ",only routed to " << end->getName() << std::endl;
				continue;
			}
			pathsWithCost.push_back(pathWithCost(currPath.parent,std::make_pair(start,goal),costSoFar[goal]));
		}

		std::sort(pathsWithCost.begin(),pathsWithCost.end(),LessThanPathWithCost());

		//Order the routable paths in the order into the treePaths data structure
		currPath.tp.sources.clear();
		for (int j = 0; j < pathsWithCost.size(); ++j) {
			currPath.tp.sources.push_back(pathsWithCost[j].path.first);
		}


//		treePaths[i].sources.clear();
//		for (int j = 0; j < pathsWithCost.size(); ++j) {
//			treePaths[i].sources.push_back(pathsWithCost[j].path.first);
//		}

		if(currPath.tp.sources.size() == 0){
			*mappingOutFile << "routing not completed as it only routed until : " << end->getName() << std::endl;
			pathsNotRouted->push_back(std::make_pair(start,goal));
			localDeadEndReached = reportDeadEnd(end,endPort,currNode,cgraEdges);
			if(deadEndReached != NULL){
				*deadEndReached = localDeadEndReached;
			}
			return false;
		}

		//start routing
		routingComplete = false;
		for (int j = 0; j < currPath.tp.sources.size(); ++j) {
			start = currPath.tp.sources[j];
			goal = currPath.tp.dest;
			currParent = currPath.parent;

			*mappingOutFile << "start = (" << start->getT() << "," << start->getY() << "," << start->getX() << ")\n";
			*mappingOutFile << "goal = (" << goal->getT() << "," << goal->getY() << "," << goal->getX() << ")\n";

			end = AStarSearchEMS(*cgraEdges,start,goal,&cameFrom, &costSoFar,&endPort);
			if(end != goal){
				errs() << "EMSRouteReal :: " << "Path is not routed = " << start->getName() << " to " << goal->getName() << "\n";
				errs() << "EMSRouteReal :: " << "But Routed until : " << (end)->getName() << "\n";
				continue;
			}
			else{
				routingComplete = true;
				break;
			}
		}

		if(!routingComplete){
			*mappingOutFile << "routing not completed as it only routed until : " << end->getName() << std::endl;
			pathsNotRouted->push_back(std::make_pair(start,goal));
			return false;
		}

		int pathLength = 0;
		int SMARTpathLength = 0;
		int regConnections = 0;
		bool SMARTPathEnd = true;
		(*currNode->getTreeBasedRoutingLocs())[currParent].clear();
		(*currNode->getTreeBasedGoalLocs())[currParent].clear();

		current = goal;

		(*currNode->getTreeBasedGoalLocs())[currParent].push_back(goal);
		if(goal->equals(4,3,0)){
			errs() << "%% Pusing Goal ::";
			errs() << ", nodeIdx=" << currNode->getIdx();
			errs() << ", nodeASAPLevel=" << currNode->getASAPnumber();
			errs() << ", cgraEdgeSize=" << (*cgraEdges)[goal].size();
			errs() << ", util = " << currDFG->findUtilTreeRoutingLocs(goal,currNode);
			errs() << "\n";
		}

//		start = currPath.path.first;
//		goal = currPath.path.second;
//
//		if(AStarSearch(*cgraEdges,start,goal,&cameFrom, &costSoFar) != goal){
//			errs() << "EMSRoute :: " << "Real Routing,Path is not routed = " << start->getName() << " to " << goal->getName() << "\n";
//			pathsNotRouted->push_back(currPath.path);
//			return false;
////			continue;
//		}
//
//		int pathLength = 0;
//		current = goal;
		*mappingOutFile << "The Path : ";

		while(current!=start){
			*mappingOutFile << "(" << current->getT() << "," << current->getY() << "," << current->getX() << ")" << " <-- ";

//			assert(cameFrom.find(current) != cameFrom.end());
			cameFromSearch = false;
			for(cameFromIt = cameFrom.begin(); cameFromIt != cameFrom.end(); cameFromIt++){
				if(cameFromIt->first.first == current){
					currPort = cameFromIt->first.second;
					cameFromSearch = true;
					break;
				}
			}
			assert(cameFromSearch);
			currNodePortPair = std::make_pair(current,currPort);

			//Adding Routing Locs to form a multicast tree in the future
//			if(cameFrom[current] != start){
				//TODO :: add the ports to getTreeBasedRoutingLocs
				(*currNode->getTreeBasedRoutingLocs())[currParent].push_back(cameFrom[currNodePortPair].first);
//			}

//				assert(cgraEdges->find(cameFrom[current]) != cgraEdges->end());
				tempCGRAEdges = currDFG->getCGRA()->findCGRAEdges(cameFrom[currNodePortPair].first,cameFrom[currNodePortPair].second,cgraEdges);
				assert(tempCGRAEdges.size() != 0);


//					std::vector<CGRANode*>::iterator found = std::find((*cgraEdges)[cameFrom[current]].begin(), (*cgraEdges)[cameFrom[current]].end(), current);

//					if(found != (*cgraEdges)[cameFrom[current]].end()){
//						(*cgraEdges)[cameFrom[current]].erase(found);
//					}

				for (int j = 0; j < tempCGRAEdges.size(); ++j) {
					if(tempCGRAEdges[j].Dst == current){
						tempCGRAEdges[j].mappedDFGEdge = currDFG->findEdge(currParent,currNode);
						break;
					}
				}

				if(current!=goal){
					//Remove other edges
					std::map<CGRANode*, std::vector<CGRAEdge> >::iterator cgraEdgeIter;
					for(cgraEdgeIter = cgraEdges->begin(); cgraEdgeIter != cgraEdges->end(); cgraEdgeIter++){
						if(cameFrom[currNodePortPair].first != cgraEdgeIter->first){
							if( (cgraEdgeIter->first->getX() == current->getX())&&(cgraEdgeIter->first->getY() == current->getY()) ){
								continue;
							}

							tempCGRAEdges = currDFG->getCGRA()->findCGRAEdges(cameFrom[currNodePortPair].first,cameFrom[currNodePortPair].second,cgraEdges);
							assert(tempCGRAEdges.size() != 0);
							for (int j = 0; j < tempCGRAEdges.size(); ++j) {
								if(tempCGRAEdges[j].Dst == current){
									tempCGRAEdges[j].mappedDFGEdge = currDFG->findEdge(currParent,currNode);
									*mappingOutFile << "DEBUG :: " << "removing edge=" << tempCGRAEdges[j].Src->getName() << " to " << tempCGRAEdges[j].Dst->getName() << std::endl;
									break;
								}
							}



//							std::vector<CGRANode*>::iterator found = std::find((*cgraEdges)[cgraEdgeIter->first].begin(), (*cgraEdges)[cgraEdgeIter->first].end(), current);
//							if(found != (*cgraEdges)[cgraEdgeIter->first].end()){
//								*mappingOutFile << "DEBUG :: " << "removing edge=" << cgraEdgeIter->first->getName() << " to " << (*found)->getName();
//								*mappingOutFile << " Number of edges=" << (*cgraEdges)[cgraEdgeIter->first].size() << " reduced to ";
//								(*cgraEdges)[cgraEdgeIter->first].erase(found);
//								*mappingOutFile << (*cgraEdges)[cgraEdgeIter->first].size() << "\n";
//							}


						}
					}
				}

//				}

			if(cameFrom[currNodePortPair].first != start){
				TwoNodesBeforeCurrent = cameFrom[cameFrom[currNodePortPair]].first;
				if((TwoNodesBeforeCurrent->getY() == cameFrom[currNodePortPair].first->getY())&&
				   (TwoNodesBeforeCurrent->getX() == cameFrom[currNodePortPair].first->getX())&&
				   (cameFrom[currNodePortPair].first->getY() == current->getY())&&
				   (cameFrom[currNodePortPair].first->getX() == current->getX())){

				}
				else{
					cameFrom[currNodePortPair].first->setMappedDFGNode(currPath.parent);
				}
			}
			current = cameFrom[currNodePortPair].first;
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

			//Use information the newly routed paths
			currPath.tp = currDFG->createTreePath(currPath.parent,currPath.tp.dest);

			pathsPrQueueTemp.push(currPath);
		}

		while(!pathsPrQueueTemp.empty()){
			currPath = pathsPrQueueTemp.top();
			pathsPrQueueTemp.pop();

			pathsWithCost.clear();
			for (int i = 0; i < currPath.tp.sources.size(); ++i) {
				start = currPath.tp.sources[i];
				goal = currPath.tp.dest;
				end = AStarSearchEMS(*cgraEdges,start,goal,&cameFrom, &costSoFar,&endPort);
				if(end != goal){
					continue;
				}
				pathsWithCost.push_back(pathWithCost(currPath.parent,std::make_pair(start,goal),costSoFar[goal]));
			}

			std::sort(pathsWithCost.begin(),pathsWithCost.end(),LessThanPathWithCost());

			if(pathsWithCost.size() == 0){
				errs() << "EMSRouteEst :: " << "Path is not routed = " << start->getName() << " to " << goal->getName() << "\n";
				errs() << "EMSRouteEst :: " << "But Routed until : " << (end)->getName() << "\n";
				localDeadEndReached = reportDeadEnd(end,endPort,currNode,cgraEdges);
				if(deadEndReached != NULL){
					*deadEndReached = localDeadEndReached;
				}
				pathsNotRouted->push_back(std::make_pair(start,goal));
				return false;
	//			continue;
			}
			else{
				currPath.tp.bestSource = pathsWithCost[0].path.first;
				currPath.tp.bestCost = pathsWithCost[0].cost;
			}

			pathsPrQueue.push(currPath);
		}
	}
	return true;
}

CGRANode* AStar::AStarSearchEMS(
		std::map<CGRANode*, std::vector<CGRAEdge> > graph,
		CGRANode* start,
		CGRANode* goal,
		std::map<std::pair<CGRANode*,Port>, std::pair<CGRANode*,Port>>* cameFrom,
		std::map<CGRANode*, int>* costSoFar,
		Port* endPort) {
	//	assert(start->getT() == goal->getT());

		std::priority_queue<CGRANodeWithCost, std::vector<CGRANodeWithCost>, LessThanCGRANodeWithCost> frontier;
		frontier.push(CGRANodeWithCost(start,0,TILE));

		cameFrom->clear();
		costSoFar->clear();

//		(*cameFrom)[std::make_pair(start,TILE)] = NULL;
		(*costSoFar)[start] = 0;

		CGRANode* current;
		Port currentPort;
		CGRANode* next;
		Port nextPort;
		std::vector<CGRAEdge> tempCGRAEdges;

		int newCost = 0;
		int priority = 0;

		while(!frontier.empty()){
			current = frontier.top().Cnode;
			currentPort = frontier.top().port;
			frontier.pop();
			if(current == goal){
				break;
			}


			tempCGRAEdges = currDFG->getCGRA()->findCGRAEdges(current,currentPort,&graph);
			for (int i = 0; i < tempCGRAEdges.size(); ++i) {
				next = tempCGRAEdges[i].Dst;
				nextPort = tempCGRAEdges[i].DstPort;

				if((current->getX() == next->getX())&&
				   (current->getY() == next->getY())){
					newCost = (*costSoFar)[current] + 1; //always graph.cost(current, next) = 1
				}else{
					if(next == goal){
						newCost = (*costSoFar)[current] + 0;
					}
					else{
						newCost = (*costSoFar)[current] + MII;
					}
				}

				if(costSoFar->find(next) != costSoFar->end()){
					if(newCost >= (*costSoFar)[next]){
						continue;
					}
				}
				(*costSoFar)[next] = newCost;
				priority = newCost + heuristic(goal,next);
				frontier.push(CGRANodeWithCost(next,priority,nextPort));
				(*cameFrom)[std::make_pair(next,nextPort)] = std::make_pair(current,currentPort);
			}
		}

		*endPort = currentPort;
		return current;
}

bool AStar::CameFromVerify(std::map<std::pair<CGRANode*,Port>,std::pair<CGRANode*,Port>> *cameFrom) {

	std::map<std::pair<CGRANode*,Port>,std::pair<CGRANode*,Port>>::iterator cameFromIt;

	for(cameFromIt = cameFrom->begin();
		cameFromIt != cameFrom->end();
		cameFromIt++){
		assert(cameFromIt->first.first == currDFG->getCGRA()->getCGRANode(cameFromIt->first.first->getT(),
																		  cameFromIt->first.first->getY(),
																		  cameFromIt->first.first->getX()));
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

bool AStar::reportDeadEnd(CGRANode* end,
		 	 	 	 	  Port endPort,
						  dfgNode* currNode,
						  std::map<CGRANode*,std::vector<CGRAEdge> >* cgraEdges) {

	int util = currDFG->findUtilTreeRoutingLocs(end,currNode);
	std::vector<CGRAEdge> tempCGRAEdges =  currDFG->getCGRA()->findCGRAEdges(end,endPort,cgraEdges);

	*mappingOutFile << "DeadEndStats :: CGRANode=" << end->getName();
	*mappingOutFile << ", utilization = " << std::to_string(util) << std::endl;
	errs() << "DeadEndStats :: CGRANode=" << end->getName() ;
	errs() << ", NodeASAPLevel=" << currNode->getASAPnumber();

//	errs() << ", CGRAEdgeSize=" << (*cgraEdges)[end].size();
	errs() << ", CGRAEdgeSize=" << tempCGRAEdges.size();

	errs() << "[";
	for (int i = 0; i < tempCGRAEdges.size(); ++i) {
		errs() << tempCGRAEdges[i].Dst->getName().c_str() << ",";
	}
	errs() << "]";
	errs() << ", MII =" << currDFG->getCGRA()->getMII();
	errs() << ", util = " << util << "\n";
	errs() << ", oriEdgesSize = " << end->originalEdgesSize << "\n";

	if(tempCGRAEdges.size() == 0){
		return true;
	}

//	if(util >= end->originalEdgesSize){
//		return true;
//	}
//	return false;

}
