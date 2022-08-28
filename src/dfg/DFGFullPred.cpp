#include <morpherdfggen/dfg/DFGFullPred.h>

void DFGFullPred::connectBBTrig(){

	std::map<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> CtrlBBInfo = getCtrlInfoBBMorePaths();

	for(std::pair<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> p1 : CtrlBBInfo){
		BasicBlock* childBB = p1.first;
		std::vector<dfgNode*> leaves = getLeafs(childBB);
		// std::vector<dfgNode*> leaves = getStoreInstructions(childBB);

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

void DFGFullPred::generateTrigDFGDOT(Function &F){


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

	printDOT(this->name + "_FullPredDFG.dot");

	classifyParents();
	// RemoveInductionControlLogic();

	// RemoveBackEdgePHIs();
	// removeOutLoopLoad();
	RemoveConstantCMERGEs();
	removeDisconnectedNodes();

	addOrphanPseudoEdges();
	addRecConnsAsPseudo();

	printDOT(this->name + "_FullPredDFG.dot");
	printNewDFGXML();
}

void DFGFullPred::printNewDFGXML() {

	std::string fileName = name + "_FullPred_DFG.xml";
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
