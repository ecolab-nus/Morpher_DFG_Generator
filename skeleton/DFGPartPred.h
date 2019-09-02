#include "dfg.h"
#include <unordered_set>

class DFGPartPred : public DFG{
	public :
		DFGPartPred(std::string name,std::map<Loop*,std::string>* lnPtr, Loop* l) : DFG(name,lnPtr), currLoop(l){}
		void connectBB();
		void connectBBTrig();
		int handlePHINodes(std::set<BasicBlock*> LoopBB);
		int handleSELECTNodes();
		void removeDisconnetedNodes();
		void printDOT(std::string fileName);
		void generateTrigDFGDOT();


		dfgNode* combineConditionAND(dfgNode* brcond, dfgNode* selcond, dfgNode* child);
		bool checkBackEdge(dfgNode* src, dfgNode* dest);
		bool checkPHILoop(dfgNode* node1, dfgNode* node2);

		bool checkBRPath(const BranchInst* BRParentBRI, const BasicBlock* currBB, bool& condVal, std::set<const BasicBlock*>& searchedSoFar);
//		bool checkControlVal(dfgNode* brnode, const BasicBlock* destBB);

		std::vector<dfgNode*> getStoreInstructions(BasicBlock* BB);
		void scheduleCleanBackedges();

		void fillCMergeMutexNodes();
		void constructCMERGETree();

		void scheduleASAP();
		void scheduleALAP();
		void balanceSched();
		void assignALAPasASAP();

		int maxASAPLevel=0;

		void printNewDFGXML();
		int classifyParents();

		void removeOutLoopLoad();

		void addRecConnsAsPseudo();
		void addOrphanPseudoEdges();

		void createCtrlBROrTree();

		void RemoveInductionControlLogic();
		void RemoveBackEdgePHIs();
		void RemoveConstantCMERGEs();

		ScalarEvolution* SE;



	private :
		dfgNode* getStartNode(BasicBlock* BB, dfgNode* PHINode);
		dfgNode* insertMergeNode(dfgNode* PHINode, dfgNode* ctrl, bool controlVal, dfgNode* data);
		dfgNode* insertMergeNode(dfgNode* PHINode, dfgNode* ctrl, bool controlVal, int val);

		dfgNode* addLoadParent(Value* ins, dfgNode* child);

		std::map<BasicBlock*,int> startNodeConsts;
		std::map<BasicBlock*,dfgNode*> startNodes;

		std::set<dfgNode*> backedgeChildMergeNodes;

		std::map<dfgNode*,std::set<dfgNode*>> mutexNodes;


		//
		std::map<dfgNode*,dfgNode*> cmergeCtrlInputs;
		std::map<dfgNode*,dfgNode*> cmergeDataInputs;

		std::map<dfgNode*,dfgNode*> cmergePHINodes;

		std::set<std::set<dfgNode*>> mutexSets;
		std::map<std::set<dfgNode*>,std::set<dfgNode*>> mutexSetCommonChildren;

		std::map<dfgNode*,dfgNode*> selectPHIAncestorMap;

		//Inherited from DFGTrig
		std::map<BasicBlock*,dfgNode*> BrParentMap;
		std::map<dfgNode*,BasicBlock*> BrParentMapInv;
		std::map<dfgNode*,std::set<dfgNode*>> leafControlInputs;
		std::map<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> getCtrlInfoBBMorePaths();


		Loop* currLoop;

		std::set<dfgNode*> realphi_as_selectphi;
		std::map<dfgNode*,std::map<Value*,dfgNode*>> PHIArgMap;

		std::map<dfgNode*,std::map<dfgNode*,int>> Edge2OperandIdxMap;



};

