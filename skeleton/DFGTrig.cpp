#include "DFGTrig.h"
#include "llvm/Analysis/CFG.h"
#include <algorithm>

void DFGTrig::connectBB(){

	SmallVector<std::pair<const BasicBlock *, const BasicBlock *>,8> BackedgeBBs;
	dfgNode firstNode = *NodeList[0];
	FindFunctionBackedges(*(firstNode.getNode()->getFunction()),BackedgeBBs);

	for(dfgNode* node : NodeList){
		if(node->getNode()==NULL) continue;
		if(BranchInst* BRI = dyn_cast<BranchInst>(node->getNode())){
			for(int i=0; i<BRI->getNumSuccessors(); i++){

				const BasicBlock* currBB = BRI->getParent();
				const BasicBlock* succBB = BRI->getSuccessor(i);
				std::pair<const BasicBlock *, const BasicBlock *> BBCouple = std::make_pair(currBB,succBB);

				bool isBackEdge = false;
				if(std::find(BackedgeBBs.begin(),BackedgeBBs.end(),BBCouple) != BackedgeBBs.end()){
					isBackEdge = true;
				}

				bool isConditional = BRI->isConditional();
				bool conditionVal = true;
				if(isConditional){
					if(i == 1){
						conditionVal = false;
					}
				}

				std::vector<dfgNode*> leafs = getLeafs(BRI->getSuccessor(i)); //this doesnt include phinodes

				if(node->getAncestors().size()!=1){
					BRI->dump();
					outs() << "BR node ancestor size = " << node->getAncestors().size() << "\n";
				}
//				assert(node->getAncestors().size()==1); //should be a unique parent;
//				dfgNode* BRParent = node->getAncestors()[0];

				for(dfgNode* BRParent : node->getAncestors()){

					if(!isConditional){
						const BasicBlock* BRParentBB = BRParent->BB;
						const BranchInst* BRParentBRI = cast<BranchInst>(BRParentBB->getTerminator());
//						assert(BRParentBRI->isConditional());
//						if(BRParentBRI->getSuccessor(0)==currBB){
//							conditionVal=true;
//						}
//						else if(BRParentBRI->getSuccessor(1)==currBB){
//							conditionVal=false;
//						}
//						else{
//							BRParentBRI->dump();
//							outs() << BRParentBRI->getSuccessor(0)->getName() << "\n";
//							outs() << BRParentBRI->getSuccessor(1)->getName() << "\n";
//							outs() << currBB->getName() << "\n";
//							outs() << BRParent->getIdx() << "\n";
//							assert(false);
//						}
						std::set<const BasicBlock*> searchSoFar;
						assert(checkBRPath(BRParentBRI,currBB,conditionVal,searchSoFar));
						isConditional=true; //this is always true. coz there is no backtoback unconditional branches.
					}

					outs() << "BR : ";
					BRI->dump();
					outs() << " and BRParent :" << BRParent->getIdx() << "\n";


					outs() << "leafs from basicblock=" << currBB->getName() << " for basicblock=" << succBB->getName().str() << "\n";
					for(dfgNode* succNode : leafs){
						outs() << succNode->getIdx() << ",";
					}
					outs() << "\n";

					for(dfgNode* succNode : leafs){
						BRParent->addChildNode(succNode,EDGE_TYPE_DATA,isBackEdge,isConditional,conditionVal);
						succNode->addAncestorNode(BRParent,EDGE_TYPE_DATA,isBackEdge,isConditional,conditionVal);
					}

				}
			}
		}
	}

}

int DFGTrig::handlePHINodes(std::set<BasicBlock*> LoopBB) {

	std::vector<dfgNode*> phiNodes;
	for(dfgNode* node : NodeList){
		if(node->getNode()==NULL)continue;
		if(PHINode* PHI = dyn_cast<PHINode>(node->getNode())){
			phiNodes.push_back(node);
		}
	}

	for(dfgNode* node : phiNodes){
		if(node->getNode()==NULL)continue;
		if(PHINode* PHI = dyn_cast<PHINode>(node->getNode())){
			std::vector<dfgNode*> mergeNodes;
			std::map<dfgNode*,std::pair<dfgNode*,bool>> mergeNodeControl;

			if(!node->getAncestors().empty()){
				PHI->dump();
				assert(false);
			}

			for (int i = 0; i < PHI->getNumIncomingValues(); ++i) {
				BasicBlock* bb = PHI->getIncomingBlock(i);
				Value* V = PHI->getIncomingValue(i);
				bool incomingCTRLVal;

				dfgNode* previousCTRLNode;
				if(LoopBB.find(bb) == LoopBB.end()){
					//Not found
					std::pair<BasicBlock*,BasicBlock*> bbPair = std::make_pair(bb,(BasicBlock*)node->BB);
					assert(loopentryBB.find(bbPair)!=loopentryBB.end());
					previousCTRLNode = getStartNode(bb,node);
					incomingCTRLVal=true; // all start nodes are assumed to be true
				}
				else{ // within the loop
					BranchInst* BRI = cast<BranchInst>(bb->getTerminator());
					previousCTRLNode = findNode(BRI);
					bool isConditional = BRI->isConditional();

					assert(previousCTRLNode->getAncestors().size()==1);
					previousCTRLNode = previousCTRLNode->getAncestors()[0];
					const BasicBlock* currBB = BRI->getParent();

					if(!isConditional){
						const BasicBlock* BRParentBB = previousCTRLNode->BB;
						const BranchInst* BRParentBRI = cast<BranchInst>(BRParentBB->getTerminator());
						assert(BRParentBRI->isConditional());
//						if(BRParentBRI->getSuccessor(0)==currBB){
//							incomingCTRLVal=true;
//						}
//						else if(BRParentBRI->getSuccessor(1)==currBB){
//							incomingCTRLVal=false;
//						}
//						else{
//							BRParentBRI->dump();
//							outs() << BRParentBRI->getSuccessor(0)->getName() << "\n";
//							outs() << BRParentBRI->getSuccessor(1)->getName() << "\n";
//							outs() << currBB->getName() << "\n";
//							outs() << previousCTRLNode->getIdx() << "\n";
//							assert(false);
//						}
						std::set<const BasicBlock*> searchSoFar;
						assert(checkBRPath(BRParentBRI,currBB,incomingCTRLVal,searchSoFar));
						isConditional=true; //this is always true. coz there is no backtoback unconditional branches.
					}
					else{
						if(BRI->getSuccessor(0) == node->BB){
							incomingCTRLVal = true;
						}
						else{
							incomingCTRLVal = false;
							assert(BRI->getSuccessor(1) == node->BB);
						}
					}
				}
				assert(previousCTRLNode!=NULL);

				dfgNode* mergeNode;
				if(ConstantInt* CI = dyn_cast<ConstantInt>(V)){
					int constant = CI->getSExtValue();
					mergeNode = insertMergeNode(node,previousCTRLNode,incomingCTRLVal,constant);

				}
				else if(ConstantFP* FP = dyn_cast<ConstantFP>(V)){
					int constant = (int)FP->getValueAPF().convertToFloat();
					mergeNode = insertMergeNode(node,previousCTRLNode,incomingCTRLVal,constant);

				}
				else if(UndefValue *UND = dyn_cast<UndefValue>(V)){
					int constant = 0;
					mergeNode = insertMergeNode(node,previousCTRLNode,incomingCTRLVal,constant);
				}
				else if(sizeArrMap.find(V->getName().str())!=sizeArrMap.end()){
					int constant = sizeArrMap[V->getName().str()];
					mergeNode = insertMergeNode(node,previousCTRLNode,incomingCTRLVal,constant);
				}
				else{
					outs() << "DEBUG........... begin\n";
					PHI->dump();
					V->dump();

					Instruction* ins = cast<Instruction>(V);
					dfgNode* phiParent = findNode(ins);
					bool isPhiParentOutLoopLoad=false;
					if(phiParent == NULL){ //not found
						isPhiParentOutLoopLoad=true;
						if(OutLoopNodeMap.find(ins)!=OutLoopNodeMap.end()){
							phiParent=OutLoopNodeMap[ins];
						}
						else{
							phiParent=addLoadParent(ins,node);
							bool isBackEdge = checkBackEdge(previousCTRLNode,phiParent);
							previousCTRLNode->addChildNode(phiParent,EDGE_TYPE_DATA,isBackEdge,true,incomingCTRLVal);
							phiParent->addAncestorNode(previousCTRLNode,EDGE_TYPE_DATA,isBackEdge,true,incomingCTRLVal);
						}
					}
					assert(phiParent!=NULL);


					bool isPhiParentSuccCTRL=false;
					for (int succ = 0; succ < previousCTRLNode->BB->getTerminator()->getNumSuccessors(); ++succ) {
						if(previousCTRLNode->BB->getTerminator()->getSuccessor(succ)==phiParent->BB){
							isPhiParentSuccCTRL=true;
							break;
						}
					}

//					if(previousCTRLNode->BB != phiParent->BB){
					if(!isPhiParentSuccCTRL){
						mergeNode = insertMergeNode(node,previousCTRLNode,incomingCTRLVal,phiParent);
					}
					else{
						outs() << "mergeNode and control are from the same BB\n";
						mergeNode = phiParent;
//						if(!isPhiParentOutLoopLoad){
//							mergeNodeControl[mergeNode]=std::make_pair(previousCTRLNode,incomingCTRLVal);
//						}
					}
				}
				mergeNodes.push_back(mergeNode);
			}
			outs() << "DEBUG........... end\n";
			bool recursivePhiNodes=false;

			for(dfgNode* child : node->getChildren()){
				for(dfgNode* mergeNode : mergeNodes){
					if(child == mergeNode){
						outs() << "recursivePhiNodes found!!!!\n";
						recursivePhiNodes=true;
						break;
					}

					if(checkPHILoop(mergeNode,node)){
						recursivePhiNodes=true;
						break;
					}
				}
				if(recursivePhiNodes) break;
			}

			assert(node->getAncestors().empty());

			if(recursivePhiNodes){
				for(dfgNode* mergeNode : mergeNodes){
					bool isBackEdge = checkBackEdge(mergeNode,node);
					node->addAncestorNode(mergeNode,EDGE_TYPE_DATA,isBackEdge);
					mergeNode->addChildNode(node,EDGE_TYPE_DATA,isBackEdge);
				}
			}
			else{
				std::vector<dfgNode*> phiChildren = node->getChildren();
				for(dfgNode* child : phiChildren){

					node->removeChild(child);
					child->removeAncestor(node);

					outs() << "PHIChild : " << child->getIdx() << " : ";

					for(dfgNode* mergeNode : mergeNodes){
						outs() << mergeNode->getIdx() << ",";

						bool isBackEdge = checkBackEdge(mergeNode,child);
						child->addAncestorNode(mergeNode,EDGE_TYPE_DATA,isBackEdge);
						mergeNode->addChildNode(child,EDGE_TYPE_DATA,isBackEdge);

						if(mergeNodeControl.find(mergeNode) != mergeNodeControl.end()){
							dfgNode* control = mergeNodeControl[mergeNode].first;
							bool controlVal = mergeNodeControl[mergeNode].second;

							child->addAncestorNode(control,EDGE_TYPE_DATA,isBackEdge,true,controlVal);
							control->addChildNode(child,EDGE_TYPE_DATA,isBackEdge,true,controlVal);
						}
					}
					outs() << "\n";
				}
			}



//			if(!node->getChildren().empty()){
//				PHI->dump();
//				assert(false);
//			}
		}
	}


	return 0;
}

dfgNode* DFGTrig::getStartNode(BasicBlock* BB, dfgNode* PHINode) {
	if(startNodes.find(BB)!=startNodes.end()){
		return startNodes[BB];
	}
	else{
		dfgNode* tempBR = new dfgNode(this);
		tempBR->setNameType("LOOPSTART");
//		tempBR->BB = PHINode->BB;
		tempBR->BB = BB;
		tempBR->setIdx(NodeList.size());
		NodeList.push_back(tempBR);

		int nextStartNodesIdx = startNodes.size();
		startNodes[BB]=tempBR;
		startNodeConsts[BB]=nextStartNodesIdx;

		tempBR->setConstantVal(nextStartNodesIdx);
		return tempBR;
	}
}

dfgNode* DFGTrig::insertMergeNode(dfgNode* PHINode, dfgNode* ctrl, bool controlVal, dfgNode* data) {
	dfgNode* temp = new dfgNode(this);

	SmallVector<std::pair<const BasicBlock *, const BasicBlock *>,8> BackedgeBBs;
	FindFunctionBackedges(*(PHINode->getNode()->getFunction()),BackedgeBBs);
	const BasicBlock* ctrlBB = ctrl->BB;
	const BasicBlock* dataBB = data->BB;
	const BasicBlock* phiBB  = PHINode->BB;

	bool isCTRL2PHIBackEdge=false;
	bool isDATA2PHIBackEdge=false;

//	std::pair<const BasicBlock *, const BasicBlock *> bbCuple;
//	bbCuple = std::make_pair(ctrlBB,phiBB);
//	if(std::find(BackedgeBBs.begin(),BackedgeBBs.end(),bbCuple)!=BackedgeBBs.end()){
//		isCTRL2PHIBackEdge=true;
//	}
	isCTRL2PHIBackEdge = checkBackEdge(ctrl,PHINode);

//	bbCuple = std::make_pair(dataBB,phiBB);
//	if(std::find(BackedgeBBs.begin(),BackedgeBBs.end(),bbCuple)!=BackedgeBBs.end()){
//		isDATA2PHIBackEdge=true;
//	}
//	isDATA2PHIBackEdge = checkBackEdge(data,PHINode);
	isDATA2PHIBackEdge = isCTRL2PHIBackEdge;

	temp->setNameType("CMERGE");
	temp->BB = PHINode->BB;
	temp->setIdx(NodeList.size());
	NodeList.push_back(temp);

	temp->addAncestorNode(ctrl,EDGE_TYPE_DATA,isCTRL2PHIBackEdge,true,controlVal);
	ctrl->addChildNode(temp,EDGE_TYPE_DATA,isCTRL2PHIBackEdge,true,controlVal);

	temp->addAncestorNode(data,EDGE_TYPE_DATA,isDATA2PHIBackEdge,false);
	data->addChildNode(temp,EDGE_TYPE_DATA,isDATA2PHIBackEdge,false);
	return temp;
}

dfgNode* DFGTrig::insertMergeNode(dfgNode* PHINode, dfgNode* ctrl,
		bool controlVal, int val) {

	dfgNode* temp = new dfgNode(this);

	SmallVector<std::pair<const BasicBlock *, const BasicBlock *>,8> BackedgeBBs;
	FindFunctionBackedges(*(PHINode->getNode()->getFunction()),BackedgeBBs);
	const BasicBlock* ctrlBB = ctrl->BB;
	const BasicBlock* phiBB  = PHINode->BB;

	bool isCTRL2PHIBackEdge=false;

//	std::pair<const BasicBlock *, const BasicBlock *> bbCuple;
//	bbCuple = std::make_pair(ctrlBB,phiBB);
//	if(std::find(BackedgeBBs.begin(),BackedgeBBs.end(),bbCuple)!=BackedgeBBs.end()){
//		isCTRL2PHIBackEdge=true;
//	}
	isCTRL2PHIBackEdge = checkBackEdge(ctrl,PHINode);

	temp->setNameType("CMERGE");
	temp->BB = PHINode->BB;
	temp->setIdx(NodeList.size());
	NodeList.push_back(temp);

	temp->addAncestorNode(ctrl,EDGE_TYPE_DATA,isCTRL2PHIBackEdge,true,controlVal);
	ctrl->addChildNode(temp,EDGE_TYPE_DATA,isCTRL2PHIBackEdge,true,controlVal);
	temp->setConstantVal(val);
	return temp;
}

void DFGTrig::removeDisconnetedNodes() {
	std::vector<dfgNode*> disconnectedNodes;
	for(dfgNode* node : NodeList){
		if(node->getChildren().size() == 0 && node->getAncestors().size() == 0){
			disconnectedNodes.push_back(node);
		}

		if(node->getNode()!=NULL){
			if(BranchInst* BRI = dyn_cast<BranchInst>(node->getNode())){
				assert(node->getChildren().empty());
				for(dfgNode* parent : node->getAncestors()){
					parent->removeChild(node);
					node->removeAncestor(parent);
				}
				disconnectedNodes.push_back(node);
			}
		}

	}

	for(dfgNode* removeNode : disconnectedNodes){
		NodeList.erase(std::remove(NodeList.begin(),NodeList.end(),removeNode), NodeList.end());
	}
	outs() << "POST REMOVAL NODE COUNT = " << NodeList.size() << "\n";
}

int DFGTrig::handleSELECTNodes() {
	for(dfgNode* node : NodeList){
		if(node->getNode()==NULL) continue;
		if(SelectInst* SLI = dyn_cast<SelectInst>(node->getNode())){
			bool isLeafIns=true;
			for(dfgNode* parent : node->getAncestors()){
				if(parent->BB == node->BB){
					isLeafIns=false;
					break;
				}
			}

			if(isLeafIns){
				bool found=false;
				for(dfgNode* parent : node->getAncestors()){
					if(node->ancestorConditionaMap.find(parent)!=node->ancestorConditionaMap.end()){
						// the condition parent
						Instruction* selCondIns = cast<Instruction>(SLI->getCondition());
						dfgNode* selCond = findNode(selCondIns);
						assert(selCond != NULL);
						combineConditionAND(parent,selCond,node);
					}
				}
				assert(found);
			}
			else{
				/// if its not a leaf instruction, I dont have to do anything

			}
		}
	}


}

dfgNode* DFGTrig::combineConditionAND(dfgNode* brcond, dfgNode* selcond, dfgNode* child) {
	dfgNode* temp = new dfgNode(this);
	temp->setNameType("CMERGECOND");
	temp->setIdx(getNodesPtr()->size());
	temp->BB = child->BB;
	getNodesPtr()->push_back(temp);

	selcond->removeChild(child);
	child->removeAncestor(selcond);

	selcond->addChildNode(temp);
	temp->addAncestorNode(selcond);

	assert(child->ancestorConditionaMap.find(brcond)!=child->ancestorConditionaMap.end());
	bool brCondVal = (child->ancestorConditionaMap[brcond] == TRUE);
	bool isBrBackEdge = checkBackEdge(brcond,child);
	brcond->removeChild(child);
	child->removeAncestor(brcond);

	brcond->addChildNode(temp,EDGE_TYPE_DATA,isBrBackEdge,true,brCondVal);
	temp->addAncestorNode(brcond,EDGE_TYPE_DATA,isBrBackEdge,true,brCondVal);

	return temp;
}

bool DFGTrig::checkBackEdge(dfgNode* src, dfgNode* dest) {
//	dfgNode* firstnode = NodeList[0];
//	SmallVector<std::pair<const BasicBlock *, const BasicBlock *>,8> BackedgeBBs;
//	FindFunctionBackedges(*(firstnode->getNode()->getFunction()),BackedgeBBs);
//	const BasicBlock* srcBB = src->BB;
//	const BasicBlock* destBB  = dest->BB;
//
//	std::pair<const BasicBlock *, const BasicBlock *> bbCuple;
//	bbCuple = std::make_pair(srcBB,destBB);
//	if(std::find(BackedgeBBs.begin(),BackedgeBBs.end(),bbCuple)!=BackedgeBBs.end()){
//		return true;
//	}
//	else{
//		return false;
//	}
	const BasicBlock* srcBB = src->BB;
	const BasicBlock* destBB = dest->BB;

	if(std::find(BBSuccBasicBlocks[srcBB].begin(), BBSuccBasicBlocks[srcBB].end(), destBB) != BBSuccBasicBlocks[srcBB].end()){
		return false; //not a backedge
	}
	else{
		return true;
	}

}

void DFGTrig::generateTrigDFGDOT() {

	connectBB();
	handlePHINodes(this->loopBB);
//	insertshiftGEPs();
	addMaskLowBitInstructions();
	removeDisconnetedNodes();
	printDOT(this->name + "_TrigDFG.dot");
}

bool DFGTrig::checkPHILoop(dfgNode* node1, dfgNode* node2) {
	if(node1->getNode()==NULL) return false;
	if(node2->getNode()==NULL) return false;

	if(PHINode* phi1 = dyn_cast<PHINode>(node1->getNode())){
		if(PHINode* phi2 = dyn_cast<PHINode>(node2->getNode())){
			bool is2isParentOf1 = false;
			bool is1isParentOf2 = false;
			for (int i1 = 0; i1 < phi1->getNumIncomingValues(); ++i1) {
				if(phi1->getIncomingValue(i1)==phi2){
					is2isParentOf1=true;
					break;
				}
			}
			for (int i2 = 0; i2 < phi2->getNumIncomingValues(); ++i2) {
				if(phi2->getIncomingValue(i2)==phi1){
					is1isParentOf2=true;
					break;
				}
			}
			if(is2isParentOf1 & is1isParentOf2) outs() << "PHI LOOP FOUND!!!!!!\n";
			return is2isParentOf1 & is1isParentOf2;
		}
		else{
			return false;
		}
	}
	else{
		return false;
	}
}

bool DFGTrig::checkBRPath(const BranchInst* BRParentBRI, const BasicBlock* currBB, bool& condVal,  std::set<const BasicBlock*>& searchedSoFar) {
	if(BRParentBRI->getSuccessor(0)==currBB){
		condVal=true;
		return true;
	}
	else if(BRParentBRI->getSuccessor(1)==currBB){
		condVal=false;
		return true;
	}
	else{
		searchedSoFar.insert(BRParentBRI->getParent());
		for (int succ = 0; succ < BRParentBRI->getNumSuccessors(); ++succ) {
			BasicBlock* succBB = BRParentBRI->getSuccessor(succ);
			if(searchedSoFar.find(succBB)==searchedSoFar.end()){
				BranchInst* succBRParentBRI = cast<BranchInst>(succBB->getTerminator());
				if(checkBRPath(succBRParentBRI,currBB,condVal,searchedSoFar)){
					return true;
				}
			}
		}

		BRParentBRI->dump();
		outs() << BRParentBRI->getSuccessor(0)->getName() << "\n";
		outs() << BRParentBRI->getSuccessor(1)->getName() << "\n";
		outs() << currBB->getName() << "\n";
		BRParentBRI->dump();
		return false;
	}

}

dfgNode* DFGTrig::addLoadParent(Instruction* ins, dfgNode* child) {
	dfgNode* temp;
	if(OutLoopNodeMap.find(ins) == OutLoopNodeMap.end()){
		temp = new dfgNode(this);
		temp->setNameType("OutLoopLOAD");
		temp->setIdx(getNodesPtr()->size());
		temp->BB = child->BB;
		getNodesPtr()->push_back(temp);
		OutLoopNodeMap[ins] = temp;
		OutLoopNodeMapReverse[temp]=ins;

		if(accumulatedBBs.find(ins->getParent())==accumulatedBBs.end()){
			temp->setTransferedByHost(true);
		}
	}
	else{
		temp = OutLoopNodeMap[ins];
	}

	if(GetElementPtrInst* GEP = dyn_cast<GetElementPtrInst>(ins)){
			temp->setTypeSizeBytes(4);
		}
	else if(AllocaInst* ALOCA = dyn_cast<AllocaInst>(ins)){
		errs() << "ALOCA\n";
		temp->setTypeSizeBytes(4);
	}
	else if(dyn_cast<PointerType>(ins->getType())){
		temp->setTypeSizeBytes(4);
	}
	else{

		if(dyn_cast<ArrayType>(ins->getType())){
			outs() << "A\n";
		}
		if(dyn_cast<StructType>(ins->getType())){
			outs() << "S\n";
		}
		if(dyn_cast<PointerType>(ins->getType())){
			outs() << "P\n";
		}

		//TODO:DAC18
//		temp->setTypeSizeBytes((ins->getType()->getIntegerBitWidth()+7)/8);
		temp->setTypeSizeBytes((ins->getType()->getPrimitiveSizeInBits()+7)/8);
	}

	return temp;
}

void DFGTrig::printDOT(std::string fileName) {
	std::ofstream ofs;
	ofs.open(fileName.c_str());

	//Write the initial info
	ofs << "digraph Region_18 {\n";
	ofs << "\tgraph [ nslimit = \"1000.0\",\n";
	ofs <<	"\torientation = landscape,\n";
	ofs <<	"\t\tcenter = true,\n";
	ofs <<	"\tpage = \"8.5,11\",\n";
	ofs << "\tcompound=true,\n";
	ofs <<	"\tsize = \"10,7.5\" ] ;" << std::endl;

	assert(NodeList.size() != 0);

	std::map<const BasicBlock*,std::set<dfgNode*>> BBNodeList;

	for(dfgNode* node : NodeList){
		BBNodeList[node->BB].insert(node);
	}

	int cluster_idx=0;
	for(std::pair<const BasicBlock*,std::set<dfgNode*>> BBNodeSet : BBNodeList){
		const BasicBlock* BB = BBNodeSet.first;

//		  subgraph cluster_0 {
//		        node [style=filled];
//		        "Item 1" "Item 2";
//		        label = "Container A";
//		        color=blue;
//		    }

		ofs << "subgraph cluster_" << cluster_idx << " {\n";
//		ofs << "node [style=filled]";
		for(dfgNode* node : BBNodeSet.second){
			ofs << "\""; //BEGIN NODE NAME
			ofs << "Op_" << node->getIdx();
			ofs << "\" [ fontname = \"Helvetica\" shape = box, label = \" ";
			if(node->getNode() != NULL){
				ofs << node->getNode()->getOpcodeName();
				ofs << " " << node->getNode()->getName().str() << " ";

				if(node->hasConstantVal()){
					ofs << " C=" << "0x" << std::hex << node->getConstantVal() << std::dec;
				}

				if(node->isGEP()){
					ofs << " C=" << "0x" << std::hex << node->getGEPbaseAddr() << std::dec;
				}

				ofs << " BB" << node->getNode()->getParent()->getName().str();
			}
			else{
				ofs << node->getNameType();

				if(node->getNameType() == "OuterLoopLOAD"){
					ofs << " " << OutLoopNodeMapReverse[node]->getName().str() << " ";
				}

				if(node->isOutLoop()){
					ofs << " C=" << "0x" << node->getoutloopAddr() << std::dec;
				}

				if(node->hasConstantVal()){
					ofs << " C=" << "0x" << node->getConstantVal() << std::dec;
				}
			}

			if(node->getFinalIns() != NOP){
				ofs << " HyIns=" << HyCUBEInsStrings[node->getFinalIns()];
			}
			ofs << ",\n";
			ofs << node->getIdx() << ", ASAP=" << node->getASAPnumber();
			ofs << ", ALAP=" << node->getALAPnumber();

			ofs << "\"]\n"; //END NODE NAME
		}

		ofs << "label = " << "\"" << BB->getName().str() << "\"" << ";\n";
		ofs << "color = purple;\n";
		ofs << "}\n";
		cluster_idx++;
	}


	//The EDGES

	for(dfgNode* node : NodeList){

		for(dfgNode* child : node->getChildren()){
			bool isCondtional=false;
			bool condition=true;

			isCondtional= node->childConditionalMap[child] != UNCOND;
			condition = (node->childConditionalMap[child] == TRUE);

			bool isBackEdge = node->childBackEdgeMap[child];


			ofs << "\"" << "Op_" << node->getIdx() << "\"";
			ofs << " -> ";
			ofs << "\"" << "Op_" << child->getIdx() << "\"";
			ofs << " [style = ";

			if(isBackEdge){
				ofs << "dashed";
			}
			else{
				ofs << "bold";
			}
			ofs << ", ";

			ofs << "color = ";
			if(isCondtional){
				if(condition==true){
					ofs << "blue";
				}
				else{
					ofs << "red";
				}
			}
			else{
				ofs << "black";
			}

//			ofs << ", headport=n, tailport=s";
			ofs << "];\n";
		}

		for(dfgNode* recChild : node->getRecChildren()){
			ofs << "\"" << "Op_" << node->getIdx() << "\"";
			ofs << " -> ";
			ofs << "\"" << "Op_" << recChild->getIdx() << "\"";
			ofs << "[style = bold, color = green];\n";
		}
	}

	ofs << "}" << std::endl;
	ofs.close();
}
