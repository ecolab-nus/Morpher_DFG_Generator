#include "DFGPartPred.h"
#include "llvm/Analysis/CFG.h"
#include <algorithm>
#include <queue>
#include <map>
#include <set>
#include <vector>

void DFGPartPred::connectBB() {

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

				std::vector<dfgNode*> stores = getStoreInstructions(BRI->getSuccessor(i)); //this doesnt include phinodes

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
					for(dfgNode* succNode : stores){
						outs() << succNode->getIdx() << ",";
					}
					outs() << "\n";

					for(dfgNode* succNode : stores){
						isBackEdge = checkBackEdge(node,succNode);
						BRParent->addChildNode(succNode,EDGE_TYPE_DATA,isBackEdge,isConditional,conditionVal);
						succNode->addAncestorNode(BRParent,EDGE_TYPE_DATA,isBackEdge,isConditional,conditionVal);
					}

				}
			}
		}
	}


}

std::vector<dfgNode*> DFGPartPred::getStoreInstructions(BasicBlock* BB) {

	std::vector<dfgNode*> res;
	for(dfgNode* node : NodeList){
		if(node->BB != BB) continue;
		if(node->getNode()){
			if(StoreInst* STI = dyn_cast<StoreInst>(node->getNode())){
				bool parentInThisBB = false;
				for(dfgNode* parent : node->getAncestors()){
					if(parent->BB == BB) parentInThisBB = true; break;
				}
				if(!parentInThisBB) res.push_back(node);
			}
			else if(BranchInst* BRI = dyn_cast<BranchInst>(node->getNode())){
				if(!BRI->isConditional()){
					res.push_back(node);
				}
			}
//			else if(PHINode* PHI = dyn_cast<PHINode>(node->getNode())){
//				res.push_back(node);
//			}
		}
		else if(node->getNameType() == "OutLoopSTORE"){
			bool parentInThisBB = false;
			for(dfgNode* parent : node->getAncestors()){
				if(parent->BB == BB) parentInThisBB = true; break;
			}
			if(!parentInThisBB) res.push_back(node);
		}
	}
	return res;
}

int DFGPartPred::handlePHINodes(std::set<BasicBlock*> LoopBB) {

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

			outs() << "DEBUG........... begin\n";
			outs() << "PHI :: "; PHI->dump();


			for (int i = 0; i < PHI->getNumIncomingValues(); ++i) {
				BasicBlock* bb = PHI->getIncomingBlock(i);
				Value* V = PHI->getIncomingValue(i);
				outs() << "V :: "; V->dump();
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

					BRI->dump();
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

					outs() << "previousCTRLNode : "; previousCTRLNode->getNode()->dump();
					outs() << "CTRL VAL : " << incomingCTRLVal << "\n";
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
//					for (int succ = 0; succ < previousCTRLNode->BB->getTerminator()->getNumSuccessors(); ++succ) {
//						if(previousCTRLNode->BB->getTerminator()->getSuccessor(succ)==phiParent->BB){
//							isPhiParentSuccCTRL=true;
//							break;
//						}
//					}

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

			outs() << "USERS ::::::::::::\n";
			for(User* u : PHI->users()){
				u->dump();
				if(PHINode* phiChildIns = dyn_cast<PHINode>(u)){
					if(dfgNode* phichildnode = findNode(phiChildIns)){
						recursivePhiNodes=true;
						break;
					}
				}
			}

			assert(node->getAncestors().empty());

//			assert(!backedgeChildMergeNodes.empty());

			//This is hack for keep phinodes being removed
//			recursivePhiNodes=true;

			if(recursivePhiNodes){
				for(dfgNode* mergeNode : mergeNodes){

					bool isBackEdge;
					if(backedgeChildMergeNodes.find(mergeNode)!=backedgeChildMergeNodes.end()){
						isBackEdge = true;
					}
					else{
						isBackEdge = checkBackEdge(mergeNode,node);
					}

					//this is if we decide to keep the phi nodes then there children should be backedges
					//if one of the ancestor is a backedge
//					if(isBackEdge){
//						for(dfgNode* child : node->getChildren()){
//							node->childBackEdgeMap[child]=true;
//							child->ancestorBackEdgeMap[node]=true;
//						}
//					}
//					isBackEdge = false;

//					bool isBackEdge = checkBackEdge(actPHI,node);
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

						bool isBackEdge;
						if(backedgeChildMergeNodes.find(mergeNode)!=backedgeChildMergeNodes.end()){
							isBackEdge = true;
						}
						else{
							isBackEdge = checkBackEdge(mergeNode,child);
						}

//						bool isBackEdge = checkBackEdge(actPHI,child);


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

dfgNode* DFGPartPred::getStartNode(BasicBlock* BB, dfgNode* PHINode) {
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

dfgNode* DFGPartPred::insertMergeNode(dfgNode* PHINode, dfgNode* ctrl, bool controlVal, dfgNode* data) {
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
//	isCTRL2PHIBackEdge = checkBackEdge(ctrl,PHINode);
	isCTRL2PHIBackEdge=false;

	cmergePHINodes[temp]=PHINode;

//	bbCuple = std::make_pair(dataBB,phiBB);
//	if(std::find(BackedgeBBs.begin(),BackedgeBBs.end(),bbCuple)!=BackedgeBBs.end()){
//		isDATA2PHIBackEdge=true;
//	}
//	isDATA2PHIBackEdge = checkBackEdge(data,PHINode);
	isDATA2PHIBackEdge = isCTRL2PHIBackEdge;

	temp->setNameType("CMERGE");
//	temp->BB = PHINode->BB;

	if(startNodes.find((BasicBlock*)ctrl->BB) != startNodes.end()){
		temp->BB = PHINode->BB;
	}
	else{
		temp->BB = ctrl->BB;
		if(checkBackEdge(ctrl,PHINode) || ctrl->BB == PHINode->BB) backedgeChildMergeNodes.insert(temp);
	}

	temp->setIdx(NodeList.size());
	NodeList.push_back(temp);

	cmergeCtrlInputs[temp]=ctrl;
	cmergeDataInputs[temp]=data;

	temp->addAncestorNode(ctrl,EDGE_TYPE_DATA,isCTRL2PHIBackEdge,true,controlVal);
	ctrl->addChildNode(temp,EDGE_TYPE_DATA,isCTRL2PHIBackEdge,true,controlVal);

	temp->addAncestorNode(data,EDGE_TYPE_DATA,isDATA2PHIBackEdge,false);
	data->addChildNode(temp,EDGE_TYPE_DATA,isDATA2PHIBackEdge,false);
	return temp;
}

dfgNode* DFGPartPred::insertMergeNode(dfgNode* PHINode, dfgNode* ctrl,
		bool controlVal, int val) {

	dfgNode* temp = new dfgNode(this);

	SmallVector<std::pair<const BasicBlock *, const BasicBlock *>,8> BackedgeBBs;
	FindFunctionBackedges(*(PHINode->getNode()->getFunction()),BackedgeBBs);
	const BasicBlock* ctrlBB = ctrl->BB;
	const BasicBlock* phiBB  = PHINode->BB;

	bool isCTRL2PHIBackEdge=false;

	cmergePHINodes[temp]=PHINode;

//	std::pair<const BasicBlock *, const BasicBlock *> bbCuple;
//	bbCuple = std::make_pair(ctrlBB,phiBB);
//	if(std::find(BackedgeBBs.begin(),BackedgeBBs.end(),bbCuple)!=BackedgeBBs.end()){
//		isCTRL2PHIBackEdge=true;
//	}
//	isCTRL2PHIBackEdge = checkBackEdge(ctrl,PHINode);
//	backedgeChildMergeNodes.insert(temp);

	temp->setNameType("CMERGE");
//	temp->BB = PHINode->BB;

	cmergeCtrlInputs[temp]=ctrl;


	if(startNodes.find((BasicBlock*)ctrl->BB) != startNodes.end()){
		temp->BB = PHINode->BB;
	}
	else{
		temp->BB = ctrl->BB;
		if(checkBackEdge(ctrl,PHINode) || ctrl->BB == PHINode->BB) backedgeChildMergeNodes.insert(temp);
	}


	temp->setIdx(NodeList.size());
	NodeList.push_back(temp);

	temp->addAncestorNode(ctrl,EDGE_TYPE_DATA,isCTRL2PHIBackEdge,true,controlVal);
	ctrl->addChildNode(temp,EDGE_TYPE_DATA,isCTRL2PHIBackEdge,true,controlVal);
	temp->setConstantVal(val);
	return temp;
}

void DFGPartPred::removeDisconnetedNodes() {
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

int DFGPartPred::handleSELECTNodes() {
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

dfgNode* DFGPartPred::combineConditionAND(dfgNode* brcond, dfgNode* selcond, dfgNode* child) {
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

bool DFGPartPred::checkBackEdge(dfgNode* src, dfgNode* dest) {
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

//	if(srcBB == destBB) return true;

	if(std::find(BBSuccBasicBlocks[srcBB].begin(), BBSuccBasicBlocks[srcBB].end(), destBB) != BBSuccBasicBlocks[srcBB].end()){
		return false; //not a backedge
	}
	else{
		return true;
	}

}

void DFGPartPred::generateTrigDFGDOT() {

	connectBB();
	handlePHINodes(this->loopBB);
//	insertshiftGEPs();
	addMaskLowBitInstructions();
	removeDisconnetedNodes();
//	scheduleCleanBackedges();
	fillCMergeMutexNodes();
	constructCMERGETree();
	scheduleASAP();
	scheduleALAP();
//	assignALAPasASAP();
//	balanceSched();
	printDOT(this->name + "_TrigDFG.dot");
	printNewDFGXML();
}

bool DFGPartPred::checkPHILoop(dfgNode* node1, dfgNode* node2) {
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

bool DFGPartPred::checkBRPath(const BranchInst* BRParentBRI, const BasicBlock* currBB, bool& condVal,  std::set<const BasicBlock*>& searchedSoFar) {
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

void DFGPartPred::scheduleCleanBackedges() {

	std::queue<dfgNode*> q;
	std::set<dfgNode*> visited;

	for(std::pair<BasicBlock*,dfgNode*> pair : startNodes){
		q.push(pair.second);
	}

	assert(q.size() == 1);

	while(!q.empty()){
		dfgNode* nFront = q.front(); q.pop();
		for(dfgNode* child : nFront->getChildren()){
			if(visited.find(child) != visited.end()){ // already visited, should be a backedge
				nFront->setBackEdge(child,true);
			}
			else{ // should not be a backedge
				nFront->setBackEdge(child,false);
				q.push(child);
			}
		}

		visited.insert(nFront);
	}

}

void DFGPartPred::fillCMergeMutexNodes() {

	for(dfgNode* node : NodeList){
		std::set<dfgNode*> currMutexSet;
		for(dfgNode* parent : node->getAncestors()){
			if(parent->getNameType() == "CMERGE"){
				currMutexSet.insert(parent);
			}
		}

		if(!currMutexSet.empty()) mutexSets.insert(currMutexSet);

		for(dfgNode* m1 : currMutexSet){
			for(dfgNode* m2 : currMutexSet){
				if(m1 == m2) continue;
				mutexNodes[m1].insert(m2);
			}
		}
	}

}

void DFGPartPred::constructCMERGETree() {
	assert(!mutexSets.empty());

	for(std::set<dfgNode*> cmergeSet : mutexSets){
		std::queue<dfgNode*> thisIterSet;
		std::queue<dfgNode*> nextIterSet;

		for(dfgNode* cmergeNode : cmergeSet){
			bool isBackEdge=false;
			for(dfgNode* child : cmergeNode->getChildren()){
				if(isBackEdge) assert(cmergeNode->childBackEdgeMap[child] == true);
				if(cmergeNode->childBackEdgeMap[child]){
					isBackEdge = true;
				}
			}

			if(isBackEdge){
				selectPHIAncestorMap[cmergeNode]=cmergePHINodes[cmergeNode];
				nextIterSet.push(cmergeNode);
			}
			else{
				selectPHIAncestorMap[cmergeNode]=cmergePHINodes[cmergeNode];
				thisIterSet.push(cmergeNode);
			}
		}

		if(thisIterSet.size() + nextIterSet.size() <= 2) continue;

		std::cout << "This ITER connections CMERGE Tree.\n";
		while(!thisIterSet.empty()){
			dfgNode* cmerge_left = thisIterSet.front(); thisIterSet.pop();
			std::cout << "CMERGE LEFT : " << cmerge_left->getIdx() << ",";
			if(thisIterSet.empty()){ //odd number of nodes

			}
			else{
				dfgNode* cmerge_right = thisIterSet.front(); thisIterSet.pop();
				if(thisIterSet.empty()){
					std::cout << "\n";
					continue; //no need to merge the last two;
				}

				std::cout << "CMERGE RIGHT : " << cmerge_right->getIdx() << ",";
				dfgNode* temp = new dfgNode(this);
				temp->setIdx(1000 + NodeList.size());
				temp->setNameType("SELECTPHI");
				temp->BB = cmerge_left->BB;
				NodeList.push_back(temp);
				selectPHIAncestorMap[temp] = selectPHIAncestorMap[cmerge_left];
				std::cout << "CREATING NEW NODE = " << temp->getIdx() << ",";

				assert(cmerge_left->getChildren().size() == cmerge_right->getChildren().size());
				std::vector<dfgNode*> cmergeLeftChildrenOrig = cmerge_left->getChildren();
				std::vector<dfgNode*> cmergeRightChildrenOrig = cmerge_right->getChildren();

				temp->addAncestorNode(cmerge_left); cmerge_left->addChildNode(temp);
				temp->addAncestorNode(cmerge_right); cmerge_right->addChildNode(temp);

				for (int i = 0; i < cmergeLeftChildrenOrig.size(); ++i) {
					dfgNode* left_child = cmergeLeftChildrenOrig[i];
					dfgNode* right_child = cmergeRightChildrenOrig[i];
					assert(left_child == right_child);

					cmerge_left->removeChild(left_child); left_child->removeAncestor(cmerge_left);
					cmerge_right->removeChild(right_child); right_child->removeAncestor(cmerge_right);
					left_child->addAncestorNode(temp); temp->addChildNode(left_child);
				}
				thisIterSet.push(temp);
			}
			std::cout << "\n";
		}

		std::cout << "Next ITER connections CMERGE Tree.\n";
		while(!nextIterSet.empty()){
			dfgNode* cmerge_left = nextIterSet.front(); nextIterSet.pop();
			std::cout << "CMERGE LEFT : " << cmerge_left->getIdx() << ",";
			if(nextIterSet.empty()){ //odd number of nodes

			}
			else{
				dfgNode* cmerge_right = nextIterSet.front(); nextIterSet.pop();
				if(nextIterSet.empty()) {
					std::cout << "\n";
					continue; //no need to merge the last two;
				}

				std::cout << "CMERGE RIGHT : " << cmerge_right->getIdx() << ",";
				dfgNode* temp = new dfgNode(this);
				temp->setIdx(1000 + NodeList.size());
				temp->setNameType("SELECTPHI");
				temp->BB = cmerge_left->BB;
				NodeList.push_back(temp);
				std::cout << "CREATING NEW NODE = " << temp->getIdx() << ",";
				selectPHIAncestorMap[temp] = selectPHIAncestorMap[cmerge_left];

				assert(cmerge_left->getChildren().size() == cmerge_right->getChildren().size());
				std::vector<dfgNode*> cmergeLeftChildrenOrig = cmerge_left->getChildren();
				std::vector<dfgNode*> cmergeRightChildrenOrig = cmerge_right->getChildren();

				temp->addAncestorNode(cmerge_left); cmerge_left->addChildNode(temp);
				temp->addAncestorNode(cmerge_right); cmerge_right->addChildNode(temp);


				for (int i = 0; i < cmergeLeftChildrenOrig.size(); ++i) {
					dfgNode* left_child = cmergeLeftChildrenOrig[i];
					dfgNode* right_child = cmergeRightChildrenOrig[i];
					assert(left_child == right_child);

					cmerge_left->removeChild(left_child); left_child->removeAncestor(cmerge_left);
					cmerge_right->removeChild(right_child); right_child->removeAncestor(cmerge_right);
					left_child->addAncestorNode(temp,EDGE_TYPE_DATA,true); temp->addChildNode(left_child,EDGE_TYPE_DATA,true);
				}
				nextIterSet.push(temp);
			}
			std::cout << "\n";
		}
	}
}

void DFGPartPred::scheduleASAP() {

	std::queue<std::vector<dfgNode*>> q;
	std::vector<dfgNode*> qv;
	for(std::pair<BasicBlock*,dfgNode*> p : startNodes){
		qv.push_back(p.second);
	}

	for(dfgNode* n : NodeList){
		if(n->getNameType() == "OutLoopLOAD"){
			if(n->getAncestors().size() == 0){
				qv.push_back(n);
			}
		}
	}

	q.push(qv);

	int level = 0;

	std::set<dfgNode*> visitedNodes;

	outs() << "Schedule ASAP ..... \n";


	while(!q.empty()){
		std::vector<dfgNode*> q_element = q.front(); q.pop();
		std::vector<dfgNode*> nqv; nqv.clear();

		if(level > maxASAPLevel) maxASAPLevel = level;


		for(dfgNode* node : q_element){
			visitedNodes.insert(node);

			if(node->getASAPnumber() < level){
				node->setASAPnumber(level);
			}



			for(dfgNode* child : node->getChildren()){
				bool isBackEdge = node->childBackEdgeMap[child];
				if(!isBackEdge){
					nqv.push_back(child);
				}
			}
		}
		if(!nqv.empty()){
			q.push(nqv);
		}
//		outs() << "Node Idx = " << node->getIdx();
		level++;
	}

	assert(visitedNodes.size() == NodeList.size());
}

void DFGPartPred::scheduleALAP() {

	std::queue<std::vector<dfgNode*>> q;
	std::vector<dfgNode*> qv;

	assert(maxASAPLevel != 0);

	//initially fill this with childless nodes
	for(dfgNode* node : NodeList){
		int childCount = 0;
		for(dfgNode* child : node->getChildren()){
			if(node->childBackEdgeMap[child]) continue;
			childCount++;
		}
		if(childCount == 0){
			qv.push_back(node);
		}
	}
	q.push(qv);

	int level = maxASAPLevel;
	std::set<dfgNode*> visitedNodes;

	outs() << "Schedule ALAP ..... \n";

	while(!q.empty()){
		assert(level >= 0);
		std::vector<dfgNode*> q_element = q.front(); q.pop();
		std::vector<dfgNode*> nq; nq.clear();

		for(dfgNode* node : q_element){
			visitedNodes.insert(node);

			if(node->getALAPnumber() > level || node->getALAPnumber() == -1){
				node->setALAPnumber(level);
			}

			for(dfgNode* parent : node->getAncestors()){
				if(parent->childBackEdgeMap[node]) continue;
				nq.push_back(parent);
			}
		}

		if(!nq.empty()) q.push(nq);

		level--;
	}


	assert(visitedNodes.size() == NodeList.size());
}

void DFGPartPred::balanceSched() {

	for(dfgNode* node : NodeList){

		int asap = node->getASAPnumber();

		int leastASAPChild = node->getALAPnumber();
		for(dfgNode* child : node->getChildren()){
			if(node->childBackEdgeMap[child]) continue;
			if(child->getASAPnumber() < leastASAPChild){
				leastASAPChild = child->getASAPnumber();
			}
		}

		if(node->getALAPnumber() == maxASAPLevel) continue; // no need balance last level nodes

		int diff = leastASAPChild - asap;

		int new_asap = asap + diff/2;
		node->setASAPnumber(new_asap);
	}

}

void DFGPartPred::printNewDFGXML() {


	std::string fileName = name + "_DFG.xml";
	std::ofstream xmlFile;
	xmlFile.open(fileName.c_str());

	nameNodes();
	classifyParents();

//    insertMOVC();
//	scheduleASAP();
//	scheduleALAP();
//	balanceASAPALAP();
//				  LoopDFG.addBreakLongerPaths();
	CreateSchList();

	std::map<BasicBlock*,std::set<BasicBlock*>> mBBs = checkMutexBBs();
	std::map<std::string,std::set<std::string>> mBBs_str;

	std::map<int,std::vector<dfgNode*>> asaplevelNodeList;
	for (dfgNode* node : NodeList){
		asaplevelNodeList[node->getASAPnumber()].push_back(node);
	}

	std::map<dfgNode*,std::string> nodeBBModified;
	for(dfgNode* node : NodeList){
		nodeBBModified[node]=node->BB->getName().str();
	}


	for(dfgNode* node : NodeList){
		int cmergeParentCount=0;
		std::set<std::string> mutexBBs;
		for(dfgNode* parent: node->getAncestors()){
			if(HyCUBEInsStrings[parent->getFinalIns()] == "CMERGE" || parent->getNameType() == "SELECTPHI"){
				nodeBBModified[parent]=nodeBBModified[parent]+ "_" + std::to_string(node->getIdx()) + "_" + std::to_string(cmergeParentCount);
				mutexBBs.insert(nodeBBModified[parent]);
				cmergeParentCount++;
			}
		}
		for(std::string bb_str1 : mutexBBs){
			for(std::string bb_str2 : mutexBBs){
				if(bb_str2==bb_str1) continue;
				mBBs_str[bb_str1].insert(bb_str2);
			}
		}
	}


	xmlFile << "<MutexBB>\n";
	for(std::pair<BasicBlock*,std::set<BasicBlock*>> pair : mBBs){
		BasicBlock* first = pair.first;
		xmlFile << "<BB1 name=\"" << first->getName().str() << "\">\n";
		for(BasicBlock* second : pair.second){
			xmlFile << "\t<BB2 name=\"" << second->getName().str() << "\"/>\n";
		}
		xmlFile << "</BB1>\n";
	}
	for(std::pair<std::string,std::set<std::string>> pair : mBBs_str){
		std::string first = pair.first;
		xmlFile << "<BB1 name=\"" << first << "\">\n";
		for(std::string second : pair.second){
			xmlFile << "\t<BB2 name=\"" << second << "\"/>\n";
		}
		xmlFile << "</BB1>\n";
	}
	xmlFile << "</MutexBB>\n";

	xmlFile << "<DFG count=\"" << NodeList.size() << "\">\n";

	for(dfgNode* node : NodeList){
//	for (int i = 0; i < maxASAPLevel; ++i) {
//		for(dfgNode* node : asaplevelNodeList[i]){
			xmlFile << "<Node idx=\"" << node->getIdx() << "\"";
		    xmlFile << " ASAP=\"" << node->getASAPnumber() << "\"";
		    xmlFile << " ALAP=\"" << node->getALAPnumber() << "\"";

//		    if(node->getNameType() == "OutLoopLOAD") {
//		    	xmlFile << "OutLoopLOAD=\"1\"";
//		    }
//		    else{
//		    	xmlFile << "OutLoopLOAD=\"0\"";
//		    }
//
//		    if(node->getNameType() == "OutLoopSTORE") {
//		    	xmlFile << "OutLoopSTORE=\"1\"";
//		    }
//		    else{
//		    	xmlFile << "OutLoopSTORE=\"0\"";
//		    }

//		    xmlFile << "BB=\"" << node->BB->getName().str() << "\"";
		    xmlFile << "BB=\"" << nodeBBModified[node] << "\"";
		    if(node->hasConstantVal()){
		    	xmlFile << "CONST=\"" << node->getConstantVal() << "\"";
		    }
			xmlFile << ">\n";

			xmlFile << "<OP>";
			if((node->getNameType() == "OutLoopLOAD") || (node->getNameType() == "OutLoopSTORE") ){
				xmlFile << "O";
			}
			xmlFile << HyCUBEInsStrings[node->getFinalIns()] << "</OP>\n";
//			xmlFile << "<OP>" << HyCUBEInsStrings[node->getFinalIns()] << "</OP>\n";

			xmlFile << "<Inputs>\n";
			for(dfgNode* parent : node->getAncestors()){
	//			xmlFile << "\t<Input idx=\"" << parent->getIdx() << "\" type=\"DATA\"/>\n";
				xmlFile << "\t<Input idx=\"" << parent->getIdx() << "\"/>\n";
			}
	//		for(dfgNode* parentPHI : node->getPHIancestors()){
	//			xmlFile << "\t<Input idx=\"" << parentPHI->getIdx() << "\" type=\"PHI\"/>\n";
	//		}
			xmlFile << "</Inputs>\n";

			xmlFile << "<Outputs>\n";
			for(dfgNode* child : node->getChildren()){
	//			xmlFile << "\t<Output idx=\"" << child->getIdx() <<"\" type=\"DATA\"/>\n";
				xmlFile << "\t<Output idx=\"" << child->getIdx() << "\" ";

				if(node->childBackEdgeMap[child]){
					xmlFile << "nextiter=\"1\" ";
				}
				else{
					xmlFile << "nextiter=\"0\" ";
				}

				bool written=false;
				if(node->getNameType()=="CMERGE"){
					if(child->getNode()){
						if(dyn_cast<PHINode>(child->getNode())){

						}
						else{ // if not phi
							written = true;
							int operand_no = findOperandNumber(child,child->getNode(),cmergePHINodes[node]->getNode());
							if( operand_no == 0){
								xmlFile << "type=\"P\"/>\n";
							}
							else if ( operand_no == 1){
								xmlFile << "type=\"I1\"/>\n";
							}
							else if(( operand_no == 2)){
								xmlFile << "type=\"I2\"/>\n";
							}
							else{
								assert(false);
							}
						}
					}
				}

				if(node->getNameType()=="SELECTPHI"){
					if(child->getNode()){
						written = true;
						int operand_no = findOperandNumber(child,child->getNode(),selectPHIAncestorMap[node]->getNode());
						if( operand_no == 0){
							xmlFile << "type=\"P\"/>\n";
						}
						else if ( operand_no == 1){
							xmlFile << "type=\"I1\"/>\n";
						}
						else if(( operand_no == 2)){
							xmlFile << "type=\"I2\"/>\n";
						}
						else{
							assert(false);
						}
					}
				}

				if(written){
				}
				else if(child->parentClassification[0]==node){
					xmlFile << "type=\"P\"/>\n";
				}
				else if(child->parentClassification[1]==node){
					xmlFile << "type=\"I1\"/>\n";
				}
				else if(child->parentClassification[2]==node){
					xmlFile << "type=\"I2\"/>\n";
				}
				else{
					bool found=false;
					for(std::pair<int,dfgNode*> pair : child->parentClassification){
						if(pair.second == node){
							xmlFile << "type=\"I2\"/>\n";
							found=true;
							break;
						}
					}

					if(!found){
						outs() << "node = " << node->getIdx() << ", child = " << child->getIdx() << "\n";
					}

					assert(found);
				}
			}
	//		for(dfgNode* phiChild : node->getPHIchildren()){
	//			xmlFile << "\t<Output idx=\"" << phiChild->getIdx() << "\" type=\"PHI\"/>\n";
	//		}
			xmlFile << "</Outputs>\n";

			xmlFile << "<RecParents>\n";
			for(dfgNode* recParent : node->getRecAncestors()){
				xmlFile << "\t<RecParent idx=\"" << recParent->getIdx() << "\"/>\n";
			}
			xmlFile << "</RecParents>\n";

			xmlFile << "</Node>\n\n";

	}
//		}
//	}




	xmlFile << "</DFG>\n";
	xmlFile.close();




}

int DFGPartPred::classifyParents() {

	dfgNode* node;
	for (int i = 0; i < NodeList.size(); ++i) {
		node = NodeList[i];

		outs() << "classifyParent::currentNode = " << node->getIdx() << "\n";
		outs() << "Parents : ";
		for (dfgNode* parent : node->getAncestors()) {
			outs() << parent->getIdx() << ",";
		}
		outs() << "\n";

			for (dfgNode* parent : node->getAncestors()) {
				Instruction* ins;
				Instruction* parentIns;



				if(node->getNode()!=NULL){
					ins=node->getNode();
				}
				else{
					if(node->getNameType().compare("XORNOT")==0){
						assert(node->parentClassification.find(1) == node->parentClassification.end());
						node->parentClassification[1]=parent;
						continue;
					}
					if(node->getNameType().compare("ORZERO")==0){
						assert(node->parentClassification.find(1) == node->parentClassification.end());
						node->parentClassification[1]=parent;
						continue;
					}
					if(node->getNameType().compare("GEPLEFTSHIFT")==0){
						if(parent->getNode()!=NULL){
							parentIns = parent->getNode();
							if(BranchInst* BRI = dyn_cast<BranchInst>(parentIns)){
								node->parentClassification[0]=parent;
								continue;
							}
							else if(CmpInst * CMPI = dyn_cast<CmpInst>(parentIns)){
								node->parentClassification[0]=parent;
								continue;
							}
						}
						assert(node->parentClassification.find(1) == node->parentClassification.end());
						node->parentClassification[1]=parent;
						continue;
					}
					if(node->getNameType().compare("MASKAND")==0){
						assert(node->parentClassification.find(1) == node->parentClassification.end());
						node->parentClassification[1]=parent;
						continue;
					}
					if(node->getNameType().compare("PREDAND")==0){
						assert(node->parentClassification.find(1) == node->parentClassification.end());
						node->parentClassification[1]=parent;
						continue;
					}
					if(node->getNameType().compare("CTRLBrOR")==0){
						if(node->parentClassification.find(1) == node->parentClassification.end()){
							node->parentClassification[1]=parent;
						}
						else{
							assert(node->parentClassification.find(2) == node->parentClassification.end());
							node->parentClassification[2]=parent;
						}
						continue;
					}
					if(node->getNameType().compare("SELECTPHI")==0){
						if(node->parentClassification.find(1) == node->parentClassification.end()){
							node->parentClassification[1]=parent;
						}
						else{
							assert(node->parentClassification.find(2) == node->parentClassification.end());
							node->parentClassification[2]=parent;
						}
						continue;
					}
					if(node->getNameType().compare("GEPADD")==0){
						if(node->parentClassification.find(1) == node->parentClassification.end()){
							node->parentClassification[1]=parent;
						}
						else{
							assert(node->parentClassification.find(2) == node->parentClassification.end());
							node->parentClassification[2]=parent;
						}
						continue;
					}
					if(node->getNameType().compare("STORESTART")==0){
						assert(parent->getNode()==NULL);
						if(parent->getNameType().compare("MOVC") == 0){
							node->parentClassification[1]=parent;
						}
						else if(parent->getNameType().compare("LOOPSTART") == 0){
							node->parentClassification[0]=parent;
						}
						else if(parent->getNameType().compare("PREDAND") == 0){
							node->parentClassification[0]=parent;
						}
						else{
							assert(false);
						}
						continue;
					}
					if(node->getNameType().compare("LOOPEXIT")==0){
						if(parent->getNode()==NULL){
							if(parent->getNameType().compare("MOVC") == 0){
								assert(node->parentClassification.find(1) == node->parentClassification.end());
								node->parentClassification[1]=parent;
							}
							else if(parent->getNameType().compare("CTRLBrOR") == 0){
								assert(node->parentClassification.find(0) == node->parentClassification.end());
								node->parentClassification[0]=parent;
							}
							else{
								assert(false);
							}
						}
						else{
							assert(node->parentClassification.find(0) == node->parentClassification.end());
							node->parentClassification[0]=parent;
						}
						continue;
					}
					if(node->getNameType().compare("OutLoopLOAD")==0){
						//do nothing
						assert(node->parentClassification.find(0) == node->parentClassification.end());
						node->parentClassification[0]=parent;
						continue;
					}
					if(node->getNameType().compare("OutLoopSTORE")==0){
						assert(node->hasConstantVal());

						//check for predicate parent
						if(parent->getNode()!=NULL){
							parentIns = parent->getNode();
							if(BranchInst* BRI = dyn_cast<BranchInst>(parentIns)){
								node->parentClassification[0]=parent;
								continue;
							}
							else if(CmpInst * CMPI = dyn_cast<CmpInst>(parentIns)){
								node->parentClassification[0]=parent;
								continue;
							}
						}
						else{
							if(parent->getNameType().compare("CTRLBrOR")==0){
								node->parentClassification[0]=parent;
								continue;
							}
						}

						//if it comes to this, it has to be a data parent.
						node->parentClassification[1]=parent;
						continue;
					}
					if(node->getNameType().compare("CMERGE")==0){

						if(cmergeCtrlInputs[node] == parent){
							node->parentClassification[0] = parent;
						}
						else if(cmergeDataInputs[node] == parent){
							node->parentClassification[1] = parent;
						}
						else{
							assert(false);
						}
						continue;

					}
				}


				if(dyn_cast<PHINode>(ins)){
					if(parent->getNode() != NULL) parent->getNode()->dump();
					outs() << "node : " << node->getIdx() << "\n";
					outs() << "Parent : " << parent->getIdx() << "\n";
					outs() << "Parent NameType : " << parent->getNameType() << "\n";

					if(parent->getNode()){
						if(dyn_cast<BranchInst>(parent->getNode())){
							node->parentClassification[0]=parent;
							continue;
						}
					}

					assert(parent->getNameType().compare("CMERGE")==0);
					if(node->parentClassification.find(1) == node->parentClassification.end()){
						node->parentClassification[1]=parent;
					}
					else if(node->parentClassification.find(2) == node->parentClassification.end()){
//						assert(node->parentClassification.find(2) == node->parentClassification.end());
						node->parentClassification[2]=parent;
					}
					else{
						outs() << "Extra parent : " << parent->getIdx() << ",Nametype = " << parent->getNameType() << ",par_idx = " << node->parentClassification.size()+1 << "\n";
						node->parentClassification[node->parentClassification.size()+1]=parent;
					}
					continue;
				}

				if( dyn_cast<SelectInst>(ins) ){


				}

				if(dyn_cast<BranchInst>(ins)){
					if(node->parentClassification.find(1) == node->parentClassification.end()){
						node->parentClassification[1]=parent;
					}
					else{
						assert(node->parentClassification.find(2) == node->parentClassification.end());
						node->parentClassification[2]=parent;
					}
					continue;
				}

				if(parent->getNode()!=NULL){
					parentIns = parent->getNode();
					if(BranchInst* BRI = dyn_cast<BranchInst>(parentIns)){
						node->parentClassification[0]=parent;
						continue;
					}
//					else if(CmpInst * CMPI = dyn_cast<CmpInst>(parentIns)){
//						node->parentClassification[0]=parent;
//						continue;
//					}
				}
				else{
					if(parent->getNameType().compare("CTRLBrOR")==0){
						node->parentClassification[0]=parent;
						continue;
					}
					if(parent->getNameType().compare("PREDAND")==0){
						node->parentClassification[0]=parent;
						continue;
					}
//					if(parent->getNameType().compare("GEPLEFTSHIFT")==0){
//						assert(node->parentClassification.find(1)==node->parentClassification.end());
//						node->parentClassification[1]=parent;
//						continue;
//					}
					if(parent->getNameType().compare("GEPLEFTSHIFT")==0){
						assert(parent->getAncestors().size()<3);
						node->parentClassification[1]=parent;
						continue;
					}
					if(parent->getNameType().compare("MASKAND")==0){
						assert(parent->getAncestors().size()==1);
						parentIns = parent->getAncestors()[0]->getNode();
					}
					if(parent->getNameType().compare("OutLoopLOAD")==0){
						parentIns = OutLoopNodeMapReverse[parent];
					}
					if(parent->getNameType().compare("CMERGE")==0){
//						if(node->parentClassification.find(1) == node->parentClassification.end()){
//							node->parentClassification[1]=parent;
//						}
//						else if(node->parentClassification.find(2) == node->parentClassification.end()){
//	//						assert(node->parentClassification.find(2) == node->parentClassification.end());
//							node->parentClassification[2]=parent;
//						}
//						else{
//							outs() << "Extra parent : " << parent->getIdx() << ",Nametype = " << parent->getNameType() << ",par_idx = " << node->parentClassification.size()+1 << "\n";
//							node->parentClassification[node->parentClassification.size()+1]=parent;
//						}
//						continue;

						parentIns = cmergePHINodes[parent]->getNode();
						outs() << "CMERGE Parent" << parent->getIdx() << "to non-nametype child=" << node->getIdx() << ".\n";
						outs() << "parentIns = "; parentIns->dump();
						outs() << "node = "; node->getNode()->dump();
						outs() << "OpNumber = " << findOperandNumber(node, ins,parentIns) << "\n";

						assert(parentIns);
					}
					if(parent->getNameType().compare("SELECTPHI")==0){
						parentIns = selectPHIAncestorMap[parent]->getNode();
						outs() << "SELECTPHI Parent" << parent->getIdx() << "to non-nametype child=" << node->getIdx() << ".\n";
						outs() << "parentIns = "; parentIns->dump();
						outs() << "node = "; node->getNode()->dump();
						outs() << "OpNumber = " << findOperandNumber(node, ins,parentIns) << "\n";

						assert(parentIns);
					}
				}
				//Only ins!=NULL and non-PHI and non-BR will reach here
				node->parentClassification[findOperandNumber(node, ins,parentIns)]=parent;
		}
	}

}

void DFGPartPred::assignALAPasASAP() {

	for(dfgNode* node : NodeList){
		node->setASAPnumber(node->getALAPnumber());
	}

}



dfgNode* DFGPartPred::addLoadParent(Instruction* ins, dfgNode* child) {
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

void DFGPartPred::printDOT(std::string fileName) {
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

//		ofs << "subgraph cluster_" << cluster_idx << " {\n";
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

			ofs << "BB=" << node->BB->getName().str();

			if(!mutexNodes[node].empty()){
				ofs << ",mutex={";
				for(dfgNode* m : mutexNodes[node]){
					ofs << m->getIdx() << ",";
				}
				ofs << "}";
			}

			if(node->getFinalIns() != NOP){
				ofs << " HyIns=" << HyCUBEInsStrings[node->getFinalIns()];
			}
			ofs << ",\n";
			ofs << node->getIdx() << ", ASAP=" << node->getASAPnumber();
			ofs << ", ALAP=" << node->getALAPnumber();

			ofs << "\"]\n"; //END NODE NAME
		}

//		ofs << "label = " << "\"" << BB->getName().str() << "\"" << ";\n";
//		ofs << "color = purple;\n";
//		ofs << "}\n";
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
