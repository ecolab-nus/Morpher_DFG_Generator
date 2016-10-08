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
					CGRANodes[t][y][x]->addConnectedNode(CGRANodes[(t+1)%MII][y-1][x],1,"north");
					phyConMat[getConMatIdx(t,y,x)][getConMatIdx((t+1)%MII,y-1,x)] = 1;
				}

				//East
				if(x+1 < XDim){
					CGRANodes[t][y][x]->addConnectedNode(CGRANodes[(t+1)%MII][y][x+1],1,"east");
					phyConMat[getConMatIdx(t,y,x)][getConMatIdx((t+1)%MII,y,x+1)] = 1;
				}

				//South
				if(y+1 < YDim){
					CGRANodes[t][y][x]->addConnectedNode(CGRANodes[(t+1)%MII][y+1][x],1,"south");
					phyConMat[getConMatIdx(t,y,x)][getConMatIdx((t+1)%MII,y+1,x)] = 1;
				}

				//West
				if(x-1 >= 0){
					CGRANodes[t][y][x]->addConnectedNode(CGRANodes[(t+1)%MII][y][x-1],1,"west");
					phyConMat[getConMatIdx(t,y,x)][getConMatIdx((t+1)%MII,y,x-1)] = 1;
				}

				//Time
//				if(t+1 < MII){
//					CGRANodes[t][y][x].addConnectedNode(&CGRANodes[(t+1)%MII][y][x],0,"next_cycle");
//				}

				for (int i = t+1; i < t+MII; ++i) {
					CGRANodes[t][y][x]->addConnectedNode(CGRANodes[(i)%MII][y][x],i-(t+1),"REGconnections");
					phyConMat[getConMatIdx(t,y,x)][getConMatIdx((i)%MII,y,x)] = 1;
				}

				//Connecting it self
				phyConMat[getConMatIdx(t,y,x)][getConMatIdx(t,y,x)] = 1;


				CGRANodes[t][y][x]->sortConnectedNodes();

			}
		}
	}

}

CGRA::CGRA(int MII, int Xdim, int Ydim, int regs, ArchType aType) {

	this->MII = MII;
	this->XDim = Xdim;
	this->YDim = Ydim;
	this->regsPerNode = regs;
	this->arch = aType;
	CGRANode* tempNodePtr;
	std::map<Port,std::vector<Port> > InOutPortMap;


//	InOutPortMap[NORTH] = {R0,R1,R2,R3,NORTH,EAST,WEST,SOUTH};
//	InOutPortMap[EAST] = {R0,R1,R2,R3,NORTH,EAST,WEST,SOUTH};
//	InOutPortMap[WEST] = {R0,R1,R2,R3,NORTH,EAST,WEST,SOUTH};
//	InOutPortMap[SOUTH] = {R0,R1,R2,R3,NORTH,EAST,WEST,SOUTH};

//	InOutPortMap[R0] = {R0,NORTH,EAST,WEST,SOUTH};
//	InOutPortMap[R1] = {R1,NORTH,EAST,WEST,SOUTH};
//	InOutPortMap[R2] = {R2,NORTH,EAST,WEST,SOUTH};
//	InOutPortMap[R3] = {R3,NORTH,EAST,WEST,SOUTH};
	switch (arch) {
		case DoubleXBar:
//			InOutPortMap[R0] = {R0,NORTH,EAST,WEST,SOUTH};
//			InOutPortMap[R1] = {R1,NORTH,EAST,WEST,SOUTH};
//			InOutPortMap[R2] = {R2,NORTH,EAST,WEST,SOUTH};
//			InOutPortMap[R3] = {R3,NORTH,EAST,WEST,SOUTH};

			switch(regsPerNode){
				case 2 :
					InOutPortMap[R0] = {R0,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R1] = {R1,NORTH,EAST,WEST,SOUTH};

					InOutPortMap[NORTH] = {R0,R1,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[EAST] = {R0,R1,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[WEST] = {R0,R1,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[SOUTH] = {R0,R1,NORTH,EAST,WEST,SOUTH};

					InOutPortMap[TILE] = {R0,R1,NORTH,EAST,WEST,SOUTH};
					break;
				case 4 :
					InOutPortMap[R0] = {R0,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R1] = {R1,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R2] = {R2,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R3] = {R3,NORTH,EAST,WEST,SOUTH};

					InOutPortMap[NORTH] = {R0,R1,R2,R3,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[EAST] = {R0,R1,R2,R3,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[WEST] = {R0,R1,R2,R3,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[SOUTH] = {R0,R1,R2,R3,NORTH,EAST,WEST,SOUTH};

					InOutPortMap[TILE] = {R0,R1,R2,R3,NORTH,EAST,WEST,SOUTH};
					break;
				case 8 :
					InOutPortMap[R0] = {R0,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R1] = {R1,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R2] = {R2,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R3] = {R3,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R4] = {R4,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R5] = {R5,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R6] = {R6,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R7] = {R7,NORTH,EAST,WEST,SOUTH};

					InOutPortMap[NORTH] = {R0,R1,R2,R3,R4,R5,R6,R7,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[EAST] = {R0,R1,R2,R3,R4,R5,R6,R7,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[WEST] = {R0,R1,R2,R3,R4,R5,R6,R7,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[SOUTH] = {R0,R1,R2,R3,R4,R5,R6,R7,NORTH,EAST,WEST,SOUTH};

					InOutPortMap[TILE] = {R0,R1,R2,R3,R4,R5,R6,R7,NORTH,EAST,WEST,SOUTH};
					break;
				default :
					errs() << "Regs Per Node can only be 2,4,8\n";
					assert(false);
			}
			break;
		case RegXbar:
			switch(regsPerNode){
				case 2 :
					InOutPortMap[R0] = {R0,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R1] = {R1,NORTH,EAST,WEST,SOUTH};

					InOutPortMap[NORTH] = {R0,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[EAST] = {R0,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[WEST] = {R1,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[SOUTH] = {R1,NORTH,EAST,WEST,SOUTH};

					InOutPortMap[TILE] = {NORTH,EAST,WEST,SOUTH};

					break;
				case 4 :
					InOutPortMap[R0] = {R0,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R1] = {R1,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R2] = {R2,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R3] = {R3,NORTH,EAST,WEST,SOUTH};

					InOutPortMap[NORTH] = {R0,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[EAST] = {R1,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[WEST] = {R2,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[SOUTH] = {R3,NORTH,EAST,WEST,SOUTH};

					InOutPortMap[TILE] = {NORTH,EAST,WEST,SOUTH};
					break;
				case 8 :
					InOutPortMap[R0] = {R0,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R1] = {R1,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R2] = {R2,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R3] = {R3,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R4] = {R4,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R5] = {R5,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R6] = {R6,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R7] = {R7,NORTH,EAST,WEST,SOUTH};

					InOutPortMap[NORTH] = {R0,R4,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[EAST] = {R1,R5,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[WEST] = {R2,R6,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[SOUTH] = {R3,R7,NORTH,EAST,WEST,SOUTH};

					InOutPortMap[TILE] = {NORTH,EAST,WEST,SOUTH};
					break;
				default :
					errs() << "Regs Per Node can only be 2,4,8\n";
					assert(false);
			}


//			InOutPortMap[R0] = {R0,NORTH};
//			InOutPortMap[R1] = {R1,EAST};
//			InOutPortMap[R2] = {R2,WEST};
//			InOutPortMap[R3] = {R3,SOUTH};

			break;

		case LatchXbar:
			switch(regsPerNode){
				case 2 :
					InOutPortMap[R0] = {NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R1] = {NORTH,EAST,WEST,SOUTH};

					InOutPortMap[NORTH] = {R0,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[EAST] = {R0,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[WEST] = {R1,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[SOUTH] = {R1,NORTH,EAST,WEST,SOUTH};

					InOutPortMap[TILE] = {NORTH,EAST,WEST,SOUTH};
					break;
				case 4 :
					InOutPortMap[R0] = {NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R1] = {NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R2] = {NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R3] = {NORTH,EAST,WEST,SOUTH};

					InOutPortMap[NORTH] = {R0,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[EAST] = {R1,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[WEST] = {R2,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[SOUTH] = {R3,NORTH,EAST,WEST,SOUTH};

					InOutPortMap[TILE] = {NORTH,EAST,WEST,SOUTH};
					break;
				case 8 :
					InOutPortMap[R0] = {NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R1] = {NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R2] = {NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R3] = {NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R4] = {NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R5] = {NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R6] = {NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R7] = {NORTH,EAST,WEST,SOUTH};

					InOutPortMap[NORTH] = {R0,R4,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[EAST] = {R1,R5,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[WEST] = {R2,R6,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[SOUTH] = {R3,R7,NORTH,EAST,WEST,SOUTH};

					InOutPortMap[TILE] = {NORTH,EAST,WEST,SOUTH};

					break;
				default :
					errs() << "Regs Per Node can only be 2,4,8\n";
					assert(false);
			}

//			InOutPortMap[R0] = {NORTH};
//			InOutPortMap[R1] = {EAST};
//			InOutPortMap[R2] = {WEST};
//			InOutPortMap[R3] = {SOUTH};
			break;

			case StdNOC:
				switch(regsPerNode){
					case 4 :
						InOutPortMap[R0] = {NORTH,EAST,WEST,SOUTH};
						InOutPortMap[R1] = {NORTH,EAST,WEST,SOUTH};
						InOutPortMap[R2] = {NORTH,EAST,WEST,SOUTH};
						InOutPortMap[R3] = {NORTH,EAST,WEST,SOUTH};

						InOutPortMap[NORTH] = {R0};
						InOutPortMap[EAST] = {R1};
						InOutPortMap[WEST] = {R2};
						InOutPortMap[SOUTH] = {R3};

						InOutPortMap[TILE] = {NORTH,EAST,WEST,SOUTH};
						break;
					default :
						errs() << "Regs Per Node can only be 4 for StdNOC\n";
						assert(false);
				}

	//			InOutPortMap[R0] = {NORTH};
	//			InOutPortMap[R1] = {EAST};
	//			InOutPortMap[R2] = {WEST};
	//			InOutPortMap[R3] = {SOUTH};
				break;


		case RegXbarTREG:
			switch(regsPerNode){
				case 2 :
					InOutPortMap[R0] = {R0,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R1] = {R1,NORTH,EAST,WEST,SOUTH};

					InOutPortMap[NORTH] = {R0,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[EAST] = {R0,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[WEST] = {R1,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[SOUTH] = {R1,NORTH,EAST,WEST,SOUTH};

					InOutPortMap[TILE] = {NORTH,EAST,WEST,SOUTH,TREG};
					InOutPortMap[TREG] = {NORTH,EAST,WEST,SOUTH,TREG};

					break;
				case 4 :
					InOutPortMap[R0] = {R0,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R1] = {R1,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R2] = {R2,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R3] = {R3,NORTH,EAST,WEST,SOUTH};

					InOutPortMap[NORTH] = {R0,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[EAST] = {R1,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[WEST] = {R2,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[SOUTH] = {R3,NORTH,EAST,WEST,SOUTH};

					InOutPortMap[TILE] = {NORTH,EAST,WEST,SOUTH,TREG};
					InOutPortMap[TREG] = {NORTH,EAST,WEST,SOUTH,TREG};
					break;
				case 8 :
					InOutPortMap[R0] = {R0,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R1] = {R1,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R2] = {R2,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R3] = {R3,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R4] = {R4,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R5] = {R5,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R6] = {R6,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[R7] = {R7,NORTH,EAST,WEST,SOUTH};

					InOutPortMap[NORTH] = {R0,R4,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[EAST] = {R1,R5,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[WEST] = {R2,R6,NORTH,EAST,WEST,SOUTH};
					InOutPortMap[SOUTH] = {R3,R7,NORTH,EAST,WEST,SOUTH};

					InOutPortMap[TILE] = {NORTH,EAST,WEST,SOUTH,TREG};
					InOutPortMap[TREG] = {NORTH,EAST,WEST,SOUTH,TREG};
					break;
				default :
					errs() << "Regs Per Node can only be 2,4,8\n";
					assert(false);
			}
			break;
	}

	OrigInOutPortMap = InOutPortMap;

	for (int t = 0; t < MII; ++t) {
		std::vector<std::vector<CGRANode*> > tempL2;
		for (int y = 0; y < Ydim; ++y) {
			std::vector<CGRANode*> tempL1;
			for (int x = 0; x < Xdim; ++x) {
				tempNodePtr = new CGRANode(x,y,t,this);
				tempNodePtr->InOutPortMap = InOutPortMap;

				if(x == 0){
					tempNodePtr->setPEType(MEM);
				}

				tempL1.push_back(tempNodePtr);
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

	return CGRANodes[t][y][x];
}

CGRANode* CGRA::getCGRANode(int phyLoc) {
	assert(phyLoc < getMII()*getYdim()*getXdim());

	int t = phyLoc/(getYdim()*getXdim());
	int y = (phyLoc % getMII())/(getXdim());
	int x = ((phyLoc % getMII()) % getYdim());

	return CGRANodes[t][y][x];
}

void CGRA::connectNeighborsMESH() {
	for (int t = 0; t < MII; ++t) {
		for (int y = 0; y < YDim; ++y) {
			for (int x = 0; x < XDim; ++x) {

				for (int yy = 0; yy < YDim; ++yy) {
					for (int xx = 0; xx < XDim; ++xx) {
						CGRANodes[t][y][x]->addConnectedNode(CGRANodes[(t+1)%MII][yy][xx],abs(yy-y) + abs(xx-x) + 1,"mesh");
					}
				}

				for (int i = t+1; i < t+MII; ++i) {
					CGRANodes[t][y][x]->addConnectedNode(CGRANodes[(i)%MII][y][x],i-(t+1),"REGconnections");
				}

				CGRANodes[t][y][x]->sortConnectedNodes();

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

//				for (int reg = 0; reg < regsPerNode; ++reg) {
//					CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[(t+1)%MII][y][x]);
//					CGRANodes[t][y][x].originalEdgesSize++;
//				}

				assert((regsPerNode == 4) || (regsPerNode == 2) || (regsPerNode == 8));
				CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],R0,CGRANodes[(t+1)%MII][y][x],R0));
				CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],R1,CGRANodes[(t+1)%MII][y][x],R1));
				CGRANodes[t][y][x]->originalEdgesSize += 2;
				if(regsPerNode > 2){
					CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],R2,CGRANodes[(t+1)%MII][y][x],R2));
					CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],R3,CGRANodes[(t+1)%MII][y][x],R3));
					CGRANodes[t][y][x]->originalEdgesSize += 2;
					if(regsPerNode > 4){
						CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],R4,CGRANodes[(t+1)%MII][y][x],R4));
						CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],R5,CGRANodes[(t+1)%MII][y][x],R5));
						CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],R6,CGRANodes[(t+1)%MII][y][x],R6));
						CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],R7,CGRANodes[(t+1)%MII][y][x],R7));
						CGRANodes[t][y][x]->originalEdgesSize += 4;
					}
				}
				if(arch == RegXbarTREG){
					CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],TREG,CGRANodes[(t+1)%MII][y][x],TREG));
					CGRANodes[t][y][x]->originalEdgesSize += 1;
				}
//				}


						if(x > 0){
//							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[t][y][x-1]);
							CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],EAST,CGRANodes[t][y][x-1],WEST));
							CGRANodes[t][y][x]->originalEdgesSize++;
						}

						if(x < XDim - 1){
//							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[t][y][x+1]);
							CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],WEST,CGRANodes[t][y][x+1],EAST));
							CGRANodes[t][y][x]->originalEdgesSize++;
						}

						if(y > 0){
//							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[t][y-1][x]);
							CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],NORTH,CGRANodes[t][y-1][x],SOUTH));
							CGRANodes[t][y][x]->originalEdgesSize++;
						}

						if(y < YDim - 1){
//							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[t][y+1][x]);
							CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],SOUTH,CGRANodes[t][y+1][x],NORTH));
							CGRANodes[t][y][x]->originalEdgesSize++;
						}
			}
		}
	}

}

int CGRA::getMII() {
	return MII;
}



int CGRA::getConMatIdx(int t, int y, int x) {
	return t*YDim*XDim + y*XDim + x;
}

void CGRA::connectNeighborsGRID() {

	//Not changed for port register mapping
	assert(false);

	for (int t = 0; t < MII; ++t) {
		for (int y = 0; y < YDim; ++y) {
			for (int x = 0; x < XDim; ++x) {

//				for (int reg = 0; reg < regsPerNode; ++reg) {
//					CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[(t+1)%MII][y][x]);
//					CGRANodes[t][y][x].originalEdgesSize++;
//				}

				assert(regsPerNode == 4);
				CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],R0,CGRANodes[(t+1)%MII][y][x],R0));
				CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],R1,CGRANodes[(t+1)%MII][y][x],R1));
				CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],R2,CGRANodes[(t+1)%MII][y][x],R2));
				CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],R3,CGRANodes[(t+1)%MII][y][x],R3));
				if(arch == RegXbarTREG){
					CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],TREG,CGRANodes[(t+1)%MII][y][x],TREG));
				}

				CGRANodes[t][y][x]->originalEdgesSize += 5;


//				for (int yy = 0; yy < YDim; ++yy) {
//					for (int xx = 0; xx < XDim; ++xx) {

						if(x > 0){
//							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[(t+1)%MII][y][x-1]);
							CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],EAST,CGRANodes[(t+1)%MII][y][x-1],WEST));
							CGRANodes[t][y][x]->originalEdgesSize++;
						}

						if(x < XDim - 1){
//							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[(t+1)%MII][y][x+1]);
							CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],WEST,CGRANodes[(t+1)%MII][y][x+1],EAST));
							CGRANodes[t][y][x]->originalEdgesSize++;
						}

						if(y > 0){
//							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[(t+1)%MII][y-1][x]);
							CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],NORTH,CGRANodes[(t+1)%MII][y-1][x],SOUTH));
							CGRANodes[t][y][x]->originalEdgesSize++;
						}

						if(y < YDim - 1){
//							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[(t+1)%MII][y+1][x]);
							CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],SOUTH,CGRANodes[(t+1)%MII][y+1][x],NORTH));
							CGRANodes[t][y][x]->originalEdgesSize++;
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

std::vector<CGRAEdge*> CGRA::findCGRAEdges(CGRANode* currCNode, Port inPort,std::map<CGRANode*,std::vector<CGRAEdge>>* cgraEdgesPtr) {
	std::vector<CGRAEdge*> candidateCGRAEdges;
	std::vector<Port> candPorts;
	Port currPort;

//		errs() << "findCGRAEdges started.\n";

		if(cgraEdgesPtr->find(currCNode) == cgraEdgesPtr->end()){
			errs() << "findCGRAEdges:: currCnode is" << currCNode->getName() << ", ptr=" << currCNode << "\n";
			errs() << "currCnode in the nodelist=" << getCGRANode(currCNode->getT(),currCNode->getY(),currCNode->getX())<< "\n";
			std::map<CGRANode*,std::vector<CGRAEdge>>::iterator cgraEdgeIt;



			errs() << "cgraEdge node list ::\n";
			for(cgraEdgeIt = cgraEdgesPtr->begin();
				cgraEdgeIt != cgraEdgesPtr->end();
				cgraEdgeIt++){
				errs() << cgraEdgeIt->first->getName() << ", ptr=" << cgraEdgeIt->first << "\n";
			}
			errs() << "cgraEdge node list end.!\n";

			errs() << "Original cgraEdge node list ::\n";
			for(cgraEdgeIt = getCGRAEdges()->begin();
				cgraEdgeIt != getCGRAEdges()->end();
				cgraEdgeIt++){
				errs() << cgraEdgeIt->first->getName() << ", ptr=" << cgraEdgeIt->first << "\n";
			}
			errs() << "Original cgraEdge node list end.!\n";

		}

		assert(cgraEdgesPtr->find(currCNode) != cgraEdgesPtr->end());
		assert((*cgraEdgesPtr)[currCNode].size() != 0);

		candPorts = currCNode->InOutPortMap[inPort];

//		errs() << "findCGRAEdges:: (*cgraEdgesPtr)[currCNode].size() = " << (*cgraEdgesPtr)[currCNode].size() << "\n";
		for (int j = 0; j < (*cgraEdgesPtr)[currCNode].size(); ++j) {
//			errs() << "findCGRAEdges:: j=" << j << "\n";
			if((*cgraEdgesPtr)[currCNode][j].mappedDFGEdge == NULL){
				currPort = (*cgraEdgesPtr)[currCNode][j].SrcPort;
				for (int i = 0; i < candPorts.size(); ++i) {
					if(currPort == candPorts[i]){
						if((*cgraEdgesPtr)[currCNode][j].Dst->getT() >= MII){
							errs() << "MII=" << MII << "\n";
						}
						assert((*cgraEdgesPtr)[currCNode][j].Dst->getT() < MII);
						candidateCGRAEdges.push_back(&(*cgraEdgesPtr)[currCNode][j]);
					}
				}
			}
		}


//		errs() << "findCGRAEdges ended.\n";
	return candidateCGRAEdges;
}

std::string CGRA::getPortName(Port p) {
	switch (p) {
		case NORTH:
			return "NORTH";
			break;
		case EAST:
			return "EAST";
			break;
		case WEST:
			return "WEST";
			break;
		case SOUTH:
			return "SOUTH";
			break;
		case R0:
			return "R0";
			break;
		case R1:
			return "R1";
			break;
		case R2:
			return "R2";
			break;
		case R3:
			return "R3";
			break;
		case TREG:
			return "TREG";
			break;
		case OP1:
			return "OP1";
			break;
		case OP2:
			return "OP2";
			break;
		case PRED:
			return "PRED";
			break;
		case TILE:
			return "TILE";
			break;
		default:
			return "INV";
			break;
	}
}

int CGRA::getTotalUnUsedMemPEs() {
	int unusedMEMPEs = 0;

	for (int t = 0; t < MII; ++t) {
		for (int y = 0; y < YDim; ++y) {
			for (int x = 0; x < XDim; ++x) {
				if(CGRANodes[t][y][x]->getmappedDFGNode() == NULL){
					if(CGRANodes[t][y][x]->getPEType() == MEM){
						unusedMEMPEs++;
					}
				}
			}
		}
	}

	return unusedMEMPEs;
}

std::vector<CGRAEdge> CGRA::getCGRAEdgesWithDest(CGRANode* Cdst) {
	std::map<CGRANode*,std::vector<CGRAEdge> >::iterator totalCGRAEdgeMapIt;

	std::vector<CGRAEdge> tempCGRAEdges;
	std::vector<CGRAEdge> retCGRAEdges;

	for (totalCGRAEdgeMapIt = getCGRAEdges()->begin();
	     totalCGRAEdgeMapIt != getCGRAEdges()->end();
	      ++totalCGRAEdgeMapIt) {

		tempCGRAEdges = totalCGRAEdgeMapIt->second;

		for (int i = 0; i < tempCGRAEdges.size(); ++i) {
			if(tempCGRAEdges[i].mappedDFGEdge != NULL){
				if(tempCGRAEdges[i].Dst == Cdst){
					retCGRAEdges.push_back(tempCGRAEdges[i]);
				}
			}
		}
	}

	return retCGRAEdges;
}
