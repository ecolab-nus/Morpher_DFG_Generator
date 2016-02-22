#include "llvm/Pass.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/CFGPrinter.h"
#include "llvm/Support/FileSystem.h"


using namespace llvm;
#define LV_NAME "cgra_dfg"
#define DEBUG_TYPE LV_NAME

static void addInnerLoop(Loop &L, SmallVectorImpl<Loop *> &V);
STATISTIC(LoopsAnalyzed, "Number of loops analyzed for vectorization");



namespace {
	struct SkeletonFunctionPass : public FunctionPass {
    static char ID;
    SkeletonFunctionPass() : FunctionPass(ID) {}

    	virtual bool runOnFunction(Function &F) {
			  errs() << "In a function called " << F.getName() << "!\n";

//			  LoopInfo& LI = getAnalysis<LoopInfo>(F);
//			  LI.

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

//			  F.dump();
//
//			  for (auto &B : F) {
//				errs() << "Basic block:\n";
//				B.dump();
//
//				for (auto &I : B) {
//				  errs() << "Instruction: ";
//				  I.dump();
//				}
//			  }

			  return false;
			} //END OF runOnFunction

//		void getAnalysisUsage(AnalysisUsage &AU) const override {
//			AU.addRequired<LoopInfo>();
//		}

	};


	struct SkeletonLoopPass : public LoopPass {
	  static char ID;
	  SkeletonLoopPass() : LoopPass(ID) {}
	  virtual bool runOnLoop(Loop* lp, LPPassManager &LPM) {
		  lp->dump();

		 std::vector<BasicBlock*> blkList = lp->getBlocks();
		 for(BasicBlock* B : blkList){
//			 B->dump();

			 for (auto &I : *B) {
				  errs() << "Instruction: ";
				  I.dump();

				  for (User *U : I.users()) {
				    if (Instruction *Inst = dyn_cast<Instruction>(U)) {
				      errs() << "\tI is used in instruction:\n";
				      errs() << "\t" <<*Inst << "\n";
				    }
				  }
			 }
		 }


		  return false;
	  }
	};



}

char SkeletonLoopPass::ID = 0;
char SkeletonFunctionPass::ID = 1;

static void addInnerLoop(Loop &L, SmallVectorImpl<Loop *> &V) {
  if (L.empty())
    return V.push_back(&L);

  for (Loop *InnerL : L)
    addInnerLoop(*InnerL, V);
}


// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerSkeletonPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new SkeletonLoopPass());
  PM.add(new SkeletonFunctionPass());
}
static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerSkeletonPass);
