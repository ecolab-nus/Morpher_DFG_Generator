#include "dfg.h"
#include <queue>
#include <unordered_set>

template <typename T>
class TreeNode{

private:
	TreeNode* left=NULL;
	TreeNode* right=NULL;
	T data;

public:
	T getData(){return data;}
	TreeNode* getLeft(){return left;}
	TreeNode* getRight(){return right;}

//	bool operator=(const TreeNode& other) const{
//		return this->data == other.data;
//	}

	bool belongsToParent(T d1, T& P, bool& P_bool){
		std::queue<std::pair<TreeNode*,bool>> q;
		q.push(std::make_pair(this,true));


		while(!q.empty()){
			std::pair<TreeNode*,bool> currPair = q.front(); q.pop();
			TreeNode* tn = currPair.first;
			bool currBool = currPair.second;

			if(tn->getLeft()){
				TreeNode* left = tn->getLeft();

				if(left->getData() == d1){
					P=tn->getData();
					P_bool = true;
					return true;
				}
			}
			if(tn->getRight()){
				TreeNode* right = tn->getRight();
				if(right->getData() == d1){
					P=tn->getData();
					P_bool = false;
					return true;
				}
			}

			if(tn->getLeft()){
				q.push(std::make_pair(tn->getLeft(),true));
			}
			if(tn->getRight()){
				q.push(std::make_pair(tn->getRight(),false));
			}
		}
		return false;
	}

	std::set<std::pair<TreeNode*,bool>> getAllNodes(){
		std::set<std::pair<TreeNode*,bool>> res;
		std::queue<TreeNode*> q;
		q.push(this);


		while(!q.empty()){
			TreeNode* tn = q.front(); q.pop();

			res.insert(std::make_pair(tn,true));
			res.insert(std::make_pair(tn,false));


			if(tn->getLeft()){
				q.push(tn->getLeft());
			}
			if(tn->getRight()){
				q.push(tn->getRight());
			}
		}
		return res;
	}

	std::set<std::pair<TreeNode*,bool>> getMutexNodes(T data, bool branchVal){
		std::set<std::pair<TreeNode*,bool>> res;

		std::map<TreeNode*,std::pair<TreeNode*,bool>> cameFrom;
//		cameFrom[this]=std::make_pair(NULL,true);

		TreeNode* dest=NULL;
		{
			std::queue<TreeNode*> q;
			q.push(this);
			while(!q.empty()){
				TreeNode* tn = q.front(); q.pop();
				if(tn->getData() == data){
					dest=tn;
					break;
				}
				else{
					if(tn->getLeft()){
						q.push(tn->getLeft());
						cameFrom[tn->getLeft()]=std::make_pair(tn,true);
					}
					if(tn->getRight()){
						q.push(tn->getRight());
						cameFrom[tn->getRight()]=std::make_pair(tn,false);
					}
				}
			}
		}
		assert(dest);

		std::set<std::pair<TreeNode*,bool>> nonMutexSet;
		nonMutexSet.insert(std::make_pair(dest,branchVal));

		std::pair<TreeNode*,bool> prevBranch = cameFrom[dest];
		while(prevBranch.first){
			nonMutexSet.insert(prevBranch);
			prevBranch=cameFrom[prevBranch.first];
		}

		std::set<std::pair<TreeNode*,bool>> MutexSet;
		{
			std::queue<TreeNode*> q;
			q.push(this);
			while(!q.empty()){
				TreeNode* tn = q.front(); q.pop();

				if(tn->getData() == data){
					if(branchVal){
						std::pair<TreeNode*,bool> test = std::make_pair(tn,false);
						if(nonMutexSet.find(test)==nonMutexSet.end()){
							MutexSet.insert(test);
						}
						if(tn->getRight()){
							q.push(tn->getRight());
						}
					}
					else{
						std::pair<TreeNode*,bool> test = std::make_pair(tn,true);
						if(nonMutexSet.find(test)==nonMutexSet.end()){
							MutexSet.insert(test);
						}
						if(tn->getLeft()){
							q.push(tn->getLeft());
						}
					}
				}
				else{
					{
						std::pair<TreeNode*,bool> test = std::make_pair(tn,true);
						if(nonMutexSet.find(test)==nonMutexSet.end()){
							MutexSet.insert(test);
						}
						if(tn->getLeft()){
							q.push(tn->getLeft());
						}
					}

					{
						std::pair<TreeNode*,bool> test = std::make_pair(tn,false);
						if(nonMutexSet.find(test)==nonMutexSet.end()){
							MutexSet.insert(test);
						}
						if(tn->getRight()){
							q.push(tn->getRight());
						}
					}
				}
			}
		}

		return MutexSet;
	}

	std::set<std::pair<TreeNode*,bool>> getLeafs(){
		std::set<std::pair<TreeNode*,bool>> res;

		std::queue<TreeNode*> q;
		q.push(this);

		while(!q.empty()){
			TreeNode* tn = q.front(); q.pop();

			if(tn->getLeft()){
				q.push(tn->getLeft());
			}
			else{
				res.insert(std::make_pair(tn,true));
			}

			if(tn->getRight()){
				q.push(tn->getRight());
			}
			else{
				res.insert(std::make_pair(tn,false));
			}
		}
		return res;
	}

	bool insertChild(T child, T parent, bool dir){
		if(data == parent){
			insertChild(child,dir);
			return true;
		}

		if(left){
			if(left->insertChild(child,parent,dir)){
				return true;
			}
		}

		if(right){
			if(right->insertChild(child,parent,dir)){
				return true;
			}
		}

		return false;
	}

	void insertChild(T data, bool dir){
		if(dir == true){
			if(left){
				if(left->getData() == data) return;
				assert(false);
			}
			left = new TreeNode(data);
			return;
		}
		else{
			if(right){
				if(right->getData() == data) return;
				assert(false);
			}
			right = new TreeNode(data);
			return;
		}
	}

	TreeNode(T data) : data(data){}
	~TreeNode(){
		if(left)  delete left;
		if(right) delete right;
	}
};

class DFGTrig : public DFG{

public :
	DFGTrig(std::string name,std::map<Loop*,std::string>* lnPtr, DominatorTree* DT, std::string mUnitName, Loop* l) : DFG(name,lnPtr), DT(DT), mUnitName(mUnitName), currLoop(l){}
	~DFGTrig(){
		for(TreeNode<BasicBlock*>* tn : CtrlTrees){
			if(tn) delete tn;
		}
	}
	void connectBB();
	int handlePHINodes(std::set<BasicBlock*> LoopBB);
	int handleSELECTNodes();
	void removeDisconnetedNodes();
	void printDOT(std::string fileName);
	void printSubPathDOT(std::string fileNamePrefix);
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

	void printDomTree();
	std::set<BasicBlock*> getBBsWhoHasIDomAs(BasicBlock* bb);
	std::map<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> getCtrlInfoBB();

	std::map<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> getCtrlInfoBBMorePaths();

	void populateSubPathDFGs();
	BasicBlock* getBackEdgeParentBB();
	void CreatSubDFGPath(std::vector<std::pair<BasicBlock*,CondVal>> path);


	std::set<dfgNode*> getRootNodes();

	std::vector<dfgNode*> getLeafs(BasicBlock* BB);
	void popCtrlTrees(std::vector<std::pair<BasicBlock*,CondVal>> path);
	void printCtrlTree();
	void annotateNodesBr();
	void mergeAnnotatedNodesBr();

	void annotateCtrlFrontierAsCtrlParent();

	void mirrorCtrlNodes();
	void ConnectBrCtrls();

	void nameNodes();

	void removeOutLoopLoad();
	void removeCMERGEData();

	void printConstHist();

	void addPseudoParentsRec();

	void removeRedudantCtrl();
	void removeCMERGEChildrenOpposingCtrl();


	std::map<dfgNode*,std::map<dfgNode*,int>> edgeClassification;






private :
	dfgNode* getStartNode(BasicBlock* BB, dfgNode* PHINode);
	dfgNode* insertMergeNode(dfgNode* PHINode, dfgNode* ctrl, bool controlVal, dfgNode* data);
	dfgNode* insertMergeNode(dfgNode* PHINode, dfgNode* ctrl, bool controlVal, int val);

	dfgNode* addLoadParent(Instruction* ins, dfgNode* child);

	void DFSCtrlPath(std::map<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>>& ctrlBBInfo, std::vector<std::pair<BasicBlock*,CondVal>> path);

	std::map<BasicBlock*,int> startNodeConsts;
	std::map<BasicBlock*,dfgNode*> startNodes;

	std::set<dfgNode*> backedgeChildMergeNodes;

	std::map<dfgNode*,std::set<dfgNode*>> mutexNodes;


	//
	std::map<dfgNode*,dfgNode*> cmergeCtrlInputs;
	std::map<dfgNode*,dfgNode*> cmergeDataInputs;

	std::map<dfgNode*,dfgNode*> cmergePHINodes;

	std::set<std::set<dfgNode*>> mutexSets;
	std::map<dfgNode*,dfgNode*> selectPHIAncestorMap;

	std::map<dfgNode*,std::set<dfgNode*>> leafControlInputs;
	DominatorTree* DT;

	std::map<BasicBlock*,dfgNode*> BrParentMap;
	std::map<dfgNode*,BasicBlock*> BrParentMapInv;
//	std::vector<std::set<dfgNode*>> subPathDFGs;
	std::map<std::set<dfgNode*>,std::vector<std::pair<BasicBlock*,CondVal>> > subPathDFGMap;

	std::vector<TreeNode<BasicBlock*>*> CtrlTrees;
	std::vector<std::pair<BasicBlock*,CondVal>> longestPath;

	std::string mUnitName;
	Loop* currLoop;

	std::set<dfgNode*> pCMPNodes;
	std::map<dfgNode*,dfgNode*> nCMP2pCMP;
	std::map<dfgNode*,dfgNode*> pCMP2nCMP;

};

