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

#include "llvm/Transforms/Utils.h"

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

#include "llvm/IR/Attributes.h"

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
#include "DFGTrig.h"
#include "DFGPartPred.h"
#include "DFGDISE.h"
#include "DFGTrMap.h"
#include "DFGBrMap.h"

//#define CDFG

AttributeList attr21;

static bool xmlRun = false;

namespace llvm
{
void initializeSkeletonFunctionPassPass(PassRegistry &);
void initializeSkeletonModulePassPass(PassRegistry &);

Pass *createskeleton();
} // namespace llvm

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
static cl::opt<std::string> dfgType("type", cl::init("PartPred"), cl::desc("The type of the dfg, valid types = PartPred, Trig, TrMap, BrMap, DFGDISE"));

STATISTIC(LoopsAnalyzed, "Number of loops analyzed for vectorization");

static std::map<std::string, int> sizeArrMap;
static std::set<BasicBlock *> LoopBB;
static std::map<const BasicBlock *, std::vector<const BasicBlock *>> BBSuccBasicBlocks;
static std::map<Loop *, std::vector<BasicBlock *>> loopsExclusieBasicBlockMap;

typedef struct
{
	bool isInnerLoop = false;
	Loop *lp;
	std::set<BasicBlock *> allBlocks;
	std::set<std::pair<BasicBlock *, BasicBlock *>> entryBlocks;
	std::set<std::pair<BasicBlock *, BasicBlock *>> exitBlocks;
} MappingUnit;

static std::map<std::string, MappingUnit> mappingUnitMap;
static std::map<std::string, Loop *> mappingUnit2LoopMap;
static std::map<BasicBlock *, std::string> BB2MUnitMap;
std::set<BasicBlock *> nonloopBBs;

typedef struct LoopTree LoopTree;
struct LoopTree
{
	Loop *lp;
	std::vector<LoopTree> lpChildren;
};
LoopTree rootLoop; //current loop is NULL is just a struct capture all the toplevel loops.

std::vector<munitTransition> munitTransitions;
std::vector<munitTransition> munitTransitionsALL;

void traverseDefTree(Instruction *I,
					 int depth,
					 DFG *currBBDFG, std::map<Instruction *, int> *insMapIn,
					 std::map<const BasicBlock *, std::vector<const BasicBlock *>> BBSuccBasicBlocks,
					 std::set<BasicBlock *> validBB,
					 MemoryDependenceAnalysis *MD = NULL)
{

	SmallVector<std::pair<const BasicBlock *, const BasicBlock *>, 1> BackEdgesBB;
	FindFunctionBackedges(*(I->getFunction()), BackEdgesBB);

	(*insMapIn)[I]++;
	dfgNode curr(I, currBBDFG);
	currBBDFG->InsertNode(I);
	dfgNode *currPtr = currBBDFG->findNode(I);

	if (!dyn_cast<PHINode>(I))
	{
		for (Use &V : I->operands())
		{
			if (Instruction *ParIns = dyn_cast<Instruction>(V))
			{
				if (validBB.find(ParIns->getParent()) == validBB.end())
				{
					currBBDFG->findNode(I)->addLoadParent(ParIns);
				}
			}

			if (Argument *arg = dyn_cast<Argument>(V))
			{
				outs() << "Argument adding load parent : ";
				V->dump();
				if (!V->getType()->isPointerTy())
				{
					currBBDFG->findNode(I)->addLoadParent(arg);
				}
			}
		}
	}

	for (User *U : I->users())
	{

		errs() << "I :";
		I->dump();
		errs() << "Inst : ";
		U->dump();

		if (Instruction *Inst = dyn_cast<Instruction>(U))
		{
			errs() << "#####TRAVDEFTREE :: Inst Valid!\n";

			//Searching inside basicblocks of the loop
			if (validBB.find(Inst->getParent()) == validBB.end())
			{
				currBBDFG->findNode(I)->addStoreChild(I);
				continue;
			}

			if (Inst->getOpcode() == Instruction::PHI)
			{
				errs() << "#####TRAVDEFTREE :: PHI Child found2!\n";
				currBBDFG->findNode(I)->addPHIchild(Inst);
				I->dump();
				Inst->dump();
				continue;
			}

			if (std::find(BBSuccBasicBlocks[I->getParent()].begin(), BBSuccBasicBlocks[I->getParent()].end(), Inst->getParent()) == BBSuccBasicBlocks[I->getParent()].end())
			{
				if (Inst->getOpcode() == Instruction::PHI)
				{
					errs() << "#####TRAVDEFTREE :: PHI Child found1!\n";
					currBBDFG->findNode(I)->addPHIchild(Inst);
				}
				errs() << "line 126, #####TRAVDEFTREE :: backedge found!\n";
				continue;
			}

			std::pair<const BasicBlock *, const BasicBlock *> bbCouple(I->getParent(), Inst->getParent());
			if (std::find(BackEdgesBB.begin(), BackEdgesBB.end(), bbCouple) != BackEdgesBB.end())
			{
				if (I->getParent() != Inst->getParent())
				{
					errs() << "line 112, #####TRAVDEFTREE :: backedge found!\n";
					continue;
				}
			}

			if (Inst->getOpcode() == Instruction::PHI)
			{
				if (I->getParent() == Inst->getParent())
				{
					errs() << "#####TRAVDEFTREE :: PHI Child found2!\n";
					currBBDFG->findNode(I)->addPHIchild(Inst);
					I->dump();
					Inst->dump();
					continue;
				}
			}

			currBBDFG->findNode(I)->addChild(Inst);

			if (insMapIn->find(Inst) == insMapIn->end())
			{
				traverseDefTree(Inst, depth + 1, currBBDFG, insMapIn, BBSuccBasicBlocks, validBB);
			}

			currBBDFG->findNode(Inst)->addAncestor(I);
		}
	}
}

void printDFGDOT(std::string fileName, DFG *currBBDFG)
{
	std::ofstream ofs;
	ofs.open(fileName.c_str());
	dfgNode *node;
	int count = 0;

	//Write the initial info
	ofs << "digraph Region_18 {\n\tgraph [ nslimit = \"1000.0\",\n\torientation = landscape,\n\t\tcenter = true,\n\tpage = \"8.5,11\",\n\tsize = \"10,7.5\" ] ;" << std::endl;

	assert(currBBDFG->getNodes().size() != 0);

	if (currBBDFG->getNodes()[0]->getNode() == NULL)
	{

	}

	for (int i = 0; i < currBBDFG->getNodes().size(); i++)
	{
		node = currBBDFG->getNodes()[i];

		if (node->getNode() == NULL)
		{

		}

		ofs << "\"Op_" << node->getIdx() << "\" [ fontname = \"Helvetica\" shape = box, label = \" ";

		if (node->getNode() != NULL)
		{
			ofs << node->getNode()->getOpcodeName();

			if (node->hasConstantVal())
			{
				ofs << " C="
					<< "0x" << std::hex << node->getConstantVal() << std::dec;
			}

			if (node->isGEP())
			{
				ofs << " C="
					<< "0x" << std::hex << node->getGEPbaseAddr() << std::dec;
			}

			ofs << " BB" << node->getNode()->getParent()->getName().str();
		}
		else
		{
			ofs << node->getNameType();
			if (node->isOutLoop())
			{
				ofs << " C="
					<< "0x" << node->getoutloopAddr() << std::dec;
			}

			if (node->hasConstantVal())
			{
				ofs << " C="
					<< "0x" << node->getConstantVal() << std::dec;
			}
		}

		if (node->getFinalIns() != NOP)
		{
			ofs << " HyIns=" << currBBDFG->HyCUBEInsStrings[node->getFinalIns()];
		}

		if (node->getMappedLoc() != NULL)
		{
			ofs << ",\n"
				<< node->getIdx() << ", ASAP=" << node->getASAPnumber()
				<< ", ALAP=" << node->getALAPnumber()
				<< ", (t,y,x)=(" << node->getMappedLoc()->getT() << "," << node->getMappedLoc()->getY() << "," << node->getMappedLoc()->getX() << ")"
				<< ",RT=" << node->getmappedRealTime()
				<< "\"]" << std::endl;
		}
		else
		{
			ofs << ",\n"
				<< node->getIdx() << ", ASAP=" << node->getASAPnumber()
				<< ", ALAP=" << node->getALAPnumber()
				<< "\"]" << std::endl;
		}
	}

	ofs << "{ rank = same ;\n}" << std::endl;

	for (int i = 0; i < currBBDFG->getNodes().size(); i++)
	{
		node = currBBDFG->getNodes()[i];

		int j;
		for (j = 0; j < node->getChildren().size(); j++)
		{
			assert(currBBDFG->findEdge(node, node->getChildren()[j]) != NULL);
			if (currBBDFG->findEdge(node, node->getChildren()[j])->getType() == EDGE_TYPE_DATA)
			{
				ofs << "\"Op_" << node->getIdx() << "\" -> \"Op_" << node->getChildren()[j]->getIdx() << "\" [style = bold, color = red];" << std::endl;
			}
			else if (currBBDFG->findEdge(node, node->getChildren()[j])->getType() == EDGE_TYPE_CTRL)
			{
				ofs << "\"Op_" << node->getIdx() << "\" -> \"Op_" << node->getChildren()[j]->getIdx() << "\" [style = bold, color = black];" << std::endl;
			}
		}

		//adding recurrence edges
		for (j = 0; j < node->getRecChildren().size(); j++)
		{
			assert(currBBDFG->findEdge(node, node->getRecChildren()[j]) != NULL);
			if (currBBDFG->findEdge(node, node->getRecChildren()[j])->getType() == EDGE_TYPE_LDST)
			{
				ofs << "\"Op_" << node->getIdx() << "\" -> \"Op_" << node->getRecChildren()[j]->getIdx() << "\" [style = bold, color = green];" << std::endl;
			}
		}

		//adding phi edges
		for (j = 0; j < node->getPHIchildren().size(); j++)
		{
			assert(currBBDFG->findEdge(node, node->getPHIchildren()[j]) != NULL);
			if (currBBDFG->findEdge(node, node->getPHIchildren()[j])->getType() == EDGE_TYPE_PHI)
			{
				ofs << "\"Op_" << node->getIdx() << "\" -> \"Op_" << node->getPHIchildren()[j]->getIdx() << "\" [style = bold, color = orange];" << std::endl;
			}
		}
	}

	ofs << "}" << std::endl;
	ofs.close();
}

Instruction *checkMemDepedency(Instruction *I, MemoryDependenceResults *MD)
{
	MemDepResult mRes;
	//errs() << "#*#*#*#*#* This is a memory op #*#*#*#*#*\n";
	mRes = MD->getDependency(I);

	if (mRes.getInst() != NULL)
	{
		//errs() << "Dependency : \n";
		mRes.getInst()->dump();
	}
	else
	{
		//errs() << "Not Dependent or cannot find the dependence : \n";
	}

	return mRes.getInst();
}

void dfsBB(SmallVector<std::pair<const BasicBlock *, const BasicBlock *>, 1> BackEdgesBB,
		   std::map<const BasicBlock *, std::vector<const BasicBlock *>> *BBSuccBasicBlocksPtr,
		   BasicBlock *currBB,
		   const BasicBlock *startBB)
{
	errs() << "currBB : " << currBB->getName() << "\n";

	succ_iterator SI(succ_begin(currBB)), SE(succ_end(currBB));
	for (; SI != SE; ++SI)
	{
		BasicBlock *succ = *SI;

		std::pair<const BasicBlock *, const BasicBlock *> bbCouple(currBB, succ);
		if (std::find(BackEdgesBB.begin(), BackEdgesBB.end(), bbCouple) != BackEdgesBB.end())
		{
			continue;
		}

		if (std::find((*BBSuccBasicBlocksPtr)[startBB].begin(), (*BBSuccBasicBlocksPtr)[startBB].end(), succ) != (*BBSuccBasicBlocksPtr)[startBB].end())
		{
			continue;
		}

		(*BBSuccBasicBlocksPtr)[startBB].push_back(succ);
		dfsBB(BackEdgesBB, BBSuccBasicBlocksPtr, succ, startBB);
	}
	return;
}

void printBBSuccMap(Function &F,
					std::map<const BasicBlock *, std::vector<const BasicBlock *>> BBSuccBasicBlocks)
{

	std::map<const BasicBlock *, std::vector<const BasicBlock *>>::iterator it;
	std::ofstream basicblockmapfile;
	std::string fname = F.getName().str() + "_basicblockmapfile.log";
	basicblockmapfile.open(fname.c_str());
	for (it = BBSuccBasicBlocks.begin(); it != BBSuccBasicBlocks.end(); it++)
	{
		basicblockmapfile << "BB::" << it->first->getName().str() << " = ";
		for (int u = 0; u < it->second.size(); ++u)
		{
			basicblockmapfile << it->second[u]->getName().str() << ", ";
		}
		basicblockmapfile << "\n";
	}
	basicblockmapfile.close();
}

void populateNonLoopBBs(Function &F, std::vector<Loop *> loops)
{
	for (BasicBlock &BB : F)
	{

		BasicBlock *BBPtr = cast<BasicBlock>(&BB);
		nonloopBBs.insert(BBPtr);
	}

	for (Loop *lp : loops)
	{
		for (BasicBlock *BB : lp->getBlocks())
		{
			nonloopBBs.erase(BB);
		}
	}
}

void getInnerMostLoops(std::vector<Loop *> *innerMostLoops, std::vector<Loop *> loops, std::map<Loop *, std::string> *loopNames, std::string lnstr, LoopTree *parentLoopTree)
{
	for (int i = 0; i < loops.size(); ++i)
	{
		std::stringstream ss;
		ss << lnstr << std::hex << (i + 1) << std::dec;
		(*loopNames)[loops[i]] = ss.str();

		LoopTree currLPTree;
		currLPTree.lp = loops[i];
		parentLoopTree->lpChildren.push_back(currLPTree);

		outs() << "LoopName : " << ss.str() << "\n";
		for (Loop::block_iterator bb = loops[i]->block_begin(); bb != loops[i]->block_end(); ++bb)
		{
			outs() << (*bb)->getName() << ",";
		}
		outs() << "; EXIT=";
		SmallVector<BasicBlock *, 8> loopExitBlocks;
		loops[i]->getExitBlocks(loopExitBlocks);
		for (int i = 0; i < loopExitBlocks.size(); ++i)
		{
			outs() << loopExitBlocks[i]->getName() << ",";
		}

		outs() << "\n";

		if (loops[i]->getSubLoops().size() == 0)
		{
			innerMostLoops->push_back(loops[i]);
			mappingUnitMap["INNERMOST_" + ss.str()].isInnerLoop = true;
			mappingUnitMap["INNERMOST_" + ss.str()].lp = loops[i];

			for (BasicBlock *BB : loops[i]->getBlocks())
			{
				loopsExclusieBasicBlockMap[loops[i]].push_back(BB);
				mappingUnitMap["INNERMOST_" + ss.str()].allBlocks.insert(BB);
				BB2MUnitMap[BB] = "INNERMOST_" + ss.str();
			}
		}
		else
		{
			getInnerMostLoops(innerMostLoops, loops[i]->getSubLoops(), loopNames, ss.str(), &parentLoopTree->lpChildren.back());

			for (BasicBlock *BB : loops[i]->getBlocks())
			{
				bool BBfound = false;
				for (std::pair<Loop *, std::vector<BasicBlock *>> pair : loopsExclusieBasicBlockMap)
				{
					if (std::find(pair.second.begin(), pair.second.end(), BB) != pair.second.end())
					{
						BBfound = true;
						break;
					}
				}
				if (!BBfound)
				{
					loopsExclusieBasicBlockMap[loops[i]].push_back(BB);
				}
			}
		}
	}
}

void printNtabs(int N)
{
	for (int i = 0; i < N; ++i)
	{
		outs() << "\t";
	}
}

void printLoopTree(LoopTree rootLoop, std::map<Loop *, std::string> *loopNames, int tabs = 0);
void printLoopTree(LoopTree rootLoop, std::map<Loop *, std::string> *loopNames, int tabs)
{

	struct less_than_lp
	{
		inline bool operator()(const LoopTree &lptree1, const LoopTree &lptree2)
		{
			BasicBlock *lp1Header = lptree1.lp->getHeader();
			BasicBlock *lp2Header = lptree2.lp->getHeader();
			if (std::find(BBSuccBasicBlocks[lp1Header].begin(),
						  BBSuccBasicBlocks[lp1Header].end(),
						  lp2Header) != BBSuccBasicBlocks[lp1Header].end())
			{
				//this means lp2 is a succesor of lp1
				return true;
			}
			return false;
		}
	};
	std::sort(rootLoop.lpChildren.begin(), rootLoop.lpChildren.end(), less_than_lp());

	std::vector<BasicBlock *> thisLoopBB;

	if (rootLoop.lp == NULL)
	{
		for (BasicBlock *BB : nonloopBBs)
		{
			thisLoopBB.push_back(BB);
		}
	}
	else
	{
		thisLoopBB = loopsExclusieBasicBlockMap[rootLoop.lp];
	}

	if (false)
	{
		outs() << "ROOT LOOP\n";
	}
	else
	{
		printNtabs(tabs);
		if (rootLoop.lp != NULL)
		{
			outs() << (*loopNames)[rootLoop.lp] << "******begin\n";
		}

		for (BasicBlock *bb : thisLoopBB)
		{
			printNtabs(tabs);
			outs() << bb->getName() << ",";
			bool found = false;
			for (LoopTree lt : rootLoop.lpChildren)
			{
				std::stringstream ss;
				BasicBlock *lpHeader = lt.lp->getHeader();
				if (std::find(BBSuccBasicBlocks[bb].begin(),
							  BBSuccBasicBlocks[bb].end(),
							  lpHeader) != BBSuccBasicBlocks[bb].end())
				{
					outs() << "PRE_" << (*loopNames)[lt.lp] << ",";
					ss << "PRE_" << (*loopNames)[lt.lp];
					mappingUnitMap[ss.str()].allBlocks.insert(bb);
					mappingUnitMap[ss.str()].lp = rootLoop.lp;
					BB2MUnitMap[bb] = ss.str();
					found = true;
					break;
				}
			}
			if (found)
			{
				outs() << "\n";
				continue;
			}

			std::vector<LoopTree> reverseLpChildren = rootLoop.lpChildren;
			std::reverse(reverseLpChildren.begin(), reverseLpChildren.end());
			for (LoopTree lt : reverseLpChildren)
			{
				std::stringstream ss;
				BasicBlock *lpHeader = lt.lp->getHeader();
				if (std::find(BBSuccBasicBlocks[lpHeader].begin(),
							  BBSuccBasicBlocks[lpHeader].end(),
							  bb) != BBSuccBasicBlocks[lpHeader].end())
				{
					outs() << "POST_" << (*loopNames)[lt.lp] << ",";
					ss << "POST_" << (*loopNames)[lt.lp];
					mappingUnitMap[ss.str()].allBlocks.insert(bb);
					mappingUnitMap[ss.str()].lp = rootLoop.lp;
					BB2MUnitMap[bb] = ss.str();
					found = true;
					break;
				}
			}
			outs() << "\n";
		}

		for (std::pair<std::string, MappingUnit> pair : mappingUnitMap)
		{
			//for entry blocks
			std::string name = pair.first;
			for (BasicBlock *BB : mappingUnitMap[name].allBlocks)
			{
				for (auto it = pred_begin(BB), et = pred_end(BB); it != et; ++it)
				{
					BasicBlock *predBB = *it;
					if (std::find(mappingUnitMap[name].allBlocks.begin(),
								  mappingUnitMap[name].allBlocks.end(),
								  predBB) == mappingUnitMap[name].allBlocks.end())
					{
						mappingUnitMap[name].entryBlocks.insert(std::make_pair(predBB, BB));
					}
				}
			}

			//for exit blocks
			for (BasicBlock *BB : mappingUnitMap[name].allBlocks)
			{
				for (auto it = succ_begin(BB), et = succ_end(BB); it != et; ++it)
				{
					BasicBlock *succBB = *it;
					if (std::find(mappingUnitMap[name].allBlocks.begin(),
								  mappingUnitMap[name].allBlocks.end(),
								  succBB) == mappingUnitMap[name].allBlocks.end())
					{
						mappingUnitMap[name].exitBlocks.insert(std::make_pair(BB, succBB));
					}
				}
			}
		}

		printNtabs(tabs);
		if (rootLoop.lp != NULL)
		{
			outs() << (*loopNames)[rootLoop.lp] << "******end\n";
		}
	}

	for (LoopTree child : rootLoop.lpChildren)
	{
		printLoopTree(child, loopNames, tabs + 1);
	}
}

void printMappableUnitMap()
{
	outs() << "Printing mappable unit map ... \n";
	for (std::pair<std::string, MappingUnit> pair : mappingUnitMap)
	{
		outs() << pair.first << " :: ";
		for (BasicBlock *bb : pair.second.allBlocks)
		{
			outs() << bb->getName() << ",";
		}
		outs() << "|entry=";
		for (std::pair<BasicBlock *, BasicBlock *> bbPair : pair.second.entryBlocks)
		{
			outs() << bbPair.first->getName() << "to" << bbPair.second->getName() << ",";
		}
		outs() << "|exit=";
		for (std::pair<BasicBlock *, BasicBlock *> bbPair : pair.second.exitBlocks)
		{
			outs() << bbPair.first->getName() << "to" << bbPair.second->getName() << ",";
		}
		outs() << "\n";
	}
}

std::string findMUofBB(BasicBlock *BB)
{
	for (std::pair<std::string, MappingUnit> pair1 : mappingUnitMap)
	{
		if (pair1.second.allBlocks.find(BB) != pair1.second.allBlocks.end())
		{
			return pair1.first;
		}
	}
	return "FUNC_BODY";
}

void dfsmunitTrans(std::pair<std::string, std::string> currEdge,
				   std::vector<std::string> currPath,
				   std::map<std::string, std::map<std::string, std::set<std::string>>> &transbasedScalarTransfers,
				   std::map<std::string, std::map<std::string, std::set<std::vector<std::string>>>> &transbasedNextMunit,
				   std::map<std::string, std::string> varOwner,
				   std::set<std::pair<std::string, std::string>> visitedEdges,
				   std::map<std::string, std::set<std::string>> &munitTrans,
				   std::map<std::string, std::set<std::string>> &varNeeds,
				   int tabs = 0)
{

	if (visitedEdges.find(currEdge) != visitedEdges.end())
	{
		return;
	}


	std::string currMunit = currEdge.second;
	for (int i = 0; i < tabs; ++i)
	{
		outs() << "\t";
	}
	outs() << "prevMunit=" << currEdge.first << ",currMunit=" << currMunit << "\n";

	for (std::string varName : varNeeds[currMunit])
	{
		assert(varOwner.find(varName) != varOwner.end());
		assert(!varOwner[varName].empty());

		for (int i = 0; i < tabs; ++i)
		{
			outs() << "\t";
		}
		outs() << "varName = " << varName << ",";
		outs() << "PrevOwner =" << varOwner[varName] << ",";

		if (varOwner[varName].compare(currMunit) == 0)
		{
			outs() << "isOwned by itself\n";
			continue;
		}
		assert(varOwner[varName].compare(currMunit) != 0);

		bool found = false;
		int foundi = 0;
		outs() << "currPath.size=" << currPath.size() << ",";
		std::vector<std::string> connectingPath;
		for (int i = 0; i < currPath.size(); ++i)
		{
			std::string nextMunit;
			if (i == currPath.size() - 1)
			{
				nextMunit = currMunit;
			}
			else
			{
				nextMunit = currPath[i + 1];
			}

			if (found)
			{
				if (connectingPath.back().compare(nextMunit) != 0)
				{
					connectingPath.push_back(nextMunit);
				}

				outs() << nextMunit << ",";
			}
			if (currPath[i].compare(varOwner[varName]) == 0)
			{
				outs() << "foundi=" << i << "," << nextMunit << ",";
				found = true;
				foundi = i;
				transbasedScalarTransfers[currPath[i]][currMunit].insert(varName);
				connectingPath.push_back(nextMunit);
			}
		}

		assert(std::find(currPath.begin(), currPath.end(), currEdge.first) != currPath.end());

		if (found)
		{
			outs() << "NewOwner =" << currMunit << "\n";
			varOwner[varName] = currMunit;
			transbasedNextMunit[currPath[foundi]][currMunit].insert(connectingPath);
		}
		else
		{
			outs() << "\n";
		}
	}


	visitedEdges.insert(currEdge);

	if (currMunit.compare("FUNC_BODY") == 0)
		return;
	for (std::string nextMunit : munitTrans[currMunit])
	{
		assert(nextMunit.compare(currMunit) != 0);
		std::pair<std::string, std::string> nextEdge = std::make_pair(currMunit, nextMunit);
		currPath.push_back(currMunit);
		dfsmunitTrans(nextEdge, currPath, transbasedScalarTransfers, transbasedNextMunit, varOwner, visitedEdges, munitTrans, varNeeds, tabs + 1);
	}
	return;
}

void printFileOutMappingUnitVars(Function &F,
								 std::map<std::string, int> *sizeArrMap,
								 std::map<Loop *, std::string> loopNames)
{

	std::ofstream outVarMapFile;
	std::string fileName = F.getName().str() + ".outMUVar.csv";
	outVarMapFile.open(fileName.c_str());

	std::map<std::string, std::set<std::string>> munitTrans;
	for (munitTransition mut : munitTransitionsALL)
	{
		std::string srcBBStr = "FUNC_BODY";
		std::string destBBStr = "FUNC_BODY";

		if (BB2MUnitMap.find(mut.srcBB) != BB2MUnitMap.end())
		{
			srcBBStr = BB2MUnitMap[mut.srcBB];
		}
		if (BB2MUnitMap.find(mut.destBB) != BB2MUnitMap.end())
		{
			destBBStr = BB2MUnitMap[mut.destBB];
		}
		munitTrans[srcBBStr].insert(destBBStr);
	}

	std::set<BasicBlock *> restBasicBlocksFn;
	for (BasicBlock &BB : F)
	{
		BasicBlock *BBPtr = cast<BasicBlock>(&BB);
		restBasicBlocksFn.insert(BBPtr);
	}

	//find basic blocks that does not belong to any mapping unit
	for (std::pair<std::string, MappingUnit> pair1 : mappingUnitMap)
	{
		for (BasicBlock *currBB : pair1.second.allBlocks)
		{
			restBasicBlocksFn.erase(currBB);
		}
	}

	std::map<std::string, std::map<std::string, int>> graphMappingUnits;
	std::map<std::string, std::map<std::string, std::set<std::string>>> graphMappingUnitVarNames;

	for (std::pair<std::string, MappingUnit> pair1 : mappingUnitMap)
	{
		for (std::pair<std::string, MappingUnit> pair2 : mappingUnitMap)
		{
			if (pair2.first.compare(pair1.first) == 0)
			{
				continue; // skip children in the same mapping unit
			}
			graphMappingUnits[pair1.first][pair2.first] = 0;
		}
		graphMappingUnits[pair1.first]["FUNC_BODY"] = 0;
		graphMappingUnits["FUNC_BODY"][pair1.first] = 0;
	}

	for (std::pair<std::string, MappingUnit> pair1 : mappingUnitMap)
	{
		for (BasicBlock *currBB : pair1.second.allBlocks)
		{

			for (Instruction &I : *currBB)
			{
				for (User *U : I.users())
				{
					if (Instruction *child = dyn_cast<Instruction>(U))
					{
						BasicBlock *childBB = child->getParent();

						for (std::pair<std::string, MappingUnit> pair2 : mappingUnitMap)
						{
							if (pair2.first.compare(pair1.first) == 0)
							{
								continue; // skip children in the same mapping unit
							}
							if (pair2.second.allBlocks.find(childBB) != pair2.second.allBlocks.end())
							{
								graphMappingUnitVarNames[pair1.first][pair2.first].insert(std::to_string((long)&I));
							}
						}

						//children that go outside of mapping units to function body
						if (restBasicBlocksFn.find(childBB) != restBasicBlocksFn.end())
						{
							graphMappingUnitVarNames[pair1.first]["FUNC_BODY"].insert(std::to_string((long)&I));
						}
					}
				}
			}
		}
	}

	//children of FUNC_BODY
	for (BasicBlock *funcbodyBB : restBasicBlocksFn)
	{
		for (Instruction &I : *funcbodyBB)
		{
			for (User *U : I.users())
			{
				if (Instruction *child = dyn_cast<Instruction>(U))
				{
					BasicBlock *childBB = child->getParent();

					for (std::pair<std::string, MappingUnit> pair2 : mappingUnitMap)
					{
						if (pair2.second.allBlocks.find(childBB) != pair2.second.allBlocks.end())
						{
							graphMappingUnitVarNames["FUNC_BODY"][pair2.first].insert(std::to_string((long)&I));
						}
					}
				}
			}
		}
	}

	//printing the file
	for (std::pair<std::string, std::set<std::string>> pair1 : munitTrans)
	{
		outVarMapFile << pair1.first << ",(";
		for (std::string destMunit : pair1.second)
		{
			outVarMapFile << destMunit << ",";
		}
		outVarMapFile << ")\n";
	}

	for (std::pair<std::string, std::map<std::string, int>> pair1 : graphMappingUnits)
	{
		for (std::pair<std::string, int> pair2 : pair1.second)
		{
			int varSize = graphMappingUnitVarNames[pair1.first][pair2.first].size();
			if (varSize != 0 && mappingUnitMap[pair1.first].lp != mappingUnitMap[pair2.first].lp)
			{

				std::string loop1Name = "FUNC_BODY";
				if (mappingUnitMap[pair1.first].lp != NULL)
					loop1Name = loopNames[mappingUnitMap[pair1.first].lp];

				std::string loop2Name = "FUNC_BODY";
				if (mappingUnitMap[pair2.first].lp != NULL)
					loop2Name = loopNames[mappingUnitMap[pair2.first].lp];

				outVarMapFile << pair1.first << "," << loop1Name << ",";
				outVarMapFile << pair2.first << "," << loop2Name << ",";
				outVarMapFile << varSize << ",";
				for (std::string varName : graphMappingUnitVarNames[pair1.first][pair2.first])
				{
					outVarMapFile << varName << ",";
				}
				outVarMapFile << "\n";
			}
		}
	}

	BasicBlock *startBB = cast<BasicBlock>(&F.getEntryBlock());
	BasicBlock *endBB;

	bool rtiFound = false;
	for (BasicBlock &BB : F)
	{
		for (Instruction &I : BB)
		{
			if (ReturnInst *RTI = dyn_cast<ReturnInst>(&I))
			{
				endBB = RTI->getParent();
				rtiFound = true;
				break;
			}
		}

		if (rtiFound)
		{
			break;
		}
	}

	//Printing array variables
	std::map<std::string, std::map<std::string, int>> grapharrMappingUnits;
	std::map<std::string, std::map<std::string, std::set<std::string>>> grapharrUnitVarNames;

	for (std::pair<std::string, MappingUnit> pair1 : mappingUnitMap)
	{
		for (std::pair<std::string, MappingUnit> pair2 : mappingUnitMap)
		{
			if (pair2.first.compare(pair1.first) == 0)
			{
				continue; // skip children in the same mapping unit
			}
			grapharrMappingUnits[pair1.first][pair2.first] = 0;
		}
		grapharrMappingUnits[pair1.first]["FUNC_BODY"] = 0;
		grapharrMappingUnits["FUNC_BODY"][pair1.first] = 0;
	}

	std::map<std::string, std::set<std::string>> GEPloadMUMap;
	std::map<std::string, std::set<std::string>> GEPstoreMUMap;
	std::map<std::string, Type *> GEPTypeMap;

	for (BasicBlock &BB : F)
	{
		for (Instruction &I : BB)
		{
			if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(&I))
			{
				outs() << "Fn : " << F.getName() << "\n";
				GEP->dump();

				std::string ptrName = GEP->getPointerOperand()->getName().str();
				outs() << "PtrName = " << ptrName << "\n";

				Instruction *pointerOp;
				if (ptrName.empty())
				{
					pointerOp = cast<Instruction>(GEP->getPointerOperand());
					pointerOp->dump();
					if (LoadInst *pointerOpLD = dyn_cast<LoadInst>(GEP->getPointerOperand()))
					{
						ptrName = pointerOpLD->getPointerOperand()->getName().str();
						outs() << "NewPtrName = " << ptrName << "\n";
					}
				}
				while (ptrName.empty())
				{
					pointerOp = cast<Instruction>(pointerOp->getOperand(0));
					pointerOp->dump();
					if (GetElementPtrInst *OrignalGEP = dyn_cast<GetElementPtrInst>(pointerOp))
					{
						ptrName = OrignalGEP->getPointerOperand()->getName().str();
						outs() << "NewPtrName = " << ptrName << "\n";
					}
					if (CallInst *CLI = dyn_cast<CallInst>(pointerOp))
					{ //2019 work
						ptrName = CLI->getName().str();
						outs() << "NewPtrName = " << ptrName << "\n";
					}
				}

				GEPTypeMap[ptrName] = GEP->getSourceElementType();
				GEPstoreMUMap[ptrName].insert(BB2MUnitMap[startBB]);
				for (User *U : GEP->users())
				{
					if (StoreInst *STI = dyn_cast<StoreInst>(U))
					{
						GEPstoreMUMap[ptrName].insert(findMUofBB(STI->getParent()));
						GEPloadMUMap[ptrName].insert(BB2MUnitMap[endBB]);
					}
					if (LoadInst *LDI = dyn_cast<LoadInst>(U))
					{
						GEPloadMUMap[ptrName].insert(findMUofBB(LDI->getParent()));
					}
				}
			}
		}
	}

	std::set<std::pair<Instruction *, std::string>> usesQ;
	for (std::pair<std::string, int> pair1 : *sizeArrMap)
	{
		outs() << "the user given pointer name = " << pair1.first << "\n";
		for (BasicBlock &BB : F)
		{
			for (Instruction &I : BB)
			{
				if (StoreInst *STI = dyn_cast<StoreInst>(&I))
					continue;
				if (dyn_cast<LoadInst>(&I) && (!dyn_cast<LoadInst>(&I)->getType()->isPointerTy()))
					continue;

				for (int i = 0; i < I.getNumOperands(); ++i)
				{
					if (I.getOperand(i)->getName().str().compare(pair1.first) == 0)
					{
						I.dump();
						if (Instruction *childIns = dyn_cast<Instruction>(&I))
						{
							usesQ.insert(std::make_pair(childIns, pair1.first));
						}
					}
				}
			}
		}
	}

	outs() << "Allocas ::\n";
	for (BasicBlock &BB : F)
	{
		const DataLayout DL = F.getParent()->getDataLayout();
		for (Instruction &I : BB)
		{
			if (AllocaInst *ALI = dyn_cast<AllocaInst>(&I))
			{
				ALI->dump();
				if (ALI->isStaticAlloca())
				{
					outs() << "Size=" << DL.getTypeAllocSize(ALI->getAllocatedType()) << "\n";
					(*sizeArrMap)[ALI->getName().str()] = (int)DL.getTypeAllocSize(ALI->getAllocatedType());
					usesQ.insert(std::make_pair(&I, ALI->getName().str()));
				}
				else
				{
					assert(false);
				}
			}
		}
	}

	std::map<Value *, std::string> truePointers;
	std::map<std::string, std::string> truePointersStr;

	outs() << "Collecting True Pointers...\n";
	while (!usesQ.empty())
	{
		std::pair<Instruction *, std::string> head = *usesQ.begin();
		Instruction *I = head.first;
		I->dump();
		std::string ptrName = head.second;
		usesQ.erase(head);
		truePointers[I] = ptrName;
		truePointersStr[I->getName().str()] = ptrName;

		for (User *U : I->users())
		{
			if (StoreInst *STI = dyn_cast<StoreInst>(U))
				continue;
			if (dyn_cast<LoadInst>(U) && (!dyn_cast<LoadInst>(U)->getType()->isPointerTy()))
				continue;
			if (Instruction *childIns = dyn_cast<Instruction>(U))
			{
				outs() << "\tchild=";
				childIns->dump();
				if (truePointers.find(childIns) == truePointers.end())
				{
					usesQ.insert(std::make_pair(childIns, ptrName));
				}
				else
				{
					outs() << "\t\tnot inserted\n";
				}
			}
		}
	}

	outs() << "TRUE_POINTERS :: \n";
	for (std::pair<Value *, std::string> tppair : truePointers)
	{
		outs() << tppair.second << "<--";
		tppair.first->dump();
	}

	std::map<std::string, int> arrSizes;

	for (std::pair<std::string, std::set<std::string>> pair1 : GEPstoreMUMap)
	{
		std::string ptrName = pair1.first;
		const DataLayout DL = F.getParent()->getDataLayout();
		Type *T = GEPTypeMap[ptrName];
		int size;

		T->dump();
		//Determing array size
		if (StructType *ST = dyn_cast<StructType>(T))
		{
			size = DL.getTypeAllocSize(ST);
		}
		else if (ArrayType *AT = dyn_cast<ArrayType>(T))
		{
			size = DL.getTypeAllocSize(AT);
		}
		else
		{
			//TODO: DAC18
			//						 IntegerType* IT = cast<IntegerType>(T);

			if (sizeArrMap->find(ptrName) != sizeArrMap->end())
			{
				size = (*sizeArrMap)[ptrName];
			}
			else if (sizeArrMap->find(truePointersStr[ptrName]) != sizeArrMap->end())
			{
				size = (*sizeArrMap)[ptrName];
			}
			else
			{
				errs() << "Please provide sizes for the arrayptr : " << ptrName << "\n";
				assert(0);
			}
		}

		arrSizes[ptrName] = size;

		for (std::string StoreMUName : pair1.second)
		{
			for (std::string LoadMUName : GEPloadMUMap[ptrName])
			{
				if (StoreMUName.compare(LoadMUName) == 0)
					continue;
				grapharrMappingUnits[StoreMUName][LoadMUName] += size;
				grapharrUnitVarNames[StoreMUName][LoadMUName].insert(ptrName);
			}
		}
	}

	outVarMapFile << "Arrays Elements : \n";
	for (std::pair<std::string, std::map<std::string, int>> pair1 : grapharrMappingUnits)
	{
		for (std::pair<std::string, int> pair2 : pair1.second)
		{
			if (pair2.second != 0)
			{
				std::string loop1Name = "FUNC_BODY";
				if (mappingUnitMap[pair1.first].lp != NULL)
					loop1Name = loopNames[mappingUnitMap[pair1.first].lp];

				std::string loop2Name = "FUNC_BODY";
				if (mappingUnitMap[pair2.first].lp != NULL)
					loop2Name = loopNames[mappingUnitMap[pair2.first].lp];

				outVarMapFile << pair1.first << "," << loop1Name << ",";
				outVarMapFile << pair2.first << "," << loop2Name << ",";

				outVarMapFile << pair2.second << ",";

				for (std::string ptrName : grapharrUnitVarNames[pair1.first][pair2.first])
				{
					outVarMapFile << ptrName << ",";
				}
				outVarMapFile << "\n";
			}
		}
	}

	outVarMapFile << "*********SMART_TRANSFERS*********\n";

	for (std::pair<std::string, int> pair : arrSizes)
	{
		outVarMapFile << pair.first << "=" << pair.second << "\n";
	}

	outVarMapFile.close();
}

void populateBBTrans()
{
	int id = 0;
	for (std::pair<std::string, MappingUnit> pair : mappingUnitMap)
	{
		for (std::pair<BasicBlock *, BasicBlock *> entryBBPair : pair.second.entryBlocks)
		{
			munitTransition munittrans;
			munittrans.srcBB = entryBBPair.first;
			munittrans.destBB = entryBBPair.second;
			munittrans.id = munitTransitions.size();
			munitTransitions.push_back(munittrans);
			munitTransitionsALL.push_back(munittrans);

		}
	}

	outs() << "MUNIT transitions = " << munitTransitions.size() << "\n";

	for (std::pair<std::string, MappingUnit> pair : mappingUnitMap)
	{
		for (std::pair<BasicBlock *, BasicBlock *> exitBBPair : pair.second.exitBlocks)
		{
			munitTransition munittrans;
			munittrans.srcBB = exitBBPair.first;
			munittrans.destBB = exitBBPair.second;
			munittrans.id = munitTransitionsALL.size();
			munitTransitionsALL.push_back(munittrans);
		}
	}
}

int getOptimumDim(unsigned int *dimX, unsigned int *dimY, int numNodes, int numMemNodes)
{
	int x;
	int y;
	int numMemPEs;

	int bestWastage = 1000000;
	for (x = 1; x <= 4; ++x)
	{
		for (y = 1; y <= 4; ++y)
		{
			numMemPEs = y;
			int memII = (numMemNodes + numMemPEs) / numMemPEs;
			int resII = (numNodes + x * y) / (x * y);
			int II = std::max(memII, resII);
			int wastage = II * y * x - numNodes;

			if (wastage < bestWastage)
			{
				*dimX = x;
				*dimY = y;
				bestWastage = wastage;
			}
		}
	}
	return bestWastage;
}

void ParseSizeAttr(Function &F, std::map<std::string, int> *sizeArrMap)
{
	auto global_annos = F.getParent()->getNamedGlobal("llvm.global.annotations");
	if (global_annos)
	{
		auto a = cast<ConstantArray>(global_annos->getOperand(0));
		for (int i = 0; i < a->getNumOperands(); i++)
		{
			auto e = cast<ConstantStruct>(a->getOperand(i));

			if (auto fn = dyn_cast<Function>(e->getOperand(0)->getOperand(0)))
			{
				auto anno = cast<ConstantDataArray>(cast<GlobalVariable>(e->getOperand(1)->getOperand(0))->getOperand(0))->getAsCString();
				fn->addFnAttr("size", anno); // <-- add function annotation here
			}
		}
	}

	if (F.hasFnAttribute("size"))
	{
		Attribute attr = F.getFnAttribute("size");
		outs() << "Size attribute : " << attr.getValueAsString() << "\n";
		StringRef sizeAttrStr = attr.getValueAsString();
		SmallVector<StringRef, 8> sizeArr;
		sizeAttrStr.split(sizeArr, ',');

		for (int i = 0; i < sizeArr.size(); ++i)
		{
			std::pair<StringRef, StringRef> splitDuple = sizeArr[i].split(':');
			uint32_t size;
			splitDuple.second.getAsInteger(10, size);
			outs() << "ParseAttr:: name:" << splitDuple.first << ",size:" << size << "\n";
			(*sizeArrMap)[splitDuple.first.str()] = size;
		}
	}

	if (F.hasFnAttribute("size"))
	{
		outs() << F.getName() << " has my attribute!\n";
	}
}

void RemoveSelectLeafs(Function &F)
{
	for (BasicBlock &BB : F)
	{
		for (Instruction &I : BB)
		{
			Instruction *ins = &I;
			if (SelectInst *SLI = dyn_cast<SelectInst>(ins))
			{
				IRBuilder<> builder(SLI);
				if (Instruction *Cond = dyn_cast<Instruction>(SLI->getCondition()))
				{
					if (Cond->getParent() == SLI->getParent())
						continue;
					outs() << "Found Alone Select=";
					SLI->dump();
					outs() << "Condition=";
					Cond->dump();
					builder.SetInsertPoint(BB.getFirstNonPHI());
					Value *OR0 = builder.CreateXor(Cond, (uint64_t)0);
					assert(OR0 != Cond);
					Instruction *ORIns = cast<Instruction>(OR0);
					outs() << "OR = ";
					ORIns->dump();

					SLI->setOperand(0, OR0);
					BB.dump();
				}
			}
		}
	}
}

void InsertORtoSingularConditionalBB(Function &F)
{
	for (BasicBlock &BB : F)
	{
		for (Instruction &I : BB)
		{
			Instruction *ins = &I;
			if (BranchInst *BRI = dyn_cast<BranchInst>(ins))
			{
				if (!BRI->isConditional())
					continue;
				IRBuilder<> builder(BRI);
				if (Instruction *Cond = dyn_cast<Instruction>(BRI->getCondition()))
				{
					if (Cond->getParent() == BRI->getParent())
						continue;
					//Coming here if the condition is from a another basicblock;
					outs() << "Found Alone Branch=";
					BRI->dump();
					outs() << "Pointer=";
					Cond->dump();
					builder.SetInsertPoint(BB.getFirstNonPHI());
					Value *OR0 = builder.CreateXor(Cond, (uint64_t)0);
					assert(OR0 != Cond);
					Instruction *ORIns = cast<Instruction>(OR0);
					outs() << "OR = ";
					ORIns->dump();

					BRI->setOperand(0, OR0);
					BB.dump();
				}
			}
		}
	}
	//	    		assert(false);
}

void ReplaceCMPs(Function &F)
{
	dfgNode *node;
	std::vector<Instruction *> instructions;

	for (auto &BB : F)
	{
		for (auto &I : BB)
		{
			instructions.push_back(&I);
		}
	}

	for (auto I : instructions)
	{
		if (CmpInst *CI = dyn_cast_or_null<CmpInst>(I))
		{
			I->dump();


			IRBuilder<> builder(CI);
			assert(CI->getNumOperands() == 2);
			switch (CI->getPredicate())
			{
			case CmpInst::ICMP_EQ:
			//TODO : DAC18
			case CmpInst::FCMP_OEQ:
			case CmpInst::FCMP_UEQ:

				break;
			case CmpInst::ICMP_NE:
				//TODO : DAC18
				{
					CmpInst *cmpEqNew = cast<CmpInst>(builder.CreateICmpEQ(CI->getOperand(0), CI->getOperand(1)));
					Instruction *notIns = cast<Instruction>(builder.CreateNot(cmpEqNew));
					BasicBlock::iterator ii(CI);
					notIns->removeFromParent();
					ReplaceInstWithInst(CI->getParent()->getInstList(), ii, notIns);
					break;
				}
			case CmpInst::FCMP_ONE:
			case CmpInst::FCMP_UNE:
			{
				CmpInst *cmpEqNew = cast<CmpInst>(builder.CreateFCmpOEQ(CI->getOperand(0), CI->getOperand(1)));
				Instruction *notIns = cast<Instruction>(builder.CreateNot(cmpEqNew));
				BasicBlock::iterator ii(CI);
				notIns->removeFromParent();
				ReplaceInstWithInst(CI->getParent()->getInstList(), ii, notIns);
				break;
			}

			case CmpInst::ICMP_SGE:
			case CmpInst::ICMP_UGE:
			//TODO : DAC18
			case CmpInst::FCMP_OGE:
			case CmpInst::FCMP_UGE:
			{
				CmpInst *cmpEqNew;
				if (CI->getPredicate() == CmpInst::ICMP_SGE)
				{
					cmpEqNew = cast<CmpInst>(builder.CreateICmpSLT(CI->getOperand(1), CI->getOperand(0)));
				}
				else
				{ // ICMP_UGE
					cmpEqNew = cast<CmpInst>(builder.CreateICmpULT(CI->getOperand(1), CI->getOperand(0)));
				}
				Instruction *notIns = cast<Instruction>(builder.CreateNot(cmpEqNew));
				BasicBlock::iterator ii(CI);
				notIns->removeFromParent();
				ReplaceInstWithInst(CI->getParent()->getInstList(), ii, notIns);
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
				CmpInst *cmpEqNew;
				if (CI->getPredicate() == CmpInst::ICMP_SLE)
				{
					cmpEqNew = cast<CmpInst>(builder.CreateICmpSGT(CI->getOperand(1), CI->getOperand(0)));
				}
				else
				{ // ICMP_ULE
					cmpEqNew = cast<CmpInst>(builder.CreateICmpUGT(CI->getOperand(1), CI->getOperand(0)));
				}
				Instruction *notIns = cast<Instruction>(builder.CreateNot(cmpEqNew));
				BasicBlock::iterator ii(CI);
				notIns->removeFromParent();
				ReplaceInstWithInst(CI->getParent()->getInstList(), ii, notIns);
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

void analyzeAllMappingUnits(Function &F,
							std::map<Loop *, std::string> loopNames)
{

	struct basicblockInfo
	{
		int noNodes;
		std::string mappingunitName;
		std::string loopName;
	};

	std::map<BasicBlock *, basicblockInfo> BasicBlockNN;

	for (std::pair<std::string, MappingUnit> pair : mappingUnitMap)
	{
		std::string munitName = pair.first;
		if (munitName.compare("FUNC_BODY") == 0)
			continue;
		if (pair.second.lp == NULL)
			continue;

		outs() << "analyzeAllMappingUnits ::" << munitName << "\n";
		std::map<Instruction *, int> insMap;

		DFG LoopDFG("test" + F.getName().str() + "_" + munitName, &loopNames);
		LoopDFG.sizeArrMap = sizeArrMap;
		LoopDFG.setLoopBB(mappingUnitMap[munitName].allBlocks,
						  mappingUnitMap[munitName].entryBlocks,
						  mappingUnitMap[munitName].exitBlocks);

		insMap.clear();
		for (BasicBlock *B : *LoopDFG.getLoopBB())
		{
			//			  	  BasicBlock *B = *bb;
			int Icount = 0;
			for (auto &I : *B)
			{

				if (insMap.find(&I) != insMap.end())
				{
					continue;
				}

				int depth = 0;
				traverseDefTree(&I, depth, &LoopDFG, &insMap, BBSuccBasicBlocks, *LoopDFG.getLoopBB());
			}
		}
		LoopDFG.addPHIChildEdges();
		LoopDFG.connectBB();
		printDFGDOT(LoopDFG.getName() + "_loopdfg.dot", &LoopDFG);

		LoopDFG.handlePHINodes(mappingUnitMap[munitName].allBlocks);

		LoopDFG.removeRedEdgesPHI();
		LoopDFG.addCMERGEtoSELECT();
		LoopDFG.handlestartstop_munit(munitTransitions);

		LoopDFG.treatFalsePaths();
		LoopDFG.insertshiftGEPs();
		LoopDFG.addMaskLowBitInstructions();
		LoopDFG.checkSanity();
		printDFGDOT(LoopDFG.getName() + "_loopdfg.dot", &LoopDFG);

		LoopDFG.scheduleASAP();
		LoopDFG.scheduleALAP();
		LoopDFG.balanceASAPALAP();
		LoopDFG.CreateSchList();
		LoopDFG.printXML();
		LoopDFG.handleMEMops();
		LoopDFG.partitionMemNodes();

		for (dfgNode *node : LoopDFG.getNodes())
		{

			BasicBlock *currBB = (BasicBlock *)node->BB;
			if (node->getNameType().compare("LOOPEXIT") == 0)
			{
				currBB = (BasicBlock *)node->getAncestors()[0]->BB;
			}

			if (BasicBlockNN.find(currBB) == BasicBlockNN.end())
			{
				basicblockInfo bbInfoIns;
				bbInfoIns.noNodes = 1;
				bbInfoIns.mappingunitName = munitName;
				bbInfoIns.loopName = loopNames[pair.second.lp];
				BasicBlockNN[currBB] = bbInfoIns;

				node->printName();
				outs() << "Adding...\n";
				outs() << "currBB name = " << currBB->getName().str() << "\n";
				outs() << " munitName = " << munitName << "\n";
				outs() << " loopNames[pair.second.lp] = " << loopNames[pair.second.lp] << "\n";
			}
			else
			{
				node->printName();
				outs() << "Incrementing...\n";
				outs() << "currBB name = " << currBB->getName().str() << "\n";
				outs() << "BasicBlockNN[currBB].mappingunitName = " << BasicBlockNN[currBB].mappingunitName;
				outs() << " munitName = " << munitName << "\n";
				outs() << "BasicBlockNN[currBB].loopName = " << BasicBlockNN[currBB].loopName;
				outs() << " loopNames[pair.second.lp] = " << loopNames[pair.second.lp] << "\n";
				assert(BasicBlockNN[currBB].mappingunitName.compare(munitName) == 0);
				assert(BasicBlockNN[currBB].loopName.compare(loopNames[pair.second.lp]) == 0);
				BasicBlockNN[currBB].noNodes = BasicBlockNN[currBB].noNodes + 1;
			}
		}
	}

	std::ofstream BasicBlockNNFile;
	BasicBlockNNFile.open("BasicBlockNNFile.log");
	for (std::pair<BasicBlock *, basicblockInfo> pair : BasicBlockNN)
	{
		BasicBlockNNFile << pair.first->getName().str() << ",";
		BasicBlockNNFile << pair.second.noNodes << ",";
		BasicBlockNNFile << pair.second.loopName << ",";
		BasicBlockNNFile << pair.second.mappingunitName << "\n";
	}
	BasicBlockNNFile.close();
}

void loopTrace(std::map<Loop *, std::string> loopNames, Function &F, LoopTree rootLoop)
{

	LLVMContext &Ctx = F.getContext();
	BasicBlock &FentryBB = F.getEntryBlock();
	IRBuilder<> builder(FentryBB.getFirstNonPHI());

	//Function Calls

	auto traceStartFn = F.getParent()->getOrInsertFunction(
		"loopTraceOpen",
		FunctionType::getVoidTy(Ctx),
		Type::getInt8PtrTy(Ctx));

	auto traceEndFn = F.getParent()->getOrInsertFunction(
		"loopTraceClose",
		FunctionType::getVoidTy(Ctx));

	auto loopInvFn = F.getParent()->getOrInsertFunction(
		"loopInvoke",
		FunctionType::getVoidTy(Ctx),
		Type::getInt8PtrTy(Ctx));

	auto loopInsUpdateFn = F.getParent()->getOrInsertFunction(
		"loopInsUpdate",
		FunctionType::getVoidTy(Ctx),
		Type::getInt8PtrTy(Ctx),
		Type::getInt32Ty(Ctx));

	auto loopBBInsUpdateFn = F.getParent()->getOrInsertFunction(
		"loopBBInsUpdate",
		FunctionType::getVoidTy(Ctx),
		Type::getInt8PtrTy(Ctx),
		Type::getInt8PtrTy(Ctx),
		Type::getInt32Ty(Ctx));

	auto loopInsClearFn = F.getParent()->getOrInsertFunction(
		"loopInsClear",
		FunctionType::getVoidTy(Ctx),
		Type::getInt8PtrTy(Ctx));

	auto loopBBInsClearFn = F.getParent()->getOrInsertFunction(
		"loopBBInsClear",
		FunctionType::getVoidTy(Ctx));

	auto loopInvokeEndFn = F.getParent()->getOrInsertFunction(
		"loopInvokeEnd",
		FunctionType::getVoidTy(Ctx),
		Type::getInt8PtrTy(Ctx));

	auto reportExecInsCountFn = F.getParent()->getOrInsertFunction(
		"reportExecInsCount",
		FunctionType::getVoidTy(Ctx),
		Type::getInt32Ty(Ctx));

	auto updateLoopPreHeaderFn = F.getParent()->getOrInsertFunction(
		"updateLoopPreHeader",
		FunctionType::getVoidTy(Ctx),
		Type::getInt8PtrTy(Ctx),
		Type::getInt8PtrTy(Ctx));

	auto loopBBMappingUnitUpdate = F.getParent()->getOrInsertFunction(
		"loopBBMappingUnitUpdate",
		FunctionType::getVoidTy(Ctx),
		Type::getInt8PtrTy(Ctx),
		Type::getInt8PtrTy(Ctx));

	auto recordUncondMunitTransition = F.getParent()->getOrInsertFunction(
		"recordUncondMunitTransition",
		FunctionType::getVoidTy(Ctx),
		Type::getInt8PtrTy(Ctx),
		Type::getInt8PtrTy(Ctx));

	auto recordCondMunitTransition = F.getParent()->getOrInsertFunction(
		"recordCondMunitTransition",
		FunctionType::getVoidTy(Ctx),
		Type::getInt8PtrTy(Ctx),
		Type::getInt8PtrTy(Ctx),
		Type::getInt8PtrTy(Ctx),
		Type::getInt1Ty(Ctx));

	for (std::pair<Loop *, std::string> lnPair : loopNames)
	{
		Value *loopName = builder.CreateGlobalStringPtr(lnPair.second);
		Value *loopNameLLVM = builder.CreateGlobalStringPtr(lnPair.second + "-" + lnPair.first->getLoopPreheader()->getName().str());
		BasicBlock *loopHeader = lnPair.first->getLoopPreheader();
		builder.SetInsertPoint(loopHeader, loopHeader->getFirstInsertionPt());
		builder.CreateCall(loopInvFn, {loopName});
		builder.CreateCall(loopInsClearFn, {loopName});

		for (BasicBlock *BB : loopsExclusieBasicBlockMap[lnPair.first])
		{
			int instructionCountBB = BB->getInstList().size();
			Value *instructionCountBBVal = ConstantInt::get(Type::getInt32Ty(Ctx), instructionCountBB);
			Value *BBName = builder.CreateGlobalStringPtr(BB->getName());

			builder.SetInsertPoint(BB, --BB->end());
			builder.CreateCall(loopInsUpdateFn, {loopName, instructionCountBBVal});
			builder.CreateCall(loopBBInsUpdateFn, {loopName, BBName, instructionCountBBVal});
		}

		SmallVector<BasicBlock *, 8> loopExitBlocks;
		lnPair.first->getExitBlocks(loopExitBlocks);
		for (BasicBlock *BB : loopExitBlocks)
		{
			builder.SetInsertPoint(BB, --BB->end());
			builder.CreateCall(loopInvokeEndFn, {loopName});
		}
	}

	//adding top most level loop's preheaders
	for (LoopTree lt : rootLoop.lpChildren)
	{
		BasicBlock *lpPreHeaderBB = lt.lp->getLoopPreheader();
		Value *loopName = builder.CreateGlobalStringPtr(loopNames[lt.lp]);

		int instructionCountBB = lpPreHeaderBB->getInstList().size();
		Value *instructionCountBBVal = ConstantInt::get(Type::getInt32Ty(Ctx), instructionCountBB);
		Value *BBName = builder.CreateGlobalStringPtr(lpPreHeaderBB->getName());

		builder.SetInsertPoint(lpPreHeaderBB, --lpPreHeaderBB->end());
		builder.CreateCall(loopBBInsUpdateFn, {loopName, BBName, instructionCountBBVal});
	}

	//find return instruction
	for (BasicBlock &BB : F)
	{
		Value *insCountBBVal = ConstantInt::get(Type::getInt32Ty(Ctx), BB.getInstList().size());
		builder.SetInsertPoint(&BB, --BB.end());
		builder.CreateCall(reportExecInsCountFn, {insCountBBVal});

		for (Instruction &I : BB)
		{
			if (ReturnInst *RI = dyn_cast<ReturnInst>(&I))
			{
				BasicBlock *retBB = RI->getParent();
				builder.SetInsertPoint(retBB, --retBB->end());
				builder.CreateCall(traceEndFn);
			}
		}
	}

	std::set<BasicBlock *> srcMunitTrans;
	for (munitTransition munitTrans : munitTransitionsALL)
	{
		srcMunitTrans.insert(munitTrans.srcBB);
	}

	for (BasicBlock *srcBB : srcMunitTrans)
	{
		for (Instruction &I : *srcBB)
		{
			Value *srcBBName = builder.CreateGlobalStringPtr(srcBB->getName());
			if (BranchInst *BRI = dyn_cast<BranchInst>(&I))
			{
				if (BRI->isConditional())
				{
					Value *dest1BBName = builder.CreateGlobalStringPtr(BRI->getSuccessor(0)->getName());
					Value *dest2BBName = builder.CreateGlobalStringPtr(BRI->getSuccessor(1)->getName());
					Value *condition = BRI->getCondition();
					builder.SetInsertPoint(BRI);
					builder.CreateCall(recordCondMunitTransition, {srcBBName, dest1BBName, dest2BBName, condition});
				}
				else
				{
					Value *destBBName = builder.CreateGlobalStringPtr(BRI->getSuccessor(0)->getName());
					builder.SetInsertPoint(BRI);
					builder.CreateCall(recordUncondMunitTransition, {srcBBName, destBBName});
				}
			}
		}
	}

	builder.SetInsertPoint(&FentryBB, FentryBB.getFirstInsertionPt());
	Value *fnName = builder.CreateGlobalStringPtr(F.getName().str());
	builder.CreateCall(traceStartFn, {fnName});
	builder.CreateCall(loopBBInsClearFn);

	//adding BB and mapping unit name relationship to the instrumentation lib
	for (std::pair<std::string, MappingUnit> pair : mappingUnitMap)
	{
		for (BasicBlock *BB : pair.second.allBlocks)
		{
			Value *BBName = builder.CreateGlobalStringPtr(BB->getName());
			Value *MunitName = builder.CreateGlobalStringPtr(pair.first);
			builder.CreateCall(loopBBMappingUnitUpdate, {BBName, MunitName});
		}
	}

	//denoting loops preheaders in the instrumentation library
	for (std::pair<Loop *, std::string> lnPair : loopNames)
	{
		Value *loopName = builder.CreateGlobalStringPtr(lnPair.second);
		Value *preHeaderBBName = builder.CreateGlobalStringPtr(lnPair.first->getLoopPreheader()->getName());
		builder.CreateCall(updateLoopPreHeaderFn, {loopName, preHeaderBBName});
	}
}


namespace
{

struct SkeletonFunctionPass : public FunctionPass
{
	static char ID;
	SkeletonFunctionPass() : FunctionPass(ID)
	{
		//    	initializeSkeletonFunctionPassPass(*PassRegistry::getPassRegistry());
	}

	virtual bool runOnFunction(Function &F)
	{
		std::map<Instruction *, int> insMap;
		std::map<Instruction *, int> insMap2;
		static std::set<const BasicBlock *> funcBB;
		std::error_code EC;

		std::ofstream timeFile;
		std::string timeFileName = "time." + F.getName().str() + ".log";
		timeFile.open(timeFileName.c_str());
		clock_t begin = clock();
		clock_t end;
		std::string loopCFGFileName;

		if (noName == false)
		{
			if (fName != "na")
			{
				if (F.getName() != fName)
				{
					errs() << "Function Name : " << F.getName() << "\n";
					return false;
				}
			}
		}

		//TODO : DAC18
		ReplaceCMPs(F);
		RemoveSelectLeafs(F);
		InsertORtoSingularConditionalBB(F);
		ParseSizeAttr(F, &sizeArrMap);

		std::string Filename = ("cfg." + F.getName() + ".dot").str();
		//errs() << "Writing '" << Filename << "'...";

		raw_fd_ostream File(Filename, EC, sys::fs::F_Text);

		if (!EC)
		{
			WriteGraph(File, (const Function *)&F);
		}
		else
		{
			errs() << "  error opening file for writing!";
			errs() << "\n";
		}

		errs() << "Processing : " << F.getName() << "\n";

		LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
		ScalarEvolution *SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
		DependenceInfo *DI = &getAnalysis<DependenceAnalysisWrapperPass>().getDI();
		DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
		const DataLayout &DL = F.getParent()->getDataLayout();

		MemDepResult mRes;

		int loopCounter = 0;

		SmallVector<std::pair<const BasicBlock *, const BasicBlock *>, 1> BackEdgesBB;
		FindFunctionBackedges(F, BackEdgesBB);

		int BBCount = 0;
		for (auto &B : F)
		{
			BBCount++;
		}

		int currBBIdx = 0;
		for (auto &B : F)
		{
			currBBIdx++;
			errs() << "Currently proessing = " << currBBIdx << "\n";
			BasicBlock *BB = dyn_cast<BasicBlock>(&B);
			funcBB.insert(BB);
			BBSuccBasicBlocks[BB].push_back(BB);
			dfsBB(BackEdgesBB, &BBSuccBasicBlocks, BB, BB);
		}
		printBBSuccMap(F, BBSuccBasicBlocks);

		end = clock();
		timeFile << "Preprocessing Time = " << double(end - begin) / CLOCKS_PER_SEC << "\n";
		insMap.clear();


		std::vector<Loop *> innerMostLoops;
		std::map<Loop *, std::string> loopNames;
		std::map<Loop *, DFG *> loopDFGs;
		std::vector<Loop *> loops;
		for (LoopInfo::iterator i = LI.begin(); i != LI.end(); ++i)
		{
			Loop *L = *i;
			loops.push_back(L);
		}

		populateNonLoopBBs(F, loops);
		std::string lnstr("LN");
		getInnerMostLoops(&innerMostLoops, loops, &loopNames, lnstr, &rootLoop);
		errs() << "Number of innermost loops : " << innerMostLoops.size() << "\n";

		printLoopTree(rootLoop, &loopNames);
		printMappableUnitMap();
		populateBBTrans();

		if (munitName == "na")
		{
			//exit here if a mapping unit name is not given
			return 0;
		}

		//-----------------------------------
		// New Code for 2018 work
		//-----------------------------------
		{
			DFG *LoopDFG;
			if (dfgType == "PartPred")
			{
				LoopDFG = new DFGPartPred(F.getName().str() + "_" + munitName, &loopNames, mappingUnitMap[munitName].lp);
				DFGPartPred *LoopDFG_PP = static_cast<DFGPartPred *>(LoopDFG);
				LoopDFG_PP->SE = SE;
			}
			else if (dfgType == "Trig")
			{
				LoopDFG = new DFGTrig(F.getName().str() + "_" + munitName, &loopNames, DT, munitName, mappingUnitMap[munitName].lp);
			}
			else if (dfgType == "TrMap")
			{
				LoopDFG = new DFGTrMap(F.getName().str() + "_" + munitName, &loopNames, mappingUnitMap[munitName].lp);
			}
			else if (dfgType == "BrMap")
			{
				LoopDFG = new DFGBrMap(F.getName().str() + "_" + munitName, &loopNames, mappingUnitMap[munitName].lp);
			}
			else if (dfgType == "DFGDISE")
			{
				LoopDFG = new DFGDISE(F.getName().str() + "_" + munitName, &loopNames, mappingUnitMap[munitName].lp);
			}
			else
			{
				outs() << "Invalid DFG Type=" << dfgType << "\n";
				assert(false);
			}


			LoopDFG->setBBSuccBasicBlocks(BBSuccBasicBlocks);
			LoopDFG->sizeArrMap = sizeArrMap;
			outs() << "Currently mapping unit : " << munitName << "\n";
			assert(!mappingUnitMap[munitName].allBlocks.empty());
			LoopDFG->setLoopBB(mappingUnitMap[munitName].allBlocks,
							   mappingUnitMap[munitName].entryBlocks,
							   mappingUnitMap[munitName].exitBlocks);
			insMap.clear();
			for (BasicBlock *B : *LoopDFG->getLoopBB())
			{
				int Icount = 0;
				for (auto &I : *B)
				{

					if (insMap.find(&I) != insMap.end())
					{
						continue;
					}

					int depth = 0;
					traverseDefTree(&I, depth, LoopDFG, &insMap, BBSuccBasicBlocks, *LoopDFG->getLoopBB());
				}
			}

			LoopDFG->addMemRecDepEdgesNew(DI);
			LoopDFG->generateTrigDFGDOT();
			delete (LoopDFG);
			outs() << "dfgType=" << dfgType << "\n";
			return false;
		}
	} //END OF runOnFunction

	void getAnalysisUsage(AnalysisUsage &AU) const override
	{
		//			AU.setPreservesAll();
		//			AU.addRequired<LoopInfoWrapperPass>();
		//			AU.addRequired<MemoryDependenceAnalysis>();
		AU.addRequired<DependenceAnalysisWrapperPass>();

		AU.setPreservesAll();
		AU.addRequired<LoopInfoWrapperPass>();
		//			AU.addRequired<MemoryDependenceAnalysis>();
		AU.addRequired<ScalarEvolutionWrapperPass>();
		AU.addRequired<AAResultsWrapperPass>();
		AU.addRequired<DominatorTreeWrapperPass>();
		//		    AU.addRequired<DependenceAnalysis>();
		AU.addRequiredID(LoopSimplifyID);
		AU.addRequiredID(LCSSAID);

		AU.addRequired<DominatorTreeWrapperPass>();
		AU.addPreserved<DominatorTreeWrapperPass>();
	}
};

} // namespace

char SkeletonFunctionPass::ID = 1;
static RegisterPass<SkeletonFunctionPass> X("skeleton", "SkeletonFunctionPass", false, false);