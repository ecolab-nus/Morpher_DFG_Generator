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
			DFG* Parent;

			int ASAPnumber = -1;
			int ALAPnumber = -1;

			int schIdx = -1;

			CGRANode* mappedLoc = NULL;
			std::vector<CGRANode*> routingLocs;

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

			Instruction* getNode();

			void addChild(Instruction *child, int type=EDGE_TYPE_DATA);
			void addAncestor(Instruction *anc, int type=EDGE_TYPE_DATA);

			int removeChild(Instruction *child);
			int removeAncestor(Instruction *anc);

			void setASAPnumber(int n);
			void setALAPnumber(int n);

			int getASAPnumber();
			int getALAPnumber();

			void setSchIdx(int n);
			int getSchIdx();

			void setMappedLoc(CGRANode* cNode);
			CGRANode* getMappedLoc();

			std::vector<CGRANode*>* getRoutingLocs();


	};


#endif
