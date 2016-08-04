#ifndef CGRA_H
#define CGRA_H

#include "edge.h"
#include "dfgnode.h"
#include "CGRANode.h"


using namespace llvm;

enum Port{NORTH,SOUTH,EAST,WEST,R0,R1,R2,R3,TILE};
enum ArchType{DoubleXBar,RegXbar,LatchXbar};

struct CGRAEdge{
	CGRANode* Src;
	Port SrcPort;
	CGRANode* Dst;
	Port DstPort;
	Edge* mappedDFGEdge;
	CGRAEdge(CGRANode* Src, Port SrcPort, CGRANode* Dst, Port DstPort) : Src(Src), SrcPort(SrcPort), Dst(Dst), DstPort(DstPort), mappedDFGEdge(NULL){}
};

class CGRA{
		private :
			std::vector<std::vector<std::vector<CGRANode*> > > CGRANodes;
//			std::map<CGRANode*,std::vector<CGRANode*> > CGRAEdges;
			std::map<CGRANode*,std::vector<CGRAEdge>> CGRAEdges;
			std::map<Port,std::vector<Port> > InOutPortMap;

			void connectNeighbors();
			void connectNeighborsMESH();
			void connectNeighborsSMART();
			void connectNeighborsGRID();

			int MII;
			int XDim;
			int YDim;
			int regsPerNode;
			ArchType arch;
			std::vector<std::vector<int> > phyConMat;

		public :
			CGRA(){};
			CGRA(int MII, int Xdim, int Ydim, int regs, ArchType aType = DoubleXBar);
			CGRANode* getCGRANode(int t, int y, int x);
			CGRANode* getCGRANode(int phyLoc);



			int getXdim();
			int getYdim();
			int getMII();

			int getConMatIdx(int t, int y, int x);
			std::vector<std::vector<int> > getPhyConMat(){return phyConMat;}
			std::vector<int> getPhyConMatNode(CGRANode* cnode){return phyConMat[getConMatIdx(cnode->getT(),cnode->getY(),cnode->getX())];}

			std::map<CGRANode*,std::vector<CGRAEdge> >* getCGRAEdges(){return &CGRAEdges;}
			void setCGRAEdges(std::map<CGRANode*,std::vector<CGRAEdge> > cgraEdges){CGRAEdges = cgraEdges;}

			void clearMapping();

			int getRegsPerNode(){return regsPerNode;}
			std::vector<CGRAEdge> findCGRAEdges(CGRANode* currCNode, Port inPort, std::map<CGRANode*,std::vector<CGRAEdge>>* cgraEdgesPtr);
	};


#endif
