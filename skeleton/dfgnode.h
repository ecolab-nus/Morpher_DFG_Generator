#ifndef DFGNODE_H
#define DFGNODE_H

#include "edge.h"

class CGRANode;
class DFG;

enum Port{NORTH,SOUTH,EAST,WEST,R0,R1,R2,R3,R4,R5,R6,R7,TILE,TREG,OP1,OP2,PRED,TILEIN,TILEOUT,INV};
enum HyCUBEIns{NOP,ADD,SUB,MUL,SEXT,DIV,LS,RS,ARS,AND,OR,XOR,SELECT,CMERGE,CMP,CLT,BR,CGT,LOADCL,MOVCL,Hy_LOAD,Hy_LOADH,Hy_LOADB,Hy_STORE,Hy_STOREH,Hy_STOREB,JUMPL,MOVC};

struct pathData{
	CGRANode* cnode;
	Port lastPort;
	int SCpathLength;
	int TdimPathLength;
	pathData(CGRANode* cnode, Port port, int SCpathLength, int TdimPathLength) : cnode(cnode), lastPort(port), SCpathLength(SCpathLength), TdimPathLength(TdimPathLength){}
	pathData(){}
};

struct pathDataComparer
{
    bool operator()(const pathData & Left, const pathData & Right) const
    {
        return (Left.cnode < Right.cnode) || (Left.cnode == Right.cnode && Left.lastPort < Right.lastPort);
    }
};

enum CondVal{UNCOND,TRUE,FALSE};

using namespace llvm;




class dfgNode{
		private :
			int idx;
			int DFSidx;
			Instruction* Node = NULL;
			std::vector<Instruction*> Children;
			std::vector<Instruction*> Ancestors;
			std::vector<Instruction*> RecChildren;
			std::vector<Instruction*> RecAncestors;
//			std::vector<Instruction*> PHIchildren;
			std::vector<Instruction*> PHIAncestors;

			std::vector<dfgNode*> ChildNodes;
			std::vector<dfgNode*> AncestorNodes;
			std::vector<dfgNode*> RecChildNodes;
			std::vector<dfgNode*> RecAncestorNodes;
			std::vector<dfgNode*> PHIchildNodes;
			std::vector<dfgNode*> PHIAncestorNodes;
			std::string nameType;
			HyCUBEIns finalIns = NOP;

			DFG* Parent;

			int ASAPnumber = -1;
			int ALAPnumber = -1;

			int schIdx = -1;

			CGRANode* mappedLoc = NULL;
			std::vector<CGRANode*> routingLocs;
			std::map<dfgNode*,std::pair<dfgNode*,dfgNode*>> sourceRoutingPath;
			std::map<dfgNode*,std::vector<pathData*>> treeBasedRoutingLocs;
			std::map<dfgNode*,std::vector<pathData*>> treeBasedGoalLocs;


			int routingPossibilities = 0;
			int mappedRealTime = 0;
			bool isMEMOp = false;

			//OuterLoop addresses
			int outloopAddr=-1;
			bool transferedByHost=false;
			int GEPbaseAddr=-1;
			int typeSizeBytes=-1;

			//load/store
			int leftAlignedMemOP=0;  //0 - any left or right , 1 - left , 2 - right

			//Constant
			int32_t constVal=-1;
			bool constValFlag=false;

			//negated predicated bit
			bool npb=false;

			//served phi index



		public :

			//For XML
			std::vector<int> ChildNodesIdx;
			std::vector<int> AncestorNodesIdx;
			std::vector<int> InEdgesIdx;
			std::vector<int> OutEdgesIdx;

			std::vector<Instruction*> PHIchildren;
			const BasicBlock* BB;

			//Parent Nodes
			std::map<int,dfgNode*> parentClassification;


			dfgNode(Instruction *ins, DFG* parent);
//			dfgNode(){}
			dfgNode(DFG* parent) : Parent(parent){}
			void setIdx(int Idx);

			int getIdx();

			void setDFSIdx(int dfsidx);

			int getDFSIdx();

			std::vector<Instruction*>::iterator getChildIterator();

//			std::vector<Instruction*> getChildren(){return Children;};
//			std::vector<Instruction*> getAncestors(){return Ancestors;};
						std::vector<dfgNode*> getChildren(){return ChildNodes;};
						std::vector<dfgNode*> getAncestors(){return AncestorNodes;};
//			std::vector<Instruction*> getRecChildren(){return RecChildren;};
//			std::vector<Instruction*> getRecAncestors(){return RecAncestors;};
//			std::vector<Instruction*> getPHIchildren(){return PHIchildren;}
//			std::vector<Instruction*> getPHIancestors(){return PHIAncestors;}
						std::vector<dfgNode*> getRecChildren(){return RecChildNodes;};
						std::vector<dfgNode*> getRecAncestors(){return RecAncestorNodes;};
						std::vector<dfgNode*> getPHIchildren(){return PHIchildNodes;}
						std::vector<dfgNode*> getPHIancestors(){return PHIAncestorNodes;}


//			void addChildNode(dfgNode* node){ChildNodes.push_back(node);}
//			void addAncestorNode(dfgNode* node){AncestorNodes.push_back(node);}
			void addChildNode(dfgNode* node, int type=EDGE_TYPE_DATA, bool isBackEdge=false, bool isControlDependent=false, bool ControlValue=true);
			void addAncestorNode(dfgNode* node, int type=EDGE_TYPE_DATA, bool isBackEdge=false, bool isControlDependent=false, bool ControlValue=true);
			void addRecChildNode(dfgNode* node){RecChildNodes.push_back(node);}
			void addRecAncestorNode(dfgNode* node){RecAncestorNodes.push_back(node);}
			void addPHIChildNode(dfgNode* node){PHIchildNodes.push_back(node);}
//			void addPHIAncestorNode(dfgNode* node){PHIAncestorNodes.push_back(node);}
			void addPHIAncestorNode(dfgNode* node);

			Instruction* getNode();

			void addChild(Instruction *child, int type=EDGE_TYPE_DATA);
			void addAncestor(Instruction *anc, int type=EDGE_TYPE_DATA);


			void addRecChild(Instruction *child, int type=EDGE_TYPE_LDST);
			void addRecAncestor(Instruction *anc, int type=EDGE_TYPE_LDST);
			void addPHIchild(Instruction *child, int type=EDGE_TYPE_PHI);
			void addPHIancestor(Instruction *anc, int type=EDGE_TYPE_PHI);

			int removeChild(Instruction *child);
			int removeChild(dfgNode *child);
			int removeAncestor(Instruction *anc);
			int removeAncestor(dfgNode *anc);
			int removeRecChild(Instruction *child);
			int removeRecAncestor(Instruction *anc);

			void setASAPnumber(int n);
			void setALAPnumber(int n);

			int getASAPnumber();
			int getALAPnumber();

			void setSchIdx(int n);
			int getSchIdx();

			void setMappedLoc(CGRANode* cNode);
			CGRANode* getMappedLoc();

			std::vector<CGRANode*>* getRoutingLocs();
			std::map<dfgNode*,std::pair<dfgNode*,dfgNode*>>* getSourceRoutingPath(){return &sourceRoutingPath;}
			std::map<dfgNode*,std::vector<pathData*>>* getTreeBasedRoutingLocs(){return &treeBasedRoutingLocs;}
			std::map<dfgNode*,std::vector<pathData*>>* getTreeBasedGoalLocs(){return &treeBasedGoalLocs;}

			void setRoutingPossibilities(int n){routingPossibilities = n;}
			int getRoutingPossibilities(){return routingPossibilities;}

			void setMappedRealTime(int t){mappedRealTime = t;}
			int getmappedRealTime(){return mappedRealTime;}

			std::map<dfgNode*,std::vector<pathData* >> getMergeRoutingLocs();

			void setNameType(std::string name){nameType = name;}
			std::string getNameType(){return nameType;}

			void setIsMemOp(bool b){isMEMOp = b;}
			bool getIsMemOp(){return isMEMOp;}

			bool isConditional();

			void setFinalIns(HyCUBEIns ins);
			HyCUBEIns getFinalIns(){return finalIns;}

			//Out of loop instructions
			dfgNode* addStoreChild(Instruction * ins);
			dfgNode* addLoadParent(Instruction * ins);

			//Memory Allocation
			int getoutloopAddr();
			void setoutloopAddr(int addr);
			bool isOutLoop();
			int getGEPbaseAddr();
			void setGEPbaseAddr(int addr);
			bool isGEP();
			void setTypeSizeBytes(int size){typeSizeBytes = size;}
			int getTypeSizeBytes(){return typeSizeBytes;}

			//Add CMerge Child
			dfgNode* addCMergeParent(dfgNode* phiBRAncestor,dfgNode* phiDataAncestor=NULL,int32_t constVal=-1, bool selectOp=false);
			dfgNode* addCMergeParent(dfgNode* phiBRAncestor,Instruction* outLoopLoadIns);
			dfgNode* addCMergeParent(Instruction* phiBRAncestorIns,Instruction* outLoopLoadIns);
			dfgNode* addCMergeParent(Instruction* phiBRAncestorIns,int32_t constVal);
			dfgNode* addCMergeParent(Instruction* phiBRAncestorIns,dfgNode* phiDataAncestor);

			//ConstantVal
			int getConstantVal(){assert(constValFlag);return constVal;}
			void setConstantVal(int val){constVal = val;constValFlag = true;}
			bool hasConstantVal(){return constValFlag;}
			bool isTransferedByHost() const {return transferedByHost;}
			void setTransferedByHost(bool transferedByHost = false) {this->transferedByHost = transferedByHost;}

			//negated predicated bit
			void setNPB(bool val){npb = val;}
			bool getNPB(){return npb;}

			//left aligned memop
			int getLeftAlignedMemOp(){return leftAlignedMemOP;}
			void setLeftAlignedMemOp(int leftAlignedMemOp) {leftAlignedMemOP = leftAlignedMemOp;}

			bool isParent(dfgNode* parent);
			void printName();

			//triggered
			std::map<dfgNode*,bool> childBackEdgeMap;
			std::map<dfgNode*,bool> ancestorBackEdgeMap;
			std::map<dfgNode*,CondVal> childConditionalMap;
			std::map<dfgNode*,CondVal> ancestorConditionaMap;

			void setBackEdge(dfgNode* child, bool val);


};


#endif
