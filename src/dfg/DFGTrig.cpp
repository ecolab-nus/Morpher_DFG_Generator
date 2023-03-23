#include <morpherdfggen/dfg/DFGTrig.h>

#include "llvm/Analysis/CFG.h"
#include <algorithm>
#include <queue>
#include <map>
#include <set>
#include <vector>
#include <unordered_set>

void DFGTrig::connectBB() {

	std::map<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> CtrlBBInfo = getCtrlInfoBBMorePaths();

	std::map<BasicBlock*,std::set<BasicBlock*>> mBBs = checkMutexBBs();

	for(std::pair<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> p1 : CtrlBBInfo){
		BasicBlock* childBB = p1.first;

		std::vector<dfgNode*> leaves;
		if(mBBs.find(childBB) != mBBs.end() && !mBBs[childBB].empty()){
			leaves = getLeafs(childBB);
		}
		else{
			leaves = getStoreInstructions(childBB);
		}


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



//
//	SmallVector<std::pair<const BasicBlock *, const BasicBlock *>,8> BackedgeBBs;
//	dfgNode firstNode = *NodeList[0];
//	FindFunctionBackedges(*(firstNode.getNode()->getFunction()),BackedgeBBs);
//
//	for(dfgNode* node : NodeList){
//		if(node->getNode()==NULL) continue;
//		if(BranchInst* BRI = dyn_cast<BranchInst>(node->getNode())){
//			const BasicBlock* currBB = BRI->getParent();
//			for(int i=0; i<BRI->getNumSuccessors(); i++){
////			for(BasicBlock* succBB : this->loopBB){
//
//
//				const BasicBlock* succBB = BRI->getSuccessor(i);
//				std::pair<const BasicBlock *, const BasicBlock *> BBCouple = std::make_pair(currBB,succBB);
//
//				bool isBackEdge = false;
//				if(std::find(BackedgeBBs.begin(),BackedgeBBs.end(),BBCouple) != BackedgeBBs.end()){
//					isBackEdge = true;
//				}
//
//
//				bool isConditional = BRI->isConditional();
//				bool conditionVal = true;
//				if(isConditional){
//					if(i == 1){
//						conditionVal = false;
//					}
//				}
//
////				std::vector<dfgNode*> leaves = getStoreInstructions(BRI->getSuccessor(i)); //this doesnt include phinodes
//				std::vector<dfgNode*> leaves = getLeafs(BRI->getSuccessor(i));
//
//
//				if(node->getAncestors().size()!=1){
//					BRI->dump();
//					outs() << "BR node ancestor size = " << node->getAncestors().size() << "\n";
//				}
////				assert(node->getAncestors().size()==1); //should be a unique parent;
////				dfgNode* BRParent = node->getAncestors()[0];
//
//				for(dfgNode* BRParent : node->getAncestors()){
//
//					if(!isConditional){
//						const BasicBlock* BRParentBB = BRParent->BB;
//						const BranchInst* BRParentBRI = cast<BranchInst>(BRParentBB->getTerminator());
////						assert(BRParentBRI->isConditional());
////						if(BRParentBRI->getSuccessor(0)==currBB){
////							conditionVal=true;
////						}
////						else if(BRParentBRI->getSuccessor(1)==currBB){
////							conditionVal=false;
////						}
////						else{
////							BRParentBRI->dump();
////							outs() << BRParentBRI->getSuccessor(0)->getName() << "\n";
////							outs() << BRParentBRI->getSuccessor(1)->getName() << "\n";
////							outs() << currBB->getName() << "\n";
////							outs() << BRParent->getIdx() << "\n";
////							assert(false);
////						}
//						std::set<const BasicBlock*> searchSoFar;
//						assert(checkBRPath(BRParentBRI,currBB,conditionVal,searchSoFar));
//						isConditional=true; //this is always true. coz there is no backtoback unconditional branches.
//					}
//
//					outs() << "BR : ";
//					BRI->dump();
//					outs() << " and BRParent :" << BRParent->getIdx() << "\n";
//
//
//					outs() << "leafs from basicblock=" << currBB->getName() << " for basicblock=" << succBB->getName().str() << "\n";
//					for(dfgNode* succNode : leaves){
//						outs() << succNode->getIdx() << ",";
//					}
//					outs() << "\n";
//
//					for(dfgNode* succNode : leaves){
//						leafControlInputs[succNode].insert(BRParent);
//						isBackEdge = checkBackEdge(node,succNode);
//						BRParent->addChildNode(succNode,EDGE_TYPE_DATA,isBackEdge,isConditional,conditionVal);
//						succNode->addAncestorNode(BRParent,EDGE_TYPE_DATA,isBackEdge,isConditional,conditionVal);
//					}
//
//				}
//			}
//		}
//	}
//

}

std::vector<dfgNode*> DFGTrig::getStoreInstructions(BasicBlock* BB) {

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

					for(dfgNode* pcn : previousCtrlNodes) {
						outs() << "previousCTRLNode : "; pcn->getNode()->dump();
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
				recursivePhiNodes=true;
				isInductionVar=true;
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
	outs() << "inserting mergenode with non-constant data input\n";
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
	outs() << "cmerge=" << temp->getIdx() << ",ctrl=" << ctrl->getIdx() << ",data=" << data->getIdx() << "\n";
	return temp;
}

dfgNode* DFGTrig::insertMergeNode(dfgNode* PHINode, dfgNode* ctrl,
		bool controlVal, int val) {

	outs() << "inserting mergenode with constant data input\n";
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

	return 0;

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

//	if(srcBB == destBB) return true;

	if(std::find(BBSuccBasicBlocks[srcBB].begin(), BBSuccBasicBlocks[srcBB].end(), destBB) != BBSuccBasicBlocks[srcBB].end()){
		return false; //not a backedge
	}
	else{
		return true;
	}

}

void DFGTrig::generateTrigDFGDOT(Function &F) {

//	printDomTree();
//	getCtrlInfoBB();
	connectBB();
	handlePHINodes(this->loopBB);

//	insertshiftGEPs();
	addMaskLowBitInstructions();
	removeDisconnetedNodes();
//	scheduleCleanBackedges();
	fillCMergeMutexNodes();
//	constructCMERGETree();
	mirrorCtrlNodes();
	scheduleASAP();
	scheduleALAP();
//	assignALAPasASAP();
//	balanceSched();

	populateSubPathDFGs();

//		printDOT(this->name + "_TrigFullPredDFG.dot"); return;

	for(std::vector<std::pair<BasicBlock*,CondVal>> path : getPaths()){
		CreatSubDFGPath(path);
//		popCtrlTrees(path);
	}

	annotateNodesBr();
	mergeAnnotatedNodesBr();
//	annotateCtrlFrontierAsCtrlParent();


	printCtrlTree();
	removeNotCtrlConns();

	GEPBaseAddrCheck(F);
	nameNodes();
//	ConnectBrCtrls();
	printConstHist();
	removeOutLoopLoad();

	nameNodes();
	classifyParents();

	removeRedudantCtrl();
	removeCMERGEChildrenOpposingCtrl();

	removeCMERGEwithoutData();

	addRecConnsAsPseudo();
	addPseudoParentsRec();

	printSubPathDOT(this->name + "_TrigFullPredDFG");
	printDOT(this->name + "_TrigFullPredDFG.dot");

	printNewDFGXML();
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

void DFGTrig::scheduleCleanBackedges() {

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

void DFGTrig::fillCMergeMutexNodes() {

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

void DFGTrig::constructCMERGETree() {
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

void DFGTrig::scheduleASAP() {

	for(dfgNode* n : NodeList){
		n->setASAPnumber(-1);
	}

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

void DFGTrig::scheduleALAP() {

	for(dfgNode* n : NodeList){
		n->setALAPnumber(-1);
	}

	std::queue<std::vector<dfgNode*>> q;
	std::vector<dfgNode*> qv;

	assert(maxASAPLevel != 0);

	//initially fill this with childless nodes
	// outs() << "adding last level nodes : ";
	for(dfgNode* node : NodeList){
		int childCount = 0;
		for(dfgNode* child : node->getChildren()){
			if(node->childBackEdgeMap[child]) continue;
			childCount++;
		}
		if(childCount == 0){
			// outs() << node->getIdx() << ",";
			qv.push_back(node);
		}
	}
	// outs() << "\n";
	q.push(qv);

	int level = maxASAPLevel;
	std::set<dfgNode*> visitedNodes;

	// outs() << "Schedule ALAP ..... \n";
	// outs() << "maxASAP = " << maxASAPLevel << "\n";

	while(!q.empty()){
		assert(level >= 0);
		std::vector<dfgNode*> q_element = q.front(); q.pop();
		std::vector<dfgNode*> nq; nq.clear();

		for(dfgNode* node : q_element){
			visitedNodes.insert(node);

			if(node->getALAPnumber() > level || node->getALAPnumber() == -1){
				node->setALAPnumber(level);
				// outs() << "node=" << node->getIdx() << ",ALAP=" << level << "\n";
			}

			for(dfgNode* parent : node->getAncestors()){
				if(parent->childBackEdgeMap[node]) continue;
				nq.push_back(parent);
			}
		}

		if(!nq.empty()) q.push(nq);

		level--;
	}

	// outs() << "Cannot reach nodes = ";
	std::set<dfgNode*> doubles_count;
	if(visitedNodes.size() != NodeList.size()){
		for(dfgNode* n : NodeList){
			if(doubles_count.find(n) != doubles_count.end()){
				// outs() << "double node = " << n->getIdx() << "\n";
			}
			doubles_count.insert(n);

			if(visitedNodes.find(n) == visitedNodes.end()){
				// outs() << n->getIdx() << ",";
			}
		}
	}
	// outs() << ", over\n";
	// outs() << "visitied nodes size = " << visitedNodes.size() << ", NodeList size = " << NodeList.size() << "\n";

	assert(visitedNodes.size() == NodeList.size());
}

void DFGTrig::balanceSched() {

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

void DFGTrig::printNewDFGXML() {


	std::string fileName = name + "_TrigFullPred_DFG.xml";
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

	std::map<dfgNode*,std::string> originalBBModified = nodeBBModified;

	for(std::pair<BasicBlock*,std::set<BasicBlock*>> pair : mBBs){
		BasicBlock* first = pair.first;
		for(BasicBlock* second : pair.second){
			mBBs_str[first->getName().str()].insert(second->getName().str());
			mBBs_str[second->getName().str()].insert(first->getName().str());
		}
	}

	for(dfgNode* node : NodeList){
		int cmergeParentCount=0;
		std::set<std::string> mutexBBs;
		for(dfgNode* parent: node->getAncestors()){
			if(HyCUBEInsStrings[parent->getFinalIns()] == "CMERGE" || parent->getNameType() == "SELECTPHI"){
				nodeBBModified[parent]=nodeBBModified[parent]+ "_" + std::to_string(node->getIdx()) + "_" + std::to_string(cmergeParentCount);
				mutexBBs.insert(nodeBBModified[parent]);
				mBBs_str[nodeBBModified[parent]]=mBBs_str[originalBBModified[parent]];
				cmergeParentCount++;
			}
		}
//		std::cout << "mutex BBs :: begin \n";
		for(std::string bb_str1 : mutexBBs){
//			std::cout << bb_str1 << "=";
			for(std::string bb_str2 : mutexBBs){
				if(bb_str2==bb_str1) continue;
				std::cout << bb_str2 << ",";
				mBBs_str[bb_str1].insert(bb_str2);
				mBBs_str[bb_str2].insert(bb_str1);
			}
			std::cout << "\n";
		}
//		std::cout << "mutex BBs :: end \n";
	}



	std::map<std::string,std::pair<std::string,std::string>> sameControlDomainBBs;


	for(std::pair<dfgNode*,dfgNode*> p1 : nCMP2pCMP){
		dfgNode* negNode = p1.first;
		dfgNode* posNode = p1.second;
		std::cout << "POSNODE BB=" << nodeBBModified[posNode] << ",NEGNODE BB=" << nodeBBModified[negNode] << "\n";
		mBBs_str[nodeBBModified[posNode] + "_P"]=mBBs_str[nodeBBModified[posNode]];
		mBBs_str[nodeBBModified[negNode] + "_N"]=mBBs_str[nodeBBModified[negNode]];
	}

	for(dfgNode* pn : pCMPNodes){
		nodeBBModified[pn]=nodeBBModified[pn] + "_P";
	}

	for(std::pair<dfgNode*,dfgNode*> p1 : nCMP2pCMP){
		dfgNode* negNode = p1.first;
		dfgNode* posNode = p1.second;
		if(posNode){
			std::string posBB = nodeBBModified[posNode];

//			nodeBBModified[posNode]=nodeBBModified[posNode] + "_P";
			nodeBBModified[negNode]=nodeBBModified[negNode] + "_N";
			sameControlDomainBBs[posBB]=std::make_pair(nodeBBModified[posNode],nodeBBModified[negNode]);

			mBBs_str[nodeBBModified[posNode]].insert(nodeBBModified[negNode]);
			mBBs_str[nodeBBModified[negNode]].insert(nodeBBModified[posNode]);
		}

	}

//	assert(CtrlTrees.size()==1);
	for(TreeNode<BasicBlock*>* ctree : CtrlTrees){
		std::set<std::pair<TreeNode<BasicBlock*>*,bool>> allCtrlNodes = ctree->getAllNodes();
		for(std::pair<TreeNode<BasicBlock*>*,bool> p1 : allCtrlNodes){
			std::cout << "CTRL :: " << p1.first->getData()->getName().str() << "(" << p1.second << ")\n";

			std::string bb1Name = p1.first->getData()->getName().str();
			bool branch1Val = p1.second;

			if(branch1Val) bb1Name = bb1Name + "_P";
			if(!branch1Val) bb1Name = bb1Name + "_N";

			std::set<std::pair<TreeNode<BasicBlock*>*,bool>> mutexNodes = ctree->getMutexNodes(p1.first->getData(),p1.second);
			for(std::pair<TreeNode<BasicBlock*>*,bool> p2 : mutexNodes){

				bool branch2Val = p2.second;
				std::string bb2Name = p2.first->getData()->getName().str();
				std::cout << "\tMUTEX :: " << p2.first->getData()->getName().str() << "(" << p2.second << ")\n";

				if(branch2Val) bb2Name = bb2Name + "_P";
				if(!branch2Val) bb2Name = bb2Name + "_N";

				mBBs_str[bb1Name].insert(bb2Name);
				mBBs_str[bb2Name].insert(bb1Name);

			}
		}
	}
//	assert(false);


//	std::set<std::pair<TreeNode<BasicBlock*>*,bool>> leaves = CtrlTrees[0]->getLeafs();
//	for(std::pair<TreeNode<BasicBlock*>*,bool> p1 : leaves){
//		std::cout << "LEAF :: " << p1.first->getData()->getName().str() << "(" << p1.second << ")\n";
//		for(std::pair<TreeNode<BasicBlock*>*,bool> p2 : leaves){
//			if(p1 == p2) continue;
//			std::cout << "\tLEAF :: " << p2.first->getData()->getName().str() << "(" << p2.second << ")\n";
//			BasicBlock* bb1 = p1.first->getData();
//			bool branch1Val = p1.second;
//			std::string bb1Name = bb1->getName().str();
//
//			BasicBlock* bb2 = p2.first->getData();
//			bool branch2Val = p2.second;
//			std::string bb2Name = bb2->getName().str();
//
////			if(sameControlDomainBBs.find(bb1Name)!=sameControlDomainBBs.end()){
//				if(branch1Val) bb1Name = bb1Name + "_P";
//				if(!branch1Val) bb1Name = bb1Name + "_N";
////			}
////			else{
////				bb1Name = bb1Name + "_P";
////			}
//
////			if(sameControlDomainBBs.find(bb2Name)!=sameControlDomainBBs.end()){
//				if(branch2Val) bb2Name = bb2Name + "_P";
//				if(!branch2Val) bb2Name = bb2Name + "_N";
////			}
////			else{
////				bb2Name = bb2Name + "_P";
////			}
//
//			mBBs_str[bb1Name].insert(bb2Name);
//			mBBs_str[bb2Name].insert(bb1Name);
//		}
//	}
////	assert(false);


	xmlFile << "<Paths>\n";
//	for(std::pair<std::set<dfgNode*>,std::vector<std::pair<BasicBlock*,CondVal>> > p1 : subPathDFGMap){
//		std::vector<std::pair<BasicBlock*,CondVal>> path = p1.second;
	for(std::vector<std::pair<BasicBlock*,CondVal>> path : getPaths()){
		std::reverse(path.begin(),path.end());
		xmlFile << "\t<Path>\n";
			for(std::pair<BasicBlock*,CondVal> p2 : path){
				xmlFile << "\t\t<Step BB=\"" << p2.first->getName().str() << "\" ";
				xmlFile << "node =\"" << BrParentMap[p2.first]->getIdx() << "\" ";
				xmlFile << "value =\"" << dfgNode::getCondValStr(p2.second) << "\"/>\n";
			}
		xmlFile << "\t</Path>\n";
	}
	xmlFile << "</Paths>\n";


	xmlFile << "<MutexBB>\n";
//	for(std::pair<BasicBlock*,std::set<BasicBlock*>> pair : mBBs){
//		BasicBlock* first = pair.first;
//		xmlFile << "<BB1 name=\"" << first->getName().str() << "\">\n";
//		for(BasicBlock* second : pair.second){
//			if(sameControlDomainBBs.find(second->getName().str()) != sameControlDomainBBs.end()){
//				assert(false);
//				xmlFile << "\t<BB2 name=\"" << sameControlDomainBBs[second->getName().str()].first << "\"/>\n";
//				xmlFile << "\t<BB2 name=\"" << sameControlDomainBBs[second->getName().str()].second << "\"/>\n";
//			}
//			xmlFile << "\t<BB2 name=\"" << second->getName().str() << "\"/>\n";
//		}
//		xmlFile << "</BB1>\n";
//	}
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

		    if(pCMPNodes.find(node)!=pCMPNodes.end() || nCMP2pCMP[node]){
		    	xmlFile << " CTRL=\"1\"";
		    }
		    else{
		    	xmlFile << " CTRL=\"0\"";
		    }

			if(nCMP2pCMP[node]){
				xmlFile << " NEG=\"1\"";
			}
			else{
				xmlFile << " NEG=\"0\"";
			}


			xmlFile << ">\n";

			xmlFile << "<OP>";
			if((node->getNameType() == "OutLoopLOAD") || (node->getNameType() == "OutLoopSTORE") ){
				xmlFile << "O";
			}

			if(nCMP2pCMP[node]){
				xmlFile << HyCUBEInsStrings[nCMP2pCMP[node]->getFinalIns()] << "</OP>\n";
			}
			else{
				xmlFile << HyCUBEInsStrings[node->getFinalIns()] << "</OP>\n";
			}


			if(node->getArrBasePtr() != "NOT_A_MEM_OP"){
				xmlFile << "<BasePointerName size=\"" << array_pointer_sizes[node->getArrBasePtr()] << "\">";	
				xmlFile << node->getArrBasePtr();
				xmlFile << "</BasePointerName>\n";
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

				if(node->childConditionalMap[child] == UNCOND){
					xmlFile << " cond=\"UNCOND\" ";
				}
				else if(node->childConditionalMap[child] == TRUE){
					xmlFile << " cond=\"TRUE\" ";
				}
				else if(node->childConditionalMap[child] == FALSE){
					xmlFile << " cond=\"FALSE\" ";
				}
				else{
					assert(false);
				}

				bool written=false;

				if(leafControlInputs[child].find(node) != leafControlInputs[child].end()){
					written=true;
					xmlFile << "type=\"P\"/>\n";
				}

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
								assert(false);
							}
						}
					}
				}
				else if(node->getNameType()=="SELECTPHI"){
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

				if(edgeClassification.find(node)!=edgeClassification.end()){
					if(edgeClassification[node].find(child)!=edgeClassification[node].end()){
						if(edgeClassification[node][child]==0){
							xmlFile << "type=\"P\"/>\n";
							written=true;
						}
						else if(edgeClassification[node][child]==1){
							xmlFile << "type=\"I1\"/>\n";
							written=true;
						}
						else if(edgeClassification[node][child]==2){
							xmlFile << "type=\"I2\"/>\n";
							written=true;
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

			//BelongstoBR
			xmlFile << "<BelongsToBr>\n";
			for(std::pair<BasicBlock*,CondVal> p1 : node->BelongsToBr){
				xmlFile << "\t<Br name=\"" << p1.first->getName().str() << "\" ";
				xmlFile << "node=\"" << BrParentMap[p1.first]->getIdx() << "\" ";
				xmlFile << "value=\"" << dfgNode::getCondValStr(p1.second) << "\"/>\n";
			}
			xmlFile << "</BelongsToBr>\n";


			xmlFile << "</Node>\n\n";

	}
//		}
//	}




	xmlFile << "</DFG>\n";
	xmlFile.close();




}

int DFGTrig::classifyParents() {

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


				if(leafControlInputs[node].find(parent) != leafControlInputs[node].end()){
					node->parentClassification[0]=parent;
					continue;
				}


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
					if(node->getNameType().compare("N_CMP")==0){
						ins=nCMP2pCMP[node]->getNode(); assert(ins);
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
						edgeClassification[parent][node]=1;
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
					if(parent->getNameType().compare("N_CMP")==0){
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
						dfgNode* repNode = node;
						if(nCMP2pCMP[node]){
							repNode = nCMP2pCMP[node];
						}

						outs() << "CMERGE Parent" << parent->getIdx() << "to non-nametype child=" << node->getIdx() << ".\n";
						outs() << "parentIns = "; parentIns->dump();
						outs() << "node = "; repNode->getNode()->dump();

						assert(repNode->getNode()); assert(repNode->getNode() == ins);
						outs() << "OpNumber = " << findOperandNumber(repNode, ins,parentIns) << "\n";

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

				dfgNode* repNode = node;
				if(nCMP2pCMP[node]){
					repNode = nCMP2pCMP[node];
				}

				int op_no = findOperandNumber(repNode, ins,parentIns);
				node->parentClassification[op_no]=parent;

				edgeClassification[parent][node] = op_no;

				outs() << "CLASSIFY PARENTS :: node=" << node->getIdx();
				outs() << ",parent=" << parent->getIdx();
				outs() << ",operand_no=" << op_no << "\n";
		}
	}

	return 0;
}

void DFGTrig::assignALAPasASAP() {

	for(dfgNode* node : NodeList){
		node->setASAPnumber(node->getALAPnumber());
	}

}

void DFGTrig::printDomTree() {
	assert(!this->loopBB.empty());
	for(BasicBlock* bb : loopBB){
		BasicBlock* idomBB = DT->getNode(bb)->getIDom()->getBlock();
		outs() << "BasicBlock : " << bb->getName().str() << " ::";
		outs() << " IDom = " << idomBB->getName() << "\n";
	}
	assert(false);
}

std::set<BasicBlock*> DFGTrig::getBBsWhoHasIDomAs(BasicBlock* bb) {
	std::set<BasicBlock*>  res;

	assert(!this->loopBB.empty());
	for(BasicBlock* B : loopBB){
		BasicBlock* idomBB = DT->getNode(B)->getIDom()->getBlock();
		outs() << "BasicBlock : " << bb->getName().str() << " ::";
		outs() << " IDom = " << idomBB->getName() << "\n";

		if(idomBB == bb){
			res.insert(B);
		}
	}

	return res;
}

std::map<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> DFGTrig::getCtrlInfoBB() {

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

	std::map<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> temp2;

	outs() << "Order=";
	for(std::pair<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> pair : temp1){
		BasicBlock* currBB = pair.first;
		std::set<std::pair<BasicBlock*,CondVal>> bbValPairs = pair.second;

		outs() << currBB->getName() << ",";

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
					if(!temp1[topBBVal1.first].empty()){
						bbValQ.push(*temp1[topBBVal1.first].rbegin());
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
		temp2[currBB]=bbValPairs;
	}
	outs() << "\n";

	outs() << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";

	for(std::pair<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> pair : temp2){
		BasicBlock* currBB = pair.first;
		std::set<std::pair<BasicBlock*,CondVal>> bbValPairs = pair.second;
		outs() << "BasicBlock : " << currBB->getName() << " :: DependentCtrlBBs = ";
		for(std::pair<BasicBlock*,CondVal> bbVal: bbValPairs){
			outs() << bbVal.first->getName();
			outs() << "(" << dfgNode::getCondValStr(bbVal.second) << "),";
		}
		outs() << "\n";
	}
//	assert(false);
	return temp2;
}

std::map<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> DFGTrig::getCtrlInfoBBMorePaths() {

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

std::vector<dfgNode*> DFGTrig::getLeafs(BasicBlock* BB) {


	errs() << "start getting the LeafNodes...!\n";
	std::vector<dfgNode*> leafNodes;
	for (int i = 0 ; i < NodeList.size() ; i++) {
		dfgNode* node = NodeList[i];
		if(node->BB == BB){

			if(node->getNode()!=NULL){
				if(dyn_cast<PHINode>(node->getNode())){
					continue;
				}
			}

			if(node->getNameType().compare("OutLoopLOAD")==0){

			}
			else{
				leafNodes.push_back(node);
			}

//			for(dfgNode* parent : node->getAncestors()){
//				if(parent->getNameType() == "OutLoopLOAD"){
//					if(parent->getAncestors().empty()){
//						leafNodes.push_back(parent);
//					}
//				}
//			}




		}
	}
	errs() << "LeafNodes init done...!\n";

	for (int i = 0 ; i < NodeList.size() ; i++) {
//		if(NodeList[i]->getNode()->getParent() == BB){
		if(NodeList[i]->BB == BB){

			if(NodeList[i]->getNameType().compare("OutLoopLOAD")==0){
				continue;
			}

			for(int j = 0; j < NodeList[i]->getChildren().size(); j++){
				dfgNode* nodeToBeRemoved = NodeList[i]->getChildren()[j];
				if(nodeToBeRemoved != NULL){
//					errs() << "LeafNodes : nodeToBeRemoved found...! : ";
					if(nodeToBeRemoved->getNode() == NULL){
//						errs() << "NodeIdx:" << nodeToBeRemoved->getIdx() << "," << nodeToBeRemoved->getNameType() << "\n";
					}
					else{
//						errs() << "NodeIdx:" << nodeToBeRemoved->getIdx() << ",";
//						nodeToBeRemoved->getNode()->dump();
					}

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

void DFGTrig::popCtrlTrees(std::vector<std::pair<BasicBlock*,CondVal>> path) {

	assert(!path.empty());
	std::reverse(path.begin(),path.end());

	TreeNode<BasicBlock*>* root;
	bool found=false;
	for(TreeNode<BasicBlock*>* ctrl : CtrlTrees){
		if(ctrl->getData() == path[0].first){
			root = ctrl;
			found = true;
			break;
		}
	}

	if(!found){
		root = new TreeNode<BasicBlock*>(path[0].first);
		CtrlTrees.push_back(root);
	}

	for (int i = 1; i < path.size(); ++i) {
		outs() << "INSERT CHILD : " << path[i].first->getName() << ", PARENT = " << path[i-1].first->getName() << "\n";
		root->insertChild(path[i].first,path[i-1].first,(path[i-1].second==TRUE));
	}

//	assert(CtrlTrees.size()==1);
}

void DFGTrig::printCtrlTree() {
//	assert(CtrlTrees.size() == 1);

	for(TreeNode<BasicBlock*>* ctree : CtrlTrees){
		std::cout << "CTREE=" << ctree->getData()->getName().str() << ",BEGIN!\n";
		std::queue<std::set<TreeNode<BasicBlock*>*>> q;
		std::set<TreeNode<BasicBlock*>*> q_init; q_init.insert(ctree);
		q.push(q_init);

		outs() << "PRINTING CTRL TREE :::\n";
		while(!q.empty()){
			std::set<TreeNode<BasicBlock*>*> front = q.front(); q.pop();
			std::set<TreeNode<BasicBlock*>*> q_new;

			for(TreeNode<BasicBlock*>* tn : front){
				outs() << tn->getData()->getName() << ",";

				std::set<TreeNode<BasicBlock*>*> lefts = tn->getLefts();
				std::set<TreeNode<BasicBlock*>*> rights = tn->getRights();

				for(TreeNode<BasicBlock*>* left : lefts){
					q_new.insert(left);
				}

				for(TreeNode<BasicBlock*>* right : rights){
					q_new.insert(right);
				}
//				if(tn->getLeft()) q_new.insert(tn->getLeft());
//				if(tn->getRight()) q_new.insert(tn->getRight());
			}
			outs() << "\n";
			if(!q_new.empty()) q.push(q_new);
		}
	}
//	assert(false);
}

void DFGTrig::annotateNodesBr() {
	for(std::pair<std::set<dfgNode*>,std::vector<std::pair<BasicBlock*,CondVal>> > p1 : subPathDFGMap){
		std::set<dfgNode*> subDFG = p1.first;
		std::vector<std::pair<BasicBlock*,CondVal>> path = p1.second;
		std::reverse(path.begin(),path.end());

		if(path.size() > longestPath.size()) longestPath = path;

		std::map<dfgNode*,std::pair<BasicBlock*,CondVal>> tempBelongBr;

		for(std::pair<BasicBlock*,CondVal> p11 : path){
			BasicBlock* ctrlBB = p11.first;
			std::set<std::pair<dfgNode*,CondVal>> steps;
			std::pair<dfgNode*,CondVal> step_pos = std::make_pair(BrParentMap[ctrlBB],p11.second);
			steps.insert(step_pos);

			if(pCMP2nCMP.find(BrParentMap[ctrlBB]) != pCMP2nCMP.end()){
				if(pCMP2nCMP[BrParentMap[ctrlBB]]){
					std::pair<dfgNode*,CondVal> step_neg = std::make_pair(pCMP2nCMP[BrParentMap[ctrlBB]],p11.second);
					steps.insert(step_neg);
				}
			}

			for(std::pair<dfgNode*,CondVal> p2 : steps){
//			BasicBlock* ctrlBB = p2.first;
			dfgNode* ctrlNode = p2.first;

			std::queue<dfgNode*> q; q.push(ctrlNode);

				while(!q.empty()){
					dfgNode* node = q.front(); q.pop();

					outs() << "annotateNodesBr :: Node=" << node->getIdx() << ',';
					for(dfgNode* child : node->getChildren()){
						bool isCondtional=false;
						bool condition=true;

						outs() << "\nChild=" << child->getIdx() << ',';
						if(subDFG.find(child) == subDFG.end()) continue;
						outs() << "inSUBDFG,";

						isCondtional= node->childConditionalMap[child] != UNCOND;
						condition = (node->childConditionalMap[child] == TRUE);

						bool isBackEdge = node->childBackEdgeMap[child];

						if(BrParentMapInv.find(node)!=BrParentMapInv.end()){
							BasicBlock* BrBB = BrParentMapInv[node];
							std::pair<BasicBlock*,CondVal> searchPair(BrBB,node->childConditionalMap[child]);

							if(std::find(path.begin(),path.end(),searchPair) == path.end()){
								continue;
							}
						}
						outs() << "inPath,";


						if(isBackEdge) continue;
						outs() << "notBackEdge,BelongsTo=" << ctrlBB->getName() << "(" << dfgNode::getCondValStr(p2.second) << ")";
//						tempBelongBr[child]=p2;
						tempBelongBr[child]=std::make_pair(ctrlBB,p2.second);
						q.push(child);
					}
					outs() << "\n";
				}

			}
		}

		for(std::pair<dfgNode*,std::pair<BasicBlock*,CondVal>> p3 : tempBelongBr){
			dfgNode* node = p3.first;
			node->BelongsToBr.insert(p3.second);
		}

	}
}

void DFGTrig::annotateCtrlFrontierAsCtrlParent() {

	for(std::pair<BasicBlock*,CondVal> p1 : longestPath){
		dfgNode* ctrlNode = BrParentMap[p1.first];
//		for(dfgNode* child : ctrlNode->getChildren()){
//			child->BelongsToBr = ctrlNode->BelongsToBr;
//		}

		std::map<std::set<dfgNode*>,std::vector<std::pair<BasicBlock*,CondVal>> > newsubPathDFGMap;
		for(std::pair<std::set<dfgNode*>,std::vector<std::pair<BasicBlock*,CondVal>>> p1 : subPathDFGMap){
			std::set<dfgNode*> subDFG = p1.first;
			std::vector<std::pair<BasicBlock*,CondVal>> path = p1.second;

			if(subDFG.find(ctrlNode)!=subDFG.end()){
				for(dfgNode* child : ctrlNode->getChildren()){
					subDFG.insert(child);
				}
			}
			newsubPathDFGMap[subDFG]=path;
		}
		subPathDFGMap = newsubPathDFGMap;
	}

}

void DFGTrig::removeOutLoopLoad() {

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

void DFGTrig::printConstHist() {

	std::map<int,std::set<dfgNode*>> byteConstHist;
	for(dfgNode* node : NodeList){
		if(node->hasConstantVal()){
			int val = node->getConstantVal();
			if(node->getNode()){
				if(dyn_cast<GetElementPtrInst>(node->getNode())){
					byteConstHist[2].insert(node);
					continue;
				}
			}
			else if(val < 128 && val >= -127){
				byteConstHist[1].insert(node);
			}
			else if(val < 32768 && val >= -32768){
				byteConstHist[2].insert(node);
			}
			else{
				byteConstHist[4].insert(node);
			}
		}
		else{
			if(node->getNameType() == "OutLoopLOAD"){
				byteConstHist[4].insert(node);
			}
		}

	}

	for(std::pair<int,std::set<dfgNode*>> p1 : byteConstHist){
		int byteCount = p1.first;
		int nodeCount = p1.second.size();
		std::cout << "BYTE=" << byteCount << ",NODES=" << nodeCount << "\n";
	}

	std::cout << "TOTAL NODES = " << NodeList.size() << "\n";
//	assert(false);
}

void DFGTrig::removeCMERGEData() {

	std::set<dfgNode*> nodeToRemove;

	for(dfgNode* node : NodeList){
		if(node->getNameType() == "CMERGE"){
			if(!node->hasConstantVal()){
				assert(node->getAncestors().size() == 2);
				assert(cmergeDataInputs.find(node)!=cmergeDataInputs.end());

				if(cmergeDataInputs[node]->BelongsToBr == node->BelongsToBr){
					// Data Input is in the same control domain
				}
			}
		}
	}

}

void DFGTrig::addPseudoParentsRec() {

	scheduleASAP();
	scheduleALAP();

	std::set<std::pair<dfgNode*,dfgNode*>> RecParents;

	for(dfgNode* node : NodeList){
		for(dfgNode* recParent : node->getRecAncestors()){
//			recParent->addChildNode(node,EDGE_TYPE_PS,false,true,true);
//			node->addAncestorNode(recParent,EDGE_TYPE_PS,false,true,true);
			RecParents.insert(std::make_pair(recParent,node));
		}

		for(dfgNode* child : node->getChildren()){
			if(node->childBackEdgeMap[child]){
				RecParents.insert(std::make_pair(child,node));
			}
		}

	}

	for(std::pair<dfgNode*,dfgNode*> p1 : RecParents){
		dfgNode* node = p1.first;
		dfgNode* recChild = p1.second;

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
				outs() << "NO critChild and NO critCand! assigning recChild...\n";
				criticalChild = recChild;
//				continue;
			}
			else{
				outs() << "No critChild , but critCand = " << critCand->getIdx()  << ",with startDiff = " << startdiff << "\n";
				criticalChild = critCand;
			}
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
					if(n->getALAPnumber() - n->getALAPnumber() <= startdiff && n->getALAPnumber() <= node->getALAPnumber()){
						std::vector<dfgNode*> n_childs = n->getChildren();
						if(std::find(n_childs.begin(),n_childs.end(),node)!=n_childs.end()) continue;
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
			if(pseudoParentFound) break;

			if(!q_next.empty()) q.push(q_next);
		}

		if(!pseduoParent) outs() << "suitable pseduo parent not found.!\n";
//		assert(pseduoParent);

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

void DFGTrig::mergeAnnotatedNodesBr() {
//	assert(CtrlTrees.size()==1);
//	for(TreeNode<BasicBlock*>* root : CtrlTrees){
	//	TreeNode<BasicBlock*>* root = CtrlTrees[0];

		outs() << "mergeAnnotatedNodesBr begin...\n";

		printCtrlTree();

		for(dfgNode* node : NodeList){

			outs() << "node=" << node->getIdx() << ",";

			std::map<BasicBlock*,std::set<CondVal>> branchInfo;
			for(std::pair<BasicBlock*,CondVal> brVal : node->BelongsToBr){
				branchInfo[brVal.first].insert(brVal.second);
			}

			while(true){
				std::map<BasicBlock*,std::set<CondVal>> nextbranchInfo;
				bool changed=false;
				bool isRoot = false;
				for(std::pair<BasicBlock*,std::set<CondVal>> p1 : branchInfo){
					if(p1.second.size() == 2 && isRoot == false){
						BasicBlock* parentBB;
						bool parentVal;

						outs() << "oldBB=" << p1.first->getName() << ",\n";

						bool retVal = false;

						for(TreeNode<BasicBlock*>* root : CtrlTrees){
							if(root->getData() == p1.first){
								isRoot=true; break;
							}
							outs() << "\tSearching ctrl tree root=" << root->getData()->getName() << "\n";
							retVal = root->belongsToParent(p1.first,parentBB,parentVal);
							if(retVal) break;
						}

						if(!retVal){
							outs() << "\t\tBB=" << p1.first->getName() << "\n";
						}
						assert(retVal || isRoot);

						if(!isRoot){
						CondVal cv;
							if(parentVal == true){
								cv = TRUE;
							}else{
								cv = FALSE;
							}
							outs() << "\tnewBB=" << parentBB->getName() << ",\n";
							nextbranchInfo[parentBB].insert(cv);
							changed=true;
						}
						else{
							outs() << "\tnewBB=" << "EMPTY"  << ",\n";
							nextbranchInfo[parentBB] = p1.second;
							changed=false;
						}

					}
					else{
						nextbranchInfo[p1.first] = p1.second;
						assert(!p1.second.empty());
					}
				}
				if(!changed) break;
				branchInfo = nextbranchInfo;
			}

			node->BelongsToBr.clear();
			outs() << "\nAssigning new belongs :: ";
			for(std::pair<BasicBlock*,std::set<CondVal>> p1 : branchInfo){
				for(CondVal cv : p1.second){
					outs() << p1.first->getName() << "," << dfgNode::getCondValStr(cv) << ",";
					node->BelongsToBr.insert(std::make_pair(p1.first,cv));
				}
			}

			outs() << "\n";
		}
//	}

	outs() << "mergeAnnotatedNodesBr end...\n";
//	assert(false);
}

void DFGTrig::removeRedudantCtrl() {

	std::set<dfgNode*> removeNodes;

	for(dfgNode* node : NodeList){
		bool canRemoveCtrl=false;
		if(node->BelongsToBr.empty()) continue;
		for(dfgNode* par : node->getAncestors()){

			if(par->childConditionalMap[node]!=UNCOND) continue;
			if(par->BelongsToBr.empty()) continue;
			if(par->BelongsToBr == node->BelongsToBr){
				canRemoveCtrl=true;
				break;
			}
		}

		for(dfgNode* par : node->getAncestors()){
			if(canRemoveCtrl){
				if(par->childConditionalMap[node]!=UNCOND){
					std::cout << "Remove Redundant ctrl from=" << par->getIdx() << " to " << node->getIdx() << "\n";
					par->removeChild(node);
					node->removeAncestor(par);

					if(node->getNameType() == "CMERGE"){
						std::cout << "\tAlso removing CMERGE...\n";
						assert(node->getAncestors().size()==1);
						dfgNode* dataParent = node->getAncestors()[0];

						dataParent->removeChild(node);
						node->removeAncestor(dataParent);

						for(dfgNode* child : node->getChildren()){
							bool isBackedge = node->childBackEdgeMap[child];

							node->removeChild(child);
							child->removeAncestor(node);

							dataParent->addChildNode(child,EDGE_TYPE_DATA,isBackedge);
							child->addAncestorNode(dataParent,EDGE_TYPE_DATA,isBackedge);

							dfgNode* repChildNode = child;
							if(nCMP2pCMP[child]){
								repChildNode = nCMP2pCMP[child];
							}

							if(dyn_cast<PHINode>(repChildNode->getNode())){
								for(std::pair<int,dfgNode*> p1 : child->parentClassification){
									if(p1.second == node){
										child->parentClassification[p1.first]=dataParent;
										break;
									}
								}
							}
							else{ // if not phi
								int operand_no = findOperandNumber(repChildNode,repChildNode->getNode(),cmergePHINodes[node]->getNode());
								std::cout << "\t data_parent=" << dataParent->getIdx() << ",child=" << child->getIdx() << ",opno=" << operand_no << "\n";
//								child->parentClassification[operand_no]=dataParent;
								edgeClassification[dataParent][child]=operand_no;
							}

						}
						removeNodes.insert(node);
					}

					break;
				}
			}
		}
	}

	for(dfgNode* rn : removeNodes){
		NodeList.erase(std::remove(NodeList.begin(), NodeList.end(), rn), NodeList.end());
	}

//	assert(false);
}

void DFGTrig::removeCMERGEChildrenOpposingCtrl() {

	std::set<dfgNode*> removeNodes;

	for(dfgNode* node : NodeList){


		std::set<dfgNode*> cmergeChildren;
		for(dfgNode* child : node->getChildren()){
			if(node->childConditionalMap[child]!=UNCOND) continue;
			if(child->getNameType()=="CMERGE"){
				cmergeChildren.insert(child);
			}
		}

		if(cmergeChildren.size() != 2) continue;

		bool allhavesinglechild=true;
		for(dfgNode* cmergeChild : cmergeChildren){
			if(cmergeChild->getChildren().size() != 1){
				allhavesinglechild=false;
				break;
			}
		}

		bool isSameChild=true;
		if(allhavesinglechild){
			for(dfgNode* cmergeChild : cmergeChildren){
				if(cmergeChild->getChildren()[0] != (*cmergeChildren.begin())->getChildren()[0]){
					isSameChild=false;
					break;
				}
			}
		}


		if(isSameChild && allhavesinglechild){
			dfgNode* sameChild = (*cmergeChildren.begin())->getChildren()[0];
			dfgNode* cmerge1 = (*cmergeChildren.begin());
			dfgNode* cmerge2 = (*cmergeChildren.rbegin());
			assert(cmerge1->BelongsToBr.size()==1);
			assert(cmerge2->BelongsToBr.size()==1);

			std::pair<BasicBlock*,CondVal> cmerge1ci = *cmerge1->BelongsToBr.begin();
			std::pair<BasicBlock*,CondVal> cmerge2ci = *cmerge2->BelongsToBr.begin();

			std::set<std::pair<BasicBlock*,CondVal>> cmerge1Parentci = BrParentMap[cmerge1ci.first]->BelongsToBr;
			std::set<std::pair<BasicBlock*,CondVal>> nodeci = node->BelongsToBr;

			if(cmerge1ci.first == cmerge2ci.first && cmerge1Parentci == nodeci){

				std::cout << "node=" << node->getIdx();
				std::cout << ",cmerge1=" << cmerge1->getIdx();
				std::cout << ",cmerge2=" << cmerge2->getIdx();
				std::cout << ",sameChild=" << sameChild->getIdx() << "\n";

				node->removeChild(cmerge1);
				cmerge1->removeAncestor(node);

				node->removeChild(cmerge2);
				cmerge2->removeChild(node);

				node->addChildNode(sameChild,EDGE_TYPE_DATA);
				sameChild->addAncestorNode(node,EDGE_TYPE_DATA);

				if(dyn_cast<PHINode>(sameChild->getNode())){
					for(std::pair<int,dfgNode*> p1 : sameChild->parentClassification){
						if(p1.second == node){
							sameChild->parentClassification[p1.first]=cmerge1;
							break;
						}
					}
				}
				else{ // if not phi
					int operand_no = findOperandNumber(sameChild,sameChild->getNode(),cmergePHINodes[cmerge1]->getNode());
					std::cout << "\t data_parent=" << node->getIdx() << ",child=" << sameChild->getIdx() << ",opno=" << operand_no << "\n";
//								child->parentClassification[operand_no]=dataParent;
					edgeClassification[node][sameChild]=operand_no;
				}

				for(dfgNode* par : cmerge1->getAncestors()){
					par->removeChild(cmerge1);
					cmerge1->removeAncestor(par);
				}

				for(dfgNode* par : cmerge2->getAncestors()){
					par->removeChild(cmerge2);
					cmerge2->removeAncestor(par);
				}


				cmerge1->removeChild(sameChild);
				sameChild->removeAncestor(cmerge1);

				cmerge2->removeChild(sameChild);
				sameChild->removeAncestor(cmerge2);

				removeNodes.insert(cmerge1);
				removeNodes.insert(cmerge2);
			}
		}


	}

//	assert(false);

	for(dfgNode* rn : removeNodes){
		assert(rn->getAncestors().empty());
		assert(rn->getChildren().empty());
		NodeList.erase(std::remove(NodeList.begin(), NodeList.end(), rn), NodeList.end());
	}
}

void DFGTrig::removeNotCtrlConns() {

	for(std::pair<dfgNode*,dfgNode*> p1 : pCMP2nCMP){
		dfgNode* posNode = p1.first;
		dfgNode* negNode = p1.second;

		{
			std::set<dfgNode*> removeChildren;

			for(dfgNode* pChild : posNode->getChildren()){
				if(posNode->childConditionalMap[pChild]!=TRUE){
					outs() << "Wrong Pos Connection :: ";
					outs() << "PosNode=" << posNode->getIdx() << ",";
					outs() << "pChild=" << pChild->getIdx() << "\n";
				}

				assert(posNode->childConditionalMap[pChild]==TRUE);
				if(posNode->BelongsToBr == pChild->BelongsToBr){
					outs() << "Removing Conn,from=" << posNode->getIdx() << ",to=" << pChild->getIdx() << "\n";
					removeChildren.insert(pChild);
//					posNode->removeChild(pChild);
//					pChild->removeAncestor(posNode);
				}
			}

			for(dfgNode* rc : removeChildren){
				posNode->removeChild(rc);
				rc->removeAncestor(posNode);
			}

		}

		{
			std::set<dfgNode*> removeChildren;

			if(negNode){
				for(dfgNode* nChild : negNode->getChildren()){
					assert(negNode->childConditionalMap[nChild]==FALSE);
					if(negNode->BelongsToBr == nChild->BelongsToBr){
						outs() << "Removing Conn,from=" << negNode->getIdx() << ",to=" << nChild->getIdx() << "\n";
						removeChildren.insert(nChild);
	//					negNode->removeChild(nChild);
	//					nChild->removeAncestor(negNode);
					}
				}

				for(dfgNode* rc : removeChildren){
					negNode->removeChild(rc);
					rc->removeAncestor(negNode);
				}
			}

		}
	}
}

std::set<std::vector<std::pair<BasicBlock*, CondVal> > > DFGTrig::getPaths() {
	outs() << "Starting basicblock =" << currLoop->getHeader()->getName().str() << "\n";
	outs() << "Ending basicblock = " << currLoop->getExitBlock()->getName().str() << "\n";

	std::set<std::vector<std::pair<BasicBlock*, CondVal> > > res;

	struct element{
		BasicBlock* BB;
		std::vector<std::pair<BasicBlock*, CondVal> > path;
	};

	element first;
	first.BB = currLoop->getHeader();
	std::stack<element> st;
	st.push(first);

	while(!st.empty()){
		element curr = st.top(); st.pop();
		element next;
		element next_false;

		if(curr.BB == currLoop->getExitBlock()->getUniquePredecessor()){
			outs() << "PATH = ";
			for(std::pair<BasicBlock*, CondVal> p1 : curr.path){
				outs() << p1.first->getName() << "(" << dfgNode::getCondValStr(p1.second) << "),";
			}
			outs() << "\n";
			std::reverse(curr.path.begin(),curr.path.end());
			res.insert(curr.path);
			continue;
		}

		BasicBlock* currBB = curr.BB;
		BranchInst* BRI = cast<BranchInst>(currBB->getTerminator());

		if(BRI->isConditional()){
			next.BB = BRI->getSuccessor(0);
			next.path = curr.path;
			next.path.push_back(std::make_pair(curr.BB,TRUE));
			st.push(next);

			next_false.BB = BRI->getSuccessor(1);
			next_false.path = curr.path;
			next_false.path.push_back(std::make_pair(curr.BB,FALSE));
			st.push(next_false);
		}
		else{
			next.BB = BRI->getSuccessor(0);
			next.path = curr.path;
			st.push(next);
		}

	}

	return res;
}

void DFGTrig::addRecConnsAsPseudo() {

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


void DFGTrig::DFSCtrlPath(std::map<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>>& ctrlBBInfo, std::vector<std::pair<BasicBlock*,CondVal>> path){

	std::pair<BasicBlock*,CondVal> lastbbVal = *path.rbegin();

	if(ctrlBBInfo.find(lastbbVal.first)!=ctrlBBInfo.end()){
//		if(ctrlBBInfo[lastbbVal.first] == lastbbVal.second){
		//path is not ended;
			for(std::pair<BasicBlock*,CondVal> p1 : ctrlBBInfo[lastbbVal.first]){
				std::vector<std::pair<BasicBlock*,CondVal>> nextPath = path; nextPath.push_back(p1);
				DFSCtrlPath(ctrlBBInfo,nextPath);
			}
//		}
	}
	else{ //path has ended
		outs() << "PATH :: ";
		path.erase(path.begin());

		bool alreadyAdded=false;
		for( std::pair<std::set<dfgNode*>,std::vector<std::pair<BasicBlock*,CondVal>>> p1 : subPathDFGMap){
			if(p1.second == path){
				std::cout << "already added.\n";
				alreadyAdded=true;
				break;
			}
		}

		if(!alreadyAdded){
			for(std::pair<BasicBlock*,CondVal> p1 : path){
				outs() << p1.first->getName() << "(" << dfgNode::getCondValStr(p1.second) << "),";
			}
			outs() << "\n";

//			CreatSubDFGPath(path);
			popCtrlTrees(path);
		}
	}

}

void DFGTrig::populateSubPathDFGs() {

//	std::queue<dfgNode*> q;
//	for(dfgNode* n : NodeList){
//
//		int parentCount=0;
//		for(dfgNode* parent : n->getAncestors()){
//			if(parent->childBackEdgeMap[n]) continue;
//			parentCount++;
//		}
//		if(parentCount == 0){
//			q.push(n);
//		}
//	}

//	std::map<dfgNode*,std::set<dfgNode*>> visitedFrom;

	std::map<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> ctrlBBInfo =  getCtrlInfoBBMorePaths();

	std::set<BasicBlock*> roots;
	for(std::pair<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> p1 : ctrlBBInfo){
		BasicBlock* currBB = p1.first;
		outs() << "currBB=" << currBB->getName() << "\n";

		bool isRoot=true;
		for(std::pair<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> p2 : ctrlBBInfo){
			if(p1 == p2) continue;
			outs() << "\totherBBD=" << p2.first->getName() << "\n";
			std::set<std::pair<BasicBlock*,CondVal>> bbVals = p2.second;
			for(std::pair<BasicBlock*,CondVal> bbVal : bbVals){
				outs() << "\t\totherBBDep=" << bbVal.first->getName() << "\n";
				if(bbVal.first == currBB){
					isRoot=false; break;
				}
			}
			if(!isRoot) break;
		}

		if(isRoot){
			outs() << "ROOT : " << currBB->getName() << "\n";
			roots.insert(currBB);
		}

	}


	for(BasicBlock* root : roots ){
		std::vector<std::pair<BasicBlock*,CondVal>> path; path.push_back(std::make_pair(root,UNCOND));
		DFSCtrlPath(ctrlBBInfo,path);
	}
	printCtrlTree();
//	assert(false);



//
//	std::stack<dfgNode*> ctrlNodes;
//
//	for(dfgNode* n : NodeList){
//		for(dfgNode* child : n->getChildren()){
//			if(n->childBackEdgeMap[child]) continue;
//			if(n->childConditionalMap[child]!=UNCOND){
//				ctrlNodes.push(n);
//				break;
//			}
//		}
//	}
//
//	while(!ctrlNodes.empty()){
//
//
//
//	}
//
//
//	while(!q.empty()){
//
//
//
//
//	}




}

BasicBlock* DFGTrig::getBackEdgeParentBB() {

		SmallVector<std::pair<const BasicBlock *, const BasicBlock *>,8> BackedgeBBs;
		dfgNode firstNode = *NodeList[0];
		FindFunctionBackedges(*(firstNode.getNode()->getFunction()),BackedgeBBs);

		std::set<BasicBlock*> backedgeParents;
		for(std::pair<const BasicBlock *, const BasicBlock *> p1 : BackedgeBBs){
			BasicBlock* parent = (BasicBlock*)p1.first;
			if(loopBB.find(parent)!=loopBB.end()){
				backedgeParents.insert(parent);
			}
		}

		assert(backedgeParents.size() == 1);
		outs() << "END BB = " << (*backedgeParents.begin())->getName() << "\n";
		return *backedgeParents.begin();
}

void DFGTrig::CreatSubDFGPath(
		std::vector<std::pair<BasicBlock*, CondVal> > path) {


	std::map<dfgNode*,CondVal> ctrlNodeVals;
	for(std::pair<BasicBlock*,CondVal> p1 : path){
		ctrlNodeVals[BrParentMap[p1.first]]=p1.second;
		ctrlNodeVals[pCMP2nCMP[BrParentMap[p1.first]]]=p1.second;
	}
//
//	for(std::pair<dfgNode*,dfgNode*> p1 : nCMP2pCMP){
//		ctrlNodeVals[p1.first]=FALSE;
//	}

	std::set<dfgNode*> rootNodes = getRootNodes();
	std::queue<dfgNode*> q;
	for(dfgNode* node : rootNodes){
		q.push(node);
	}

	std::map<dfgNode*,int> workingSet;
	std::map<dfgNode*,int> nonBEParentCount;


	for(dfgNode* node : NodeList){
		int parentCount = 0;
		bool CMERGEfound=false;
		bool ctrlInputfound=false;
		std::set<int> cmerge_operands;
		for(dfgNode* parent : node->getAncestors()){
			if(parent->childBackEdgeMap[node]) continue;
			if(parent->getNameType() == "CMERGE" || parent->getNameType() == "SELECTPHI"){

				int operand_no;
				outs() << "node=" << node->getIdx() << ",parent=" << parent->getIdx() << "\n";

				if(node->getNode() && dyn_cast<PHINode>(node->getNode())){
					operand_no = 1;
				}
				else if(parent->getNameType() == "CMERGE"){
					dfgNode* repNode = node;
					if(nCMP2pCMP[node]){
						repNode = nCMP2pCMP[node];
					}
					assert(repNode->getNode()); assert(cmergePHINodes[parent]->getNode());
					operand_no = findOperandNumber(repNode,repNode->getNode(),cmergePHINodes[parent]->getNode());
				}
				else if(parent->getNameType() == "SELECTPHI"){
					dfgNode* repNode = node;
					if(nCMP2pCMP[node]){
						repNode = nCMP2pCMP[node];
					}
					assert(repNode->getNode()); assert(selectPHIAncestorMap[parent]->getNode());
					operand_no = findOperandNumber(repNode,repNode->getNode(),selectPHIAncestorMap[parent]->getNode());
				}
				cmerge_operands.insert(operand_no);
				CMERGEfound=true;
				continue;
			}
			if(parent->childConditionalMap[node]!=UNCOND){
				ctrlInputfound=true;
				continue;
			}
			parentCount++;
		}

		if(CMERGEfound){
			parentCount = parentCount + cmerge_operands.size();
		}
		if(ctrlInputfound) parentCount++;

		assert(node->getAncestors().size() != 0 || node->getChildren().size() != 0);

		nonBEParentCount[node]=parentCount;
	}

	std::set<dfgNode*> tempSet;

	while(!q.empty()){
		dfgNode* currNode = q.front(); q.pop();
		tempSet.insert(currNode);

		for(dfgNode* child : currNode->getChildren()){
			if(currNode->childBackEdgeMap[child]) continue;
			if(ctrlNodeVals.find(currNode)!=ctrlNodeVals.end()){
				if(currNode->childConditionalMap[child] != ctrlNodeVals[currNode]) continue;
			}
			else{
				// this means currnode is a control node who is not even executed in the current path
				if(BrParentMapInv.find(currNode)!=BrParentMapInv.end()) continue;
				if(nCMP2pCMP.find(currNode)!=nCMP2pCMP.end()) continue;
			}

			if(workingSet.find(child)!=workingSet.end()){
				workingSet[child]++;
			}
			else{
				workingSet[child]=1;
			}

			if(workingSet[child] > nonBEParentCount[child]){
				outs() << "FATAL! : " << "child = " << child->getIdx();
				outs() << ", parentCount = " << nonBEParentCount[child];
				outs() << ", currNode = " << currNode->getIdx() << "\n";
			}

			assert(workingSet[child] <= nonBEParentCount[child]);
			if(workingSet[child] == nonBEParentCount[child]){

				q.push(child);
			}
		}
	}

	std::set<dfgNode*> subDFGNodes;

	for(dfgNode* node : tempSet){
		bool atleastOneChildFound=false;
		bool atleastOneParentFound=false;

		for(dfgNode* child : node->getChildren()){
			if(tempSet.find(child)!=tempSet.end()){
				atleastOneChildFound=true; break;
			}
		}
		for(dfgNode* parent : node->getAncestors()){
			if(tempSet.find(parent)!=tempSet.end()){
				atleastOneParentFound=true; break;
			}
		}
		if(atleastOneChildFound || atleastOneParentFound){
//			outs() << "Node = " << node->getIdx() << "\n";
			subDFGNodes.insert(node);
		}
	}
//	subPathDFGs.push_back(subDFGNodes);
	subPathDFGMap[subDFGNodes]=path;
//	assert(false);
}

std::set<dfgNode*> DFGTrig::getRootNodes() {

	std::set<dfgNode*> res;
	for(dfgNode* node : NodeList){
		int parentCount = 0;
		for(dfgNode* parent : node->getAncestors()){
			if(parent->childBackEdgeMap[node]) continue;
			parentCount++;
		}

		if(parentCount == 0) res.insert(node);
	}
	return res;
}

dfgNode* DFGTrig::addLoadParent(Value* ins, dfgNode* child) {
	dfgNode* temp;
	outs() << "adding load parent.\n";
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
			ofs << "\n";


			//BelongstoBR
			if(node->BelongsToBr.empty()){
				ofs << "Br = (EMPTY)";
			}
			else{
				ofs << "Br = ";
				for(std::pair<BasicBlock*,CondVal> p1 : node->BelongsToBr){
					ofs << p1.first->getName().str() << "[" << dfgNode::getCondValStr(p1.second) << "]\n";
				}
			}
			ofs << "\"]\n";
			//END NODE NAME
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


void DFGTrig::printSubPathDOT(std::string fileNamePrefix) {

	int pathIdx=0;
	for(std::pair<std::set<dfgNode*>,std::vector<std::pair<BasicBlock*,CondVal>> > p1 : subPathDFGMap){

		std::set<dfgNode*> subDFG = p1.first;
		std::vector<std::pair<BasicBlock*,CondVal>> path = p1.second;
		std::reverse(path.begin(),path.end());

		std::stringstream pathName; pathName << "_PATH_";
		for(std::pair<BasicBlock*,CondVal> pair : path){
			dfgNode* ctrl = BrParentMap[pair.first];
			pathName << ctrl->getIdx();
			if(pair.second == TRUE){
				pathName << "T_";
			}
			else{
				assert(pair.second == FALSE);
				pathName << "F_";
			}
		}


		std::ofstream ofs;
		ofs.open((fileNamePrefix + pathName.str() + ".dot").c_str());

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

		for(dfgNode* node : subDFG){
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
				ofs << "\n";


				//BelongstoBR
				if(node->BelongsToBr.empty()){
					ofs << "Br = (EMPTY)";
				}
				else{
					ofs << "Br = ";
					for(std::pair<BasicBlock*,CondVal> p1 : node->BelongsToBr){
						ofs << p1.first->getName().str() << "[" << dfgNode::getCondValStr(p1.second) << "]\n";
					}
				}
				ofs << "\"]\n";
				//END NODE NAME
			}

	//		ofs << "label = " << "\"" << BB->getName().str() << "\"" << ";\n";
	//		ofs << "color = purple;\n";
	//		ofs << "}\n";
			cluster_idx++;
		}


		//The EDGES

		for(dfgNode* node : subDFG){

			for(dfgNode* child : node->getChildren()){
				bool isCondtional=false;
				bool condition=true;

				if(subDFG.find(child) == subDFG.end()) continue;

				isCondtional= node->childConditionalMap[child] != UNCOND;
				condition = (node->childConditionalMap[child] == TRUE);

				bool isBackEdge = node->childBackEdgeMap[child];

//				if(BrParentMapInv.find(node)!=BrParentMapInv.end()){
//					BasicBlock* BrBB = BrParentMapInv[node];
//					std::pair<BasicBlock*,CondVal> searchPair(BrBB,node->childConditionalMap[child]);
//
//					if(std::find(path.begin(),path.end(),searchPair) == path.end()){
//						continue;
//					}
//				}

				if(!node->BelongsToBr.empty()){
					bool needToCont=true;
					for(std::pair<BasicBlock*,CondVal> p1 : node->BelongsToBr){
						if(std::find(path.begin(),path.end(),p1) != path.end()){
							needToCont=false;
							break;
						}
					}
					if(needToCont) continue;
				}


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
				if(subDFG.find(recChild) == subDFG.end()) continue;
				ofs << "\"" << "Op_" << node->getIdx() << "\"";
				ofs << " -> ";
				ofs << "\"" << "Op_" << recChild->getIdx() << "\"";
				ofs << "[style = bold, color = green];\n";
			}
		}

		ofs << "}" << std::endl;
		ofs.close();







		pathIdx++;
	}






}

void DFGTrig::ConnectBrCtrls() {

//	Loop* lp = mappingUnitMap[this->mUnitName];
	Value* v = this->currLoop->getCanonicalInductionVariable();
	v->dump();

	dfgNode* IndNode = findNode(cast<Instruction>(v)); assert(IndNode);
	outs() << "Node = " << IndNode->getIdx() << "\n";


	outs()  << "Longest Path = ";
	for(std::pair<BasicBlock*,CondVal> p1 : longestPath){
		dfgNode* ctrlNode = BrParentMap[p1.first];
		outs()  << ctrlNode->getIdx();
		if(p1.second == TRUE){
			outs()  << "T_";
		}
		else{
			assert(p1.second == FALSE);
			outs()  << "F_";
		}
	}
	outs()  << "\n";

//	assert(CtrlTrees.size() == 1);
	for(TreeNode<BasicBlock*>* rootTree : CtrlTrees){
//	TreeNode<BasicBlock*>* rootTree = CtrlTrees[0];

		std::set<std::pair<TreeNode<BasicBlock*>*,bool>> leaves = rootTree->getLeafs();
		outs()  << "Leaf of ctrl tree :: ";
		for(std::pair<TreeNode<BasicBlock*>*,bool> p1 : leaves){
			dfgNode* ctrlNode = BrParentMap[p1.first->getData()];
			outs()  << ctrlNode->getIdx();
			if(p1.second){
				outs()  << "T,";
				ctrlNode->addChildNode(IndNode,EDGE_TYPE_DATA,true,true,true);
				IndNode->addAncestorNode(ctrlNode,EDGE_TYPE_DATA,true,true,true);
				leafControlInputs[IndNode].insert(ctrlNode);
			}
			else{
				outs()  << "F,";
				dfgNode* neg_ctrlNode = pCMP2nCMP[ctrlNode];
				if(neg_ctrlNode){
					neg_ctrlNode->addChildNode(IndNode,EDGE_TYPE_DATA,true,true,false);
					IndNode->addAncestorNode(neg_ctrlNode,EDGE_TYPE_DATA,true,true,false);
					leafControlInputs[IndNode].insert(neg_ctrlNode);
				}
			}
		}
		outs()  << "\n";

	}


//	assert(false);

}

void DFGTrig::mirrorCtrlNodes() {

	for(std::pair<BasicBlock*,dfgNode*> p1 : BrParentMap){
		dfgNode* ctrlNode = p1.second;
		pCMPNodes.insert(ctrlNode);

		bool negativeChildFound=false;
		for(dfgNode* child : ctrlNode->getChildren()){
			if(ctrlNode->childConditionalMap[child]==FALSE){
				negativeChildFound=true;
				break;
			}
		}
		if(!negativeChildFound) continue;

		dfgNode* temp = new dfgNode(this);
		temp->setIdx(2000 + NodeList.size());
		temp->setNameType("N_CMP");
		temp->BB = ctrlNode->BB;
		NodeList.push_back(temp);
		nCMP2pCMP[temp]=ctrlNode;
		pCMP2nCMP[ctrlNode]=temp;

		for(dfgNode* child : ctrlNode->getChildren()){
			assert(ctrlNode->childConditionalMap[child]!=UNCOND);
			if(ctrlNode->childConditionalMap[child]==FALSE){
				bool isBackEdge = ctrlNode->childBackEdgeMap[child];

				std::cout << "remove child=" << child->getIdx() << " of " << ctrlNode->getIdx() << "\n";
				ctrlNode->removeChild(child);
				child->removeAncestor(ctrlNode);

				std::cout << "add child=" << child->getIdx() << " to " << temp->getIdx() << "\n";
				temp->addChildNode(child,EDGE_TYPE_DATA,isBackEdge,true,false);
				child->addAncestorNode(temp,EDGE_TYPE_DATA,isBackEdge,true,false);

				if(child->getNameType() == "CMERGE"){
					cmergeCtrlInputs[child]=temp;
//					leafControlInputs[child].insert(temp);
					continue;
				}


				assert(leafControlInputs.find(child) != leafControlInputs.end());
				assert(leafControlInputs[child].find(ctrlNode) != leafControlInputs[child].end());
				leafControlInputs[child].erase(ctrlNode);
				leafControlInputs[child].insert(temp);


			}
		}

		for(dfgNode* parent : ctrlNode->getAncestors()){
			bool isBackedge = parent->childBackEdgeMap[ctrlNode];
			CondVal cv = parent->childConditionalMap[ctrlNode];

			std::cout << "add parent=" << parent->getIdx() << " to " << temp->getIdx() << "\n";
			parent->addChildNode(temp,EDGE_TYPE_DATA,isBackedge,(cv!=UNCOND),(cv==TRUE));
			temp->addAncestorNode(parent,EDGE_TYPE_DATA,isBackedge,(cv!=UNCOND),(cv==TRUE));

			if(leafControlInputs.find(ctrlNode) != leafControlInputs.end()){
				leafControlInputs[temp]=leafControlInputs[ctrlNode];
			}

		}
	}






}

void DFGTrig::removeCMERGEwithoutData() {

	scheduleASAP();
	scheduleALAP();

	std::set<dfgNode*> rn;

	std::set<std::pair<dfgNode*,dfgNode*>> already_added_edges;

	for(dfgNode* node : NodeList){
		if(node->getNameType() == "CMERGE"){
			for(dfgNode* anc : node->getAncestors()){
				for(dfgNode* child : node->getChildren()){
					if(already_added_edges.find(std::make_pair(anc,child)) != already_added_edges.end()){
						continue;
					}

					int operand_no=-1;
					if( ( child->getNode() && (dyn_cast<PHINode>(child->getNode())) ) ||
					      (!child->getNode()) ){
							if(child->parentClassification[0]==node){
								operand_no = 0;
							}
							else if(child->parentClassification[1]==node){
								operand_no = 1;
							}
							else if(child->parentClassification[2]==node){
								operand_no = 2;
							}
							else if(edgeClassification.find(node)!=edgeClassification.end() && edgeClassification[node].find(child)!=edgeClassification[node].end()){
								operand_no = edgeClassification[node][child];
							}
							else{
								bool found=false;
								for(int p = 3; p < child->parentClassification.size(); p++){
									if(child->parentClassification[p]==node){
										found=true;
										break;
									}
								}
								assert(found);
								outs() << "node=" << node->getIdx() << ",child=" << child->getIdx() << "\n";
								// operand_no = 1;
								assert(false);
							}
					}
					else{ // if not phi
						assert(child->getNode());
						operand_no = findOperandNumber(child,child->getNode(),cmergePHINodes[node]->getNode());
					}


					assert(operand_no != -1);
					if(anc == cmergeCtrlInputs[node]){
						edgeClassification[anc][child] = 0;
					}
					else if(anc == cmergeDataInputs[node]){
						edgeClassification[anc][child] = operand_no;
					}
					else{
						assert(false);
					}

					CondVal cond = anc->ancestorConditionaMap[node];
					bool isConditional = (cond != UNCOND);
					bool isBackEdge = anc->childBackEdgeMap[node] || node->childBackEdgeMap[child];

					// if(isBackEdge){
					// 	continue;
					// }
					rn.insert(node);
					anc->addChildNode(child,EDGE_TYPE_DATA,isBackEdge,isConditional,cond);
					child->addAncestorNode(anc,EDGE_TYPE_DATA,isBackEdge,isConditional,cond);
					already_added_edges.insert(std::make_pair(anc,child));
				}
			}
		}
	}

	for(dfgNode* node : rn){
		for(dfgNode* anc : node->getAncestors()){
			anc->removeChild(node);
			node->removeAncestor(anc);
		}
		for(dfgNode* child : node->getChildren()){
			node->removeChild(child);
			child->removeAncestor(node);
		}
		NodeList.erase(std::remove(NodeList.begin(), NodeList.end(), node), NodeList.end());
	}

	scheduleASAP();
	scheduleALAP();

}

void DFGTrig::nameNodes() {

	dfgNode* node;

	for (int i = 0; i < NodeList.size(); ++i) {
		node = NodeList[i];

		if(node->getNode() != NULL){
			switch(node->getNode()->getOpcode()){
				case Instruction::Add:
				case Instruction::FAdd:
					node->setFinalIns(ADD);
					break;
				case Instruction::Sub:
				case Instruction::FSub:
					node->setFinalIns(SUB);
					break;
				case Instruction::Mul:
				case Instruction::FMul:
					node->setFinalIns(MUL);
					break;
				case Instruction::UDiv:
				case Instruction::SDiv:
				case Instruction::FDiv:
					node->setFinalIns(DIV);
					break;
				case Instruction::URem:
				case Instruction::SRem:
				case Instruction::FRem:
					errs() << "REM operations are not implemented\n";
					assert(false);
					break;
				case Instruction::Shl:
					node->setFinalIns(LS);
					break;
				case Instruction::LShr:
					node->setFinalIns(RS);
					break;
				case Instruction::AShr:
					node->setFinalIns(ARS);
					break;
				case Instruction::And:
					node->setFinalIns(AND);
					break;
				case Instruction::Or:
					node->setFinalIns(OR);
					break;
				case Instruction::Xor:
					node->setFinalIns(XOR);
					break;
				case Instruction::Load:
					if(node->getTypeSizeBytes()>=4){ //TODO : remove this, we dont support 8 byte data structs
						node->setFinalIns(Hy_LOAD);
					}
					else if(node->getTypeSizeBytes()==2){
						node->setFinalIns(Hy_LOADH);
					}
					else if(node->getTypeSizeBytes()==1){
						node->setFinalIns(Hy_LOADB);
					}
					else if(node->getNode()->getType()->isPointerTy()){ //pointer is a 32 bit address
						node->setFinalIns(Hy_LOAD);
					}
					else{
						node->getNode()->dump();
						outs() << "OutLoopLOAD size = " << node->getTypeSizeBytes() << "\n";
						if(node->getNode()->getType()==Type::getDoubleTy(node->getNode()->getContext())){
							node->setFinalIns(Hy_LOAD);
						}
						if(node->getNode()->getType()==Type::getDoublePtrTy(node->getNode()->getContext())){
							node->setFinalIns(Hy_LOAD);
						}
						else{
							assert(0);
						}
					}
					break;
				case Instruction::Store:
					if(node->getTypeSizeBytes()>=4){ //TODO : remove this, we dont support 8 byte data structs
						node->setFinalIns(Hy_STORE);
					}
					else if(node->getTypeSizeBytes()==2){
						node->setFinalIns(Hy_STOREH);
					}
					else if(node->getTypeSizeBytes()==1){
						node->setFinalIns(Hy_STOREB);
					}
					else if(node->getNode()->getType()->isPointerTy() || node->getNode()->getOperand(0)->getType()->isPointerTy()){ //pointer is a 32 bit address
						node->setFinalIns(Hy_STORE);
					}
					else{
						node->getNode()->dump();
						outs() << "TypeSize : " << node->getTypeSizeBytes() << "\n";
						assert(0);
					}
					break;
				case Instruction::GetElementPtr:
					node->setFinalIns(ADD);
					node->setConstantVal(node->getGEPbaseAddr());
					break;
				case Instruction::Trunc:
					{TruncInst* TI = cast<TruncInst>(node->getNode());
//					node->setConstantVal(TI->getDestTy()->getIntegerBitWidth()/8);
					double destBitWidthDbl = (double)(TI->getDestTy()->getIntegerBitWidth());
					double bitMaskDbl = pow(2,destBitWidthDbl)-1;
					uint32_t bitMask= ((uint32_t)bitMaskDbl);
					node->setConstantVal(bitMask);
					node->setFinalIns(AND);}
					break;
				case Instruction::SExt:
					{SExtInst* SI = cast<SExtInst>(node->getNode());
					uint32_t srcByteWidth = SI->getSrcTy()->getIntegerBitWidth()/8;
					uint32_t destByteWidth = SI->getDestTy()->getIntegerBitWidth()/8;
					uint32_t constOperand = (srcByteWidth << 16) | destByteWidth;
					node->setConstantVal(constOperand);
					node->setFinalIns(SEXT);}
					break;
				case Instruction::ZExt:
					node->setFinalIns(OR);
					node->setConstantVal(0);
					break;
				case Instruction::SIToFP:
				case Instruction::FPExt:
				case Instruction::FPTrunc:
				case Instruction::FPToUI:
				case Instruction::FPToSI:
				case Instruction::UIToFP:
				case Instruction::PtrToInt:
				case Instruction::IntToPtr:
				case Instruction::BitCast:
				case Instruction::AddrSpaceCast:
					node->getNode()->dump();
//					assert(0);
					//2019 work
					node->setFinalIns(OR);
					node->setConstantVal(0);
					break;
				case Instruction::PHI:
				case Instruction::Select:
					node->setFinalIns(SELECT);
					break;
				case Instruction::Br:
					{BranchInst* BRI = cast<BranchInst>(node->getNode());
					if(BRI->isUnconditional()){
						node->setFinalIns(OR);
						node->setConstantVal(1);
					}
					else{
						node->setFinalIns(OR);
						node->setConstantVal(0);
					}}
					break;
				case Instruction::ICmp:
					{
						outs() << "NameNodes::Node=" << node->getIdx() << ",CMP=";
						CmpInst* CI = cast<CmpInst>(node->getNode());
						CI->dump();
						switch(CI->getPredicate()){
							case CmpInst::ICMP_SLT:
							case CmpInst::ICMP_ULT:
								outs() << "LT\n";
								node->setFinalIns(CLT);
								break;
							case CmpInst::ICMP_SGT:
							case CmpInst::ICMP_UGT:
								outs() << "GT\n";
								node->setFinalIns(CGT);
								break;
							case CmpInst::ICMP_EQ:
								outs() << "EQ\n";
								node->setFinalIns(CMP);
								break;
							default:
								assert(false);
								break;
						}
					}
					break;
				case Instruction::FCmp:
					outs() << "NameNodes::Node=" << node->getIdx() << ",FCMP\n";
					node->setFinalIns(CMP);
					//2019 work
//					assert(false);
					break;
				default :
					errs() << "The Op :" << node->getNode()->getOpcodeName() << " that I thought would not be in the compiled code\n";
					node->getNode()->dump();
					assert(false);
					break;
			}
		}
		else{
			if(node->getNameType().compare("LOAD") == 0){
				if(node->getTypeSizeBytes()==4){
					node->setFinalIns(Hy_LOAD);
				}
				else if(node->getTypeSizeBytes()==2){
					node->setFinalIns(Hy_LOADH);
				}
				else if(node->getTypeSizeBytes()==1){
					node->setFinalIns(Hy_LOADB);
				}
				else{
					assert(0);
				}
			}
			else if(node->getNameType().compare("STORE") == 0){
				if(node->getTypeSizeBytes()==4){
					node->setFinalIns(Hy_STORE);
				}
				else if(node->getTypeSizeBytes()==2){
					node->setFinalIns(Hy_STOREH);
				}
				else if(node->getTypeSizeBytes()==1){
					node->setFinalIns(Hy_STOREB);
				}
				else{
					assert(0);
				}
			}
			else if(node->getNameType().compare("NORMAL") == 0){
				node->setFinalIns(ADD);
			}
			else if(node->getNameType().compare("CTRLBrOR") == 0){
				node->setFinalIns(BR);
			}
			else if(node->getNameType().compare("SELECTPHI") == 0){
				node->setFinalIns(SELECT);
			}
			else if(node->getNameType().compare("OutLoopSTORE") == 0){
				if(node->getTypeSizeBytes()==4){
					node->setFinalIns(Hy_STORE);
				}
				else if(node->getTypeSizeBytes()==2){
					node->setFinalIns(Hy_STOREH);
				}
				else if(node->getTypeSizeBytes()==1){
					node->setFinalIns(Hy_STOREB);
				}
				else{
					assert(0);
				}
				node->setConstantVal(node->getoutloopAddr());
			}
			else if(node->getNameType().compare("OutLoopLOAD") == 0){
				if(node->getTypeSizeBytes()==4){
					node->setFinalIns(Hy_LOAD);
				}
				else if(node->getTypeSizeBytes()==2){
					node->setFinalIns(Hy_LOADH);
				}
				else if(node->getTypeSizeBytes()==1){
					node->setFinalIns(Hy_LOADB);
				}
				else if(node->getTypeSizeBytes()==8){
					node->setFinalIns(Hy_LOAD);
				}
				else{
					OutLoopNodeMapReverse[node]->dump();
					outs() << "OutLoopLOAD size = " << node->getTypeSizeBytes() << "\n";
					assert(0);
				}
				node->setConstantVal(node->getoutloopAddr());
			}
			else if(node->getNameType().compare("CMERGE") == 0){
				node->setFinalIns(CMERGE);
			}
			else if(node->getNameType().compare("LOOPSTART") == 0){
//				node->setFinalIns(BR);
				node->setFinalIns(Hy_LOADB);
				node->setConstantVal(MEM_SIZE - 2);
			}
			else if(node->getNameType().compare("LOADSTART") == 0){
				node->setFinalIns(Hy_LOADB);
				node->setConstantVal(MEM_SIZE - 2);
			}
			else if(node->getNameType().compare("STORESTART") == 0){
				node->setFinalIns(Hy_STOREB);
				node->setConstantVal(MEM_SIZE - 2);
			}
			else if(node->getNameType().compare("LOOPEXIT") == 0){
				node->setFinalIns(Hy_STOREB);
				node->setConstantVal(MEM_SIZE/2 - 1);
			}
			else if(node->getNameType().compare("MOVC") == 0){
				node->setFinalIns(MOVC);
			}
			else if(node->getNameType().compare("XORNOT") == 0){
				node->setFinalIns(XOR);
				node->setConstantVal(1);
			}
			else if(node->getNameType().compare("ORZERO") == 0){
				node->setFinalIns(OR);
				node->setConstantVal(0);
			}
			else if(node->getNameType().compare("GEPLEFTSHIFT") == 0){
				node->setFinalIns(LS);
			}
			else if(node->getNameType().compare("GEPADD") == 0){
				node->setFinalIns(ADD);
			}
			else if(node->getNameType().compare("MASKAND") == 0){
				node->setFinalIns(AND);
			}
			else if(node->getNameType().compare("PREDAND") == 0){
				node->setFinalIns(AND);
			}
			else if(node->getNameType().compare("N_CMP") == 0){
				node->setFinalIns(NCMP);
			}
			else {
				errs() << "Unknown custom node \n";
				assert(false);
			}
		}

		hyCUBEInsHist[node->getFinalIns()]++;
	}


}

