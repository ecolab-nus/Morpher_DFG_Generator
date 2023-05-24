#ifndef DFG_H
#define DFG_H

#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <iostream>
#include <fstream>

#include <unordered_set>
#include <unordered_map>
#include <morpherdfggen/arch/CGRA.h>
#include <morpherdfggen/arch/CGRANode.h>
#include <morpherdfggen/common/dfgnode.h>
#include <morpherdfggen/common/edge.h>


#define REGS_PER_NODE 4
//Uncomment this if compiling for PACE0.5
#define ARCHI_16BIT
extern int MEM_SIZE;

class AStar;

using namespace llvm;

struct less_than_schIdx{
	inline bool operator()(dfgNode* node1, dfgNode* node2){
		return (node1->getSchIdx() < node2->getSchIdx());
	}
};

struct exitNode{
	dfgNode* ctrlNode;
	bool ctrlVal;
	BasicBlock* src;
	BasicBlock* dest;

	const bool operator< (const exitNode& other) const{
		if(src != other.src || dest != other.dest){
			return true;
		}
		else{
			return false;
		}
	}
	exitNode(dfgNode* cn, bool cv, BasicBlock* s, BasicBlock* d) : ctrlNode(cn), ctrlVal(cv), src(s), dest(d){}
};

struct ScheduleOrder{
	inline bool operator()(dfgNode* node1, dfgNode* node2){

		int slack1 = node1->getALAPnumber() - node1->getASAPnumber();
		int slack2 = node2->getALAPnumber() - node2->getASAPnumber();

		if(node1->getALAPnumber() < node2->getALAPnumber()){
			return true;
		}
		else{
			return false;
		}

//		if(node1.getALAPnumber() < node2.getALAPnumber()){
//			return true;
//		}
//		else if(node1.getALAPnumber() > node2.getALAPnumber()){
//			return false;
//		}
//		else if(slack1 == slack2){ //node1.getALAPnumber() == node2.getALAPnumber()
//			return(node1.getASAPnumber() < node2.getASAPnumber());
//		}
//		else if(slack1 < slack2){
//			return true;
//		}
//		else{ // slack1 > slack2
//			return false;
//		}

	}
};

struct ValueComparer{
	inline bool operator()(const std::pair<Instruction*,int> a, const std::pair<Instruction*,int> b){
		if(a.second < b.second){
			return true;
		}
		else{
			return false;
		}
	}
};

struct CostComparer{
	inline bool operator()(const ConnectedCGRANode a, const ConnectedCGRANode b){
		if(a.cost < b.cost){
			return true;
		}
		else{
			return false;
		}
	}
};

struct phyLoc{
	int x;
	int y;
	int t;
};

struct nodeWithCost{
	dfgNode* node;
	CGRANode* cnode;
	int cost;
	int mappedRealTime;
	int routingCost;
	int affinityCost;
	nodeWithCost(dfgNode* node, CGRANode* cnode, int cost, int mappedRealTime) : node(node), cnode(cnode), cost(cost), mappedRealTime(mappedRealTime){}
};

struct LessThanNodeWithCost{
    bool operator()(nodeWithCost const & p1, nodeWithCost const & p2) {
        // return "true" if "p1" is ordered before "p2", for example:
        return p1.cost < p2.cost;
    }
};

struct TreePath{
	std::vector<CGRANode*> sources;
	std::map<CGRANode*,Port> sourcePorts;
	std::map<CGRANode*,int> sourceSCpathLengths;
	std::map<CGRANode*,int> sourceSCpathTdimLengths;
	std::map<CGRANode*,std::pair<dfgNode*,dfgNode*> > sourcePaths;
	CGRANode* bestSource;
	int bestCost;
	CGRANode* dest;
	CGRANode* PHIDest;
	TreePath() : bestSource(NULL), dest(NULL), bestCost(-1), PHIDest(NULL){}
};

typedef struct{
	std::map<Port,uint16_t> outMap;
	uint16_t regwen;
	uint16_t regbypass;
	uint16_t tregwen;
	uint16_t opcode;
	uint32_t constant;
	uint8_t constantValid;
	uint8_t npb;
} binOp;

struct munitTransition{
	int id;
	BasicBlock* srcBB;
	BasicBlock* destBB;
};

struct munitTransitionComparer
{
    bool operator()(const munitTransition & Left, const munitTransition & Right) const
    {
        return (Left.srcBB < Right.srcBB) || (Left.srcBB == Right.srcBB && Left.destBB < Right.destBB);
    }
};

enum MemOp   {LOAD,STORE,INVALID};
enum DFGType {NOLOOP,OUTLOOP,INLOOP};

//only for hycube simulation

enum SPM_BANK
{
	BANK0,
	BANK1,
	BANK2,
	BANK3,
	BANK4,
	BANK5,
	BANK6,
	BANK7
};
static SPM_BANK SPMBANKOfIndex(int i) { return static_cast<SPM_BANK>(i); }
//only for hycube simulation

class DFG{
		protected :
			std::vector<dfgNode*> NodeList;
			std::ofstream xmlFile;
			std::vector<Edge> edgeList;
			int maxASAPLevel = -1;
			int maxRecDist = -1;
			std::map<const BasicBlock*,std::vector<const BasicBlock*>> BBSuccBasicBlocks;
			bool deadEndReached = false;
			std::string name;
			std::string kernelname = "kernel";
			std::vector<nodeWithCost> globalNodesWithCost;
//			std::vector<std::vector<int> > conMat;
			std::vector<std::vector<std::vector<unsigned char> > > conMatArr;

			//the pointer which the anyother outerloop load and stores should be written to
			uint32_t outloopAddrPtrRight = MEM_SIZE - 1; // -1 for CL
			uint32_t outloopAddrPtrLeft = MEM_SIZE/2 - 2; // -2 for {CL,loopstart}

//			uint32_t arrayAddrPtr=0;
			uint32_t arrayAddrPtrRight = MEM_SIZE/2;
			uint32_t arrayAddrPtrLeft = 0;

			//LoopInfo if its a loop
			Loop* L=NULL;
			std::set<BasicBlock*> loopBB;
			std::set<std::pair<BasicBlock*,BasicBlock*>> loopentryBB;
			std::set<std::pair<BasicBlock*,BasicBlock*>> loopexitBB;
			int loopIdx=-1;
			std::map<Loop*,std::string>* loopNamesPtr;

			//macro placement space




			void renumber();
			void traverseBFS(dfgNode* node, int ASAPlevel);
			void traverseInvBFS(dfgNode* node, int ALAPlevel);

			std::vector<dfgNode*> schedule;
			void traverseCriticalPath(dfgNode* node, int level);

			CGRA* currCGRA=NULL;
			std::vector<ConnectedCGRANode> searchCandidates(CGRANode* mappedLoc, dfgNode* node, std::vector<std::pair<Instruction*,int>>* candidateNumbers);
			void eraseAlreadyMappedNodes(std::vector<ConnectedCGRANode>* candidates);
			void backTrack(int nodeSeq);

			void ExpandCandidatesAddingRoutingNodes(std::vector<std::pair<Instruction*,int>>* candidateNumbers);
			std::vector<ConnectedCGRANode> getConnectedCGRANodes(dfgNode* node);

			int getConMatIdx(int t, int y, int x);

			bool MapMultiDestRec(std::map<dfgNode*,std::vector< std::pair<CGRANode*,int> > > *nodeDestMap,
					             std::map<CGRANode*,std::vector<dfgNode*> > *destNodeMap,
								 std::map<dfgNode*,std::vector< std::pair<CGRANode*,int> > >::iterator it,
								 std::map<CGRANode*,std::vector<CGRAEdge> > cgraEdges,
								 int index);

			int CalculateGEPBaseAddr(GetElementPtrInst *GEP);
			std::unordered_map<GetElementPtrInst*,int> GEPOffsetMap;


		public :
			std::ofstream mappingOutFile;
			AStar* astar;
			std::map<Value*,dfgNode*> OutLoopNodeMap;
			std::map<dfgNode*,Value*> OutLoopNodeMapReverse;
			std::map<Instruction*,dfgNode*> LoopStartMap;

			std::map<HyCUBEIns,int> hyCUBEInsHist;

			DFG(std::string name,std::map<Loop*,std::string>* lnPtr);


			void setName(std::string str){name = str;}
			void setKernelName(std::string str){kernelname = str;}
			std::string getName(){return name;}

			dfgNode* getEntryNode();

			std::vector<dfgNode*> getNodes();
			std::vector<dfgNode*>* getNodesPtr(){return &NodeList;}

			std::vector<Edge> getEdges();
			void InsertNode(Instruction* Node);

			void setNodes(std::vector<dfgNode*> nodes){NodeList = nodes;}

//			void InsertNode(dfgNode Node);

			void InsertEdge(Edge e);

			dfgNode* findNode(Instruction* I);

			Edge* findEdge(dfgNode* src, dfgNode* dest, CGRANode* goal=NULL);

			std::vector<dfgNode*> getRoots();

			std::vector<dfgNode*> getLeafs(BasicBlock* BB);
			std::vector<dfgNode*> getLeafs();

			void connectBB();

			void addMemDepEdges(MemoryDependenceResults *MD);
			void addMemRecDepEdges(DependenceInfo *DI);
			void addMemRecDepEdgesNew(DependenceInfo *DI);
			MemOp isMemoryOp(dfgNode* node);

		    int removeEdge(Edge* e);
		    int removeNode(dfgNode* n);
			void removeAlloc();

			void traverseDFS(dfgNode* startNode, int dfsCount=0);


			void printXML();

			void printInEdges(dfgNode* node, int depth = 0);

			void printOutEdges(dfgNode* node, int depth = 0);

			void printOP(dfgNode* node, int depth=0);

			void printOPs(int depth=0);

			void printDFGInfo();

			void printHeaderTag(std::string tagName, int depth=0);

			void printFooterTag(std::string tagName, int depth=0);

			void printEdges(int depth = 0);

			void printEdge(Edge* e, int depth = 0);

			void scheduleASAP();
			void scheduleALAP();
			void balanceASAPALAP();

			void MapCGRA(int XDim, int YDim);
			void CreateSchList();

			std::vector<ConnectedCGRANode> FindCandidateCGRANodes(dfgNode* node);

			bool MapCGRA_SMART(int XDim, int YDim, ArchType arch = RegXbarTREG, int bTrack = 100, int initMII=1);
			void MapCGRA_SA(int XDim, int YDim, std::string mapfileName = "Mapping.log");
			bool MapMultiDest(std::map<dfgNode*,std::vector< std::pair<CGRANode*,int> > > *nodeDestMap, std::map<CGRANode*,std::vector<dfgNode*> > *destNodeMap);
			bool MapASAPLevel(int MII, int XDim, int YDim, ArchType arch);
			bool MapASAPLevelUnWrapped(int MII, int XDim, int YDim, ArchType arch);
			int getAffinityCost(dfgNode* a, dfgNode* b);

			std::vector<std::vector<unsigned char> > getConMat();
			void printConMat(std::vector<std::vector<unsigned char> > conMat);
			std::vector<std::vector<unsigned char> > selfMulConMat(std::vector<std::vector<unsigned char> > in);
			std::map<dfgNode*,std::vector<CGRANode*> > getPrimarySlots(std::vector<dfgNode*> nodes);

			std::vector<int> getIntersection(std::vector<std::vector<int> > vectors);
			int AddRoutingEdges(dfgNode* node);
			int AStarSP(CGRANode* src, CGRANode* dest, std::vector<CGRANode*>* path);
			int getDist(CGRANode* a, CGRANode*b);

			void setMaxRecDist (int d){maxRecDist = d;}
			int getMaxRecDist(){return maxRecDist;}

			void findMaxRecDist();

			void setBBSuccBasicBlocks(std::map<const BasicBlock*,std::vector<const BasicBlock*>> map){BBSuccBasicBlocks = map;}
			std::map<const BasicBlock*,std::vector<const BasicBlock*>> getBBSuccBasicBlocks(){return BBSuccBasicBlocks;}

			void MapCGRA_EMS(int XDim, int YDim, std::string mapfileName = "Mapping.log");
			bool MapCGRA_EMS_ASAPLevel(int MII,int XDim, int YDim);
			bool MAPCGRA_EMS_MultDest(std::map<dfgNode*,std::vector< std::pair<CGRANode*,int> > > *nodeDestMap,
								      std::map<CGRANode*,std::vector<dfgNode*> > *destNodeMap,
//									  std::map<dfgNode*,std::vector< std::pair<CGRANode*,int> > >::iterator it,
									  const std::vector<nodeWithCost>::iterator it,
									  std::map<CGRANode*,std::vector<CGRAEdge> > cgraEdges,
									  int index);

			bool EMSSortNodeDest(std::map<dfgNode*,std::vector< std::pair<CGRANode*,int> > > *nodeDestMap,
									 std::map<CGRANode*,std::vector<CGRAEdge> > cgraEdges,
									 int index);

			TreePath createTreePath(dfgNode* parent, CGRANode* dest);
			void clearMapping();

			CGRA* getCGRA(){return currCGRA;}

//			int findUtilTreeRoutingLocs(CGRANode* cnode, dfgNode* currNode);
			void printOutSMARTRoutes();
			int convertToPhyLoc(int t, int y, int x);
			int convertToPhyLoc(int y, int x);
			int getDistCGRANodes(CGRANode* a, CGRANode* b);
			int getStaticRoutingCost(dfgNode* node, CGRANode* dest, std::map<CGRANode*,std::vector<CGRAEdge> > Edges);


			//readXML
			int readXML(std::string fileName);

			//Print REGIMap Outs
			int printREGIMapOuts();

			//Print out turn information for each CGRANode
			int printTurns();

			//Print out switching statistics for registers
			int printRegStats();

			//Sort the possible dests in the SMART based mapper
			int sortPossibleDests(std::vector<std::pair<CGRANode*,int>>* possibleDests);

			//PrintMapping
			int printMapping();

			//Print Possible Congestion Info
			int printCongestionInfo();

			//Treat PHI Nodes
			int handlePHINodes(std::set<BasicBlock*> LoopBB);
			int phiselectInsert();
			int removeRedEdgesPHI();
			int addCMERGEtoSELECT();

			//Treat high-fan in PHI Nodes
			int handlePHINodeFanIn();

			int checkSanity();

			//Special Treatment to MEM Nodes in order to map them specialized PEs
			int handleMEMops();

			int getMEMOpsToBePlaced();
			int getOutLoopMEMOps();

			int nameNodes();
			int nameNodesCGRAME();
			std::map<HyCUBEIns,std::string> HyCUBEInsStrings;
			std::map<HyCUBEIns,uint64_t> HyCUBEInsBinary;

			int updateBinOp(binOp* binOpIns, Port outPort, Port inPort);
			std::string getArchName(ArchType arch);


			//Backtrack counter
			int initBtrack;
			int backtrackCounter = 100;

			//Handling PHIChildren
			TreePath createTreePathPHIDest(dfgNode* node, CGRANode* dest, CGRANode* phiChildDest);
			void addPHIChildEdges();
			void addPHIParents();
			dfgNode* findNodeMappedLoc(CGRANode* cnode);

			//type
			DFGType type;
			static void partitionFuncDFG(DFG* funcDFG, std::vector<DFG*> dfgVectorPtr);

			//investigate GEP
			void GEPInvestigate(Function &F, std::map<std::string,int>* sizeArrMap);
			void GEPInvestigate(Function &F, Loop* L, std::map<std::string,int>* sizeArrMap);
			std::map<std::string,uint32_t> allocatedArraysMap;

			//outerloop load and stores address assignment
			void AssignOutLoopAddr();

			int PlaceMacro(DFG* mappedDFG, int XDim, int YDim, ArchType arch);

			//LoopInfo
			std::vector<DFG*> subLoopDFGs;
			std::set<BasicBlock*> accumulatedBBs;
			void setLoop(Loop* loop){L=loop;}
			Loop* getLoop(){return L;}
			void setLoopBB(std::set<BasicBlock*> BBs){loopBB=BBs;}
			void setLoopBB(std::set<BasicBlock*> BBs,std::set<std::pair<BasicBlock*,BasicBlock*>> entryBBs, std::set<std::pair<BasicBlock*,BasicBlock*>> exitBBs){loopBB=BBs;loopentryBB=entryBBs;loopexitBB=exitBBs;}
			std::set<BasicBlock*>* getLoopBB(){return &loopBB;}
			void setLoopIdx(int i){loopIdx=i;}
			int getLoopIdx(){return loopIdx;}

			// parent classification as operand1, operand2 or predicate
			int classifyParents();
			int findOperandNumber(dfgNode* node, Instruction* child, Value* parent);

			// add not instructions for ctrlbr instructions
			int treatFalsePaths();

			//Memory Parition
			std::map<dfgNode*,int> memNodePartitionMap;
			int partitionMemNodes();

			//starting and stopping loops
			int handlestartstop();
			int handlestartstop_munit(std::vector<munitTransition> bbTrans);

			//print JUMPL header
			int printJUMPLHeader(std::ofstream& binFile, std::ofstream& binOpNameFile);


			// add shift operations before the GEPs of STOREH and STORE.
			int insertshiftGEPs();
			int insertshiftGEPsCorrect();


			//add load stores which does not use GEPs with constants.
			int nonGEPLoadStorecheck();

			//add masking functions for instruction where word length < 4 bytes.
			//Maybe we can fix this in the architecture ?
			int addMaskLowBitInstructions();

			//break longer paths by adding or 0 instructions
			int addBreakLongerPaths();

			//making node route map of printsmartroute public and accessible
			std::map<dfgNode*,std::map<dfgNode*,std::vector<CGRANode*>>> nodeRouteMap;

			int analyzeRTpaths();

			//Mutually Excluded Path Exploration
			std::map<BasicBlock*,std::set<BasicBlock*>> checkMutexBBs();

			int printHyCUBEInsHist();

			//setting sizeArrMap accesible for the DFG class
			std::map<std::string,int> sizeArrMap;
			std::unordered_map<std::string,int> array_pointer_sizes;


			std::unordered_map<Value *, int> outVals_inorout; // load:0 store:1

			void printNewDFGXML();
			void printREGIMapfiles();
			std::map<Edge*,int> xmlEdgeIdxMap;
			int xmlEdgeCount=0;

			void MergeCMerge();

			void insertMOVC();
			virtual void generateTrigDFGDOT(Function &F){assert(false);}
			virtual void generateCGRAMEDFGDOT(Function &F){assert(false);}
			virtual void PrintOuts(){assert(false);}

			void removeDisconnectedNodes();
			std::unordered_set<dfgNode*> getLineage(dfgNode* n);


//			int getMUnitTransID(BasicBlock* src, BasicBlock* dest);

			void getTransferVariables(std::unordered_set<Value*>& outer_vals,
							std::unordered_map<Value*,GetElementPtrInst*>& mem_ptrs, 
							std::unordered_map<Value*,int>& acc,
							Function& F);

			void GEPBaseAddrCheck(Function &F);
			void SetBasePointers(std::unordered_set<Value*>& outer_vals,
			                     std::unordered_map<Value*,GetElementPtrInst*>& mem_ptrs, std::map<dfgNode*,Value*> &OLNodesWithPtrTyUsage, Function &F);
			void InstrumentInOutVars(Function &F, std::unordered_map<Value *, int> mem_accesses,  std::map<dfgNode*,Value*> &OLNodesWithPtrTyUsage,std::unordered_map<Value *, int>& spm_base_address);
			void UpdateSPMAllocation(std::unordered_map<Value *, int>& spm_base_address,
			                         std::unordered_map<Value *, SPM_BANK>& spm_base_allocation,
									 std::unordered_map<Value *, GetElementPtrInst *>& arr_ptrs);

			DominatorTree* DT;
	};


#endif
