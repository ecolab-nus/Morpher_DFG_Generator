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


namespace llvm {
	void initializeSkeletonFunctionPassPass(PassRegistry &);
}

using namespace llvm;
#define LV_NAME "sfp"
#define DEBUG_TYPE LV_NAME

STATISTIC(LoopsAnalyzed, "Number of loops analyzed for vectorization");


namespace {
	struct SkeletonFunctionPass : public FunctionPass {
    static char ID;
    std::map<Instruction*,int> insMap;
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

			  for (LoopInfo::iterator i = LI.begin(); i != LI.end() ; ++i){
				  Loop *L = *i;
				  errs() << "*********Loop***********\n";
				  L->dump();
				  errs() << "\n\n";

				  for (Loop::block_iterator bb = L->block_begin(); bb!= L->block_end(); ++bb){
					 BasicBlock *B = *bb;

					 errs() << "\n*********BasicBlock : " << B->getName() << "\n\n";

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
						  traverseDefTree(&I, depth);
//						  for (User *U : I.users()) {
//							if (Instruction *Inst = dyn_cast<Instruction>(U)) {
//							  errs() << "\tI is used in instruction:\n";
//							  errs() << "\t" <<*Inst << "\n";
//							}
//						  }

						errs() << "Ins Count : " << Icount++ << "\n";
					 }
				  }
			  }

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


    	void traverseDefTree(Instruction *I, int depth){
    		 insMap[I]++;
    		 errs() << "DEPTH = " << depth << "\n";
			  for (User *U : I->users()) {
				if (Instruction *Inst = dyn_cast<Instruction>(U)) {
				  errs() << "\t" <<*Inst << "\n";
				  traverseDefTree(Inst, depth + 1);
				}
			  }
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


		void getAnalysisUsage(AnalysisUsage &AU) const override {
			AU.setPreservesAll();
			AU.addRequired<LoopInfoWrapperPass>();
			AU.addRequired<MemoryDependenceAnalysis>();
		}

	};
}

char SkeletonFunctionPass::ID = 1;
INITIALIZE_PASS_BEGIN(SkeletonFunctionPass, "sfp", "SkeletonFunctionPass", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(MemoryDependenceAnalysis)
INITIALIZE_PASS_END(SkeletonFunctionPass, "sfp", "SkeletonFunctionPass", false, false)

//namespace {
//	struct SkeletonLoopPass : public LoopPass {
//	  static char ID;
//	  SkeletonLoopPass() : LoopPass(ID) {}
//	  virtual bool runOnLoop(Loop* lp, LPPassManager &LPM) {
//		  lp->dump();
//
//
//		 std::vector<BasicBlock*> blkList = lp->getBlocks();
//		 for(BasicBlock* B : blkList){
//
////			 for (auto &I : *B) {
////				  errs() << "Instruction: ";
////				  I.dump();
////
////				  for (User *U : I.users()) {
////				    if (Instruction *Inst = dyn_cast<Instruction>(U)) {
////				      errs() << "\tI is used in instruction:\n";
////				      errs() << "\t" <<*Inst << "\n";
////				    }
////				  }
////			 }
//
////			 MemoryDependenceAnalysis *MD = &getAnalysisUsage<MemoryDependenceAnalysis>();
//		 }
//		  return false;
//	  }
//
//	    // We don't modify the program, so we preserve all analyses.
//	    void getAnalysisUsage(AnalysisUsage &AU) const override {
//	    	AU.setPreservesAll();
//	    }
//
//	};
//}
//
//char SkeletonLoopPass::ID = 0;





// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerSkeletonPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
//  PM.add(new SkeletonLoopPass());
  PM.add(new SkeletonFunctionPass());
}
static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerSkeletonPass);
