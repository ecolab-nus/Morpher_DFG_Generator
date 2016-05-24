#ifndef DFGNODE_H
#define DFGNODE_H

#include "edge.h"

class CGRANode;
class DFG;


using namespace llvm;


class dfgNode{
		private :
			int idx;
			int DFSidx;
			Instruction* Node;
			std::vector<Instruction*> Children;
			std::vector<Instruction*> Ancestors;

			std::vector<Instruction*> RecChildren;
			std::vector<Instruction*> RecAncestors;

			std::vector<Instruction*> PHIchildren;
			std::vector<Instruction*> PHIAncestors;

			DFG* Parent;

			int ASAPnumber = -1;
			int ALAPnumber = -1;

			int schIdx = -1;

			CGRANode* mappedLoc = NULL;
			std::vector<CGRANode*> routingLocs;

			int routingPossibilities = 0;
			int mappedRealTime = 0;

		public :
			dfgNode(Instruction *ins, DFG* parent);
			dfgNode(){}

			void setIdx(int Idx);

			int getIdx();

			void setDFSIdx(int dfsidx);

			int getDFSIdx();

			std::vector<Instruction*>::iterator getChildIterator();

			std::vector<Instruction*> getChildren();
			std::vector<Instruction*> getAncestors();
			std::vector<Instruction*> getRecChildren(){return RecChildren;};
			std::vector<Instruction*> getRecAncestors(){return RecAncestors;};
			std::vector<Instruction*> getPHIchildren(){return PHIchildren;}
			std::vector<Instruction*> getPHIancestors(){return PHIAncestors;}

			Instruction* getNode();

			void addChild(Instruction *child, int type=EDGE_TYPE_DATA);
			void addAncestor(Instruction *anc, int type=EDGE_TYPE_DATA);
			void addRecChild(Instruction *child, int type=EDGE_TYPE_LDST);
			void addRecAncestor(Instruction *anc, int type=EDGE_TYPE_LDST);
			void addPHIchild(Instruction *child, int type=EDGE_TYPE_PHI);
			void addPHIancestor(Instruction *anc, int type=EDGE_TYPE_PHI);

			int removeChild(Instruction *child);
			int removeAncestor(Instruction *anc);
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

			void setRoutingPossibilities(int n){routingPossibilities = n;}
			int getRoutingPossibilities(){return routingPossibilities;}

			void setMappedRealTime(int t){mappedRealTime = t;}
			int getmappedRealTime(){return mappedRealTime;}


	};


#endif
