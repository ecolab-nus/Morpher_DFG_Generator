#include "dfg.h"
#include "llvm/Analysis/CFG.h"
#include "CGRA.h"
#include <algorithm>


dfgNode* DFG::getEntryNode(){
	if(NodeList.size() > 0){
		return &(NodeList[0]);
	}
	return NULL;
}

std::vector<dfgNode> DFG::getNodes(){
	return NodeList;
}

std::vector<Edge> DFG::getEdges(){
	return edgeList;
}

void DFG::InsertNode(Instruction* Node){
	dfgNode temp(Node, this);
	temp.setIdx(NodeList.size());
	NodeList.push_back(temp);
}

void DFG::InsertNode(dfgNode Node){
	errs() << "Inserted Node with Instruction : " << Node.getNode() << "\n";
	Node.setIdx(NodeList.size());
	NodeList.push_back(Node);
}

void DFG::InsertEdge(Edge e){
	edgeList.push_back(e);
}

dfgNode* DFG::findNode(Instruction* I){
	for(int i = 0 ; i < NodeList.size() ; i++){
		if (I == NodeList[i].getNode()){
			return &(NodeList[i]);
		}
	}
	return NULL;
}

Edge* DFG::findEdge(Instruction* src, Instruction* dest){
	for (int i = 0; i < edgeList.size(); ++i) {
		if(edgeList[i].getSrc() == src && edgeList[i].getDest() == dest) {
			return &(edgeList[i]);
		}
	}
	return NULL;
}

std::vector<dfgNode*> DFG::getRoots(){
	std::vector<dfgNode*> rootNodes;
	for (int i = 0 ; i < NodeList.size() ; i++) {
		if(NodeList[i].getChildren().size() == 0){
			rootNodes.push_back(&NodeList[i]);
		}
	}
	return rootNodes;
}

std::vector<dfgNode*> DFG::getLeafs(BasicBlock* BB){
	errs() << "start getting the LeafNodes...!\n";
	std::vector<dfgNode*> leafNodes;
	for (int i = 0 ; i < NodeList.size() ; i++) {
		if(NodeList[i].getNode()->getParent() == BB){
			leafNodes.push_back(&NodeList[i]);
		}
	}
	errs() << "LeafNodes init done...!\n";

	for (int i = 0 ; i < NodeList.size() ; i++) {
		if(NodeList[i].getNode()->getParent() == BB){
			for(int j = 0; j < NodeList[i].getChildren().size(); j++){
				dfgNode* nodeToBeRemoved = this->findNode(NodeList[i].getChildren()[j]);
				if(nodeToBeRemoved != NULL){
					errs() << "LeafNodes : nodeToBeRemoved found...! : ";
					nodeToBeRemoved->getNode()->dump();
					if (std::find(leafNodes.begin(), leafNodes.end(), nodeToBeRemoved) != leafNodes.end()){
						leafNodes.erase(std::remove(leafNodes.begin(),leafNodes.end(), nodeToBeRemoved));
					}
				}
			}
		}
	}
	errs() << "got the LeafNodes...!\n";
	return leafNodes;
}

void DFG::connectBB(){
	errs() << "ConnectBB called!\n";

	assert(NodeList.size() > 0);
	dfgNode firstNode = NodeList[0];
	SmallVector<std::pair<const BasicBlock *, const BasicBlock *>,1 > Result;
	FindFunctionBackedges(*(firstNode.getNode()->getFunction()),Result);

	for (int i = 0; i < Result.size(); ++i) {
		errs() << "Backedges .... :: \n";
		Result[i].first->dump();
		Result[i].second->dump();
		errs() << "\n";
	}


	std::vector<BasicBlock*> analysedBB;
	for (int i = 0 ; i < NodeList.size() ; i++) {
		if(NodeList[i].getNode()->getOpcode() == Instruction::Br){
			errs() << "$$$$$ This belongs to BB";
			NodeList[i].getNode()->getParent()->dump();
//			if (std::find(analysedBB.begin(), analysedBB.end(), NodeList[i].getNode()->getParent()) == analysedBB.end()){
//				analysedBB.push_back(NodeList[i].getNode()->getParent());
//			}
			BasicBlock* BB = NodeList[i].getNode()->getParent();
			succ_iterator SI(succ_begin(BB)), SE(succ_end(BB));
			 for (; SI != SE; ++SI){
				 BasicBlock* succ = *SI;
				 errs() << "$%$%$%$%$ successor Basic Blocks";
				 succ->dump();
				 errs() << "$%$%$%$%$\n";


				 std::pair <const BasicBlock*,const BasicBlock*> bbCouple(BB,succ);
				 if(std::find(Result.begin(),Result.end(),bbCouple)!=Result.end()){
					 continue;
				 }


//				 if (std::find(analysedBB.begin(), analysedBB.end(), succ) == analysedBB.end()){
//					 analysedBB.push_back(succ);
					 std::vector<dfgNode*> succLeafs = this->getLeafs(succ);
					 for (int j = 0; j < succLeafs.size(); j++){
						 NodeList[i].addChild(succLeafs[j]->getNode());
						 succLeafs[j]->addAncestor(NodeList[i].getNode());
					 }
//				 }
			 }
		}
	}
}

void DFG::printXML(std::string fileName){
	xmlFile.open(fileName.c_str());
	printDFGInfo();
	printHeaderTag("DFG-information");
	printOPs(1); //depth = 1
	printEdges(1);
	printFooterTag("DFG-information");
	xmlFile.close();
}

void DFG::printInEdges(dfgNode* node, int depth){
	printHeaderTag("In-edge-number",depth);
	xmlFile << node->getAncestors().size() ;
	printFooterTag("In-edge-number",depth);

	printHeaderTag("In-edges",depth);
	for (int i = 0; i < node->getAncestors().size() ; ++i) {
		printHeaderTag("Edge",depth+1);

		printHeaderTag("ID",depth+2);
//		xmlFile << node->getAncestors()[i] << "to" << node->getNode();
		xmlFile << findEdge(node->getAncestors()[i],node->getNode())->getID() ;
		printFooterTag("ID",depth+2);

		printFooterTag("Edge",depth+1);
	}

	printFooterTag("In-edges",depth);
}

void DFG::printOutEdges(dfgNode* node, int depth){
	printHeaderTag("Out-edge-number",depth);
	xmlFile << node->getChildren().size() ;
	printFooterTag("Out-edge-number",depth);

	printHeaderTag("Out-edges",depth);
	for (int i = 0; i < node->getChildren().size() ; ++i) {
		printHeaderTag("Edge",depth+1);

		printHeaderTag("ID",depth+2);
//		xmlFile << node->getNode() << "to" << node->getChildren()[i];
		xmlFile << findEdge(node->getNode(),node->getChildren()[i])->getID() ;
		printFooterTag("ID",depth+2);

		printFooterTag("Edge",depth+1);
	}

	printFooterTag("Out-edges",depth);
}

void DFG::printOP(dfgNode* node, int depth){
	assert(xmlFile.is_open());
	printHeaderTag("OP",depth);

	printHeaderTag("ID",depth+1);
//	xmlFile << (node->getNode());
	xmlFile << node->getIdx();
	printFooterTag("ID",depth+1);

	printHeaderTag("OP-type",depth+1);
	xmlFile << "NORMAL"; //TODO :: Hardcoded to be Normal, fix the hardcoding
	printFooterTag("OP-type",depth+1);

	printHeaderTag("Cycles",depth+1);
	xmlFile << 1; //TODO :: Remove the hardcoding
	printFooterTag("Cycles",depth+1);

	printInEdges(node, depth+1);
	printOutEdges(node,depth+1);

	printFooterTag("OP",depth);
}

void DFG::printOPs(int depth){
	assert(xmlFile.is_open());
	printHeaderTag("OPs",depth);

	printHeaderTag("OP-number",depth+1);
	xmlFile << NodeList.size();
	printFooterTag("OP-number",depth+1);

	for (int i = 0; i < NodeList.size(); ++i) {
		printOP(&(NodeList[i]),depth+1);
	}
	printFooterTag("OPs",depth);
}

void DFG::printDFGInfo(){
	assert(xmlFile.is_open());
	xmlFile << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
}

void DFG::printHeaderTag(std::string tagName, int depth){
	assert(xmlFile.is_open());
	xmlFile << std::endl;
	for (int i = 0; i < depth; ++i) {
		xmlFile << "\t";
	}
	xmlFile << "<" << tagName << ">" ;
}

void DFG::printFooterTag(std::string tagName, int depth){
	assert(xmlFile.is_open());
	xmlFile << std::endl;
	for (int i = 0; i < depth; ++i) {
		xmlFile << "\t";
	}
	xmlFile << "</" << tagName << ">";
}

void DFG::printEdges(int depth) {
	assert(xmlFile.is_open());
	printHeaderTag("EDGEs",depth);

	printHeaderTag("Edge-number",depth);
	xmlFile << edgeList.size();
	printFooterTag("Edge-number",depth);

	for (int i = 0; i < edgeList.size(); ++i) {
		printEdge(&(edgeList[i]),depth);
	}

	printFooterTag("EDGEs",depth);
}

void DFG::renumber() {
	for (int i = 0; i < NodeList.size(); ++i) {
		NodeList[i].setIdx(i);
	}

	for (int i = 0; i < edgeList.size(); ++i) {
		edgeList[i].setID(i);
	}
}

void DFG::printEdge(Edge* e, int depth) {
	assert(xmlFile.is_open());
	printHeaderTag("EDGE",depth);

	printHeaderTag("ID",depth+1);
	xmlFile << e->getID();
	printFooterTag("ID",depth+1);

	printHeaderTag("Attribute",depth+1);
	xmlFile << 1; //God knows what this is
	printFooterTag("Attribute",depth+1);

	printHeaderTag("Start-OP",depth+1);
//	xmlFile << e->getSrc();
	xmlFile << findNode(e->getSrc())->getIdx();
	printFooterTag("Start-OP",depth+1);

	printHeaderTag("End-OP",depth+1);
//	xmlFile << e->getDest();
	xmlFile << findNode(e->getDest())->getIdx();
	printFooterTag("End-OP",depth+1);

	printFooterTag("EDGE",depth);
}

void DFG::addMemDepEdges(MemoryDependenceAnalysis *MD) {

	assert(NodeList.size() > 0);
	dfgNode firstNode = NodeList[0];
	SmallVector<std::pair<const BasicBlock *, const BasicBlock *>,1 > Result;
	FindFunctionBackedges(*(firstNode.getNode()->getFunction()),Result);

	Instruction* it;
	MemDepResult mRes;
	SmallVector<NonLocalDepResult,1> result;
	for (int i = 0; i < NodeList.size(); ++i) {
		it = NodeList[i].getNode();
		if(it->mayReadOrWriteMemory()){
			mRes = MD->getDependency(it);

			errs() << "Dependent :";
			it->dump();

			if(mRes.isNonLocal()){
				if(auto CS = CallSite(it)){

				}
				else{
					errs() << "****** GET NON LOCAL MEM DEPENDENCIES ******\n";

					MD->getNonLocalPointerDependency(it,result);

					for (int j = 0; j < result.size(); ++j) {
						if(result[j].getResult().getInst() != NULL){

							if((result[j].getResult().getInst()->getOpcode() == Instruction::Load)&&(it->getOpcode() == Instruction::Load)){

							}
							else{

								 std::pair <const BasicBlock*,const BasicBlock*> bbCouple(result[j].getResult().getInst()->getParent(),it->getParent());
								 if(std::find(Result.begin(),Result.end(),bbCouple)!=Result.end()){
									 continue;
								 }

								result[j].getResult().getInst()->dump();
								this->findNode(result[j].getResult().getInst())->addChild(it,EDGE_TYPE_LDST);
								NodeList[i].addAncestor(result[j].getResult().getInst());
							}

						}
					}

					errs() << "****** DONE NON LOCAL MEM DEPENDENCIES ******\n";
				}


			}


			if(mRes.getInst() != NULL){

				if( (mRes.getInst()->getOpcode() == Instruction::Load)&&(it->getOpcode() == Instruction::Load)){
//					continue;
				}
				else {
					this->findNode(mRes.getInst())->addChild(it,EDGE_TYPE_LDST);
					NodeList[i].addAncestor(mRes.getInst());
				}
			}
		}
	}
}

int DFG::removeEdge(Edge* e) {
	Edge* edgePtr;
	edgePtr = findEdge(e->getSrc(),e->getDest());
	if(edgePtr == NULL){
		return -1;
	}

	for (int i = 0; i < edgeList.size(); ++i) {
		if(edgePtr == &(edgeList[i])){
			edgeList.erase(edgeList.begin()+i);
		}
	}

	return 1;
}

int DFG::removeNode(dfgNode* n) {
	dfgNode* nodePtr;
	nodePtr = findNode(n->getNode());
	if(nodePtr == NULL){
		return -1;
	}

	dfgNode* nodeTmp;
	dfgNode* nodeTmp2;
	Edge* edgeTmp;

	//Treat the ancestors
	for (int i = 0; i < nodePtr->getAncestors().size(); ++i) {
		nodeTmp = findNode(nodePtr->getAncestors()[i]);
		assert(nodeTmp->removeChild(nodePtr->getNode()) != -1);

		for (int j = 0; j < nodePtr->getChildren().size(); ++j) {
			nodeTmp->addChild(nodePtr->getChildren()[j]);  //TODO : Generalize for types of children
			nodeTmp2 = findNode(nodePtr->getChildren()[j]);

			assert(nodeTmp2->removeAncestor(nodePtr->getNode()) != -1);
			nodeTmp2->addAncestor(nodePtr->getAncestors()[i]);
		}

		edgeTmp = findEdge(nodePtr->getAncestors()[i],nodePtr->getNode());
		assert(removeEdge(edgeTmp) != -1);
	}

	//Treat the children
	for (int i = 0; i < nodePtr->getChildren().size(); ++i) {
		nodeTmp = findNode(nodePtr->getChildren()[i]);
		assert(nodeTmp->removeAncestor(nodePtr->getNode()) != -1);

		for (int j = 0; j < nodePtr->getAncestors().size(); ++j) {
			nodeTmp->addAncestor(nodePtr->getAncestors()[j]);
			nodeTmp2 = findNode(nodePtr->getAncestors()[j]);

			assert(nodeTmp2->removeChild(nodePtr->getNode()) != -1);
			nodeTmp2->addChild(nodePtr->getChildren()[i]);
		}

		edgeTmp = findEdge(nodePtr->getNode(),nodePtr->getChildren()[i]);
		assert(removeEdge(edgeTmp) != -1);
	}

//	NodeList.erase(std::remove(NodeList.begin(),NodeList.end(),*n),NodeList.end());

	for (int i = 0; i < NodeList.size(); ++i) {
		if(nodePtr == &(NodeList[i])){
			NodeList.erase(NodeList.begin()+i);
		}
	}

	return 1;
}

void DFG::removeAlloc() {
	Instruction* insTmp;
	for (int i = 0; i < NodeList.size(); ++i) {
		insTmp = NodeList[i].getNode();
		if(insTmp->getOpcode() == Instruction::Alloca){
			removeNode(&(NodeList[i]));
		}
	}
	renumber();
}

void DFG::traverseDFS(dfgNode* startNode, int dfsCount) {
//	//Assumption should be a connected DFG
//	assert(getRoots().size()==1);
//
//	for (int i = 0; i < startNode->getChildren(); ++i) {
//
//	}

}

void DFG::traverseBFS(dfgNode* node, int ASAPlevel) {



	dfgNode* child;
	for (int i = 0; i < node->getChildren().size(); ++i) {

		if(maxASAPLevel < ASAPlevel){
			maxASAPLevel = ASAPlevel;
		}

		child = findNode(node->getChildren()[i]);
		if(child->getASAPnumber() < ASAPlevel){
			child->setASAPnumber(ASAPlevel);
			traverseBFS(child,ASAPlevel+1);
		}
	}
}

void DFG::traverseInvBFS(dfgNode* node, int ALAPlevel) {
	dfgNode* ancestor;
	for (int i = 0; i < node->getAncestors().size(); ++i) {
		ancestor = findNode(node->getAncestors()[i]);
		if(ancestor->getALAPnumber() < ALAPlevel){
			ancestor->setALAPnumber(ALAPlevel);
			traverseInvBFS(ancestor,ALAPlevel+1);
		}
	}
}

void DFG::scheduleASAP() {
	std::vector<dfgNode*> leafs;
	leafs = getLeafs();
	int nodesVisited = 0;

	for (int i = 0; i < leafs.size(); ++i) {
		leafs[i]->setASAPnumber(0);
		traverseBFS(leafs[i],1);
	}
}

void DFG::scheduleALAP() {
	dfgNode* node;

	int currASAPnumber = -1;
	std::vector<dfgNode*> roots;
//	for (int i = 0; i < NodeList.size(); ++i) {
//		node = &(NodeList[i]);
////		assert(node->getASAPnumber() != -1);
//		if(node->getASAPnumber() > currASAPnumber){
//			currASAPnumber = node->getASAPnumber();
//		}
//	}

//	for (int i = 0; i < NodeList.size(); ++i) {
//		if(node->getASAPnumber() == currASAPnumber){
//			roots.push_back(node);
//		}
//	}

	roots = getRoots();

	for (int i = 0; i < roots.size(); ++i) {
		roots[i]->setALAPnumber(0);
		traverseInvBFS(roots[i],1);
	}

	for (int i = 0; i < NodeList.size(); ++i) {
		NodeList[i].setALAPnumber(maxASAPLevel - NodeList[i].getALAPnumber());
	}



}

std::vector<dfgNode*> DFG::getLeafs() {
	std::vector<dfgNode*> leafNodes;
	for (int i = 0 ; i < NodeList.size() ; i++) {
		if(NodeList[i].getAncestors().size() == 0){
			leafNodes.push_back(&NodeList[i]);
		}
	}
	return leafNodes;
}






void DFG::MapCGRA(int XDim, int YDim) {

	//TODO : Add recurrence constrained MII
	int MII = ceil((float)NodeList.size()/((float)XDim*(float)YDim));
	std::vector<ConnectedCGRANode> candidateCGRANodes;
	dfgNode* temp;

	struct cand{
		int max = 0;
		int curr = 0;

		cand(int m, int c) : max(m), curr(c){}
	};

	std::vector<cand> chosenCandidates;


	while(1){
		errs() << "Mapping started with MII = " << MII << "\n";
		currCGRA = new CGRA(MII,XDim,YDim);
		int nodeListSequencer = 0;
		int min_nodeListSequencer = NodeList.size();

		for (int i = 0; i < NodeList.size(); ++i) {
			chosenCandidates.push_back(cand(0,0));
		}

		while(1){
			temp = &NodeList[nodeListSequencer];
			errs() << "Mapping " << nodeListSequencer << "/" << NodeList.size() << "..." << ", with MII = " << MII << "\n";

			if(nodeListSequencer == NodeList.size()){
				errs() << "Mapping done...\n";
				break;
			}


			errs() << "Finding candidates for NodeSeq =" << nodeListSequencer << "...\n";
			candidateCGRANodes = FindCandidateCGRANodes(temp);
			errs() << "Found candidates : " << candidateCGRANodes.size() << "\n";

//			for (int i = 0; i < candidateCGRANodes.size(); ++i) {
//				errs() << "(t,y,x) = (" << candidateCGRANodes[i].node->getT() << ","
//										<< candidateCGRANodes[i].node->getY() << ","
//										<< candidateCGRANodes[i].node->getX() << ")\n";
//			}

			//Remove this special Debug
//			if(nodeListSequencer == 28){
//				if(currCGRA->getCGRANode(2,2,1)->getmappedDFGNode() == NULL){
//					errs() << "SPECIAL_DEBUG : (221) is NULL\n";
//				}
//				else{
//					errs() << "SPECIAL_DEBUG : (221) is "<< currCGRA->getCGRANode(2,2,1)->getmappedDFGNode()->getIdx() <<"\n";
//				}
//			}


			if(candidateCGRANodes.size() == 0){
				do{
					chosenCandidates[nodeListSequencer].curr = 0;
					nodeListSequencer--;
					backTrack(nodeListSequencer);

					errs() << "nodeListSequencer =" << nodeListSequencer << "\n";
				}while((chosenCandidates[nodeListSequencer].curr + 1 >= chosenCandidates[nodeListSequencer].max)
						&&(nodeListSequencer >= 0));
				errs() << "Backtracked to nodeListSequencer =" << nodeListSequencer << "\n";

				if(nodeListSequencer < 0){
					errs() << "Mapping failed for MII = "<< MII << "...\n";
					break;
				}

				chosenCandidates[nodeListSequencer].curr = chosenCandidates[nodeListSequencer].curr + 1;
			}
			else{
				errs() << "current candidate = " << chosenCandidates[nodeListSequencer].curr << "\n";
				chosenCandidates[nodeListSequencer].max = candidateCGRANodes.size();
				candidateCGRANodes[chosenCandidates[nodeListSequencer].curr].node->setMappedDFGNode(temp);
				temp->setMappedLoc(candidateCGRANodes[chosenCandidates[nodeListSequencer].curr].node);

				errs() << "Mapped node with sequence =" << nodeListSequencer << "\n";
				errs() << "(t,y,x) = (" << candidateCGRANodes[chosenCandidates[nodeListSequencer].curr].node->getT()  << ","
									    << candidateCGRANodes[chosenCandidates[nodeListSequencer].curr].node->getY()  << ","
									    << candidateCGRANodes[chosenCandidates[nodeListSequencer].curr].node->getX()  << ")\n";
//				errs() << "(t,y,x) = (" << currCGRA->getCGRANode(temp->getMappedLoc()->getT(),temp->getMappedLoc()->getY(),temp->getMappedLoc()->getX())->getT()  << ","
//									    << currCGRA->getCGRANode(temp->getMappedLoc()->getT(),temp->getMappedLoc()->getY(),temp->getMappedLoc()->getX())->getY()   << ","
//									    << currCGRA->getCGRANode(temp->getMappedLoc()->getT(),temp->getMappedLoc()->getY(),temp->getMappedLoc()->getX())->getX()   << ")\n";
				nodeListSequencer++;
			}
//			delete(&candidateCGRANodes);
		}

		if(nodeListSequencer == NodeList.size()){
			errs() << "Mapping done...\n";
			break;
		}


		MII++;
		delete(currCGRA);
	}

}

void DFG::traverseCriticalPath(dfgNode* node, int level) {
//	dfgNode* temp;
//	int i;
//	for (i = 0; i < node->getChildren().size(); ++i) {
//		temp = findNode(node->getChildren()[i]);
//		if(temp->getASAPnumber() == temp->getALAPnumber()) {
//
//		}
//	}

}

void DFG::CreateSchList() {
	dfgNode* temp;

	errs() << "#################Begin :: The Schedule List#################\n";
	std::sort(NodeList.begin(),NodeList.end(),ScheduleOrder());
	for (int i = 0; i < NodeList.size(); ++i) {
		temp = &NodeList[i];
		temp->setSchIdx(i);

		errs() << "NodeIdx=" << temp->getIdx() << ", ASAP =" << temp->getASAPnumber() << ", ALAP =" << temp->getALAPnumber() << "\n";
	}
	errs() << "#################End :: The Schedule List#################\n";

}

std::vector<ConnectedCGRANode> DFG::searchCandidates(CGRANode* mappedLoc, dfgNode* node, std::vector<std::pair<Instruction*,int>>* candidateNumbers) {
			std::vector<ConnectedCGRANode> candidates = mappedLoc->getConnectedNodes();
			eraseAlreadyMappedNodes(&candidates);
			candidateNumbers->push_back(std::make_pair(node->getAncestors()[0],candidates.size()));
			dfgNode* temp;
	//		candidateNumbers[node->getAncestors()[0]] = candidates.size();

			std::vector<ConnectedCGRANode> candidates2;
			std::vector<ConnectedCGRANode> candidates3;
			bool matchFound = false;

			for (int i = 0; i < node->getAncestors().size(); ++i) {
				if(mappedLoc->getmappedDFGNode()->getNode() != node->getAncestors()[i]) {
					temp = findNode(node->getAncestors()[i]);
					assert(temp->getMappedLoc() != NULL);
					candidates2 = temp->getMappedLoc()->getConnectedNodes();
					eraseAlreadyMappedNodes(&candidates2);

					candidateNumbers->push_back(std::make_pair(node->getAncestors()[i],candidates2.size()));

					for (int j = 0; j < candidates.size(); ++j) {
						for (int k = 0; k < candidates2.size(); ++k) {
							if(candidates[j].node == candidates2[k].node){
								candidates[j].cost = candidates[j].cost + candidates2[k].cost;
								matchFound = true;
								break;
							}
						}
						if(matchFound){
							candidates3.push_back(candidates[j]);
						}
						matchFound = false;
					}
					candidates = candidates3;
				}
			}
			eraseAlreadyMappedNodes(&candidates);
			return candidates;
}

void DFG::eraseAlreadyMappedNodes(std::vector<ConnectedCGRANode>* candidates) {
	std::vector<ConnectedCGRANode>::iterator it = candidates->begin();

	while(it != candidates->end()){
		if(it->node->getmappedDFGNode() != NULL){
//			errs() << "eraseAlreadyMappedNodes :: Already mapped = (" << it->node->getT() << ","
//																	  << it->node->getY() << ","
//																	  << it->node->getX() << ")\n";
			it = candidates->erase(it);
		}
		else {
			++it;
		}
	}

}

void DFG::backTrack(int nodeSeq) {
	dfgNode* temp = &NodeList[nodeSeq];
	dfgNode* anc;
//	errs() << "Backtrack : NodeSeq=" << nodeSeq << "\n";


	if(temp->getMappedLoc() != NULL){
//		errs() << "(t,y,x) = (" << temp->getMappedLoc()->getT() << "," << temp->getMappedLoc()->getY() << "," << temp->getMappedLoc()->getX() << "\n";
		temp->getMappedLoc()->setMappedDFGNode(NULL);
		temp->setMappedLoc(NULL);
	}

	for (int i = 0; i < temp->getAncestors().size(); ++i) {
		anc = findNode(temp->getAncestors()[i]);

		std::vector<CGRANode*>::iterator it = anc->getRoutingLocs()->begin();

		while(it != anc->getRoutingLocs()->end()){
//			errs() << "BackTrack :: erasing routing=(" << (*it)->getT() << ","
//													   << (*it)->getY() << ","
//													   << (*it)->getX() << ")\n";
			(*it)->setMappedDFGNode(NULL);
			it = anc->getRoutingLocs()->erase(it);
		}
	}




}

std::vector<ConnectedCGRANode> DFG::FindCandidateCGRANodes(dfgNode* node) {
	dfgNode* temp;
	std::vector<ConnectedCGRANode> candidates;
	std::vector<std::pair<Instruction*,int>> candidateNumbers;
	std::vector<std::pair<Instruction*,int>> candidateNumbersLvl2;

	assert(currCGRA != NULL);

	if(node->getAncestors().size() == 0){
		for (int t = 0; t < currCGRA->getMII(); ++t) {
			for (int y = 0; y < currCGRA->getYdim(); ++y) {
				for (int x = 0; x < currCGRA->getXdim(); ++x) {
					if(currCGRA->getCGRANode(t,y,x)->getmappedDFGNode() == NULL){
						ConnectedCGRANode tempCGRANode(currCGRA->getCGRANode(t,y,x),0,"startNodes");
						candidates.push_back(tempCGRANode);
					}
				}
			}
		}
		return candidates;
	}
	else if (node->getAncestors().size() == 1){
		temp = findNode(node->getAncestors()[0]);
		assert(temp->getMappedLoc() != NULL);
		candidates = temp->getMappedLoc()->getConnectedNodes();

//		for (int i = 0; i < candidates.size(); ++i) {
//			if(candidates[i].node->getmappedDFGNode() != NULL){
//				candidates.erase(candidates.begin() + i);
//			}
//		}
		eraseAlreadyMappedNodes(&candidates);

		return candidates;
	} else{ //(node->getAncestors().size() > 1
		temp = findNode(node->getAncestors()[0]);

		if(temp->getMappedLoc() == NULL){
			errs() << "Unmapped ancestor : " << temp->getIdx() << "\n";
		}

		assert(temp->getMappedLoc() != NULL);

		if(temp->getMappedLoc()->getmappedDFGNode() == NULL){
			errs() << "Unmapped ancestor : " << temp->getIdx() << "\n";
		}

		assert(temp->getMappedLoc()->getmappedDFGNode() != NULL);
		candidateNumbers.clear();
		candidates = searchCandidates(temp->getMappedLoc(),node,&candidateNumbers);

//		errs() << "FindCandidateCGRANodes :: candidates.size()=" << candidates.size() << "\n";

		std::vector<ConnectedCGRANode> candidates2;

		if(candidates.size() == 0){
			std::sort(candidateNumbers.begin(),candidateNumbers.end(),ValueComparer());
			for (int i = 0; i < candidateNumbers.size(); ++i) {
				temp = findNode(candidateNumbers[i].first);
				candidates2 = temp->getMappedLoc()->getConnectedNodes();
				eraseAlreadyMappedNodes(&candidates2);
				std::sort(candidates2.begin(),candidates2.end(),CostComparer());

				for (int j = 0; j < candidates2.size(); ++j) {
//					errs() << "candidates2 = (" << candidates2[j].node->getT() << ","
//							   	   	   	   	    << candidates2[j].node->getY() << ","
//												<< candidates2[j].node->getX() << ")\n";
				}

				//search one-level with routing nodes
				for (int j = 0; j < candidates2.size(); ++j) {
					candidates2[j].node->setMappedDFGNode(temp);
					candidates = searchCandidates(candidates2[j].node,node,&candidateNumbersLvl2);
					if(candidates.size() == 0){
						candidates2[j].node->setMappedDFGNode(NULL);
					}else{
//						errs() << "FindCandidateCGRANodes :: Routing node added=(" << candidates2[j].node->getT() << ","
//																				   << candidates2[j].node->getY() << ","
//																				   << candidates2[j].node->getX() << ")\n";
						temp->getRoutingLocs()->push_back(candidates2[j].node);
						break;
					}
				}
				if(candidates.size() != 0){
					break;
				}
			}


		}

		if(candidates.size() == 0){
			errs() << "No Candidates found for NODE_ID:" << node->getIdx() << "\n";
			errs() << "Need to backtrack...\n";
			return candidates;
		}

		std::sort(candidates.begin(),candidates.end(),CostComparer());
		return candidates;
	}


}
