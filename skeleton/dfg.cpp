#include "dfg.h"
#include "llvm/Analysis/CFG.h"
#include "CGRA.h"
#include <algorithm>
#include <queue>
#include "astar.h"


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

void DFG::addMemRecDepEdges(DependenceAnalysis* DA) {
	std::vector<dfgNode*> memNodes;
	dfgNode* nodePtr;
	Instruction *Ins;
	std::ofstream log;

	static int count = 0;
	std::string Filename = (NodeList[0].getNode()->getFunction()->getName() + "_L" + std::to_string(count++) + "addMemRecDepEdges.log").str();
	log.open(Filename.c_str());
	log << "Started...\n";

	errs() << "&&&&&&&&&&&&&&&&&&&&Recurrence search started.....!\n";

	//Create a list of memory instructions
	for (int i = 0; i < NodeList.size(); ++i) {
		nodePtr = &NodeList[i];
		Ins = dyn_cast<Instruction>(nodePtr->getNode());
		if (!Ins)
		return;

		LoadInst *Ld = dyn_cast<LoadInst>(nodePtr->getNode());
		StoreInst *St = dyn_cast<StoreInst>(nodePtr->getNode());

		if (!St && !Ld)
		continue;
		if (Ld && !Ld->isSimple())
		return;
		if (St && !St->isSimple())
		return;
		errs() << "ID=" << nodePtr->getIdx() << " ,";
		nodePtr->getNode()->dump();
		memNodes.push_back(nodePtr);
	}

	log << "addMemRecDepEdges : Found " << memNodes.size() << "Loads and Stores to analyze\n";

	for (int i = 0; i < memNodes.size(); ++i) {
		for (int j = i; j < memNodes.size(); ++j) {
			  std::vector<char> Dep;
		      Instruction *Src = dyn_cast<Instruction>(memNodes[i]->getNode());
		      Instruction *Des = dyn_cast<Instruction>(memNodes[j]->getNode());

			  if (Src == Des)
				  continue;

			  if (isa<LoadInst>(Src) && isa<LoadInst>(Des))
				  continue;

			  if (auto D = DA->depends(Src, Des, true)) {
				  log << "addMemRecDepEdges :" << "Found Dependency between Src=" << memNodes[i]->getIdx() << " Des=" << memNodes[j]->getIdx() << "\n";


				  if (D->isFlow()) {
				// TODO: Handle Flow dependence.Check if it is sufficient to populate
				// the Dependence Matrix with the direction reversed.
					  log << "addMemRecDepEdges :" << "Flow dependence not handled\n";
					  continue;
//					  return;
				  }
				  if (D->isAnti()) {
					  log << "Found Anti dependence \n";

						this->findNode(Src)->addChild(Des,EDGE_TYPE_LDST);
						this->findNode(Des)->addAncestor(Src);

					  unsigned Levels = D->getLevels();
					  char Direction;
					  for (unsigned II = 1; II <= Levels; ++II) {
						  const SCEV *Distance = D->getDistance(II);

						  const SCEVConstant *SCEVConst = dyn_cast_or_null<SCEVConstant>(Distance);
						  if (SCEVConst) {
							  const ConstantInt *CI = SCEVConst->getValue();
							  if (CI->isNegative()) {
								  Direction = '<';
							  }
							  else if (CI->isZero()) {
								  Direction = '=';
							  }
							  else {
								  Direction = '>';
							  }
							  log << SCEVConst->getAPInt().abs().getZExtValue() << std::endl;
							  Dep.push_back(Direction);
						  }
						  else if (D->isScalar(II)) {
							Direction = 'S';
							Dep.push_back(Direction);
						  }
						  else {
							  unsigned Dir = D->getDirection(II);
							  if (Dir == Dependence::DVEntry::LT || Dir == Dependence::DVEntry::LE){
								  Direction = '<';
							  }
							  else if (Dir == Dependence::DVEntry::GT || Dir == Dependence::DVEntry::GE){
								  Direction = '>';
							  }
							  else if (Dir == Dependence::DVEntry::EQ) {
								  Direction = '=';
							  }
							  else {
								  Direction = '*';
							  }
							  Dep.push_back(Direction);
						  }
					  }
//					while (Dep.size() != Level) {
//					  Dep.push_back('I');
//					}
				  }

				  for (int k = 0; k < Dep.size(); ++k) {
					  log << Dep[k];
				  }
				  log << "\n";
			  }
		}
	}



	log.close();
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
		if(insTmp->getOpcode() == Instruction::Alloca /*|| insTmp->getOpcode() == Instruction::PHI*/){
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

std::vector<std::vector<int> > DFG::selfMulConMat(
		std::vector<std::vector<int> > in) {

	std::vector<std::vector<int> > out;

	for (int i = 0; i < in.size(); ++i) {
		std::vector<int> tempVec;
		for (int j = 0; j < in.size(); ++j) {
			int tempInt = 0;
			for (int k = 0; k < in.size(); ++k) {
				tempInt = tempInt | (in[i][k] & in[k][j]);
			}
			tempVec.push_back(tempInt);
		}
		out.push_back(tempVec);
	}

	return out;
}



std::vector<std::vector<int> > DFG::getConMat() {

	int nodelistSize = NodeList.size();

	std::vector<std::vector<int> > conMat;
	dfgNode* node;
	dfgNode* child;

	for (int i = 0; i < nodelistSize; ++i) {
		std::vector<int> temp;
		for (int j = 0; j < nodelistSize; ++j) {
			temp.push_back(0);
		}
		conMat.push_back(temp);
	}

	for (int i = 0; i < nodelistSize; ++i) {
		node = &NodeList[i];
		for (int j = 0; j < node->getChildren().size(); ++j) {
			child = findNode(node->getChildren()[j]);
			conMat[node->getIdx()][child->getIdx()] = 1;
		}
	}

	return conMat;
}

int DFG::getAffinityCost(dfgNode* a, dfgNode* b) {
	int nodelistSize = NodeList.size();

	assert(a->getASAPnumber() == b->getASAPnumber());
	int currLevel = a->getASAPnumber();
	int maxDist = maxASAPLevel - currLevel;

	int affLevel = 0;
	std::vector<std::vector<int> > conMat = getConMat();
	int affCost = 0;

	while(1) {
		if(affLevel == maxDist) {
			return affCost;
		}


		for (int i = 0; i < nodelistSize; ++i) {

			if(conMat[a->getIdx()][i] == 1) {
				if(conMat[b->getIdx()][i] == 1){
//					affCost = affCost + 2**(maxDist - (affLevel+1));
					affCost = affCost + (int)pow(2,(maxDist - (affLevel+1)));
				}
			}


		}

		conMat = selfMulConMat(conMat);
		affLevel++;
	}
}

std::vector<int> DFG::getIntersection(std::vector<std::vector<int> > vectors) {

	assert(vectors.size() != 0);
	std::vector<int> result;

	for (int i = 0; i < vectors[0].size(); ++i) {
		result.push_back(1);
	}


	for (int i = 1; i < vectors.size(); ++i) {
		assert(vectors[i].size() == vectors[0].size());
		for (int j = 0; j < vectors[i].size(); ++j) {
			result[j] = result[j] & vectors[i][j];
		}
	}

	return result;
}



int DFG::getDist(CGRANode* src, CGRANode* dest) {
	assert(src->getmappedDFGNode() != NULL);
	assert(src->getmappedDFGNode() != NULL);

	int xCost = abs(src->getX() - dest->getX());
	int yCost = abs(src->getY() - dest->getY());
	int tCost = dest->getT() - src->getT();

	if(tCost < 0){
		return INT32_MAX;
	}

	if(xCost + yCost > tCost){
		return INT32_MAX;
	}

	return xCost + yCost;
}

int DFG::AStarSP(CGRANode* src, CGRANode* dest, std::vector<CGRANode*>* path) {

	struct LessThanFScore
	{
		bool operator()(const std::pair<CGRANode*,int>& left, const std::pair<CGRANode*,int>& right) const
		{
			return left.second < right.second;
		}
	};

	path->clear();
	std::vector<ConnectedCGRANode> connectedNodes;
	std::vector<CGRANode*> closedSet;
	std::vector<CGRANode*> openSet;

	std::map<CGRANode*,int> fscore;
	std::map<CGRANode*,int> gscore;
	std::map<CGRANode*,CGRANode*> prevNode;
//	std::priority_queue<AStarNode, std::vector<AStarNode>, LessThanCost> openSet;

	openSet.push_back(src);
	fscore[src]=getDist(src,dest);

	if(fscore[src] == INT32_MAX){
		return -1;
	}

	gscore[src]=0;

	CGRANode* current;
	int tentgscore;
	int dist;
	while(!openSet.empty()){
//		anode = openSet.top();
		std::map<CGRANode*,int>::iterator it = std::min_element(fscore.begin(),fscore.end(),LessThanFScore());
		current = (*it).first;
		fscore.erase(it);
		openSet.erase(std::remove(openSet.begin(), openSet.end(), current), openSet.end());
		closedSet.push_back(current);
//		openSet.pop();

		if(current == dest){
			break;
		}


		connectedNodes = current->getConnectedNodes();
		for (int i = 0; i < connectedNodes.size(); ++i) {

			if(connectedNodes[i].node->getmappedDFGNode() != NULL){
				continue;
			}

			dist = getDist(current,connectedNodes[i].node);
			if(dist == INT32_MAX || gscore[current]==INT32_MAX){
				tentgscore = INT32_MAX;
			}
			else{
				tentgscore = gscore[current] + getDist(current,connectedNodes[i].node);
			}

			if (std::find(closedSet.begin(), closedSet.end(),connectedNodes[i].node)!=closedSet.end()){
				continue;
			}

			if (std::find(openSet.begin(), openSet.end(),connectedNodes[i].node)==openSet.end()){
				openSet.push_back(connectedNodes[i].node);
			}
			else{
				if (gscore.find(connectedNodes[i].node) == gscore.end() ) {
					gscore[connectedNodes[i].node] = INT32_MAX;
				}

				if(tentgscore >= gscore[connectedNodes[i].node]){
					continue;
				}
			}

			prevNode[connectedNodes[i].node] = current;
			gscore[connectedNodes[i].node] = tentgscore;

			dist = getDist(connectedNodes[i].node,dest);
			if(dist == INT32_MAX){
	            fscore[connectedNodes[i].node] = INT32_MAX;
			}
			else{
	            fscore[connectedNodes[i].node] = tentgscore + getDist(connectedNodes[i].node,dest);
			}

		}
	}

	int routeCost = 0;

	if(current != dest){
		return -1;
	}
	else{
		while(current != src){
			path->push_back(current);
			current = prevNode[current];
			routeCost++;
		}
		return routeCost;
	}

}

int DFG::AddRoutingEdges(dfgNode* node){

	std::vector<dfgNode*> parents;
	CGRANode* cnode;
	std::vector<std::vector<int> > parentConnectedPhyNodes;
	std::vector<int> parentIntersectionTmp;
	int routingPossibilities=0;



	for (int j = 0; j < node->getAncestors().size(); ++j) {
		parents.push_back(findNode(node->getAncestors()[j]));
	}

	for (int j = 0; j < parents.size(); ++j) {
		parentConnectedPhyNodes.push_back(currCGRA->getPhyConMatNode(parents[j]->getMappedLoc()));
	}

	do{
		parentIntersectionTmp = getIntersection(parentConnectedPhyNodes);

		routingPossibilities = 0;
		for (int j = 0; j < parentIntersectionTmp.size(); ++j) {
			cnode = currCGRA->getCGRANode(j);
			if(cnode->getmappedDFGNode() != NULL){
				parentIntersectionTmp[j] = 0;
			}

			routingPossibilities = routingPossibilities + parentIntersectionTmp[j];
		}

		if(routingPossibilities == 0){

		}

	}while(routingPossibilities == 0);


}


std::map<dfgNode*, std::vector<CGRANode*> > DFG::getPrimarySlots(
		std::vector<dfgNode*> nodes) {

	dfgNode* node;
//	dfgNode* parent;
	CGRANode* cnode;

	std::map<dfgNode*, std::vector<CGRANode*> > result;
	std::vector<dfgNode*> parents;

	std::vector<std::vector<int> > parentConnectedPhyNodes;
	std::vector<int> parentIntersection;
	std::vector<int> parentIntersectionTmp;

	std::vector<std::vector<int> > phyConMat;


	bool commonConsumer = false;
	float probCost;
	int routingPossibilities = 0;

	for (int i = 0; i < nodes.size(); ++i) {
		node = nodes[i];
		for (int j = 0; j < node->getAncestors().size(); ++j) {
			parents.push_back(findNode(node->getAncestors()[j]));
		}


		for (int j = 0; j < parents.size(); ++j) {
			parentConnectedPhyNodes.push_back(currCGRA->getPhyConMatNode(parents[j]->getMappedLoc()));
		}

		parentIntersectionTmp = getIntersection(parentConnectedPhyNodes);

		for (int j = 0; j < parentIntersectionTmp.size(); ++j) {
			cnode = currCGRA->getCGRANode(j);
			if(cnode->getmappedDFGNode() != NULL){
				parentIntersectionTmp[j] = 0;
			}

			routingPossibilities = routingPossibilities + parentIntersectionTmp[j];
		}

		if(routingPossibilities == 0){

		}


		node->setRoutingPossibilities(routingPossibilities);

		for (int j = 0; j < parentIntersectionTmp.size(); ++j) {
			if(parentIntersectionTmp[j] == 1){
				cnode = currCGRA->getCGRANode(j);
				if(cnode->getRoutingNode() != NULL){
					if(node->getRoutingPossibilities() < cnode->getRoutingNode()->getRoutingPossibilities()){
						cnode->getRoutingNode()->setRoutingPossibilities(cnode->getRoutingNode()->getRoutingPossibilities() - 1);
						result[cnode->getRoutingNode()].erase(std::remove(result[cnode->getRoutingNode()].begin(), result[cnode->getRoutingNode()].end(), cnode), result[cnode->getRoutingNode()].end());
						cnode->setRoutingNode(node);
					}
				}
				else{
					cnode->setRoutingNode(node);
					result[node].push_back(cnode);
				}
			}
		}


	}

	for (int i = 0; i < nodes.size(); ++i) {
		node = nodes[i];
		for (int j = 0; j < node->getAncestors().size(); ++j) {
			parents.push_back(findNode(node->getAncestors()[j]));

		}

//		if(parents.size() == 0){
//			for (int var = 0; var < max; ++var) {
//
//			}
//		}

		for (int j = 0; j < parents.size(); ++j) {
			parentConnectedPhyNodes.push_back(currCGRA->getPhyConMatNode(parents[j]->getMappedLoc()));
		}

		parentIntersection = getIntersection(parentConnectedPhyNodes);

		while (parentIntersection.size() == 0){
			parentConnectedPhyNodes.clear();
			phyConMat = currCGRA->getPhyConMat();
			phyConMat = selfMulConMat(phyConMat);

			for (int j = 0; j < parents.size(); ++j) {
				cnode = parents[j]->getMappedLoc();
				parentConnectedPhyNodes.push_back( phyConMat[currCGRA->getConMatIdx(cnode->getT(),cnode->getY(),cnode->getX())] );
			}
		}

		std::vector<CGRANode*> resultNodeArr;
		for (int j = 0; j < parentIntersection.size(); ++j) {
			if(parentIntersection[j] == 1){
				resultNodeArr.push_back(currCGRA->getCGRANode(j));
			}
		}
		result[node] = resultNodeArr;
	}

	return result;
}

bool DFG::MapMultiDestRec(
		std::map<dfgNode*,std::vector<std::pair<CGRANode*,int>> > *nodeDestMap,
		std::map<CGRANode*, std::vector<dfgNode*> >* destNodeMap,
		std::map<dfgNode*,std::vector< std::pair<CGRANode*,int> > >::iterator it,
		std::map<CGRANode*,std::vector<CGRANode*> > cgraEdges,
		int index) {

	std::map<dfgNode*,std::vector<std::pair<CGRANode*,int>> > localNodeDestMap = *nodeDestMap;
	std::map<CGRANode*, std::vector<dfgNode*> > localdestNodeMap = *destNodeMap;

	dfgNode* node;
	dfgNode* otherNode;
	dfgNode* parent;
	CGRANode* cnode;
	std::pair<CGRANode*,int> cnodePair;
	CGRANode* parentExt;
//	std::vector< std::pair<CGRANode*,int> > possibleDests;
	std::vector<std::pair<CGRANode*, CGRANode*> > paths;
	std::vector<std::pair<CGRANode*, CGRANode*> > pathsNotRouted;
	bool success = false;
	int idx = index;

//	CGRA localCGRA = *inCGRA;
	std::map<CGRANode*,std::vector<CGRANode*> > localCGRAEdges = cgraEdges;

	node = it->first;
//	possibleDests = it->second;

	if(node == NULL){
		errs() << "Node is NULL...\n";
	}else{
		errs() << "Node = \n";
		node->getNode()->dump();
	}

	errs() << "MapMultiDestRec : Procesing NodeIdx = " << node->getIdx() << " ,PossibleDests = " << it->second.size() << "\n";

	for (int i = 0; i < it->second.size(); ++i) {
		success = false;
		errs() << "Possible Dest = "
			   << "(" << it->second[i].first->getT() << ","
			   	   	  << it->second[i].first->getY() << ","
					  << it->second[i].first->getX() << ")\n";

		if(it->second[i].first->getmappedDFGNode() == NULL){
			errs() << "Possible Dest is NULL\n";
			cnode = it->second[i].first;
			cnodePair = it->second[i];
			for (int j = 0; j < node->getAncestors().size(); ++j) {
				parent = findNode(node->getAncestors()[j]);
//				parentExt = currCGRA->getCGRANode(cnode->getT(),parent->getMappedLoc()->getY(),parent->getMappedLoc()->getX());
				parentExt = parent->getMappedLoc();

				errs() << "Path = "
					   << "(" << parentExt->getT() << ","
					   	   	  << parentExt->getY() << ","
							  << parentExt->getX() << ") to ";

				errs() << "(" << cnode->getT() << ","
					   	   	  << cnode->getY() << ","
							  << cnode->getX() << ")\n";

				paths.push_back(std::make_pair(parentExt,cnode));
			}

			localCGRAEdges = cgraEdges;
			mappingOutFile << "nodeIdx=" << node->getIdx() << ", placed=(" << cnode->getT() << "," << cnode->getY() << "," << cnode->getX() << ")" << "\n";
			astar->Route(paths,localCGRAEdges,&pathsNotRouted);
			paths.clear();
			if(!pathsNotRouted.empty()){
				errs() << "all paths are not routed.\n";
				pathsNotRouted.clear();
				continue;
			}
			pathsNotRouted.clear();

			for (int j = 0; j < localdestNodeMap[cnode].size(); ++j) {
				otherNode = localdestNodeMap[cnode][j];
				localNodeDestMap[otherNode].erase(std::remove(localNodeDestMap[otherNode].begin(), localNodeDestMap[otherNode].end(), cnodePair), localNodeDestMap[otherNode].end());
			}

//			it->second.clear();
//			it->second.push_back(cnodePair);
			localdestNodeMap[cnode].clear();

			errs() << "Placed = " << "(" << cnode->getT() << "," << cnode->getY() << "," << cnode->getX() << ")\n";
			node->setMappedLoc(cnode);
			cnode->setMappedDFGNode(node);
			node->setMappedRealTime(cnodePair.second);

			std::map<dfgNode*,std::vector< std::pair<CGRANode*,int> > >::iterator itlocal;
			itlocal = it;
			itlocal++;
//			it++;

			idx++;
			if(idx < nodeDestMap->size()){
				success = MapMultiDestRec(
						&localNodeDestMap,
						&localdestNodeMap,
						itlocal,
						localCGRAEdges,
						idx);
			}
			else{
				errs() << "nodeDestMap end reached..\n";
				*nodeDestMap = localNodeDestMap;
				*destNodeMap = localdestNodeMap;
				currCGRA->setCGRAEdges(localCGRAEdges);
				success = true;
			}

			if(success){
				it->second.clear();
				it->second.push_back(cnodePair);
				break;
			}
			else{
				errs() << "MapMultiDestRec : fails next possible destination\n";
				node->setMappedLoc(NULL);
				cnode->setMappedDFGNode(NULL);
			}
		}
	}

	if(success){

	}
	return success;
}


//bool DFG::MapMultiDest(std::map<dfgNode*, std::vector<CGRANode*> > *nodeDestMap,std::map<CGRANode*, std::vector<dfgNode*> > *destNodeMap) {
//	std::map<dfgNode*, std::vector<CGRANode*> > localNodeDestMap = *nodeDestMap;
//	std::map<CGRANode*, std::vector<dfgNode*> > localdestNodeMap = *destNodeMap;
//
//	dfgNode* node;
//	dfgNode* otherNode;
//	dfgNode* parent;
//	CGRANode* cnode;
//	CGRANode* parentExt;
//	std::vector<CGRANode*> possibleDests;
//	std::vector<std::pair<CGRANode*, CGRANode*> > paths;
//	bool success = false;
//
//	std::map<dfgNode*, std::vector<CGRANode*> > ::iterator it;
////	for (it = localNodeDestMap.begin(); it != localNodeDestMap.end(); it++){
//	it = localNodeDestMap.begin();
//
//
////	}
//
//	*nodeDestMap = localNodeDestMap;
//	*destNodeMap = localdestNodeMap;
//	return true;
//}



bool DFG::MapASAPLevel(int MII, int XDim, int YDim) {
	//TODO : Add recurrence constrained MII
	currCGRA = new CGRA(MII,XDim,YDim);

	errs() << "STARTING MAPASAP with MII = " << MII << "with maxASAPLevel = " << maxASAPLevel << "\n";

//	std::map<dfgNode*,std::vector<CGRANode*> > nodeDestMap;
	std::map<dfgNode*,std::vector< std::pair<CGRANode*,int> > > nodeDestMap;
	std::map<CGRANode*,std::vector<dfgNode*> > destNodeMap;
	std::map<dfgNode*, std::map<CGRANode*,int>> nodeDestCostMap;

	std::vector<dfgNode*> currLevelNodes;
	dfgNode* node;
	dfgNode* parent;
	CGRANode* parentExt;

	for (int level = 0; level < maxASAPLevel; ++level) {
		currLevelNodes.clear();
		nodeDestMap.clear();
		destNodeMap.clear();
		nodeDestCostMap.clear();

		errs() << "level = " << level << "\n";

		for (int j = 0; j < NodeList.size(); ++j) {
			node = &NodeList[j];

			if(node->getASAPnumber() == level){
				currLevelNodes.push_back(node);
			}
		}

		errs() << "numOfNodes = " << currLevelNodes.size() << "\n";

		for (int i = 0; i < currLevelNodes.size(); ++i) {
				node = currLevelNodes[i];
				int ll = 0;
				int el = INT32_MAX;
				for (int j = 0; j < node->getAncestors().size(); ++j) {
					parent = findNode(node->getAncestors()[j]);


					//every parent should be mapped
					assert(parent->getMappedLoc() != NULL);

					if(parent->getmappedRealTime() > ll){
						assert( parent->getmappedRealTime()%MII == parent->getMappedLoc()->getT() );
//						ll = parent->getMappedLoc()->getT();
						ll = parent->getmappedRealTime();
					}

					if(parent->getmappedRealTime() < el){
						assert( parent->getmappedRealTime()%MII == parent->getMappedLoc()->getT() );
						el = parent->getMappedLoc()->getT();
//						el = parent->getmappedRealTime();
					}

				}

//				if(el > ll){
					el = ll;
//				}

//				while((ll)%MII != el){
					for (int var = 0; var < MII; ++var) {
						for (int y = 0; y < YDim; ++y) {
							for (int x = 0; x < XDim; ++x) {
								if(currCGRA->getCGRANode((ll+1)%MII,y,x)->getmappedDFGNode() == NULL){
									nodeDestMap[node].push_back(std::make_pair(currCGRA->getCGRANode((ll+1)%MII,y,x),(ll+1)));
									destNodeMap[currCGRA->getCGRANode((ll+1)%MII,y,x)].push_back(node);
								}
							}
						}
						ll = (ll+1);
					}
//				}
				errs() << "MapASAPLevel:: nodeIdx=" << node->getIdx() << " ,Possible Dests = " << nodeDestMap[node].size() << "\n";
			}

			errs() << "MapASAPLevel:: Finding dests are done!\n";

			bool changed = false;
			std::vector<std::pair<CGRANode*, CGRANode*> > paths;
			CGRANode* nodeBeingMapped;
			std::pair<CGRANode*,int> nodeBeingMappedpair;
			dfgNode* otherNode;
			std::vector<dfgNode*> singleDests;

			//Handle single destination nodes
			do{
				changed = false;
				for (int i = 0; i < currLevelNodes.size(); ++i) {
					node = currLevelNodes[i];
					if(nodeDestMap.find(node) == nodeDestMap.end()){
						errs() << "No dests mapping fails\n";
						return false; //mapping fails if no placement found
					}
					if(nodeDestMap[node].size() == 1){
						nodeBeingMapped = nodeDestMap[node][0].first;
						nodeBeingMappedpair = nodeDestMap[node][0];

						if(destNodeMap[nodeBeingMapped].empty()){
							break; //already handled
						}

						singleDests.push_back(node);

						for (int j = 0; j < node->getAncestors().size(); ++j) {
							parent = findNode(node->getAncestors()[j]);
//							parentExt = currCGRA->getCGRANode(nodeBeingMapped->getT(),parent->getMappedLoc()->getY(),parent->getMappedLoc()->getX());
							parentExt = parent->getMappedLoc();
							paths.push_back(std::make_pair(parentExt,nodeBeingMapped));
						}

						for (int j = 0; j < destNodeMap[nodeBeingMapped].size(); ++j) {
							otherNode = destNodeMap[nodeBeingMapped][j];
							nodeDestMap[otherNode].erase(std::remove(nodeDestMap[otherNode].begin(), nodeDestMap[otherNode].end(), nodeBeingMappedpair), nodeDestMap[otherNode].end());
						}

						destNodeMap[nodeBeingMapped].clear();
		//				destNodeMap[nodeBeingMapped].push_back(node);

						changed = true;
					}
				}
			}while(changed);

			std::vector<std::pair<CGRANode*, CGRANode*> > pathsNotRouted;
			astar->Route(paths,currCGRA->getCGRAEdges(),&pathsNotRouted);

			if(!pathsNotRouted.empty()){
				errs() << "mapping fails :: single destinations could not be routed\n";
				return false; //mapping fails if single destinations could not be routed
			}

			for (int i = 0; i < singleDests.size(); ++i) {
				singleDests[i]->setMappedLoc(nodeDestMap[singleDests[i]][0].first);
				nodeDestMap[singleDests[i]][0].first->setMappedDFGNode(singleDests[i]);
			}

			//Multiple Desination Nodes
			if(!MapMultiDestRec(&nodeDestMap,&destNodeMap,nodeDestMap.begin(),currCGRA->getCGRAEdges(),0)){
				return false;
			}
//			return true;
	}
	return true;
}

void DFG::MapCGRAsa(int XDim, int YDim) {
		int MII = ceil((float)NodeList.size()/((float)XDim*(float)YDim));
//		int MII = 11;
	astar = new AStar(&mappingOutFile,MII);
	mappingOutFile.open("Mapping.log");

	while(1){
		if(MapASAPLevel(MII,XDim,YDim)){
			break;
		}
		errs() << "MapCGRAsa :: Mapping failed with MII = " << MII << "\n";
		MII++;
	}
	mappingOutFile.close();
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

					if(nodeListSequencer >= 0){
						backTrack(nodeListSequencer);
					}

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
					candidates2 = getConnectedCGRANodes(temp);
//					candidates2 = temp->getMappedLoc()->getConnectedNodes();
//					eraseAlreadyMappedNodes(&candidates2);

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

std::vector<ConnectedCGRANode> DFG::ExpandCandidatesAddingRoutingNodes(
		std::vector<std::pair<Instruction*, int> >* candidateNumbers) {

	dfgNode* temp;
	std::vector<ConnectedCGRANode> candidates2;
	std::vector<std::pair<Instruction*, int> > candidateNumbersTemp;
	std::sort(candidateNumbers->begin(),candidateNumbers->end(),ValueComparer());

//	int conMat[currCGRA->getMII()*currCGRA->getYdim()*currCGRA->getXdim()][currCGRA->getMII()*currCGRA->getYdim()*currCGRA->getXdim()];
	std::vector<std::vector<int> > conMat;


	for (int i = 0; i < currCGRA->getMII()*currCGRA->getYdim()*currCGRA->getXdim(); ++i) {
		std::vector<int> temp;
		for (int j = 0; j < currCGRA->getMII()*currCGRA->getYdim()*currCGRA->getXdim(); ++j) {
			temp.push_back(0);
		}
		conMat.push_back(temp);
	}



	std::vector<ConnectedCGRANode> phyCand;

	for (int t = 0; t < currCGRA->getMII(); ++t) {
		for (int y = 0; y < currCGRA->getYdim(); ++y) {
			for (int x = 0; x < currCGRA->getXdim(); ++x) {
				conMat[getConMatIdx(t,y,x)][getConMatIdx(t,y,x)] = 1;

				phyCand = currCGRA->getCGRANode(t,y,x)->getConnectedNodes();
				for (int i = 0; i < phyCand.size(); ++i) {
					if(phyCand[i].node->getmappedDFGNode() == NULL){
						conMat[getConMatIdx(t,y,x)][getConMatIdx(phyCand[i].node->getT(),phyCand[i].node->getY(),phyCand[i].node->getX())] = 1;
					}
				}
			}
		}
	}


	for (int i = 0; i < candidateNumbers->size(); ++i) {
					temp = findNode((*candidateNumbers)[i].first);

					candidates2 = getConnectedCGRANodes(temp);
					std::sort(candidates2.begin(),candidates2.end(),CostComparer());

					for (int j = 0; j < candidates2.size(); ++j) {




	//					errs() << "candidates2 = (" << candidates2[j].node->getT() << ","
	//							   	   	   	   	    << candidates2[j].node->getY() << ","
	//												<< candidates2[j].node->getX() << ")\n";
					}
	}


}

std::vector<ConnectedCGRANode> DFG::getConnectedCGRANodes(dfgNode* node) {
	assert(node->getMappedLoc() != NULL);

	std::vector<ConnectedCGRANode> candidates;
	std::vector<ConnectedCGRANode> candidates2;
	candidates = node->getMappedLoc()->getConnectedNodes();
	bool found = false;

	int routingCost;
	for (int i = 0; i < node->getRoutingLocs()->size(); ++i) {
		found = false;
		routingCost = (*node->getRoutingLocs())[i]->getT() - node->getMappedLoc()->getT();
		if (routingCost < 0){
			routingCost = routingCost + currCGRA->getMII();
		}

		candidates2 = (*node->getRoutingLocs())[i]->getConnectedNodes();

		for (int j = 0; j < candidates2.size(); ++j) {
			candidates2[j].cost = candidates2[j].cost + routingCost;

			found = false;
			for (int k = 0; k < candidates.size(); ++k) {
				if(candidates2[j].node == candidates[k].node){
					found = true;
				}
			}

			if(!found){
				candidates.push_back(candidates2[j]);
			}

		}

	}
	eraseAlreadyMappedNodes(&candidates);
	return candidates;
}

int DFG::getConMatIdx(int t, int y, int x) {
	return t*currCGRA->getYdim()*currCGRA->getXdim() + y*currCGRA->getXdim() + x;
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

		candidates = getConnectedCGRANodes(temp);
//		candidates = temp->getMappedLoc()->getConnectedNodes();

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
				candidates2 = getConnectedCGRANodes(temp);
//				candidates2 = temp->getMappedLoc()->getConnectedNodes();
//				eraseAlreadyMappedNodes(&candidates2);
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

