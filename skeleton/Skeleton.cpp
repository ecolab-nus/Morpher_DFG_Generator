#include "llvm/Pass.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/CFGPrinter.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"

#include "llvm/Transforms/Scalar.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/CaptureTracking.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/Support/CommandLine.h"

#include "llvm/ADT/GraphTraits.h"

#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/Passes.h"

#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/LoopAccessAnalysis.h"

#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"

//#include "/home/manupa/manycore/llvm-latest/llvm/lib/Transforms/Scalar/GVN.cpp"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <set>


//My Classes
#include "edge.h"
#include "dfgnode.h"
#include "dfg.h"

//#define CDFG

static bool xmlRun = false;

namespace llvm {
	void initializeSkeletonFunctionPassPass(PassRegistry &);
	void initializeSkeletonModulePassPass(PassRegistry &);

	Pass* createskeleton();
}

using namespace llvm;
#define LV_NAME "sfp"
#define DEBUG_TYPE LV_NAME

static cl::opt<unsigned> loopNumber("ln", cl::init(0), cl::desc("The loop number to map"));
static cl::opt<std::string> fName("fn", cl::init("na"), cl::desc("the function name"));
static cl::opt<bool> noName("nn", cl::desc("map all functions and loops"));

static std::map<std::string,int> sizeArrMap;

STATISTIC(LoopsAnalyzed, "Number of loops analyzed for vectorization");

static std::set<BasicBlock*> LoopBB;
static std::map<const BasicBlock*,std::vector<const BasicBlock*>> BBSuccBasicBlocks;

static std::map<Loop*,std::vector<BasicBlock*> > loopsExclusieBasicBlockMap;

	void traverseDefTree(Instruction *I,
				 	 	 int depth,
						 DFG* currBBDFG, std::map<Instruction*,int>* insMapIn,
						 std::map<const BasicBlock*,std::vector<const BasicBlock*>> BBSuccBasicBlocks,
						 std::set<BasicBlock*> validBB,
						 MemoryDependenceAnalysis *MD = NULL){


		 	 	 //errs() << "DEPTH = " << depth << "\n";
//		 	 	 if(insMapIn->find(I) != insMapIn->end())
//		 	 	 {
//					 //errs() << "Instruction = %" << *I << "% is already there\n";
//					 return;
//				 }


		 		SmallVector<std::pair<const BasicBlock *, const BasicBlock *>,1 > BackEdgesBB;
		 		FindFunctionBackedges(*(I->getFunction()),BackEdgesBB);


				 (*insMapIn)[I]++;
	    		 dfgNode curr(I,currBBDFG);
				 currBBDFG->InsertNode(I);
				 dfgNode* currPtr = currBBDFG->findNode(I);

				 if(!dyn_cast<PHINode>(I)){
					 for (Use &V : I->operands()) {
						 if (Instruction *ParIns = dyn_cast<Instruction>(V)) {
							 if(validBB.find(ParIns->getParent()) == validBB.end()){
								 currBBDFG->findNode(I)->addLoadParent(ParIns);
							 }
						 }
					 }
				 }

				  for (User *U : I->users()) {

					if (Instruction *Inst = dyn_cast<Instruction>(U)) {

						errs() << "I :";
						I->dump();
						errs() << "Inst : ";
						Inst->dump();

						//Searching inside basicblocks of the loop
						if(validBB.find(Inst->getParent()) == validBB.end()){
							currBBDFG->findNode(I)->addStoreChild(I);
							continue;
						}


						 if(std::find(BBSuccBasicBlocks[I->getParent()].begin(),BBSuccBasicBlocks[I->getParent()].end(),Inst->getParent())==BBSuccBasicBlocks[I->getParent()].end()){
							 if(Inst->getOpcode() == Instruction::PHI){
								 errs() << "#####TRAVDEFTREE :: PHI Child found1!\n";

								 //TODO :: Please uncomment in order to have phi relationships.
								 // This was done because of EPIMap

								 currBBDFG->findNode(I)->addPHIchild(Inst);
//								 currBBDFG->findNode(Inst)->addPHIancestor(I);
							 }
							 errs() << "line 126, #####TRAVDEFTREE :: backedge found!\n";
							 continue;
						 }

						 std::pair <const BasicBlock*,const BasicBlock*> bbCouple(I->getParent(),Inst->getParent());
						 if(std::find(BackEdgesBB.begin(),BackEdgesBB.end(),bbCouple)!=BackEdgesBB.end()){
							 if(I->getParent() != Inst->getParent()){
								 errs() << "line 112, #####TRAVDEFTREE :: backedge found!\n";
								 continue;
							 }
						 }

						 //TODO :: Handle nicely PHI that use values defined in the same basicblock
						 if(Inst->getOpcode() == Instruction::PHI){
							 if(I->getParent() == Inst->getParent()){
								 //errs() << "Assertion is going to fail\n";
								 //errs() << "Parent : ";
								 errs() << "#####TRAVDEFTREE :: PHI Child found2!\n";

								 //TODO :: Please uncomment in order to have phi relationships
								 // This was done because of EPIMap

								 currBBDFG->findNode(I)->addPHIchild(Inst);
//								 currBBDFG->findNode(Inst)->addPHIancestor(I);
								 I->dump();
								 //errs() << "Child : ";
								 Inst->dump();
								 continue;
							 }
//							 assert(I->getParent() != Inst->getParent());
						 }

						currBBDFG->findNode(I)->addChild(Inst);
					    //errs() << "\t" <<*Inst << "\n";

					  if(insMapIn->find(Inst) == insMapIn->end()){
						traverseDefTree(Inst, depth + 1, currBBDFG, insMapIn,BBSuccBasicBlocks,validBB);
					  }
					  else{
						  //errs() << Inst << " Already found on the map\n";
						  if(currBBDFG->findNode(Inst) == NULL) {
							  //errs() << "This is NULLL....\n";
						  }
					  }

					  currBBDFG->findNode(Inst)->addAncestor(I);
					  //errs() << "Depthfor : " << depth << " returned here!\n";
					}
				  }
				  //errs() << "Depthendfor : " << depth << " returned here!\n";
				  if (I->getOpcode() == Instruction::Br ) {
					  //errs() << "Branch instruction met!\n";
//					  I->getPrevNode()->dump();
					  I->dump();
					  I->getParent()->dump();



#ifdef CDFG
					  std::vector<dfgNode*> rootNodes = currBBDFG->getRoots();
					  for (int i = 0; i < rootNodes.size(); i++){
						  if ((rootNodes[i]->getNode() != I)&&(rootNodes[i]->getNode()->getParent() == I->getParent())){
							  rootNodes[i]->addChild(I,EDGE_TYPE_CTRL);
							  currBBDFG->findNode(I)->addAncestor(rootNodes[i]->getNode());
						  }
					  }
#endif

				  }

				  //errs() << "Depthendfunc : " << depth << "returned here!\n";
	    	}

	    	void printDFGDOT(std::string fileName ,DFG* currBBDFG){
	    		std::ofstream ofs;
	    		ofs.open(fileName.c_str());
	    		dfgNode* node;
	    		int count = 0;

	    		//Write the initial info
	    		ofs << "digraph Region_18 {\n\tgraph [ nslimit = \"1000.0\",\n\torientation = landscape,\n\t\tcenter = true,\n\tpage = \"8.5,11\",\n\tsize = \"10,7.5\" ] ;" << std::endl;

	    		//errs() << "Node List Size : " << currBBDFG->getNodes().size() << "\n";
	    		assert(currBBDFG->getNodes().size() != 0);

				if(currBBDFG->getNodes()[0]->getNode() == NULL) {
					//errs() << "NULLL!\n";
				}


	    		//fprintf(fp_dot, "\"Op_%d\" [ fontname = \"Helvetica\" shape = box, label = \"%d\"] ;\n", i, i);
	//    		std::vector<dfgNode>::iterator ii;
	//    		for(ii = currBBDFG->getNodes().begin(); ii != currBBDFG->getNodes().end() ; ii++ ){

				for (int i = 0 ; i < currBBDFG->getNodes().size() ; i++) {
	    			node = currBBDFG->getNodes()[i];

	    			if(node->getNode() == NULL) {
	    				//errs() << "NULLL! :" << i << "\n";
	    			}

//	    			Instruction* ins = node->getNode();
	//    			//errs() << "\"Op_" << *ins << "\" [ fontname = \"Helvetica\" shape = box, label = \"" << *ins << "\"]" << "\n" ;
	    			ofs << "\"Op_" << node->getIdx()  << "\" [ fontname = \"Helvetica\" shape = box, label = \" ";

	    			if(node->getNode() != NULL){
		    			ofs << node->getNode()->getOpcodeName() ;

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
		    			if(node->isOutLoop()){
		    				ofs << " C=" << "0x" << node->getoutloopAddr() << std::dec;
		    			}

		    			if(node->hasConstantVal()){
		    				ofs << " C=" << "0x" << node->getConstantVal() << std::dec;
		    			}
	    			}

	    			if(node->getFinalIns() != NOP){
	    				ofs << " HyIns=" << currBBDFG->HyCUBEInsStrings[node->getFinalIns()];
	    			}
//	    			for (int j = 0; j < ins->getNumOperands(); ++j) {
//	    				ofs << ins->getOperand(j)->getName().str() << ",";
//					}
//	    			ofs << " ) ";

	    			if(node->getMappedLoc() != NULL){
						ofs << ",\n" << node->getIdx() << ", ASAP=" << node->getASAPnumber()
													 << ", ALAP=" << node->getALAPnumber()
													 << ", (t,y,x)=(" << node->getMappedLoc()->getT() << "," << node->getMappedLoc()->getY() << "," << node->getMappedLoc()->getX() << ")"
													 << ",RT=" << node->getmappedRealTime()
													 << "\"]" << std::endl;
	    			}
	    			else{
						ofs << ",\n" << node->getIdx() << ", ASAP=" << node->getASAPnumber()
													 << ", ALAP=" << node->getALAPnumber()
	//												 << ", (t,y,x)=(" << node.getMappedLoc()->getT() << "," << node.getMappedLoc()->getY() << "," << node.getMappedLoc()->getX() << ")"
													 << "\"]" << std::endl;

	    			}


	    		}

	    		//	fprintf(fp_dot, "{ rank = same ;\n}\n");
	    		ofs << "{ rank = same ;\n}" << std::endl;

	//    		for(ii = currBBDFG->getNodes().begin(); ii != currBBDFG->getNodes().end() ; ii++ ){
				for (int i = 0 ; i < currBBDFG->getNodes().size() ; i++) {
	//    			fprintf(fp_dot, "\"Op_%d\" -> \"Op_%d\" [style = bold, color = red] ;\n", i, j);
	    			node = currBBDFG->getNodes()[i];
//	    			Instruction* destIns;
	//    			std::vector<Instruction*>::iterator cc;
	//    			for(cc = node.getChildren().begin(); cc != node.getChildren().end(); cc++){

	    			int j;
	    			for (j=0 ; j < node->getChildren().size(); j++){
//	    				destIns = node->getChildren()[j]->getNode();
//	    				if(destIns != NULL) {
	    					//errs() << destIns->getOpcodeName() << "\n";
//	    					ofs << "\"Op_" << node.getNode() << "\" -> \"Op_" << destIns << "\" [style = bold, color = red];" << std::endl;

	    					assert(currBBDFG->findEdge(node,node->getChildren()[j])!=NULL);
	    					if(currBBDFG->findEdge(node,node->getChildren()[j])->getType() == EDGE_TYPE_DATA){
	    						ofs << "\"Op_" << node->getIdx() << "\" -> \"Op_" << node->getChildren()[j]->getIdx() << "\" [style = bold, color = red];" << std::endl;
	    					}
	    					else if (currBBDFG->findEdge(node,node->getChildren()[j])->getType() == EDGE_TYPE_CTRL){
	    						ofs << "\"Op_" << node->getIdx() << "\" -> \"Op_" << node->getChildren()[j]->getIdx() << "\" [style = bold, color = black];" << std::endl;
	    					}

//	    				}
	    			}

	    			//adding recurrence edges
	    			for (j=0 ; j < node->getRecChildren().size(); j++){
//	    				destIns = node->getRecChildren()[j];
//	    				if(destIns != NULL) {
	    					//errs() << destIns->getOpcodeName() << "\n";
//	    					ofs << "\"Op_" << node.getNode() << "\" -> \"Op_" << destIns << "\" [style = bold, color = red];" << std::endl;

	    					assert(currBBDFG->findEdge(node,node->getRecChildren()[j])!=NULL);
	    					if(currBBDFG->findEdge(node,node->getRecChildren()[j])->getType() == EDGE_TYPE_LDST){
	    						ofs << "\"Op_" << node->getIdx() << "\" -> \"Op_" << node->getRecChildren()[j]->getIdx() << "\" [style = bold, color = green];" << std::endl;
	    					}

//	    				}
	    			}

	    			//adding phi edges
	    			for (j=0 ; j < node->getPHIchildren().size(); j++){
//	    				destIns = node->getPHIchildren()[j];
//	    				if(destIns != NULL) {
	    					//errs() << destIns->getOpcodeName() << "\n";
//	    					ofs << "\"Op_" << node.getNode() << "\" -> \"Op_" << destIns << "\" [style = bold, color = red];" << std::endl;

	    					assert(currBBDFG->findEdge(node,node->getPHIchildren()[j])!=NULL);
	    					if(currBBDFG->findEdge(node,node->getPHIchildren()[j])->getType() == EDGE_TYPE_PHI){
	    						ofs << "\"Op_" << node->getIdx() << "\" -> \"Op_" << node->getPHIchildren()[j]->getIdx() << "\" [style = bold, color = orange];" << std::endl;
	    					}

//	    				}
	    			}



	    		}

	    		ofs << "}" << std::endl;
	    		ofs.close();
	    	}

	    	Instruction* checkMemDepedency(Instruction *I, MemoryDependenceAnalysis *MD){
				  MemDepResult mRes;
				  //errs() << "#*#*#*#*#* This is a memory op #*#*#*#*#*\n";
				  mRes = MD->getDependency(I);

				  if(mRes.getInst() != NULL){
					  //errs() << "Dependency : \n";
					  mRes.getInst()->dump();
				  }
				  else{
					  //errs() << "Not Dependent or cannot find the dependence : \n";
				  }

				  return mRes.getInst();
	    	}

	    	void dfsBB(SmallVector<std::pair<const BasicBlock *, const BasicBlock *>,1 > BackEdgesBB,
	    			   std::map<const BasicBlock*,std::vector<const BasicBlock*>> *BBSuccBasicBlocksPtr,
					   BasicBlock* currBB,
					   const BasicBlock* startBB
	    			  ){
	    			errs() << "currBB : " << currBB->getName() << "\n";

				succ_iterator SI(succ_begin(currBB)), SE(succ_end(currBB));
				 for (; SI != SE; ++SI){
					 BasicBlock* succ = *SI;

					 std::pair <const BasicBlock*,const BasicBlock*> bbCouple(currBB,succ);
					 if(std::find(BackEdgesBB.begin(),BackEdgesBB.end(),bbCouple)!=BackEdgesBB.end()){
						 continue;
					 }

					 if(std::find((*BBSuccBasicBlocksPtr)[startBB].begin(),(*BBSuccBasicBlocksPtr)[startBB].end(),succ) != (*BBSuccBasicBlocksPtr)[startBB].end()){
						 continue;
					 }

					 (*BBSuccBasicBlocksPtr)[startBB].push_back(succ);
					 dfsBB(BackEdgesBB,BBSuccBasicBlocksPtr,succ,startBB);
				 }
				 return;
	    	}

	    	void printBBSuccMap(Function &F,
	    					    std::map<const BasicBlock*,std::vector<const BasicBlock*>> BBSuccBasicBlocks){

				  std::map<const BasicBlock*,std::vector<const BasicBlock*>>::iterator it;
				  std::ofstream basicblockmapfile;
				  std::string fname = F.getName().str() + "_basicblockmapfile.log";
				  basicblockmapfile.open(fname.c_str());
				  for (it = BBSuccBasicBlocks.begin(); it!=BBSuccBasicBlocks.end(); it++) {
					  basicblockmapfile << "BB::" << it->first->getName().str() << " = ";
					  for (int u = 0; u < it->second.size(); ++u) {
						  basicblockmapfile << it->second[u]->getName().str() << ", ";
					  }
					  basicblockmapfile << "\n";
				  }
				  basicblockmapfile.close();
	    	}

	    	void getInnerMostLoops(std::vector<Loop*>* innerMostLoops, std::vector<Loop*> loops, std::map<Loop*,std::string>* loopNames, std::string lnstr){
				for (int i = 0; i < loops.size(); ++i) {
					std::stringstream ss;
					ss << lnstr << (i+1);
					(*loopNames)[loops[i]]=ss.str();

					outs() << "LoopName : " << ss.str() << "\n";
					  for (Loop::block_iterator bb = loops[i]->block_begin(); bb!= loops[i]->block_end(); ++bb){
						outs() << (*bb)->getName() << ",";
					}
					outs() << "; EXIT=";
					SmallVector<BasicBlock*,8> loopExitBlocks;
					loops[i]->getExitBlocks(loopExitBlocks);
					for (int i = 0; i < loopExitBlocks.size(); ++i) {
						outs() << loopExitBlocks[i]->getName() << ",";
					}

					outs() << "\n";

					if(loops[i]->getSubLoops().size() == 0){
						innerMostLoops->push_back(loops[i]);

						for(BasicBlock* BB : loops[i]->getBlocks()){
							loopsExclusieBasicBlockMap[loops[i]].push_back(BB);
						}
					}
					else{
						getInnerMostLoops(innerMostLoops, loops[i]->getSubLoops(), loopNames, ss.str());

						for(BasicBlock* BB : loops[i]->getBlocks()){
							bool BBfound=false;
							for(std::pair<Loop*,std::vector<BasicBlock*>> pair : loopsExclusieBasicBlockMap){
								if(std::find(pair.second.begin(),pair.second.end(),BB) != pair.second.end()){
									BBfound=true;
									break;
								}
							}
							if(!BBfound){
								loopsExclusieBasicBlockMap[loops[i]].push_back(BB);
							}
						}

					}
				}
	    	}

	    	void ParseSizeAttr(Function &F, std::map<std::string,int>* sizeArrMap){
				  auto global_annos = F.getParent()->getNamedGlobal("llvm.global.annotations");
				  if (global_annos) {
				    auto a = cast<ConstantArray>(global_annos->getOperand(0));
				    for (int i=0; i<a->getNumOperands(); i++) {
				      auto e = cast<ConstantStruct>(a->getOperand(i));

				      if (auto fn = dyn_cast<Function>(e->getOperand(0)->getOperand(0))) {
				        auto anno = cast<ConstantDataArray>(cast<GlobalVariable>(e->getOperand(1)->getOperand(0))->getOperand(0))->getAsCString();
				        fn->addFnAttr("size",anno); // <-- add function annotation here
				      }
				    }
				  }

				  if(F.hasFnAttribute("size")){
					  Attribute attr = F.getFnAttribute("size");
					  outs() << "Size attribute : " << attr.getValueAsString() << "\n";
					  StringRef sizeAttrStr = attr.getValueAsString();
					  SmallVector<StringRef,8> sizeArr;
					  sizeAttrStr.split(sizeArr,',');

					  for (int i = 0; i < sizeArr.size(); ++i) {
						  std::pair<StringRef,StringRef> splitDuple = sizeArr[i].split(':');
						  uint32_t size;
						  splitDuple.second.getAsInteger(10,size);
						  outs() << "ParseAttr:: name:" << splitDuple.first << ",size:" << size << "\n";
 						  (*sizeArrMap)[splitDuple.first.str()]=size;
					  }
				  }

					if (F.hasFnAttribute("size")) {
						outs() << F.getName() << " has my attribute!\n";
					}
	    	}

	    	void ReplaceCMPs(Function &F) {
	    		dfgNode* node;
	    		std::vector<Instruction*> instructions;

	    		for (auto &BB : F){
	    			for(auto &I : BB){
	    				instructions.push_back(&I);
	    			}
				}

	    			for(auto I : instructions){
							if(CmpInst* CI = dyn_cast_or_null<CmpInst>(I)){
								I->dump();
	//	    					if(BI->isConditional()){
	//	    						if(CmpInst* CI = dyn_cast<CmpInst>(&I)){

										IRBuilder<> builder(CI);
					//					builder.SetInsertPoint(&BB,++builder.GetInsertPoint());
										assert(CI->getNumOperands() == 2);
										switch (CI->getPredicate()) {
											case CmpInst::ICMP_EQ:
					//						case CmpInst::FCMP_OEQ:
					//						case CmpInst::FCMP_UEQ:

												break;
											case CmpInst::ICMP_NE:
					//						case CmpInst::FCMP_ONE:
					//						case CmpInst::FCMP_UNE:
											{
												CmpInst* cmpEqNew=cast<CmpInst>(builder.CreateICmpEQ(CI->getOperand(0),CI->getOperand(1)));
												Instruction* notIns=cast<Instruction>(builder.CreateNot(cmpEqNew));
												BasicBlock::iterator ii(CI);
												notIns->removeFromParent();
												ReplaceInstWithInst(CI->getParent()->getInstList(),ii,notIns);
												break;
											}
											case CmpInst::ICMP_SGE:
											case CmpInst::ICMP_UGE:
					//						case CmpInst::FCMP_OGE:
					//						case CmpInst::FCMP_UGE:
											{
												CmpInst* cmpEqNew;
												if(CI->getPredicate() == CmpInst::ICMP_SGE){
													cmpEqNew=cast<CmpInst>(builder.CreateICmpSLT(CI->getOperand(1),CI->getOperand(0)));
												}
												else{ // ICMP_UGE
													cmpEqNew=cast<CmpInst>(builder.CreateICmpULT(CI->getOperand(1),CI->getOperand(0)));
												}
												Instruction* notIns=cast<Instruction>(builder.CreateNot(cmpEqNew));
												BasicBlock::iterator ii(CI);
												notIns->removeFromParent();
												ReplaceInstWithInst(CI->getParent()->getInstList(),ii,notIns);
												break;
											}
											case CmpInst::ICMP_SGT:
											case CmpInst::ICMP_UGT:
					//						case CmpInst::FCMP_OGT:
					//						case CmpInst::FCMP_UGT:

												break;
											case CmpInst::ICMP_SLT:
											case CmpInst::ICMP_ULT:
					//						case CmpInst::FCMP_OLT:
					//						case CmpInst::FCMP_ULT:

												break;
											case CmpInst::ICMP_SLE:
											case CmpInst::ICMP_ULE:
					//						case CmpInst::FCMP_OLE:
					//						case CmpInst::FCMP_ULE:
											{
												CmpInst* cmpEqNew;
												if(CI->getPredicate() == CmpInst::ICMP_SLE){
													cmpEqNew=cast<CmpInst>(builder.CreateICmpSGT(CI->getOperand(1),CI->getOperand(0)));
												}
												else{ // ICMP_ULE
													cmpEqNew=cast<CmpInst>(builder.CreateICmpUGT(CI->getOperand(1),CI->getOperand(0)));
												}
												Instruction* notIns=cast<Instruction>(builder.CreateNot(cmpEqNew));
												BasicBlock::iterator ii(CI);
												notIns->removeFromParent();
												ReplaceInstWithInst(CI->getParent()->getInstList(),ii,notIns);
												break;
											}
												break;
											default:
												assert(0);
												break;
										}
									}
	    			} //iter thru BB

	    	}


	    	void mapParentLoop(DFG* childLoopDFG,std::map<Loop*,std::string>* loopNames, std::map<Loop*,DFG*>* loopDFGs){
	    		assert(childLoopDFG->getLoop()!=NULL);
	    		if(Loop* ParentLoop = childLoopDFG->getLoop()->getParentLoop()){

	    			//currently assume single child loops
	    			assert(ParentLoop->getSubLoops().size()==1);
	    			assert(childLoopDFG->getCGRA()->getMapped());

	    			outs() << "innerLoop Basic Blocks : \n";
	    			for (BasicBlock* BB : (*childLoopDFG->getLoopBB())) {
	    				outs() << BB->getName() << ",";
					}
	    			outs() << "\n innerLoop Basic Blocks end \n";

	    			Function *F = ParentLoop->getHeader()->getParent();
	    			assert(loopNames->find(ParentLoop) != loopNames->end());

	    			DFG* parentLoopDFG;
	    			if(loopDFGs->find(ParentLoop)!=loopDFGs->end()){
	    				outs() << "Parent Loop already existed!\n";
	    				parentLoopDFG = (*loopDFGs)[ParentLoop];
	    			}
	    			else{
	    				outs() << "creating parent loop!\n";
	    				parentLoopDFG = new DFG(F->getName().str()+"_"+ (*loopNames)[ParentLoop],loopNames);
	    				(*loopDFGs)[ParentLoop] = parentLoopDFG;
	    			}

	    			parentLoopDFG->setLoop(ParentLoop);
	    			parentLoopDFG->subLoopDFGs.push_back(childLoopDFG);
	    			parentLoopDFG->accumulatedBBs.insert(childLoopDFG->getLoopBB()->begin(),childLoopDFG->getLoopBB()->end());

	    			std::set<BasicBlock*> ParentLoopBB;
					for (BasicBlock* BB : ParentLoop->getBlocks()){
						ParentLoopBB.insert(BB);
					}

					SmallVector<BasicBlock*,8> loopExitBlocks;
					ParentLoop->getExitBlocks(loopExitBlocks);
					for (int i = 0; i < loopExitBlocks.size(); ++i) {
						ParentLoopBB.insert(loopExitBlocks[i]);
					}

					//remove innerloop blocks
					for (BasicBlock* BB : parentLoopDFG->accumulatedBBs) {
						outs() << "Removing : " << BB->getName() << "\n";
						ParentLoopBB.erase(BB);
					}

	    			outs() << "outerLoop Basic Blocks : \n";
	    			for (BasicBlock* BB : ParentLoopBB) {
	    				outs() << BB->getName() << ",";
					}
	    			outs() << "\n outerLoop Basic Blocks end \n";

	    			//-----------------------
	    			//Mapping the parent loop
	    			//-----------------------
					  parentLoopDFG->setBBSuccBasicBlocks(BBSuccBasicBlocks);
					  parentLoopDFG->setLoopBB(ParentLoopBB);

					  std::map<Instruction*,int> insMap;
					  for (std::set<BasicBlock*>::iterator bb = ParentLoopBB.begin(); bb!=ParentLoopBB.end();++bb){
						 BasicBlock *B = *bb;
						 int Icount = 0;
						 for (auto &I : *B) {

							 if(insMap.find(&I) != insMap.end()){
								 continue;
							 }

							  int depth = 0;
							  traverseDefTree(&I, depth, parentLoopDFG, &insMap,BBSuccBasicBlocks,ParentLoopBB);
						 }
					  }
					  parentLoopDFG->addPHIChildEdges();
					  parentLoopDFG->connectBB();
					  parentLoopDFG->handlePHINodes(ParentLoopBB);
	//				  LoopDFG.handlePHINodeFanIn();
					  parentLoopDFG->checkSanity();
	//				  LoopDFG.addMemDepEdges(MD);
	//				  LoopDFG.removeAlloc();
	//				  LoopDFG.addMemRecDepEdges(DA);
	//				  LoopDFG.addMemRecDepEdgesNew(DA);
					  printDFGDOT (F->getName().str() + (*loopNames)[ParentLoop] + "_loopdfg.dot", parentLoopDFG);

					  parentLoopDFG->scheduleASAP();
					  parentLoopDFG->scheduleALAP();
					  parentLoopDFG->CreateSchList();
	//				  LoopDFG.MapCGRA(4,4);
					  parentLoopDFG->printXML();
	//				  LoopDFG.printREGIMapOuts();
					  parentLoopDFG->handleMEMops();
					  parentLoopDFG->nameNodes();


					  //Checking Instrumentation Code
					  parentLoopDFG->AssignOutLoopAddr();
					  parentLoopDFG->GEPInvestigate(*F,ParentLoop,&sizeArrMap);
	//				  return true;

					  ArchType arch = RegXbarTREG;

					  if(parentLoopDFG->getCGRA()){
						  if(parentLoopDFG->getCGRA()->getMapped()){
							  outs() << "Parent Loop DFG Mapped : true \n";
						  }
						  else{
							  outs() << "Parent Loop DFG Mapped : false \n";
						  }
					  }

					  int initMII = parentLoopDFG->PlaceMacro(childLoopDFG,4,4,arch);
					  parentLoopDFG->MapCGRA_SMART(4,4, arch, 20, initMII);
					  parentLoopDFG->addPHIParents();
	//				  LoopDFG.MapCGRA_EMS(4,4,F.getName().str() + "_L" + std::to_string(loopCounter) + "_mapping.log");
					  printDFGDOT (F->getName().str() + (*loopNames)[ParentLoop] + "_loopdfg.dot", parentLoopDFG);
	//				  LoopDFG.printTurns();

					  if((arch != NoNOC)&&(arch != ALL2ALL)){
						  parentLoopDFG->printOutSMARTRoutes();
						  parentLoopDFG->printMapping();
					  }



	    		}
	    		else{
	    			outs() << "There is not parent loop\n";
	    		}
	    	}

	    	void loopTrace(std::map<Loop*,std::string> loopNames, Function& F){

	    		LLVMContext& Ctx = F.getContext();
	    		BasicBlock& FentryBB = F.getEntryBlock();
	    		IRBuilder<> builder(FentryBB.getFirstNonPHI());

	    		//Function Calls

	    		Constant* traceStartFn = F.getParent()->getOrInsertFunction(
	    								"loopTraceOpen",
	    								FunctionType::getVoidTy(Ctx),
										Type::getInt8PtrTy(Ctx),
	    								NULL);

	    		Constant* traceEndFn = F.getParent()->getOrInsertFunction(
	    								"loopTraceClose",
	    								FunctionType::getVoidTy(Ctx),
	    								NULL);

	    		Constant* loopInvFn = F.getParent()->getOrInsertFunction(
	    								"loopInvoke",
	    								FunctionType::getVoidTy(Ctx),
										Type::getInt8PtrTy(Ctx),
	    								NULL);

	    		Constant* loopInsUpdateFn = F.getParent()->getOrInsertFunction(
	    								"loopInsUpdate",
	    								FunctionType::getVoidTy(Ctx),
										Type::getInt8PtrTy(Ctx),
										Type::getInt32Ty(Ctx),
	    								NULL);

	    		Constant* loopInsClearFn = F.getParent()->getOrInsertFunction(
	    								"loopInsClear",
	    								FunctionType::getVoidTy(Ctx),
										Type::getInt8PtrTy(Ctx),
	    								NULL);

	    		Constant* loopInvokeEndFn = F.getParent()->getOrInsertFunction(
	    								"loopInvokeEnd",
	    								FunctionType::getVoidTy(Ctx),
										Type::getInt8PtrTy(Ctx),
	    								NULL);

	    		Constant* reportExecInsCountFn = F.getParent()->getOrInsertFunction(
	    								"reportExecInsCount",
	    								FunctionType::getVoidTy(Ctx),
										Type::getInt32Ty(Ctx),
	    								NULL);

	    		builder.SetInsertPoint(&FentryBB,FentryBB.getFirstInsertionPt());
	    		Value* fnName = builder.CreateGlobalStringPtr(F.getName().str());
	    		builder.CreateCall(traceStartFn,{fnName});

	    		for(std::pair<Loop*,std::string> lnPair : loopNames){
	    			Value* loopName = builder.CreateGlobalStringPtr(lnPair.second);
	    			Value* loopNameLLVM = builder.CreateGlobalStringPtr(lnPair.second + "-" + lnPair.first->getLoopPreheader()->getName().str());
	    			BasicBlock* loopHeader = lnPair.first->getLoopPreheader();
	    			builder.SetInsertPoint(loopHeader,loopHeader->getFirstInsertionPt());
	    			builder.CreateCall(loopInvFn,{loopName});
	    			builder.CreateCall(loopInsClearFn,{loopName});

	    			for(BasicBlock* BB : loopsExclusieBasicBlockMap[lnPair.first]){
	    				int instructionCountBB = BB->getInstList().size();
	    				Value* instructionCountBBVal = ConstantInt::get(Type::getInt32Ty(Ctx),instructionCountBB);
	    				builder.SetInsertPoint(BB,--BB->end());
	    				builder.CreateCall(loopInsUpdateFn,{loopName,instructionCountBBVal});
	    			}

	    			SmallVector<BasicBlock*,8> loopExitBlocks;
	    		    lnPair.first->getExitBlocks(loopExitBlocks);
	    		    for(BasicBlock* BB : loopExitBlocks){
	    				builder.SetInsertPoint(BB,--BB->end());
	    				builder.CreateCall(loopInvokeEndFn,{loopName});
	    		    }
	    		}

	    		//find return instruction
	    		for(BasicBlock& BB : F){
	    			Value* insCountBBVal = ConstantInt::get(Type::getInt32Ty(Ctx),BB.getInstList().size());
					builder.SetInsertPoint(&BB,--BB.end());
					builder.CreateCall(reportExecInsCountFn,{insCountBBVal});

					for(Instruction& I : BB){
						if(ReturnInst* RI = dyn_cast<ReturnInst>(&I)){
							BasicBlock* retBB = RI->getParent();
							builder.SetInsertPoint(retBB,--retBB->end());
							builder.CreateCall(traceEndFn);
						}
					}
	    		}

	    	}


namespace {

	struct SkeletonFunctionPass : public FunctionPass {
    static char ID;
    SkeletonFunctionPass() : FunctionPass(ID) {
//    	initializeSkeletonFunctionPassPass(*PassRegistry::getPassRegistry());
    }

    	virtual bool runOnFunction(Function &F) {
				std::map<Instruction*,int> insMap;
				std::map<Instruction*,int> insMap2;
				static std::set<const BasicBlock*> funcBB;
				std::error_code EC;

				ReplaceCMPs(F);

				std::ofstream timeFile;
				std::string timeFileName = "time." + F.getName().str() + ".log";
				timeFile.open(timeFileName.c_str());
				clock_t begin = clock();
				clock_t end;
				std::string loopCFGFileName;

				  std::string Filename = ("cfg." + F.getName() + ".dot").str();
				  //errs() << "Writing '" << Filename << "'...";


				  raw_fd_ostream File(Filename, EC, sys::fs::F_Text);

				  if (!EC){
					  WriteGraph(File, (const Function*)&F);
				  }
				  else{
					  errs() << "  error opening file for writing!";
				  errs() << "\n";
				  }

			  //errs() << "In a function calledd " << F.getName() << "!\n";
			  //TODO : please remove this after dtw test
			  if(noName == false){
				  if(fName != "na"){
					  if (F.getName() != fName){
						  errs() << "Function Name : " << F.getName() << "\n";
						  return false;
					  }
				  }
			  }

			  errs() << "Processing : " << F.getName() << "\n";

			  LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
//			  MemoryDependenceAnalysis *MD = &getAnalysis<MemoryDependenceAnalysis>();
			  ScalarEvolution* SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
			  DependenceAnalysis* DA = &getAnalysis<DependenceAnalysis>();

			  ParseSizeAttr(F,&sizeArrMap);

			  const DataLayout &DL = F.getParent()->getDataLayout();
//			  auto *LAA = &getAnalysis<LoopAccessAnalysis>();

			  MemDepResult mRes;

			  int loopCounter = 0;
			  //errs() << F.getName() << "\n";

			  SmallVector<std::pair<const BasicBlock *, const BasicBlock *>,1 > BackEdgesBB;
			  FindFunctionBackedges(F,BackEdgesBB);


			  //errs() << "Starting search of successive basic blocks :\n";

			  int BBCount = 0;
			  for (auto &B : F) {
				  BBCount++;
			  }

			  //errs() << "Total BasicBlocks : " << BBCount << "\n";

			  int currBBIdx = 0;
			  for (auto &B : F) {
				  currBBIdx++;
				  errs() << "Currently proessing = " << currBBIdx << "\n";
				  BasicBlock* BB = dyn_cast<BasicBlock>(&B);
				  funcBB.insert(BB);
				  BBSuccBasicBlocks[BB].push_back(BB);
				  dfsBB(BackEdgesBB,&BBSuccBasicBlocks,BB,BB);
			  }
			  printBBSuccMap(F,BBSuccBasicBlocks);

			  end = clock();
			  timeFile << "Preprocessing Time = " << double(end-begin)/CLOCKS_PER_SEC << "\n";

			  //errs() << "Succesive basic block search completed.\n";

			  //Create a large dfg for the whole function
			  insMap.clear();
//			  DFG funcDFG("funcDFG");
//			  std::vector<DFG> dfgVector;
//			  for (auto &B : F) {
//				  BasicBlock* BB = dyn_cast<BasicBlock>(&B);
//				  for (auto &I : *BB) {
//					  Instruction* ins = I;
//					  traverseDefTree(ins,0,&funcDFG,&insMap,BBSuccBasicBlocks,funcBB);
//				  }
//			  }


			  std::vector<Loop*> innerMostLoops;
			  std::map<Loop*,std::string> loopNames;
			  std::map<Loop*,DFG*> loopDFGs;
			  std::vector<Loop*> loops;
			  for (LoopInfo::iterator i = LI.begin(); i != LI.end() ; ++i){
				  Loop *L = *i;
				  loops.push_back(L);
			  }

			  std::string lnstr("LN");
			  getInnerMostLoops(&innerMostLoops,loops,&loopNames,lnstr);
			  errs() << "Number of innermost loops : " << innerMostLoops.size() << "\n";


			  for (int i = 0; i < innerMostLoops.size(); ++i) {
				  Loop *L = innerMostLoops[i];
				  errs() << "*********Loop***********" << "\n";
				  errs() << "\n\n";


				  //Only the innermost Loop
				  assert(L->getSubLoops().size() == 0);

				 if(noName == false){
					 if(loopCounter != loopNumber){
						 loopCounter++;
						 continue;
					 }
				 }


				 errs() << "The Loop we are dealing with : \n";
				 L->dump();
				 LoopBB.clear();
				  for (Loop::block_iterator bb = L->block_begin(); bb!= L->block_end(); ++bb){
					  (*bb)->dump();
					  LoopBB.insert(*bb);
				  }
				SmallVector<BasicBlock*,8> loopExitBlocks;
				L->getExitBlocks(loopExitBlocks);
				for (int i = 0; i < loopExitBlocks.size(); ++i) {
					loopExitBlocks[i]->dump();
//					LoopBB.insert(loopExitBlocks[i]);
				}
//				LoopBB.insert(L->getLoopPreheader());
//				L->getLoopPreheader()->dump();
				 errs() << "end of the dealing loop....\n";


				  begin = clock();

				  DFG LoopDFG(F.getName().str() + "_L" + std::to_string(loopCounter),&loopNames);
				  loopDFGs[L]=&LoopDFG;
				  LoopDFG.setBBSuccBasicBlocks(BBSuccBasicBlocks);
				  LoopDFG.setLoop(L);
				  LoopDFG.setLoopBB(LoopBB);
				  LoopDFG.setLoopIdx(loopCounter);

				  insMap.clear();
//				  for (Loop::block_iterator bb = L->block_begin(); bb!= L->block_end(); ++bb){
				  for (std::set<BasicBlock*>::iterator bb = LoopBB.begin(); bb!=LoopBB.end();++bb){
					 BasicBlock *B = *bb;
					 int Icount = 0;
					 for (auto &I : *B) {

						 if(insMap.find(&I) != insMap.end()){
							 continue;
						 }

						  int depth = 0;
						  traverseDefTree(&I, depth, &LoopDFG, &insMap,BBSuccBasicBlocks,LoopBB);
					 }
				  }
				  LoopDFG.addPHIChildEdges();
				  LoopDFG.connectBB();
				  LoopDFG.handlePHINodes(LoopBB);

				  LoopDFG.removeRedEdgesPHI();
				  LoopDFG.addCMERGEtoSELECT();
				  LoopDFG.handlestartstop();

//				  LoopDFG.handlePHINodeFanIn();
				  LoopDFG.treatFalsePaths();
				  LoopDFG.insertshiftGEPs();
				  LoopDFG.addMaskLowBitInstructions();
				  //Checking
//				  LoopDFG.nonGEPLoadStorecheck();
				  LoopDFG.checkSanity();
//				  LoopDFG.addMemDepEdges(MD);
//				  LoopDFG.removeAlloc();
//				  LoopDFG.addMemRecDepEdges(DA);
//				  LoopDFG.addMemRecDepEdgesNew(DA);
				  printDFGDOT (F.getName().str() + "_L" + std::to_string(loopCounter) + "_loopdfg.dot", &LoopDFG);

				  LoopDFG.scheduleASAP();
				  LoopDFG.scheduleALAP();
				  LoopDFG.balanceASAPALAP();
				  LoopDFG.CreateSchList();
//				  LoopDFG.MapCGRA(4,4);
				  LoopDFG.printXML();
//				  LoopDFG.printREGIMapOuts();
				  LoopDFG.handleMEMops();
				  LoopDFG.partitionMemNodes();
//				  LoopDFG.nameNodes();


				  //Checking Instrumentation Code
//				  LoopDFG.GEPInvestigate(F,L,&sizeArrMap);
//				  return true;

				  std::string Filename = ("cfg." + F.getName() + ".dot").str();
				  //errs() << "Writing '" << Filename << "'...";


				  raw_fd_ostream File(Filename, EC, sys::fs::F_Text);

				  if (!EC){
					  WriteGraph(File, (const Function*)&F);
				  }
				  else{
					  errs() << "  error opening file for writing!";
				  errs() << "\n";
				  }
				  ArchType arch = RegXbarTREG;
				  printDFGDOT (F.getName().str() + "_L" + std::to_string(loopCounter) + "_loopdfg.dot", &LoopDFG);
				  LoopDFG.MapCGRA_SMART(4,4, arch, 20);
				  LoopDFG.AssignOutLoopAddr();
				  LoopDFG.GEPInvestigate(F,L,&sizeArrMap);
				  LoopDFG.addPHIParents();
				  LoopDFG.classifyParents();
				  LoopDFG.nameNodes();
//				  LoopDFG.MapCGRA_EMS(4,4,F.getName().str() + "_L" + std::to_string(loopCounter) + "_mapping.log");
				  printDFGDOT (F.getName().str() + "_L" + std::to_string(loopCounter) + "_loopdfg.dot", &LoopDFG);
//				  LoopDFG.printTurns();

				  if((arch != NoNOC)&&(arch != ALL2ALL)){
					  LoopDFG.printOutSMARTRoutes();
					  LoopDFG.printMapping();
				  }

//				  LoopDFG.printCongestionInfo();

				  end = clock();
				  timeFile << F.getName().str() << "_L" << std::to_string(loopCounter) << " time = " << double(end-begin)/CLOCKS_PER_SEC << "\n";

//				  mapParentLoop(&LoopDFG,&loopNames,&loopDFGs);


				  loopCounter++;
			  } //end loopIterator

			  loopTrace(loopNames,F);

//			  if(!xmlRun){
//				  DFG xmlDFG("asdsa");
//				  assert(xmlDFG.readXML("epimap_benchmarks/fdctfst/DFG.xml") == 0);
//				  xmlDFG.scheduleASAP();
//				  xmlDFG.scheduleALAP();
//				  xmlDFG.CreateSchList();
//				  xmlDFG.handleMEMops();
//				  xmlDFG.MapCGRA_SMART(4,4,xmlDFG.getName()+ "_mapping.log");
//				  printDFGDOT(xmlDFG.getName() + ".dot",&xmlDFG);
//				  xmlDFG.printREGIMapOuts();
//				  xmlDFG.printTurns();
//				  xmlDFG.printMapping();
//				  xmlDFG.printCongestionInfo();
//			  }
//			  //Assure a single run instead of multiple runs
//			  xmlRun = true;
//
//
//			  timeFile.close();

			  //errs() << "Function body:\n";




			  return true;
			} //END OF runOnFunction





		void getAnalysisUsage(AnalysisUsage &AU) const override {
//			AU.setPreservesAll();
//			AU.addRequired<LoopInfoWrapperPass>();
//			AU.addRequired<MemoryDependenceAnalysis>();

			AU.setPreservesAll();
			AU.addRequired<LoopInfoWrapperPass>();
//			AU.addRequired<MemoryDependenceAnalysis>();
		    AU.addRequired<ScalarEvolutionWrapperPass>();
		    AU.addRequired<AAResultsWrapperPass>();
		    AU.addRequired<DominatorTreeWrapperPass>();
		    AU.addRequired<DependenceAnalysis>();
		    AU.addRequiredID(LoopSimplifyID);
		    AU.addRequiredID(LCSSAID);
		}

	};


}

char SkeletonFunctionPass::ID = 1;


namespace {
	struct SkeletonLoopPass : public LoopPass {
	  static char ID;
	  SkeletonLoopPass() : LoopPass(ID) {}
	  virtual bool runOnLoop(Loop* lp, LPPassManager &LPM) {
		  lp->dump();


		 std::vector<BasicBlock*> blkList = lp->getBlocks();
		 for(BasicBlock* B : blkList){

//			 for (auto &I : *B) {
//				  //errs() << "Instruction: ";
//				  I.dump();
//
//				  for (User *U : I.users()) {
//				    if (Instruction *Inst = dyn_cast<Instruction>(U)) {
//				      //errs() << "\tI is used in instruction:\n";
//				      //errs() << "\t" <<*Inst << "\n";
//				    }
//				  }
//			 }

//			 MemoryDependenceAnalysis *MD = &getAnalysisUsage<MemoryDependenceAnalysis>();
		 }
		  return false;
	  }

	    // We don't modify the program, so we preserve all analyses.
	    void getAnalysisUsage(AnalysisUsage &AU) const override {
	    	AU.setPreservesAll();
	    }

	};
}

char SkeletonLoopPass::ID = 0;


namespace {
	struct SkeletonModulePass : public ModulePass {
		static char ID;
		SkeletonModulePass() : ModulePass(ID) {
			initializeSkeletonModulePassPass(*PassRegistry::getPassRegistry());
		}

		virtual bool runOnModule(Module &M){

		  LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
//		  MemoryDependenceAnalysis *MD = &getAnalysis<MemoryDependenceAnalysis>();

			  for (LoopInfo::iterator i = LI.begin(); i != LI.end() ; ++i){
				  Loop *L = *i;
				  //errs() << "*********Loop***********\n";
				  L->dump();
				  //errs() << "\n\n";
			  }


			return false;
		}

		void getAnalysisUsage(AnalysisUsage &AU) const override {
			AU.setPreservesAll();
			AU.addRequired<LoopInfoWrapperPass>();
//			AU.addRequired<MemoryDependenceAnalysis>();
		    AU.addRequired<ScalarEvolutionWrapperPass>();
		    AU.addRequired<AAResultsWrapperPass>();
		    AU.addRequired<DominatorTreeWrapperPass>();
//		    AU.addRequired<LoopInfoWrapperPass>();
		    AU.addRequired<DependenceAnalysis>();
		    AU.addRequiredID(LoopSimplifyID);
		    AU.addRequiredID(LCSSAID);
//		    AU.addRequired<LoopAccessAnalysis>();
		}

	};
}

char SkeletonModulePass::ID = 2;


//INITIALIZE_PASS_BEGIN(SkeletonFunctionPass, "skeleton", "SkeletonFunctionPass", false, false)
//INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
////INITIALIZE_PASS_DEPENDENCY(MemoryDependenceAnalysis)
//INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
//INITIALIZE_PASS_DEPENDENCY(DependenceAnalysis)
//INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
//INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
//INITIALIZE_PASS_DEPENDENCY(LoopSimplify)
//INITIALIZE_PASS_DEPENDENCY(LCSSA)
//INITIALIZE_PASS_END(SkeletonFunctionPass, "skeleton", "SkeletonFunctionPass", false, false)

//Pass* llvm::createskeleton() {
//	return new SkeletonFunctionPass();
//}


//INITIALIZE_PASS_BEGIN(SkeletonModulePass, "smp", "SkeletonModulePass", false, false)
//INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
//INITIALIZE_PASS_DEPENDENCY(MemoryDependenceAnalysis)
//INITIALIZE_PASS_END(SkeletonModulePass, "smp", "SkeletonModulePass", false, false)

static RegisterPass<SkeletonFunctionPass> X("skeleton", "SkeletonFunctionPass", false, false);

//// Automatically enable the pass.
//// http://adriansampson.net/blog/clangpass.html
//static void registerSkeletonPass(const PassManagerBuilder &,
//                         legacy::PassManagerBase &PM) {
////  PM.add(new SkeletonLoopPass());
////  PM.add(new SkeletonModulePass());
////  PM.add(new GVN());
//  PM.add(new SkeletonFunctionPass());
//}
//static RegisterStandardPasses
//  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
//                 registerSkeletonPass);
