#include "CGRA.h"

void CGRA::connectNeighbors() {

	int sizeAllPhyNodes = MII*YDim*XDim;

	for (int i = 0; i < sizeAllPhyNodes; ++i) {
		std::vector<int> temp;
		for (int j = 0; j < sizeAllPhyNodes; ++j) {
			temp.push_back(0);
		}
		phyConMat.push_back(temp);
	}


	for (int t = 0; t < MII; ++t) {
		for (int y = 0; y < YDim; ++y) {
			for (int x = 0; x < XDim; ++x) {

				//North
				if(y-1 >= 0){
					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(t+1)%MII][y-1][x],1,"north");
					phyConMat[getConMatIdx(t,y,x)][getConMatIdx((t+1)%MII,y-1,x)] = 1;
				}

				//East
				if(x+1 < XDim){
					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(t+1)%MII][y][x+1],1,"east");
					phyConMat[getConMatIdx(t,y,x)][getConMatIdx((t+1)%MII,y,x+1)] = 1;
				}

				//South
				if(y+1 < YDim){
					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(t+1)%MII][y+1][x],1,"south");
					phyConMat[getConMatIdx(t,y,x)][getConMatIdx((t+1)%MII,y+1,x)] = 1;
				}

				//West
				if(x-1 >= 0){
					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(t+1)%MII][y][x-1],1,"west");
					phyConMat[getConMatIdx(t,y,x)][getConMatIdx((t+1)%MII,y,x-1)] = 1;
				}

				//Time
//				if(t+1 < MII){
//					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(t+1)%MII][y][x],0,"next_cycle");
//				}

				for (int i = t+1; i < t+MII; ++i) {
					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(i)%MII][y][x],i-(t+1),"REGconnections");
					phyConMat[getConMatIdx(t,y,x)][getConMatIdx((i)%MII,y,x)] = 1;
				}

				//Connecting it self
				phyConMat[getConMatIdx(t,y,x)][getConMatIdx(t,y,x)] = 1;


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

//	connectNeighbors();
//	connectNeighborsMESH();
	connectNeighborsSMART();
//	connectNeighborsGRID();

}

int CGRA::getXdim() {
	return XDim;
}

int CGRA::getYdim() {
	return YDim;
}

CGRANode* CGRA::getCGRANode(int t, int y, int x) {

	if(t >= getMII()){
		errs() << "t = " << t << " MII = " << getMII() << "\n";
	}

	assert(t < getMII());
	assert(y < getYdim());
	assert(x < getXdim());

	return &CGRANodes[t][y][x];
}

CGRANode* CGRA::getCGRANode(int phyLoc) {
	assert(phyLoc < getMII()*getYdim()*getXdim());

	int t = phyLoc/(getYdim()*getXdim());
	int y = (phyLoc % getMII())/(getXdim());
	int x = ((phyLoc % getMII()) % getYdim());

	return &CGRANodes[t][y][x];
}

void CGRA::connectNeighborsMESH() {
	for (int t = 0; t < MII; ++t) {
		for (int y = 0; y < YDim; ++y) {
			for (int x = 0; x < XDim; ++x) {

				for (int yy = 0; yy < YDim; ++yy) {
					for (int xx = 0; xx < XDim; ++xx) {
						CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(t+1)%MII][yy][xx],abs(yy-y) + abs(xx-x) + 1,"mesh");
					}
				}

				for (int i = t+1; i < t+MII; ++i) {
					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(i)%MII][y][x],i-(t+1),"REGconnections");
				}

				CGRANodes[t][y][x].sortConnectedNodes();

			}
		}
	}

}

void CGRA::connectNeighborsSMART() {
	for (int t = 0; t < MII; ++t) {
		for (int y = 0; y < YDim; ++y) {
			for (int x = 0; x < XDim; ++x) {

				//Connect All the nodes in the time axis
//				for (int i = t+1; i < t+MII; ++i) {
//					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(i)%MII][y][x],i-(t+1),"REGconnections");

				for (int reg = 0; reg < 8; ++reg) {
					CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[(t+1)%MII][y][x]);
				}
//				}

//				for (int yy = 0; yy < YDim; ++yy) {
//					for (int xx = 0; xx < XDim; ++xx) {

						if(x > 0){
							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[t][y][x-1]);
						}

						if(x < XDim - 1){
							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[t][y][x+1]);
						}

						if(y > 0){
							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[t][y-1][x]);
						}

						if(y < YDim - 1){
							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[t][y+1][x]);
						}
//						CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(t+1)%MII][yy][xx],abs(yy-y) + abs(xx-x) + 1,"mesh");
//					}
//				}

			}
		}
	}

}

int CGRA::getMII() {
	return MII;
}

void CGRA::removeEdge(CGRANode* a, CGRANode* b) {
	assert(a->getT() == b->getT());
	if(CGRAEdges.find(a) != CGRAEdges.end()){
		std::vector<CGRANode*> *vec = &(CGRAEdges.find(a)->second);
		vec->erase(std::remove(vec->begin(), vec->end(), b), vec->end());
	}
}


int CGRA::getConMatIdx(int t, int y, int x) {
	return t*YDim*XDim + y*XDim + x;
}

void CGRA::connectNeighborsGRID() {
	for (int t = 0; t < MII; ++t) {
		for (int y = 0; y < YDim; ++y) {
			for (int x = 0; x < XDim; ++x) {

				for (int reg = 0; reg < 4; ++reg) {
					CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[(t+1)%MII][y][x]);
				}

//				for (int yy = 0; yy < YDim; ++yy) {
//					for (int xx = 0; xx < XDim; ++xx) {

						if(x > 0){
							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[(t+1)%MII][y][x-1]);
						}

						if(x < XDim - 1){
							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[(t+1)%MII][y][x+1]);
						}

						if(y > 0){
							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[(t+1)%MII][y-1][x]);
						}

						if(y < YDim - 1){
							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[(t+1)%MII][y+1][x]);
						}
//						CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(t+1)%MII][yy][xx],abs(yy-y) + abs(xx-x) + 1,"mesh");
//					}
//				}




			}
		}
	}
}

void CGRA::clearMapping() {

}
