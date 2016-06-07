#ifndef CGRA_H
#define CGRA_H

#include "edge.h"
#include "dfgnode.h"
#include "CGRANode.h"


using namespace llvm;

class CGRA{
		private :
			std::vector<std::vector<std::vector<CGRANode> > > CGRANodes;
			std::map<CGRANode*,std::vector<CGRANode*> > CGRAEdges;
			void connectNeighbors();
			void connectNeighborsMESH();
			void connectNeighborsSMART();
			void connectNeighborsGRID();

			int MII;
			int XDim;
			int YDim;
			std::vector<std::vector<int> > phyConMat;

		public :
			CGRA(){};
			CGRA(int MII, int Xdim, int Ydim);
			CGRANode* getCGRANode(int t, int y, int x);
			CGRANode* getCGRANode(int phyLoc);

			void removeEdge(CGRANode* a, CGRANode* b);


			int getXdim();
			int getYdim();
			int getMII();

			int getConMatIdx(int t, int y, int x);
			std::vector<std::vector<int> > getPhyConMat(){return phyConMat;}
			std::vector<int> getPhyConMatNode(CGRANode* cnode){return phyConMat[getConMatIdx(cnode->getT(),cnode->getY(),cnode->getX())];}

			std::map<CGRANode*,std::vector<CGRANode*> >* getCGRAEdges(){return &CGRAEdges;}
			void setCGRAEdges(std::map<CGRANode*,std::vector<CGRANode*> > cgraEdges){CGRAEdges = cgraEdges;}

			void clearMapping();
	};


#endif
