#include <morpherdfggen/arch/CGRA.h>

#include <morpherdfggen/common/dfg.h>

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

	this->busyPorts.clear();

	freeBlocks.push_back(freeBlock(0,0,0,Xdim,Ydim,MII));


//	InOutPortMap[NORTH] = {R0,R1,R2,R3,NORTH,EAST,WEST,SOUTH};
//	InOutPortMap[EAST] = {R0,R1,R2,R3,NORTH,EAST,WEST,SOUTH};
//	InOutPortMap[WEST] = {R0,R1,R2,R3,NORTH,EAST,WEST,SOUTH};
//	InOutPortMap[SOUTH] = {R0,R1,R2,R3,NORTH,EAST,WEST,SOUTH};

//	InOutPortMap[R0] = {R0,NORTH,EAST,WEST,SOUTH};
//	InOutPortMap[R1] = {R1,NORTH,EAST,WEST,SOUTH};
//	InOutPortMap[R2] = {R2,NORTH,EAST,WEST,SOUTH};
//	InOutPortMap[R3] = {R3,NORTH,EAST,WEST,SOUTH};
	switch (arch) {
		case ALL2ALL:
			switch(regsPerNode){
				case 2 :
					InOutPortMap[R0] = {R0,TILEOUT};
					InOutPortMap[R1] = {R1,TILEOUT};

					InOutPortMap[TILEIN] = {R0,R1,TILEOUT};
					InOutPortMap[TILE] = {R0,R1,TILEOUT};
					break;
				case 4 :
					InOutPortMap[R0] = {R0,TILEOUT};
					InOutPortMap[R1] = {R1,TILEOUT};
					InOutPortMap[R2] = {R2,TILEOUT};
					InOutPortMap[R3] = {R3,TILEOUT};

					InOutPortMap[TILEIN] = {R0,R1,R2,R3,TILEOUT};
					InOutPortMap[TILE] = {R0,R1,R2,R3,TILEOUT};
					break;
				case 8 :
					InOutPortMap[R0] = {R0,TILEOUT};
					InOutPortMap[R1] = {R1,TILEOUT};
					InOutPortMap[R2] = {R2,TILEOUT};
					InOutPortMap[R3] = {R3,TILEOUT};
					InOutPortMap[R4] = {R4,TILEOUT};
					InOutPortMap[R5] = {R5,TILEOUT};
					InOutPortMap[R6] = {R6,TILEOUT};
					InOutPortMap[R7] = {R7,TILEOUT};

					InOutPortMap[TILEIN] = {R0,R1,R2,R3,R4,R5,R6,R7,TILEOUT};
					InOutPortMap[TILE] = {R0,R1,R2,R3,R4,R5,R6,R7,TILEOUT};
					break;
				default :
					errs() << "Regs Per Node can only be 2,4,8\n";
					assert(false);
			}
			break;
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

			case NoNOC:
				assert(regsPerNode == 4);
				InOutPortMap[TILE] = {NORTH,EAST,WEST,SOUTH,R0,R1,R2,R3};
				InOutPortMap[NORTH] = {NORTH,EAST,WEST,SOUTH,R0,R1,R2,R3};
				InOutPortMap[EAST] = {NORTH,EAST,WEST,SOUTH,R0,R1,R2,R3};
				InOutPortMap[SOUTH] = {NORTH,EAST,WEST,SOUTH,R0,R1,R2,R3};
				InOutPortMap[WEST] = {NORTH,EAST,WEST,SOUTH,R0,R1,R2,R3};

				InOutPortMap[R0] = {NORTH,EAST,WEST,SOUTH,R0};
				InOutPortMap[R1] = {NORTH,EAST,WEST,SOUTH,R1};
				InOutPortMap[R2] = {NORTH,EAST,WEST,SOUTH,R2};
				InOutPortMap[R3] = {NORTH,EAST,WEST,SOUTH,R3};
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

//				if((x==0)&&(y==0)){
//					tempNodePtr->setPEType(MEM);
//				}
//				else if((x==0)&&(y==Ydim-1)){
//					tempNodePtr->setPEType(MEM);
//				}
//				else if((x==Xdim-1)&&(y==0)){
//					tempNodePtr->setPEType(MEM);
//				}
//				else if((x==Xdim-1)&&(y==Ydim-1)){
//					tempNodePtr->setPEType(MEM);
//				}


				tempL1.push_back(tempNodePtr);
			}
			tempL2.push_back(tempL1);
		}
		CGRANodes.push_back(tempL2);
	}



//	connectNeighbors();
//	connectNeighborsMESH();

	switch (arch) {
		case NoNOC:
			connectNeighborsGRID();
			break;
		case ALL2ALL:
			connectNeighborsALL();
			break;
		default:
			connectNeighborsSMART();
			break;
	}
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
							CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],WEST,CGRANodes[t][y][x-1],EAST));
							CGRANodes[t][y][x]->originalEdgesSize++;
						}

						if(x < XDim - 1){
//							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[t][y][x+1]);
							CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],EAST,CGRANodes[t][y][x+1],WEST));
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
//	assert(false);
	assert(arch == NoNOC);



	for (int t = 0; t < MII; ++t) {
		for (int y = 0; y < YDim; ++y) {
			for (int x = 0; x < XDim; ++x) {

						assert(regsPerNode == 4);
						CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],R0,CGRANodes[(t+1)%MII][y][x],R0));
						CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],R1,CGRANodes[(t+1)%MII][y][x],R1));
						CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],R2,CGRANodes[(t+1)%MII][y][x],R2));
						CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],R3,CGRANodes[(t+1)%MII][y][x],R3));
						CGRANodes[t][y][x]->originalEdgesSize += 4;

						if(x > 0){
//							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[(t+1)%MII][y][x-1]);
							CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],WEST,CGRANodes[(t+1)%MII][y][x-1],EAST));
							CGRANodes[t][y][x]->originalEdgesSize++;
						}

						if(x < XDim - 1){
//							CGRAEdges[&CGRANodes[t][y][x]].push_back(&CGRANodes[(t+1)%MII][y][x+1]);
							CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],EAST,CGRANodes[(t+1)%MII][y][x+1],WEST));
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

			}
		}
	}
}

void CGRA::clearMapping() {

}

std::vector<CGRAEdge*> CGRA::findCGRAEdges(CGRANode* currCNode,
		                                   Port inPort,
										   std::map<CGRANode*,std::vector<CGRAEdge>>* cgraEdgesPtr,
										   CGRANode* goal) {
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

//		std::set<Port> busyPorts;

		for (int j = 0; j < (*cgraEdgesPtr)[currCNode].size(); ++j) {
			currPort = (*cgraEdgesPtr)[currCNode][j].SrcPort;
			CGRANode* dest = (*cgraEdgesPtr)[currCNode][j].Dst;
			Port destPort = (*cgraEdgesPtr)[currCNode][j].DstPort;

			if((*cgraEdgesPtr)[currCNode][j].mappedDFGEdge != NULL){
				if((dest == goal && goal != NULL)){
					errs() << "findCGRAEdges :: pushing busy port=(" << dest->getName() << "," << getPortName(destPort)<<")\n";
					busyPorts[dest].insert(destPort);
				}
			}
		}

		for (int j = 0; j < (*cgraEdgesPtr)[currCNode].size(); ++j) {
//			errs() << "findCGRAEdges:: j=" << j << "\n";
			currPort = (*cgraEdgesPtr)[currCNode][j].SrcPort;
			CGRANode* dest = (*cgraEdgesPtr)[currCNode][j].Dst;
			Port destPort = (*cgraEdgesPtr)[currCNode][j].DstPort;

			if( (this->getArch() == RegXbarTREG) || (this->getArch() == RegXbar) ){
//				if ((goal != NULL)&&(dest == goal)){
					if(destPort==R0){
						if( busyPorts[dest].find(NORTH)!=busyPorts[dest].end() ){
							continue;
						}
					}
					else if(destPort==R1){
						if( busyPorts[dest].find(EAST)!=busyPorts[dest].end() ){
							continue;
						}
					}
					else if(destPort==R2){
						if( busyPorts[dest].find(WEST)!=busyPorts[dest].end() ){
							continue;
						}
					}
					else if(destPort==R3){
						if( busyPorts[dest].find(SOUTH)!=busyPorts[dest].end() ){
							continue;
						}
					}
					else if(destPort==NORTH){
						if( busyPorts[dest].find(R0)!=busyPorts[dest].end() ){
							continue;
						}
					}
					else if(destPort==EAST){
						if( busyPorts[dest].find(R1)!=busyPorts[dest].end() ){
							continue;
						}
					}
					else if(destPort==WEST){
						if( busyPorts[dest].find(R2)!=busyPorts[dest].end() ){
							continue;
						}
					}
					else if(destPort==SOUTH){
						if( busyPorts[dest].find(R3)!=busyPorts[dest].end() ){
							continue;
						}
					}
//				}

			}

			if((*cgraEdgesPtr)[currCNode][j].mappedDFGEdge == NULL){


				for (int i = 0; i < candPorts.size(); ++i) {
					if(currPort == candPorts[i]){

						//TODO : DAC18
//						if(currPort==R0) assert(inPort == R0 || inPort == NORTH);
//						if(currPort==R1) assert(inPort == R1 || inPort == EAST);
//						if(currPort==R2) assert(inPort == R2 || inPort == WEST);
//						if(currPort==R3) assert(inPort == R3 || inPort == SOUTH);


						if((*cgraEdgesPtr)[currCNode][j].Dst->getT() >= MII){
							errs() << "MII=" << MII << "\n";
						}
						assert((*cgraEdgesPtr)[currCNode][j].Dst->getT() < MII);
//						if(!busyPorts[dest].empty()) printCGRAEdge((*cgraEdgesPtr)[currCNode][j]);
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
		case TILEIN:
			return "TILEIN";
			break;
		case TILEOUT:
			return "TILEOUT";
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

std::vector<CGRAEdge*> CGRA::getCGRAEdgesWithDest(CGRANode* Cdst,
		std::map<CGRANode*, std::vector<CGRAEdge> >* cgraEdgesPtr) {

	std::map<CGRANode*,std::vector<CGRAEdge> >::iterator totalCGRAEdgeMapIt;

	std::vector<CGRAEdge>* tempCGRAEdges;
	std::vector<CGRAEdge*> retCGRAEdges;

	for (totalCGRAEdgeMapIt = cgraEdgesPtr->begin();
	     totalCGRAEdgeMapIt != cgraEdgesPtr->end();
	      ++totalCGRAEdgeMapIt) {

		tempCGRAEdges = &(*cgraEdgesPtr)[totalCGRAEdgeMapIt->first];

		for (int i = 0; i < tempCGRAEdges->size(); ++i) {
			if((*tempCGRAEdges)[i].mappedDFGEdge == NULL){
				if((*tempCGRAEdges)[i].Dst == Cdst){
					retCGRAEdges.push_back(&(*tempCGRAEdges)[i]);
				}
			}
			else{
//				errs() << "mappedDFGEdge = " << (*tempCGRAEdges)[i].mappedDFGEdge->getName() << "\n";
			}
		}
	}

	return retCGRAEdges;
}

void CGRA::connectNeighborsALL() {

	for (int t = 0; t < MII; ++t) {
			for (int y = 0; y < YDim; ++y) {
				for (int x = 0; x < XDim; ++x) {

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

					for (int yy = 0; yy < YDim; ++yy) {
						for (int xx = 0; xx < XDim; ++xx) {
							CGRAEdges[CGRANodes[t][y][x]].push_back(CGRAEdge(CGRANodes[t][y][x],TILEOUT,CGRANodes[t][yy][xx],TILEIN));
							CGRANodes[t][y][x]->originalEdgesSize++;
						}
					}


				}
			}
		}
}

void CGRA::addIINewNodes() {
	CGRANodeNew* tempNewNode;
//	std::vector<std::vector<std::vector<CGRANodeNew*> > > tempNewIINodes;

	int currEndTime = CGRANodesNew.size();
	assert(currEndTime%MII == 0); //Should be a multiple of MII

	for (int t = 0; t < MII; ++t) {
		std::vector<std::vector<CGRANodeNew*> > tempL2;
		for (int y = 0; y < YDim; ++y) {
			std::vector<CGRANodeNew*> tempL1;
			for (int x = 0; x < XDim; ++x) {
				tempNewNode = new CGRANodeNew(CGRANodes[t][y][x],currEndTime+t);
				tempL1.push_back(tempNewNode);
			}
			tempL2.push_back(tempL1);
		}
		CGRANodesNew.push_back(tempL2);
	}

	CGRANode* cnode;
	CGRAEdgeNew CENew;

	for (int t = currEndTime; t < currEndTime+MII; ++t) {
		for (int y = 0; y < YDim; ++y) {
			for (int x = 0; x < XDim; ++x) {
				cnode = CGRANodesNew[t][y][x]->oldCnode;
				for (int i = 0; i < CGRAEdges[cnode].size(); ++i) {
					assert(CGRAEdges[cnode][i].Src == cnode);

					//Check for edges connecting in time dimension
					if((CGRAEdges[cnode][i].DstPort != NORTH) &&
					   (CGRAEdges[cnode][i].DstPort != EAST) &&
					   (CGRAEdges[cnode][i].DstPort != WEST) &&
					   (CGRAEdges[cnode][i].DstPort != SOUTH)){

						//Check for wrap around edges
						if(CGRAEdges[cnode][i].Dst->getT() <= cnode->getT()){
							assert(t%MII == MII-1);
						}
						else{ //Non wrap around edges
							CENew  = CGRAEdgeNew(CGRAEdges[cnode][i],
												 CGRANodesNew[CGRAEdges[cnode][i].Src->getT()]
															 [CGRAEdges[cnode][i].Src->getY()]
															 [CGRAEdges[cnode][i].Src->getX()],
									             CGRANodesNew[CGRAEdges[cnode][i].Dst->getT()]
															 [CGRAEdges[cnode][i].Dst->getY()]
															 [CGRAEdges[cnode][i].Dst->getX()]
												 );
						}
					}
					else{ // These edges connecting to NEWS
						CENew  = CGRAEdgeNew(CGRAEdges[cnode][i],
											 CGRANodesNew[CGRAEdges[cnode][i].Src->getT()]
														 [CGRAEdges[cnode][i].Src->getY()]
														 [CGRAEdges[cnode][i].Src->getX()],
								             CGRANodesNew[CGRAEdges[cnode][i].Dst->getT()]
														 [CGRAEdges[cnode][i].Dst->getY()]
														 [CGRAEdges[cnode][i].Dst->getX()]
											 );
					}

					CGRAEdgesNew[CGRANodesNew[t][y][x]].push_back(CENew);
				}
			}
		}
	}

	//Connect the last time to new II nodes
	for (int y = 0; y < YDim; ++y) {
		for (int x = 0; x < XDim; ++x) {
			cnode = CGRANodesNew[currEndTime-1][y][x]->oldCnode;
			for (int i = 0; i < CGRAEdges[cnode].size(); ++i) {
				if((CGRAEdges[cnode][i].DstPort != NORTH) &&
				   (CGRAEdges[cnode][i].DstPort != EAST) &&
				   (CGRAEdges[cnode][i].DstPort != WEST) &&
				   (CGRAEdges[cnode][i].DstPort != SOUTH)){

					//Check for wrap around edges
					if(CGRAEdges[cnode][i].Dst->getT() <= cnode->getT()){
						CENew  = CGRAEdgeNew(CGRAEdges[cnode][i],
											 CGRANodesNew[currEndTime-1]
														 [CGRAEdges[cnode][i].Src->getY()]
														 [CGRAEdges[cnode][i].Src->getX()],
								             CGRANodesNew[currEndTime]
														 [CGRAEdges[cnode][i].Dst->getY()]
														 [CGRAEdges[cnode][i].Dst->getX()]
											 );
					}
				}
			}
		}
	}



}

void CGRA::combineCGRAs(CGRA* otherCGRA, int insertX, int insertY,
		int insertT) {

	bool additionisBoundary=(insertX==this->getXdim()-1);
	additionisBoundary = additionisBoundary || (insertY==this->getYdim()-1);
	additionisBoundary = additionisBoundary || (insertT==this->getMII()-1);

	//Add all otherCGRA edges to this one
	std::map<CGRANode*,std::vector<CGRAEdge>>::iterator edgeIt;
	for (edgeIt = otherCGRA->getCGRAEdges()->begin(); edgeIt != otherCGRA->getCGRAEdges()->end(); ++edgeIt) {
		assert(CGRAEdges.find(edgeIt->first)==CGRAEdges.end()); //should not be already included.
		CGRAEdges[edgeIt->first]=edgeIt->second;
	}

	int x;
	int y;
	int t;


	//Connect YT Plane
	x=insertX;
	for (y = insertY; y < insertY+otherCGRA->getYdim(); ++y) {
		for (t = insertT; t < insertT+otherCGRA->getMII(); ++t) {
			if((x<getXdim())&&(y<getYdim())&&(t<getMII())){ //insertion point is inside the current CGRA

				CGRANode* cnode = getCGRANode(t,y,x);
				assert(CGRAEdges.find(cnode) != CGRAEdges.end());

				for (int i = 0; i < CGRAEdges[cnode].size(); ++i) {
					CGRANode* srcNode = CGRAEdges[cnode][i].Src;
					assert(cnode==srcNode);
					CGRANode* destNode = CGRAEdges[cnode][i].Dst;

					if((destNode->getT() >= insertT)&&(destNode->getX() >= insertX)&&(destNode->getY() >= insertY)){
						continue; // the destination node is inside the newly added CGRA fabric.
					}

					for (int j = 0; j < CGRAEdges[destNode].size(); ++j) {
						if(CGRAEdges[destNode][j].Dst == srcNode){
							CGRANode* newNode = otherCGRA->getCGRANode(t-insertT,y-insertY,x-insertX);
							CGRAEdges[destNode][j].Dst = newNode;
							CGRAEdges[newNode].push_back(CGRAEdge(newNode,CGRAEdges[destNode][j].DstPort,destNode,CGRAEdges[destNode][j].SrcPort));

							for (int k = 0; k < CGRAEdges[srcNode].size(); ++k) {
								if(CGRAEdges[srcNode][k].Dst == destNode){
									//end node of x dimension
									CGRANode* endNewNode = otherCGRA->getCGRANode(t-insertT,y-insertY,otherCGRA->getXdim()-1);
									CGRAEdges[srcNode][k].Dst = endNewNode;
									CGRAEdges[endNewNode].push_back(CGRAEdge(endNewNode,CGRAEdges[srcNode][k].DstPort,srcNode,CGRAEdges[srcNode][k].SrcPort));
								}
							}

						}
					}
				}
			}
			else{ //insertion point is outside the current CGRA.
				if(x == getXdim()){// boundrying to x
					assert(y<getYdim());
					assert(t<getMII());
					assert(getXdim() > 1);
					for (int i = 0; i < CGRAEdges[getCGRANode(0,0,1)].size(); ++i) {
						if(CGRAEdges[getCGRANode(0,0,1)][i].Dst == getCGRANode(0,0,0)){
							CGRAEdge cedge = CGRAEdges[getCGRANode(0,0,1)][i];
							CGRAEdges[getCGRANode(t,y,x)].push_back(CGRAEdge(getCGRANode(t,y,x),cedge.SrcPort,getCGRANode(t,y,x-1),cedge.DstPort));
							CGRAEdges[getCGRANode(t,y,x-1)].push_back(CGRAEdge(getCGRANode(t,y,x-1),cedge.DstPort,getCGRANode(t,y,x),cedge.SrcPort));
						}
					}
				}
				else if(y == getYdim()){
					assert(t<getMII());
					assert(x<getXdim());
					assert(getYdim() > 1);
					for (int i = 0; i < CGRAEdges[getCGRANode(0,1,0)].size(); ++i) {
						if(CGRAEdges[getCGRANode(0,1,0)][i].Dst == getCGRANode(0,0,0)){
							CGRAEdge cedge = CGRAEdges[getCGRANode(0,1,0)][i];
							CGRAEdges[getCGRANode(t,y,x)].push_back(CGRAEdge(getCGRANode(t,y,x),cedge.SrcPort,getCGRANode(t,y-1,x),cedge.DstPort));
							CGRAEdges[getCGRANode(t,y-1,x)].push_back(CGRAEdge(getCGRANode(t,y-1,x),cedge.DstPort,getCGRANode(t,y,x),cedge.SrcPort));
						}
					}
				}
				else if(t == getMII()){
					assert(y<getYdim());
					assert(x<getXdim());
					assert(getMII() > 1);
					for (int i = 0; i < CGRAEdges[getCGRANode(1,0,0)].size(); ++i) {
						if(CGRAEdges[getCGRANode(1,0,0)][i].Dst == getCGRANode(0,0,0)){
							CGRAEdge cedge = CGRAEdges[getCGRANode(1,0,0)][i];
							CGRAEdges[getCGRANode(t,y,x)].push_back(CGRAEdge(getCGRANode(t,y,x),cedge.SrcPort,getCGRANode(t,y-1,x),cedge.DstPort));
							CGRAEdges[getCGRANode(t,y-1,x)].push_back(CGRAEdge(getCGRANode(t,y-1,x),cedge.DstPort,getCGRANode(t,y,x),cedge.SrcPort));
						}
					}
				}
			}
		}
	}


	//Connect XT Plane
		y=insertY;
		for (x = insertX; x < insertX+otherCGRA->getXdim(); ++x) {
			for (t = insertT; t < insertT+otherCGRA->getMII(); ++t) {
				if((x<getXdim())&&(y<getYdim())&&(t<getMII())){

					CGRANode* cnode = getCGRANode(t,y,x);
					assert(CGRAEdges.find(cnode) != CGRAEdges.end());

					for (int i = 0; i < CGRAEdges[cnode].size(); ++i) {
						CGRANode* srcNode = CGRAEdges[cnode][i].Src;
						assert(cnode==srcNode);
						CGRANode* destNode = CGRAEdges[cnode][i].Dst;

						if((destNode->getT() >= insertT)&&(destNode->getX() >= insertX)&&(destNode->getY() >= insertY)){
							continue; // the destination node is inside the newly added CGRA fabric.
						}

						for (int j = 0; j < CGRAEdges[destNode].size(); ++j) {
							if(CGRAEdges[destNode][j].Dst == srcNode){
								CGRANode* newNode = otherCGRA->getCGRANode(t-insertT,y-insertY,x-insertX);
								CGRAEdges[destNode][j].Dst = newNode;
								CGRAEdges[newNode].push_back(CGRAEdge(newNode,CGRAEdges[destNode][j].DstPort,destNode,CGRAEdges[destNode][j].SrcPort));

								for (int k = 0; k < CGRAEdges[srcNode].size(); ++k) {
									if(CGRAEdges[srcNode][k].Dst == destNode){
										//end node of x dimension
										CGRANode* endNewNode = otherCGRA->getCGRANode(t-insertT,otherCGRA->getYdim()-1,x-insertX);
										CGRAEdges[srcNode][k].Dst = endNewNode;
										CGRAEdges[endNewNode].push_back(CGRAEdge(endNewNode,CGRAEdges[srcNode][k].DstPort,srcNode,CGRAEdges[srcNode][k].SrcPort));
									}
								}

							}
						}
					}
				}
			}
		}


}

bool CGRA::PlaceMacro(DFG* mappedDFG) {
	int macroSizeX = mappedDFG->getCGRA()->getXdim();
	int macroSizeY = mappedDFG->getCGRA()->getYdim();
	int macroSizeT = mappedDFG->getCGRA()->getMII();
	bool found=false;

	freeBlock currFreeBlock(-1,-1,-1,-1,-1,-1);
	CGRA* otherCGRA = mappedDFG->getCGRA();
	std::map<CGRANode*,std::vector<CGRAEdge>>* otherCGRAEdges = otherCGRA->getCGRAEdges();

	std::sort(freeBlocks.begin(),freeBlocks.end(), freeBlock::compare);
	for (int i = 0; i < freeBlocks.size(); ++i) {
		if(freeBlocks[i].fitsSize(macroSizeX,macroSizeY,macroSizeT)){
			found=true;

			if(macroSizeT < freeBlocks[i].sizeT){
				freeBlock newFreeBlock(freeBlocks[i].posX,freeBlocks[i].posY,freeBlocks[i].posT+macroSizeT,freeBlocks[i].sizeX,freeBlocks[i].sizeY,freeBlocks[i].sizeT-macroSizeT);
				freeBlocks.push_back(newFreeBlock);
			}

			int diffY = freeBlocks[i].sizeY - macroSizeY;
			int diffX = freeBlocks[i].sizeX - macroSizeX;

			if(diffY > diffX){
				freeBlock newFreeBlock(freeBlocks[i].posX,freeBlocks[i].posY+macroSizeY,freeBlocks[i].posT,freeBlocks[i].sizeX,diffY,freeBlocks[i].sizeT);
				freeBlocks.push_back(newFreeBlock);

				freeBlock newFreeBlock2(freeBlocks[i].posX+macroSizeX,freeBlocks[i].posY,freeBlocks[i].posT,diffX,diffY,freeBlocks[i].sizeT);
				freeBlocks.push_back(newFreeBlock2);
			}
			else{
				freeBlock newFreeBlock(freeBlocks[i].posX+macroSizeX,freeBlocks[i].posY,freeBlocks[i].posT,diffX,freeBlocks[i].sizeY,freeBlocks[i].sizeT);
				freeBlocks.push_back(newFreeBlock);

				freeBlock newFreeBlock2(freeBlocks[i].posX,freeBlocks[i].posY+macroSizeY,freeBlocks[i].posT,diffX,diffY,freeBlocks[i].sizeT);
				freeBlocks.push_back(newFreeBlock2);
			}
			//erase the currently consumed block
			currFreeBlock = freeBlocks[i];
			currFreeBlock.sizeX = macroSizeX;
			currFreeBlock.sizeY = macroSizeY;
			currFreeBlock.sizeT = macroSizeT;
			freeBlocks.erase(freeBlocks.begin()+i);
			break;
		}
	}

	if(!found){
		return false;
	}

	//Copy the node and edge information
	for (int t = 0; t < macroSizeT; ++t) {
		for (int y = 0; y < macroSizeY; ++y) {
			for (int x = 0; x < macroSizeX; ++x) {

				CGRANode* currCnode = getCGRANode(currFreeBlock.posT+t,currFreeBlock.posY+y,currFreeBlock.posX+x);
				CGRANode* macroCnode = mappedDFG->getCGRA()->getCGRANode(t,y,x);

				//copy the node
				currCnode->setMappedDFGNode(macroCnode->getmappedDFGNode());

				//copy the edges
				for (int i = 0; i < (*otherCGRAEdges)[macroCnode].size(); ++i) {
					for (int j = 0; j < CGRAEdges[currCnode].size(); ++j) {
						if(CGRAEdges[currCnode][j].SrcPort == (*otherCGRAEdges)[macroCnode][j].SrcPort){
							assert(CGRAEdges[currCnode][j].DstPort == (*otherCGRAEdges)[macroCnode][j].DstPort);
							CGRAEdges[currCnode][j].mappedDFGEdge=(*otherCGRAEdges)[macroCnode][j].mappedDFGEdge;
							break;
						}
					}
				}
			}
		}
	}

	return true;
}

void CGRA::printCGRAEdge(CGRAEdge ce) {

	errs() << "CGRAEdge::" << "(" << ce.Src->getName() << "," << getPortName(ce.SrcPort) << ")";
	errs() << " to " << ce.Dst->getName() << "," << getPortName(ce.DstPort) << "),";

	if(ce.mappedDFGEdge == NULL){
		errs() << "mapped to NULL\n";
	}
	else{
		assert(ce.mappedDFGEdge->getSrc());
		assert(ce.mappedDFGEdge->getDest());
		errs() << "mapped to " << ce.mappedDFGEdge->getSrc()->getIdx() << "_to_" << ce.mappedDFGEdge->getDest()->getIdx() << "\n";
	}

}
