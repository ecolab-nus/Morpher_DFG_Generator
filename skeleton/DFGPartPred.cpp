#include "DFGPartPred.h"
#include "llvm/Analysis/CFG.h"
#include <algorithm>
#include <queue>
#include <map>
#include <set>
#include <vector>


// #include "llvm/Analysis/IVUsers.h"

using namespace std;

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

void DFGPartPred::removeOutLoopLoad() {
	outs() << "Removing outloop loads begin...\n";
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

	outs() << "Removing outloop loads end...\n";
}


std::vector<dfgNode*> DFGPartPred::getStoreInstructions(BasicBlock* BB) {

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
//				bool incomingCTRLVal;
				std::map<dfgNode*,bool> cnodectrlvalmap;

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
							// if(BRParentBRI->getSuccessor(0)==currBB){
							// 	incomingCTRLVal=true;
							// }
							// else if(BRParentBRI->getSuccessor(1)==currBB){
							// 	incomingCTRLVal=false;
							// }
							// else{
							// 	BRParentBRI->dump();
							// 	outs() << BRParentBRI->getSuccessor(0)->getName() << "\n";
							// 	outs() << BRParentBRI->getSuccessor(1)->getName() << "\n";
							// 	outs() << currBB->getName() << "\n";
							// 	outs() << previousCTRLNode->getIdx() << "\n";
							// 	assert(false);
							// }
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
//					else if(sizeArrMap.find(V->getName().str())!=sizeArrMap.end()){
//						int constant = sizeArrMap[V->getName().str()];
//						mergeNode = insertMergeNode(node,previousCTRLNode,incomingCTRLVal,constant);
//					}
					else if(Argument *ARG = dyn_cast<Argument>(V)){
						//FIXME : need cleaner way link the argument value when implementing the backend
						int constant = 0;
						mergeNode = insertMergeNode(node,previousCTRLNode,incomingCTRLVal,constant);
						PHIArgMap[node][V]=mergeNode;
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
			// if(PHI == currLoop->getCanonicalInductionVariable()){
			// 	recursivePhiNodes=true;
			// 	isInductionVar=true;
			// }

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

					if(!(isInductionVar && isBackEdge)){
						if(isInductionVar) {
							outs() << "this is induction variable, node=" << node->getIdx() << ",mergeNode=" << mergeNode->getIdx() << "\n";
							assert(isBackEdge == false);
						}
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

void DFGPartPred::generateTrigDFGDOT(Function &F) {

//	connectBB();
	removeAlloc();
	connectBBTrig();
	createCtrlBROrTree();

	handlePHINodes(this->loopBB);
	// insertshiftGEPs();
	addMaskLowBitInstructions();
	removeDisconnetedNodes();
//	scheduleCleanBackedges();
	fillCMergeMutexNodes();


	constructCMERGETree();

//	printDOT(this->name + "_PartPredDFG.dot"); return;
	handlestartstop();

	scheduleASAP();
	scheduleALAP();
	// assignALAPasASAP();
//	balanceSched();

	GEPBaseAddrCheck(F);
	nameNodes();
	classifyParents();
	// RemoveInductionControlLogic();

	// RemoveBackEdgePHIs();
	// removeOutLoopLoad();
	RemoveConstantCMERGEs();
	removeDisconnectedNodes();

	addOrphanPseudoEdges();
	addRecConnsAsPseudo();

	printDOT(this->name + "_PartPredDFG.dot");
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
		mutexSetCommonChildren[currMutexSet].insert(node);

		for(dfgNode* m1 : currMutexSet){
			for(dfgNode* m2 : currMutexSet){
				if(m1 == m2) continue;
				mutexNodes[m1].insert(m2);
			}
		}
	}

}

void DFGPartPred::constructCMERGETree() {
	// assert(!mutexSets.empty());

	//if there are no mutex sets no point creating the cmerge tree
	if(mutexSets.empty()) return;

	for(std::set<dfgNode*> cmergeSet : mutexSets){
		std::queue<dfgNode*> thisIterSet;
		std::queue<dfgNode*> nextIterSet;

		outs() << "CMERGE(ThisIter) set =";
		for(dfgNode* cmergeNode : cmergeSet){
			bool isBackEdge=false;
			outs() << "CMERGE Tree children = "; 
			for(dfgNode* child : mutexSetCommonChildren[cmergeSet]){
				outs() << child->getIdx();
				if(isBackEdge) assert(cmergeNode->childBackEdgeMap[child] == true);
				if(cmergeNode->childBackEdgeMap[child]){
					isBackEdge = true;
					outs() << "[BE]";
				}
				outs() << ",";
			}
			outs() << "\n";

			if(mutexSetCommonChildren[cmergeSet].size() == 1){
				dfgNode* singleChild = *mutexSetCommonChildren[cmergeSet].begin();
				if(singleChild->getNode() && dyn_cast<PHINode>(singleChild->getNode())){
					realphi_as_selectphi.insert(singleChild);
					outs() << "Only single child is a phi node, hence marking it as a SELECTPHI\n";
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

void DFGPartPred::scheduleASAP() {

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

void DFGPartPred::scheduleALAP() {

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


	std::string fileName = name + "_PartPred_DFG.xml";
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

			if(node->getArrBasePtr() != "NOT_A_MEM_OP"){
				xmlFile << "<BasePointerName size=\"" << array_pointer_sizes[node->getArrBasePtr()] << "\">";	
				xmlFile << node->getArrBasePtr();
				xmlFile << "</BasePointerName>\n";
			}

			if(node->getGEPbaseAddr() != -1){
				GetElementPtrInst* GEP = cast<GetElementPtrInst>(node->getNode());
				int gep_offset = GEPOffsetMap[GEP];
				xmlFile << "<GEPOffset>";	
				xmlFile << gep_offset;
				xmlFile << "</GEPOffset>\n";
			}

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
								child->parentClassification[0]=node;
								if(child->getNPB()){
									xmlFile << "NPB=\"1\" ";
								}
								else{
									xmlFile << "NPB=\"0\" ";
								}
								xmlFile << "type=\"P\"/>\n";
							}
							else if ( operand_no == 1){
								child->parentClassification[1]=node;
								xmlFile << "type=\"I1\"/>\n";
							}
							else if(( operand_no == 2)){
								child->parentClassification[2]=node;
								xmlFile << "type=\"I2\"/>\n";
							}
							else{
								outs() << "node=" << node->getIdx() << ",child=" << child->getIdx() << "\n";
								assert(false);
							}
						}
					}
				}
				else if(node->getNameType()=="SELECTPHI" && (child != selectPHIAncestorMap[node])){
					if(child->getNode()){
						written = true;
						outs() << "SELECTPHI :: " << node->getIdx();
						outs() << ",child = " << child->getIdx() << " | "; child->getNode()->dump();
						outs() << ",phiancestor = " << selectPHIAncestorMap[node]->getIdx() << " | "; selectPHIAncestorMap[node]->getNode()->dump();

						int operand_no = findOperandNumber(child,child->getNode(),selectPHIAncestorMap[node]->getNode());
						if( operand_no == 0){
							child->parentClassification[0]=node;
								if(child->getNPB()){
									xmlFile << "NPB=\"1\" ";
								}
								else{
									xmlFile << "NPB=\"0\" ";
								}
							xmlFile << "type=\"P\"/>\n";
						}
						else if ( operand_no == 1){
							child->parentClassification[1]=node;
							xmlFile << "type=\"I1\"/>\n";
						}
						else if(( operand_no == 2)){
							child->parentClassification[2]=node;
							xmlFile << "type=\"I2\"/>\n";
						}
						else{
							outs() << "node = " << node->getIdx() << ", child = " << child->getIdx() << "\n";
							assert(false);
						}
					}
				}

				if(Edge2OperandIdxMap.find(node) != Edge2OperandIdxMap.end() 
					 && Edge2OperandIdxMap[node].find(child) != Edge2OperandIdxMap[node].end()){
						int operand_no = Edge2OperandIdxMap[node][child];
						if( operand_no == 0){
							child->parentClassification[0]=node;
								if(child->getNPB()){
									xmlFile << "NPB=\"1\" ";
								}
								else{
									xmlFile << "NPB=\"0\" ";
								}
							xmlFile << "type=\"P\"/>\n";
						}
						else if ( operand_no == 1){
							child->parentClassification[1]=node;
							xmlFile << "type=\"I1\"/>\n";
						}
						else if(( operand_no == 2)){
							child->parentClassification[2]=node;
							xmlFile << "type=\"I2\"/>\n";
						}
						written = true;
				}

				if(written){
				}
				else if(child->parentClassification[0]==node){
								if(child->getNPB()){
									xmlFile << "NPB=\"1\" ";
								}
								else{
									xmlFile << "NPB=\"0\" ";
								}
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
					if(node->getNameType().compare("SELECTPHI")==0 || (realphi_as_selectphi.find(node) != realphi_as_selectphi.end())){
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

				if(node->ancestorConditionaMap[parent] != UNCOND){
					node->parentClassification[0] = parent;
				}
				else{
					//Only ins!=NULL and non-PHI and non-BR will reach here
					node->parentClassification[findOperandNumber(node, ins,parentIns)]=parent;
				}


		}
	}

}

void DFGPartPred::assignALAPasASAP() {

	for(dfgNode* node : NodeList){
		node->setASAPnumber(node->getALAPnumber());
	}

}

void DFGPartPred::addRecConnsAsPseudo() {

	scheduleASAP();
	scheduleALAP();

	for(dfgNode* node : NodeList){
		for(dfgNode* recParent : node->getRecAncestors()){
			outs() << "Adding rec Pseudo Connection :: parent=" << recParent->getIdx() << ",to" << node->getIdx() << "\n";
			removeEdge(findEdge(recParent,node));
			recParent->addChildNode(node,EDGE_TYPE_PS,false,true,true);
			node->addAncestorNode(recParent,EDGE_TYPE_PS,false,true,true);
		}
	}

	scheduleASAP();
	scheduleALAP();

}

void DFGPartPred::addOrphanPseudoEdges() {

	scheduleASAP();
	scheduleALAP();

	std::set<dfgNode*> RecParents;
	std::map<dfgNode*,int> maxDelayASAP;

	for(dfgNode* node : NodeList){
		for(dfgNode* recParent : node->getRecAncestors()){
			outs() << "child = " << node->getIdx() << ", recparent = " << recParent->getIdx() << "\n";
			if(recParent->getALAPnumber() == recParent->getASAPnumber()) continue;
			RecParents.insert(recParent);
		}

		for(dfgNode* child : node->getChildren()){
			if(node->childBackEdgeMap[child]){
				if(child->getALAPnumber() == child->getASAPnumber()) continue;
				RecParents.insert(child);
			}
		}
	}

	for(dfgNode* n : RecParents){
		maxDelayASAP[n]=10000000;
		for(dfgNode* child : n->getChildren()){
			if(n->childBackEdgeMap[child]) continue;
			if(child->getASAPnumber() < maxDelayASAP[n]){
				maxDelayASAP[n] = child->getASAPnumber();
			}
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
					outs() << "n=" << n->getIdx() << ",diff=" << n->getALAPnumber() - n->getASAPnumber() << "\n";
					if(n->getALAPnumber() - n->getASAPnumber() <= startdiff && 
					   n->getALAPnumber() < node->getALAPnumber() &&
						 (n->getASAPnumber() < maxDelayASAP[node] - 1)
						){
						pseduoParent=n;
						
						std::vector<dfgNode*> node_childs = node->getChildren();

						if(std::find(node_childs.begin(),node_childs.end(),pseduoParent) == node_childs.end()
						   && !node->isParent(pseduoParent)){
							//pseudoparent is not a child of the node
							outs() << "Adding Pseudo Connection :: parent=" << pseduoParent->getIdx() << ",to" << node->getIdx() << "\n";
							pseduoParent->addChildNode(node,EDGE_TYPE_PS,false,true,true);
							node->addAncestorNode(pseduoParent,EDGE_TYPE_PS,false,true,true);

							pseudoParentFound=true;
							break;
						}
					}
//				}
				for(dfgNode* parent : n->getAncestors()){
					if(parent->childBackEdgeMap[n]) continue;
					if(parent == node) continue;
					q_next.insert(parent);
				}
			}
			if(pseudoParentFound) break;

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

dfgNode* DFGPartPred::addLoadParent(Value* ins, dfgNode* child) {
	dfgNode* temp;
	if(OutLoopNodeMap.find(ins) == OutLoopNodeMap.end()){
		temp = new dfgNode(this);
		temp->setNameType("OutLoopLOAD");
		temp->setIdx(getNodesPtr()->size());
		temp->BB = child->BB;
		getNodesPtr()->push_back(temp);
		OutLoopNodeMap[ins] = temp;
		OutLoopNodeMapReverse[temp]=ins;

		temp->setArrBasePtr(ins->getName());
		DataLayout DL = temp->BB->getParent()->getParent()->getDataLayout();
		array_pointer_sizes[ins->getName()] = DL.getTypeAllocSize(ins->getType());

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

void DFGPartPred::connectBBTrig() {


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

void DFGPartPred::createCtrlBROrTree() {

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

std::map<BasicBlock*, std::set<std::pair<BasicBlock*, CondVal> > > DFGPartPred::getCtrlInfoBBMorePaths() {


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
			  assert(predecessor->getTerminator());
			  predecessor->getTerminator()->dump();
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

int findParentClassification(dfgNode* par, dfgNode* child){

	if(child->parentClassification[0]==par){
		return 0;
	}
	else if(child->parentClassification[1]==par){
		return 1;
	}
	else if(child->parentClassification[2]==par){
		return 2;
	}

	return -1;
}



void DFGPartPred::RemoveInductionControlLogic() {
	
	// ScalarEvolution* SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();

	PHINode* ind_phi = currLoop->getInductionVariable(*SE);
	if(!ind_phi){
		// 'cannot find the induction variable, probably the increments in loop induction variables are not 1'
		assert(false);
	}

	dfgNode* ind_phi_node = findNode(ind_phi);
	assert(ind_phi_node);

	std::vector<dfgNode*> phiChildren = ind_phi_node->getChildren();
	std::map<dfgNode*,int> phiChildParIdxMap;

	//Ind phi node should have two parents
	// assert(ind_phi_node->getAncestors().size() == 2);
	int anc_count = 0;
	for(dfgNode* par : ind_phi_node->getAncestors()){
		Edge* e = findEdge(par,ind_phi_node);
		if(e->getType() == EDGE_TYPE_PS){
			continue;
		}
		std::cout << "induction phi parents :: \n";
		std::cout << par->getIdx() << ",";
		anc_count++;
	}
	assert(anc_count == 2);


	dfgNode* ind_phi_init_par = NULL;
	dfgNode* ctrlNode = NULL;
	dfgNode* dataNode = NULL;
	dfgNode* cmerge = NULL;

	for(dfgNode* par : ind_phi_node->getAncestors()){
		if(ind_phi_node->ancestorBackEdgeMap[par]){
			
			//backedge cmerge
			assert(par->getNameType() == "CMERGE");
			cmerge = par;
			
			ctrlNode = cmergeCtrlInputs[par];  assert(ctrlNode);
			dataNode = cmergeDataInputs[par];	 assert(dataNode);

			std::vector<dfgNode*> dParents = dataNode->getAncestors();

			//Parent of the data of cmerge should be phi
			assert(std::find(dParents.begin(),dParents.end(),ind_phi_node) != dParents.end());

			//removing parents of ctrl
			for(dfgNode* cPar : ctrlNode->getAncestors()){
				cPar->removeChild(ctrlNode);
				ctrlNode->removeAncestor(cPar);
			}

			//removing parents of cmerge
			for(dfgNode* cmPar : par->getAncestors()){
				cmPar->removeChild(par);
				par->removeAncestor(cmPar);
			}
		}
		else{
			ind_phi_init_par = par;
			ind_phi_init_par->removeChild(ind_phi_node);
			ind_phi_node->removeAncestor(ind_phi_init_par);
		}
	}

	for(dfgNode* phiC : ind_phi_node->getChildren()){
			int par_class = findParentClassification(ind_phi_node,phiC);
			assert(par_class!=-1);
			phiChildParIdxMap[phiC]=par_class;

			ind_phi_node->removeChild(phiC);
			phiC->removeAncestor(ind_phi_node);
	}

	assert(ctrlNode);
	assert(dataNode);
	assert(ind_phi_init_par);
	assert(cmerge);

	//Adding new connections
	ind_phi_init_par->addChildNode(dataNode,EDGE_TYPE_DATA);
	dataNode->addAncestorNode(ind_phi_init_par,EDGE_TYPE_DATA);

	int phi_init_class = findParentClassification(ind_phi_init_par,ind_phi_node);
	assert(phi_init_class != -1);
	dataNode->parentClassification[phi_init_class]=ind_phi_init_par;

	//add its own self as a recurrent edge
	//FIXME : hardcoding the recurrent edge as parent 2
	dataNode->addChildNode(dataNode,EDGE_TYPE_DATA,true);
	dataNode->addAncestorNode(dataNode,EDGE_TYPE_DATA,true);
	Edge2OperandIdxMap[dataNode][dataNode]=phi_init_class;

	for(dfgNode* phiChild : phiChildren){
		if(phiChild == dataNode) continue;
		dataNode->addChildNode(phiChild,EDGE_TYPE_DATA);
		phiChild->addAncestorNode(dataNode,EDGE_TYPE_DATA);
		phiChild->parentClassification[phiChildParIdxMap[phiChild]]=dataNode;
	}



	//remove nodes
	NodeList.erase(std::remove(NodeList.begin(), NodeList.end(), cmerge), NodeList.end());
	NodeList.erase(std::remove(NodeList.begin(), NodeList.end(), ind_phi_node), NodeList.end());
}


void DFGPartPred::RemoveBackEdgePHIs() {
	cout << "removebackedge phis begin...\n";

	for(dfgNode* n : NodeList){
		if(n->getNode()){
			if(PHINode* phiNode = dyn_cast<PHINode>(n->getNode())){
				
				if(n->getAncestors().size() != 2) continue;

				cout << "processing phi node = " << n->getIdx();

				dfgNode* normal_anc = NULL;
				dfgNode* backedge_anc = NULL;

				for(dfgNode* anc : n->getAncestors()){
					if(n->ancestorBackEdgeMap[anc]){
						assert(backedge_anc == NULL);
						backedge_anc = anc;
					}
					else{
						assert(normal_anc == NULL);
						normal_anc = anc;
					}
				}

				cout << ",BE_anc = " << backedge_anc->getIdx() << ",normal_anc = " << normal_anc->getIdx() << "\n";

				//process non-back edge node
				assert(normal_anc->getNameType() == "CMERGE");
				if(normal_anc->hasConstantVal()){
					for(dfgNode* phiC : n->getChildren()){

						cout << "adding child:" << phiC->getIdx() << ",to:" << normal_anc->getIdx() << "\n";
						normal_anc->addChildNode(phiC,EDGE_TYPE_DATA);
						phiC->addAncestorNode(normal_anc,EDGE_TYPE_DATA);
						Edge2OperandIdxMap[normal_anc][phiC] = findParentClassification(n,phiC);

						if(phiC->getNameType() == "CMERGE") cmergeDataInputs[phiC]=normal_anc;

						// phiC->parentClassification[findParentClassification(n,phiC)]=normal_anc;
					}
					normal_anc->removeChild(n);
					n->removeAncestor(normal_anc);
				}
				else{
					dfgNode* ctrlNode = cmergeCtrlInputs[normal_anc]; assert(ctrlNode);
					// dfgNode* dataNode = cmergeDataInputs[normal_anc]; assert(dataNode);

					// dfgNode* ctrlNode = normal_anc->parentClassification[0]; assert(ctrlNode);
					// dfgNode* dataNode = dataNode->parentClassification[1]; assert(dataNode);

					vector<dfgNode*> dataParents = normal_anc->getAncestors();
					for(dfgNode* dataParent : dataParents){
							if(dataParent == ctrlNode) continue;
							if(findEdge(dataParent,normal_anc)->getType() == EDGE_TYPE_PS) continue;
							bool isBackEdge = dataParent->childBackEdgeMap[normal_anc];

							for(dfgNode* phiC : n->getChildren()){
								cout << "adding child:" << phiC->getIdx() << ",to:" << dataParent->getIdx() << "\n";
								dataParent->addChildNode(phiC,EDGE_TYPE_DATA,isBackEdge);
								phiC->addAncestorNode(dataParent,EDGE_TYPE_DATA,isBackEdge);
								Edge2OperandIdxMap[dataParent][phiC] = findParentClassification(n,phiC);

								if(phiC->getNameType() == "CMERGE") cmergeDataInputs[phiC]=dataParent;

								dataParent->removeChild(normal_anc);
								normal_anc->removeAncestor(dataParent);

								// phiC->parentClassification[findParentClassification(n,phiC)]=dataNode;
							}
					}



					// dataNode->removeChild(normal_anc);
					// normal_anc->removeAncestor(dataNode);

					ctrlNode->removeChild(normal_anc);
					normal_anc->removeAncestor(ctrlNode);

					normal_anc->removeChild(n);
					n->removeAncestor(normal_anc);
				}

				//process backedge
				if(backedge_anc->hasConstantVal()){
					for(dfgNode* phiC : n->getChildren()){

						cout << "adding (with backedge) child:" << phiC->getIdx() << ",to:" << backedge_anc->getIdx() << "\n";
						backedge_anc->addChildNode(phiC,EDGE_TYPE_DATA,true);
						phiC->addAncestorNode(backedge_anc,EDGE_TYPE_DATA,true);
						Edge2OperandIdxMap[backedge_anc][phiC] = findParentClassification(n,phiC);

						if(phiC->getNameType() == "CMERGE") cmergeDataInputs[phiC]=backedge_anc;
						// phiC->parentClassification[findParentClassification(n,phiC)]=backedge_anc;

						std::unordered_set<dfgNode*> lineage = getLineage(backedge_anc);
						if(lineage.find(phiC) == lineage.end() && !backedge_anc->getChildren().empty()){
							std::vector<dfgNode*> dChlds = backedge_anc->getChildren();
							dfgNode* nonbedge_child = NULL;
							for(dfgNode* chld : dChlds){
								std::vector<dfgNode*> next_childs = chld->getChildren();
								if(!backedge_anc->childBackEdgeMap[chld] &&
								    std::find(next_childs.begin(),next_childs.end(),phiC) == next_childs.end()){
										nonbedge_child = chld; break;
								}
							}
							
								if(nonbedge_child){
									cout << "adding (with pseudo) child:" << phiC->getIdx() << ",to:" << nonbedge_child->getIdx() << "\n";
									nonbedge_child->addChildNode(phiC,EDGE_TYPE_PS,false,true,true);
									phiC->addAncestorNode(nonbedge_child,EDGE_TYPE_PS,false,true,true);
								}
							
						}

					}
					backedge_anc->removeChild(n);
					n->removeAncestor(backedge_anc);
				}
				else{
					dfgNode* ctrlNode = cmergeCtrlInputs[backedge_anc]; assert(ctrlNode);
					// dfgNode* dataNode = cmergeDataInputs[backedge_anc]; assert(dataNode);

					vector<dfgNode*> dataParents = backedge_anc->getAncestors();
					for(dfgNode* dataParent : dataParents){
						if(dataParent == ctrlNode) continue;
						if(findEdge(dataParent,backedge_anc)->getType() == EDGE_TYPE_PS) continue;

						bool isBackEdge = dataParent->childBackEdgeMap[normal_anc];
						for(dfgNode* phiC : n->getChildren()){
							cout << "adding (with backedge) child:" << phiC->getIdx() << ",to:" << dataParent->getIdx() << "\n";
							dataParent->addChildNode(phiC,EDGE_TYPE_DATA,true);
							phiC->addAncestorNode(dataParent,EDGE_TYPE_DATA,true);
							Edge2OperandIdxMap[dataParent][phiC] = findParentClassification(n,phiC);

							if(phiC->getNameType() == "CMERGE") cmergeDataInputs[phiC]=dataParent;
							// phiC->parentClassification[findParentClassification(n,phiC)]=dataNode;

							std::unordered_set<dfgNode*> lineage = getLineage(dataParent);
							if(lineage.find(phiC) == lineage.end() && !dataParent->getChildren().empty()){
								std::vector<dfgNode*> dChlds = dataParent->getChildren();
								dfgNode* nonbedge_child = NULL;
								for(dfgNode* chld : dChlds){
									std::vector<dfgNode*> next_childs = chld->getChildren();
									if(!dataParent->childBackEdgeMap[chld] &&
											std::find(next_childs.begin(),next_childs.end(),phiC) == next_childs.end()){
											nonbedge_child = chld; break;
									}
								}

								if(nonbedge_child){
									cout << "adding (with pseudo) child:" << phiC->getIdx() << ",to:" << nonbedge_child->getIdx() << "\n";
									nonbedge_child->addChildNode(phiC,EDGE_TYPE_PS,false,true,true);
									phiC->addAncestorNode(nonbedge_child,EDGE_TYPE_PS,false,true,true);
								}

							}
						}

						dataParent->removeChild(backedge_anc);
						backedge_anc->removeAncestor(dataParent);

					}


					// dfgNode* ctrlNode = normal_anc->parentClassification[0]; assert(ctrlNode);
					// dfgNode* dataNode = dataNode->parentClassification[1]; assert(dataNode);

					

					// dataNode->removeChild(backedge_anc);
					// backedge_anc->removeAncestor(dataNode);

					ctrlNode->removeChild(backedge_anc);
					backedge_anc->removeAncestor(ctrlNode);

					backedge_anc->removeChild(n);
					n->removeAncestor(backedge_anc);
				}

				for(dfgNode* phiC : n->getChildren()){
					n->removeChild(phiC);
					phiC->removeAncestor(n);
				}


			}
		}
	}

	cout << "removebackedge phis done.!\n";
}

void DFGPartPred::RemoveConstantCMERGEs(){

	for(dfgNode* n : NodeList){
		if(n->getNameType() == "CMERGE" && n->hasConstantVal()){
			dfgNode* ctrlNode = cmergeCtrlInputs[n];
			bool ctrlVal = n->ancestorConditionaMap[ctrlNode];

			vector<dfgNode*> cmChilds = n->getChildren();

			for(dfgNode* c : cmChilds){

				bool onlyParentCMERGE=true;
				for(dfgNode* par : c->getAncestors()){
					if(par != n && !c->ancestorBackEdgeMap[par]){
						onlyParentCMERGE=false;
					}
				}

				if(onlyParentCMERGE){
					ctrlNode->addChildNode(c,EDGE_TYPE_CTRL,false,true,ctrlVal);
					c->addAncestorNode(ctrlNode,EDGE_TYPE_CTRL,false,true,ctrlVal);
					Edge2OperandIdxMap[ctrlNode][c]=0;
				}

				n->removeChild(c);
				c->removeAncestor(n);
			}

			ctrlNode->removeChild(n);
			n->removeAncestor(ctrlNode);
		}
	}


}


void DFGPartPred::getLoopExitConditionNodes(std::set<exitNode> &exitNodes)
{
	bool is_ctrl_node_null = false;
	std::unordered_set<BasicBlock *> exitSrcs;

	for (auto it = loopexitBB.begin(); it != loopexitBB.end(); it++)
	{

		BasicBlock *src = it->first;
		BasicBlock *dest = it->second;
		exitSrcs.insert(src);

		outs() << "loop exit src = " << src->getName() << ", dest = " << dest->getName() << ",";

		BranchInst *BRI = static_cast<BranchInst *>(src->getTerminator());

		if (BRI->isConditional())
		{
			dfgNode *ctrlNode = findNode(cast<Instruction>(BRI->getCondition()));
			assert(ctrlNode);
			for (int j = 0; j < BRI->getNumSuccessors(); ++j)
			{
				if (dest == BRI->getSuccessor(j))
				{
					if (j == 0)
					{
						outs() << "true path\n";
						// exitNodes.insert(std::make_pair(ctrlNode, true));
						exitNodes.insert(exitNode(ctrlNode, true, BRI->getParent(), dest));
					}
					else
					{
						outs() << "false path\n";
						exitNodes.insert(exitNode(ctrlNode, false, BRI->getParent(), dest));
					}
				}
			}
		}
		else
		{
			outs() << "BRI BasicBlock = " << BRI->getParent()->getName() << "\n";

			auto ThisBasicBlockDTNode = DT->getNode(BRI->getParent());
			auto IDOM_DTNode = ThisBasicBlockDTNode->getIDom();
			BasicBlock *IDOM = IDOM_DTNode->getBlock();

			BranchInst *IDOM_BRI = static_cast<BranchInst *>(IDOM->getTerminator());
			outs() << "IDOM BRI BasicBlock = " << IDOM_BRI->getParent()->getName() << "\n";
			assert(IDOM_BRI->isConditional());

			bool isTruePath = true;
			for (int j = 0; j < IDOM_BRI->getNumSuccessors(); ++j)
			{
				if (DT->dominates(IDOM_BRI->getSuccessor(j), BRI->getParent()))
				{
					if (j == 0)
					{
						outs() << "true path\n";
						// exitNodes.insert(std::make_pair(ctrlNode, true));
						isTruePath = true;
					}
					else
					{
						outs() << "false path\n";
						isTruePath = false;
					}
				}
			}

			dfgNode *ctrlNode = findNode(cast<Instruction>(IDOM_BRI->getCondition()));

			//ctrlNode could be null that means it is always executed
			exitNodes.insert(exitNode(ctrlNode, isTruePath, BRI->getParent(), dest));

			if (ctrlNode)
			{
			}
			else
			{
				is_ctrl_node_null = true;
			}
		}
	}

	if (is_ctrl_node_null)
	{
		assert(exitSrcs.size() == 1);
	}
}

// void DFGPartPred::addLoopExitStoreHyCUBE(std::set<exitNode> &exitNodes)
// {
// 	outs() << "addLoopExitStoreHyCUBE begin.\n";
// 	for (exitNode en : exitNodes)
// 	{

// 		dfgNode *loopexit = new dfgNode(this);
// 		loopexit->setIdx(this->getNodesPtr()->size() + 20000);
// 		this->getNodesPtr()->push_back(loopexit);
// 		loopexit->setNameType("LOOPEXIT");
// 		loopexit->BB = en.dest;
// 		loopexit->setLeftAlignedMemOp(1);

// 		dfgNode *mov_const = new dfgNode(this);
// 		mov_const->setIdx(this->getNodesPtr()->size() + 20000);
// 		this->getNodesPtr()->push_back(mov_const);
// 		mov_const->setNameType("MOVC");
// 		mov_const->BB = loopexit->BB;

// 		int mu_trans_id = getMUnitTransID(en.src, en.dest);
// 		mov_const->setConstantVal(mu_trans_id);

// 		mov_const->addChildNode(loopexit);
// 		loopexit->addAncestorNode(mov_const);
// 		loopexit->parentClassification[1] = mov_const;
// 		outs() << "Adding connection : " << mov_const->getIdx() << " to " << loopexit->getIdx() << "\n";

// 		if (en.ctrlNode)
// 		{
// 			en.ctrlNode->getNode()->dump();
// 			bool isTruePath = en.ctrlVal;
// 			en.ctrlNode->addChildNode(loopexit, EDGE_TYPE_DATA, false, true, isTruePath);
// 			loopexit->addAncestorNode(en.ctrlNode, EDGE_TYPE_DATA, false, true, isTruePath);
// 			loopexit->parentClassification[0] = en.ctrlNode;
// 			if (!isTruePath)
// 				loopexit->setNPB(true);
// 			outs() << "Adding connection : " << en.ctrlNode->getIdx() << " to " << loopexit->getIdx() << "\n";
// 		}
// 		else
// 		{
// 			dfgNode *sample_start_node = startNodes.begin()->second;
// 			assert(sample_start_node);
// 			sample_start_node->addChildNode(loopexit, EDGE_TYPE_PS, false, true, true);
// 			loopexit->addAncestorNode(sample_start_node, EDGE_TYPE_PS, false, true, true);

// 			outs() << "Adding connection PS : " << sample_start_node->getIdx() << " to " << loopexit->getIdx() << "\n";
// 		}
// 	}

// 	outs() << "addLoopExitStoreHyCUBE end.\n";

// 	// for(auto it = exitNodes.begin(); it != exitNodes.end(); it++){
// 	// 	dfgNode* control_node = it->first;
// 	// 	bool isTruePath = it->second;

// 	// 	node->addChildNode(loopexit,EDGE_TYPE_DATA,false,true,isTruePath);
// 	// 	loopexit->addAncestorNode(node,EDGE_TYPE_DATA,false,true,isTruePath);

// 	// 	//There is a problem here, we cannot have multiple nodes coming to the same store
// 	// 	loopexit->parentClassification[0] = node;
// 	// 	if(!isTruePath) loopexit->setNPB(true);
// 	// }
// }