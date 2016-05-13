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

#include "llvm/ADT/GraphTraits.h"

#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/Passes.h"

#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/CFG.h"

//#include "/home/manupa/manycore/llvm-latest/llvm/lib/Transforms/Scalar/GVN.cpp"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>


//My Classes
#include "edge.h"
#include "dfgnode.h"
#include "dfg.h"

//#define CDFG



namespace llvm {
	void initializeSkeletonFunctionPassPass(PassRegistry &);
	void initializeSkeletonModulePassPass(PassRegistry &);

	Pass* createskeleton();
}

using namespace llvm;
#define LV_NAME "sfp"
#define DEBUG_TYPE LV_NAME

STATISTIC(LoopsAnalyzed, "Number of loops analyzed for vectorization");



	void traverseDefTree(Instruction *I, int depth, DFG* currBBDFG, std::map<Instruction*,int>* insMapIn,MemoryDependenceAnalysis *MD = NULL){
		 	 	 errs() << "DEPTH = " << depth << "\n";
//		 	 	 if(insMapIn->find(I) != insMapIn->end())
//		 	 	 {
//					 errs() << "Instruction = %" << *I << "% is already there\n";
//					 return;
//				 }

		 		SmallVector<std::pair<const BasicBlock *, const BasicBlock *>,1 > BackEdgesBB;
		 		FindFunctionBackedges(*(I->getFunction()),BackEdgesBB);


				 (*insMapIn)[I]++;
	    		 dfgNode curr(I,currBBDFG);
				 currBBDFG->InsertNode(curr);
				 dfgNode* currPtr = currBBDFG->findNode(I);
				  for (User *U : I->users()) {

					if (Instruction *Inst = dyn_cast<Instruction>(U)) {

						 std::pair <const BasicBlock*,const BasicBlock*> bbCouple(I->getParent(),Inst->getParent());
						 if(std::find(BackEdgesBB.begin(),BackEdgesBB.end(),bbCouple)!=BackEdgesBB.end()){
							 continue;
						 }

						currBBDFG->findNode(I)->addChild(Inst);
					    errs() << "\t" <<*Inst << "\n";

					  if(insMapIn->find(Inst) == insMapIn->end()){
						traverseDefTree(Inst, depth + 1, currBBDFG, insMapIn);
					  }
					  else{
						  errs() << Inst << " Already found on the map\n";
						  if(currBBDFG->findNode(Inst) == NULL) {
							  errs() << "This is NULLL....\n";
						  }
					  }

					  currBBDFG->findNode(Inst)->addAncestor(I);
					  errs() << "Depthfor : " << depth << " returned here!\n";
					}
				  }
				  errs() << "Depthendfor : " << depth << " returned here!\n";
				  if (I->getOpcode() == Instruction::Br ) {
					  errs() << "Branch instruction met!\n";
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

				  errs() << "Depthendfunc : " << depth << "returned here!\n";
	    	}

	    	void printDFGDOT(std::string fileName ,DFG* currBBDFG){
	    		std::ofstream ofs;
	    		ofs.open(fileName.c_str());
	    		dfgNode node;
	    		int count = 0;

	    		//Write the initial info
	    		ofs << "digraph Region_18 {\n\tgraph [ nslimit = \"1000.0\",\n\torientation = landscape,\n\t\tcenter = true,\n\tpage = \"8.5,11\",\n\tsize = \"10,7.5\" ] ;" << std::endl;

	    		errs() << "Node List Size : " << currBBDFG->getNodes().size() << "\n";

				if(currBBDFG->getNodes()[0].getNode() == NULL) {
					errs() << "NULLL!\n";
				}


	    		//fprintf(fp_dot, "\"Op_%d\" [ fontname = \"Helvetica\" shape = box, label = \"%d\"] ;\n", i, i);
	//    		std::vector<dfgNode>::iterator ii;
	//    		for(ii = currBBDFG->getNodes().begin(); ii != currBBDFG->getNodes().end() ; ii++ ){

				for (int i = 0 ; i < currBBDFG->getNodes().size() ; i++) {
	    			node = currBBDFG->getNodes()[i];

	    			if(node.getNode() == NULL) {
	    				errs() << "NULLL! :" << i << "\n";
	    			}

	    			Instruction* ins = node.getNode();
	//    			errs() << "\"Op_" << *ins << "\" [ fontname = \"Helvetica\" shape = box, label = \"" << *ins << "\"]" << "\n" ;
	    			ofs << "\"Op_" << ins << "\" [ fontname = \"Helvetica\" shape = box, label = \" ";

	    			ofs << ins->getOpcodeName(); //<< " ( ";
//	    			for (int j = 0; j < ins->getNumOperands(); ++j) {
//	    				ofs << ins->getOperand(j)->getName().str() << ",";
//					}
//	    			ofs << " ) ";
	    			ofs << ", " << node.getIdx() << ", ASAP=" << node.getASAPnumber()
	    					                     << ", ALAP=" << node.getALAPnumber()
//												 << ", (t,y,x)=(" << node.getMappedLoc()->getT() << "," << node.getMappedLoc()->getY() << "," << node.getMappedLoc()->getX() << ")"
												 << "\"]" << std::endl;
	    		}

	    		//	fprintf(fp_dot, "{ rank = same ;\n}\n");
	    		ofs << "{ rank = same ;\n}" << std::endl;

	//    		for(ii = currBBDFG->getNodes().begin(); ii != currBBDFG->getNodes().end() ; ii++ ){
				for (int i = 0 ; i < currBBDFG->getNodes().size() ; i++) {
	//    			fprintf(fp_dot, "\"Op_%d\" -> \"Op_%d\" [style = bold, color = red] ;\n", i, j);
	    			node = currBBDFG->getNodes()[i];
	    			Instruction* destIns;
	//    			std::vector<Instruction*>::iterator cc;
	//    			for(cc = node.getChildren().begin(); cc != node.getChildren().end(); cc++){

	    			int j;
	    			for (j=0 ; j < node.getChildren().size(); j++){
	    				destIns = node.getChildren()[j];
	    				if(destIns != NULL) {
	    					errs() << destIns->getOpcodeName() << "\n";
//	    					ofs << "\"Op_" << node.getNode() << "\" -> \"Op_" << destIns << "\" [style = bold, color = red];" << std::endl;

	    					assert(currBBDFG->findEdge(node.getNode(),node.getChildren()[j])!=NULL);
	    					if(currBBDFG->findEdge(node.getNode(),node.getChildren()[j])->getType() == EDGE_TYPE_DATA){
	    						ofs << "\"Op_" << node.getNode() << "\" -> \"Op_" << destIns << "\" [style = bold, color = red];" << std::endl;
	    					}
	    					else if(currBBDFG->findEdge(node.getNode(),node.getChildren()[j])->getType() == EDGE_TYPE_LDST){
	    						ofs << "\"Op_" << node.getNode() << "\" -> \"Op_" << destIns << "\" [style = bold, color = green];" << std::endl;
	    					}
	    					else {
	    						ofs << "\"Op_" << node.getNode() << "\" -> \"Op_" << destIns << "\" [style = bold, color = black];" << std::endl;
	    					}

	    				}
	    			}
	    		}

	    		ofs << "}" << std::endl;
	    		ofs.close();
	    	}

	    	Instruction* checkMemDepedency(Instruction *I, MemoryDependenceAnalysis *MD){
				  MemDepResult mRes;
				  errs() << "#*#*#*#*#* This is a memory op #*#*#*#*#*\n";
				  mRes = MD->getDependency(I);

				  if(mRes.getInst() != NULL){
					  errs() << "Dependency : \n";
					  mRes.getInst()->dump();
				  }
				  else{
					  errs() << "Not Dependent or cannot find the dependence : \n";
				  }

				  return mRes.getInst();
	    	}



//	    	void analyzeMachineFunction(Function &F){
//	    		  //Get MachineFunction
//	    		  MachineFunction &MF = MachineFunction::get(&F);
//
//
//	    		  //Print out machine function
//	    		  DEBUG(MF.print(std::cerr));
//	    	}




namespace {

	struct SkeletonFunctionPass : public FunctionPass {
    static char ID;
    SkeletonFunctionPass() : FunctionPass(ID) {
//    	initializeSkeletonFunctionPassPass(*PassRegistry::getPassRegistry());
    }

    	virtual bool runOnFunction(Function &F) {
				std::map<Instruction*,int> insMap;
				std::map<Instruction*,int> insMap2;

			  errs() << "In a function calledd " << F.getName() << "!\n";

//			  //TODO : please remove this after dtw test
//			  if (F.getName() != "fft_float"){
//				  return false;
//			  }

			  LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
//			  MemoryDependenceAnalysis *MD = &getAnalysis<MemoryDependenceAnalysis>();
			  ScalarEvolution* SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
			  DependenceAnalysis* DA = &getAnalysis<DependenceAnalysis>();

			  MemDepResult mRes;

			  int loopCounter = 0;
			  errs() << F.getName() << "\n";

			  DFG funcDFG;
			  for (auto &B : F) {
					 int Icount = 0;
					 for (auto &I : B) {
						 if(insMap2.find(&I) != insMap2.end()){
							 continue;
						 }
						  int depth = 0;
						  traverseDefTree((Instruction*)&I, depth, &funcDFG, &insMap2);
						  errs() << "Ins Count : " << Icount++ << "\n";
					 }

			  }
			  funcDFG.connectBB();
//			  funcDFG.addMemDepEdges(MD);
			  funcDFG.removeAlloc();
			  funcDFG.scheduleASAP();
			  funcDFG.scheduleALAP();
			  funcDFG.CreateSchList();
//			  funcDFG.addMemRecDepEdges(DA);
			  funcDFG.MapCGRAsa(4,4);
			  printDFGDOT (F.getName().str() + "_funcdfg.dot", &funcDFG);
			  return true;


//			  funcDFG.MapCGRA(4,4);
//			  printDFGDOT (F.getName().str() + "_funcdfg.dot", &funcDFG);

//			  funcDFG.printXML(F.getName().str() + "_func.xml");


			  for (LoopInfo::iterator i = LI.begin(); i != LI.end() ; ++i){
				  Loop *L = *i;
				  errs() << "*********Loop***********" << "\n";
				  L->dump();
				  errs() << "\n\n";


				  //Only the innermost Loop

				  if(L->getSubLoops().size() != 0){
					  continue;
				  }



				  std::vector<DFG> DFGs;


				  DFG LoopDFG;

				  for (Loop::block_iterator bb = L->block_begin(); bb!= L->block_end(); ++bb){
					 BasicBlock *B = *bb;

					 errs() << "\n*********BasicBlock : " << B->getName() << "\n\n";


//					 DFG currBBDFG;


					 int Icount = 0;
					 for (auto &I : *B) {

						 if(insMap.find(&I) != insMap.end()){
							 continue;
						 }


						  errs() << "Instruction: ";
						  I.dump();


//						  if(I.mayReadOrWriteMemory()){
//							  checkMemDepedency(&I,MD);
//						  }

						  int depth = 0;
//						  traverseDefTree(&I, depth, &currBBDFG, &insMap);
						  traverseDefTree(&I, depth, &LoopDFG, &insMap);


//						  for (User *U : I.users()) {
//							if (Instruction *Inst = dyn_cast<Instruction>(U)) {
//							  errs() << "\tI is used in instruction:\n";
//							  errs() << "\t" <<*Inst << "\n";
//							}
//						  }

						errs() << "Ins Count : " << Icount++ << "\n";
					 }
//					 printDFGDOT (F.getName().str() + "_" + B->getName().str() + "_dfg.dot", &currBBDFG);
				  }
				  LoopDFG.connectBB();
//				  LoopDFG.addMemDepEdges(MD);
				  LoopDFG.removeAlloc();
				  LoopDFG.scheduleASAP();
				  LoopDFG.scheduleALAP();
				  LoopDFG.CreateSchList();
				  LoopDFG.addMemRecDepEdges(DA);
//				  LoopDFG.MapCGRA(4,4);
				  printDFGDOT (F.getName().str() + "_L" + std::to_string(loopCounter) + "_loopdfg.dot", &LoopDFG);
//				  LoopDFG.printXML(F.getName().str() + "_L" + std::to_string(loopCounter) + "_loopdfg.xml");
				  loopCounter++;
			  } //end loopIterator

			  errs() << "Function body:\n";

			  std::string Filename = ("cfg." + F.getName() + ".dot").str();
			  errs() << "Writing '" << Filename << "'...";

			  std::error_code EC;
			  raw_fd_ostream File(Filename, EC, sys::fs::F_Text);

			  if (!EC)
				  WriteGraph(File, (const Function*)&F);
			  else
				  errs() << "  error opening file for writing!";
			  errs() << "\n";


			  return false;
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
//				  errs() << "Instruction: ";
//				  I.dump();
//
//				  for (User *U : I.users()) {
//				    if (Instruction *Inst = dyn_cast<Instruction>(U)) {
//				      errs() << "\tI is used in instruction:\n";
//				      errs() << "\t" <<*Inst << "\n";
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
				  errs() << "*********Loop***********\n";
				  L->dump();
				  errs() << "\n\n";
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
