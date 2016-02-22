#include "llvm/Pass.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
using namespace llvm;

namespace {
	struct SkeletonFunctionPass : public FunctionPass {
    static char ID;
    SkeletonFunctionPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {
//      errs() << "In a function called " << F.getName() << "!\n";
//
//      errs() << "Function body:\n";
//      F.dump();
//
//      for (auto &B : F) {
//        errs() << "Basic block:\n";
//        B.dump();
//
//        for (auto &I : B) {
//          errs() << "Instruction: ";
//          I.dump();
//        }
//      }

      return false;
    }
  };

	struct SkeletonLoopPass : public LoopPass {
	  static char ID;
	  SkeletonLoopPass() : LoopPass(ID) {}
	  virtual bool runOnLoop(Loop* lp, LPPassManager &LPM) {
		  lp->dump();
		  return false;
	  }
	};
}

char SkeletonLoopPass::ID = 0;
char SkeletonFunctionPass::ID = 1;

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
