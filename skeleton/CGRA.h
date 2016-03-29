#ifndef CGRA_H
#define CGRA_H

#include "edge.h"
#include "dfgnode.h"
#include "CGRANode.h"


using namespace llvm;

class CGRA{
		private :
			std::vector<std::vector<std::vector<CGRANode> > > CGRANodes;
			void connectNeighbors();

			int MII;
			int XDim;
			int YDim;

		public :
			CGRA(int MII, int Xdim, int Ydim);
			CGRANode* getCGRANode(int t, int y, int x);


			int getXdim();
			int getYdim();
			int getMII();
	};


#endif
