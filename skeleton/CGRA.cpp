#include "CGRA.h"

void CGRA::connectNeighbors() {
	for (int t = 0; t < MII; ++t) {
		for (int y = 0; y < YDim; ++y) {
			for (int x = 0; x < XDim; ++x) {

				//North
				if(y-1 >= 0){
					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(t+1)%MII][y-1][x],1,"north");
				}

				//East
				if(x+1 < XDim){
					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(t+1)%MII][y][x+1],1,"east");
				}

				//South
				if(y+1 < YDim){
					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(t+1)%MII][y+1][x],1,"south");
				}

				//West
				if(x-1 >= 0){
					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(t+1)%MII][y][x-1],1,"west");
				}

				//Time
//				if(t+1 < MII){
//					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(t+1)%MII][y][x],0,"next_cycle");
//				}

				for (int i = t+1; i < t+MII; ++i) {
					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(i)%MII][y][x],i-(t+1),"REGconnections");
				}


				CGRANodes[t][y][x].sortConnectedNodes();

			}
		}
	}
}

CGRA::CGRA(int MII, int Xdim, int Ydim) {

	this->MII = MII;
	this->XDim = Xdim;
	this->YDim = Ydim;

	for (int t = 0; t < MII; ++t) {
		std::vector<std::vector<CGRANode> > tempL2;
		for (int y = 0; y < Ydim; ++y) {
			std::vector<CGRANode> tempL1;
			for (int x = 0; x < Xdim; ++x) {
				CGRANode tempNode(x,y,t);
				tempL1.push_back(tempNode);
			}
			tempL2.push_back(tempL1);
		}
		CGRANodes.push_back(tempL2);
	}

	connectNeighbors();

}

int CGRA::getXdim() {
	return XDim;
}

int CGRA::getYdim() {
	return YDim;
}

CGRANode* CGRA::getCGRANode(int t, int y, int x) {
	assert(t < getMII());
	assert(y < getYdim());
	assert(x < getXdim());

	return &CGRANodes[t][y][x];
}

int CGRA::getMII() {
	return MII;
}
