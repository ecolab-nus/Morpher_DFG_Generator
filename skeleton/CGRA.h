#ifndef CGRA_H
#define CGRA_H

#include "edge.h"
#include "dfgnode.h"
#include "CGRANode.h"


using namespace llvm;

enum ArchType{DoubleXBar,RegXbar,LatchXbar,RegXbarTREG,StdNOC,NoNOC};

struct CGRAEdge{
	CGRANode* Src;
	Port SrcPort;
	CGRANode* Dst;
	Port DstPort;
	Edge* mappedDFGEdge;
	CGRAEdge(CGRANode* Src, Port SrcPort, CGRANode* Dst, Port DstPort) : Src(Src), SrcPort(SrcPort), Dst(Dst), DstPort(DstPort), mappedDFGEdge(NULL){}
};

struct CGRANodeNew{
	CGRANode* oldCnode;
	int unwrappedTime;
	CGRANodeNew(CGRANode* oldCnode, int unwrappedTime) : oldCnode(oldCnode), unwrappedTime(unwrappedTime){}
};

struct CGRAEdgeNew{
	CGRAEdge CEdge;
	CGRANodeNew* Src;
	CGRANodeNew* Dest;
	CGRAEdgeNew(CGRAEdge CEdge, CGRANodeNew* Src, CGRANodeNew* Dest);
	CGRAEdgeNew();
};

class CGRA{
		private :
			std::vector<std::vector<std::vector<CGRANode*> > > CGRANodes;
			std::map<CGRANode*,std::vector<CGRAEdge>> CGRAEdges;

			std::vector<std::vector<std::vector<CGRANodeNew*> > > CGRANodesNew;
			std::map<CGRANodeNew*,std::vector<CGRAEdgeNew>> CGRAEdgesNew;

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

			//Creating unwrapped substrate


		public :
			CGRA(){};
			CGRA(int MII, int Xdim, int Ydim, int regs, ArchType aType = DoubleXBar);
			CGRANode* getCGRANode(int t, int y, int x);
			CGRANode* getCGRANode(int phyLoc);
			std::map<Port,std::vector<Port> > OrigInOutPortMap;



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
			std::vector<CGRAEdge*> findCGRAEdges(CGRANode* currCNode, Port inPort, std::map<CGRANode*,std::vector<CGRAEdge>>* cgraEdgesPtr);
			static std::string getPortName(Port p);

			ArchType getArch(){return arch;}
			int getTotalUnUsedMemPEs();

			std::vector<CGRAEdge> getCGRAEdgesWithDest(CGRANode* Cdst);
			std::vector<CGRAEdge*> getCGRAEdgesWithDest(CGRANode* Cdst, std::map<CGRANode*,std::vector<CGRAEdge>>* cgraEdgesPtr);

			//Creating unwrapped substrate
			void addIINewNodes();
	};


#endif
