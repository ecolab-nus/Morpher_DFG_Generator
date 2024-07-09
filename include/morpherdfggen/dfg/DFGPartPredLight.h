#ifndef DFGPARTPREDLIGHT_H
#define DFGPARTPREDLIGHT_H


#include <unordered_set>
#include <morpherdfggen/common/dfg.h>
#include <morpherdfggen/dfg/DFGPartPred.h>

//comment this in normal compilation
//#define REMOVE_AGI
//Uncomment this if compiling fo the pace0.5 architecture
//#define ARCHI_16BIT



class DFGPartPredLight : public DFGPartPred{
public :
	DFGPartPredLight(std::string name,std::map<Loop*,std::string>* lnPtr, Loop* l) : DFGPartPred(name,lnPtr,l){}
	// void connectBB();
	// virtual void connectBBTrig();
	// int handlePHINodes(std::set<BasicBlock*> LoopBB);
	// int handleSELECTNodes();
	// void removeDisconnetedNodes();
	// void printDOT(std::string fileName);
	// void printDOTsimple(std::string fileName);
	//needed
	virtual void generateTrigDFGDOT(Function &F);


	// dfgNode* combineConditionAND(dfgNode* brcond, dfgNode* selcond, dfgNode* child);
	// bool checkBackEdge(dfgNode* src, dfgNode* dest);
	// bool checkPHILoop(dfgNode* node1, dfgNode* node2);

	// bool checkBRPath(const BranchInst* BRParentBRI, const BasicBlock* currBB, bool& condVal, std::set<const BasicBlock*>& searchedSoFar);
	//		bool checkControlVal(dfgNode* brnode, const BasicBlock* destBB);

	// std::vector<dfgNode*> getStoreInstructions(BasicBlock* BB);
	// void scheduleCleanBackedges();

	// void fillCMergeMutexNodes();
	// void constructCMERGETree();
    //needed
	void scheduleASAP();
	// void scheduleALAP();
	// void balanceSched();
	// void assignALAPasASAP();

	// int maxASAPLevel=0;
     //needed
	virtual void printNewDFGXML();
	// int classifyParents();

	// void removeOutLoopLoad();

	// void addRecConnsAsPseudo();
	// void addOrphanPseudoEdges();
	// void changeTypeofSingleSourceCompNodes();

	// void createCtrlBROrTree();

	// void RemoveInductionControlLogic();
	// void RemoveBackEdgePHIs();
	// void RemoveConstantCMERGEs();

	// ScalarEvolution* SE;

	// void getLoopExitConditionNodes(std::set<exitNode> &exitNodes);
	// // void addLoopExitStoreHyCUBE(std::set<exitNode> &exitNodes);
	// void addLoopExitStoreHyCUBE(std::set<exitNode>& exitNodes);

	// void PrintOuts(){
	// 	printDOT(this->kernelname + "_PartPredDFG.dot");
	// 	printDOTsimple(this->kernelname + "_PartPredDFG_simple.dot");
	// 	printNewDFGXML();
	// }
	//needed
    void removeAGI();
	void addParentsToRemovalNodes(dfgNode* node, std::set<dfgNode*> &removalNodes, std::set<dfgNode*> &non_agi_NodesSet);
	void addChildsToNonAGINodes(dfgNode* node, std::set<dfgNode*> &non_agi_NodesSet, std::set<dfgNode*> &tempNodesSet);
	void addParentsToNonAGINodes(dfgNode* node, std::set<dfgNode*> &non_agi_NodesSet, std::set<dfgNode*> &tempNodesSet);


// protected :
// 	dfgNode* getStartNode(BasicBlock* BB, dfgNode* PHINode);
// 	dfgNode* insertMergeNode(dfgNode* PHINode, dfgNode* ctrl, bool controlVal, dfgNode* data);
// 	dfgNode* insertMergeNode(dfgNode* PHINode, dfgNode* ctrl, bool controlVal, int val);
// 	dfgNode* insertMergeNodeBeforeSEL(dfgNode* SELNode, dfgNode* ctrl, bool controlVal, dfgNode* data);
// 	dfgNode* insertMergeNodeBeforeSEL(dfgNode* SELNode, dfgNode* ctrl, bool controlVal, int val);

// 	dfgNode* addLoadParent(Value* ins, dfgNode* child);

// 	std::map<BasicBlock*,int> startNodeConsts;
// 	std::map<BasicBlock*,dfgNode*> startNodes;

// 	std::set<dfgNode*> backedgeChildMergeNodes;

// 	std::map<dfgNode*,std::set<dfgNode*>> mutexNodes;


// 	//
// 	std::map<dfgNode*,dfgNode*> cmergeCtrlInputs;
// 	std::map<dfgNode*,dfgNode*> cmergeDataInputs;

// 	std::map<dfgNode*,dfgNode*> cmergePHINodes;

// 	std::set<std::set<dfgNode*>> mutexSets;
// 	std::map<std::set<dfgNode*>,std::set<dfgNode*>> mutexSetCommonChildren;

// 	std::map<dfgNode*,dfgNode*> selectPHIAncestorMap;

// 	//Inherited from DFGTrig
// 	std::map<BasicBlock*,dfgNode*> BrParentMap;
// 	std::map<dfgNode*,BasicBlock*> BrParentMapInv;
// 	std::map<dfgNode*,std::set<dfgNode*>> leafControlInputs;
// 	std::map<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> getCtrlInfoBBMorePaths();


// 	Loop* currLoop;

// 	std::set<dfgNode*> realphi_as_selectphi;
// 	std::map<dfgNode*,std::map<Value*,dfgNode*>> PHIArgMap;

// 	std::map<dfgNode*,std::map<dfgNode*,int>> Edge2OperandIdxMap;



};



#endif
