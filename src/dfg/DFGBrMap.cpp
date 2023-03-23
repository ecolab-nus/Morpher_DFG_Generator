#include <morpherdfggen/dfg/DFGBrMap.h>

#include "llvm/Analysis/CFG.h"
#include <algorithm>
#include <queue>
#include <map>
#include <set>
#include <vector>

void DFGBrMap::connectBB() {

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

void DFGBrMap::removeOutLoopLoad() {

	std::set<dfgNode*> removalNodes;
	for(dfgNode* node : NodeList){

		if(node->getNameType() == "OutLoopLOAD"){
			bool oloadonlyparent=false;
			for(dfgNode* child : node->getChildren()){
				if(child->getAncestors().size() == 1){
					oloadonlyparent=true;
					break;
				}
			}
			if(oloadonlyparent) continue;

			removalNodes.insert(node);
			assert(node->getAncestors().empty());

			for(dfgNode* child : node->getChildren()){
				node->removeChild(child);
				child->removeAncestor(node);
			}
		}

	}

	for(dfgNode* rn : removalNodes){
		std::cout << "Removing Node = " << rn->getIdx() << "\n";
		NodeList.erase(std::remove(NodeList.begin(), NodeList.end(), rn), NodeList.end());
	}

	name = name + "_noSemiOLOAD";

	scheduleASAP();
	scheduleALAP();

}


std::vector<dfgNode*> DFGBrMap::getStoreInstructions(BasicBlock* BB) {

	std::vector<dfgNode*> res;
	for(dfgNode* node : NodeList){
		if(node->BB != BB) continue;
		if(node->getNode()){
			if(StoreInst* STI = dyn_cast<StoreInst>(node->getNode())){
				bool parentInThisBB = false;
//				for(dfgNode* parent : node->getAncestors()){
//					if(parent->BB == BB) parentInThisBB = true; break;
//				}
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
//			for(dfgNode* parent : node->getAncestors()){
//				if(parent->BB == BB) parentInThisBB = true; break;
//			}
			if(!parentInThisBB) res.push_back(node);
		}
	}
	return res;
}

int DFGBrMap::handlePHINodes(std::set<BasicBlock*> LoopBB) {

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
//				bool incomingCTRLVal;
				std::map<dfgNode*,bool> cnodectrlvalmap;
				std::map<dfgNode*,BasicBlock*> cnodeDestBBMap;

//				dfgNode* previousCTRLNode;
				std::vector<dfgNode*> previousCtrlNodes;

				if(LoopBB.find(bb) == LoopBB.end()){
					//Not found
					std::pair<BasicBlock*,BasicBlock*> bbPair = std::make_pair(bb,(BasicBlock*)node->BB);
					assert(loopentryBB.find(bbPair)!=loopentryBB.end());
//					previousCTRLNode = getStartNode(bb,node);
					previousCtrlNodes.push_back(getStartNode(bb,node));
//					incomingCTRLVal=true; // all start nodes are assumed to be true
					cnodectrlvalmap[getStartNode(bb,node)]=true;
//					cnodeDestBBMap[getStartNode(bb,node)]=node->BB;
				}
				else{ // within the loop
					BranchInst* BRI = cast<BranchInst>(bb->getTerminator());
					dfgNode* BRNode = findNode(BRI);
					bool isConditional = BRI->isConditional();

//					BRI->dump();
//					assert(BRNode->getAncestors().size()==1);
//					previousCTRLNode = BRNode->getAncestors()[0];


					if(!isConditional){
						const BasicBlock* currBB = BRI->getParent();
						previousCtrlNodes = BRNode->getAncestors();

						for(dfgNode* pcn : previousCtrlNodes){
							const BasicBlock* BRParentBB = pcn->BB;
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
							bool incomingCTRLVal;
							assert(checkBRPath(BRParentBRI,currBB,incomingCTRLVal,searchSoFar));
							cnodectrlvalmap[pcn]=incomingCTRLVal;
							isConditional=true; //this is always true. coz there is no backtoback unconditional branches.

						}

					}
					else{
						assert(BRNode->getAncestors().size()==1);
//						previousCTRLNode = BRNode->getAncestors()[0];
						previousCtrlNodes.push_back(BRNode->getAncestors()[0]);
						if(BRI->getSuccessor(0) == node->BB){
//							incomingCTRLVal = true;
							cnodectrlvalmap[BRNode->getAncestors()[0]]=true;
						}
						else{
//							incomingCTRLVal = false;
							cnodectrlvalmap[BRNode->getAncestors()[0]]=false;
							assert(BRI->getSuccessor(1) == node->BB);
						}
					}

					assert(!previousCtrlNodes.empty());

					for(dfgNode* pcn : previousCtrlNodes) {
						outs() << "pcn idx=" << pcn->getIdx() << ",";
						if(pcn->getNode()){
							outs() << "previousCTRLNode : "; pcn->getNode()->dump();
						}
						else{
							outs() << "\n";
						}
						outs() << "CTRL VAL : " << cnodectrlvalmap[pcn] << "\n";
					}

				}
//				assert(previousCTRLNode!=NULL);
				assert(!previousCtrlNodes.empty());

				for(dfgNode* previousCTRLNode : previousCtrlNodes){
					bool incomingCTRLVal = cnodectrlvalmap[previousCTRLNode];
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

						dfgNode* phiParent = NULL;
						if(Instruction* ins = dyn_cast<Instruction>(V)){
							phiParent = findNode(ins);
						}
						bool isPhiParentOutLoopLoad=false;
						if(phiParent == NULL){ //not found
							isPhiParentOutLoopLoad=true;
							if(OutLoopNodeMap.find(V)!=OutLoopNodeMap.end()){
								phiParent=OutLoopNodeMap[V];
							}
							else{
								phiParent=addLoadParent(V,node);
								bool isBackEdge = checkBackEdge(previousCTRLNode,phiParent);
	//							previousCTRLNode->addChildNode(phiParent,EDGE_TYPE_DATA,isBackEdge,true,incomingCTRLVal);
	//							phiParent->addAncestorNode(previousCTRLNode,EDGE_TYPE_DATA,isBackEdge,true,incomingCTRLVal);
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
							outs() << "PhiParent BB = " << phiParent->BB->getName() << "\n";
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
			// UPDATE : i just keep the induction variable
			bool isInductionVar=false;
			if(PHI == currLoop->getCanonicalInductionVariable()){
//				recursivePhiNodes=true;
//				isInductionVar=true;
			}

			//cmerge node has backedges, its better to keep the phi node
			for(dfgNode* mergeNode : mergeNodes){
				if(backedgeChildMergeNodes.find(mergeNode)!=backedgeChildMergeNodes.end()){
					recursivePhiNodes=true;
				}
			}

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
					if(!isInductionVar || !isBackEdge ){
						node->addAncestorNode(mergeNode,EDGE_TYPE_DATA,isBackEdge);
						mergeNode->addChildNode(node,EDGE_TYPE_DATA,isBackEdge);
					}
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

dfgNode* DFGBrMap::getStartNode(BasicBlock* BB, dfgNode* PHINode) {
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

dfgNode* DFGBrMap::insertMergeNode(dfgNode* PHINode, dfgNode* ctrl, bool controlVal, dfgNode* data) {
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
	cmergeNodesPHI[PHINode].insert(temp);

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
		temp->BB = PHINode->BB;
//		temp->BB = ctrl->BB;
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

dfgNode* DFGBrMap::insertMergeNode(dfgNode* PHINode, dfgNode* ctrl,
		bool controlVal, int val) {

	dfgNode* temp = new dfgNode(this);

	SmallVector<std::pair<const BasicBlock *, const BasicBlock *>,8> BackedgeBBs;
	FindFunctionBackedges(*(PHINode->getNode()->getFunction()),BackedgeBBs);
	const BasicBlock* ctrlBB = ctrl->BB;
	const BasicBlock* phiBB  = PHINode->BB;

	bool isCTRL2PHIBackEdge=false;

	cmergePHINodes[temp]=PHINode;
	cmergeNodesPHI[PHINode].insert(temp);

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
		temp->BB = PHINode->BB;
//		temp->BB = ctrl->BB;
		if(checkBackEdge(ctrl,PHINode) || ctrl->BB == PHINode->BB) backedgeChildMergeNodes.insert(temp);
	}


	temp->setIdx(NodeList.size());
	NodeList.push_back(temp);

	temp->addAncestorNode(ctrl,EDGE_TYPE_DATA,isCTRL2PHIBackEdge,true,controlVal);
	ctrl->addChildNode(temp,EDGE_TYPE_DATA,isCTRL2PHIBackEdge,true,controlVal);
	temp->setConstantVal(val);
	return temp;
}

void DFGBrMap::removeDisconnetedNodes() {
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

int DFGBrMap::handleSELECTNodes() {
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


	return 0;
}

dfgNode* DFGBrMap::combineConditionAND(dfgNode* brcond, dfgNode* selcond, dfgNode* child) {
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

bool DFGBrMap::checkBackEdge(dfgNode* src, dfgNode* dest) {
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

void DFGBrMap::generateTrigDFGDOT(Function &F) {

//	connectBB();
	connectBBTrig();
	createCtrlBROrTree();

	handlePHINodes(this->loopBB);
//	insertshiftGEPs();
	addMaskLowBitInstructions();
	removeDisconnetedNodes();
//	scheduleCleanBackedges();
	fillCMergeMutexNodes();


//	constructCMERGETree();

//	printDOT(this->name + "_PartPredDFG.dot"); return;

	scheduleASAP();
	scheduleALAP();
//	assignALAPasASAP();
//	balanceSched();
	removeOutLoopLoad();

	GEPBaseAddrCheck(F);
	nameNodes();
	classifyParents();

	addOrphanPseudoEdges();

//	createDualInsNodes();
	mergePHIParents();

	printDOT(this->name + "_DFGBrMap.dot");
	printNewDFGXML();
}

bool DFGBrMap::checkPHILoop(dfgNode* node1, dfgNode* node2) {
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

bool DFGBrMap::checkBRPath(const BranchInst* BRParentBRI, const BasicBlock* currBB, bool& condVal,  std::set<const BasicBlock*>& searchedSoFar) {
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

void DFGBrMap::scheduleCleanBackedges() {

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

void DFGBrMap::fillCMergeMutexNodes() {

	for(dfgNode* node : NodeList){
		std::set<dfgNode*> currMutexSet;
		for(dfgNode* parent : node->getAncestors()){
			if(parent->getNameType() == "CMERGE"){
				currMutexSet.insert(parent);
			}
		}

		if(!currMutexSet.empty()) mutexSets.insert(currMutexSet);
		mutexSetCommonChildren[currMutexSet].insert(node);

		for(dfgNode* m1 : currMutexSet){
			for(dfgNode* m2 : currMutexSet){
				if(m1 == m2) continue;
				mutexNodes[m1].insert(m2);
			}
		}
	}

}

void DFGBrMap::constructCMERGETree() {
	assert(!mutexSets.empty());

	for(std::set<dfgNode*> cmergeSet : mutexSets){
		std::queue<dfgNode*> thisIterSet;
		std::queue<dfgNode*> nextIterSet;

		outs() << "CMERGE(ThisIter) set =";
		for(dfgNode* cmergeNode : cmergeSet){
			bool isBackEdge=false;
			for(dfgNode* child : mutexSetCommonChildren[cmergeSet]){
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
				outs() << cmergeNode->getIdx() << ",";
				selectPHIAncestorMap[cmergeNode]=cmergePHINodes[cmergeNode];
				thisIterSet.push(cmergeNode);
			}
		}
		outs() << "\n";

		if(thisIterSet.size() + nextIterSet.size() <= 1) continue;

		std::cout << "This ITER connections CMERGE Tree.\n";
		while(!thisIterSet.empty()){
			dfgNode* cmerge_left = thisIterSet.front(); thisIterSet.pop();
			std::cout << "CMERGE LEFT : " << cmerge_left->getIdx() << ",";
			if(thisIterSet.empty()){ //odd number of nodes

			}
			else{
				dfgNode* cmerge_right = thisIterSet.front(); thisIterSet.pop();
//				if(thisIterSet.empty()){
//					std::cout << "\n";
//					continue; //no need to merge the last two;
//				}

				std::cout << "CMERGE RIGHT : " << cmerge_right->getIdx() << ",";
				dfgNode* temp = new dfgNode(this);
				temp->setIdx(1000 + NodeList.size());
				temp->setNameType("SELECTPHI");
				temp->BB = cmerge_left->BB;
				NodeList.push_back(temp);
				selectPHIAncestorMap[temp] = selectPHIAncestorMap[cmerge_left];
				std::cout << "CREATING NEW NODE = " << temp->getIdx() << ",";

//				assert(cmerge_left->getChildren().size() == cmerge_right->getChildren().size());
				std::set<dfgNode*> cmergeLeftChildrenOrig = mutexSetCommonChildren[cmergeSet];
				std::set<dfgNode*> cmergeRightChildrenOrig = mutexSetCommonChildren[cmergeSet];

				temp->addAncestorNode(cmerge_left); cmerge_left->addChildNode(temp);
				temp->addAncestorNode(cmerge_right); cmerge_right->addChildNode(temp);

//				for (int i = 0; i < cmergeLeftChildrenOrig.size(); ++i) {
				for(dfgNode* child : cmergeLeftChildrenOrig){
					dfgNode* left_child = child;
					dfgNode* right_child = child;
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
//				if(nextIterSet.empty()) {
//					std::cout << "\n";
//					continue; //no need to merge the last two;
//				}

				std::cout << "CMERGE RIGHT : " << cmerge_right->getIdx() << ",";
				dfgNode* temp = new dfgNode(this);
				temp->setIdx(1000 + NodeList.size());
				temp->setNameType("SELECTPHI");
				temp->BB = cmerge_left->BB;
				NodeList.push_back(temp);
				std::cout << "CREATING NEW NODE = " << temp->getIdx() << ",";
				selectPHIAncestorMap[temp] = selectPHIAncestorMap[cmerge_left];

//				assert(cmerge_left->getChildren().size() == cmerge_right->getChildren().size());
				std::set<dfgNode*> cmergeLeftChildrenOrig = mutexSetCommonChildren[cmergeSet];
				std::set<dfgNode*> cmergeRightChildrenOrig = mutexSetCommonChildren[cmergeSet];

				temp->addAncestorNode(cmerge_left); cmerge_left->addChildNode(temp);
				temp->addAncestorNode(cmerge_right); cmerge_right->addChildNode(temp);

	//				for (int i = 0; i < cmergeLeftChildrenOrig.size(); ++i) {
					for(dfgNode* child : cmergeLeftChildrenOrig){
						dfgNode* left_child = child;
						dfgNode* right_child = child;
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

void DFGBrMap::scheduleASAP() {

	for(dfgNode* n : NodeList){
		n->setASAPnumber(-1);
	}

	std::queue<std::vector<dfgNode*>> q;
	std::vector<dfgNode*> qv;
	for(std::pair<BasicBlock*,dfgNode*> p : startNodes){
		qv.push_back(p.second);
	}

	for(dfgNode* n : NodeList){
//		if(n->getNameType() == "OutLoopLOAD"){
			if(n->getAncestors().size() == 0){
				qv.push_back(n);
			}
//		}
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

void DFGBrMap::scheduleALAP() {

	for(dfgNode* n : NodeList){
		n->setALAPnumber(-1);
	}

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

void DFGBrMap::balanceSched() {

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

void DFGBrMap::printNewDFGXML() {


	std::string fileName = name + "_BrMap_DFG.xml";
	std::ofstream xmlFile;
	xmlFile.open(fileName.c_str());



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

			if(node->getArrBasePtr() != "NOT_A_MEM_OP"){
				xmlFile << "<BasePointerName size=\"" << array_pointer_sizes[node->getArrBasePtr()] << "\">";	
				xmlFile << node->getArrBasePtr();
				xmlFile << "</BasePointerName>\n";
			}

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
				if(findEdge(node,child)->getType() == EDGE_TYPE_PS){
					xmlFile << "type=\"PS\"/>\n";
					written=true;
				}
				else if(node->getNameType()=="CMERGE"){
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
								outs() << "node=" << node->getIdx() << ",child=" << child->getIdx() << "\n";
								assert(false);
							}
						}
					}
				}
				else if(node->getNameType()=="SELECTPHI"){
					if(child->getNode()){
						written = true;
						outs() << "SELECTPHI :: " << node->getIdx();
						outs() << ",child = " << child->getIdx() << " | "; child->getNode()->dump();
						outs() << ",phiancestor = " << selectPHIAncestorMap[node]->getIdx() << " | "; selectPHIAncestorMap[node]->getNode()->dump();

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
							outs() << "node = " << node->getIdx() << ", child = " << child->getIdx() << "\n";
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

					if(Edge2OperandIdxMap.find(node) != Edge2OperandIdxMap.end()){
						if(Edge2OperandIdxMap[node].find(child) != Edge2OperandIdxMap[node].end()){
							found=true;
							if(Edge2OperandIdxMap[node][child] == 0){
								xmlFile << "type=\"P\"/>\n";
							}
							else if(Edge2OperandIdxMap[node][child] == 1){
								xmlFile << "type=\"I1\"/>\n";
							}
							else if(Edge2OperandIdxMap[node][child] == 2){
								xmlFile << "type=\"I2\"/>\n";
							}
							else{
								assert(child->getNode() && dyn_cast<PHINode>(child->getNode()));
								xmlFile << "type=\"I1\"/>\n";
								outs() << "since child is phi assigning I1 for node = " << node->getIdx() << ", child = " << child->getIdx() << "\n";
								// assert(false);
							}
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

int DFGBrMap::classifyParents() {

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
				Value* parentIns;



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

					assert(parent->getNameType().compare("CMERGE")==0 || parent->getNameType().compare("SELECTPHI")==0);
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

	return 0;
}

void DFGBrMap::assignALAPasASAP() {

	for(dfgNode* node : NodeList){
		node->setASAPnumber(node->getALAPnumber());
	}

}

void DFGBrMap::addOrphanPseudoEdges() {



	std::set<dfgNode*> RecParents;

	for(dfgNode* node : NodeList){
		for(dfgNode* recParent : node->getRecAncestors()){
			RecParents.insert(recParent);
		}
	}

	for(dfgNode* node : RecParents){

		outs() << "Searching for recParent=" << node->getIdx() << "\n";

//		for(dfgNode* parCand : NodeList){
//			if(parCand->getASAPnumber() == node->getALAPnumber()-1 && parCand->getALAPnumber() == node->getALAPnumber()-1){
//				outs() << "Adding Pseudo Connection :: parent=" << parCand->getIdx() << ",to" << node->getIdx() << "\n";
//				parCand->addChildNode(node,EDGE_TYPE_PS,false,true,true);
//				node->addAncestorNode(parCand,EDGE_TYPE_PS,false,true,true);
//				node->parentClassification[0]=parCand;
//				break;
//			}
//		}

		std::queue<std::set<dfgNode*>> q;
		std::set<dfgNode*> q_init;
		for(dfgNode* child : node->getChildren()){
			if(node->childBackEdgeMap[child]) continue;
			q_init.insert(child);
		}
		q.push(q_init);

		dfgNode* criticalChild = NULL;
		int startdiff = node->getALAPnumber() - node->getASAPnumber();
		dfgNode* critCand = NULL;

		while(!q.empty()){
			std::set<dfgNode*> curr = q.front(); q.pop();
			std::set<dfgNode*> q_next;
			bool critChildFound=false;
			for(dfgNode* n : curr){
				if(n->getASAPnumber() == n->getALAPnumber()){
					criticalChild=n;
					critChildFound=true;
					startdiff=0;
					break;
				}

				int currDiff = n->getALAPnumber() - n->getASAPnumber();
				if(currDiff < startdiff){
					startdiff = currDiff;
					critCand = n;
				}

				for(dfgNode* child : n->getChildren()){
					if(n->childBackEdgeMap[child]) continue;
					q_next.insert(child);
				}
			}
			if(critChildFound) break;

			if(!q_next.empty()) q.push(q_next);
		}

		while(!q.empty()) q.pop();

		if(!criticalChild){
			if(!critCand){
				outs() << "NO critChild and NO critCand! continue...\n";
				continue;
			}
			outs() << "No critChild , but critCand = " << critCand->getIdx()  << ",with startDiff = " << startdiff << "\n";
			criticalChild = critCand;
//			for(dfgNode* n : NodeList){
//				if(n == node) continue;
//				if(n->getALAPnumber() == node->getALAPnumber() - 1){
//
//					dfgNode* pseduoParent = n;
//					outs() << "Adding Pseudo Connection :: parent=" << pseduoParent->getIdx() << ",to" << node->getIdx() << "\n";
//					pseduoParent->addChildNode(node,EDGE_TYPE_PS,false);
//					node->addAncestorNode(pseduoParent,EDGE_TYPE_PS,false);
//				}
//			}
//			continue;
		}

		assert(criticalChild);
		q_init.clear();
		q_init.insert(criticalChild);
		q.push(q_init);

		dfgNode* pseduoParent = NULL;
		while(!q.empty()){
			std::set<dfgNode*> curr = q.front(); q.pop();
			std::set<dfgNode*> q_next;
			bool pseudoParentFound=false;
			for(dfgNode* n : curr){
//				if(n->getASAPnumber() == n->getALAPnumber()){
//					if(n->getALAPnumber() == node->getALAPnumber()){
					if(node->getALAPnumber() - n->getALAPnumber() == startdiff){
						pseduoParent=n;
						outs() << "Adding Pseudo Connection :: parent=" << pseduoParent->getIdx() << ",to" << node->getIdx() << "\n";
						pseduoParent->addChildNode(node,EDGE_TYPE_PS,false,true,true);
						node->addAncestorNode(pseduoParent,EDGE_TYPE_PS,false,true,true);
						pseudoParentFound=true;
//						break;
					}
//				}
				for(dfgNode* parent : n->getAncestors()){
					if(parent->childBackEdgeMap[n]) continue;
					if(parent == node) continue;
					q_next.insert(parent);
				}
			}
//			if(pseudoParentFound) break;

			if(!q_next.empty()) q.push(q_next);
		}

		assert(pseduoParent);

//		outs() << "Adding Pseudo Connection :: parent=" << pseduoParent->getIdx() << ",to" << node->getIdx() << "\n";
//		pseduoParent->addChildNode(node,EDGE_TYPE_PS,false,true,true);
//		node->addAncestorNode(pseduoParent,EDGE_TYPE_PS,false,true,true);


//		std::map<int,dfgNode*> asapchild;
//		for(dfgNode* child : node->getChildren()){
//			if(node->childBackEdgeMap[child]) continue;
//			asapchild[child->getASAPnumber()]=child;
//		}
//
//		assert(!asapchild.empty());
//		dfgNode* earliestChild = (*asapchild.begin()).second;
//
//		std::map<int,dfgNode*> asapcousin;
//		for(dfgNode* parent : earliestChild->getAncestors()){
//			if(parent == node) continue;
//			if(parent->childBackEdgeMap[earliestChild]) continue;
//			asapcousin[parent->getASAPnumber()]=parent;
//		}
//
//		if(!asapcousin.empty()){
//			dfgNode* latestCousin = (*asapcousin.rbegin()).second;
//
//			outs() << "Adding Pseudo Connection :: parent=" << latestCousin->getIdx() << ",to" << node->getIdx() << "\n";
//			latestCousin->addChildNode(node,EDGE_TYPE_PS,false,true,true);
//			node->addAncestorNode(latestCousin,EDGE_TYPE_PS,false,true,true);
//			node->parentClassification[0]=latestCousin;
//		}


	}
//	assert(false);
	scheduleASAP();
	scheduleALAP();



}

dfgNode* DFGBrMap::addLoadParent(Value* ins, dfgNode* child) {
	dfgNode* temp;
	if(OutLoopNodeMap.find(ins) == OutLoopNodeMap.end()){
		temp = new dfgNode(this);
		temp->setNameType("OutLoopLOAD");
		temp->setIdx(getNodesPtr()->size());
		temp->BB = child->BB;
		getNodesPtr()->push_back(temp);
		OutLoopNodeMap[ins] = temp;
		OutLoopNodeMapReverse[temp]=ins;

		temp->setArrBasePtr(ins->getName().str());
		DataLayout DL = temp->BB->getParent()->getParent()->getDataLayout();
		array_pointer_sizes[ins->getName().str()] = DL.getTypeAllocSize(ins->getType());

	  if(Instruction* real_ins = dyn_cast<Instruction>(ins)){		
			if(accumulatedBBs.find(real_ins->getParent())==accumulatedBBs.end()){
				temp->setTransferedByHost(true);
			}	
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

void DFGBrMap::printDOT(std::string fileName) {
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
				if(findEdge(node,child)->getType() == EDGE_TYPE_PS){
					ofs << "cyan";
				}
				else if(condition==true){
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

void DFGBrMap::connectBBTrig() {


	std::map<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> CtrlBBInfo = getCtrlInfoBBMorePaths();

	for(std::pair<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> p1 : CtrlBBInfo){
		BasicBlock* childBB = p1.first;
//		std::vector<dfgNode*> leaves = getLeafs(childBB);
		std::vector<dfgNode*> leaves = getStoreInstructions(childBB);

		for(std::pair<BasicBlock*,CondVal> p2 : p1.second){
			BasicBlock* parentBB = p2.first;
			CondVal parentVal = p2.second;

			BranchInst* BRI = cast<BranchInst>(parentBB->getTerminator());
			bool isConditional = BRI->isConditional();
			assert(isConditional);

			dfgNode* BR = findNode(BRI); assert(BR);

//			dfgNode* mergeNode;
			if(BR->getAncestors().size() != 1){
				assert(BR->getAncestors().size() == 2);
				outs() << "Br=" << BR->getIdx() << ",Ins=";
				BRI->dump();
				outs() << "Ancestor Count=" << BR->getAncestors().size() << "\n";
				outs() << "Ancestors=";
				for(dfgNode* par : BR->getAncestors()){
					outs() << par->getIdx() << ",BB=" << par->BB->getName() <<":";
					if(par->getNode()) par->getNode()->dump();
				}
			}
			assert(BR->getAncestors().size() == 1);
			BrParentMap[parentBB]=BR->getAncestors()[0];
			BrParentMapInv[BR->getAncestors()[0]]=parentBB;

			outs() << "ConnectBB :: From=" << BrParentMap[parentBB]->getIdx() << "(" << parentBB->getName() << ")" << ",To=";
			for(dfgNode* succNode : leaves){
				outs() << succNode->getIdx() << ",";
				assert(succNode->getNameType() != "OutLoopLOAD");
			}
			outs() << ",destBB = " << childBB->getName();
			outs() << "\n";

			BB2ControlParentMap[childBB].insert(std::make_pair(BR->getAncestors()[0],parentVal));

			for(dfgNode* BRParent : BR->getAncestors()){
				for(dfgNode* succNode : leaves){
					leafControlInputs[succNode].insert(BRParent);
	//				isBackEdge = checkBackEdge(node,succNode); //getCtrlInfoBB does not return backedges
//					if(!succNode->isParent(BRParent)){
						BRParent->addChildNode(succNode,EDGE_TYPE_DATA,false,isConditional,(parentVal==TRUE));
						succNode->addAncestorNode(BRParent,EDGE_TYPE_DATA,false,isConditional,(parentVal==TRUE));
//					}
				}
			}
		}
	}
}

void DFGBrMap::createCtrlBROrTree() {

	outs() << "createCtrlBROrTree STARTED!\n";

	for(dfgNode* node : NodeList){
		std::set<dfgNode*> predicateParents;
		for(dfgNode* par : node->getAncestors()){
			if(par->childConditionalMap[node] != UNCOND){
				predicateParents.insert(par);
			}
		}

		if(predicateParents.size() > 1){
			std::queue<dfgNode*> q;
			for(dfgNode* pp : predicateParents){
				q.push(pp);
			}
			while(!q.empty()){
				dfgNode* pp1 = q.front(); q.pop();
				if(!q.empty()){
					dfgNode* pp2 = q.front(); q.pop();

					outs() << "Connecting pp1 = " << pp1->getIdx() << ",";
					outs() << "pp2 = " << pp2->getIdx() << ",";


					dfgNode* temp = new dfgNode(this);
					temp->setIdx(5000 + NodeList.size());
					temp->setNameType("CTRLBrOR");
					temp->BB = pp1->BB;
					NodeList.push_back(temp);

					outs() << "newNode = " << temp->getIdx() << "\n";

					bool isPP1BackEdge = pp1->childBackEdgeMap[node];
					bool isPP2BackEdge = pp2->childBackEdgeMap[node];

					pp1->addChildNode(temp);
					temp->addAncestorNode(pp1);

					pp2->addChildNode(temp);
					temp->addAncestorNode(pp2);

					temp->addChildNode(node,EDGE_TYPE_DATA,isPP1BackEdge,true,pp1->childConditionalMap[node]);
					node->addAncestorNode(temp,EDGE_TYPE_DATA,isPP1BackEdge,true,pp1->childConditionalMap[node]);

					pp1->removeChild(node);
					node->removeAncestor(pp1);
					pp2->removeChild(node);
					node->removeAncestor(pp2);
					q.push(temp);
				}
				else{
					outs() << "Alone node = " << pp1->getIdx() << "\n";
				}

			}
		}
	}

	outs() << "createCtrlBROrTree ENDED!\n";

}

void DFGBrMap::createDualInsNodes() {


	for(BasicBlock* BB : this->currLoop->getBlocks()){
		if(BranchInst* BRI = dyn_cast<BranchInst>(BB->getTerminator())){
			if(BRI->isConditional()){
				BRI->dump();
				dfgNode* ctrlNode = BrParentMap[BB]; if(!ctrlNode) continue;
				outs() << "CTRLNODE = " << ctrlNode->getIdx() << "\n";

				outs() << "\tTrue Successor : " << BRI->getSuccessor(0)->getName() << ",";
				outs() << "False Successor : "<< BRI->getSuccessor(1)->getName() << ",";
				outs() << "\n";

				BasicBlock* BBT = BRI->getSuccessor(0);

				outs() << "\t\tTRUE Nodes : ";
				for(dfgNode* n : NodeList){
					if(n->BB == BBT){
						outs() << n->getIdx() << "(" << HyCUBEInsStrings[n->getFinalIns()];
						outs() << ",AS=" << n->getASAPnumber() << ",AL=" << n->getALAPnumber();
						outs() << "),";
					}
				}
				outs() << "\n";


				BasicBlock* BBF = BRI->getSuccessor(1);

				outs() << "\t\tFALSE Nodes : ";
				for(dfgNode* n : NodeList){
					if(n->BB == BBF){
						outs() << n->getIdx() << "(" << HyCUBEInsStrings[n->getFinalIns()];
						outs() << ",AS=" << n->getASAPnumber() << ",AL=" << n->getALAPnumber();
						outs() << "),";
					}
				}
				outs() << "\n";


			}
		}
	}

//	assert(false);


	std::queue<dfgNode*> lq, rq;
	typedef std::pair<std::set<dfgNode*>,std::set<dfgNode*>> LRSet;

	std::map<dfgNode*,LRSet> select_LR_sets;

	std::map<dfgNode*,LRSet> ctrlDepChilds;

	for(dfgNode* node : NodeList){
		if(node->getNameType() == "CMERGE"){
			dfgNode* ctrlNode = cmergeCtrlInputs[node]; assert(ctrlNode);
			CondVal condition = node->ancestorConditionaMap[ctrlNode];
			if(condition == TRUE){
//				assert(ctrlDepChilds[ctrlNode].first == NULL);
				ctrlDepChilds[ctrlNode].first.insert(node);
			}
			else if(condition == FALSE){
//				assert(ctrlDepChilds[ctrlNode].second == NULL);
				ctrlDepChilds[ctrlNode].second.insert(node);
			}
			else{
				assert(false);
			}
		}
	}


	for(std::pair <dfgNode*,LRSet> pair :  ctrlDepChilds){
		dfgNode* ctrlNode = pair.first;

		std::set<dfgNode*> trueNodes = pair.second.first;
		std::set<dfgNode*> falseNodes = pair.second.second;

		outs() << "CTRL = "  << ctrlNode->getIdx();
		if(!trueNodes.empty()){
			outs()  << ", TRU = ";
			for(dfgNode* truNode : trueNodes){
				outs() << truNode->getIdx() << ",";
			}
		}
		if(!falseNodes.empty()){
			outs() << ", FALSE = ";
			for(dfgNode* falseNode : falseNodes){
				outs() << falseNode->getIdx() << ",";
			}
		}
		outs() << "\n";
	}

//	assert(false);
}

void DFGBrMap::mergePHIParents() {

	outs() << "BasicBlock control domains begin :: \n";
	for(std::pair<BasicBlock*,std::set<std::pair<dfgNode*,CondVal>>> pair : BB2ControlParentMap){
		outs() << pair.first->getName() << ":";
		for(std::pair<dfgNode*,CondVal> nodeVal : pair.second){
			outs() << nodeVal.first->getIdx() << ":" << dfgNode::getCondValStr(nodeVal.second) << ",";
		}
		outs() << "\n";
	}
	outs() << "BasicBlock control domains end :: \n";

	std::set<dfgNode*> remNodes;
	std::map<dfgNode*,std::set<dfgNode*>> mergeNodesGlobal;
	std::map<dfgNode*,std::set<dfgNode*>> connectedTo;
	std::map<std::pair<dfgNode*,dfgNode*>,CondVal> connectedToCond;
	std::map<std::pair<dfgNode*,dfgNode*>,bool> connectedToBEdge;
	std::map<dfgNode*,std::set<dfgNode*>> connectedFrom;
	std::map<dfgNode*,dfgNode*> cmerges_with_data_nodes_global;

	for(std::pair<dfgNode*,std::set<dfgNode*>> pair : cmergeNodesPHI){
		dfgNode* phi = pair.first;
		std::set<dfgNode*> cmergeNodes = pair.second;

		dfgNode* prevNode=NULL;
		bool skip=false;
		for(dfgNode* cmerge : pair.second){
			if(prevNode){
				if(cmerge->getALAPnumber() != prevNode->getALAPnumber()){
					outs() << "ALAP mismatch! skipping this set of cmerge nodes!\n";
					skip=true;
					break;
				}
			}
			prevNode=cmerge;
		}
		if(skip) continue;

		// std::map<int,std::map<BasicBlock*,std::set<dfgNode*>>> mergeCandidateSets;
		std::map<int,std::map<dfgNode*,std::map<CondVal,std::set<dfgNode*>>>> mergeCandidateSets;
		std::map<dfgNode*,dfgNode*> cmerges_with_data_nodes;

		outs() << "PHI=" << phi->getIdx() << "\n";

		std::set<std::set<dfgNode*>> cmergePairs;
		std::set<dfgNode*> temp_pair;
		for(dfgNode* cmerge : pair.second){
				if(temp_pair.size() == 0){
					temp_pair.insert(cmerge);
				}
				else if(temp_pair.size() == 1){
					temp_pair.insert(cmerge);
					cmergePairs.insert(temp_pair);
					temp_pair.clear();
				}
				else{
					assert(false);
				}
		}

		assert(temp_pair.size() < 2);
		if(temp_pair.size() == 1) cmergePairs.insert(temp_pair);

		for(std::set<dfgNode*> cmerge_couple : cmergePairs){
			mergeCandidateSets.clear();
			cmerges_with_data_nodes.clear();
			for(dfgNode* cmerge : cmerge_couple){
				outs() << "\tCMERGE=" << cmerge->getIdx() << ",AS=" << cmerge->getASAPnumber() << ",AL=" << cmerge->getALAPnumber() << "\n";

				mergeCandidateSets[cmerge->getALAPnumber()][cmergeCtrlInputs[cmerge]][cmergeCtrlInputs[cmerge]->childConditionalMap[cmerge]].insert(cmerge);
				if(cmergeDataInputs[cmerge]){
					
					if(cmergeDataInputs[cmerge]->getChildren().size() == 1){
						cmerges_with_data_nodes[cmerge] = cmergeDataInputs[cmerge];
					}

					if(cmergeDataInputs[cmerge]->getNameType() == "OutLoopLOAD") continue;

					BasicBlock* cmergeDataBB = (BasicBlock*)cmergeDataInputs[cmerge]->BB;

					bool cmergeIsUniqueChild = true;
					for(dfgNode* cmergeDChild : cmergeDataInputs[cmerge]->getChildren()){
						//if(cmergeDChild != cmerge){
						if(cmergeNodes.find(cmergeDChild)==cmergeNodes.end()){
							cmergeIsUniqueChild = false; break;
						}
					}

					if(!cmergeIsUniqueChild){
						outs() << "cmerge is not an unique child!\n";
						continue;
					}

					std::queue<std::set<dfgNode*>> q;
					std::set<dfgNode*> qinit; qinit.insert(cmergeDataInputs[cmerge]);

					q.push(qinit);
					while(!q.empty()){
						std::set<dfgNode*> head = q.front(); q.pop();
						std::set<dfgNode*> qnext;
						outs() << "\t\t";
						for(dfgNode* hn : head){
							if(hn->BB == cmergeDataBB){
								outs() << "(" << hn->getIdx() << ",AS=" << hn->getASAPnumber() << ",AL=" << hn->getALAPnumber() <<  ")" << ",";
								// mergeCandidateSets[hn->getALAPnumber()][(BasicBlock*)hn->BB].insert(hn);
								for(std::pair<dfgNode*,CondVal> cv : BB2ControlParentMap[(BasicBlock*)hn->BB]){
									mergeCandidateSets[hn->getALAPnumber()][cv.first][cv.second].insert(hn);
								}

								for(dfgNode* hn_parent :  hn->getAncestors()){
									bool notUniqueChild=false;

									if(hn_parent->getNameType() == "OutLoopLOAD") continue;

									for(dfgNode* child : hn_parent->getChildren()){
										if(child != hn){
											notUniqueChild=true;
											break;
										}
									}
									if(notUniqueChild) continue;
									qnext.insert(hn_parent);
								}
							}
						}
						outs() << "\n";
						if(!qnext.empty()) q.push(qnext);
					}

				}

			}
			// std::map<int,dfgNode*> alap2merge;
			std::map<dfgNode*,std::set<dfgNode*>> mergeNodes;
	    std::map<dfgNode*,dfgNode*> reverseTrigMergeMap;

			std::map<dfgNode*,std::pair<dfgNode*,CondVal>> earliestMergeNodeMap;

			//this is section is being modified to support multiple nodes in the diferrent ALAP levels

			std::map<int,std::set<std::set<dfgNode*>>> alap2mergable_nodes;
			std::map<dfgNode*,std::pair<dfgNode*,CondVal>> mergeNodeCV;

			for(std::map<int,std::map<dfgNode*,std::map<CondVal,std::set<dfgNode*>>>>::reverse_iterator rit = mergeCandidateSets.rbegin() ; rit != mergeCandidateSets.rend() ; rit++){
				int ALAPNumber = rit->first;

				std::set<std::set<dfgNode*>> crossmergeInput;
				for(std::pair<dfgNode*,std::map<CondVal,std::set<dfgNode*>>> p1 : rit->second){
				//Need to create best set of merge nodes
					for(std::pair<CondVal,std::set<dfgNode*>> p2 : p1.second){
						crossmergeInput.insert(p2.second);
						for(dfgNode* n : p2.second){
							mergeNodeCV[n]=std::make_pair(p1.first,p2.first);
						}
					}
				}

				int mergesetsize = crossmergeInput.size();

				std::set<std::set<dfgNode*>> crossproductoutput = getCartesianProductNSets(crossmergeInput);

				outs() << "Sets begin.\n";
				for(std::set<dfgNode*> setA : crossproductoutput){
					for(dfgNode* n : setA){
						outs() << n->getIdx() << ",";
					}
					outs() << "commonAncCost=" << commonAncCost(setA);
					outs() << "\n";
				}
				outs() << "Sets end.\n";

				std::set<std::set<dfgNode*>> mergeSets;

				std::set<std::set<dfgNode*>> crossmergeInput_next;
				while(!crossmergeInput.empty()){
					crossmergeInput_next.clear();
					crossproductoutput = getCartesianProductNSets(crossmergeInput);
					std::set<dfgNode*> firstcrossset = *crossproductoutput.begin();
					mergeSets.insert(firstcrossset);


					for(std::set<dfgNode*> setIn : crossmergeInput){
						for(dfgNode* n : firstcrossset){
							setIn.erase(n);
						}
						if(!setIn.empty()) crossmergeInput_next.insert(setIn);
					}
					crossmergeInput = crossmergeInput_next;
				}

				for(std::set<dfgNode*> ms : mergeSets){
					outs() << "mergeSetBegin.\n";
					for(dfgNode* n : ms){
						outs() << n->getIdx() << ",";
					}
					outs() << "\n mergeSetEnd.\n";
				}

				alap2mergable_nodes[ALAPNumber] = mergeSets;
			}

			dfgNode* nextNode = phi;
			for(std::map<int,std::set<std::set<dfgNode*>>>::reverse_iterator rit = alap2mergable_nodes.rbegin(); rit != alap2mergable_nodes.rend(); rit++){
				int alap = rit->first;
				std::set<std::set<dfgNode*>> sets_of_merge_nodes = rit->second;

				for(std::set<dfgNode*> mergeset : sets_of_merge_nodes){
					dfgNode* temp = new dfgNode(this);
					temp->setIdx(8000 + NodeList.size()); NodeList.push_back(temp);
					temp->setNameType("TRIGMERGE");
					temp->BB = phi->BB;
					mergeNodes[temp] = mergeset;

					outs() << "trigmerge=" << temp->getIdx() << ":";

					for(dfgNode* n : mergeset){
						outs() << n->getIdx() << ",";
						reverseTrigMergeMap[n]=temp;
					}
					outs() << "\n";
				}
			}

			std::map<std::pair<dfgNode*,CondVal>,std::set<dfgNode*>> control2trigmerge;
			std::map<dfgNode*,std::set<std::pair<dfgNode*,CondVal>>> trigmerge2control;

			for(std::pair<dfgNode*,std::set<dfgNode*>> p1 : mergeNodes){
				dfgNode* trigmerge = p1.first;
				std::set<dfgNode*> repNodes = p1.second;

				for(dfgNode* rep : repNodes){
					for(dfgNode* child : rep->getChildren()){

						int operand_idx = -1;
						if(rep->getNameType() == "CMERGE" && child->getNode() && !dyn_cast<PHINode>(child->getNode())){
							operand_idx = findOperandNumber(child,child->getNode(),cmergePHINodes[rep]->getNode());
						}
						else{
							for(std::pair<int,dfgNode*> p2 : child->parentClassification){
								if(rep == p2.second){
									operand_idx = p2.first;
									break;
								}
							}
						}
						if(operand_idx == -1){
							outs() << "err :: src=" << rep->getIdx() << ",dest=" << child->getIdx() << "\n";
						}
						assert(operand_idx != -1);


						CondVal condition = rep->childConditionalMap[child];
						bool isConditional = condition != UNCOND;
						bool isBackEdge = rep->childBackEdgeMap[child];

						if(reverseTrigMergeMap[child]) child = reverseTrigMergeMap[child];
						Edge2OperandIdxMap[trigmerge][child]=operand_idx;

					  assert(trigmerge);
						connectedTo[trigmerge].insert(child);
						connectedFrom[child].insert(trigmerge);
						if(isConditional){
							connectedToCond[std::make_pair(trigmerge,child)]=condition;
						}
						connectedToBEdge[std::make_pair(trigmerge,child)]=isBackEdge;
					}

					for(dfgNode* anc : rep->getAncestors()){

						int operand_idx = -1;
						if(anc->getNameType() == "CMERGE" && rep->getNode() && !dyn_cast<PHINode>(rep->getNode())){
							operand_idx = findOperandNumber(rep,rep->getNode(),cmergePHINodes[anc]->getNode());
						}
						else{
							for(std::pair<int,dfgNode*> p2 : rep->parentClassification){
								if(anc == p2.second){
									operand_idx = p2.first;
									break;
								}
							}
							if(operand_idx == -1){
								outs() << "classification not found from=" << anc->getIdx() << ", to=" << rep->getIdx() << "\n";
							}
							assert(operand_idx != -1);
						}


						CondVal condition = anc->childConditionalMap[rep];
						bool isConditional = condition != UNCOND;
						bool isBackEdge = anc->childBackEdgeMap[rep];

						if(reverseTrigMergeMap[anc]) anc = reverseTrigMergeMap[anc];
						Edge2OperandIdxMap[anc][trigmerge]=operand_idx;


						if(isConditional){
							connectedToCond[std::make_pair(anc,trigmerge)]=condition;
							control2trigmerge[std::make_pair(anc,condition)].insert(trigmerge);
							assert(anc);
							trigmerge2control[trigmerge].insert(std::make_pair(anc,condition));
						}
						else{
							connectedTo[anc].insert(trigmerge);
							connectedFrom[trigmerge].insert(anc);
						}
						connectedToBEdge[std::make_pair(anc,trigmerge)]=isBackEdge;
					}
				}
			}

			for(std::pair<dfgNode*,std::set<dfgNode*>> p1 : mergeNodes){
				for(dfgNode* n : p1.second){
					control2trigmerge[mergeNodeCV[n]].insert(p1.first);
					assert(mergeNodeCV[n].first);
					trigmerge2control[p1.first].insert(mergeNodeCV[n]);

					// connectedTo[mergeNodeCV[n].first].insert(p1.first);
					// connectedFrom[p1.first].insert(mergeNodeCV[n].first);

					// Edge2OperandIdxMap[mergeNodeCV[n].first][p1.first]=0;
				}
			}

			for(std::pair<dfgNode*,std::set<dfgNode*>> p1 : mergeNodes){

				std::set<std::pair<dfgNode*,CondVal>> controlNodes = trigmerge2control[p1.first];
				std::queue<dfgNode*> q; q.push(p1.first);
				while(!q.empty()){
					dfgNode* head = q.front(); q.pop();
					for(dfgNode* child : connectedTo[head]){
						if(connectedToBEdge[std::make_pair(head,child)]) continue;
						if(child->getNameType() == "TRIGMERGE"){
							for(std::pair<dfgNode*,CondVal> cn : controlNodes){
								trigmerge2control[child].erase(cn);
								control2trigmerge[cn].erase(child);

								// connectedTo[cn].erase(child);
								// connectedFrom[child].erase(cn);
							}
						}
						q.push(child);
					}
				}

			}


			for(std::pair<dfgNode*,std::set<std::pair<dfgNode*,CondVal>>> p4 : trigmerge2control){
				dfgNode* trigmerge = p4.first;
				for(std::pair<dfgNode*,CondVal> cv : p4.second){
					assert(cv.first);
					connectedTo[cv.first].insert(trigmerge);
					connectedFrom[trigmerge].insert(cv.first);
					Edge2OperandIdxMap[cv.first][trigmerge]=0;
					connectedToCond[std::make_pair(cv.first,trigmerge)]=cv.second;
				}
			}

					//Merge data of cmerge
		for(std::pair<dfgNode*,std::set<dfgNode*>> mp : mergeNodes){
			std::set<dfgNode*> oldNodes = mp.second;
			for(dfgNode* on : oldNodes){
				if(cmerges_with_data_nodes[on]){
					//valid data input;
					dfgNode* cmerge_data = cmerges_with_data_nodes[on];
					outs() << "Removing Cmergedatainput = " << cmerge_data->getIdx() << "\n";

					if(reverseTrigMergeMap[cmerge_data]) cmerge_data = reverseTrigMergeMap[cmerge_data];
					for(dfgNode* anc : cmerge_data->getAncestors()){

						int operand_idx = -1;
						if(anc->getNameType() == "CMERGE" && cmerge_data->getNode() && !dyn_cast<PHINode>(cmerge_data->getNode())){
							operand_idx = findOperandNumber(cmerge_data,cmerge_data->getNode(),cmergePHINodes[anc]->getNode());
						}
						else{
							for(std::pair<int,dfgNode*> p2 : cmerge_data->parentClassification){
								if(anc == p2.second){
									operand_idx = p2.first;
									break;
								}
							}
							assert(operand_idx != -1);
						}

						connectedTo[anc].insert(mp.first);
						connectedFrom[mp.first].insert(anc);
						Edge2OperandIdxMap[anc][mp.first]=operand_idx;

						CondVal condition = anc->childConditionalMap[mp.first];
						connectedToCond[std::make_pair(anc,mp.first)]=condition;

					}

				}
			}
		}

			for(std::pair<dfgNode*,std::set<dfgNode*>> p2 : connectedTo){
				assert(p2.first);
				outs() << "src=" << p2.first->getIdx() << "|| dests=";
				for(dfgNode* dest : p2.second){
					outs() << dest->getIdx() << ",";
				}
				outs() << "\n";
			}

			outs() << "\n";

			//remove cmerge nodes
			// for(dfgNode* on : cmergeNodes){
			// 	for(dfgNode* anc : on->getAncestors()){
			// 		anc->removeChild(on);
			// 		on->removeAncestor(anc);
			// 	}
			// 	for(dfgNode* child : on->getChildren()){
			// 		on->removeChild(child);
			// 		child->removeAncestor(on);
			// 	}
			// }

			for(std::pair<dfgNode*,std::set<dfgNode*>> mp : mergeNodes){
				for(dfgNode* repNode : mp.second){
					mergeNodesGlobal[mp.first].insert(repNode);
				}
			}

			for(std::pair<dfgNode*,dfgNode*> pr : cmerges_with_data_nodes){
				cmerges_with_data_nodes_global[pr.first] = pr.second;
			}

		}
	}

	//add real connections based on the trigmerge
	for(std::pair<dfgNode*,std::set<dfgNode*>> p1 : connectedTo){
		dfgNode* srcNode = p1.first;
		for(dfgNode* dest : p1.second){
			std::pair<dfgNode*,dfgNode*> edge = std::make_pair(srcNode,dest);
			if(connectedToCond.find(edge) != connectedToCond.end()){

				if(connectedToBEdge.find(edge) != connectedToBEdge.end()){
					srcNode->addChildNode(dest,EDGE_TYPE_DATA,connectedToBEdge[edge],true,connectedToCond[edge]);
					dest->addAncestorNode(srcNode,EDGE_TYPE_DATA,connectedToBEdge[edge],true,connectedToCond[edge]);
				}
				else{
					srcNode->addChildNode(dest,EDGE_TYPE_DATA,false,true,connectedToCond[edge]);
					dest->addAncestorNode(srcNode,EDGE_TYPE_DATA,false,true,connectedToCond[edge]);
				}

			}
			else{
				if(connectedToBEdge.find(edge) != connectedToBEdge.end()){
					srcNode->addChildNode(dest,EDGE_TYPE_DATA,connectedToBEdge[edge],false);
					dest->addAncestorNode(srcNode,EDGE_TYPE_DATA,connectedToBEdge[edge],false);
				}
				else{
					srcNode->addChildNode(dest,EDGE_TYPE_DATA,false,false);
					dest->addAncestorNode(srcNode,EDGE_TYPE_DATA,false,false);
				}
			}
		}
	}

	//remove connections from old

	for(std::pair<dfgNode*,std::set<dfgNode*>> mp : mergeNodesGlobal){
		std::set<dfgNode*> oldNodes = mp.second;

		for(dfgNode* on : oldNodes){
			remNodes.insert(on);
			for(dfgNode* anc : on->getAncestors()){
				anc->removeChild(on);
				on->removeAncestor(anc);
			}
			for(dfgNode* child : on->getChildren()){
				on->removeChild(child);
				child->removeAncestor(on);
			}
		}
	}

	//removing cmergedata nodes that have cmerge as the only child
	for(std::pair<dfgNode*,dfgNode*> cmd : cmerges_with_data_nodes_global){
			dfgNode* on = cmd.second;
			if(on){
				remNodes.insert(on);
				for(dfgNode* anc : on->getAncestors()){
					anc->removeChild(on);
					on->removeAncestor(anc);
				}
				for(dfgNode* child : on->getChildren()){
					on->removeChild(child);
					child->removeAncestor(on);
				}
			}
	}

	for(dfgNode* rn : remNodes){
		outs() << "MERGEPHI : Removing node=" << rn->getIdx() << "\n";
		NodeList.erase(std::remove(NodeList.begin(), NodeList.end(), rn), NodeList.end());
	}


	nameNodes();
	scheduleASAP();
	scheduleALAP();

}

std::map<BasicBlock*, std::set<std::pair<BasicBlock*, CondVal> > > DFGBrMap::getCtrlInfoBBMorePaths() {


	//This function will return a map of basicblocks to their control dependent parents with the
	// respective control value

	SmallVector<std::pair<const BasicBlock *, const BasicBlock *>,8> BackedgeBBs;
	dfgNode firstNode = *NodeList[0];
	FindFunctionBackedges(*(firstNode.getNode()->getFunction()),BackedgeBBs);

	std::map<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> res;

	for(BasicBlock* BB : loopBB){

		std::queue<BasicBlock*> q;
		q.push(BB);

		std::map<BasicBlock*,std::set<CondVal>> visited;

		while(!q.empty()){
			BasicBlock* curr = q.front(); q.pop();
//			outs() << "curr=" << curr->getName() << "\n";

			for (auto it = pred_begin(curr), et = pred_end(curr); it != et; ++it)
			{
			  BasicBlock* predecessor = *it;
		      if(loopBB.find(predecessor)==loopBB.end()){
		    	  continue; // no need to care for out of loop BBs.
		      }

//		      if(predecessor == BB) continue;

		      std::pair<const BasicBlock*,const BasicBlock*> bbPair = std::make_pair((const BasicBlock*)predecessor,(const BasicBlock*)curr);
		      if(std::find(BackedgeBBs.begin(),BackedgeBBs.end(),bbPair)!=BackedgeBBs.end()){
		    	  continue; // no need to traverse backedges;
		      }

			  CondVal cv;
			  BranchInst* BRI = cast<BranchInst>(predecessor->getTerminator());
//			  BRI->dump();

			  if(!BRI->isConditional()){
//				  outs() << "\t0 :: ";
				  cv = UNCOND;
			  }
			  else{
				  for (int i = 0; i < BRI->getNumSuccessors(); ++i) {
					  if(BRI->getSuccessor(i) == curr){
						  if(i==0){
//							  outs() << "\t1 :: ";
							  cv = TRUE;
						  }
						  else if(i==1){
//							  outs() << "\t2 :: ";
							  cv = FALSE;
						  }
						  else{
							  assert(false);
						  }
					  }
				  }
			  }

//			  outs() << "\tPred=" << predecessor->getName() << "(" << dfgNode::getCondValStr(cv) << ")\n";

			  visited[predecessor].insert(cv);
			  q.push(predecessor);
			}
		}

		outs() << "BasicBlock : " << BB->getName() << " :: DependentCtrlBBs = ";
		res[BB];
		for(std::pair<BasicBlock*,std::set<CondVal>> pair : visited){
			BasicBlock* bb = pair.first;
			std::set<CondVal> brOuts = pair.second;


//			if(brOuts.size() == 1){
//				if(brOuts.find(UNCOND) == brOuts.end()){
					outs() << bb->getName();
//					if(brOuts.find(UNCOND) != brOuts.end()){
//						res[BB].insert(std::make_pair(bb,UNCOND));
//						outs() << "(UNCOND),";
//					}
					if(brOuts.find(TRUE) != brOuts.end()){
						res[BB].insert(std::make_pair(bb,TRUE));
						outs() << "(TRUE),";
					}
					if(brOuts.find(FALSE) != brOuts.end()){
						res[BB].insert(std::make_pair(bb,FALSE));
						outs() << "(FALSE),";
					}
//					res[BB].insert(std::make_pair(bb,cv));
//				}
//			}
		}
		outs() << "\n";
	}

	outs() << "###########################################################################\n";

	std::map<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> temp1;

	for(std::pair<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> pair : res){
		std::set<std::pair<BasicBlock*,CondVal>> tobeRemoved;
		BasicBlock* currBB = pair.first;
		outs() << "BasicBlock : " << currBB->getName() << " :: DependentCtrlBBs = ";

		temp1[currBB];
		for(std::pair<BasicBlock*,CondVal> bbVal : pair.second){
			BasicBlock* depBB = bbVal.first;
			for(std::pair<BasicBlock*,CondVal> p2 : res[depBB]){
				tobeRemoved.insert(p2);
			}
		}

		for(std::pair<BasicBlock*,CondVal> bbVal : pair.second){
			if(tobeRemoved.find(bbVal)==tobeRemoved.end()){
				outs() << bbVal.first->getName();
				outs() << "(" << dfgNode::getCondValStr(bbVal.second) << "),";
				temp1[currBB].insert(bbVal);
			}
		}
		outs() << "\n";
	}
//	return temp1;

	std::map<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> temp2;

	outs() << "Order=";
	for(std::pair<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> pair : temp1){
		BasicBlock* currBB = pair.first;
		outs() << currBB->getName() << ",";
		std::set<std::pair<BasicBlock*,CondVal>> bbValPairs = pair.second;

		std::queue<std::pair<BasicBlock*,CondVal>> bbValQ;
		for(std::pair<BasicBlock*,CondVal> bbVal: bbValPairs){
			bbValQ.push(bbVal);
		}

		bbValPairs.clear();
		int rounds = 0;
		while(!bbValQ.empty()){
			if(rounds == bbValQ.size()*2) break;
			std::pair<BasicBlock*,CondVal> topBBVal1 = bbValQ.front(); bbValQ.pop();
			if(!bbValQ.empty()){
				std::pair<BasicBlock*,CondVal> topBBVal2 = bbValQ.front();
				if(topBBVal1.first == topBBVal2.first){
					rounds=0;
					bbValQ.pop();
					if(!temp2[topBBVal1.first].empty()){
						bbValQ.push(*temp2[topBBVal1.first].rbegin());
					}
				}
				else{
					bbValQ.push(topBBVal1);
					rounds++;
//					break;
				}
			}else{
				bbValPairs.insert(topBBVal1);
			}
		}

		while(!bbValQ.empty()){
			bbValPairs.insert(bbValQ.front()); bbValQ.pop();
		}

		if(!bbValPairs.empty()) temp2[currBB]=bbValPairs;
	}
	outs() << "\n";

	outs() << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";

	temp1.clear();
	for(std::pair<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> pair : temp2){
		BasicBlock* currBB = pair.first;
		std::set<std::pair<BasicBlock*,CondVal>> bbValPairs = pair.second;
		outs() << "BasicBlock : " << currBB->getName() << " :: DependentCtrlBBs = ";
		for(std::pair<BasicBlock*,CondVal> bbVal: bbValPairs){
			outs() << bbVal.first->getName();
			outs() << "(" << dfgNode::getCondValStr(bbVal.second) << "),";
		}

		if(!bbValPairs.empty()){
			temp1[currBB] = temp2[currBB];
		}
		else{
			outs() << "Not added!";
		}
		outs() << "\n";
	}
//	assert(false);
	return temp1;

}

std::set<std::set<dfgNode*> > DFGBrMap::getCartesianProduct2Sets(std::set<std::set<dfgNode*>> a, std::set<dfgNode*> b) {

	std::set<std::set<dfgNode*>> res;
	for(std::set<dfgNode*> subset_a : a){
		for(dfgNode* node_b : b){
			std::set<dfgNode*> single_res = subset_a;
			single_res.insert(node_b);
			res.insert(single_res);
		}
	}

	return res;
}

std::set<std::set<dfgNode*> > DFGBrMap::getCartesianProductNSets(
		std::set<std::set<dfgNode*> > all_sets) {

	std::set<std::set<dfgNode*>> temp;

	for(dfgNode* n : *all_sets.begin()){
		std::set<dfgNode*> single_element_set;
		single_element_set.insert(n);
		temp.insert(single_element_set);
	}

	std::set<std::set<dfgNode*>>::iterator it = all_sets.begin();
	it++; //skipping 1 since its already allocated

	while(it != all_sets.end()){
		temp = getCartesianProduct2Sets(temp,*it);
		it++;
	}

	return temp;
}

int DFGBrMap::commonAncCost(std::set<dfgNode*> nodes) {

	std::set<dfgNode*> parents;
	for(dfgNode* n : nodes){
		for(dfgNode* par : n->getAncestors()){
			parents.insert(par);
		}
	}

	return parents.size();
}
