//#include "../../include/dfg/DFGOpenCGRA.h"
#include <morpherdfggen/dfg/DFGOpenCGRA.h>

#include "llvm/Analysis/CFG.h"
#include <algorithm>
#include <queue>
#include <map>
#include <set>
#include <vector>

using namespace std;
#define LV_NAME "dfg_gen" //"sfp"
#define DEBUG_TYPE LV_NAME

void DFGOpenCGRA::generateTrigDFGDOT(Function &F) {


	std::set<exitNode> exitNodes;

	getLoopExitConditionNodes(exitNodes);
	//	connectBB();
	removeAlloc();
	connectBBTrig();
	createCtrlBROrTree();
	//printDOT(this->name + "bef_handlePHINodes_PartPredDFG.dot");
	//LLVM_DEBUG(dbgs() << "\n[DFGOpenCGRA.cpp][handlePHINodes begin]\n");
	handlePHINodes(this->loopBB);

	//LLVM_DEBUG(dbgs() << "\n[DFGOpenCGRA.cpp][handlePHINodes end]\n");
	printDOT(this->name + "after_handlePHINodes.dot");
	//LLVM_DEBUG(dbgs() << "\n[DFGOpenCGRA.cpp][handleSELECTNodes begin]\n");
	//handleSELECTNodes();

	//LLVM_DEBUG(dbgs() << "\n[DFGOpenCGRA.cpp][handleSELECTNodes end]\n");
	//printDOT(this->name + "after_handleSELECTNodes_PartPredDFG.dot");

	//exit(true);

	// insertshiftGEPs();
	addMaskLowBitInstructions();
	//insertshiftGEPsCorrect();
	//removeDisconnetedNodes();
	//	scheduleCleanBackedges();
	//LLVM_DEBUG(dbgs() << "\n[DFGOpenCGRA.cpp][fillCMergeMutexNodes begin]\n");
	//fillCMergeMutexNodes();
	//LLVM_DEBUG(dbgs() << "\n[DFGOpenCGRA.cpp][fillCMergeMutexNodes end]\n");


	//LLVM_DEBUG(dbgs() << "\n[DFGOpenCGRA.cpp][constructCMERGETree begin]\n");
	//constructCMERGETree();
	//LLVM_DEBUG(dbgs() << "\n[DFGOpenCGRA.cpp][constructCMERGETree end]\n");

	//	printDOT(this->name + "_PartPredDFG.dot"); return;
	//LLVM_DEBUG(dbgs() << "\n[DFGOpenCGRA.cpp][addLoopExitStoreHyCUBE begin]\n");
	//addLoopExitStoreHyCUBE(exitNodes);
	//LLVM_DEBUG(dbgs() << "[DFGOpenCGRA.cpp][addLoopExitStoreHyCUBE end]\n\n");
	//LLVM_DEBUG(dbgs() << "\n[DFGOpenCGRA.cpp][handlestartstop begin]\n");
	//handlestartstop();
	//LLVM_DEBUG(dbgs() << "[DFGOpenCGRA.cpp][handlestartstop end] Nodelist size:" <<NodeList.size()<<"\n\n");

	//	printDOT(this->name + "afterhandlestartstop_PartPredDFG.dot");
	LLVM_DEBUG(dbgs() << "\n[DFGOpenCGRA.cpp][scheduleASAP begin]\n");
	scheduleASAP();
	LLVM_DEBUG(dbgs() << "[DFGOpenCGRA.cpp][scheduleASAP end]\n\n");
	//	printDOT(this->name + "afterscheduleASAP_PartPredDFG.dot");
	//	return;
	LLVM_DEBUG(dbgs() << "\n[DFGOpenCGRA.cpp][scheduleALAP begin]\n");
	scheduleALAP();
	LLVM_DEBUG(dbgs() << "[DFGOpenCGRA.cpp][scheduleALAP end]\n\n");
	// assignALAPasASAP();
	//	balanceSched();



	//LLVM_DEBUG(dbgs() << "\n[DFGOpenCGRA.cpp][GEPBaseAddrCheck begin]\n");
	//GEPBaseAddrCheck(F);
	//LLVM_DEBUG(dbgs() << "[DFGOpenCGRA.cpp][GEPBaseAddrCheck end]\n\n");
	LLVM_DEBUG(dbgs() << "\n[DFGOpenCGRA.cpp][nameNodes begin]\n");
	nameNodesCGRAME();
	LLVM_DEBUG(dbgs() << "[DFGOpenCGRA.cpp][nameNodes end]\n\n");
	LLVM_DEBUG(dbgs() << "\n[DFGOpenCGRA.cpp][classifyParents begin]\n");
	classifyParents();
	// RemoveInductionControlLogic();

	// RemoveBackEdgePHIs();
	// removeOutLoopLoad();
	//	RemoveConstantCMERGEs(); Originally on morpher
	LLVM_DEBUG(dbgs() << "[DFGOpenCGRA.cpp][classifyParents end]\n\n");
	//LLVM_DEBUG(dbgs() << "\n[DFGOpenCGRA.cpp][removeDisconnectedNodes begin]\n");
	//removeDisconnectedNodes();
	//LLVM_DEBUG(dbgs() << "[DFGOpenCGRA.cpp][removeDisconnectedNodes end]\n\n");
	//LLVM_DEBUG(dbgs() << "\n[DFGOpenCGRA.cpp][addOrphanPseudoEdges begin]\n");
	//addOrphanPseudoEdges();
	//LLVM_DEBUG(dbgs() << "[DFGOpenCGRA.cpp][addOrphanPseudoEdges end]\n\n");
	//LLVM_DEBUG(dbgs() << "\n[DFGOpenCGRA.cpp][addRecConnsAsPseudo begin]\n");
	//addRecConnsAsPseudo();
	//LLVM_DEBUG(dbgs() << "[DFGOpenCGRA.cpp][addRecConnsAsPseudo end]\n\n");
	//LLVM_DEBUG(dbgs() << "\n[DFGOpenCGRA.cpp][changeTypeofSingleSourceCompNodes begin]\n");
	// printDOT(this->name + "_PartPredDFG.dot");
	// printNewDFGXML();
	//LLVM_DEBUG(dbgs() << "[DFGOpenCGRA.cpp][changeTypeofSingleSourceCompNodes end]\n\n");
	LLVM_DEBUG(dbgs() << "\n[DFGOpenCGRA.cpp][removeDisconnectedNodes begin]\n");


	//function removeDisconnetedNodes() should be called at last, otherwise same idx would be reused
	//when new instructions are added
	removeDisconnetedNodes();
	LLVM_DEBUG(dbgs() << "[DFGOpenCGRA.cpp][removeDisconnectedNodes end]\n\n");

}


void DFGOpenCGRA::getLoopExitConditionNodes(std::set<exitNode> &exitNodes)
{
	bool is_ctrl_node_null = false;
	std::unordered_set<BasicBlock *> exitSrcs;

	for (auto it = loopexitBB.begin(); it != loopexitBB.end(); it++)
	{

		BasicBlock *src = it->first;
		BasicBlock *dest = it->second;
		exitSrcs.insert(src);

		LLVM_DEBUG(dbgs() << "loop exit src = " << src->getName() << ", dest = " << dest->getName() << ",");

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
						LLVM_DEBUG(dbgs() << "true path\n");
						// exitNodes.insert(std::make_pair(ctrlNode, true));
						exitNodes.insert(exitNode(ctrlNode, true, BRI->getParent(), dest));
					}
					else
					{
						LLVM_DEBUG(dbgs() << "false path\n");
						exitNodes.insert(exitNode(ctrlNode, false, BRI->getParent(), dest));
					}
				}
			}
		}
		else
		{
			LLVM_DEBUG(dbgs() << "BRI BasicBlock = " << BRI->getParent()->getName() << "\n");

			auto ThisBasicBlockDTNode = DT->getNode(BRI->getParent());
			auto IDOM_DTNode = ThisBasicBlockDTNode->getIDom();
			BasicBlock *IDOM = IDOM_DTNode->getBlock();

			BranchInst *IDOM_BRI = static_cast<BranchInst *>(IDOM->getTerminator());
			LLVM_DEBUG(dbgs() << "IDOM BRI BasicBlock = " << IDOM_BRI->getParent()->getName() << "\n");
			assert(IDOM_BRI->isConditional());

			bool isTruePath = true;
			for (int j = 0; j < IDOM_BRI->getNumSuccessors(); ++j)
			{
				if (DT->dominates(IDOM_BRI->getSuccessor(j), BRI->getParent()))
				{
					if (j == 0)
					{
						LLVM_DEBUG(dbgs() << "true path\n");
						// exitNodes.insert(std::make_pair(ctrlNode, true));
						isTruePath = true;
					}
					else
					{
						LLVM_DEBUG(dbgs() << "false path\n");
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

void DFGOpenCGRA::connectBBTrig() {


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
				LLVM_DEBUG(dbgs() << "Br=" << BR->getIdx() << ",Ins=");
				LLVM_DEBUG(BRI->dump());
				LLVM_DEBUG(dbgs() << "Ancestor Count=" << BR->getAncestors().size() << "\n");
				LLVM_DEBUG(dbgs() << "Ancestors=");
				for(dfgNode* par : BR->getAncestors()){
					LLVM_DEBUG(dbgs() << par->getIdx() << ",BB=" << par->BB->getName() <<":");
					if(par->getNode()) LLVM_DEBUG(par->getNode()->dump());
				}
			}
			assert(BR->getAncestors().size() == 1);
			BrParentMap[parentBB]=BR->getAncestors()[0];
			BrParentMapInv[BR->getAncestors()[0]]=parentBB;

			LLVM_DEBUG(dbgs() << "ConnectBB :: From=" << BrParentMap[parentBB]->getIdx() << "(" << parentBB->getName() << ")" << ",To=");
			for(dfgNode* succNode : leaves){
				LLVM_DEBUG(dbgs() << succNode->getIdx() << ",");
				assert(succNode->getNameType() != "OutLoopLOAD");
			}
			LLVM_DEBUG(dbgs() << ",destBB = " << childBB->getName());
			LLVM_DEBUG(dbgs() << "\n");

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



void DFGOpenCGRA::createCtrlBROrTree() {

	LLVM_DEBUG(dbgs() << "createCtrlBROrTree STARTED!\n");

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

					LLVM_DEBUG(dbgs() << "Connecting pp1 = " << pp1->getIdx() << ",");
					LLVM_DEBUG(dbgs() << "pp2 = " << pp2->getIdx() << ",");


					dfgNode* temp = new dfgNode(this);
					temp->setIdx(5000 + NodeList.size());
					temp->setNameType("CTRLBrOR");
					temp->BB = pp1->BB;
					NodeList.push_back(temp);

					LLVM_DEBUG(dbgs() << "newNode = " << temp->getIdx() << "\n");

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
					LLVM_DEBUG(dbgs() << "Alone node = " << pp1->getIdx() << "\n");
				}

			}
		}
	}

	LLVM_DEBUG(dbgs() << "createCtrlBROrTree ENDED!\n");

}


void DFGOpenCGRA::printDOT(std::string fileName) {
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


void DFGOpenCGRA::scheduleASAP() {

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
		//cout<< n->getIdx() <<":"<< n->getRecAncestors().size() << endl;
		if(n->getAncestors().size() == 0){// || n->getRecAncestors().size()!=0 ){
			qv.push_back(n);
		}
		//		}
	}

	q.push(qv);

	int level = 0;

	std::set<dfgNode*> visitedNodes;

	LLVM_DEBUG(dbgs() << "Schedule ASAP ..... \n");


	while(!q.empty()){
		std::vector<dfgNode*> q_element = q.front(); q.pop();
		std::vector<dfgNode*> nqv; nqv.clear();

		std::set<dfgNode*> nqv_set; nqv_set.clear();
		std::pair<std::set<dfgNode*>::iterator,bool> ret;

		if(level > maxASAPLevel) maxASAPLevel = level;


		for(dfgNode* node : q_element){
			visitedNodes.insert(node);

			if(node->getASAPnumber() < level){
				node->setASAPnumber(level);
			}



			for(dfgNode* child : node->getChildren()){
				bool isBackEdge = node->childBackEdgeMap[child];
				if(!isBackEdge){
					//					nqv.push_back(child);
					ret = nqv_set.insert(child);
					if(ret.second == true){
						nqv.push_back(child);
					}
				}
			}

		}
		if(!nqv.empty()){
			q.push(nqv);
		}
		level++;
	}
	LLVM_DEBUG(dbgs() << "Visited nodes size : " << visitedNodes.size() << ", Nodes list size:" << NodeList.size() );
	//cout << "Visited nodes size : " << visitedNodes.size() << ", Nodes list size:" << NodeList.size() << endl;

	assert(visitedNodes.size() == NodeList.size());

}

void DFGOpenCGRA::scheduleALAP() {

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

	LLVM_DEBUG(dbgs() << "Schedule ALAP ..... \n");

	while(!q.empty()){
		assert(level >= 0);
		std::vector<dfgNode*> q_element = q.front(); q.pop();
		std::vector<dfgNode*> nq; nq.clear();

		std::set<dfgNode*> nq_set; nq_set.clear();
		std::pair<std::set<dfgNode*>::iterator,bool> ret;

		for(dfgNode* node : q_element){
			visitedNodes.insert(node);

			if(node->getALAPnumber() > level || node->getALAPnumber() == -1){
				node->setALAPnumber(level);
			}

			for(dfgNode* parent : node->getAncestors()){
				if(parent->childBackEdgeMap[node]) continue;

				//				nq.push_back(parent);
				ret = nq_set.insert(parent);
				if(ret.second == true){
					nq.push_back(parent);
				}
			}
		}

		if(!nq.empty()) q.push(nq);

		level--;
	}


	assert(visitedNodes.size() == NodeList.size());
}


int DFGOpenCGRA::classifyParents() {

	dfgNode* node;
	int parent_count=0;
	for (int i = 0; i < NodeList.size(); ++i) {
		node = NodeList[i];

		LLVM_DEBUG(dbgs() << "classifyParent::currentNode = " << node->getIdx() << "\n");
		LLVM_DEBUG(dbgs() << "Parents : ");
		parent_count=0;
		for (dfgNode* parent : node->getAncestors()) {
			LLVM_DEBUG(dbgs() << parent->getIdx() << ",");
			parent_count++;
		}
		LLVM_DEBUG(dbgs() << "\n");

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
							//addLoopExitStoreHyCUBE might already set the classification
							//assert(node->parentClassification.find(1) == node->parentClassification.end());
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
						//							assert(node->parentClassification.find(0) == node->parentClassification.end());
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
				if(parent->getNode() != NULL) LLVM_DEBUG(parent->getNode()->dump());
				LLVM_DEBUG(dbgs() << "node : " << node->getIdx() << "\n");
				LLVM_DEBUG(dbgs() << "Parent : " << parent->getIdx() << "\n");
				LLVM_DEBUG(dbgs() << "Parent NameType : " << parent->getNameType() << "\n");

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
					LLVM_DEBUG(dbgs() << "Extra parent : " << parent->getIdx() << ",Nametype = " << parent->getNameType() << ",par_idx = " << node->parentClassification.size()+1 << "\n");
					node->parentClassification[node->parentClassification.size()+1]=parent;
				}
				continue;
			}

			if( dyn_cast<SelectInst>(ins) ){

				LLVM_DEBUG(dbgs() << "node : " << node->getIdx() << "\n");
				LLVM_DEBUG(dbgs() << "Parent : " << parent->getIdx() << "\n");
				LLVM_DEBUG(dbgs() << "Parent NameType : " << parent->getNameType() << "\n");

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
					LLVM_DEBUG(dbgs() << "Extra parent : " << parent->getIdx() << ",Nametype = " << parent->getNameType() << ",par_idx = " << node->parentClassification.size()+1 << "\n");
					node->parentClassification[node->parentClassification.size()+1]=parent;
				}
				continue;
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
					assert(parent->getAncestors().size() == 1);
					dfgNode* parpar = parent->getAncestors()[0];
					if(OutLoopNodeMapReverse.find(parpar) != OutLoopNodeMapReverse.end()
							&& OutLoopNodeMapReverse[parpar]){
						parentIns = OutLoopNodeMapReverse[parpar];
					}
					else{
						assert(parpar->getNode());
						parentIns = parpar->getNode();
					}
					//						assert(parent->getAncestors().size()<3);
					//						node->parentClassification[1]=parent;
					//						continue;
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
					//							LLVM_DEBUG(dbgs() << "Extra parent : " << parent->getIdx() << ",Nametype = " << parent->getNameType() << ",par_idx = " << node->parentClassification.size()+1 << "\n";
					//							node->parentClassification[node->parentClassification.size()+1]=parent;
					//						}
					//						continue;

					parentIns = cmergePHINodes[parent]->getNode();
					LLVM_DEBUG(dbgs() << "CMERGE Parent" << parent->getIdx() << "to non-nametype child=" << node->getIdx() << ".\n");
					LLVM_DEBUG(dbgs() << "parentIns = "); LLVM_DEBUG(parentIns->dump());
					LLVM_DEBUG(dbgs() << "node = "); LLVM_DEBUG(node->getNode()->dump());
					LLVM_DEBUG(dbgs() << "OpNumber = " << findOperandNumber(node, ins,parentIns) << "\n");

					assert(parentIns);
				}
				if(parent->getNameType().compare("SELECTPHI")==0){
					parentIns = selectPHIAncestorMap[parent]->getNode();
					LLVM_DEBUG(dbgs() << "SELECTPHI Parent" << parent->getIdx() << "to non-nametype child=" << node->getIdx() << ".\n");
					LLVM_DEBUG(dbgs() << "parentIns = "); LLVM_DEBUG(parentIns->dump());
					LLVM_DEBUG(dbgs() << "node = "); LLVM_DEBUG(node->getNode()->dump());
					LLVM_DEBUG(dbgs() << "OpNumber = " << findOperandNumber(node, ins,parentIns) << "\n");

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

			//DMD: Set type I3 to handle single source nodes in mapper
			if(parent_count < 2 && node->hasConstantVal()==false && (HyCUBEInsStrings[node->getFinalIns()].compare("MUL")==0 || HyCUBEInsStrings[node->getFinalIns()].compare("ADD")==0 || HyCUBEInsStrings[node->getFinalIns()].compare("SUB")==0)){
				node->parentClassification.erase(1);node->parentClassification.erase(2);
				node->parentClassification[3]=parent;
				LLVM_DEBUG(dbgs() <<"Has Const:" <<node->hasConstantVal()<<", Single source dest node:" << node->getIdx() << ", source node:" << parent->getIdx() << "\n");
			}


		}
	}

	return 0;
}

void DFGOpenCGRA::addRecConnsAsPseudo() {

	scheduleASAP();
	scheduleALAP();

	for(dfgNode* node : NodeList){
		for(dfgNode* recParent : node->getRecAncestors()){
			LLVM_DEBUG(dbgs() << "Adding rec Pseudo Connection :: parent=" << recParent->getIdx() << ",to" << node->getIdx() << "\n");
			removeEdge(findEdge(recParent,node));
			recParent->addChildNode(node,EDGE_TYPE_PS,false,true,true);
			node->addAncestorNode(recParent,EDGE_TYPE_PS,false,true,true);
		}
	}

	scheduleASAP();
	scheduleALAP();

}


void DFGOpenCGRA::addOrphanPseudoEdges() {

	scheduleASAP();
	scheduleALAP();

	std::set<dfgNode*> RecParents;
	std::map<dfgNode*,int> maxDelayASAP;

	for(dfgNode* node : NodeList){
		for(dfgNode* recParent : node->getRecAncestors()){
			LLVM_DEBUG(dbgs() << "child = " << node->getIdx() << ", recparent = " << recParent->getIdx() << "\n");
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

		LLVM_DEBUG(dbgs() << "Searching for recParent=" << node->getIdx() << "\n");

		//		for(dfgNode* parCand : NodeList){
		//			if(parCand->getASAPnumber() == node->getALAPnumber()-1 && parCand->getALAPnumber() == node->getALAPnumber()-1){
		//				LLVM_DEBUG(dbgs() << "Adding Pseudo Connection :: parent=" << parCand->getIdx() << ",to" << node->getIdx() << "\n";
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
				LLVM_DEBUG(dbgs() << "NO critChild and NO critCand! continue...\n");
				continue;
			}
			LLVM_DEBUG(dbgs() << "No critChild , but critCand = " << critCand->getIdx()  << ",with startDiff = " << startdiff << "\n");
			criticalChild = critCand;
			//			for(dfgNode* n : NodeList){
			//				if(n == node) continue;
			//				if(n->getALAPnumber() == node->getALAPnumber() - 1){
			//
			//					dfgNode* pseduoParent = n;
			//					LLVM_DEBUG(dbgs() << "Adding Pseudo Connection :: parent=" << pseduoParent->getIdx() << ",to" << node->getIdx() << "\n";
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
				LLVM_DEBUG(dbgs() << "n=" << n->getIdx() << ",diff=" << n->getALAPnumber() - n->getASAPnumber() << "\n");
				if(n->getALAPnumber() - n->getASAPnumber() <= startdiff &&
						n->getALAPnumber() < node->getALAPnumber() &&
						(n->getASAPnumber() < maxDelayASAP[node] - 1)
				){
					pseduoParent=n;

					std::vector<dfgNode*> node_childs = node->getChildren();

					if(std::find(node_childs.begin(),node_childs.end(),pseduoParent) == node_childs.end()
							&& !node->isParent(pseduoParent)){
						//pseudoparent is not a child of the node
						LLVM_DEBUG(dbgs() << "Adding Pseudo Connection :: parent=" << pseduoParent->getIdx() << ",to" << node->getIdx() << "\n");
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

		//		LLVM_DEBUG(dbgs() << "Adding Pseudo Connection :: parent=" << pseduoParent->getIdx() << ",to" << node->getIdx() << "\n";
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
		//			LLVM_DEBUG(dbgs() << "Adding Pseudo Connection :: parent=" << latestCousin->getIdx() << ",to" << node->getIdx() << "\n";
		//			latestCousin->addChildNode(node,EDGE_TYPE_PS,false,true,true);
		//			node->addAncestorNode(latestCousin,EDGE_TYPE_PS,false,true,true);
		//			node->parentClassification[0]=latestCousin;
		//		}


	}
	//	assert(false);
	scheduleASAP();
	scheduleALAP();



}

void DFGOpenCGRA::removeDisconnetedNodes() {
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
	LLVM_DEBUG(dbgs() << "POST REMOVAL NODE COUNT = " << NodeList.size() << "\n");
}


void DFGOpenCGRA::printNewDFGXML() {


	std::string fileName = kernelname + "_PartPredDFG.xml";
#ifdef REMOVE_AGI
	fileName = kernelname + "_PartPred_AGI_REMOVED_DFG.xml";
#endif
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
	std::cout << "DFG node count: " << NodeList.size() << "\n";

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

//#ifdef ARCHI_16BIT
		if(node->getNameType() == "LOOPSTART")
			xmlFile << "<BasePointerName size=\"1\">loopstart</BasePointerName>\n";
		if(node->getNameType() == "LOOPEXIT")
			xmlFile << "<BasePointerName size=\"1\">loopend</BasePointerName>\n";
		if(node->getNameType() == "STORESTART")
			xmlFile << "<BasePointerName size=\"1\">loopstart</BasePointerName>\n";
//#endif
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
					else if(dyn_cast<SelectInst>(child->getNode())){

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
							LLVM_DEBUG(dbgs() << "node=" << node->getIdx() << ",child=" << child->getIdx() << "\n");
							assert(false);
						}
					}
				}
			}
			else if(node->getNameType()=="SELECTPHI" && (child != selectPHIAncestorMap[node])){
				if(child->getNode()){
					written = true;
					LLVM_DEBUG(dbgs() << "SELECTPHI :: " << node->getIdx());
					LLVM_DEBUG(dbgs() << ",child = " << child->getIdx() << " | "); LLVM_DEBUG(child->getNode()->dump());
					LLVM_DEBUG(dbgs() << ",phiancestor = " << selectPHIAncestorMap[node]->getIdx() << " | ");
					LLVM_DEBUG(selectPHIAncestorMap[node]->getNode()->dump());

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
						LLVM_DEBUG(dbgs() << "node = " << node->getIdx() << ", child = " << child->getIdx() << "\n");
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
			else if(child->parentClassification[3]==node){
				xmlFile << "type=\"I3\"/>\n";// type to handle single source nodes in mapper
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
					LLVM_DEBUG(dbgs() << "node = " << node->getIdx() <<"child getNameType= " << child->getNameType() << ", child = " << child->getIdx() << "\n");
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

/*
 * DOT file with only operation name inside nodes
 * */
void DFGOpenCGRA::printDOTsimple(std::string fileName) {
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
//			if(node->getNode() != NULL){
//				ofs << node->getNode()->getOpcodeName();
//				ofs << " " << node->getNode()->getName().str() << " ";
//
//				if(node->hasConstantVal()){
//					ofs << " C=" << "0x" << std::hex << node->getConstantVal() << std::dec;
//				}
//
//				if(node->isGEP()){
//					ofs << " C=" << "0x" << std::hex << node->getGEPbaseAddr() << std::dec;
//				}
//
//			}
//			else{
//				ofs << node->getNameType();
//
//				if(node->getNameType() == "OuterLoopLOAD"){
//					ofs << " " << OutLoopNodeMapReverse[node]->getName().str() << " ";
//				}
//
//				if(node->isOutLoop()){
//					ofs << " C=" << "0x" << node->getoutloopAddr() << std::dec;
//				}
//
//				if(node->hasConstantVal()){
//					ofs << " C=" << "0x" << node->getConstantVal() << std::dec;
//				}
//			}

//			ofs << "BB=" << node->BB->getName().str();

//			if(!mutexNodes[node].empty()){
////				ofs << ",mutex={";
////				for(dfgNode* m : mutexNodes[node]){
////					ofs << m->getIdx() << ",";
////				}
////				ofs << "}";
//			}

			if(node->getFinalIns() != NOP){
				ofs << "" << HyCUBEInsStrings[node->getFinalIns()];
			}
//			ofs << ",\n";
//			ofs << node->getIdx() << ", ASAP=" << node->getASAPnumber();
//			ofs << ", ALAP=" << node->getALAPnumber();

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
			if(findEdge(node,child)->getType() == EDGE_TYPE_PS){
				continue;
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
					ofs << "black";
				}
				else if(condition==true){
					ofs << "black";
				}
				else{
					ofs << "black";
				}
			}
			else{
				ofs << "black";

			}

			//			ofs << ", headport=n, tailport=s";
			ofs << "];\n";
		}

//		for(dfgNode* recChild : node->getRecChildren()){
//			ofs << "\"" << "Op_" << node->getIdx() << "\"";
//			ofs << " -> ";
//			ofs << "\"" << "Op_" << recChild->getIdx() << "\"";
//			ofs << "[style = bold, color = black];\n";
//		}
	}

	ofs << "}" << std::endl;
	ofs.close();
}



std::map<BasicBlock*, std::set<std::pair<BasicBlock*, CondVal> > > DFGOpenCGRA::getCtrlInfoBBMorePaths() {


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
			//			LLVM_DEBUG(dbgs() << "curr=" << curr->getName() << "\n";

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
				LLVM_DEBUG(predecessor->getTerminator()->dump());
				BranchInst* BRI = cast<BranchInst>(predecessor->getTerminator());
				//			  BRI->dump();

				if(!BRI->isConditional()){
					//				  LLVM_DEBUG(dbgs() << "\t0 :: ";
					cv = UNCOND;
				}
				else{
					for (int i = 0; i < BRI->getNumSuccessors(); ++i) {
						if(BRI->getSuccessor(i) == curr){
							if(i==0){
								//							  LLVM_DEBUG(dbgs() << "\t1 :: ";
								cv = TRUE;
							}
							else if(i==1){
								//							  LLVM_DEBUG(dbgs() << "\t2 :: ";
								cv = FALSE;
							}
							else{
								assert(false);
							}
						}
					}
				}

				//			  LLVM_DEBUG(dbgs() << "\tPred=" << predecessor->getName() << "(" << dfgNode::getCondValStr(cv) << ")\n";

				visited[predecessor].insert(cv);
				q.push(predecessor);
			}
		}

		LLVM_DEBUG(dbgs() << "BasicBlock : " << BB->getName() << " :: DependentCtrlBBs = ");
		res[BB];
		for(std::pair<BasicBlock*,std::set<CondVal>> pair : visited){
			BasicBlock* bb = pair.first;
			std::set<CondVal> brOuts = pair.second;


			//			if(brOuts.size() == 1){
			//				if(brOuts.find(UNCOND) == brOuts.end()){
			LLVM_DEBUG(dbgs() << bb->getName());
			//					if(brOuts.find(UNCOND) != brOuts.end()){
			//						res[BB].insert(std::make_pair(bb,UNCOND));
			//						LLVM_DEBUG(dbgs() << "(UNCOND),";
			//					}
			if(brOuts.find(TRUE) != brOuts.end()){
				res[BB].insert(std::make_pair(bb,TRUE));
				LLVM_DEBUG(dbgs() << "(TRUE),");
			}
			if(brOuts.find(FALSE) != brOuts.end()){
				res[BB].insert(std::make_pair(bb,FALSE));
				LLVM_DEBUG(dbgs() << "(FALSE),");
			}
			//					res[BB].insert(std::make_pair(bb,cv));
			//				}
			//			}
		}
		LLVM_DEBUG(dbgs() << "\n");
	}

	LLVM_DEBUG(dbgs() << "###########################################################################\n");

	std::map<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> temp1;

	for(std::pair<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> pair : res){
		std::set<std::pair<BasicBlock*,CondVal>> tobeRemoved;
		BasicBlock* currBB = pair.first;
		LLVM_DEBUG(dbgs() << "BasicBlock : " << currBB->getName() << " :: DependentCtrlBBs = ");

		temp1[currBB];
		for(std::pair<BasicBlock*,CondVal> bbVal : pair.second){
			BasicBlock* depBB = bbVal.first;
			for(std::pair<BasicBlock*,CondVal> p2 : res[depBB]){
				tobeRemoved.insert(p2);
			}
		}

		for(std::pair<BasicBlock*,CondVal> bbVal : pair.second){
			if(tobeRemoved.find(bbVal)==tobeRemoved.end()){
				LLVM_DEBUG(dbgs() << bbVal.first->getName());
				LLVM_DEBUG(dbgs() << "(" << dfgNode::getCondValStr(bbVal.second) << "),");
				temp1[currBB].insert(bbVal);
			}
		}
		LLVM_DEBUG(dbgs() << "\n");
	}
	//	return temp1;

	std::map<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> temp2;

	LLVM_DEBUG(dbgs() << "Order=");
	for(std::pair<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> pair : temp1){
		BasicBlock* currBB = pair.first;
		LLVM_DEBUG(dbgs() << currBB->getName() << ",");
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
	LLVM_DEBUG(dbgs() << "\n");

	LLVM_DEBUG(dbgs() << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");

	temp1.clear();
	for(std::pair<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> pair : temp2){
		BasicBlock* currBB = pair.first;
		std::set<std::pair<BasicBlock*,CondVal>> bbValPairs = pair.second;
		LLVM_DEBUG(dbgs() << "BasicBlock : " << currBB->getName() << " :: DependentCtrlBBs = ");
		for(std::pair<BasicBlock*,CondVal> bbVal: bbValPairs){
			LLVM_DEBUG(dbgs() << bbVal.first->getName());
			LLVM_DEBUG(dbgs() << "(" << dfgNode::getCondValStr(bbVal.second) << "),");
		}

		if(!bbValPairs.empty()){
			temp1[currBB] = temp2[currBB];
		}
		else{
			LLVM_DEBUG(dbgs() << "Not added!");
		}
		LLVM_DEBUG(dbgs() << "\n");
	}
	//	assert(false);
	return temp1;

}

/*
 * This function
 * 1) connects PHI nodes with its parents by analyzing PHI instruction[value,basic block][value, basic block]
 * 2) add CMERGE nodes between PHI nodes and their parents (+control conditions)
 * 3) add LOOPSTART control node
 *
 * PHI is implemented as SELECT instruction in HyCUBE
 *
 * SELECT nodes select a valid data input and forward it to output
 * CMERGE nodes generate valid data based on a branch outcome
 *
 * Therefore SELECT nodes should be connected through CMERGE nodes
 *
 * */
int DFGOpenCGRA::handlePHINodes(std::set<BasicBlock*> LoopBB) {

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
				LLVM_DEBUG(PHI->dump());
				assert(false);
			}

			LLVM_DEBUG(dbgs() << "PHI :: ");
			LLVM_DEBUG(PHI->dump());


			for (int i = 0; i < PHI->getNumIncomingValues(); ++i) {
				BasicBlock* bb = PHI->getIncomingBlock(i);
				Value* V = PHI->getIncomingValue(i);
				LLVM_DEBUG(dbgs() << "V :: ");LLVM_DEBUG( V->dump());
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
							// 	LLVM_DEBUG(dbgs() << BRParentBRI->getSuccessor(0)->getName() << "\n";
							// 	LLVM_DEBUG(dbgs() << BRParentBRI->getSuccessor(1)->getName() << "\n";
							// 	LLVM_DEBUG(dbgs() << currBB->getName() << "\n";
							// 	LLVM_DEBUG(dbgs() << previousCTRLNode->getIdx() << "\n";
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
						LLVM_DEBUG(dbgs() << "pcn idx=" << pcn->getIdx() << ",");
						if(pcn->getNode()){
							LLVM_DEBUG(dbgs() << "previousCTRLNode : "); LLVM_DEBUG(pcn->getNode()->dump());
						}
						else{
							LLVM_DEBUG(dbgs() << "\n");
						}
						LLVM_DEBUG(dbgs() << "CTRL VAL : " << cnodectrlvalmap[pcn] << "\n");
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
							LLVM_DEBUG(dbgs() << "mergeNode and control are from the same BB\n");
							mergeNode = phiParent;
							//						if(!isPhiParentOutLoopLoad){
							//							mergeNodeControl[mergeNode]=std::make_pair(previousCTRLNode,incomingCTRLVal);
							//						}
						}
					}
					mergeNodes.push_back(mergeNode);
				}
			}
			LLVM_DEBUG(dbgs() << "DEBUG........... end\n");
			bool recursivePhiNodes=false;

			for(dfgNode* child : node->getChildren()){
				for(dfgNode* mergeNode : mergeNodes){
					if(child == mergeNode){
						LLVM_DEBUG(dbgs() << "recursivePhiNodes found!!!!\n");
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

			LLVM_DEBUG(dbgs() << "USERS ::::::::::::\n");
			for(User* u : PHI->users()){
				LLVM_DEBUG(u->dump());
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
							LLVM_DEBUG(dbgs() << "this is induction variable, node=" << node->getIdx() << ",mergeNode=" << mergeNode->getIdx() << "\n");
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

					LLVM_DEBUG(dbgs() << "PHIChild : " << child->getIdx() << " : ");

					for(dfgNode* mergeNode : mergeNodes){
						LLVM_DEBUG(dbgs() << mergeNode->getIdx() << ",");

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
					LLVM_DEBUG(dbgs() << "\n");
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


dfgNode* DFGOpenCGRA::getStartNode(BasicBlock* BB, dfgNode* PHINode) {
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


dfgNode* DFGOpenCGRA::insertMergeNode(dfgNode* PHINode, dfgNode* ctrl, bool controlVal, dfgNode* data) {
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

dfgNode* DFGOpenCGRA::insertMergeNodeBeforeSEL(dfgNode* SELNode, dfgNode* ctrl, bool controlVal, dfgNode* data) {//for select Node
	dfgNode* temp = new dfgNode(this);

	SmallVector<std::pair<const BasicBlock *, const BasicBlock *>,8> BackedgeBBs;
	//FindFunctionBackedges(*(PHINode->getNode()->getFunction()),BackedgeBBs);
	const BasicBlock* ctrlBB = ctrl->BB;
	const BasicBlock* dataBB = data->BB;
	const BasicBlock* selBB  = SELNode->BB;

	bool isCTRL2PHIBackEdge=false;
	bool isDATA2PHIBackEdge=false;

	isCTRL2PHIBackEdge=false;

	cmergePHINodes[temp]=SELNode;//create cmergeSELNodes?

	isDATA2PHIBackEdge = isCTRL2PHIBackEdge;

	temp->setNameType("CMERGE");

//	if(startNodes.find((BasicBlock*)ctrl->BB) != startNodes.end()){
//		temp->BB = PHINode->BB;
//	}
//	else{
//		temp->BB = ctrl->BB;
//		if(checkBackEdge(ctrl,PHINode) || ctrl->BB == PHINode->BB) backedgeChildMergeNodes.insert(temp);
//	}
	temp->BB = SELNode->BB;

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

dfgNode* DFGOpenCGRA::insertMergeNodeBeforeSEL(dfgNode* SELNode, dfgNode* ctrl,
		bool controlVal, int val) {

	dfgNode* temp = new dfgNode(this);

	SmallVector<std::pair<const BasicBlock *, const BasicBlock *>,8> BackedgeBBs;
	FindFunctionBackedges(*(SELNode->getNode()->getFunction()),BackedgeBBs);
	const BasicBlock* ctrlBB = ctrl->BB;
	const BasicBlock* selBB  = SELNode->BB;

	bool isCTRL2PHIBackEdge=false;

	cmergePHINodes[temp]=SELNode;


	temp->setNameType("CMERGE");

	cmergeCtrlInputs[temp]=ctrl;


//	if(startNodes.find((BasicBlock*)ctrl->BB) != startNodes.end()){
//		temp->BB = SELNode->BB;
//	}
//	else{
//		temp->BB = ctrl->BB;
//		if(checkBackEdge(ctrl,PHINode) || ctrl->BB == PHINode->BB) backedgeChildMergeNodes.insert(temp);
//	}
	temp->BB = SELNode->BB;


	temp->setIdx(NodeList.size());
	NodeList.push_back(temp);

	temp->addAncestorNode(ctrl,EDGE_TYPE_DATA,isCTRL2PHIBackEdge,true,controlVal);
	ctrl->addChildNode(temp,EDGE_TYPE_DATA,isCTRL2PHIBackEdge,true,controlVal);
	temp->setConstantVal(val);
	return temp;
}


dfgNode* DFGOpenCGRA::insertMergeNode(dfgNode* PHINode, dfgNode* ctrl,
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


bool DFGOpenCGRA::checkBackEdge(dfgNode* src, dfgNode* dest) {
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


bool DFGOpenCGRA::checkPHILoop(dfgNode* node1, dfgNode* node2) {
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
			if(is2isParentOf1 & is1isParentOf2) LLVM_DEBUG(dbgs() << "PHI LOOP FOUND!!!!!!\n");
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

