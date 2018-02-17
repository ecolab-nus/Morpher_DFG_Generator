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

//static cl::opt<unsigned> loopNumber("ln", cl::init(0), cl::desc("The loop number to map"));
static cl::opt<std::string> munitName("munit", cl::init("na"), cl::desc("the mapping unit name, e.g. : PRE_LN11, INNERMOST_LN11"));
static cl::opt<std::string> fName("fn", cl::init("na"), cl::desc("the function name"));
static cl::opt<bool> noName("nn", cl::desc("map all functions and loops"));
static cl::opt<unsigned> initMII("ii", cl::init(0), cl::desc("The starting II for the mapping"));
static cl::opt<unsigned> dimX("dx", cl::init(4), cl::desc("DimX"));
static cl::opt<unsigned> dimY("dy", cl::init(4), cl::desc("DimY"));

static std::map<std::string,int> sizeArrMap;

STATISTIC(LoopsAnalyzed, "Number of loops analyzed for vectorization");

static std::set<BasicBlock*> LoopBB;
static std::map<const BasicBlock*,std::vector<const BasicBlock*>> BBSuccBasicBlocks;

static std::map<Loop*,std::vector<BasicBlock*> > loopsExclusieBasicBlockMap;

typedef struct{
	bool isInnerLoop=false;
	Loop* lp;
	std::set<BasicBlock*> allBlocks;
	std::set<std::pair<BasicBlock*,BasicBlock*>> entryBlocks;
	std::set<std::pair<BasicBlock*,BasicBlock*>> exitBlocks;
} MappingUnit;

static std::map<std::string,MappingUnit> mappingUnitMap;
static std::map<std::string,Loop*> mappingUnit2LoopMap;
static std::map<BasicBlock*,std::string> BB2MUnitMap;
std::set<BasicBlock*> nonloopBBs;

typedef struct LoopTree LoopTree;
struct LoopTree{
	Loop* lp;
	std::vector<LoopTree> lpChildren;
};
LoopTree rootLoop;  //current loop is NULL is just a struct capture all the toplevel loops.

std::vector<munitTransition> munitTransitions;
std::vector<munitTransition> munitTransitionsALL;


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

	    	void populateNonLoopBBs(Function& F, std::vector<Loop*> loops){
	    		for(BasicBlock &BB : F){
	    			BasicBlock* BBPtr = cast<BasicBlock>(&BB);
	    			nonloopBBs.insert(BBPtr);
	    		}

	    		for(Loop* lp : loops){
	    			for(BasicBlock* BB : lp->getBlocks()){
	    				nonloopBBs.erase(BB);
	    			}
	    		}
	    	}

	    	void getInnerMostLoops(std::vector<Loop*>* innerMostLoops, std::vector<Loop*> loops, std::map<Loop*,std::string>* loopNames, std::string lnstr, LoopTree* parentLoopTree){
				for (int i = 0; i < loops.size(); ++i) {
					std::stringstream ss;
					ss << lnstr << std::hex << (i+1) << std::dec;
					(*loopNames)[loops[i]]=ss.str();

					LoopTree currLPTree;
					currLPTree.lp=loops[i];
					parentLoopTree->lpChildren.push_back(currLPTree);

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
						mappingUnitMap["INNERMOST_" + ss.str()].isInnerLoop=true;
						mappingUnitMap["INNERMOST_" + ss.str()].lp=loops[i];


						for(BasicBlock* BB : loops[i]->getBlocks()){
							loopsExclusieBasicBlockMap[loops[i]].push_back(BB);
							mappingUnitMap["INNERMOST_" + ss.str()].allBlocks.insert(BB);
							BB2MUnitMap[BB]="INNERMOST_" + ss.str();
						}

//						for(BasicBlock* BB : loops[i]->getBlocks()){
//							for (auto it = pred_begin(BB), et = pred_end(BB); it != et; ++it){
//								BasicBlock* predBB = *it;
//								if(predBB == loops[i]->getHeader()){
//									outs() << "DEBUG::" << "INNERMOST_" + ss.str() << "entry : " << loops[i]->getHeader()->getName() << "\n";
//									mappingUnitMap["INNERMOST_" + ss.str()].entryBlocks.insert(std::make_pair(loops[i]->getHeader(),BB));
//									break;
//								}
//							}
//						}
//
//						for (BasicBlock* BB : mappingUnitMap["INNERMOST_" + ss.str()].allBlocks){
//							for (auto it = succ_begin(BB), et = succ_end(BB); it != et; ++it)
//							{
//							    BasicBlock* succBB = *it;
//								if(std::find(mappingUnitMap["INNERMOST_" + ss.str()].allBlocks.begin(),
//										     mappingUnitMap["INNERMOST_" + ss.str()].allBlocks.end(),
//											 succBB)==mappingUnitMap["INNERMOST_" + ss.str()].allBlocks.end()){
//									mappingUnitMap["INNERMOST_" + ss.str()].exitBlocks.insert(std::make_pair(BB,succBB));
//									break;
//								}
//							}
//						}



					}
					else{
						getInnerMostLoops(innerMostLoops, loops[i]->getSubLoops(), loopNames, ss.str(), &parentLoopTree->lpChildren.back());

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

	    	void printNtabs(int N){
    			for (int i = 0; i < N; ++i) {
					outs() << "\t";
				}
	    	}

	    	void printLoopTree(LoopTree rootLoop, std::map<Loop*,std::string>* loopNames, int tabs=0);
	    	void printLoopTree(LoopTree rootLoop, std::map<Loop*,std::string>* loopNames, int tabs){

	    		struct less_than_lp
	    		{
	    		    inline bool operator() (const LoopTree& lptree1, const LoopTree& lptree2)
	    		    {
	    		    	BasicBlock* lp1Header = lptree1.lp->getHeader();
	    		    	BasicBlock* lp2Header = lptree2.lp->getHeader();
	    		    	if(std::find(BBSuccBasicBlocks[lp1Header].begin(),
	    		    			     BBSuccBasicBlocks[lp1Header].end(),
									 lp2Header) != BBSuccBasicBlocks[lp1Header].end()){
	    		    	//this means lp2 is a succesor of lp1
	    		    		return true;
	    		    	}
	    		    	return false;
	    		    }
	    		};
	    		std::sort(rootLoop.lpChildren.begin(),rootLoop.lpChildren.end(),less_than_lp());


	    		std::vector<BasicBlock*> thisLoopBB;

	    		if(rootLoop.lp == NULL){
	    			for(BasicBlock* BB : nonloopBBs){
	    				thisLoopBB.push_back(BB);
	    			}
	    		}
	    		else{
	    			thisLoopBB = loopsExclusieBasicBlockMap[rootLoop.lp];
	    		}

	    		if(false){
//	    		if(rootLoop.lp == NULL){
	    			outs() << "ROOT LOOP\n";
	    		}
	    		else {
	    			printNtabs(tabs);
	    			if(rootLoop.lp != NULL){
	    				outs() << (*loopNames)[rootLoop.lp] << "******begin\n";
	    			}

//	    			for (BasicBlock* bb : loopsExclusieBasicBlockMap[rootLoop.lp]) {
					for (BasicBlock* bb : thisLoopBB) {
	    				printNtabs(tabs);
	    				outs() << bb->getName() << ",";
	    				bool found=false;
	    				for(LoopTree lt : rootLoop.lpChildren){
	    					std::stringstream ss;
	    					BasicBlock* lpHeader = lt.lp->getHeader();
	    					if(std::find(BBSuccBasicBlocks[bb].begin(),
							             BBSuccBasicBlocks[bb].end(),
							             lpHeader)!=BBSuccBasicBlocks[bb].end() ){
	    						outs() << "PRE_" << (*loopNames)[lt.lp] << ",";
	    						ss << "PRE_" << (*loopNames)[lt.lp];
	    						mappingUnitMap[ss.str()].allBlocks.insert(bb);
	    						mappingUnitMap[ss.str()].lp=rootLoop.lp;
	    						BB2MUnitMap[bb]=ss.str();
	    						found=true;
	    						break;
	    					}
	    				}
	    				if(found){
    	    				outs() << "\n";
	    					continue;
	    				}

	    				std::vector<LoopTree> reverseLpChildren = rootLoop.lpChildren;
	    				std::reverse(reverseLpChildren.begin(),reverseLpChildren.end());
	    				for(LoopTree lt : reverseLpChildren){
	    					std::stringstream ss;
	    					BasicBlock* lpHeader = lt.lp->getHeader();
	    					if(std::find(BBSuccBasicBlocks[lpHeader].begin(),
							             BBSuccBasicBlocks[lpHeader].end(),
							             bb)!=BBSuccBasicBlocks[lpHeader].end() ){
	    						outs() << "POST_" << (*loopNames)[lt.lp] << ",";
	    						ss << "POST_" << (*loopNames)[lt.lp];
	    						mappingUnitMap[ss.str()].allBlocks.insert(bb);
	    						mappingUnitMap[ss.str()].lp=rootLoop.lp;
	    						BB2MUnitMap[bb]=ss.str();
	    						found=true;
	    						break;
	    					}
	    				}
	    				outs() << "\n";
	    			}

	    			for (std::pair<std::string,MappingUnit> pair : mappingUnitMap){
						//for entry blocks
	    				std::string name = pair.first;
//	    				outs() << "DEBUG::mUnit : " << name << "\n";
						for (BasicBlock* BB : mappingUnitMap[name].allBlocks){
							for (auto it = pred_begin(BB), et = pred_end(BB); it != et; ++it)
							{
							  BasicBlock* predBB = *it;
								if(std::find(mappingUnitMap[name].allBlocks.begin(),
											 mappingUnitMap[name].allBlocks.end(),
											 predBB)==mappingUnitMap[name].allBlocks.end()){
//									outs() << "DEBUG::" << predBB->getName() << "is not inside the munit\n";
									mappingUnitMap[name].entryBlocks.insert(std::make_pair(predBB,BB));
								}
							}
						}

						//for exit blocks
						for (BasicBlock* BB : mappingUnitMap[name].allBlocks){
							for (auto it = succ_begin(BB), et = succ_end(BB); it != et; ++it)
							{
							  BasicBlock* succBB = *it;
								if(std::find(mappingUnitMap[name].allBlocks.begin(),
											 mappingUnitMap[name].allBlocks.end(),
											 succBB)==mappingUnitMap[name].allBlocks.end()){
									mappingUnitMap[name].exitBlocks.insert(std::make_pair(BB,succBB));
								}
							}
						}
	    			}


	    			printNtabs(tabs);
	    			if(rootLoop.lp!=NULL){
	    				outs() << (*loopNames)[rootLoop.lp] << "******end\n";
	    			}
	    		}

	    		for (LoopTree child : rootLoop.lpChildren){
	    			printLoopTree(child,loopNames,tabs+1);
	    		}
	    	}

	    	void printMappableUnitMap(){
	    		outs() << "Printing mappable unit map ... \n";
	    		for(std::pair<std::string,MappingUnit> pair : mappingUnitMap){
	    			outs() << pair.first << " :: ";
	    			for (BasicBlock* bb : pair.second.allBlocks){
	    				outs() << bb->getName() << ",";
	    			}
	    			outs() << "|entry=";
	    			for (std::pair<BasicBlock*,BasicBlock*> bbPair : pair.second.entryBlocks){
	    				outs() << bbPair.first->getName() << "to" << bbPair.second->getName() << ",";
	    			}
	    			outs() << "|exit=";
	    			for (std::pair<BasicBlock*,BasicBlock*> bbPair : pair.second.exitBlocks){
	    				outs() << bbPair.first->getName() << "to" << bbPair.second->getName() << ",";
	    			}
	    			outs() << "\n";
	    		}
	    	}

	    	std::string findMUofBB(BasicBlock* BB){
				for(std::pair<std::string,MappingUnit> pair1 : mappingUnitMap){
					if(pair1.second.allBlocks.find(BB)!=pair1.second.allBlocks.end()){
						return pair1.first;
					}
				}
				return "FUNC_BODY";
	    	}


			void dfsmunitTrans(std::pair<std::string,std::string> currEdge,
							   std::vector<std::string> currPath,
					           std::map<std::string,std::map<std::string,std::set<std::string>>>& transbasedScalarTransfers,
							   std::map<std::string,std::map<std::string,std::set<std::vector<std::string>>>>& transbasedNextMunit,
					           std::map<std::string,std::string> varOwner,
							   std::set<std::pair<std::string,std::string>> visitedEdges,
							   std::map<std::string,std::set<std::string>>& munitTrans,
							   std::map<std::string,std::set<std::string>>& varNeeds,
							   int tabs=0){


				if(visitedEdges.find(currEdge)!=visitedEdges.end()){
					return;
				}

//				for(std::pair<std::string,std::string> visitedEdge : visitedEdges){
//					if(currEdge.first.compare(visitedEdge.first)==0){
//						return;
//					}
//				}


				std::string currMunit = currEdge.second;
				for (int i = 0; i < tabs; ++i) {
					outs() << "\t";
				}
				outs() << "prevMunit=" << currEdge.first << ",currMunit=" << currMunit  << "\n";

				for(std::string varName : varNeeds[currMunit]){
					assert(varOwner.find(varName)!=varOwner.end());
					assert(!varOwner[varName].empty());

					for (int i = 0; i < tabs; ++i) {
						outs() << "\t";
					}
					outs() << "varName = " << varName << ",";
					outs() << "PrevOwner =" << varOwner[varName] << ",";

					if(varOwner[varName].compare(currMunit)==0){
						outs() << "isOwned by itself\n";
						continue;
					}
					assert(varOwner[varName].compare(currMunit)!=0);

//					transbasedScalarTransfers[varOwner[varName]][currMunit].insert(varName);

					bool found=false;
					int foundi=0;
					outs() << "currPath.size=" << currPath.size() << ",";
					std::vector<std::string> connectingPath;
					for (int i = 0; i < currPath.size(); ++i) {
						std::string nextMunit;
						if(i==currPath.size()-1){
							nextMunit=currMunit;
						}
						else{
							nextMunit=currPath[i+1];
						}

						if(found){
//							if(transbasedNextMunit[currPath[foundi]][currMunit].back().compare(nextMunit)!=0){
							if(connectingPath.back().compare(nextMunit)!=0){
//								transbasedNextMunit[currPath[foundi]][currMunit].push_back(nextMunit);
								connectingPath.push_back(nextMunit);
							}

							outs() << nextMunit << ",";
						}
						if(currPath[i].compare(varOwner[varName])==0){
							outs() << "foundi=" << i << "," << nextMunit << ",";
							found=true;
							foundi=i;
							transbasedScalarTransfers[currPath[i]][currMunit].insert(varName);
//							transbasedNextMunit[currPath[i]][currMunit].clear();
//							transbasedNextMunit[currPath[i]][currMunit].push_back(nextMunit);
							connectingPath.push_back(nextMunit);
//							break;
						}
					}

					assert(std::find(currPath.begin(),currPath.end(),currEdge.first)!=currPath.end());

					if(found){
						outs() << "NewOwner =" << currMunit << "\n";
						varOwner[varName]=currMunit;
						transbasedNextMunit[currPath[foundi]][currMunit].insert(connectingPath);
					}
					else{
						outs() << "\n";
					}
				}
//
//				for(std::string varName : varNeeds[currMunit]){
//
//				}

				visitedEdges.insert(currEdge);

				if(currMunit.compare("FUNC_BODY")==0)return;
				for(std::string nextMunit : munitTrans[currMunit]){
					assert(nextMunit.compare(currMunit)!=0);
					std::pair<std::string,std::string> nextEdge = std::make_pair(currMunit,nextMunit);
					currPath.push_back(currMunit);
					dfsmunitTrans(nextEdge,currPath,transbasedScalarTransfers,transbasedNextMunit,varOwner,visitedEdges,munitTrans,varNeeds,tabs+1);
				}
				return;
			}

	    	void printFileOutMappingUnitVars(Function &F,
	    									 std::map<std::string,int>* sizeArrMap,
											 std::map<Loop*,std::string> loopNames){

				std::ofstream outVarMapFile;
				std::string fileName = F.getName().str() + ".outMUVar.csv";
				outVarMapFile.open(fileName.c_str());

				std::map<std::string,std::set<std::string>> munitTrans;
				for(munitTransition mut : munitTransitionsALL){
					std::string srcBBStr="FUNC_BODY";
					std::string destBBStr="FUNC_BODY";

					if(BB2MUnitMap.find(mut.srcBB)!=BB2MUnitMap.end()){
						srcBBStr=BB2MUnitMap[mut.srcBB];
					}
					if(BB2MUnitMap.find(mut.destBB)!=BB2MUnitMap.end()){
						destBBStr=BB2MUnitMap[mut.destBB];
					}
					munitTrans[srcBBStr].insert(destBBStr);
				}


				std::set<BasicBlock*> restBasicBlocksFn;
				for(BasicBlock &BB : F){
					BasicBlock* BBPtr = cast<BasicBlock>(&BB);
					restBasicBlocksFn.insert(BBPtr);
				}

				//find basic blocks that does not belong to any mapping unit
				for(std::pair<std::string,MappingUnit> pair1 : mappingUnitMap){
					for(BasicBlock* currBB : pair1.second.allBlocks){
						restBasicBlocksFn.erase(currBB);
					}
				}

				std::map<std::string,std::map<std::string,int>> graphMappingUnits;
				std::map<std::string,std::map<std::string,std::set<std::string>>> graphMappingUnitVarNames;


				for(std::pair<std::string,MappingUnit> pair1 : mappingUnitMap){
					for(std::pair<std::string,MappingUnit> pair2 : mappingUnitMap){
						 if(pair2.first.compare(pair1.first)==0){
							 continue; // skip children in the same mapping unit
						 }
						 graphMappingUnits[pair1.first][pair2.first]=0;
					}
					graphMappingUnits[pair1.first]["FUNC_BODY"]=0;
					graphMappingUnits["FUNC_BODY"][pair1.first]=0;
				}


				for(std::pair<std::string,MappingUnit> pair1 : mappingUnitMap){
					for(BasicBlock* currBB : pair1.second.allBlocks){

						for(Instruction& I : *currBB){
							 for (User *U : I.users()) {
								 if(Instruction* child = dyn_cast<Instruction>(U)){
									 BasicBlock* childBB = child->getParent();

									 for(std::pair<std::string,MappingUnit> pair2 : mappingUnitMap){
										 if(pair2.first.compare(pair1.first)==0){
											 continue; // skip children in the same mapping unit
										 }
										 if(pair2.second.allBlocks.find(childBB)!=pair2.second.allBlocks.end()){
//											 graphMappingUnits[pair1.first][pair2.first]++;
											 graphMappingUnitVarNames[pair1.first][pair2.first].insert(std::to_string((long)&I));
//											 graphMappingUnits[pair2.first][pair1.first]++;
										 }
									 }

									//children that go outside of mapping units to function body
									if(restBasicBlocksFn.find(childBB)!=restBasicBlocksFn.end()){
//										graphMappingUnits[pair1.first]["FUNC_BODY"]++;
										graphMappingUnitVarNames[pair1.first]["FUNC_BODY"].insert(std::to_string((long)&I));
									}
								 }
							 }
						}
					}
				}

				//children of FUNC_BODY
				for (BasicBlock* funcbodyBB : restBasicBlocksFn){
					for(Instruction& I : *funcbodyBB){
						 for (User *U : I.users()) {
							 if(Instruction* child = dyn_cast<Instruction>(U)){
								 BasicBlock* childBB = child->getParent();

								 for(std::pair<std::string,MappingUnit> pair2 : mappingUnitMap){
									 if(pair2.second.allBlocks.find(childBB)!=pair2.second.allBlocks.end()){
//										 graphMappingUnits["FUNC_BODY"][pair2.first]++;
										 graphMappingUnitVarNames["FUNC_BODY"][pair2.first].insert(std::to_string((long)&I));
									 }
								 }
							 }
						 }
					}
				}

				//printing the file
				for(std::pair<std::string,std::set<std::string>> pair1 : munitTrans){
					outVarMapFile << pair1.first << ",(";
					for(std::string destMunit : pair1.second){
						outVarMapFile << destMunit << ",";
					}
					outVarMapFile << ")\n";
				}

				for(std::pair<std::string,std::map<std::string,int>> pair1: graphMappingUnits){
					for( std::pair<std::string,int> pair2 : pair1.second){
						int varSize = graphMappingUnitVarNames[pair1.first][pair2.first].size();
						if(varSize != 0 && mappingUnitMap[pair1.first].lp!=mappingUnitMap[pair2.first].lp){

							std::string loop1Name = "FUNC_BODY";
							if(mappingUnitMap[pair1.first].lp!=NULL) loop1Name = loopNames[mappingUnitMap[pair1.first].lp];

							std::string loop2Name = "FUNC_BODY";
							if(mappingUnitMap[pair2.first].lp!=NULL) loop2Name = loopNames[mappingUnitMap[pair2.first].lp];

							outVarMapFile << pair1.first << "," << loop1Name << ",";
						    outVarMapFile << pair2.first << "," << loop2Name << ",";
							outVarMapFile << varSize << ",";
							for (std::string varName : graphMappingUnitVarNames[pair1.first][pair2.first]){
								outVarMapFile << varName << ",";
							}
							outVarMapFile << "\n";
						}
					}
				}

//				//modified based on the possible transitions
//				std::map<std::string,std::set<std::string>> varNeeds;
//				std::map<std::string,std::string> varOwner;
//				std::map<std::string,std::map<std::string,std::set<std::string>>> transbasedScalarTransfers;
//				std::map<std::string,std::map<std::string,std::set<std::vector<std::string>>>> transbasedNextMunit;
//
//
//				//init owners
//				for (std::pair<std::string,std::map<std::string,std::set<std::string>>> pair1 : graphMappingUnitVarNames){
//					for(std::pair<std::string,std::set<std::string>> pair2 : pair1.second){
//						for(std::string varName : pair2.second){
//							varOwner[varName]=pair1.first;
//							varNeeds[pair2.first].insert(varName);
//							assert(pair1.first.compare(pair2.first)!=0);
//						}
//					}
//				}

				BasicBlock* startBB = cast<BasicBlock>(&F.getEntryBlock());
				BasicBlock* endBB;

				bool rtiFound=false;
				for(BasicBlock& BB : F){
					for(Instruction& I : BB){
						if(ReturnInst* RTI = dyn_cast<ReturnInst>(&I)){
							endBB = RTI->getParent();
							rtiFound=true;
							break;
						}
					}

					if(rtiFound){
						break;
					}
				}


//				std::set<std::pair<std::string,std::string>> visitedEdges;
////				for(std::pair<std::string,std::set<std::string>> pair1 : munitTrans){
//					std::string srcMunit = BB2MUnitMap[startBB];
//					for(std::string destMunit : munitTrans[srcMunit]){
//						std::pair<std::string,std::string> currEdge = std::make_pair(srcMunit,destMunit);
//						std::vector<std::string> currPath;
//						currPath.push_back(srcMunit);
//						dfsmunitTrans(currEdge,currPath,transbasedScalarTransfers,transbasedNextMunit,varOwner,visitedEdges,munitTrans,varNeeds);
//					}
////				}

//				outVarMapFile << "*********SMART_TRANSFERS*********\n";
//
//				for(std::pair<std::string,std::map<std::string,std::set<std::string>>> pair1 : transbasedScalarTransfers){
//					std::string srcMunit = pair1.first;
//					for(std::pair<std::string,std::set<std::string>> pair2 : pair1.second){
////						outVarMapFile << srcMunit << ",";
//						std::string destMunit = pair2.first;
////						outVarMapFile << destMunit << "," << pair2.second.size() << ",";
////						for(std::string varName : pair2.second){
////							outVarMapFile << varName << ",";
////						}
////						for(std::string pathMunit : transbasedNextMunit[srcMunit][destMunit]){
////							outVarMapFile << pathMunit << ",";
////						}
//						for(std::vector<std::string> path : transbasedNextMunit[srcMunit][destMunit]){
//							outVarMapFile << srcMunit << ",";
//							outVarMapFile << destMunit << "," << pair2.second.size() << ",";
//							for(std::string varName : pair2.second){
//								outVarMapFile << varName << ",";
//							}
//							for(std::string pathUnit : path){
//								outVarMapFile << pathUnit << ",";
//							}
//							outVarMapFile << "\n";
//						}
//
////						outVarMapFile << "\n";
//					}
////					outVarMapFile << "\n";
//				}


				//Printing array variables
				std::map<std::string,std::map<std::string,int>> grapharrMappingUnits;
				std::map<std::string,std::map<std::string,std::set<std::string>>> grapharrUnitVarNames;

				for(std::pair<std::string,MappingUnit> pair1 : mappingUnitMap){
					for(std::pair<std::string,MappingUnit> pair2 : mappingUnitMap){
						 if(pair2.first.compare(pair1.first)==0){
							 continue; // skip children in the same mapping unit
						 }
						 grapharrMappingUnits[pair1.first][pair2.first]=0;
					}
					grapharrMappingUnits[pair1.first]["FUNC_BODY"]=0;
					grapharrMappingUnits["FUNC_BODY"][pair1.first]=0;
				}

				std::map<std::string,std::set<std::string>> GEPloadMUMap;
				std::map<std::string,std::set<std::string>> GEPstoreMUMap;
				std::map<std::string,Type*> GEPTypeMap;




				for(BasicBlock &BB : F){
					for(Instruction& I : BB){
						if(GetElementPtrInst* GEP = dyn_cast<GetElementPtrInst>(&I)){
							 outs() << "Fn : " << F.getName() << "\n";
							 GEP->dump();


							 std::string ptrName = GEP->getPointerOperand()->getName().str();
							 outs() << "PtrName = " << ptrName << "\n";

							 Instruction* pointerOp;
							 if(ptrName.empty()){
								 pointerOp = cast<Instruction>(GEP->getPointerOperand());
								 if(LoadInst* pointerOpLD = dyn_cast<LoadInst>(GEP->getPointerOperand())){
									 ptrName =  pointerOpLD->getPointerOperand()->getName().str();
									 outs() << "NewPtrName = " << ptrName << "\n";
								 }
							 }
							 while(ptrName.empty()){
								 pointerOp = cast<Instruction>(pointerOp->getOperand(0));
								 if(GetElementPtrInst* OrignalGEP = dyn_cast<GetElementPtrInst>(pointerOp)){
									 ptrName = OrignalGEP->getPointerOperand()->getName().str();
									 outs() << "NewPtrName = " << ptrName << "\n";
								 }
							 }

							 GEPTypeMap[ptrName]=GEP->getSourceElementType();
							 GEPstoreMUMap[ptrName].insert(BB2MUnitMap[startBB]);
							 for (User *U : GEP->users()) {
								 if(StoreInst* STI = dyn_cast<StoreInst>(U)){
									 GEPstoreMUMap[ptrName].insert(findMUofBB(STI->getParent()));
									 GEPloadMUMap[ptrName].insert(BB2MUnitMap[endBB]);
								 }
								 if(LoadInst* LDI = dyn_cast<LoadInst>(U)){
									 GEPloadMUMap[ptrName].insert(findMUofBB(LDI->getParent()));
								 }
							 }
						}
					}
				}

				std::set<std::pair<Instruction*,std::string>> usesQ;
				for (std::pair<std::string,int> pair1 : *sizeArrMap){
					outs() << "the user given pointer name = " << pair1.first << "\n";
					for(BasicBlock &BB : F){
						for(Instruction& I : BB){
							if(StoreInst* STI = dyn_cast<StoreInst>(&I)) continue;
							if(LoadInst* LDI = dyn_cast<LoadInst>(&I)) continue;

							for (int i = 0; i < I.getNumOperands(); ++i) {
								if(I.getOperand(i)->getName().str().compare(pair1.first) == 0){
									for(User *U : I.users()){
										if(StoreInst* STI = dyn_cast<StoreInst>(U)) continue;
										if(LoadInst* LDI = dyn_cast<LoadInst>(U)) continue;
										if(Instruction* childIns = dyn_cast<Instruction>(U)){
											usesQ.insert(std::make_pair(childIns,pair1.first));
										}
									}
								}
							}
						}
					}
				}

				std::map<std::string,std::string> truePointers;

				outs() << "Collecting True Pointers...\n";
				while(!usesQ.empty()){
					std::pair<Instruction*,std::string> head = *usesQ.begin();
					Instruction* I = head.first;
					I->dump();
					std::string ptrName = head.second;
					usesQ.erase(head);
					truePointers[I->getName().str()]=ptrName;

					for (int i = 0; i < I->getNumOperands(); ++i) {
						for(User *U : I->users()){
							if(StoreInst* STI = dyn_cast<StoreInst>(U)) continue;
							if(LoadInst* LDI = dyn_cast<LoadInst>(U)) continue;
							if(Instruction* childIns = dyn_cast<Instruction>(U)){
								if(truePointers.find(childIns->getName().str())==truePointers.end()){
									usesQ.insert(std::make_pair(childIns,ptrName));
								}
							}
						}
					}
				}

				outs() << "TRUE_POINTERS :: \n";
				for(std::pair<std::string,std::string> tppair : truePointers){
					outs() << tppair.first << "-->" << tppair.second << "\n";
				}


				std::map<std::string,int> arrSizes;

				for (std::pair <std::string,std::set<std::string>> pair1 : GEPstoreMUMap){
					 std::string ptrName = pair1.first;
					 const DataLayout DL = F.getParent()->getDataLayout();
					 Type *T = GEPTypeMap[ptrName];
					 int size;

					T->dump();
					 //Determing array size
					 if(StructType* ST = dyn_cast<StructType>(T)){
						 size=DL.getTypeAllocSize(ST);
					 }
					 else if(ArrayType* AT = dyn_cast<ArrayType>(T)){
						 size=DL.getTypeAllocSize(AT);
					 }
					 else{
						 //TODO: DAC18
//						 IntegerType* IT = cast<IntegerType>(T);

						 if(sizeArrMap->find(ptrName)!=sizeArrMap->end()){
							 size = (*sizeArrMap)[ptrName];
						 }
						 else if(sizeArrMap->find(truePointers[ptrName])!=sizeArrMap->end()){
							 size = (*sizeArrMap)[ptrName];
						 }
						 else{
							errs() << "Please provide sizes for the arrayptr : " << ptrName << "\n";
							assert(0);
						 }
					 }

					 arrSizes[ptrName]=size;

					 for (std::string StoreMUName : pair1.second){
						 for (std::string LoadMUName : GEPloadMUMap[ptrName]){
							 if(StoreMUName.compare(LoadMUName)==0)continue;
							 grapharrMappingUnits[StoreMUName][LoadMUName]+=size;
							 grapharrUnitVarNames[StoreMUName][LoadMUName].insert(ptrName);
						 }
					 }
				}

//				//modified based on transitions
//				//modified based on the possible transitions
//				std::map<std::string,std::set<std::string>> arrNeeds;
//				std::map<std::string,std::string> arrOwner;
//				std::map<std::string,std::map<std::string,std::set<std::string>>> transbasedArrTransfers;
//				std::map<std::string,std::map<std::string,std::set<std::vector<std::string>>>> transbasedArrNextMunit;
//
//
//				//init owners
//				for (std::pair<std::string,std::map<std::string,std::set<std::string>>> pair1 : grapharrUnitVarNames){
//					for(std::pair<std::string,std::set<std::string>> pair2 : pair1.second){
//						for(std::string arrName : pair2.second){
//							arrOwner[arrName]=BB2MUnitMap[startBB];
//							arrNeeds[pair2.first].insert(arrName);
//							arrNeeds[pair1.first].insert(arrName);
//							assert(pair1.first.compare(pair2.first)!=0);
//						}
//					}
//				}
//
//				std::set<std::pair<std::string,std::string>> visitedEdgesArr;
//				std::string srcMunitArr = BB2MUnitMap[startBB];
//				for(std::string destMunit : munitTrans[srcMunitArr]){
//					std::pair<std::string,std::string> currEdge = std::make_pair(srcMunitArr,destMunit);
//					std::vector<std::string> currPath;
//					currPath.push_back(srcMunitArr);
//					dfsmunitTrans(currEdge,currPath,transbasedArrTransfers,transbasedArrNextMunit,arrOwner,visitedEdgesArr,munitTrans,arrNeeds);
//				}

				outVarMapFile << "Arrays Elements : \n";
				for(std::pair<std::string,std::map<std::string,int>> pair1: grapharrMappingUnits){
					for( std::pair<std::string,int> pair2 : pair1.second){
						if(pair2.second != 0){
							std::string loop1Name = "FUNC_BODY";
							if(mappingUnitMap[pair1.first].lp!=NULL) loop1Name = loopNames[mappingUnitMap[pair1.first].lp];

							std::string loop2Name = "FUNC_BODY";
							if(mappingUnitMap[pair2.first].lp!=NULL) loop2Name = loopNames[mappingUnitMap[pair2.first].lp];

							outVarMapFile << pair1.first << "," << loop1Name << ",";
						    outVarMapFile << pair2.first << "," << loop2Name << ",";

							outVarMapFile << pair2.second << ",";

							for(std::string ptrName : grapharrUnitVarNames[pair1.first][pair2.first]){
								outVarMapFile << ptrName << ",";
							}
							outVarMapFile << "\n";
						}
					}
				}

				outVarMapFile << "*********SMART_TRANSFERS*********\n";


				for(std::pair<std::string,int> pair: arrSizes){
					outVarMapFile << pair.first << "=" << pair.second << "\n";
				}
//
//				for(std::pair<std::string,std::map<std::string,std::set<std::string>>> pair1 : transbasedArrTransfers){
//					std::string srcMunit = pair1.first;
//					for(std::pair<std::string,std::set<std::string>> pair2 : pair1.second){
////						outVarMapFile << srcMunit << ",";
//						std::string destMunit = pair2.first;
////						outVarMapFile << destMunit << "," << pair2.second.size() << ",";
////						for(std::string arrName : pair2.second){
////							outVarMapFile << arrName << ",";
////						}
//
//						for(std::vector<std::string> path : transbasedArrNextMunit[srcMunit][destMunit]){
//							outVarMapFile << srcMunit << ",";
//							outVarMapFile << destMunit << "," << pair2.second.size() << ",";
//							for(std::string varName : pair2.second){
//								outVarMapFile << varName << ",";
//							}
//							for(std::string pathUnit : path){
//								outVarMapFile << pathUnit << ",";
//							}
//							outVarMapFile << "\n";
//						}
//
////						for(std::string pathMunit : transbasedArrNextMunit[srcMunit][destMunit]){
////							outVarMapFile << pathMunit << ",";
////						}
////						outVarMapFile << "\n";
//					}
////					outVarMapFile << "\n";
//				}

				outVarMapFile.close();
	    	}

	    	void populateBBTrans(){
	    		int id=0;
	    		for(std::pair<std::string,MappingUnit> pair : mappingUnitMap){
	    			for (std::pair<BasicBlock*,BasicBlock*> entryBBPair : pair.second.entryBlocks){
	    				munitTransition munittrans;
	    				munittrans.srcBB = entryBBPair.first;
						munittrans.destBB = entryBBPair.second;
						munittrans.id=munitTransitions.size();
						munitTransitions.push_back(munittrans);
						munitTransitionsALL.push_back(munittrans);
//	    				for(Instruction &I : *entryBBPair.first){
//	    					if(PHINode* PHI = dyn_cast<PHINode>(&I)){
//	    						for (int i = 0; i < PHI->getNumIncomingValues(); ++i) {
//									BasicBlock* incomingBB = PHI->getIncomingBlock(i);
//
//									if(std::find(pair.second.allBlocks.begin(),
//											     pair.second.allBlocks.end(),
//												 incomingBB) != pair.second.allBlocks.end()){
//										continue;
//									}
//
//									BasicBlock::iterator instIter = --incomingBB->end();
//									Instruction* brIns = &*instIter;
//									assert(brIns->getOpcode() == Instruction::Br);
//
//
//
//								}
//	    					}
//	    				}
	    			}
	    		}

	    		outs() << "MUNIT transitions = " << munitTransitions.size() << "\n";
//	    		std::cin.get();

	    		for(std::pair<std::string,MappingUnit> pair : mappingUnitMap){
	    			for (std::pair<BasicBlock*,BasicBlock*> exitBBPair : pair.second.exitBlocks){
	    				munitTransition munittrans;
	    				munittrans.srcBB = exitBBPair.first;
						munittrans.destBB = exitBBPair.second;
						munittrans.id=munitTransitionsALL.size();
						munitTransitionsALL.push_back(munittrans);
	    			}
	    		}
	    	}

	    	int getOptimumDim(unsigned int* dimX, unsigned int* dimY, int numNodes, int numMemNodes){
	    		int x;
	    		int y;
	    		int numMemPEs;

	    		int bestWastage = 1000000;
	    		for (x = 1; x <= 4; ++x) {
		    		for (y = 1; y <= 4; ++y) {
		    			numMemPEs = y;
		    			int memII = (numMemNodes+numMemPEs)/numMemPEs;
		    			int resII = (numNodes+x*y)/(x*y);
		    			int II = std::max(memII,resII);
		    			int wastage = II*y*x - numNodes;

		    			if(wastage < bestWastage){
		    				*dimX = x;
		    				*dimY = y;
		    				bestWastage = wastage;
		    			}
					}
				}
	    		return bestWastage;
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
											//TODO : DAC18
											case CmpInst::FCMP_OEQ:
											case CmpInst::FCMP_UEQ:

												break;
											case CmpInst::ICMP_NE:
											//TODO : DAC18
											case CmpInst::FCMP_ONE:
											case CmpInst::FCMP_UNE:
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
											//TODO : DAC18
											case CmpInst::FCMP_OGE:
											case CmpInst::FCMP_UGE:
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
												//TODO : DAC18
											case CmpInst::FCMP_OGT:
											case CmpInst::FCMP_UGT:

												break;
											case CmpInst::ICMP_SLT:
											case CmpInst::ICMP_ULT:
												//TODO : DAC18
											case CmpInst::FCMP_OLT:
											case CmpInst::FCMP_ULT:

												break;
											case CmpInst::ICMP_SLE:
											case CmpInst::ICMP_ULE:
												//TODO : DAC18
											case CmpInst::FCMP_OLE:
											case CmpInst::FCMP_ULE:
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

	    	void analyzeAllMappingUnits(Function &F,
	    			                    std::map<Loop*,std::string> loopNames){


	    		struct basicblockInfo{
	    			int noNodes;
	    			std::string mappingunitName;
	    			std::string loopName;
	    		};

	    		std::map<BasicBlock*,basicblockInfo> BasicBlockNN;

	    		for (std::pair<std::string,MappingUnit> pair : mappingUnitMap){
	    			 std::string munitName = pair.first;
	    			 if(munitName.compare("FUNC_BODY")==0)continue;
	    			 if(pair.second.lp==NULL)continue;

	    			 outs() << "analyzeAllMappingUnits ::" << munitName << "\n";
	    			 std::map<Instruction*,int> insMap;

	    			 DFG LoopDFG("test" + F.getName().str() + "_" + munitName,&loopNames);
	    			 LoopDFG.sizeArrMap = sizeArrMap;
	    			 LoopDFG.setLoopBB(mappingUnitMap[munitName].allBlocks,
									   mappingUnitMap[munitName].entryBlocks,
	    							   mappingUnitMap[munitName].exitBlocks);


	    			 insMap.clear();
				  	 for (BasicBlock* B : *LoopDFG.getLoopBB()){
	    			//			  	  BasicBlock *B = *bb;
						 int Icount = 0;
						 for (auto &I : *B) {

							 if(insMap.find(&I) != insMap.end()){
								 continue;
							 }

							  int depth = 0;
							  traverseDefTree(&I, depth, &LoopDFG, &insMap,BBSuccBasicBlocks,*LoopDFG.getLoopBB());
						 }
					  }
					  LoopDFG.addPHIChildEdges();
					  LoopDFG.connectBB();
					  printDFGDOT (LoopDFG.getName() + "_loopdfg.dot", &LoopDFG);

					  LoopDFG.handlePHINodes(mappingUnitMap[munitName].allBlocks);

					  LoopDFG.removeRedEdgesPHI();
					  LoopDFG.addCMERGEtoSELECT();
					  LoopDFG.handlestartstop_munit(munitTransitions);

					  LoopDFG.treatFalsePaths();
					  LoopDFG.insertshiftGEPs();
					  LoopDFG.addMaskLowBitInstructions();
					  LoopDFG.checkSanity();
					  printDFGDOT (LoopDFG.getName() + "_loopdfg.dot", &LoopDFG);

					  LoopDFG.scheduleASAP();
					  LoopDFG.scheduleALAP();
					  LoopDFG.balanceASAPALAP();
					  LoopDFG.CreateSchList();
					  LoopDFG.printXML();
					  LoopDFG.handleMEMops();
					  LoopDFG.partitionMemNodes();

					  for(dfgNode* node : LoopDFG.getNodes()){

						  BasicBlock* currBB = (BasicBlock*)node->BB;
						  if(node->getNameType().compare("LOOPEXIT")==0){
							  currBB = (BasicBlock*)node->getAncestors()[0]->BB;
						  }

						  if( BasicBlockNN.find(currBB) == BasicBlockNN.end() ){
							  basicblockInfo bbInfoIns;
							  bbInfoIns.noNodes=1;
							  bbInfoIns.mappingunitName=munitName;
							  bbInfoIns.loopName=loopNames[pair.second.lp];
							  BasicBlockNN[currBB]=bbInfoIns;

							  node->printName();
							  outs() << "Adding...\n";
							  outs() << "currBB name = " << currBB->getName().str() << "\n";
							  outs() << " munitName = " << munitName << "\n";
							  outs() << " loopNames[pair.second.lp] = " << loopNames[pair.second.lp] << "\n";
						  }
						  else{
							  node->printName();
							  outs() << "Incrementing...\n";
							  outs() << "currBB name = " << currBB->getName().str() << "\n";
							  outs() << "BasicBlockNN[currBB].mappingunitName = " << BasicBlockNN[currBB].mappingunitName;
							  outs() << " munitName = " << munitName << "\n";
							  outs() << "BasicBlockNN[currBB].loopName = " << BasicBlockNN[currBB].loopName;
							  outs() << " loopNames[pair.second.lp] = " << loopNames[pair.second.lp] << "\n";
							  assert(BasicBlockNN[currBB].mappingunitName.compare(munitName)==0);
							  assert(BasicBlockNN[currBB].loopName.compare(loopNames[pair.second.lp])==0);
							  BasicBlockNN[currBB].noNodes = BasicBlockNN[currBB].noNodes+1;
						  }
					  }
	    		}


				std::ofstream BasicBlockNNFile;
				BasicBlockNNFile.open("BasicBlockNNFile.log");
	    		for(std::pair<BasicBlock*,basicblockInfo> pair : BasicBlockNN){
	    			BasicBlockNNFile << pair.first->getName().str() << ",";
	    			BasicBlockNNFile << pair.second.noNodes << ",";
	    			BasicBlockNNFile << pair.second.loopName << ",";
	    			BasicBlockNNFile << pair.second.mappingunitName << "\n";
	    		}
	    		BasicBlockNNFile.close();
	    	}

	    	void loopTrace(std::map<Loop*,std::string> loopNames, Function& F, LoopTree rootLoop){

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

	    		Constant* loopBBInsUpdateFn = F.getParent()->getOrInsertFunction(
	    								"loopBBInsUpdate",
	    								FunctionType::getVoidTy(Ctx),
										Type::getInt8PtrTy(Ctx),
										Type::getInt8PtrTy(Ctx),
										Type::getInt32Ty(Ctx),
	    								NULL);

	    		Constant* loopInsClearFn = F.getParent()->getOrInsertFunction(
	    								"loopInsClear",
	    								FunctionType::getVoidTy(Ctx),
										Type::getInt8PtrTy(Ctx),
	    								NULL);

	    		Constant* loopBBInsClearFn = F.getParent()->getOrInsertFunction(
	    								"loopBBInsClear",
	    								FunctionType::getVoidTy(Ctx),
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

	    		Constant* updateLoopPreHeaderFn = F.getParent()->getOrInsertFunction(
	    								"updateLoopPreHeader",
	    								FunctionType::getVoidTy(Ctx),
										Type::getInt8PtrTy(Ctx),
										Type::getInt8PtrTy(Ctx),
	    								NULL);

	    		Constant* loopBBMappingUnitUpdate = F.getParent()->getOrInsertFunction(
	    								"loopBBMappingUnitUpdate",
	    								FunctionType::getVoidTy(Ctx),
										Type::getInt8PtrTy(Ctx),
										Type::getInt8PtrTy(Ctx),
	    								NULL);

	    		Constant* recordUncondMunitTransition = F.getParent()->getOrInsertFunction(
	    								"recordUncondMunitTransition",
	    								FunctionType::getVoidTy(Ctx),
										Type::getInt8PtrTy(Ctx),
										Type::getInt8PtrTy(Ctx),
	    								NULL);

	    		Constant* recordCondMunitTransition = F.getParent()->getOrInsertFunction(
	    								"recordCondMunitTransition",
	    								FunctionType::getVoidTy(Ctx),
										Type::getInt8PtrTy(Ctx),
										Type::getInt8PtrTy(Ctx),
										Type::getInt8PtrTy(Ctx),
										Type::getInt1Ty(Ctx),
	    								NULL);

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
	    				Value* BBName = builder.CreateGlobalStringPtr(BB->getName());

	    				builder.SetInsertPoint(BB,--BB->end());
	    				builder.CreateCall(loopInsUpdateFn,{loopName,instructionCountBBVal});
	    				builder.CreateCall(loopBBInsUpdateFn,{loopName,BBName,instructionCountBBVal});
	    			}

	    			SmallVector<BasicBlock*,8> loopExitBlocks;
	    		    lnPair.first->getExitBlocks(loopExitBlocks);
	    		    for(BasicBlock* BB : loopExitBlocks){
	    				builder.SetInsertPoint(BB,--BB->end());
	    				builder.CreateCall(loopInvokeEndFn,{loopName});
	    		    }
	    		}

	    		//adding top most level loop's preheaders
	    		for(LoopTree lt : rootLoop.lpChildren){
	    			BasicBlock* lpPreHeaderBB = lt.lp->getLoopPreheader();
	    			Value* loopName = builder.CreateGlobalStringPtr(loopNames[lt.lp]);

	    			int instructionCountBB = lpPreHeaderBB->getInstList().size();
    				Value* instructionCountBBVal = ConstantInt::get(Type::getInt32Ty(Ctx),instructionCountBB);
    				Value* BBName = builder.CreateGlobalStringPtr(lpPreHeaderBB->getName());

    				builder.SetInsertPoint(lpPreHeaderBB,--lpPreHeaderBB->end());
    				builder.CreateCall(loopBBInsUpdateFn,{loopName,BBName,instructionCountBBVal});
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

	    		std::set<BasicBlock*> srcMunitTrans;
	    		for(munitTransition munitTrans : munitTransitionsALL){
	    			srcMunitTrans.insert(munitTrans.srcBB);
	    		}

	    		for(BasicBlock* srcBB : srcMunitTrans){
	    			for(Instruction &I : *srcBB){
	    				Value* srcBBName = builder.CreateGlobalStringPtr(srcBB->getName());
	    				if(BranchInst* BRI = dyn_cast<BranchInst>(&I)){
	    					if(BRI->isConditional()){
	    						Value* dest1BBName = builder.CreateGlobalStringPtr(BRI->getSuccessor(0)->getName());
	    						Value* dest2BBName = builder.CreateGlobalStringPtr(BRI->getSuccessor(1)->getName());
	    						Value* condition = BRI->getCondition();
	    						builder.SetInsertPoint(BRI);
	    						builder.CreateCall(recordCondMunitTransition,{srcBBName,dest1BBName,dest2BBName,condition});
	    					}
	    					else{
	    						Value* destBBName = builder.CreateGlobalStringPtr(BRI->getSuccessor(0)->getName());
	    						builder.SetInsertPoint(BRI);
	    						builder.CreateCall(recordUncondMunitTransition,{srcBBName,destBBName});
	    					}
	    				}
	    			}
	    		}

	    		builder.SetInsertPoint(&FentryBB,FentryBB.getFirstInsertionPt());
	    		Value* fnName = builder.CreateGlobalStringPtr(F.getName().str());
	    		builder.CreateCall(traceStartFn,{fnName});
				builder.CreateCall(loopBBInsClearFn);

	    		//adding BB and mapping unit name relationship to the instrumentation lib
	    		for(std::pair<std::string,MappingUnit> pair : mappingUnitMap){
	    			for(BasicBlock* BB : pair.second.allBlocks){
	    				Value* BBName = builder.CreateGlobalStringPtr(BB->getName());
	    				Value* MunitName = builder.CreateGlobalStringPtr(pair.first);
	    				builder.CreateCall(loopBBMappingUnitUpdate,{BBName,MunitName});
	    			}
	    		}

	    		//denoting loops preheaders in the instrumentation library
	    		for(std::pair<Loop*,std::string> lnPair : loopNames){
	    			Value* loopName = builder.CreateGlobalStringPtr(lnPair.second);
	    			Value* preHeaderBBName = builder.CreateGlobalStringPtr(lnPair.first->getLoopPreheader()->getName());
	    			builder.CreateCall(updateLoopPreHeaderFn,{loopName,preHeaderBBName});
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

				std::ofstream timeFile;
				std::string timeFileName = "time." + F.getName().str() + ".log";
				timeFile.open(timeFileName.c_str());
				clock_t begin = clock();
				clock_t end;
				std::string loopCFGFileName;


//				  return 0;

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

			  //TODO : DAC18
//			  ReplaceCMPs(F);
			  ParseSizeAttr(F,&sizeArrMap);

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

			  errs() << "Processing : " << F.getName() << "\n";

			  LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
//			  MemoryDependenceAnalysis *MD = &getAnalysis<MemoryDependenceAnalysis>();
			  ScalarEvolution* SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
			  DependenceAnalysis* DA = &getAnalysis<DependenceAnalysis>();




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

			  populateNonLoopBBs(F,loops);
			  std::string lnstr("LN");
			  getInnerMostLoops(&innerMostLoops,loops,&loopNames,lnstr,&rootLoop);
			  errs() << "Number of innermost loops : " << innerMostLoops.size() << "\n";

			  printLoopTree(rootLoop,&loopNames);
			  printMappableUnitMap();
			  populateBBTrans();
			  printFileOutMappingUnitVars(F,&sizeArrMap,loopNames);
			  loopTrace(loopNames,F,rootLoop);
			  analyzeAllMappingUnits(F,loopNames);
			  return 0;

			  if(munitName == "na"){
				  //exit here if a mapping unit name is not given
				  return 0;
			  }

			  //-----------------------------------
			  // New Code for 2017 work
			  //-----------------------------------

			  DFG LoopDFG(F.getName().str() + "_" + munitName,&loopNames);
//			  loopDFGs[L]=&LoopDFG;
			  LoopDFG.setBBSuccBasicBlocks(BBSuccBasicBlocks);
//			  LoopDFG.setLoop(L);

			  outs() << "Currently mapping unit : " << munitName << "\n";
			  assert(!mappingUnitMap[munitName].allBlocks.empty());
//			  LoopDFG.setLoopBB(mappingUnitMap[munitName.ValueStr.str()].allBlocks);
			  LoopDFG.setLoopBB(mappingUnitMap[munitName].allBlocks,
					  	  	    mappingUnitMap[munitName].entryBlocks,
								mappingUnitMap[munitName].exitBlocks);
//			  LoopDFG.setLoopIdx(loopCounter);

			  insMap.clear();
//				  for (Loop::block_iterator bb = L->block_begin(); bb!= L->block_end(); ++bb){
//			  for (std::set<BasicBlock*>::iterator bb = LoopDFG.getLoopBB().begin(); bb!=LoopDFG.getLoopBB().end();++bb){
			  for (BasicBlock* B : *LoopDFG.getLoopBB()){
//			  	  BasicBlock *B = *bb;
				 int Icount = 0;
				 for (auto &I : *B) {

					 if(insMap.find(&I) != insMap.end()){
						 continue;
					 }

					  int depth = 0;
					  traverseDefTree(&I, depth, &LoopDFG, &insMap,BBSuccBasicBlocks,*LoopDFG.getLoopBB());
				 }
			  }
			  LoopDFG.addPHIChildEdges();
			  LoopDFG.connectBB();
			  printDFGDOT (LoopDFG.getName() + "_loopdfg.dot", &LoopDFG);

			  LoopDFG.handlePHINodes(mappingUnitMap[munitName].allBlocks);

			  LoopDFG.removeRedEdgesPHI();
			  LoopDFG.addCMERGEtoSELECT();
			  LoopDFG.handlestartstop_munit(munitTransitions);

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
//		      LoopDFG.addMemRecDepEdgesNew(DA);
			  printDFGDOT (LoopDFG.getName() + "_loopdfg.dot", &LoopDFG);
			  LoopDFG.checkMutexBBs();

			  LoopDFG.scheduleASAP();
			  LoopDFG.scheduleALAP();
			  LoopDFG.balanceASAPALAP();
//				  LoopDFG.addBreakLongerPaths();
			  LoopDFG.CreateSchList();
//				  LoopDFG.MapCGRA(4,4);
			  LoopDFG.printXML();
//				  LoopDFG.printREGIMapOuts();
			  LoopDFG.handleMEMops();
			  LoopDFG.partitionMemNodes();
		      LoopDFG.nameNodes();
		      LoopDFG.printHyCUBEInsHist();
			  return 0;


			  //Checking Instrumentation Code
//				  LoopDFG.GEPInvestigate(F,L,&sizeArrMap);
//				  return true;

//			  std::string Filename = ("cfg." + F.getName() + ".dot").str();
			  //errs() << "Writing '" << Filename << "'...";


//			  raw_fd_ostream File(Filename, EC, sys::fs::F_Text);

			  if (!EC){
				  WriteGraph(File, (const Function*)&F);
			  }
			  else{
				  errs() << "  error opening file for writing!";
			  errs() << "\n";
			  }
			  ArchType arch = NoNOC;
			  printDFGDOT (LoopDFG.getName() + "_loopdfg.dot", &LoopDFG);

//			  unsigned int dX;
//			  unsigned int dY;
//			  getOptimumDim(&dX,&dY,LoopDFG.getNodes().size(),LoopDFG.getMEMOpsToBePlaced());

			  if(!LoopDFG.MapCGRA_SMART(dimX,dimY,arch, 20,initMII)){
				  return false;
			  }
			  LoopDFG.AssignOutLoopAddr();
//			  LoopDFG.GEPInvestigate(F,L,&sizeArrMap);
			  LoopDFG.addPHIParents();
			  LoopDFG.classifyParents();
			  LoopDFG.nameNodes();
//				  LoopDFG.MapCGRA_EMS(4,4,F.getName().str() + "_L" + std::to_string(loopCounter) + "_mapping.log");
			  printDFGDOT (LoopDFG.getName() + "_loopdfg.dot", &LoopDFG);
//				  LoopDFG.printTurns();

			  if((arch != NoNOC)&&(arch != ALL2ALL)){
				  LoopDFG.printOutSMARTRoutes();
//					  LoopDFG.analyzeRTpaths();
				  LoopDFG.printMapping();
			  }

//				  LoopDFG.printCongestionInfo();

			  end = clock();
			  timeFile << F.getName().str() << "_L" << std::to_string(loopCounter) << " time = " << double(end-begin)/CLOCKS_PER_SEC << "\n";
			  loopTrace(loopNames,F,rootLoop);



			  return true;


			  for (int i = 0; i < innerMostLoops.size(); ++i) {
				  Loop *L = innerMostLoops[i];
				  errs() << "*********Loop***********" << "\n";
				  errs() << "\n\n";


				  //Only the innermost Loop
				  assert(L->getSubLoops().size() == 0);

				  // NOTE : if we this is to be used remove this and include as an argument
				 int loopNumber=0;

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
//				  LoopDFG.addBreakLongerPaths();
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
				  LoopDFG.MapCGRA_SMART(dimX,dimY, arch, 20,initMII);
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
//					  LoopDFG.analyzeRTpaths();
					  LoopDFG.printMapping();
				  }

//				  LoopDFG.printCongestionInfo();

				  end = clock();
				  timeFile << F.getName().str() << "_L" << std::to_string(loopCounter) << " time = " << double(end-begin)/CLOCKS_PER_SEC << "\n";

//				  mapParentLoop(&LoopDFG,&loopNames,&loopDFGs);


				  loopCounter++;
			  } //end loopIterator

			  loopTrace(loopNames,F,rootLoop);

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
