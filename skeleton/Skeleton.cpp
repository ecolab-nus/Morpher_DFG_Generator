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

#include "llvm/ADT/GraphTraits.h"

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>


namespace llvm {
	void initializeSkeletonFunctionPassPass(PassRegistry &);
	void initializeSkeletonModulePassPass(PassRegistry &);

	Pass* createskeleton();
}

using namespace llvm;
#define LV_NAME "sfp"
#define DEBUG_TYPE LV_NAME

STATISTIC(LoopsAnalyzed, "Number of loops analyzed for vectorization");


class dfgNode{
		private :
			Instruction* Node;
			std::vector<Instruction*> Children;

		public :
			dfgNode(Instruction *ins){
				this->Node = ins;
			}

			dfgNode(){

			}

			std::vector<Instruction*>::iterator getChildIterator(){
				return Children.begin();
			}

			std::vector<Instruction*> getChildren(){
				return Children;
			}

			Instruction* getNode(){
				return Node;
			}
			void addChild(Instruction *child){
				Children.push_back(child);
			}
	};

class DFG{
		private :
			std::vector<dfgNode> NodeList;

		public :

			DFG(){

			}
			dfgNode* getEntryNode(){
				if(NodeList.size() > 0){
					return &(NodeList[0]);
				}
				return NULL;
			}

			std::vector<dfgNode> getNodes(){
				return NodeList;
			}

			void InsertNode(Instruction* Node){
				dfgNode temp(Node);
				NodeList.push_back(temp);
			}

			void InsertNode(dfgNode Node){
				NodeList.push_back(Node);
			}

			dfgNode* findNode(Instruction* I){
				for(int i = 0 ; i < NodeList.size() ; i++){
					if (I == NodeList[i].getNode()){
						return &(NodeList[i]);
					}
				}
				return NULL;
			}

			std::vector<dfgNode*> getRoots(){
				std::vector<dfgNode*> rootNodes;
				for (int i = 0 ; i < NodeList.size() ; i++) {
					if(NodeList[i].getChildren().size() == 0){
						rootNodes.push_back(&NodeList[i]);
					}
				}
				return rootNodes;
			}

			std::vector<dfgNode*> getLeafs(BasicBlock* BB){
				errs() << "start getting the LeafNodes...!\n";
				std::vector<dfgNode*> leafNodes;
				for (int i = 0 ; i < NodeList.size() ; i++) {
					if(NodeList[i].getNode()->getParent() == BB){
						leafNodes.push_back(&NodeList[i]);
					}
				}
				errs() << "LeafNodes init done...!\n";

				for (int i = 0 ; i < NodeList.size() ; i++) {
					if(NodeList[i].getNode()->getParent() == BB){
						for(int j = 0; j < NodeList[i].getChildren().size(); j++){
							dfgNode* nodeToBeRemoved = this->findNode(NodeList[i].getChildren()[j]);
							if(nodeToBeRemoved != NULL){
								errs() << "LeafNodes : nodeToBeRemoved found...! : ";
								nodeToBeRemoved->getNode()->dump();
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

			void connectBB(){
				std::vector<BasicBlock*> analysedBB;
				for (int i = 0 ; i < NodeList.size() ; i++) {
					if(NodeList[i].getNode()->getOpcode() == Instruction::Br){
						if (std::find(analysedBB.begin(), analysedBB.end(), NodeList[i].getNode()->getParent()) == analysedBB.end()){
							analysedBB.push_back(NodeList[i].getNode()->getParent());
						}
						BasicBlock* BB = NodeList[i].getNode()->getParent();
						succ_iterator SI(succ_begin(BB)), SE(succ_end(BB));
						 for (; SI != SE; ++SI){
							 BasicBlock* succ = *SI;
							 if (std::find(analysedBB.begin(), analysedBB.end(), succ) == analysedBB.end()){
								 std::vector<dfgNode*> succLeafs = this->getLeafs(succ);
								 for (int j = 0; j < succLeafs.size(); j++){
									 NodeList[i].addChild(succLeafs[j]->getNode());
								 }
							 }
						 }
					}
				}
			}

	};

	void traverseDefTree(Instruction *I, int depth, DFG* currBBDFG, std::map<Instruction*,int>* insMapIn){
		 	 	 errs() << "DEPTH = " << depth << "\n";
//		 	 	 if(insMapIn->find(I) != insMapIn->end())
//		 	 	 {
//					 errs() << "Instruction = %" << *I << "% is already there\n";
//					 return;
//				 }
				 (*insMapIn)[I]++;
	    		 dfgNode curr(I);
				 currBBDFG->InsertNode(curr);
				 dfgNode* currPtr = currBBDFG->findNode(I);
				  for (User *U : I->users()) {
					if (Instruction *Inst = dyn_cast<Instruction>(U)) {
						currBBDFG->findNode(I)->addChild(Inst);
					  errs() << "\t" <<*Inst << "\n";
//					  if(insMapIn->find(Inst) == insMapIn->end()){
						  traverseDefTree(Inst, depth + 1, currBBDFG, insMapIn);
//					  }
					  errs() << "Depthfor : " << depth << " returned here!\n";
					}
				  }
				  errs() << "Depthendfor : " << depth << " returned here!\n";
				  if (I->getOpcode() == Instruction::Br ) {
					  errs() << "Branch instruction met!\n";
//					  I->getPrevNode()->dump();


					  std::vector<dfgNode*> rootNodes = currBBDFG->getRoots();
					  for (int i = 0; i < rootNodes.size(); i++){
						  if ((rootNodes[i]->getNode() != I)&&(rootNodes[i]->getNode()->getParent() == I->getParent())){
							  rootNodes[i]->addChild(I);
						  }
					  }

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
	    			ofs << "\"Op_" << ins << "\" [ fontname = \"Helvetica\" shape = box, label = \"" << ins->getOpcodeName() << "\"]" << std::endl;
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
	    					ofs << "\"Op_" << node.getNode() << "\" -> \"Op_" << destIns << "\" [style = bold, color = red];" << std::endl;
	    				}
	    			}
	    		}

	    		ofs << "}" << std::endl;
	    		ofs.close();
	    	}

	    	void checkMemDepedency(Instruction *I, MemoryDependenceAnalysis *MD){
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
	    	}




namespace {

	struct SkeletonFunctionPass : public FunctionPass {
    static char ID;
    std::map<Instruction*,int> insMap;
    std::map<Instruction*,int> insMap2;
    SkeletonFunctionPass() : FunctionPass(ID) {
    	initializeSkeletonFunctionPassPass(*PassRegistry::getPassRegistry());
    }

    	virtual bool runOnFunction(Function &F) {
			  errs() << "In a function called " << F.getName() << "!\n";

			  LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
			  MemoryDependenceAnalysis *MD = &getAnalysis<MemoryDependenceAnalysis>();
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
			  printDFGDOT (F.getName().str() + "_funcdfg.dot", &funcDFG);


			  for (LoopInfo::iterator i = LI.begin(); i != LI.end() ; ++i){
				  Loop *L = *i;
				  errs() << "*********Loop***********" << "\n";
				  L->dump();
				  errs() << "\n\n";

				  std::vector<DFG> DFGs;


				  DFG LoopDFG;

				  for (Loop::block_iterator bb = L->block_begin(); bb!= L->block_end(); ++bb){
					 BasicBlock *B = *bb;

					 errs() << "\n*********BasicBlock : " << B->getName() << "\n\n";


					 DFG currBBDFG;


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
						  traverseDefTree(&I, depth, &currBBDFG, &insMap);
						  traverseDefTree(&I, depth, &LoopDFG, &insMap);


//						  for (User *U : I.users()) {
//							if (Instruction *Inst = dyn_cast<Instruction>(U)) {
//							  errs() << "\tI is used in instruction:\n";
//							  errs() << "\t" <<*Inst << "\n";
//							}
//						  }

						errs() << "Ins Count : " << Icount++ << "\n";
					 }
					 printDFGDOT (F.getName().str() + "_" + B->getName().str() + "_dfg.dot", &currBBDFG);
				  }
				  LoopDFG.connectBB();
				  printDFGDOT (F.getName().str() + "_L" + std::to_string(loopCounter) + "_funcdfg.dot", &LoopDFG);
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
			AU.setPreservesAll();
			AU.addRequired<LoopInfoWrapperPass>();
			AU.addRequired<MemoryDependenceAnalysis>();
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
			AU.addRequired<MemoryDependenceAnalysis>();
		}

	};
}

char SkeletonModulePass::ID = 2;


INITIALIZE_PASS_BEGIN(SkeletonFunctionPass, "skeleton", "SkeletonFunctionPass", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(MemoryDependenceAnalysis)
INITIALIZE_PASS_END(SkeletonFunctionPass, "skeleton", "SkeletonFunctionPass", false, false)

Pass* llvm::createskeleton() {
	return new SkeletonFunctionPass();
}


//INITIALIZE_PASS_BEGIN(SkeletonModulePass, "smp", "SkeletonModulePass", false, false)
//INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
//INITIALIZE_PASS_DEPENDENCY(MemoryDependenceAnalysis)
//INITIALIZE_PASS_END(SkeletonModulePass, "smp", "SkeletonModulePass", false, false)

//static RegisterPass<SkeletonFunctionPass> X("sfp", "Hello World Pass");

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerSkeletonPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
//  PM.add(new SkeletonLoopPass());
//  PM.add(new SkeletonModulePass());
  PM.add(new SkeletonFunctionPass());
}
static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerSkeletonPass);
