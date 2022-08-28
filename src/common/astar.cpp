#include <morpherdfggen/common/astar.h>

#include <assert.h>
#include <queue>
#include <morpherdfggen/arch/CGRA.h>
#include <morpherdfggen/common/dfg.h>
#include <morpherdfggen/common/edge.h>


int AStar::heuristic(CGRANode* a, CGRANode* b) {
//	assert(a->getT() == b->getT());
	return abs(a->getX() - b->getX()) + abs(a->getY() - b->getY()) + 16*((b->getT() - a->getT() + MII)%MII);
}

CGRANode* AStar::AStarSearch(std::map<CGRANode*,std::vector<CGRAEdge> > graph,
						CGRANode* start,
						Port startPort,
						CGRANode* goal,
						int minkII,
						std::map<pathData,pathData,pathDataComparer> *cameFrom,
						std::map<CGRANode*,int> *costSoFar,
						Port* endPort,
						int SCpathLength,
						int SCpathTDimLength,
						int* endPathLength,
						int* endPathTDimLength) {
//	assert(start->getT() == goal->getT());


	std::priority_queue<CGRANodeWithCost, std::vector<CGRANodeWithCost>, LessThanCGRANodeWithCost> frontier;
	std::vector<CGRAEdge*> tempCGRAEdges;
	frontier.push(CGRANodeWithCost(start,0,startPort,SCpathLength,SCpathTDimLength));

	std::pair<CGRANode*,Port> nextPair;
	std::pair<CGRANode*,Port> currPair;

	std::map<std::pair<CGRANode*,Port>,std::pair<CGRANode*,Port>>::iterator cameFromIt;
	cameFrom->clear();
	costSoFar->clear();

//	std::map<CGRANode*,pathData> localCameFrom;
//	std::map<CGRANode*,Port> localCameFromPort;
//	std::map<CGRANode*,int> localSCpathLength;
//	std::map<CGRANode*,int> localSCpathTdimLength;



//	(*cameFrom)[std::make_pair(start,TILE)] = NULL;
	(*costSoFar)[start] = 0;

	CGRANode* current;
	Port currentPort;
	int currPathLength;
	int currPathTdimLength;
	CGRANode* next;
	Port nextPort;
	int nextCost;
	int nextPathLength;
	int nextPathTdimLength;

	int newCost = 0;
	int priority = 0;
	int maxEdgeCount = 0;
	switch (currDFG->getCGRA()->getArch()) {
		case ALL2ALL:
			maxEdgeCount = currDFG->getCGRA()->getXdim()*currDFG->getCGRA()->getYdim() + 1
						 + currDFG->getCGRA()->getRegsPerNode();
			break;
		default:
			maxEdgeCount = 5 + currDFG->getCGRA()->getRegsPerNode();
			break;
	}

	while(!frontier.empty()){
		current = frontier.top().Cnode;
		currentPort = frontier.top().port;
		currPathLength = frontier.top().SCpathLength;
		currPathTdimLength = frontier.top().SCpathTdimLength;
		frontier.pop();

		pathData currPathData(current,currentPort,currPathLength,currPathTdimLength);

//		outs() << "current=" << current->getName() ;
//		outs() << ",currentPort=" << currDFG->getCGRA()->getPortName(currentPort);
//		outs() << ",TLength=" << currPathTdimLength;
//		outs() << ",Length=" << currPathLength;
//		outs() << ",minKII=" << minkII;
////		if(current!=start){
////			outs() << "|camefrom=" << (*cameFrom)[currPathData].cnode->getName();
////			outs()  << "," << currDFG->getCGRA()->getPortName((*cameFrom)[currPathData].lastPort);
////		}
// 		outs() << "\n";

		if(minkII >= 0){
//			if((current == goal)&&((currPathTdimLength-1)/currDFG->getCGRA()->getMII()==minkII)){
			if((current == goal)&&((currPathTdimLength)==minkII)){
//				outs() << "currPathTdimLength=" << currPathTdimLength;
//				outs() << ",minkII=" << minkII << "\n";
				break;
			}
		}
		else{
			if(current == goal){
				break;
			}
		}

		if(minkII >= 0){ //break for paths violating latency gap
//			if((currPathTdimLength-1)/currDFG->getCGRA()->getMII()>minkII){
			if((currPathTdimLength)>minkII){
				continue;
			}
		}

		if(MaxSCpathLength != -1){ //break if the start it self is exceeding sc hop limit
			if(currPathLength >= MaxSCpathLength){
				continue;
			}
		}

//		CameFromVerify(cameFrom);
		tempCGRAEdges = currDFG->getCGRA()->findCGRAEdges(current,currentPort,&graph,goal);
		for (int i = 0; i < tempCGRAEdges.size(); ++i) {
			next = tempCGRAEdges[i]->Dst;
			nextPort = tempCGRAEdges[i]->DstPort;

			assert(next->getT() < MII);

			nextCost = next->originalEdgesSize + 1 - currDFG->getCGRA()->findCGRAEdges(next,nextPort,&graph).size();
			nextCost = maxEdgeCount - currDFG->getCGRA()->findCGRAEdges(next,nextPort,&graph).size();
			assert(nextCost >= 0);

#ifdef TREG_MUTEX
			//Use TREG as a last resource, since the mux change will block TILE output
			// Rendering the processing element useless when TREG comes out to the crossbar
			if(tempCGRAEdges[i]->SrcPort == TREG){
				nextCost = nextCost*100000;
			}
#endif

			//disble UE cost heuristic
//			nextCost = 1;

			if(current->getT() == next->getT()){
				newCost = (*costSoFar)[current] + nextCost; //always graph.cost(current, next) = 1
				nextPathLength = currPathLength + 1;
				nextPathTdimLength = currPathTdimLength;
			}else{
				newCost = (*costSoFar)[current] + nextCost + 1000*((next->getT() - current->getT() + MII)%MII);
				nextPathLength = 0;
				nextPathTdimLength = currPathTdimLength + 1;
			}

			if(newCost <= (*costSoFar)[current]){
				errs() << "next=" << next->getName();
				errs() << ",nextPort=" << currDFG->getCGRA()->getPortName(nextPort);
				errs() << ",nextCost=" << nextCost;// << "\n";
				errs() << ",start=" << start->getName();// << "\n";
				errs() << ",MII=" << MII;// << "\n";
				errs() << ",next->getT()=" << next->getT();// << "\n";
				errs() << ",current->getT()=" << current->getT() << "\n";
			}

			assert(newCost > (*costSoFar)[current]);

			if(costSoFar->find(next) != costSoFar->end()){
				if(newCost >= (*costSoFar)[next]){
					continue;
				}
			}
			(*costSoFar)[next] = newCost;
			priority = newCost + heuristic(goal,next);
			frontier.push(CGRANodeWithCost(next,priority,nextPort,nextPathLength, nextPathTdimLength));

//			nextPair = std::make_pair(next,nextPort);
			currPair = std::make_pair(current,currentPort);

//			if(cameFrom->find(nextPair) != cameFrom->end()){
//				errs() << "Faulty cameFrom::\n";
//				errs() << "next=" << next->getName() << ",curr=" << current->getName() << "\n";
//				for(cameFromIt = cameFrom->begin();
//					cameFromIt != cameFrom->end();
//					++cameFromIt){
//					errs() << cameFromIt->first.first->getName() << "<--" << cameFromIt->second.first->getName() << "\n";
//				}
//			}
//			assert(cameFrom->find(nextPair) == cameFrom->end());

			//TODO : DAC18
//			if(nextPort==R0) assert(currentPort == R0 || currentPort == NORTH);
//			if(nextPort==R1) assert(currentPort == R1 || currentPort == EAST);
//			if(nextPort==R2) assert(currentPort == R2 || currentPort == WEST);
//			if(nextPort==R3) assert(currentPort == R3 || currentPort == SOUTH);

			pathData nextPathData(next,nextPort,nextPathLength,nextPathTdimLength);
			(*cameFrom)[nextPathData]=currPathData;
//			localCameFrom[next] = tempPathData;
//			localCameFromPort[next] = nextPort;
//			localSCpathLength[next] = nextPathLength;
//			localSCpathTdimLength[next] = nextPathTdimLength;
//			(*cameFrom)[nextPair] = currPair;
		}
	}

//	std::map<CGRANode*,pathData>::iterator It;
//	pathData currPathData;
//	for(It = localCameFrom.begin(); It != localCameFrom.end(); It++){
//		currPathData = It->second;
//		pathData nextPathData(It->first,localCameFromPort[It->first],localSCpathLength[It->first], localSCpathTdimLength[It->first]);
////		nextPair = std::make_pair(It->first,localCameFromPort[It->first]);
////		pathData tempPathData(It->first,localCameFromPort[It->first],localSCpathLength[It->first]);
////		(*cameFrom)[nextPair] = currPair;
//		(*cameFrom)[nextPathData] = currPathData;
//	}

	*endPathLength = currPathLength;
	*endPort = currentPort;
	*endPathTDimLength = currPathTdimLength;
	return current;
}

int AStar::calckII(CGRANode* start, int startRT, CGRANode* goal, int goalRT){
//	outs() << "start" << start->getName() <<",startRT=" << startRT << ",goalRT=" << goalRT << ",goal=" << goal->getName() << "\n";
	assert(start->getT() == startRT%(this->MII));
	assert(goal->getT() == goalRT%(this->MII));
	int kII = goalRT - startRT;
	if(kII < 0) return -1;

	//Delete this is for testing
	return -1;

//	return kII/currDFG->getCGRA()->getMII();
	return kII;
}

bool AStar::Route(dfgNode* currNode,
				  std::vector<dfgNode*> parents,
//				  std::vector<std::pair<CGRANode*, CGRANode*> > paths,
//				  std::vector<TreePath> treePaths,
				  std::vector<CGRANode*>* dests,
				  std::map<CGRANode*,int>* destllmap,
			      std::map<CGRANode*,std::vector<CGRAEdge> >* cgraEdges,
				  std::vector<std::pair<CGRANode*,CGRANode*> > *pathsNotRouted,
				  CGRANode** chosenDest,
				  bool* deadEndReached) {

	CGRANode* start;
	Port startPort;
	int startPathLength;
	int startPathTdimLength;
	CGRANode* goal;

	CGRANode* current;
	Port currPort;
	int currPathLength;
	int currPathTdimLength;
//	pathData currPathData;
//	std::pair<CGRANode*,Port> currNodePortPair;

	CGRANode* end;
	Port endPort;
	int endPathLength;
	int endPathTdimLength;
	std::vector<Port>::iterator PortIter;

	dfgNode* currParent;
	dfgNode* currAffNode;
//	std::vector<TreePath> treePaths;
	std::map<CGRANode*,std::vector<TreePath>> destTreePathMap;
	std::map<CGRANode*,std::vector<TreePath>>::iterator destTreePathMapIt;

	//PHINode Treatment
	std::map<CGRANode*,std::vector<TreePath>> destTreePathMapPHI;

	std::map<pathData,pathData,pathDataComparer> cameFrom;
	std::map<pathData,pathData,pathDataComparer>::iterator cameFromIt;
	bool cameFromSearch = false;

	std::map<CGRANode*,int> costSoFar;
	bool routingComplete = false;

	std::vector<CGRAEdge*> tempCGRAEdges;
	std::vector<CGRAEdge*> tempDestCGRAEdges;
	std::vector<CGRAEdge*> tempDestCGRAEdges2;
	std::map<CGRANode*,std::vector<CGRAEdge> > originalCGRAEdges = *cgraEdges;
	std::map<CGRANode*,std::set<Port>> originalbusyPorts = currDFG->getCGRA()->busyPorts;

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

//	std::vector<pathWithCost> pathsWithCost;
	std::map<CGRANode*,std::vector<pathWithCost>> destPathWithCostMap;
	std::map<CGRANode*,std::vector<pathWithCost>>::iterator destPathWithCostMapIt;

//	std::vector<treePathWithCost> treePathsWithCost;
	std::map<CGRANode*,std::vector<treePathWithCost>> destTreePathWithCostMap;
	std::map<CGRANode*,std::vector<treePathWithCost>>::iterator destTreePathWithCostMapIt;

	struct DestCost{
		CGRANode* dest;
		int cost;
		double normCost;
		int affCost;
		double normAffCost;
		double normFinalCost;
		DestCost(CGRANode* dest) : dest(dest), cost(0), affCost(0){}
		DestCost(CGRANode* dest, int cost) : dest(dest), cost(cost), affCost(0){}
		DestCost(CGRANode* dest, int cost, int affCost) : dest(dest), cost(cost), affCost(affCost){}
	};

	struct LessThanDestCost{
	    bool operator()(DestCost const & p1, DestCost const & p2) {
	        // return "true" if "p1" is ordered before "p2", for example:
	        return p1.cost < p2.cost;
	    }
	};

	struct LessThanDestNormTotalCost{
	    bool operator()(DestCost const & p1, DestCost const & p2) {
	        // return "true" if "p1" is ordered before "p2", for example:
	        return p1.normFinalCost < p2.normFinalCost;
	    }
	};

	std::vector<DestCost> destCostArray;

	bool localDeadEndReached = false;
	CGRANode* dest;
	int destCost;
	int affCost;

	int currCGRAMEMPEs = currDFG->getCGRA()->getTotalUnUsedMemPEs();
	int memOpOptCost = 0;
	int MemOpsToBePlaced = currDFG->getMEMOpsToBePlaced();

	assert(MemOpsToBePlaced <= currCGRAMEMPEs);
	if(currCGRAMEMPEs != 0){
		memOpOptCost = (int)(10000.0*((float)MemOpsToBePlaced/(float)currCGRAMEMPEs));
	}
	else{
		memOpOptCost = 0;
	}



	pathsNotRouted->clear();

	//Clearing all the routing associated with current node on previous routing attempts
	for (int i = 0; i < currNode->getAncestors().size(); ++i) {
		currParent = currNode->getAncestors()[i];
		(*currNode->getTreeBasedRoutingLocs())[currParent].clear();
		(*currNode->getTreeBasedGoalLocs())[currParent].clear();
	}

	errs() << "Astar::Route:: dests.size = " << dests->size() << "\n";
	errs() << "Astar::Route:: parents.size = " << parents.size() << "\n";


	if(parents.empty()){
		for (int i = 0; i < dests->size(); ++i) {
			dest = (*dests)[i];
			destCost = 9 - currDFG->getCGRA()->findCGRAEdges(dest,TILE,cgraEdges).size();
			destCost = destCost + 1000*dest->getT();
			if(dest->getPEType() == MEM){
				destCost += memOpOptCost;
				if(MemOpsToBePlaced == currCGRAMEMPEs){
					if(!currNode->getIsMemOp()){
						continue;
					}
				}
			}
			destCostArray.push_back(DestCost(dest,destCost));
		}
		std::sort(destCostArray.begin(),destCostArray.end(),LessThanDestCost());

//		assert(!destCostArray.empty());
		if(destCostArray.empty()){
			errs() << "Route:: destCostArray is empty!!\n";
			return false;
		}

		errs() << "Chosen Dest=" << destCostArray[0].dest->getName() << "\n";
		*chosenDest = destCostArray[0].dest;
		dests->erase(std::remove(dests->begin(),dests->end(),dest),dests->end());
		return true;
	}


	//Creating TreePaths Data Structure
	for (int i = 0; i < dests->size(); ++i) {
		for (int j = 0; j < parents.size(); ++j) {
			if(parents[j] == currNode){
				continue;
			}
			destTreePathMap[(*dests)[i]].push_back(currDFG->createTreePath(parents[j],(*dests)[i]));
		}

		//IF PHI Children are there
		for (int j = 0; j < currNode->getPHIchildren().size(); ++j) {
			dfgNode* phiChildNode = currNode->getPHIchildren()[j];
			assert(phiChildNode != NULL);

			if(phiChildNode->getMappedLoc() != NULL){
				destTreePathMap[(*dests)[i]].push_back(currDFG->createTreePathPHIDest(currNode,(*dests)[i],phiChildNode->getMappedLoc()));
				parents.push_back(currNode);
			}
			else{
				phiChildNode->addAncestorNode(currNode);
				currNode->addChildNode(phiChildNode);
			}
		}
	}

	//Route estimation
	outs() << "Route Estimation!\n";
	for(destTreePathMapIt = destTreePathMap.begin();
		destTreePathMapIt != destTreePathMap.end();
		destTreePathMapIt++){

		dest = destTreePathMapIt->first;
		destCost = 0;

		for (int i = 0; i < destTreePathMap[dest].size(); ++i) {

			//Sorting TreePath Paths
			destPathWithCostMap[dest].clear();
			for (int j = 0; j < destTreePathMap[dest][i].sources.size(); ++j) {
				start = destTreePathMap[dest][i].sources[j];
				start = currDFG->getCGRA()->getCGRANode(start->getT(),start->getY(),start->getX());
				startPort = destTreePathMap[dest][i].sourcePorts[start];

				assert(std::string("INV").compare(currDFG->getCGRA()->getPortName(startPort)) != 0);

				startPathLength = destTreePathMap[dest][i].sourceSCpathLengths[start];
				startPathTdimLength = destTreePathMap[dest][i].sourceSCpathTdimLengths[start];
				goal = destTreePathMap[dest][i].dest;
				goal = currDFG->getCGRA()->getCGRANode(goal->getT(),goal->getY(),goal->getX());

//				bool check = parents[i]==currNode;
//				outs() << "parent is currNode = " << check << "\n";
//				outs() << ",ParentPos=" << parents[i]->getMappedLoc()->getName();
//				outs() << ",ParentRT=" << parents[i]->getmappedRealTime();
//				outs() << "\n";

//				int startRT = parents[i]->getmappedRealTime() + startPathTdimLength;
				int parentRT = parents[i]->getmappedRealTime();
				int kII;
				if(parents[i]==currNode){
					kII = -1; //phi check
				}
				else if((parents[i]->getNameType().compare("OutLoopLOAD")==0)){
					kII = -1;
				}
				else if((parents[i]->getNameType().compare("MOVC")==0)){
					kII = -1;
				}
				else if((parents[i]->getNameType().compare("LOOPSTART")==0)){
					kII = -1;
				}
//				else if(parents.size() < 2){
//					kII = -1;
//				}
				else{
//					outs() << "currParentLoc=" << parents[i]->getMappedLoc()->getName();
//					outs() << ",currParentRT=" << parents[i]->getmappedRealTime();
//					outs() << ",startPathTdimLength=" << startPathTdimLength;
//					outs() << ",startPathLength=" << startPathLength;
//					outs() << "\n";
					kII = calckII(parents[i]->getMappedLoc(),parentRT,goal,(*destllmap)[goal]);
				}

//				outs() << "currNodephichildrensize=" << currNode->PHIchildren.size() << "\n";
//				if(parents[i]->getMappedLoc() != NULL){
//					outs() << "ParentCNode=" << parents[i]->getMappedLoc()->getName();
//				}
//				outs() << "Goal=" << goal->getName() << "\n";

				end = AStarSearch(*cgraEdges,
							      start,
								  startPort,
								  goal,
								  kII,
								  &cameFrom,
								  &costSoFar,
								  &endPort,
								  startPathLength,
								  startPathTdimLength,
								  &endPathLength,
								  &endPathTdimLength);
				if(end != goal){
					errs() << "failed::" << start->getName() << "->" << goal->getName() << ",";
					continue;
				}

//				outs() << "TADA!!!!!!!!!\n";

				assert(destTreePathMap[dest][i].sourcePaths[start].first != NULL);
				assert(destTreePathMap[dest][i].sourcePaths[start].second != NULL);
//				errs() << "\n";
//				pathsWithCost.push_back(pathWithCost(std::make_pair(start,goal),costSoFar[goal],parents[i]));
				if(parents[i] == currNode){
					assert(dest != goal);
				}
				else{
					assert(dest == goal);
				}
				destPathWithCostMap[dest].push_back(pathWithCost(std::make_pair(start,goal),costSoFar[goal],parents[i]));
//				outs() << "1:destPathWithCostMap size=" << destPathWithCostMap[dest].size() << "\n";
			}
//			errs() << "\n";

//			outs() << "2:destPathWithCostMap size=" << destPathWithCostMap[dest].size() << "\n";
			std::sort(destPathWithCostMap[dest].begin(),destPathWithCostMap[dest].end(),LessThanPathWithCost());
//			outs() << "3:destPathWithCostMap size=" << destPathWithCostMap[dest].size() << "\n";

			if(destPathWithCostMap[dest].size() == 0){
				errs() << "SMARTRouteEst :: " << "Path is not routed = " << start->getName() << " to " << goal->getName() << "\n";
				errs() << "SMARTRouteEst :: " << "But Routed until : " << (end)->getName() << "\n";
				localDeadEndReached = reportDeadEnd(end,endPort,currNode,cgraEdges);
				//it was not able to route from the starting point which happen to be due to previously placed
				if(start == end){
//					localDeadEndReached = true;
				}

				if(deadEndReached != NULL){
					*deadEndReached = localDeadEndReached;
				}
				pathsNotRouted->push_back(std::make_pair(start,goal));
	//			return false;
				break;
			}
			else{
				destTreePathMap[dest][i].bestSource = destPathWithCostMap[dest][0].path.first;
				destTreePathMap[dest][i].bestCost = destPathWithCostMap[dest][0].cost;
				destCost += destTreePathMap[dest][i].bestCost;
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

//			treePathsWithCost.push_back(treePathWithCost(treePaths[i],treePaths[i].bestCost,parents[i]));
			destTreePathWithCostMap[dest].push_back(treePathWithCost(destTreePathMap[dest][i],destTreePathMap[dest][i].bestCost,parents[i]));
	//		pathsWithCost.push_back(pathWithCost(std::make_pair(treePaths[i].bestSource,treePaths[i].dest),,parents[i]));
		}
		if(destPathWithCostMap[dest].size() == 0){
			continue;
		}
		std::sort(destTreePathWithCostMap[dest].begin(),destTreePathWithCostMap[dest].end(),LessThanTreePathWithCost());

		//Calculating AffCost associated with this particular dest
		affCost = 0;
		for (int i = 0; i < ASAPLevelNodeMap[currNode->getASAPnumber()].size(); ++i) {
			currAffNode = ASAPLevelNodeMap[currNode->getASAPnumber()][i];
			if(currAffNode->getMappedLoc() != NULL){
				affCost+=currDFG->getAffinityCost(currNode,currAffNode)*currDFG->getDistCGRANodes(dest,currAffNode->getMappedLoc());
			}
		}

		if(dest->getPEType() == MEM){
			destCost += memOpOptCost;
			if(MemOpsToBePlaced == currCGRAMEMPEs){
				if(!currNode->getIsMemOp()){
					outs() << "mem op problem\n";
					continue;
				}
			}
		}
//		outs() << "Pushing to destcostarr\n";
		destCostArray.push_back(DestCost(dest,destCost,affCost));
	}

	if(destCostArray.empty()){
		outs() << "parents.size()=" << parents.size() << "\n";
		errs() << "Route:: destCostArray is empty!!\n";
		return false;
	}

//	errs() << "treePathsWithCost size=" << treePathsWithCost.size() << "\n";

	cameFrom.clear();
	costSoFar.clear();

	//Calculate Norm Costs of destCostArray
	int destCostMax = 0;
	int destCostMin = INT_MAX;
	int destAffCostMax = 0;
	int destAffCostMin = INT_MAX;
	for (int i = 0; i < destCostArray.size(); ++i) {
		if(destCostArray[i].cost < destCostMin){
			destCostMin = destCostArray[i].cost;
		}

		if(destCostArray[i].cost > destCostMax){
			destCostMax = destCostArray[i].cost;
		}

		if(destCostArray[i].affCost < destAffCostMin){
			destAffCostMin = destCostArray[i].affCost;
		}

		if(destCostArray[i].affCost > destAffCostMax){
			destAffCostMax = destCostArray[i].affCost;
		}
	}

	assert(destCostMin != INT_MAX);
	assert(destAffCostMin != INT_MAX);

	double temp1;
	double costVar = (double)(destCostMax - destCostMin);
	double affCostVar = (double)(destAffCostMax - destAffCostMin);

	for (int i = 0; i < destCostArray.size(); ++i) {
		if(destCostMax > destCostMin){
			temp1 = (double)(destCostArray[i].cost - destCostMin);
			destCostArray[i].normCost = temp1/costVar;
		}
		else{
			destCostArray[i].normCost = 1;
		}

		if(destAffCostMax > destAffCostMin){
			temp1 = (double)(destCostArray[i].affCost - destAffCostMin);
			destCostArray[i].normAffCost = temp1/affCostVar;
		}
		else{
			destCostArray[i].normAffCost = 1;
		}

		destCostArray[i].normFinalCost = (destCostArray[i].normCost + destCostArray[i].normAffCost)/2;
	}
	std::sort(destCostArray.begin(),destCostArray.end(),LessThanDestNormTotalCost());

	//Real Routing happens here

	for (int j = 0; j < destCostArray.size(); ++j) {
		dest = destCostArray[j].dest;
		outs() << "iterating through dests,j=" << j+1 << " of " << destCostArray.size() <<"\n";
		for (int i = 0; i < destTreePathWithCostMap[dest].size(); ++i) {
			goal = destTreePathWithCostMap[dest][i].tp.dest;
			goal = currDFG->getCGRA()->getCGRANode(goal->getT(),goal->getY(),goal->getX());
			currParent = destTreePathWithCostMap[dest][i].parent;
			errs() << "Nodes cleared for NodeIdx=" << currNode->getIdx() << ", ParentIdx=" << currParent->getIdx() << "\n";


			assert(destTreePathWithCostMap[dest][i].tp.sources.size() != 0);

			//Check for the shortest-routable path
//			pathsWithCost.clear();
			destPathWithCostMap[dest].clear();
			for (int j = 0; j < destTreePathWithCostMap[dest][i].tp.sources.size(); ++j) {
				start = destTreePathWithCostMap[dest][i].tp.sources[j];
				startPort = destTreePathWithCostMap[dest][i].tp.sourcePorts[start];
				startPathLength = destTreePathWithCostMap[dest][i].tp.sourceSCpathLengths[start];
				startPathTdimLength = destTreePathWithCostMap[dest][i].tp.sourceSCpathTdimLengths[start];
				start = currDFG->getCGRA()->getCGRANode(start->getT(),start->getY(),start->getX());
				errs() << "AStar Search starts...\n";

//				int startRT = currParent->getmappedRealTime() + startPathTdimLength;
				int parentRT = currParent->getmappedRealTime();
				int kII;
				if(currParent==currNode){
					kII = -1; //phi check
				}
				else if((currParent->getNameType().compare("OutLoopLOAD")==0)){
					kII = -1;
				}
				else if((currParent->getNameType().compare("MOVC")==0)){
					kII = -1;
				}
				else if((currParent->getNameType().compare("LOOPSTART")==0)){
					kII = -1;
				}
//				else if(parents.size() < 2){
//					kII = -1;
//				}
				else{
//					outs() << "currParentLoc=" << currParent->getMappedLoc()->getName();
//					outs() << ",currParentRT=" << currParent->getmappedRealTime();
//					outs() << ",startPathTdimLength=" << startPathTdimLength;
//					outs() << "\n";
					kII = calckII(currParent->getMappedLoc(),parentRT,goal,(*destllmap)[goal]);
				}
				end = AStarSearch(*cgraEdges,
						          start,
								  startPort,
								  goal,
								  kII,
								  &cameFrom,
								  &costSoFar,
								  &endPort,
								  startPathLength,
								  startPathTdimLength,
								  &endPathLength,
								  &endPathTdimLength);
				errs() << "AStar Search ends...\n";
				if(end != goal){
					outs() << "Routing Failed : " << start->getName() << "->" << goal->getName() << ",only routed to " << end->getName() << "\n";
					*mappingOutFile << "Routing Failed : " << start->getName() << "->" << goal->getName() << ",only routed to " << end->getName() << std::endl;
					continue;
				}
				*mappingOutFile << "Routing Success : " << start->getName() << "->" << goal->getName() <<  std::endl;

				assert(destTreePathWithCostMap[dest][i].tp.sourcePaths[start].first != NULL);
				assert(destTreePathWithCostMap[dest][i].tp.sourcePaths[start].second != NULL);

//				pathsWithCost.push_back(pathWithCost(std::make_pair(start,goal),costSoFar[goal],parents[i]));
				if(destTreePathWithCostMap[dest][i].parent == currNode){
					assert(dest != goal);
				}
				else{
					if(dest != goal) errs() << "goal = " << goal->getName() << ",dest = " << dest->getName() << "\n";
					assert(dest == goal);
				}
				destPathWithCostMap[dest].push_back(pathWithCost(std::make_pair(start,goal),costSoFar[goal],destTreePathWithCostMap[dest][i].parent));
			}

//			std::sort(pathsWithCost.begin(),pathsWithCost.end(),LessThanPathWithCost());
			std::sort(destPathWithCostMap[dest].begin(),destPathWithCostMap[dest].end(),LessThanPathWithCost());

			//Order the routable paths in the order into the treePaths data structure
//			treePaths[i].sources.clear();
			destTreePathMap[dest][i].sources.clear();
			for (int j = 0; j < destPathWithCostMap[dest].size(); ++j) {
				destTreePathMap[dest][i].sources.push_back(destPathWithCostMap[dest][j].path.first);
			}
			destTreePathMap[dest][i].sourcePorts = destTreePathWithCostMap[dest][i].tp.sourcePorts;
			destTreePathMap[dest][i].sourcePaths = destTreePathWithCostMap[dest][i].tp.sourcePaths;
			destTreePathMap[dest][i].dest = destTreePathWithCostMap[dest][i].tp.dest;
			destTreePathMap[dest][i].sourceSCpathLengths = destTreePathWithCostMap[dest][i].tp.sourceSCpathLengths;
			destTreePathMap[dest][i].sourceSCpathTdimLengths = destTreePathWithCostMap[dest][i].tp.sourceSCpathTdimLengths;
//			destTreePathMap[dest][i].

			if(destTreePathMap[dest][i].sources.size() == 0){
				*mappingOutFile << "routing not completed as it only routed until : " << end->getName() << std::endl;
				pathsNotRouted->push_back(std::make_pair(start,goal));
				localDeadEndReached = reportDeadEnd(end,endPort,currNode,cgraEdges);
				if(deadEndReached != NULL){
					*deadEndReached = localDeadEndReached;
				}
//				return false;
				*cgraEdges = originalCGRAEdges;
				currDFG->getCGRA()->busyPorts = originalbusyPorts;
				routingComplete = false;
				break;
			}

			std::pair<dfgNode*,dfgNode*> currSourcePath;
			//start routing
			routingComplete = false;
			for (int j = 0; j < destTreePathMap[dest][i].sources.size(); ++j) {
				start = destTreePathMap[dest][i].sources[j];
				startPort = destTreePathMap[dest][i].sourcePorts[start];
				startPathLength = destTreePathMap[dest][i].sourceSCpathLengths[start];
				startPathTdimLength = destTreePathMap[dest][i].sourceSCpathTdimLengths[start];
				start = currDFG->getCGRA()->getCGRANode(start->getT(),start->getY(),start->getX());
				goal = destTreePathMap[dest][i].dest;
				goal = currDFG->getCGRA()->getCGRANode(goal->getT(),goal->getY(),goal->getX());
//				currParent = parents[i];
				currParent = destTreePathWithCostMap[dest][i].parent;

				assert(destTreePathMap[dest][i].sourcePaths[start].first != NULL);
				assert(destTreePathMap[dest][i].sourcePaths[start].second != NULL);

				currSourcePath = destTreePathMap[dest][i].sourcePaths[start];

				*mappingOutFile << "start = (" << start->getT() << "," << start->getY() << "," << start->getX() << ")\n";
				*mappingOutFile << "goal = (" << goal->getT() << "," << goal->getY() << "," << goal->getX() << ")\n";

//				int startRT = currParent->getmappedRealTime() + startPathTdimLength;
				int parentRT = currParent->getmappedRealTime();
				int kII;
				if(currParent==currNode){
					kII = -1; //phi check
				}
				else if((currParent->getNameType().compare("OutLoopLOAD")==0)){
					kII = -1;
				}
				else if((currParent->getNameType().compare("MOVC")==0)){
					kII = -1;
				}
				else if((currParent->getNameType().compare("LOOPSTART")==0)){
					kII = -1;
				}
//				else if(parents.size() < 2){
//					kII = -1;
//				}
				else{
//					outs() << "currParentLoc=" << currParent->getMappedLoc()->getName();
//					outs() << ",currParentRT=" << currParent->getmappedRealTime();
//					outs() << ",startPathTdimLength=" << startPathTdimLength;
//					outs() << "\n";
					kII = calckII(currParent->getMappedLoc(),parentRT,goal,(*destllmap)[goal]);
				}
				end = AStarSearch(*cgraEdges,
						          start,
								  startPort,
								  goal,
								  kII,
								  &cameFrom,
								  &costSoFar,
								  &endPort,
								  startPathLength,
								  startPathTdimLength,
								  &endPathLength,
								  &endPathTdimLength);
//				assert(end == goal);
	//			CameFromVerify(&cameFrom);


				if(end != goal){
					errs() << "SMARTRouteReal :: " << "Path is not routed = " << start->getName() << " to " << goal->getName() << "\n";
					errs() << "SMARTRouteReal :: " << "But Routed until : " << (end)->getName() << "\n";
					continue;
				}
				else{
					// removing this to avoid re-consideration of this dest again
					dests->erase(std::remove(dests->begin(),dests->end(),dest),dests->end());
					*chosenDest = dest;
					routingComplete = true;
					errs() << "Route Complete :: end=" << end->getName() << ",endPort=" << currDFG->getCGRA()->getPortName(endPort) << "\n";
					break;
				}
			}

			if(!routingComplete){
				*mappingOutFile << "routing not completed as it only routed until : " << end->getName() << std::endl;
				pathsNotRouted->push_back(std::make_pair(start,goal));
				(*currNode->getTreeBasedRoutingLocs())[currParent].clear();
				(*currNode->getTreeBasedGoalLocs())[currParent].clear();
				*cgraEdges = originalCGRAEdges;
				currDFG->getCGRA()->busyPorts = originalbusyPorts;
				break;
			}



			int pathLength = 0;
			int SMARTpathLength = 0;
			int regConnections = 0;
			bool SMARTPathEnd = true;
			(*currNode->getTreeBasedRoutingLocs())[currParent].clear();
			(*currNode->getTreeBasedGoalLocs())[currParent].clear();

			current = goal;
			currPort = endPort;
			currPathLength = endPathLength;
			currPathTdimLength = endPathTdimLength;

			assert(std::string("INV").compare(currDFG->getCGRA()->getPortName(endPort)) != 0);
			(*currNode->getTreeBasedGoalLocs())[currParent].push_back(new pathData(goal,endPort,endPathLength,endPathTdimLength));
			(*currNode->getSourceRoutingPath())[currParent] = currSourcePath;

			assert(currSourcePath.first != NULL);
			assert(currSourcePath.second != NULL);
			errs() << "Route :: currNode=" << currNode->getIdx() << ",currParent=" << currParent->getIdx();
			errs() << "SourcePath=(" << currSourcePath.first->getIdx() << "," << currSourcePath.second->getIdx() << ")\n";

			if(currSourcePath.first == currSourcePath.second){
				errs() << "start=" << start->getName() << "\n";

				if(currParent == currNode){
					errs() << "PHIChildMapping : CurrNodeLoc =" << goal->getName() << "\n";
				}
				else{
					errs() << "currParentLoc=" << currParent->getMappedLoc()->getName() << "\n";
				}


			}

			if(cameFrom.size() > 0){
				cameFromSearch = false;
				errs() << "start = " << start->getName() << "\n";
				for(cameFromIt = cameFrom.begin(); cameFromIt != cameFrom.end(); ++cameFromIt){
					if(cameFromIt->second.cnode == start){
						cameFromSearch = true;
						break;
					}
				}
				assert(cameFromSearch);
			}
			else{
				assert(current == start);
			}

			*mappingOutFile << "The Path : ";

			while(current!=start){
				*mappingOutFile << "(" << current->getT() << "," << current->getY() << "," << current->getX() << ")" << " <-- ";

	//			assert(cameFrom.find(current) != cameFrom.end());
				cameFromSearch = false;
				for(cameFromIt = cameFrom.begin(); cameFromIt != cameFrom.end(); cameFromIt++){
					if(cameFromIt->first.cnode == current){
//						if(current == goal){
//							currPort = cameFromIt->first.lastPort;
//							currPathLength = cameFromIt->first.SCpathLength;
//							currPathTdimLength = cameFromIt->first.TdimPathLength;
//						}
						cameFromSearch = true;
						break;
					}
//					outs() << "current=" << current->getName() << "\n";
//					outs() << ",wrong node=" << cameFromIt->first.cnode->getName() << "\n";
				}
				assert(cameFromSearch);

				if(current==goal){
					currDFG->getCGRA()->busyPorts[current].insert(currPort);
				}

//				currNodePortPair = std::make_pair(current,currPort);

				//Adding Routing Locs to form a multicast tree in the future
	//			if(cameFrom[current] != start){

					assert(current == currDFG->getCGRA()->getCGRANode(current->getT(),current->getY(),current->getX()));
					pathData currPathData(current,currPort,currPathLength,currPathTdimLength);
					assert(std::string("INV").compare(currDFG->getCGRA()->getPortName(currPort)) != 0);
					errs() << "getTreeBasedRoutingLocs update :: ";
					errs() << ", currNode=" << currNode->getIdx();
					errs() << ", currParent=" << currParent->getIdx();
					errs() << ", cnode=" << current->getName();
					errs() << ", sclength=" << currPathLength;
					errs() << ", port=" << currDFG->getCGRA()->getPortName(currPort) << "\n";

//					if(currPort==R0) assert(cameFrom[currPathData].lastPort == R0 || cameFrom[currPathData].lastPort == NORTH);
//					if(currPort==R1) assert(cameFrom[currPathData].lastPort == R1 || cameFrom[currPathData].lastPort == EAST);
//					if(currPort==R2) assert(cameFrom[currPathData].lastPort == R2 || cameFrom[currPathData].lastPort == WEST);
//					if(currPort==R3) assert(cameFrom[currPathData].lastPort == R3 || cameFrom[currPathData].lastPort == SOUTH);

					if(currParent == currNode){
						errs() << "PHI Child : " << currDFG->findNodeMappedLoc(goal)->getIdx() << "\n";
						(*currDFG->findNodeMappedLoc(goal)->getTreeBasedRoutingLocs())[currParent].push_back(new pathData(current,currPort,currPathLength,currPathTdimLength));
					}
					else{
						(*currNode->getTreeBasedRoutingLocs())[currParent].push_back(new pathData(current,currPort,currPathLength,currPathTdimLength));
					}


	//			}


	//			assert(cgraEdges->find(cameFrom[current]) != cgraEdges->end());

	//			CameFromVerify(&cameFrom);
				tempCGRAEdges = currDFG->getCGRA()->findCGRAEdges(cameFrom[currPathData].cnode,cameFrom[currPathData].lastPort,cgraEdges);
				assert(tempCGRAEdges.size() != 0);

	//			std::vector<CGRANode*>::iterator found = std::find((*cgraEdges)[cameFrom[current]].begin(), (*cgraEdges)[cameFrom[current]].end(), current);
				bool found=false;
				for (int j = 0; j < tempCGRAEdges.size(); ++j) {
					if(tempCGRAEdges[j]->Dst == current){

						if(currPort != TILE){
							if(tempCGRAEdges[j]->DstPort != currPort){
								errs() << "Wrong Edge :: ";
								currDFG->getCGRA()->printCGRAEdge(*tempCGRAEdges[j]);
								continue;
							}
						}
						found=true;

						tempCGRAEdges[j]->mappedDFGEdge = currDFG->findEdge(currParent,currNode,goal);
						currDFG->getCGRA()->printCGRAEdge(*tempCGRAEdges[j]);

						if((currDFG->getCGRA()->getArch() != DoubleXBar)&&(currDFG->getCGRA()->getArch() != ALL2ALL)){
							if(currDFG->getCGRA()->getArch() != NoNOC){
								switch(cameFrom[currPathData].lastPort){
									case R0:
										if(tempCGRAEdges[j]->DstPort != R0){
											cameFrom[currPathData].cnode->InOutPortMap[NORTH] = {R0};
										}
									break;
									case R1:
										if(tempCGRAEdges[j]->DstPort != R1){
											cameFrom[currPathData].cnode->InOutPortMap[EAST] = {R1};
										}
									break;
									case R2:
										if(tempCGRAEdges[j]->DstPort != R2){
											cameFrom[currPathData].cnode->InOutPortMap[WEST] = {R2};
										}
									break;
									case R3:
										if(tempCGRAEdges[j]->DstPort != R3){
											cameFrom[currPathData].cnode->InOutPortMap[SOUTH] = {R3};
										}
									break;
#ifdef TREG_MUTEX
									case TREG:
										if(tempCGRAEdges[j]->DstPort != TREG){
											cameFrom[currPathData].cnode->InOutPortMap[TILE] = {};
										}
									break;
#endif
									case NORTH:
										if(tempCGRAEdges[j]->DstPort != R0){
											cameFrom[currPathData].cnode->InOutPortMap[R0] = {R0};
										}
									break;
									case EAST:
										if(tempCGRAEdges[j]->DstPort != R1){
											cameFrom[currPathData].cnode->InOutPortMap[R1] = {R1};
										}
									break;
									case WEST:
										if(tempCGRAEdges[j]->DstPort != R2){
											cameFrom[currPathData].cnode->InOutPortMap[R2] = {R2};
										}
									break;
									case SOUTH:
										if(tempCGRAEdges[j]->DstPort != R3){
											cameFrom[currPathData].cnode->InOutPortMap[R3] = {R3};
										}
									break;
#ifdef TREG_MUTEX
									case TILE:
										if(tempCGRAEdges[j]->DstPort != TREG){
											cameFrom[currPathData].cnode->InOutPortMap[TREG] = {TREG};
										}
									break;
#endif
								}
							}
							else if(current != goal){ //NoNOC
								tempDestCGRAEdges = currDFG->getCGRA()->getCGRAEdgesWithDest(currPathData.cnode,cgraEdges);
								errs() << "AStar :: before tempDestCGRAEdges.size() = " << tempDestCGRAEdges.size() << "\n";
								for (int k = 0; k < tempDestCGRAEdges.size(); ++k) {
										//If register edges no need delete other edges arriving this cgranode
										if((tempDestCGRAEdges[k]->DstPort == R0) ||
										   (tempDestCGRAEdges[k]->DstPort == R1) ||
										   (tempDestCGRAEdges[k]->DstPort == R2) ||
										   (tempDestCGRAEdges[k]->DstPort == R3) ){

											continue;
										}
										//if this is executing, then current input port is not register

										//If input port is register and output port is not register
										// which means next guys input port is not register.
										//Therefore here current input port is not register but following checks previous is register.
										// that means this will occupy the PE to do the transfer.
										if((cameFrom[currPathData].lastPort == R0)||
										   (cameFrom[currPathData].lastPort == R1)||
										   (cameFrom[currPathData].lastPort == R2)||
										   (cameFrom[currPathData].lastPort == R3)){

											cameFrom[currPathData].cnode->InOutPortMap[R0] = {R0};
											cameFrom[currPathData].cnode->InOutPortMap[R1] = {R1};
											cameFrom[currPathData].cnode->InOutPortMap[R2] = {R2};
											cameFrom[currPathData].cnode->InOutPortMap[R3] = {R3};

											tempDestCGRAEdges2 = currDFG->getCGRA()->getCGRAEdgesWithDest(cameFrom[currPathData].cnode,cgraEdges);
											for (int l = 0; l < tempDestCGRAEdges2.size(); ++l) {

												if((tempDestCGRAEdges2[l]->DstPort == R0) ||
												   (tempDestCGRAEdges2[l]->DstPort == R1) ||
												   (tempDestCGRAEdges2[l]->DstPort == R2) ||
												   (tempDestCGRAEdges2[l]->DstPort == R3) ){

													continue;
												}

												if(tempDestCGRAEdges2[l]->mappedDFGEdge == NULL){
													tempDestCGRAEdges2[l]->mappedDFGEdge = currDFG->findEdge(currParent,currNode,goal);
												}
											}
										}

										assert(tempDestCGRAEdges[k]->Dst == currPathData.cnode);
										if(tempDestCGRAEdges[k]->mappedDFGEdge == NULL){
											tempDestCGRAEdges[k]->mappedDFGEdge = currDFG->findEdge(currParent,currNode,goal);
										}
								}
								tempDestCGRAEdges = currDFG->getCGRA()->getCGRAEdgesWithDest(currPathData.cnode,cgraEdges);
								errs() << "AStar :: after tempDestCGRAEdges.size() = " << tempDestCGRAEdges.size() << "\n";
							}
						}


						*mappingOutFile << "[" << CGRA::getPortName(tempCGRAEdges[j]->SrcPort) << "of" << tempCGRAEdges[j]->Src->getName() << "]";
						break;
					}
				}
				assert(found);


				//Logging - Begin
				if(current->getT() != cameFrom[currPathData].cnode->getT()){
					if(!SMARTPathEnd){
						SMARTPathEnd = true;
						if(SMARTPathHist.find(SMARTpathLength) == SMARTPathHist.end()){
							SMARTPathHist[SMARTpathLength] = 1;
							if(currParent->isConditional()){
								SMARTPredicatePathHist[SMARTpathLength] = 1;
							}
//							if(currParent->getNode() != NULL){
//								if(currParent->getNode()->getOpcode() == Instruction::Br){
//									SMARTPredicatePathHist[SMARTpathLength] = 1;
//								}
//							}

						}
						else{
							SMARTPathHist[SMARTpathLength]++;
							if(currParent->isConditional()){
								SMARTPredicatePathHist[SMARTpathLength]++;
							}
//							if(currParent->getNode() != NULL){
//								if(currParent->getNode()->getOpcode() == Instruction::Br){
//									SMARTPredicatePathHist[SMARTpathLength]++;
//								}
//							}
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
				current = cameFrom[currPathData].cnode;
				currPort = cameFrom[currPathData].lastPort;
				currPathLength = cameFrom[currPathData].SCpathLength;
				currPathTdimLength = cameFrom[currPathData].TdimPathLength;
			}
			assert(std::string("INV").compare(currDFG->getCGRA()->getPortName(startPort)) != 0);
			errs() << "getTreeBasedRoutingLocs update :: ";
			errs() << ", currNode=" << currNode->getIdx();
			errs() << ", currParent=" << currParent->getIdx();
			errs() << ", cnode=" << start->getName();
			errs() << ", sclength=" << startPathLength;
			errs() << ", port=" << currDFG->getCGRA()->getPortName(startPort) << "\n";

			if(currParent == currNode){
				errs() << "PHI Child : " << currDFG->findNodeMappedLoc(goal)->getIdx() << "\n";
				(*currDFG->findNodeMappedLoc(goal)->getTreeBasedRoutingLocs())[currParent].push_back(new pathData(start,startPort,startPathLength,startPathTdimLength));
			}
			else{
				(*currNode->getTreeBasedRoutingLocs())[currParent].push_back(new pathData(start,startPort,startPathLength,startPathTdimLength));
			}


//			(*currNode->getTreeBasedRoutingLocs())[currParent].push_back(new pathData(start,startPort,startPathLength));

			if(currDFG->getCGRA()->getArch() == NoNOC){
				tempDestCGRAEdges = currDFG->getCGRA()->getCGRAEdgesWithDest(start,cgraEdges);
				for (int k = 0; k < tempDestCGRAEdges.size(); ++k) {

					if((tempDestCGRAEdges[k]->DstPort == R0) ||
					   (tempDestCGRAEdges[k]->DstPort == R1) ||
					   (tempDestCGRAEdges[k]->DstPort == R2) ||
					   (tempDestCGRAEdges[k]->DstPort == R3) ){
						continue;
					}

					start->InOutPortMap[R0] = {R0};
					start->InOutPortMap[R1] = {R1};
					start->InOutPortMap[R2] = {R2};
					start->InOutPortMap[R3] = {R3};

					if(tempDestCGRAEdges[k]->mappedDFGEdge == NULL){
						tempDestCGRAEdges[k]->mappedDFGEdge = currDFG->findEdge(currParent,currNode,goal);
					}
				}

				tempDestCGRAEdges = currDFG->getCGRA()->getCGRAEdgesWithDest(currParent->getMappedLoc(),cgraEdges);
				for (int k = 0; k < tempDestCGRAEdges.size(); ++k) {

					if((tempDestCGRAEdges[k]->DstPort == R0) ||
					   (tempDestCGRAEdges[k]->DstPort == R1) ||
					   (tempDestCGRAEdges[k]->DstPort == R2) ||
					   (tempDestCGRAEdges[k]->DstPort == R3) ){
						continue;
					}

					currParent->getMappedLoc()->InOutPortMap[R0] = {R0};
					currParent->getMappedLoc()->InOutPortMap[R1] = {R1};
					currParent->getMappedLoc()->InOutPortMap[R2] = {R2};
					currParent->getMappedLoc()->InOutPortMap[R3] = {R3};

					if(tempDestCGRAEdges[k]->mappedDFGEdge == NULL){
						tempDestCGRAEdges[k]->mappedDFGEdge = currDFG->findEdge(currParent,currNode,goal);
					}
				}
			}

			//Logging - SMART Path Length
			if(SMARTpathLength != 0){
				if(SMARTPathHist.find(SMARTpathLength) == SMARTPathHist.end()){
					SMARTPathHist[SMARTpathLength] = 1;
					if(currParent->isConditional()){
						SMARTPredicatePathHist[SMARTpathLength] = 1;
					}
//					if(currParent->getNode() != NULL){
//						if(currParent->getNode()->getOpcode() == Instruction::Br){
//							SMARTPredicatePathHist[SMARTpathLength] = 1;
//						}
//					}
				}
				else{
					SMARTPathHist[SMARTpathLength]++;
					if(currParent->isConditional()){
						SMARTPredicatePathHist[SMARTpathLength]++;
					}
//					if(currParent->getNode() != NULL){
//						if(currParent->getNode()->getOpcode() == Instruction::Br){
//							SMARTPredicatePathHist[SMARTpathLength]++;
//						}
//					}
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
		} // PathsWithCost Array

		if(routingComplete){
			if(currDFG->getCGRA()->getArch() == NoNOC){
				tempDestCGRAEdges = currDFG->getCGRA()->getCGRAEdgesWithDest(goal,cgraEdges);
				for (int k = 0; k < tempDestCGRAEdges.size(); ++k) {

					if((tempDestCGRAEdges[k]->DstPort == R0) ||
					   (tempDestCGRAEdges[k]->DstPort == R1) ||
					   (tempDestCGRAEdges[k]->DstPort == R2) ||
					   (tempDestCGRAEdges[k]->DstPort == R3) ){
						continue;
					}

					if(tempDestCGRAEdges[k]->mappedDFGEdge == NULL){
						tempDestCGRAEdges[k]->mappedDFGEdge = currDFG->findEdge(currParent,currNode,goal);
					}
				}

				goal->InOutPortMap[R0] = {R0};
				goal->InOutPortMap[R1] = {R1};
				goal->InOutPortMap[R2] = {R2};
				goal->InOutPortMap[R3] = {R3};
			}
			break;
		}
	} //DestCostArray

	if(!routingComplete){
		return false;
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
	assert(false);

	CGRANode* start;
	CGRANode* goal;

	CGRANode* current;
	Port currPort;
	CGRANode* end;
	Port endPort;
	std::pair<CGRANode*,Port> currNodePortPair;
	std::vector<CGRAEdge*> tempCGRAEdges;

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

		assert(false); //please fix the following line
//		(*currNode->getTreeBasedGoalLocs())[currParent].push_back(std::make_pair(goal,endPort));
		if(goal->equals(4,3,0)){
			errs() << "%% Pusing Goal ::";
			errs() << ", nodeIdx=" << currNode->getIdx();
			errs() << ", nodeASAPLevel=" << currNode->getASAPnumber();
			errs() << ", cgraEdgeSize=" << (*cgraEdges)[goal].size();
//			errs() << ", util = " << currDFG->findUtilTreeRoutingLocs(goal,currNode);
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
				assert(false); //please fix the following line
//				(*currNode->getTreeBasedRoutingLocs())[currParent].push_back(cameFrom[currNodePortPair]);
//			}

//				assert(cgraEdges->find(cameFrom[current]) != cgraEdges->end());
				tempCGRAEdges = currDFG->getCGRA()->findCGRAEdges(cameFrom[currNodePortPair].first,cameFrom[currNodePortPair].second,cgraEdges);
				assert(tempCGRAEdges.size() != 0);


//					std::vector<CGRANode*>::iterator found = std::find((*cgraEdges)[cameFrom[current]].begin(), (*cgraEdges)[cameFrom[current]].end(), current);

//					if(found != (*cgraEdges)[cameFrom[current]].end()){
//						(*cgraEdges)[cameFrom[current]].erase(found);
//					}

				for (int j = 0; j < tempCGRAEdges.size(); ++j) {
					if(tempCGRAEdges[j]->Dst == current){
						tempCGRAEdges[j]->mappedDFGEdge = currDFG->findEdge(currParent,currNode,goal);
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
								if(tempCGRAEdges[j]->Dst == current){
									tempCGRAEdges[j]->mappedDFGEdge = currDFG->findEdge(currParent,currNode,goal);
									*mappingOutFile << "DEBUG :: " << "removing edge=" << tempCGRAEdges[j]->Src->getName() << " to " << tempCGRAEdges[j]->Dst->getName() << std::endl;
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
							assert(false); //please fix the following line

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
				return false;		assert(false); //please fix the following line
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
		std::vector<CGRAEdge*> tempCGRAEdges;

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
				next = tempCGRAEdges[i]->Dst;
				nextPort = tempCGRAEdges[i]->DstPort;

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

//	int util = currDFG->findUtilTreeRoutingLocs(end,currNode);
	int util = 0;

//	std::vector<CGRAEdge*> tempCGRAEdges =  currDFG->getCGRA()->findCGRAEdges(end,endPort,cgraEdges);

	assert(cgraEdges->find(end) != cgraEdges->end());
	assert((*cgraEdges)[end].size() != 0);
	std::vector<Port> candPorts = end->InOutPortMap[endPort];
	Port currPort;
	std::vector<CGRAEdge*> tempCGRAEdges;

	for (int i = 0; i < (*cgraEdges)[end].size(); ++i) {
		if(((*cgraEdges)[end][i].mappedDFGEdge == NULL)){
			currPort = (*cgraEdges)[end][i].SrcPort;
			for (int j = 0; j < candPorts.size(); ++j) {
				if(currPort == candPorts[j]){
					tempCGRAEdges.push_back(&(*cgraEdges)[end][i]);
				}
	 		}
		}
		else{
			if ((*cgraEdges)[end][i].mappedDFGEdge->getDest()->getASAPnumber() == currNode->getASAPnumber()){
				currPort = (*cgraEdges)[end][i].SrcPort;
				for (int j = 0; j < candPorts.size(); ++j) {
					if(currPort == candPorts[j]){
						errs() << "OccupiedBy=" << (*cgraEdges)[end][i].mappedDFGEdge->getDest()->getIdx();
//						errs() << ",CGRANode=" << (*cgraEdges)[end][i].mappedDFGEdge->getDest()->getMappedLoc()->getName() << "\n";
						tempCGRAEdges.push_back(&(*cgraEdges)[end][i]);
					}
		 		}
			}
		}
	}

	*mappingOutFile << "DeadEndStats :: CGRANode=" << end->getName();
	*mappingOutFile << ", utilization = " << std::to_string(util) << std::endl;
	errs() << "DeadEndStats :: CGRANode=" << end->getName() ;
	errs() << ", Port=" << currDFG->getCGRA()->getPortName(endPort);
	errs() << ", NodeASAPLevel=" << currNode->getASAPnumber();

//	errs() << ", CGRAEdgeSize=" << (*cgraEdges)[end].size();
	errs() << ", CGRAEdgeSize=" << tempCGRAEdges.size();

	errs() << "[";
	for (int i = 0; i < tempCGRAEdges.size(); ++i) {
		errs() << tempCGRAEdges[i]->Dst->getName().c_str() << ",";
	}
	errs() << "]";
	errs() << ", MII =" << currDFG->getCGRA()->getMII();
	errs() << ", util = " << util << "\n";
	errs() << ", oriEdgesSize = " << end->originalEdgesSize << "\n";

//	if(tempCGRAEdges.size() == 0){
//		return true;
//	}
	return false;
//	if(util >= end->originalEdgesSize){
//		return true;
//	}
//	return false;

}


