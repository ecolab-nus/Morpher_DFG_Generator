#include "dfg.h"

class DFGTrig : public DFG{
	public :
		DFGTrig(std::string name,std::map<Loop*,std::string>* lnPtr) : DFG(name,lnPtr){}
		void connectBB();
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



	private :
		dfgNode* getStartNode(BasicBlock* BB, dfgNode* PHINode);
		dfgNode* insertMergeNode(dfgNode* PHINode, dfgNode* ctrl, bool controlVal, dfgNode* data);
		dfgNode* insertMergeNode(dfgNode* PHINode, dfgNode* ctrl, bool controlVal, int val);

		dfgNode* addLoadParent(Instruction* ins, dfgNode* child);

		std::map<BasicBlock*,int> startNodeConsts;
		std::map<BasicBlock*,dfgNode*> startNodes;


};

