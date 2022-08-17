#include "dfg.h"
#include "llvm/Analysis/CFG.h"
#include "CGRA.h"
#include <algorithm>
#include <queue>
#include "astar.h"
#include <ctime>
#include "tinyxml2.h"
#include <functional>
#include <bitset>
#include <set>

#define LV_NAME "dfg_gen" //"sfp"
#define DEBUG_TYPE LV_NAME

dfgNode *DFG::getEntryNode()
{
	if (NodeList.size() > 0)
	{
		return NodeList[0];
	}
	return NULL;
}

std::vector<dfgNode *> DFG::getNodes()
{
	return NodeList;
}

std::vector<Edge> DFG::getEdges()
{
	return edgeList;
}

void DFG::InsertNode(Instruction *Node)
{
	dfgNode *temp = new dfgNode(Node, this);
	temp->setIdx(NodeList.size());
	NodeList.push_back(temp);
}

//void DFG::InsertNode(dfgNode Node){
//	LLVM_DEBUG(dbgs() << "Inserted Node with Instruction : " << Node.getNode() << "\n";
//	Node.setIdx(NodeList.size());
//	NodeList.push_back(Node);
//}

void DFG::InsertEdge(Edge e)
{
	edgeList.push_back(e);
}

dfgNode *DFG::findNode(Instruction *I)
{
	for (int i = 0; i < NodeList.size(); i++)
	{
		if (I == NodeList[i]->getNode())
		{
			return NodeList[i];
		}
	}

	for (std::pair<Value *, dfgNode *> pair : OutLoopNodeMap)
	{
		if (pair.first == I)
		{
			return pair.second;
		}
	}

	return NULL;
}

Edge *DFG::findEdge(dfgNode *src, dfgNode *dest, CGRANode *goal)
{

	// 	if(src == dest){
	// //		assert(src->getPHIchildren().size()==1);
	// 		assert(goal!=NULL);
	// 		for(dfgNode* phiChild : src->getPHIchildren()){
	// 			if(phiChild->getMappedLoc() == goal){
	// 				dest = phiChild;
	// 			}
	// 		}
	// 		assert(dest!=src);
	// //		dest = src->getPHIchildren()[0];
	// 	}

	for (int i = 0; i < edgeList.size(); ++i)
	{
		if (edgeList[i].getSrc() == src && edgeList[i].getDest() == dest)
		{
			return &(edgeList[i]);
		}
	}

	LLVM_DEBUG(dbgs() << "edge is NULL, src = " << src->getIdx() << ",dest = " << dest->getIdx() << "\n");

	assert(false);
	return NULL;
}

std::vector<dfgNode *> DFG::getRoots()
{
	std::vector<dfgNode *> rootNodes;
	for (int i = 0; i < NodeList.size(); i++)
	{
		if (NodeList[i]->getChildren().size() == 0)
		{
			rootNodes.push_back(NodeList[i]);
		}
	}
	return rootNodes;
}

std::vector<dfgNode *> DFG::getLeafs(BasicBlock *BB)
{
	LLVM_DEBUG(dbgs() << "start getting the LeafNodes...!\n");
	std::vector<dfgNode *> leafNodes;
	for (int i = 0; i < NodeList.size(); i++)
	{
		//		if(NodeList[i]->getNode()->getParent() == BB){
		if (NodeList[i]->BB == BB)
		{

			if (NodeList[i]->getNode() != NULL)
			{
				if (dyn_cast<PHINode>(NodeList[i]->getNode()))
				{
					continue;
				}
			}

			if (NodeList[i]->getNameType().compare("OutLoopLOAD") == 0)
			{
			}
			else
			{
				leafNodes.push_back(NodeList[i]);
			}
		}
	}
	LLVM_DEBUG(dbgs() << "LeafNodes init done...!\n");

	for (int i = 0; i < NodeList.size(); i++)
	{
		//		if(NodeList[i]->getNode()->getParent() == BB){
		if (NodeList[i]->BB == BB)
		{

			if (NodeList[i]->getNameType().compare("OutLoopLOAD") == 0)
			{
				continue;
			}

			for (int j = 0; j < NodeList[i]->getChildren().size(); j++)
			{
				dfgNode *nodeToBeRemoved = NodeList[i]->getChildren()[j];
				if (nodeToBeRemoved != NULL)
				{
					//					LLVM_DEBUG(dbgs() << "LeafNodes : nodeToBeRemoved found...! : ";
					if (nodeToBeRemoved->getNode() == NULL)
					{
						//						LLVM_DEBUG(dbgs() << "NodeIdx:" << nodeToBeRemoved->getIdx() << "," << nodeToBeRemoved->getNameType() << "\n";
					}
					else
					{
						//						LLVM_DEBUG(dbgs() << "NodeIdx:" << nodeToBeRemoved->getIdx() << ",";
						LLVM_DEBUG(nodeToBeRemoved->getNode()->dump());
					}

					if (std::find(leafNodes.begin(), leafNodes.end(), nodeToBeRemoved) != leafNodes.end())
					{
						leafNodes.erase(std::remove(leafNodes.begin(), leafNodes.end(), nodeToBeRemoved));
					}
				}
			}
		}
	}
	LLVM_DEBUG(dbgs() << "got the LeafNodes...!\n");
	return leafNodes;
}

void DFG::connectBB()
{
	LLVM_DEBUG(dbgs() << "ConnectBB called!\n");

	std::map<dfgNode *, std::vector<dfgNode *>> BrSuccesors;
	std::map<dfgNode *, std::set<dfgNode *>> BrBackEdgeSuccessors;
	std::map<const BasicBlock *, dfgNode *> BBPredicate;
	dfgNode *temp;
	dfgNode *node;

	assert(NodeList.size() > 0);
	dfgNode firstNode = *NodeList[0];
	SmallVector<std::pair<const BasicBlock *, const BasicBlock *>, 8> Result;
	FindFunctionBackedges(*(firstNode.getNode()->getFunction()), Result);
	LLVM_DEBUG(dbgs() << "Number of Backedges = " << Result.size() << "\n");

	for (int i = 0; i < Result.size(); ++i)
	{
		LLVM_DEBUG(dbgs() << "Backedges .... :: \n");
		LLVM_DEBUG(dbgs() << "From : " << Result[i].first->getName());
		LLVM_DEBUG(dbgs() << ",To : " << Result[i].second->getName());
		LLVM_DEBUG(dbgs() << "\n");
	}

	std::vector<BasicBlock *> analysedBB;
	for (int i = 0; i < NodeList.size(); i++)
	{
		if (NodeList[i]->getNode() != NULL)
		{
			//			if(NodeList[i]->getNode()->getOpcode() == Instruction::Br){
			if (BranchInst *BI = dyn_cast<BranchInst>(NodeList[i]->getNode()))
			{
				LLVM_DEBUG(dbgs() << "$$$$$ This belongs to BB=" << NodeList[i]->getNode()->getParent()->getName() << "\n");

				BasicBlock *BB = NodeList[i]->getNode()->getParent();
				succ_iterator SI(succ_begin(BB)), SE(succ_end(BB));
				for (; SI != SE; ++SI)
				{
					BasicBlock *succ = *SI;
					LLVM_DEBUG(dbgs() << "$%$%$%$%$ successor Basic Blocks\n");
					LLVM_DEBUG(dbgs() << "Name=" << succ->getName() << "\n");
					LLVM_DEBUG(dbgs() << "$%$%$%$%$\n");

					std::vector<dfgNode *> succLeafs = this->getLeafs(succ);
					LLVM_DEBUG(dbgs() << "succLeafs.size = " << succLeafs.size() << "\n");

					std::pair<const BasicBlock *, const BasicBlock *> bbCouple(BB, succ);
					if (std::find(Result.begin(), Result.end(), bbCouple) != Result.end())
					{
						for (int j = 0; j < succLeafs.size(); j++)
						{
							LLVM_DEBUG(dbgs() << "Backedge from : " << NodeList[i]->getIdx() << ",To :" << succLeafs[j]->getIdx() << "\n");
							BrBackEdgeSuccessors[succLeafs[j]].insert(NodeList[i]);
						}
						//						 continue;
					}

					for (int j = 0; j < succLeafs.size(); j++)
					{
						BrSuccesors[succLeafs[j]].push_back(NodeList[i]);
					}
				}
			}
		}
	}

	//Connect the BBs here using the BrSuccesors Map
	std::map<dfgNode *, std::vector<dfgNode *>>::iterator it;
	std::vector<dfgNode *> workingSet;
	std::vector<dfgNode *> nextWorkingSet;
	int numberofbrs = 0;
	for (it = BrSuccesors.begin(); it != BrSuccesors.end(); it++)
	{
		numberofbrs = it->second.size();
		node = it->first;
		workingSet.clear();
		nextWorkingSet.clear();
		LLVM_DEBUG(dbgs() << "ConnectBB :: "
				<< "Init Round\n");

		if (BBPredicate.find(it->first->BB) != BBPredicate.end())
		{
			if (BrBackEdgeSuccessors[node].find(BBPredicate[node->BB]) ==
					BrBackEdgeSuccessors[node].end())
			{
				BBPredicate[node->BB]->addChildNode(node);
				node->addAncestorNode(BBPredicate[node->BB]);
			}
			else
			{
				BBPredicate[node->BB]->addPHIChildNode(node);
				node->addPHIAncestorNode(BBPredicate[node->BB]);
			}
			LLVM_DEBUG(dbgs() << "ConnectBB :: "
					<< "BB already done\n");
			continue;
		}

		if (numberofbrs == 1)
		{

			if (BrBackEdgeSuccessors[node].find(BrSuccesors[node][0]) ==
					BrBackEdgeSuccessors[node].end())
			{
				BrSuccesors[node][0]->addChildNode(node);
				node->addAncestorNode(BrSuccesors[node][0]);
			}
			else
			{
				BrSuccesors[node][0]->addPHIChildNode(node);
				node->addPHIAncestorNode(BrSuccesors[node][0]);
			}

			BBPredicate[node->BB] = BrSuccesors[node][0];
			continue;
		}

		for (int i = 0; i < numberofbrs - 1; i = i + 2)
		{
			temp = new dfgNode(this);
			temp->setNameType("CTRLBrOR");
			temp->setIdx(NodeList.size());
			temp->BB = node->BB;
			NodeList.push_back(temp);

			if (BrBackEdgeSuccessors[node].find(BrSuccesors[node][i]) ==
					BrBackEdgeSuccessors[node].end())
			{
				temp->addAncestorNode(BrSuccesors[node][i]);
				BrSuccesors[node][i]->addChildNode(temp);
			}
			else
			{
				temp->addPHIAncestorNode(BrSuccesors[node][i]);
				BrSuccesors[node][i]->addPHIChildNode(temp);
			}

			if (BrBackEdgeSuccessors[node].find(BrSuccesors[node][i + 1]) ==
					BrBackEdgeSuccessors[node].end())
			{
				temp->addAncestorNode(BrSuccesors[node][i + 1]);
				BrSuccesors[node][i + 1]->addChildNode(temp);
			}
			else
			{
				temp->addPHIAncestorNode(BrSuccesors[node][i + 1]);
				BrSuccesors[node][i + 1]->addPHIChildNode(temp);
			}
			workingSet.push_back(temp);
		}

		if (numberofbrs % 2 == 1)
		{
			workingSet.push_back(BrSuccesors[node][numberofbrs - 1]);
		}
		LLVM_DEBUG(dbgs() << "ConnectBB :: "
				<< "Rest Rounds\n");
		while (workingSet.size() > 1)
		{
			LLVM_DEBUG(dbgs() << "ConnectBB :: "
					<< "workingSet.size() = " << workingSet.size() << "\n");
			for (int i = 0; i < workingSet.size() - 1; i = i + 2)
			{
				temp = new dfgNode(this);
				temp->setIdx(NodeList.size());
				NodeList.push_back(temp);
				temp->setNameType("CTRLBrOR");
				temp->BB = node->BB;
				LLVM_DEBUG(dbgs() << "ConnectBB :: "
						<< "workingSet[i+1] = " << workingSet[i + 1]->getIdx() << "\n");

				if (BrBackEdgeSuccessors[node].find(workingSet[i]) ==
						BrBackEdgeSuccessors[node].end())
				{
					workingSet[i]->addChildNode(temp);
					temp->addAncestorNode(workingSet[i]);
				}
				else
				{
					workingSet[i]->addPHIChildNode(temp);
					temp->addPHIAncestorNode(workingSet[i]);
				}

				if (BrBackEdgeSuccessors[node].find(workingSet[i + 1]) ==
						BrBackEdgeSuccessors[node].end())
				{
					workingSet[i + 1]->addChildNode(temp);
					temp->addAncestorNode(workingSet[i + 1]);
				}
				else
				{
					workingSet[i + 1]->addPHIChildNode(temp);
					temp->addPHIAncestorNode(workingSet[i + 1]);
				}

				nextWorkingSet.push_back(temp);
			}
			if (workingSet.size() % 2 == 1)
			{
				nextWorkingSet.push_back(workingSet[workingSet.size() - 1]);
			}

			workingSet.clear();
			for (int i = 0; i < nextWorkingSet.size(); ++i)
			{
				workingSet.push_back(nextWorkingSet[i]);
			}
			nextWorkingSet.clear();
		}

		assert(workingSet.size() == 1);

		if (BrBackEdgeSuccessors[node].find(workingSet[0]) ==
				BrBackEdgeSuccessors[node].end())
		{
			workingSet[0]->addChildNode(node);
			node->addAncestorNode(workingSet[0]);
		}
		else
		{
			workingSet[0]->addPHIChildNode(node);
			node->addAncestorNode(workingSet[0]);
		}

		BBPredicate[node->BB] = workingSet[0];
	}
	LLVM_DEBUG(dbgs() << "ConnectBB DONE! \n");
}

//WIP
int DFG::handlePHINodeFanIn()
{
	dfgNode *node;
	dfgNode *ancestor;
	dfgNode *temp;
	std::vector<dfgNode *> phiNodes;
	std::vector<dfgNode *> workingSet;
	std::vector<dfgNode *> nextWorkingSet;

	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		if (node->getNode() != NULL)
		{
			if (node->getNode()->getOpcode() == Instruction::PHI)
			{
				phiNodes.push_back(node);
			}
		}
	}

	if (phiNodes.empty())
	{
		return 0;
	}

	//Remove the current connection
	for (int i = 0; i < phiNodes.size(); ++i)
	{
		node = phiNodes[i];

		if (node->getAncestors().size() <= 2)
		{
			continue;
		}

		for (int j = 0; j < node->getAncestors().size(); ++j)
		{
			ancestor = node->getAncestors()[j];
			workingSet.push_back(node->getAncestors()[j]);

			ancestor->removeChild(node);
			node->removeAncestor(ancestor);
			removeEdge(findEdge(ancestor, node));
		}

		while (workingSet.size() > 1)
		{
			LLVM_DEBUG(dbgs() << "handlePHINodeFanIn :: "
					<< "workingSet.size() = " << workingSet.size() << "\n");
			for (int i = 0; i < workingSet.size() - 1; i = i + 2)
			{
				temp = new dfgNode(this);
				temp->setIdx(NodeList.size());
				NodeList.push_back(temp);
				temp->setNameType("SELECTPHI");
				workingSet[i]->addChildNode(temp);
				LLVM_DEBUG(dbgs() << "handlePHINodeFanIn :: "
						<< "workingSet[i+1] = " << workingSet[i + 1]->getIdx() << "\n");
				workingSet[i + 1]->addChildNode(temp);
				temp->addAncestorNode(workingSet[i]);
				temp->addAncestorNode(workingSet[i + 1]);
				nextWorkingSet.push_back(temp);
			}
			if (workingSet.size() % 2 == 1)
			{
				nextWorkingSet.push_back(workingSet[workingSet.size() - 1]);
			}

			workingSet.clear();
			for (int i = 0; i < nextWorkingSet.size(); ++i)
			{
				workingSet.push_back(nextWorkingSet[i]);
			}
			nextWorkingSet.clear();
		}
		assert(workingSet.size() == 1);
		workingSet[0]->addChildNode(node);
		node->addAncestorNode(workingSet[0]);
	}
}

void DFG::printXML()
{
	std::string fileName = name + "_dfg.xml";
	xmlFile.open(fileName.c_str());
	printDFGInfo();
	printHeaderTag("DFG-information");
	printOPs(1); //depth = 1
	printEdges(1);
	printFooterTag("DFG-information");
	xmlFile.close();
}

void DFG::printInEdges(dfgNode *node, int depth)
{
	printHeaderTag("In-edge-number", depth);
	xmlFile << node->getAncestors().size();
	printFooterTag("In-edge-number", depth);

	printHeaderTag("In-edges", depth);
	for (int i = 0; i < node->getAncestors().size(); ++i)
	{
		printHeaderTag("Edge", depth + 1);

		printHeaderTag("ID", depth + 2);
		//		xmlFile << node->getAncestors()[i] << "to" << node->getNode();
		Edge *e = findEdge(node->getAncestors()[i], node);
		assert(e);

		if (xmlEdgeIdxMap.find(e) == xmlEdgeIdxMap.end())
		{
			xmlEdgeIdxMap[e] = xmlEdgeCount;
			xmlEdgeCount++;
		}

		xmlFile << xmlEdgeIdxMap[e];
		printFooterTag("ID", depth + 2);

		printFooterTag("Edge", depth + 1);
	}

	printFooterTag("In-edges", depth);
}

void DFG::printOutEdges(dfgNode *node, int depth)
{
	printHeaderTag("Out-edge-number", depth);
	xmlFile << node->getChildren().size();
	printFooterTag("Out-edge-number", depth);

	printHeaderTag("Out-edges", depth);

	for (int i = 0; i < node->getChildren().size(); ++i)
	{
		printHeaderTag("Edge", depth + 1);

		printHeaderTag("ID", depth + 2);
		//		xmlFile << node->getNode() << "to" << node->getChildren()[i];
		//		xmlFile << findEdge(node,node->getChildren()[i])->getID() ;
		Edge *e = findEdge(node, node->getChildren()[i]);
		assert(e);
		if (xmlEdgeIdxMap.find(e) == xmlEdgeIdxMap.end())
		{
			xmlEdgeIdxMap[e] = xmlEdgeCount;
			xmlEdgeCount++;
		}
		xmlFile << xmlEdgeIdxMap[e];
		printFooterTag("ID", depth + 2);

		printFooterTag("Edge", depth + 1);
	}

	printFooterTag("Out-edges", depth);
}

void DFG::printOP(dfgNode *node, int depth)
{
	assert(xmlFile.is_open());
	printHeaderTag("OP", depth);

	printHeaderTag("ID", depth + 1);
	//	xmlFile << (node->getNode());
	xmlFile << node->getIdx();
	printFooterTag("ID", depth + 1);

	printHeaderTag("OP-type", depth + 1);
	//	xmlFile << "NORMAL"; //TODO :: Hardcoded to be Normal, fix the hardcoding

	if (node->getNode() != NULL)
	{
		switch (isMemoryOp(node))
		{
		case LOAD:
			xmlFile << "LOAD";
			break;
		case STORE:
			xmlFile << "STORE";
			break;
		default:
			xmlFile << "NORMAL";
			break;
		}
	}
	else
	{
		if (node->getNameType().compare("OutLoopLOAD") == 0)
		{
			xmlFile << "LOAD";
		}
		else if (node->getNameType().compare("OutLoopSTORE") == 0)
		{
			xmlFile << "STORE";
		}
		else
		{
			xmlFile << "NORMAL";
		}
		//xmlFile << node->getNameType();
	}

	printFooterTag("OP-type", depth + 1);

	printHeaderTag("Cycles", depth + 1);
	xmlFile << 1; //TODO :: Remove the hardcoding
	printFooterTag("Cycles", depth + 1);

	printInEdges(node, depth + 1);
	printOutEdges(node, depth + 1);

	printFooterTag("OP", depth);
}

void DFG::printOPs(int depth)
{
	assert(xmlFile.is_open());
	printHeaderTag("OPs", depth);

	printHeaderTag("OP-number", depth + 1);
	xmlFile << NodeList.size();
	printFooterTag("OP-number", depth + 1);

	for (int i = 0; i < NodeList.size(); ++i)
	{
		printOP(NodeList[i], depth + 1);
	}
	printFooterTag("OPs", depth);
}

void DFG::printDFGInfo()
{
	assert(xmlFile.is_open());
	xmlFile << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
}

void DFG::printHeaderTag(std::string tagName, int depth)
{
	assert(xmlFile.is_open());
	xmlFile << std::endl;
	for (int i = 0; i < depth; ++i)
	{
		xmlFile << "\t";
	}
	xmlFile << "<" << tagName << ">";
}

void DFG::printFooterTag(std::string tagName, int depth)
{
	assert(xmlFile.is_open());
	xmlFile << std::endl;
	for (int i = 0; i < depth; ++i)
	{
		xmlFile << "\t";
	}
	xmlFile << "</" << tagName << ">";
}

void DFG::printEdges(int depth)
{
	Edge *e;
	int edgeCount = 0;
	std::vector<Edge> localEdgeList;

	assert(xmlFile.is_open());
	printHeaderTag("EDGEs", depth);

	printHeaderTag("Edge-number", depth);

	//	for (int i = 0; i < edgeList.size(); ++i) {
	//		e = &(edgeList[i]);
	//		if(e->getType() == EDGE_TYPE_DATA){
	//			edgeCount++;
	//		}
	//	}
	//	xmlFile << edgeCount;
	assert(xmlEdgeIdxMap.size() == xmlEdgeCount);
	xmlFile << xmlEdgeIdxMap.size();

	printFooterTag("Edge-number", depth);

	// Currently only printing DATA dependency edges only
	//	for (int i = 0; i < edgeList.size(); ++i) {
	//		e = &(edgeList[i]);
	//		if(e->getType() == EDGE_TYPE_DATA){
	//			printEdge(e,depth);
	//		}
	//	}

	for (std::pair<Edge *, int> pair : xmlEdgeIdxMap)
	{
		printEdge(pair.first, depth);
	}

	printFooterTag("EDGEs", depth);
}

void DFG::renumber()
{
	for (int i = 0; i < NodeList.size(); ++i)
	{
		NodeList[i]->setIdx(i);
	}

	for (int i = 0; i < edgeList.size(); ++i)
	{
		edgeList[i].setID(i);
	}
}

void DFG::printEdge(Edge *e, int depth)
{
	assert(xmlFile.is_open());
	printHeaderTag("EDGE", depth);

	printHeaderTag("ID", depth + 1);
	xmlFile << xmlEdgeIdxMap[e];
	printFooterTag("ID", depth + 1);

	printHeaderTag("Attribute", depth + 1);
	xmlFile << 1; //God knows what this is
	printFooterTag("Attribute", depth + 1);

	printHeaderTag("Start-OP", depth + 1);
	//	xmlFile << e->getSrc();
	xmlFile << e->getSrc()->getIdx();
	printFooterTag("Start-OP", depth + 1);

	printHeaderTag("End-OP", depth + 1);
	//	xmlFile << e->getDest();
	xmlFile << e->getDest()->getIdx();
	printFooterTag("End-OP", depth + 1);

	printFooterTag("EDGE", depth);
}

void DFG::addMemRecDepEdges(DependenceInfo *DI)
{
	std::vector<dfgNode *> memNodes;
	dfgNode *nodePtr;
	Instruction *Ins;
	std::ofstream log;

	static int count = 0;
	std::string Filename = (NodeList[0]->getNode()->getFunction()->getName() + "_L" + std::to_string(count++) + "addMemRecDepEdges.log").str();
	log.open(Filename.c_str());
	log << "Started...\n";

	LLVM_DEBUG(dbgs() << "&&&&&&&&&&&&&&&&&&&&Recurrence search started.....!\n");

	//Create a list of memory instructions
	for (int i = 0; i < NodeList.size(); ++i)
	{
		nodePtr = NodeList[i];
		Ins = dyn_cast<Instruction>(nodePtr->getNode());
		if (!Ins)
			return;

		LoadInst *Ld = dyn_cast<LoadInst>(nodePtr->getNode());
		StoreInst *St = dyn_cast<StoreInst>(nodePtr->getNode());

		if (!St && !Ld)
			continue;
		if (Ld && !Ld->isSimple())
			return;
		if (St && !St->isSimple())
			return;
		LLVM_DEBUG(dbgs() << "ID=" << nodePtr->getIdx() << " ,");
		LLVM_DEBUG(nodePtr->getNode()->dump());
		memNodes.push_back(nodePtr);
	}

	log << "addMemRecDepEdges : Found " << memNodes.size() << "Loads and Stores to analyze\n";

	for (int i = 0; i < memNodes.size(); ++i)
	{
		for (int j = i; j < memNodes.size(); ++j)
		{
			std::vector<char> Dep;
			Instruction *Src = dyn_cast<Instruction>(memNodes[i]->getNode());
			Instruction *Des = dyn_cast<Instruction>(memNodes[j]->getNode());

			if (Src == Des)
				continue;

			if (isa<LoadInst>(Src) && isa<LoadInst>(Des))
				continue;

			if (auto D = DI->depends(Src, Des, true))
			{
				log << "addMemRecDepEdges :"
						<< "Found Dependency between Src=" << memNodes[i]->getIdx() << " Des=" << memNodes[j]->getIdx() << "\n";

				if (D->isFlow())
				{
					// TODO: Handle Flow dependence.Check if it is sufficient to populate
					// the Dependence Matrix with the direction reversed.
					log << "addMemRecDepEdges :"
							<< "Flow dependence not handled\n";
					continue;
					//					  return;
				}
				if (D->isAnti())
				{
					log << "Found Anti dependence \n";

					this->findNode(Src)->addChild(Des, EDGE_TYPE_LDST);
					this->findNode(Des)->addAncestor(Src);

					unsigned Levels = D->getLevels();
					char Direction;
					for (unsigned II = 1; II <= Levels; ++II)
					{
						const SCEV *Distance = D->getDistance(II);

						const SCEVConstant *SCEVConst = dyn_cast_or_null<SCEVConstant>(Distance);
						if (SCEVConst)
						{
							const ConstantInt *CI = SCEVConst->getValue();
							if (CI->isNegative())
							{
								Direction = '<';
							}
							else if (CI->isZero())
							{
								Direction = '=';
							}
							else
							{
								Direction = '>';
							}
							log << SCEVConst->getAPInt().abs().getZExtValue() << std::endl;
							Dep.push_back(Direction);
						}
						else if (D->isScalar(II))
						{
							Direction = 'S';
							Dep.push_back(Direction);
						}
						else
						{
							unsigned Dir = D->getDirection(II);
							if (Dir == Dependence::DVEntry::LT || Dir == Dependence::DVEntry::LE)
							{
								Direction = '<';
							}
							else if (Dir == Dependence::DVEntry::GT || Dir == Dependence::DVEntry::GE)
							{
								Direction = '>';
							}
							else if (Dir == Dependence::DVEntry::EQ)
							{
								Direction = '=';
							}
							else
							{
								Direction = '*';
							}
							Dep.push_back(Direction);
						}
					}
					//					while (Dep.size() != Level) {
					//					  Dep.push_back('I');
					//					}
				}

				for (int k = 0; k < Dep.size(); ++k)
				{
					log << Dep[k];
				}
				log << "\n";
			}
		}
	}

	log.close();
}

void DFG::findMaxRecDist()
{
	dfgNode *node;
	int recDist;
	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];

		for (int j = 0; j < node->getRecAncestors().size(); ++j)
		{
			recDist = node->getASAPnumber() - node->getRecAncestors()[j]->getASAPnumber();
			if (maxRecDist < recDist)
			{
				maxRecDist = recDist;
			}
		}

		for (dfgNode *phiParent : node->getPHIancestors())
		{
			const BasicBlock *phiBB = phiParent->BB;
			const BasicBlock *nodeBB = node->BB;

			SmallVector<std::pair<const BasicBlock *, const BasicBlock *>, 8> Result;
			FindFunctionBackedges(*(nodeBB->getParent()), Result);

			std::pair<const BasicBlock *, const BasicBlock *> bbCouple(phiBB, nodeBB);
			if (std::find(Result.begin(), Result.end(), bbCouple) != Result.end())
			{
				int phiDist = phiParent->getASAPnumber() - node->getASAPnumber();
				if (maxRecDist < phiDist)
				{
					maxRecDist = phiDist;
				}
			}
		}
	}
}

void DFG::addMemRecDepEdgesNew(DependenceInfo *DI)
{
	std::vector<dfgNode *> memNodes;
	dfgNode *nodePtr;
	Instruction *Ins;
	std::ofstream log;
	int RecDist;

	static int count = 0;
	std::string Filename = (NodeList[0]->getNode()->getFunction()->getName() + "_L" + std::to_string(count++) + "addMemRecDepEdges.log").str();
	log.open(Filename.c_str());
	log << "Started...\n";

	LLVM_DEBUG(dbgs() << "&&&&&&&&&&&&&&&&&&&&Recurrence search started.....!\n");

	//Create a list of memory instructions
	for (int i = 0; i < NodeList.size(); ++i)
	{
		nodePtr = NodeList[i];

		if (nodePtr->getNode() == NULL)
		{
			continue;
		}
		Ins = dyn_cast<Instruction>(nodePtr->getNode());
		if (!Ins)
			return;

		LoadInst *Ld = dyn_cast<LoadInst>(nodePtr->getNode());
		StoreInst *St = dyn_cast<StoreInst>(nodePtr->getNode());

		if (!St && !Ld)
			continue;
		if (Ld && !Ld->isSimple())
			return;
		if (St && !St->isSimple())
			return;
		LLVM_DEBUG(dbgs() << "ID=" << nodePtr->getIdx() << " ,");
		LLVM_DEBUG(nodePtr->getNode()->dump());
		memNodes.push_back(nodePtr);
	}

	log << "addMemRecDepEdges : Found " << memNodes.size() << "Loads and Stores to analyze\n";

	for (int i = 0; i < memNodes.size(); ++i)
	{
		for (int j = i; j < memNodes.size(); ++j)
		{
			std::vector<char> Dep;
			Instruction *Src = dyn_cast<Instruction>(memNodes[i]->getNode());
			Instruction *Des = dyn_cast<Instruction>(memNodes[j]->getNode());

			if (Src == Des)
				continue;

			if (isa<LoadInst>(Src) && isa<LoadInst>(Des))
				continue;

			if (auto D = DI->depends(Src, Des, true))
			{
				log << "addMemRecDepEdges :"
						<< "Found Dependency between Src=" << memNodes[i]->getIdx() << " Des=" << memNodes[j]->getIdx() << "\n";
				LLVM_DEBUG(dbgs() << "addMemRecDepEdges :"
						<< "Found Dependency between Src=" << memNodes[i]->getIdx() << " Des=" << memNodes[j]->getIdx() << "\n");

				if (D->isLoopIndependent())
				{
					LLVM_DEBUG(dbgs() << "Loop Independent.\n");
				}
				else
				{
					LLVM_DEBUG(dbgs() << "Loop Dependent.\n");
				}

				LLVM_DEBUG(outs() << "Levels = " << D->getLevels() << "\n");
				LLVM_DEBUG(D->dump(outs()));

				char Direction;
				for (int i = 1; i <= D->getLevels(); i++)
				{
					const SCEV *Distance = D->getDistance(i);
					const SCEVConstant *SCEVConst = dyn_cast_or_null<SCEVConstant>(Distance);
					if (SCEVConst)
					{
						const ConstantInt *CI = SCEVConst->getValue();
						if (CI->isNegative())
							Direction = '<';
						else if (CI->isZero())
							Direction = '=';
						else
							Direction = '>';
					}
					else if (D->isScalar(i))
					{
						Direction = 'S';
					}
					else
					{
						unsigned Dir = D->getDirection(i);
						if (Dir == Dependence::DVEntry::LT || Dir == Dependence::DVEntry::LE)
							Direction = '<';
						else if (Dir == Dependence::DVEntry::GT || Dir == Dependence::DVEntry::GE)
							Direction = '>';
						else if (Dir == Dependence::DVEntry::EQ)
							Direction = '=';
						else
							Direction = '*';
					}

					LLVM_DEBUG(dbgs() << "\tlevel=" << i << ",dir=" << Direction << "\n");
				}

				if (D->isAnti())
				{
					// TODO: Handle Anit dependence.Check if it is sufficient to populate
					// the Dependence Matrix with the direction reversed.
					log << "addMemRecDepEdges :"
							<< "Anti dependence not handled\n";
					continue;
					//					  return;
				}

				if (D->isFlow())
				{
					log << "Found Flow dependence \n";

					if (std::find(BBSuccBasicBlocks[Src->getParent()].begin(), BBSuccBasicBlocks[Src->getParent()].end(), Des->getParent()) == BBSuccBasicBlocks[Src->getParent()].end())
					{
						continue;
					}

					//							this->findNode(Src)->addRecChild(Des,EDGE_TYPE_LDST);
					//							this->findNode(Des)->addRecAncestor(Src);
					std::string depType;
					if (D->isFlow())
					{
						depType = "FLOW";
					}
					else
					{
						assert(D->isAnti());
						depType = "ANTI";
					}

					if (D->isConfused())
						continue;

					LLVM_DEBUG(dbgs() << "adding recurrence relation!\n");
					this->findNode(Des)->addRecChild(Src, depType, EDGE_TYPE_LDST);
					this->findNode(Src)->addRecAncestor(Des, depType);

					//							RecDist = this->findNode(Des)->getASAPnumber() - this->findNode(Src)->getASAPnumber();
					//							if(maxRecDist < RecDist){
					//								maxRecDist = RecDist;
					//							}

					unsigned Levels = D->getLevels();
					char Direction;

					for (unsigned II = 1; II <= Levels; ++II)
					{
						const SCEV *Distance = D->getDistance(II);

						const SCEVConstant *SCEVConst = dyn_cast_or_null<SCEVConstant>(Distance);
						if (SCEVConst)
						{
							const ConstantInt *CI = SCEVConst->getValue();
							if (CI->isNegative())
							{
								Direction = '<';
							}
							else if (CI->isZero())
							{
								Direction = '=';
							}
							else
							{
								Direction = '>';
							}
							log << SCEVConst->getAPInt().abs().getZExtValue() << std::endl;
							Dep.push_back(Direction);
						}
						else if (D->isScalar(II))
						{
							Direction = 'S';
							Dep.push_back(Direction);
						}
						else
						{
							unsigned Dir = D->getDirection(II);
							if (Dir == Dependence::DVEntry::LT || Dir == Dependence::DVEntry::LE)
							{
								Direction = '<';
							}
							else if (Dir == Dependence::DVEntry::GT || Dir == Dependence::DVEntry::GE)
							{
								Direction = '>';
							}
							else if (Dir == Dependence::DVEntry::EQ)
							{
								Direction = '=';
							}
							else
							{
								Direction = '*';
							}
							Dep.push_back(Direction);
						}
					}
					//					while (Dep.size() != Level) {
					//					  Dep.push_back('I');
					//					}
				}

				for (int k = 0; k < Dep.size(); ++k)
				{
					log << Dep[k];
				}
				log << "\n";
			}
		}
	}

	log.close();
	//		assert(false);
	LLVM_DEBUG(dbgs() << "&&&&&&&&&&&&&&&&&&&&Recurrence search done.....!\n");
}

void DFG::addMemDepEdges(MemoryDependenceResults *MD)
{

	assert(NodeList.size() > 0);
	dfgNode firstNode = *NodeList[0];
	SmallVector<std::pair<const BasicBlock *, const BasicBlock *>, 1> Result;
	FindFunctionBackedges(*(firstNode.getNode()->getFunction()), Result);

	Instruction *it;
	MemDepResult mRes;
	SmallVector<NonLocalDepResult, 1> result;
	for (int i = 0; i < NodeList.size(); ++i)
	{
		it = NodeList[i]->getNode();
		if (it->mayReadOrWriteMemory())
		{
			mRes = MD->getDependency(it);

			LLVM_DEBUG(dbgs() << "Dependent :");
			LLVM_DEBUG(it->dump());

			if (mRes.isNonLocal())
			{
				if (auto CS = CallSite(it))
				{
				}
				else
				{
					LLVM_DEBUG(dbgs() << "****** GET NON LOCAL MEM DEPENDENCIES ******\n");

					MD->getNonLocalPointerDependency(it, result);

					for (int j = 0; j < result.size(); ++j)
					{
						if (result[j].getResult().getInst() != NULL)
						{

							if ((result[j].getResult().getInst()->getOpcode() == Instruction::Load) && (it->getOpcode() == Instruction::Load))
							{
							}
							else
							{

								std::pair<const BasicBlock *, const BasicBlock *> bbCouple(result[j].getResult().getInst()->getParent(), it->getParent());
								if (std::find(Result.begin(), Result.end(), bbCouple) != Result.end())
								{
									continue;
								}

								LLVM_DEBUG(result[j].getResult().getInst()->dump());
								this->findNode(result[j].getResult().getInst())->addChild(it, EDGE_TYPE_LDST);
								NodeList[i]->addAncestor(result[j].getResult().getInst());
							}
						}
					}

					LLVM_DEBUG(dbgs() << "****** DONE NON LOCAL MEM DEPENDENCIES ******\n");
				}
			}

			if (mRes.getInst() != NULL)
			{

				if ((mRes.getInst()->getOpcode() == Instruction::Load) && (it->getOpcode() == Instruction::Load))
				{
					//					continue;
				}
				else
				{
					this->findNode(mRes.getInst())->addChild(it, EDGE_TYPE_LDST);
					NodeList[i]->addAncestor(mRes.getInst());
				}
			}
		}
	}
}

int DFG::removeEdge(Edge *e)
{
	Edge *edgePtr;
	edgePtr = findEdge(e->getSrc(), e->getDest());
	if (edgePtr == NULL)
	{
		return -1;
	}

	for (int i = 0; i < edgeList.size(); ++i)
	{
		if (edgePtr == &(edgeList[i]))
		{
			edgeList.erase(edgeList.begin() + i);
		}
	}

	return 1;
}

int DFG::removeNode(dfgNode *n)
{
	dfgNode *nodePtr;
	nodePtr = findNode(n->getNode());
	if (nodePtr == NULL)
	{
		return -1;
	}

	dfgNode *nodeTmp;
	dfgNode *nodeTmp2;
	Edge *edgeTmp;

	//Treat the ancestors
	for (int i = 0; i < nodePtr->getAncestors().size(); ++i)
	{
		nodeTmp = nodePtr->getAncestors()[i];
		assert(nodeTmp->removeChild(nodePtr->getNode()) != -1);

		for (int j = 0; j < nodePtr->getChildren().size(); ++j)
		{
			nodeTmp->addChild(nodePtr->getChildren()[j]->getNode()); //TODO : Generalize for types of children
			nodeTmp2 = nodePtr->getChildren()[j];

			assert(nodeTmp2->removeAncestor(nodePtr->getNode()) != -1);
			nodeTmp2->addAncestor(nodePtr->getAncestors()[i]->getNode());
		}

		edgeTmp = findEdge(nodePtr->getAncestors()[i], nodePtr);
		assert(removeEdge(edgeTmp) != -1);
	}

	//Treat the children
	for (int i = 0; i < nodePtr->getChildren().size(); ++i)
	{
		nodeTmp = nodePtr->getChildren()[i];
		assert(nodeTmp->removeAncestor(nodePtr->getNode()) != -1);

		for (int j = 0; j < nodePtr->getAncestors().size(); ++j)
		{
			nodeTmp->addAncestor(nodePtr->getAncestors()[j]->getNode());
			nodeTmp2 = nodePtr->getAncestors()[j];

			assert(nodeTmp2->removeChild(nodePtr->getNode()) != -1);
			nodeTmp2->addChild(nodePtr->getChildren()[i]->getNode());
		}

		edgeTmp = findEdge(nodePtr, nodePtr->getChildren()[i]);
		assert(removeEdge(edgeTmp) != -1);
	}

	//	NodeList.erase(std::remove(NodeList.begin(),NodeList.end(),*n),NodeList.end());

	for (int i = 0; i < NodeList.size(); ++i)
	{
		if (nodePtr == NodeList[i])
		{
			NodeList.erase(NodeList.begin() + i);
		}
	}

	return 1;
}

void DFG::removeAlloc()
{
	std::unordered_set<dfgNode*> rn;

	for(dfgNode* n : NodeList){
		if(n->getNode() && isa<AllocaInst>(n->getNode())){
			rn.insert(n);
		}
		else if(OutLoopNodeMapReverse.find(n) != OutLoopNodeMapReverse.end()){
			if(isa<AllocaInst>(OutLoopNodeMapReverse[n])){
				rn.insert(n);
			}
		}
	}

	for(dfgNode* n : rn){
		for(dfgNode* chl : n->getChildren()){
			chl->removeAncestor(n);
		}
		for(dfgNode* anc : n->getAncestors()){
			anc->removeChild(n);
		}
		NodeList.erase(std::remove(NodeList.begin(), NodeList.end(), n), NodeList.end());
	}
}

void DFG::traverseDFS(dfgNode *startNode, int dfsCount)
{
	//	//Assumption should be a connected DFG
	//	assert(getRoots().size()==1);
	//
	//	for (int i = 0; i < startNode->getChildren(); ++i) {
	//
	//	}
}

void DFG::traverseBFS(dfgNode *node, int ASAPlevel)
{

	if (node->getNode())
	{
		LLVM_DEBUG(node->getNode()->dump());
	}

	dfgNode *child;
	for (int i = 0; i < node->getChildren().size(); ++i)
	{

		if (maxASAPLevel < ASAPlevel)
		{
			maxASAPLevel = ASAPlevel;
		}

		child = node->getChildren()[i];
		if (child->getASAPnumber() < ASAPlevel)
		{
			child->setASAPnumber(ASAPlevel);
			traverseBFS(child, ASAPlevel + 1);
		}
	}

	for (int i = 0; i < node->getRecChildren().size(); ++i)
	{

		if (maxASAPLevel < ASAPlevel)
		{
			maxASAPLevel = ASAPlevel;
		}

		child = node->getRecChildren()[i];
		if (child->getASAPnumber() < ASAPlevel)
		{
			child->setASAPnumber(ASAPlevel);
			traverseBFS(child, ASAPlevel + 1);
		}
	}
}

void DFG::traverseInvBFS(dfgNode *node, int ALAPlevel)
{
	dfgNode *ancestor;
	for (int i = 0; i < node->getAncestors().size(); ++i)
	{
		ancestor = node->getAncestors()[i];
		if (ancestor->getALAPnumber() < ALAPlevel)
		{
			ancestor->setALAPnumber(ALAPlevel);
			traverseInvBFS(ancestor, ALAPlevel + 1);
		}
	}

	for (int i = 0; i < node->getRecAncestors().size(); ++i)
	{
		ancestor = node->getRecAncestors()[i];
		if (ancestor->getALAPnumber() < ALAPlevel)
		{
			ancestor->setALAPnumber(ALAPlevel);
			traverseInvBFS(ancestor, ALAPlevel + 1);
		}
	}
}

void DFG::scheduleASAP()
{
	std::vector<dfgNode *> leafs;
	leafs = getLeafs();
	int nodesVisited = 0;

	for (int i = 0; i < leafs.size(); ++i)
	{
		leafs[i]->setASAPnumber(0);
		traverseBFS(leafs[i], 1);
	}
	LLVM_DEBUG(dbgs() << "scheduleASAP DONE!\n");
}

void DFG::scheduleALAP()
{
	dfgNode *node;

	int currASAPnumber = -1;
	std::vector<dfgNode *> roots;
	//	for (int i = 0; i < NodeList.size(); ++i) {
	//		node = &(NodeList[i]);
	////		assert(node->getASAPnumber() != -1);
	//		if(node->getASAPnumber() > currASAPnumber){
	//			currASAPnumber = node->getASAPnumber();
	//		}
	//	}

	//	for (int i = 0; i < NodeList.size(); ++i) {
	//		if(node->getASAPnumber() == currASAPnumber){
	//			roots.push_back(node);
	//		}
	//	}

	roots = getRoots();

	for (int i = 0; i < roots.size(); ++i)
	{
		roots[i]->setALAPnumber(0);
		traverseInvBFS(roots[i], 1);
	}

	for (int i = 0; i < NodeList.size(); ++i)
	{
		NodeList[i]->setALAPnumber(maxASAPLevel - NodeList[i]->getALAPnumber());
	}

	LLVM_DEBUG(dbgs() << "scheduleALAP DONE!\n");
}

void DFG::balanceASAPALAP()
{
	dfgNode *node;

	std::map<dfgNode *, int> newASAPVal;

	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		int ASAP_ALAP_diff = node->getALAPnumber() - node->getASAPnumber();
		ASAP_ALAP_diff = ASAP_ALAP_diff / 2;
		newASAPVal[node] = node->getASAPnumber() + ASAP_ALAP_diff;
	}

	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		if (node->getChildren().empty())
		{
			node->setASAPnumber(newASAPVal[node]);
		}
		else
		{
			int minASAP = 0;
			for (dfgNode *child : node->getChildren())
			{
				if (minASAP < newASAPVal[child])
				{
					minASAP = newASAPVal[child];
				}
			}

			if (newASAPVal[node] < minASAP)
			{
				node->setASAPnumber(newASAPVal[node]);
			}
			else
			{
				node->setASAPnumber(minASAP - 1);
				newASAPVal[node] = minASAP - 1;
			}
		}
	}

	//	for (int i = 0; i < NodeList.size(); ++i) {
	//		node = NodeList[i];
	//		int ASAP_ALAP_diff = node->getALAPnumber() - node->getASAPnumber();
	//		ASAP_ALAP_diff = ASAP_ALAP_diff/2;
	//		if(ASAP_ALAP_diff > 2){ //Hack for compressed sensing
	//			node->setASAPnumber(node->getASAPnumber() + ASAP_ALAP_diff + 2);
	//		}
	//		else if(ASAP_ALAP_diff>0){
	//			node->setASAPnumber(node->getASAPnumber() + ASAP_ALAP_diff);
	//		}
	////		node->setASAPnumber(node->getALAPnumber());
	//	}
}

std::vector<dfgNode *> DFG::getLeafs()
{
	std::vector<dfgNode *> leafNodes;
	for (int i = 0; i < NodeList.size(); i++)
	{
		if (NodeList[i]->getAncestors().size() == 0)
		{
			leafNodes.push_back(NodeList[i]);
		}
	}
	return leafNodes;
}

std::vector<std::vector<unsigned char>> DFG::selfMulConMat(
		std::vector<std::vector<unsigned char>> in)
{

	std::vector<std::vector<unsigned char>> out;

	for (int i = 0; i < in.size(); ++i)
	{

		//Only support square matrices
		assert(in.size() == in[i].size());

		std::vector<unsigned char> tempVec;
		for (int j = 0; j < in.size(); ++j)
		{
			unsigned char tempInt = 0;
			for (int k = 0; k < in.size(); ++k)
			{
				tempInt = tempInt | (in[i][k] & in[k][j]);
			}
			tempVec.push_back(tempInt);
		}
		out.push_back(tempVec);
	}

	return out;
}

std::vector<std::vector<unsigned char>> DFG::getConMat()
{

	int nodelistSize = NodeList.size();

	std::vector<std::vector<unsigned char>> conMat;
	dfgNode *node;
	dfgNode *child;

	for (int i = 0; i < nodelistSize; ++i)
	{
		std::vector<unsigned char> temp;
		for (int j = 0; j < nodelistSize; ++j)
		{
			temp.push_back(0);
		}
		conMat.push_back(temp);
	}

	for (int i = 0; i < nodelistSize; ++i)
	{
		node = NodeList[i];
		for (int j = 0; j < node->getChildren().size(); ++j)
		{
			child = node->getChildren()[j];
			conMat[node->getIdx()][child->getIdx()] = 1;
		}
	}

	return conMat;
}

int DFG::getAffinityCost(dfgNode *a, dfgNode *b)
{
	int nodelistSize = NodeList.size();

	assert(a->getASAPnumber() == b->getASAPnumber());
	int currLevel = a->getASAPnumber();
	int maxDist = std::min(maxASAPLevel - currLevel, 2);

	int affLevel = 0;
	int affCost = 0;

	if (maxASAPLevel == currLevel)
	{
		return 0;
	}

	while (1)
	{
		for (int i = 0; i < nodelistSize; ++i)
		{

			if (conMatArr[affLevel][a->getIdx()][i] == 1)
			{
				if (conMatArr[affLevel][b->getIdx()][i] == 1)
				{
					//					affCost = affCost + 2**(maxDist - (affLevel+1));
					//					affCost = affCost + (int)pow(2,(maxDist - (affLevel+1)));
					affCost = affCost + (2 << (maxDist - (affLevel + 1)));
				}
			}
		}

		affLevel++;
		if (affLevel == maxDist)
		{
			return affCost;
		}

		if (affLevel == conMatArr.size())
		{
			conMatArr.push_back(selfMulConMat(conMatArr[affLevel - 1]));
		}
	}
}

std::vector<int> DFG::getIntersection(std::vector<std::vector<int>> vectors)
{

	assert(vectors.size() != 0);
	std::vector<int> result;

	for (int i = 0; i < vectors[0].size(); ++i)
	{
		result.push_back(1);
	}

	for (int i = 1; i < vectors.size(); ++i)
	{
		assert(vectors[i].size() == vectors[0].size());
		for (int j = 0; j < vectors[i].size(); ++j)
		{
			result[j] = result[j] & vectors[i][j];
		}
	}

	return result;
}

int DFG::getDist(CGRANode *src, CGRANode *dest)
{
	assert(src->getmappedDFGNode() != NULL);
	assert(src->getmappedDFGNode() != NULL);

	int xCost = abs(src->getX() - dest->getX());
	int yCost = abs(src->getY() - dest->getY());
	int tCost = dest->getT() - src->getT();

	if (tCost < 0)
	{
		return INT32_MAX;
	}

	if (xCost + yCost > tCost)
	{
		return INT32_MAX;
	}

	return xCost + yCost;
}

int DFG::AStarSP(CGRANode *src, CGRANode *dest, std::vector<CGRANode *> *path)
{

	struct LessThanFScore
	{
		bool operator()(const std::pair<CGRANode *, int> &left, const std::pair<CGRANode *, int> &right) const
		{
			return left.second < right.second;
		}
	};

	path->clear();
	std::vector<ConnectedCGRANode> connectedNodes;
	std::vector<CGRANode *> closedSet;
	std::vector<CGRANode *> openSet;

	std::map<CGRANode *, int> fscore;
	std::map<CGRANode *, int> gscore;
	std::map<CGRANode *, CGRANode *> prevNode;
	//	std::priority_queue<AStarNode, std::vector<AStarNode>, LessThanCost> openSet;

	openSet.push_back(src);
	fscore[src] = getDist(src, dest);

	if (fscore[src] == INT32_MAX)
	{
		return -1;
	}

	gscore[src] = 0;

	CGRANode *current;
	int tentgscore;
	int dist;
	while (!openSet.empty())
	{
		//		anode = openSet.top();
		std::map<CGRANode *, int>::iterator it = std::min_element(fscore.begin(), fscore.end(), LessThanFScore());
		current = (*it).first;
		fscore.erase(it);
		openSet.erase(std::remove(openSet.begin(), openSet.end(), current), openSet.end());
		closedSet.push_back(current);
		//		openSet.pop();

		if (current == dest)
		{
			break;
		}

		connectedNodes = current->getConnectedNodes();
		for (int i = 0; i < connectedNodes.size(); ++i)
		{

			if (connectedNodes[i].node->getmappedDFGNode() != NULL)
			{
				continue;
			}

			dist = getDist(current, connectedNodes[i].node);
			if (dist == INT32_MAX || gscore[current] == INT32_MAX)
			{
				tentgscore = INT32_MAX;
			}
			else
			{
				tentgscore = gscore[current] + getDist(current, connectedNodes[i].node);
			}

			if (std::find(closedSet.begin(), closedSet.end(), connectedNodes[i].node) != closedSet.end())
			{
				continue;
			}

			if (std::find(openSet.begin(), openSet.end(), connectedNodes[i].node) == openSet.end())
			{
				openSet.push_back(connectedNodes[i].node);
			}
			else
			{
				if (gscore.find(connectedNodes[i].node) == gscore.end())
				{
					gscore[connectedNodes[i].node] = INT32_MAX;
				}

				if (tentgscore >= gscore[connectedNodes[i].node])
				{
					continue;
				}
			}

			prevNode[connectedNodes[i].node] = current;
			gscore[connectedNodes[i].node] = tentgscore;

			dist = getDist(connectedNodes[i].node, dest);
			if (dist == INT32_MAX)
			{
				fscore[connectedNodes[i].node] = INT32_MAX;
			}
			else
			{
				fscore[connectedNodes[i].node] = tentgscore + getDist(connectedNodes[i].node, dest);
			}
		}
	}

	int routeCost = 0;

	if (current != dest)
	{
		return -1;
	}
	else
	{
		while (current != src)
		{
			path->push_back(current);
			current = prevNode[current];
			routeCost++;
		}
		return routeCost;
	}
}

int DFG::AddRoutingEdges(dfgNode *node)
{

	std::vector<dfgNode *> parents;
	CGRANode *cnode;
	std::vector<std::vector<int>> parentConnectedPhyNodes;
	std::vector<int> parentIntersectionTmp;
	int routingPossibilities = 0;

	for (int j = 0; j < node->getAncestors().size(); ++j)
	{
		parents.push_back(node->getAncestors()[j]);
	}

	for (int j = 0; j < parents.size(); ++j)
	{
		parentConnectedPhyNodes.push_back(currCGRA->getPhyConMatNode(parents[j]->getMappedLoc()));
	}

	do
	{
		parentIntersectionTmp = getIntersection(parentConnectedPhyNodes);

		routingPossibilities = 0;
		for (int j = 0; j < parentIntersectionTmp.size(); ++j)
		{
			cnode = currCGRA->getCGRANode(j);
			if (cnode->getmappedDFGNode() != NULL)
			{
				parentIntersectionTmp[j] = 0;
			}

			routingPossibilities = routingPossibilities + parentIntersectionTmp[j];
		}

		if (routingPossibilities == 0)
		{
		}

	} while (routingPossibilities == 0);
}

std::map<dfgNode *, std::vector<CGRANode *>> DFG::getPrimarySlots(
		std::vector<dfgNode *> nodes)
{

	dfgNode *node;
	//	dfgNode* parent;
	CGRANode *cnode;

	std::map<dfgNode *, std::vector<CGRANode *>> result;
	std::vector<dfgNode *> parents;

	std::vector<std::vector<int>> parentConnectedPhyNodes;
	std::vector<int> parentIntersection;
	std::vector<int> parentIntersectionTmp;

	std::vector<std::vector<int>> phyConMat;

	bool commonConsumer = false;
	float probCost;
	int routingPossibilities = 0;

	for (int i = 0; i < nodes.size(); ++i)
	{
		node = nodes[i];
		for (int j = 0; j < node->getAncestors().size(); ++j)
		{
			parents.push_back(node->getAncestors()[j]);
		}

		for (int j = 0; j < parents.size(); ++j)
		{
			parentConnectedPhyNodes.push_back(currCGRA->getPhyConMatNode(parents[j]->getMappedLoc()));
		}

		parentIntersectionTmp = getIntersection(parentConnectedPhyNodes);

		for (int j = 0; j < parentIntersectionTmp.size(); ++j)
		{
			cnode = currCGRA->getCGRANode(j);
			if (cnode->getmappedDFGNode() != NULL)
			{
				parentIntersectionTmp[j] = 0;
			}

			routingPossibilities = routingPossibilities + parentIntersectionTmp[j];
		}

		if (routingPossibilities == 0)
		{
		}

		node->setRoutingPossibilities(routingPossibilities);

		for (int j = 0; j < parentIntersectionTmp.size(); ++j)
		{
			if (parentIntersectionTmp[j] == 1)
			{
				cnode = currCGRA->getCGRANode(j);
				if (cnode->getRoutingNode() != NULL)
				{
					if (node->getRoutingPossibilities() < cnode->getRoutingNode()->getRoutingPossibilities())
					{
						cnode->getRoutingNode()->setRoutingPossibilities(cnode->getRoutingNode()->getRoutingPossibilities() - 1);
						result[cnode->getRoutingNode()].erase(std::remove(result[cnode->getRoutingNode()].begin(), result[cnode->getRoutingNode()].end(), cnode), result[cnode->getRoutingNode()].end());
						cnode->setRoutingNode(node);
					}
				}
				else
				{
					cnode->setRoutingNode(node);
					result[node].push_back(cnode);
				}
			}
		}
	}

	for (int i = 0; i < nodes.size(); ++i)
	{
		node = nodes[i];
		for (int j = 0; j < node->getAncestors().size(); ++j)
		{
			parents.push_back(node->getAncestors()[j]);
		}

		//		if(parents.size() == 0){
		//			for (int var = 0; var < max; ++var) {
		//
		//			}
		//		}

		for (int j = 0; j < parents.size(); ++j)
		{
			parentConnectedPhyNodes.push_back(currCGRA->getPhyConMatNode(parents[j]->getMappedLoc()));
		}

		parentIntersection = getIntersection(parentConnectedPhyNodes);

		while (parentIntersection.size() == 0)
		{
			parentConnectedPhyNodes.clear();
			phyConMat = currCGRA->getPhyConMat();
			//			phyConMat = selfMulConMat(phyConMat);

			for (int j = 0; j < parents.size(); ++j)
			{
				cnode = parents[j]->getMappedLoc();
				parentConnectedPhyNodes.push_back(phyConMat[currCGRA->getConMatIdx(cnode->getT(), cnode->getY(), cnode->getX())]);
			}
		}

		std::vector<CGRANode *> resultNodeArr;
		for (int j = 0; j < parentIntersection.size(); ++j)
		{
			if (parentIntersection[j] == 1)
			{
				resultNodeArr.push_back(currCGRA->getCGRANode(j));
			}
		}
		result[node] = resultNodeArr;
	}

	return result;
}

bool DFG::MapMultiDestRec(
		std::map<dfgNode *, std::vector<std::pair<CGRANode *, int>>> *nodeDestMap,
		std::map<CGRANode *, std::vector<dfgNode *>> *destNodeMap,
		std::map<dfgNode *, std::vector<std::pair<CGRANode *, int>>>::iterator it,
		std::map<CGRANode *, std::vector<CGRAEdge>> cgraEdges,
		int index)
{

	std::map<dfgNode *, std::vector<std::pair<CGRANode *, int>>> localNodeDestMap = *nodeDestMap;
	std::map<CGRANode *, std::vector<dfgNode *>> localdestNodeMap = *destNodeMap;

	dfgNode *node;
	dfgNode *otherNode;
	dfgNode *parent;
	CGRANode *cnode;
	CGRANode *chosenCnode = NULL;
	std::pair<CGRANode *, int> cnodePair;
	CGRANode *parentExt;
	//	std::vector< std::pair<CGRANode*,int> > possibleDests;
	std::vector<dfgNode *> parents;

	//	std::vector<std::pair<CGRANode*, CGRANode*> > paths;
	std::vector<CGRANode *> dests;
	std::map<CGRANode *, int> destllMap;

	std::vector<std::pair<CGRANode *, CGRANode *>> pathsNotRouted;
	bool success = false;

	//	CGRA localCGRA = *inCGRA;
	std::map<CGRANode *, std::vector<CGRAEdge>> localCGRAEdges = cgraEdges;

	node = it->first;
	//	possibleDests = it->second;

	bool routeComplete = false;

	std::vector<std::pair<CGRANode *, int>> PossibleDests = it->second;

	LLVM_DEBUG(dbgs() << "MapMultiDestRec : Procesing NodeIdx = " << node->getIdx());
	LLVM_DEBUG(dbgs() << ", PossibleDests = " << it->second.size());
	LLVM_DEBUG(dbgs() << ", MII = " << currCGRA->getMII());
	LLVM_DEBUG(dbgs() << ", currASAPLevel = " << node->getASAPnumber() << "/" << maxASAPLevel);
	LLVM_DEBUG(dbgs() << ", NodeProgress = " << index + 1 << "/" << nodeDestMap->size());
	LLVM_DEBUG(dbgs() << "\n");

	for (int j = 0; j < node->getAncestors().size(); ++j)
	{
		parent = node->getAncestors()[j];
		parentExt = currCGRA->getCGRANode((parent->getMappedLoc()->getT() + 1) % (currCGRA->getMII()), parent->getMappedLoc()->getY(), parent->getMappedLoc()->getX());

		LLVM_DEBUG(dbgs() << "ParentIdx=" << parent->getIdx());
		LLVM_DEBUG(dbgs() << ",ParentType=" << parent->getNameType());
		LLVM_DEBUG(dbgs() << ",ParentRT=" << parent->getmappedRealTime());
		LLVM_DEBUG(dbgs() << ",Path = "
				<< "(" << parentExt->getT() << ","
				<< parentExt->getY() << ","
				<< parentExt->getX() << ") to ...\n");
		parents.push_back(parent);
	}

	for (int i = 0; i < it->second.size(); ++i)
	{
		success = false;
		if (it->second[i].first->getmappedDFGNode() == NULL)
		{
			//			LLVM_DEBUG(dbgs() << "Possible Dest = "
			//				   << "(" << it->second[i].first->getT() << ","
			//				   	   	  << it->second[i].first->getY() << ","
			//						  << it->second[i].first->getX() << ")\n";

			cnode = it->second[i].first;
			//			cnodePair = it->second[i];
			dests.push_back(cnode);
			destllMap[cnode] = it->second[i].second;
		}
	}

	std::map<dfgNode *, std::vector<std::pair<CGRANode *, int>>>::iterator itlocal;

	while (!success)
	{
		localCGRAEdges = cgraEdges;
		routeComplete = astar->Route(node, parents, &dests, &destllMap, &localCGRAEdges, &pathsNotRouted, &chosenCnode, &deadEndReached);

		LLVM_DEBUG(dbgs() << "MapMultiDestRec::routeComplete=" << routeComplete << "\n");
		LLVM_DEBUG(dbgs() << "MapMultiDestRec::deadEndReached=" << deadEndReached << "\n");

		if (deadEndReached || !routeComplete)
		{
			return false;
		}
		assert(chosenCnode != NULL);
		mappingOutFile << "nodeIdx=" << node->getIdx() << ", placed=(" << chosenCnode->getT() << "," << chosenCnode->getY() << "," << chosenCnode->getX() << ")"
				<< "\n";
		mappingOutFile << "routing success, keeping the current edges\n";

		LLVM_DEBUG(dbgs() << "Placed = "
				<< "(" << chosenCnode->getT() << "," << chosenCnode->getY() << "," << chosenCnode->getX() << ")\n");
		node->setMappedLoc(chosenCnode);
		chosenCnode->setMappedDFGNode(node);
		node->setMappedRealTime(destllMap[chosenCnode]);

		itlocal = it;
		itlocal++;

		if (index + 1 < nodeDestMap->size())
		{
			success = MapMultiDestRec(
					&localNodeDestMap,
					&localdestNodeMap,
					itlocal,
					localCGRAEdges,
					index + 1);
		}
		else
		{
			LLVM_DEBUG(dbgs() << "nodeDestMap end reached..\n");
			*nodeDestMap = localNodeDestMap;
			*destNodeMap = localdestNodeMap;
			currCGRA->setCGRAEdges(localCGRAEdges);
			success = true;
		}

		if (!success)
		{
			LLVM_DEBUG(dbgs() << "MapMultiDestRec : fails next possible destination\n");
			node->setMappedLoc(NULL);
			chosenCnode->setMappedDFGNode(NULL);
			mappingOutFile << std::to_string(index + 1) << " :: mapping failed, therefore trying mapping again for index=" << std::to_string(index) << "\n";
			if (backtrackCounter == 0)
			{
				return false;
			}
			backtrackCounter--;
		}
	}

	backtrackCounter = std::min(initBtrack, backtrackCounter + 1);
	return true;

	//	//Old Code From Here
	//
	//	for (int i = 0; i < it->second.size(); ++i) {
	//		success = false;
	//		LLVM_DEBUG(dbgs() << "Possible Dest = "
	//			   << "(" << it->second[i].first->getT() << ","
	//			   	   	  << it->second[i].first->getY() << ","
	//					  << it->second[i].first->getX() << ")\n";
	//
	//		if(it->second[i].first->getmappedDFGNode() == NULL){
	//			LLVM_DEBUG(dbgs() << "Possible Dest is NULL\n";
	//			cnode = it->second[i].first;
	//			cnodePair = it->second[i];
	//			for (int j = 0; j < node->getAncestors().size(); ++j) {
	//				parent = node->getAncestors()[j];
	////				parentExt = currCGRA->getCGRANode(cnode->getT(),parent->getMappedLoc()->getY(),parent->getMappedLoc()->getX());
	////				parentExt = parent->getMappedLoc();
	//				parentExt = currCGRA->getCGRANode((parent->getMappedLoc()->getT() + 1)%(currCGRA->getMII()),parent->getMappedLoc()->getY(),parent->getMappedLoc()->getX());
	//
	//				LLVM_DEBUG(dbgs() << "Path = "
	//					   << "(" << parentExt->getT() << ","
	//					   	   	  << parentExt->getY() << ","
	//							  << parentExt->getX() << ") to ";
	//
	//				LLVM_DEBUG(dbgs() << "(" << cnode->getT() << ","
	//					   	   	  << cnode->getY() << ","
	//							  << cnode->getX() << ")\n";
	//
	////				paths.push_back(std::make_pair(parentExt,cnode));
	////				treePaths.push_back(createTreePath(parent,cnode));
	//				dests.push_back(cnode);
	//				parents.push_back(parent);
	//			}
	//
	//			localCGRAEdges = cgraEdges;
	//			mappingOutFile << "nodeIdx=" << node->getIdx() << ", placed=(" << cnode->getT() << "," << cnode->getY() << "," << cnode->getX() << ")" << "\n";
	////			astar->Route(node,parents,paths,&localCGRAEdges,&pathsNotRouted);
	//			astar->Route(node,parents,dests,&localCGRAEdges,&pathsNotRouted,&deadEndReached);
	//			dests.clear();
	////			paths.clear();
	//			if(!pathsNotRouted.empty()){
	//				mappingOutFile << "routing failed, clearing edges\n";
	//				LLVM_DEBUG(dbgs() << "all paths are not routed.\n";
	//				pathsNotRouted.clear();
	//				if(deadEndReached){
	//					return false;
	//				}
	//				continue;
	//			}
	//			mappingOutFile << "routing success, keeping the current edges\n";
	//			pathsNotRouted.clear();
	//
	//			for (int j = 0; j < localdestNodeMap[cnode].size(); ++j) {
	//				otherNode = localdestNodeMap[cnode][j];
	//				localNodeDestMap[otherNode].erase(std::remove(localNodeDestMap[otherNode].begin(), localNodeDestMap[otherNode].end(), cnodePair), localNodeDestMap[otherNode].end());
	//			}
	//
	////			it->second.clear();
	////			it->second.push_back(cnodePair);
	//			localdestNodeMap[cnode].clear();
	//
	//			LLVM_DEBUG(dbgs() << "Placed = " << "(" << cnode->getT() << "," << cnode->getY() << "," << cnode->getX() << ")\n";
	//			node->setMappedLoc(cnode);
	//			cnode->setMappedDFGNode(node);
	//			node->setMappedRealTime(cnodePair.second);
	//
	//			std::map<dfgNode*,std::vector< std::pair<CGRANode*,int> > >::iterator itlocal;
	//			itlocal = it;
	//			itlocal++;
	////			it++;
	//
	//			if(index + 1 < nodeDestMap->size()){
	//				success = MapMultiDestRec(
	//						&localNodeDestMap,
	//						&localdestNodeMap,
	//						itlocal,
	//						localCGRAEdges,
	//						index + 1);
	//			}
	//			else{
	//				LLVM_DEBUG(dbgs() << "nodeDestMap end reached..\n";
	//				*nodeDestMap = localNodeDestMap;
	//				*destNodeMap = localdestNodeMap;
	//				currCGRA->setCGRAEdges(localCGRAEdges);
	//				success = true;
	//			}
	//
	//			if(deadEndReached){
	//				success = false;
	//				break;
	//			}
	//
	//
	//			if(success){
	//				it->second.clear();
	//				it->second.push_back(cnodePair);
	//				break;
	//			}
	//			else{
	//				LLVM_DEBUG(dbgs() << "MapMultiDestRec : fails next possible destination\n";
	//				node->setMappedLoc(NULL);
	//				cnode->setMappedDFGNode(NULL);
	//				mappingOutFile << std::to_string(index + 1) << " :: mapping failed, therefore trying mapping again for index=" << std::to_string(index) << "\n";
	//			}
	//		}
	//	}
	//
	//	if(success){
	//
	//	}
	//	return success;
}

//bool DFG::MapMultiDest(std::map<dfgNode*, std::vector<CGRANode*> > *nodeDestMap,std::map<CGRANode*, std::vector<dfgNode*> > *destNodeMap) {
//	std::map<dfgNode*, std::vector<CGRANode*> > localNodeDestMap = *nodeDestMap;
//	std::map<CGRANode*, std::vector<dfgNode*> > localdestNodeMap = *destNodeMap;
//
//	dfgNode* node;
//	dfgNode* otherNode;
//	dfgNode* parent;
//	CGRANode* cnode;
//	CGRANode* parentExt;
//	std::vector<CGRANode*> possibleDests;
//	std::vector<std::pair<CGRANode*, CGRANode*> > paths;
//	bool success = false;
//
//	std::map<dfgNode*, std::vector<CGRANode*> > ::iterator it;
////	for (it = localNodeDestMap.begin(); it != localNodeDestMap.end(); it++){
//	it = localNodeDestMap.begin();
//
//
////	}
//
//	*nodeDestMap = localNodeDestMap;
//	*destNodeMap = localdestNodeMap;
//	return true;
//}

bool DFG::MapASAPLevel(int MII, int XDim, int YDim, ArchType arch)
{

	astar = new AStar(&mappingOutFile, MII, this);
	currCGRA = new CGRA(MII, XDim, YDim, REGS_PER_NODE, arch /*RegXbarTREG*/);

	for (int i = 0; i < subLoopDFGs.size(); ++i)
	{
		assert(currCGRA->PlaceMacro(subLoopDFGs[i]));
	}

	currCGRA->setMapped(true);

	LLVM_DEBUG(dbgs() << "STARTING MAPASAP with MII = " << MII << "with maxASAPLevel = " << maxASAPLevel << "\n");

	//	std::map<dfgNode*,std::vector<CGRANode*> > nodeDestMap;
	std::map<dfgNode *, std::vector<std::pair<CGRANode *, int>>> nodeDestMap;
	std::map<CGRANode *, std::vector<dfgNode *>> destNodeMap;
	std::map<dfgNode *, std::map<CGRANode *, int>> nodeDestCostMap;

	std::vector<dfgNode *> currLevelNodes;
	dfgNode *node;
	dfgNode *phiChild;
	dfgNode *parent;
	CGRANode *parentExt;

	for (int level = 0; level <= maxASAPLevel; ++level)
	{
		currLevelNodes.clear();
		nodeDestMap.clear();
		destNodeMap.clear();
		nodeDestCostMap.clear();

		LLVM_DEBUG(dbgs() << "level = " << level << "\n");

		for (int j = 0; j < NodeList.size(); ++j)
		{
			node = NodeList[j];

			if (node->getASAPnumber() == level)
			{
				currLevelNodes.push_back(node);
				astar->ASAPLevelNodeMap[level].push_back(node);
			}
		}

		LLVM_DEBUG(dbgs() << "numOfNodes = " << currLevelNodes.size() << "\n");

		for (int i = 0; i < currLevelNodes.size(); ++i)
		{
			node = currLevelNodes[i];
			int ll = (node->getASAPnumber() / MII) * MII - 1;
			//				int ll = node->getASAPnumber()-1;
			//				int ll=-1;
			int el = INT32_MAX;
			dfgNode *latestParent;
			for (int j = 0; j < node->getAncestors().size(); ++j)
			{
				parent = node->getAncestors()[j];

				//every parent should be mapped
				if (parent->getMappedLoc() == NULL)
				{
					LLVM_DEBUG(dbgs() << "Parent : " << parent->getIdx() << " is not mapped!, this node=" << node->getIdx() << "\n");
				}
				assert(parent->getMappedLoc() != NULL);

				if (parent->getmappedRealTime() > ll)
				{
					if (parent->getmappedRealTime() % MII != parent->getMappedLoc()->getT())
					{
						LLVM_DEBUG(dbgs() << "parent->getmappedRealTime()%MII = " << parent->getmappedRealTime() % MII << "\n");
						LLVM_DEBUG(dbgs() << "parent->getMappedLoc()->getT() = " << parent->getMappedLoc()->getT() << "\n");
					}

					assert(parent->getmappedRealTime() % MII == parent->getMappedLoc()->getT());
					//						ll = parent->getMappedLoc()->getT();
					ll = parent->getmappedRealTime();
					latestParent = parent;
				}

				//					if(parent->getmappedRealTime() < el){
				//						assert( parent->getmappedRealTime()%MII == parent->getMappedLoc()->getT() );
				//						el = parent->getMappedLoc()->getT();
				////						el = parent->getmappedRealTime();
				//					}
			}
			//				if(!node->getAncestors().empty()){
			//					ll+=node->getASAPnumber() - latestParent->getASAPnumber() - 1;
			//				}
			LLVM_DEBUG(dbgs() << "ll=" << ll << "\n");

			if (!node->getRecAncestors().empty())
			{
				LLVM_DEBUG(dbgs() << "RecAnc for Node" << node->getIdx());
				mappingOutFile << "RecAnc for Node" << node->getIdx();
			}

			for (int j = 0; j < node->getRecAncestors().size(); ++j)
			{
				parent = node->getRecAncestors()[j];

				LLVM_DEBUG(dbgs() << " (Id=" << parent->getIdx() <<

						",rt=" << parent->getmappedRealTime() << ",t=" << parent->getMappedLoc()->getT() << "),");

				mappingOutFile << " (Id=" << parent->getIdx() << ",rt=" << parent->getmappedRealTime() << ",t=" << parent->getMappedLoc()->getT() << "),";

				if (parent->getmappedRealTime() < el)
				{
					assert(parent->getmappedRealTime() % MII == parent->getMappedLoc()->getT());
					el = parent->getmappedRealTime();
				}
			}

			if (!node->getRecAncestors().empty())
			{
				LLVM_DEBUG(dbgs() << "\n");
				mappingOutFile << "\n";
				el = el % MII;
			}

			for (int var = 0; var < MII; ++var)
			{

				if (ll + 1 == el)
				{ //this is only set if reccurence ancestors are found.
					mappingOutFile << "MapASAPLevel breaking since reccurence ancestor found\n";
					break;
				}

				if (/*!node->getPHIchildren().empty()*/ false)
				{

					assert(node->getPHIchildren().size() == 1);
					phiChild = node->getPHIchildren()[0];
					assert(phiChild->getMappedLoc() != NULL);
					int y = phiChild->getMappedLoc()->getY();
					int x = phiChild->getMappedLoc()->getX();

					if (currCGRA->getCGRANode((ll + 1) % MII, y, x)->getmappedDFGNode() == NULL)
					{
						if (node->getIsMemOp() == (currCGRA->getCGRANode((ll + 1) % MII, y, x)->getPEType() == MEM))
						{

							nodeDestMap[node].push_back(std::make_pair(currCGRA->getCGRANode((ll + 1) % MII, y, x), (ll + 1)));
							destNodeMap[currCGRA->getCGRANode((ll + 1) % MII, y, x)].push_back(node);
						}
					}
				}
				else
				{

					for (int y = 0; y < YDim; ++y)
					{
						//Remove this after running mad benchmark
						if (nodeDestMap[node].size() >= 100)
						{
							break;
						}
						for (int x = 0; x < XDim; ++x)
						{
							//Remove this after running mad benchmark
							if (nodeDestMap[node].size() >= 100)
							{
								break;
							}
							if (currCGRA->getCGRANode((ll + 1) % MII, y, x)->getmappedDFGNode() == NULL)
							{
								if (node->getIsMemOp())
								{ //if current operation is a memory operation
									if (currCGRA->getCGRANode((ll + 1) % MII, y, x)->getPEType() == MEM)
									{ // only allocate PEs capable of doing memory operations

										if (node->getLeftAlignedMemOp() == 1)
										{
											if (true)
											{
												//													if(x==0){
												//													if((y==0||y==1)&&x==0){
												//													if(( (y < currCGRA->getYdim()/2) || (currCGRA->getYdim() == 1) )&&x==0){
												nodeDestMap[node].push_back(std::make_pair(currCGRA->getCGRANode((ll + 1) % MII, y, x), (ll + 1)));
												destNodeMap[currCGRA->getCGRANode((ll + 1) % MII, y, x)].push_back(node);
											}
										}
										else if (node->getLeftAlignedMemOp() == 2)
										{
											if (true)
											{
												//													if(x==currCGRA->getXdim()-1){
												//													assert(currCGRA->getYdim() > 2);
												//													if((y==2||y==3)&&x==0){
												//													if(( (y >= currCGRA->getYdim()/2) || (currCGRA->getYdim() == 1) )&&x==0){
												nodeDestMap[node].push_back(std::make_pair(currCGRA->getCGRANode((ll + 1) % MII, y, x), (ll + 1)));
												destNodeMap[currCGRA->getCGRANode((ll + 1) % MII, y, x)].push_back(node);
											}
										}
										else
										{
											nodeDestMap[node].push_back(std::make_pair(currCGRA->getCGRANode((ll + 1) % MII, y, x), (ll + 1)));
											destNodeMap[currCGRA->getCGRANode((ll + 1) % MII, y, x)].push_back(node);
										}
									}
								}
								else
								{ // for any other operations can allocate any PE.
									nodeDestMap[node].push_back(std::make_pair(currCGRA->getCGRANode((ll + 1) % MII, y, x), (ll + 1)));
									destNodeMap[currCGRA->getCGRANode((ll + 1) % MII, y, x)].push_back(node);
								}
							}
						}
					}
				}

				ll = (ll + 1);
			}
			LLVM_DEBUG(dbgs() << "MapASAPLevel:: nodeIdx=" << node->getIdx() << " ,Possible Dests = " << nodeDestMap[node].size() << "\n");
			for (int i = 0; i < nodeDestMap[node].size(); ++i)
			{
				LLVM_DEBUG(dbgs() << nodeDestMap[node][i].first->getName() << ",RT=" << nodeDestMap[node][i].second << ";");
			}
			LLVM_DEBUG(dbgs() << "\n");
		}

		LLVM_DEBUG(dbgs() << "MapASAPLevel:: Finding dests are done!\n");

		//Multiple Desination Nodes
		deadEndReached = false;
		if (!MapMultiDestRec(&nodeDestMap, &destNodeMap, nodeDestMap.begin(), *(currCGRA->getCGRAEdges()), 0))
		{
			delete currCGRA;
			for (int var = 0; var < NodeList.size(); ++var)
			{
				node = NodeList[var];
				node->setMappedLoc(NULL);
			}
			return false;
		}
		//			return true;
	}
	return true;
}

bool DFG::MapCGRA_SMART(int XDim, int YDim, ArchType arch, int bTrack, int initMII)
{

	this->initBtrack = bTrack;
	this->backtrackCounter = bTrack;
	this->setName(this->getName() + "_" + getArchName(arch) + "_" + std::to_string(XDim) + "_" + std::to_string(YDim) + "_BT" + std::to_string(initBtrack));
	std::string mapfileName = this->getName() + "_mapping.log";
	mappingOutFile.open(mapfileName.c_str());
	clock_t begin = clock();
	int MII = ceil((float)NodeList.size() / ((float)XDim * (float)YDim));
	int memMII = ceil((float)getMEMOpsToBePlaced() / ((float)YDim));
	findMaxRecDist();
	conMatArr.push_back(getConMat());

	LLVM_DEBUG(dbgs() << "MapCGRAsa:: Resource Constrained MII = " << MII << "\n");
	LLVM_DEBUG(dbgs() << "MapCGRAsa:: Recurrence Constrained MII = " << getMaxRecDist() << "\n");

	LLVM_DEBUG(dbgs() << "MapCGRAsa:: MEMNodes/TotalNodes = " << getMEMOpsToBePlaced() << "/" << NodeList.size() << "\n");
	LLVM_DEBUG(dbgs() << "MapCGRAsa:: MEM Constrained MII = " << memMII << "\n");
	mappingOutFile << "MapCGRAsa:: Number of nodes = " << NodeList.size() << ", Edges = " << edgeList.size() << "\n";
	mappingOutFile << "MapCGRAsa:: Resource Constrained MII = " << MII << "\n";
	mappingOutFile << "MapCGRAsa:: MEMNodes/TotalNodes = " << getMEMOpsToBePlaced() << "/" << NodeList.size() << "\n";
	mappingOutFile << "MapCGRAsa:: OutLoopMEMOps = " << getOutLoopMEMOps() << "\n";
	mappingOutFile << "MapCGRAsa:: MEM Constrained MII = " << memMII << "\n";
	mappingOutFile << "MapCGRAsa:: Recurrence Constrained MII = " << getMaxRecDist() << "\n";

	MII = std::max(std::max(MII, getMaxRecDist()), std::max(memMII, initMII));
	//	int initMII = MII;
	//	MII = 19;

	//Sanity Check
	dfgNode *node;
	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		if (node->getAncestors().size() > 5)
		{
			LLVM_DEBUG(dbgs() << "Cannot map applications that have Fan in nodes more than 5\n");
			return false;
		}
	}

	int latency = 0;

	while (1)
	{
		if (MapASAPLevel(MII, XDim, YDim, arch))
		{
			clock_t end = clock();
			double elapsed_time = double(end - begin) / CLOCKS_PER_SEC;
			LLVM_DEBUG(dbgs() << "MapCGRAsa :: Mapping success with MII = " << MII << "\n");

			for (int i = 0; i < NodeList.size(); ++i)
			{
				node = NodeList[i];
				if (node->getmappedRealTime() > latency)
				{
					latency = node->getmappedRealTime();
				}
			}

			mappingOutFile << "MapCGRAsa :: Mapping success with MII = " << MII << " with a latency of " << latency << "\n";

			//Printing out SMART_PATH Histogram
			mappingOutFile << "\n MapCGRAsa :: Beginning of SMART Path Histogram = \n";
			mappingOutFile << "Path Size \tNumber of Paths \tPredicatePaths\n";
			for (int i = 0; i <= astar->maxSMARTPathLength; ++i)
			{
				if (astar->SMARTPathHist.find(i) == astar->SMARTPathHist.end())
				{
					mappingOutFile << i << "\t\t\t0\t\t\t";
				}
				else
				{
					mappingOutFile << i << "\t\t\t" << astar->SMARTPathHist[i] << "\t\t\t";
				}

				if (astar->SMARTPredicatePathHist.find(i) == astar->SMARTPathHist.end())
				{
					mappingOutFile << "0\n";
				}
				else
				{
					mappingOutFile << astar->SMARTPredicatePathHist[i] << "\n";
				}
			}
			mappingOutFile << "\n MapCGRAsa :: End of SMART Path Histogram.\n\n";

			//Printing out PATH Histrogram
			mappingOutFile << "\n MapCGRAsa :: Beginning of Path Histogram = \n";
			mappingOutFile << "PathSize \tNumber of Paths \tCountRegConnections \tRegConnections \n";
			for (int i = 0; i <= astar->maxPathLength; ++i)
			{
				if (astar->PathHist.find(i) == astar->PathHist.end())
				{
					mappingOutFile << i << "\t\t\t0\n";
				}
				else
				{
					mappingOutFile << i << "\t\t\t" << astar->PathHist[i];
					mappingOutFile << "\t\t\t\t\t" << astar->PathSMARTPathCount[i];
					mappingOutFile << "\t\t\t\t\t (";
					for (int j = 0; j < astar->PathSMARTPaths[i].size(); j++)
					{
						mappingOutFile << astar->PathSMARTPaths[i][j] << ",";
					}
					mappingOutFile << ")\n";
				}
			}

			//Printing out mapped routes
			printOutSMARTRoutes();
			//			printTurns();

			std::map<CGRANode *, std::vector<CGRAEdge>> *cgraEdgesPtr = currCGRA->getCGRAEdges();
			std::vector<CGRAEdge> connections;
			int count = 0;

			mappingOutFile << "Reg Connections Available after use :: \n";

			for (int t = 0; t < currCGRA->getMII(); ++t)
			{
				for (int y = 0; y < currCGRA->getYdim(); ++y)
				{
					for (int x = 0; x < currCGRA->getXdim(); ++x)
					{
						connections = (*cgraEdgesPtr)[currCGRA->getCGRANode(t, y, x)];
						for (int c = 0; c < connections.size(); ++c)
						{
							if ((connections[c].SrcPort == R0) ||
									(connections[c].SrcPort == R1) ||
									(connections[c].SrcPort == R2) ||
									(connections[c].SrcPort == R3))
							{
								if (connections[c].mappedDFGEdge == NULL)
								{
									count++;
								}
							}
						}
						mappingOutFile << "(" << t << "," << y << "," << x << ")"
								<< "=" << count << "\n";
						count = 0;
					}
				}
			}

			mappingOutFile << "Duration :: " << elapsed_time << "\n";
			break;
		}
		LLVM_DEBUG(dbgs() << "MapCGRAsa :: Mapping failed with MII = " << MII << "\n");
		mappingOutFile << "MapCGRAsa :: Mapping failed with MII = " << MII << "\n";
		if (MII >= 25)
		{
			LLVM_DEBUG(dbgs() << "Largest MII is 25, current MII = 25 , therefore aborting mapping\n");
			return false;
		}

		double currTime = double(clock() - begin) / CLOCKS_PER_SEC;
		//TODO : DAC18 removed time limit
		//		if(currTime > 60*15){
		//			LLVM_DEBUG(dbgs() << "8 mins window expired for the compilation, exiting\n";
		//			return false;
		//		}

		MII++;
	}
	mappingOutFile.close();
	return true;
}

void DFG::MapCGRA(int XDim, int YDim)
{

	//TODO : Add recurrence constrained MII
	int MII = ceil((float)NodeList.size() / ((float)XDim * (float)YDim));
	std::vector<ConnectedCGRANode> candidateCGRANodes;
	dfgNode *temp;

	struct cand
	{
		int max = 0;
		int curr = 0;

		cand(int m, int c) : max(m), curr(c) {}
	};

	std::vector<cand> chosenCandidates;

	while (1)
	{
		LLVM_DEBUG(dbgs() << "Mapping started with MII = " << MII << "\n");
		currCGRA = new CGRA(MII, XDim, YDim, REGS_PER_NODE);
		int nodeListSequencer = 0;
		int min_nodeListSequencer = NodeList.size();

		for (int i = 0; i < NodeList.size(); ++i)
		{
			chosenCandidates.push_back(cand(0, 0));
		}

		while (1)
		{
			temp = NodeList[nodeListSequencer];
			LLVM_DEBUG(dbgs() << "Mapping " << nodeListSequencer << "/" << NodeList.size() << "..."
					<< ", with MII = " << MII << "\n");

			if (nodeListSequencer == NodeList.size())
			{
				LLVM_DEBUG(dbgs() << "Mapping done...\n");
				break;
			}

			LLVM_DEBUG(dbgs() << "Finding candidates for NodeSeq =" << nodeListSequencer << "...\n");
			candidateCGRANodes = FindCandidateCGRANodes(temp);
			LLVM_DEBUG(dbgs() << "Found candidates : " << candidateCGRANodes.size() << "\n");

			//			for (int i = 0; i < candidateCGRANodes.size(); ++i) {
			//				LLVM_DEBUG(dbgs() << "(t,y,x) = (" << candidateCGRANodes[i].node->getT() << ","
			//										<< candidateCGRANodes[i].node->getY() << ","
			//										<< candidateCGRANodes[i].node->getX() << ")\n";
			//			}

			//Remove this special Debug
			//			if(nodeListSequencer == 28){
			//				if(currCGRA->getCGRANode(2,2,1)->getmappedDFGNode() == NULL){
			//					LLVM_DEBUG(dbgs() << "SPECIAL_DEBUG : (221) is NULL\n";
			//				}
			//				else{
			//					LLVM_DEBUG(dbgs() << "SPECIAL_DEBUG : (221) is "<< currCGRA->getCGRANode(2,2,1)->getmappedDFGNode()->getIdx() <<"\n";
			//				}
			//			}

			if (candidateCGRANodes.size() == 0)
			{
				do
				{
					chosenCandidates[nodeListSequencer].curr = 0;
					nodeListSequencer--;

					if (nodeListSequencer >= 0)
					{
						backTrack(nodeListSequencer);
					}

					LLVM_DEBUG(dbgs() << "nodeListSequencer =" << nodeListSequencer << "\n");
				} while ((chosenCandidates[nodeListSequencer].curr + 1 >= chosenCandidates[nodeListSequencer].max) && (nodeListSequencer >= 0));
				LLVM_DEBUG(dbgs() << "Backtracked to nodeListSequencer =" << nodeListSequencer << "\n");

				if (nodeListSequencer < 0)
				{
					LLVM_DEBUG(dbgs() << "Mapping failed for MII = " << MII << "...\n");
					break;
				}

				chosenCandidates[nodeListSequencer].curr = chosenCandidates[nodeListSequencer].curr + 1;
			}
			else
			{
				LLVM_DEBUG(dbgs() << "current candidate = " << chosenCandidates[nodeListSequencer].curr << "\n");
				chosenCandidates[nodeListSequencer].max = candidateCGRANodes.size();
				candidateCGRANodes[chosenCandidates[nodeListSequencer].curr].node->setMappedDFGNode(temp);
				temp->setMappedLoc(candidateCGRANodes[chosenCandidates[nodeListSequencer].curr].node);

				LLVM_DEBUG(dbgs() << "Mapped node with sequence =" << nodeListSequencer << "\n");
				LLVM_DEBUG(dbgs() << "(t,y,x) = (" << candidateCGRANodes[chosenCandidates[nodeListSequencer].curr].node->getT() << ","
						<< candidateCGRANodes[chosenCandidates[nodeListSequencer].curr].node->getY() << ","
						<< candidateCGRANodes[chosenCandidates[nodeListSequencer].curr].node->getX() << ")\n");
				//				LLVM_DEBUG(dbgs() << "(t,y,x) = (" << currCGRA->getCGRANode(temp->getMappedLoc()->getT(),temp->getMappedLoc()->getY(),temp->getMappedLoc()->getX())->getT()  << ","
				//									    << currCGRA->getCGRANode(temp->getMappedLoc()->getT(),temp->getMappedLoc()->getY(),temp->getMappedLoc()->getX())->getY()   << ","
				//									    << currCGRA->getCGRANode(temp->getMappedLoc()->getT(),temp->getMappedLoc()->getY(),temp->getMappedLoc()->getX())->getX()   << ")\n";
				nodeListSequencer++;
			}
			//			delete(&candidateCGRANodes);
		}

		if (nodeListSequencer == NodeList.size())
		{
			LLVM_DEBUG(dbgs() << "Mapping done...\n");
			break;
		}

		MII++;
		delete (currCGRA);
	}
}

void DFG::traverseCriticalPath(dfgNode *node, int level)
{
	//	dfgNode* temp;
	//	int i;
	//	for (i = 0; i < node->getChildren().size(); ++i) {
	//		temp = findNode(node->getChildren()[i]);
	//		if(temp->getASAPnumber() == temp->getALAPnumber()) {
	//
	//		}
	//	}
}

void DFG::CreateSchList()
{
	dfgNode *temp;

	LLVM_DEBUG(dbgs() << "#################Begin :: The Schedule List#################\n");

	//	for (int i = 0; i < NodeList.size(); ++i) {
	//		temp = &NodeList[i];
	//		LLVM_DEBUG(dbgs() << "NodeIdx=" << temp->getIdx() << ", ASAP =" << temp->getASAPnumber() << ", ALAP =" << temp->getALAPnumber() << "\n";
	//	}
	//	LLVM_DEBUG(dbgs() << "Done\n";

	std::sort(NodeList.begin(), NodeList.end(), ScheduleOrder());
	LLVM_DEBUG(dbgs() << "CreateSchList::Done sorting...\n");
	for (int i = 0; i < NodeList.size(); ++i)
	{
		temp = NodeList[i];
		temp->setSchIdx(i);

		LLVM_DEBUG(dbgs() << "NodeIdx=" << temp->getIdx() <<  ", NameType=" << temp->getNameType() << ", ASAP =" << temp->getASAPnumber() << ", ALAP =" << temp->getALAPnumber() << ", CONST VAL =" << (temp->hasConstantVal()? temp->getConstantVal():0) << "\n");
	}
	LLVM_DEBUG(dbgs() << "#################End :: The Schedule List#################\n");
}

std::vector<ConnectedCGRANode> DFG::searchCandidates(CGRANode *mappedLoc, dfgNode *node, std::vector<std::pair<Instruction *, int>> *candidateNumbers)
{
	std::vector<ConnectedCGRANode> candidates = mappedLoc->getConnectedNodes();
	eraseAlreadyMappedNodes(&candidates);
	candidateNumbers->push_back(std::make_pair(node->getAncestors()[0]->getNode(), candidates.size()));
	dfgNode *temp;
	//		candidateNumbers[node->getAncestors()[0]] = candidates.size();

	std::vector<ConnectedCGRANode> candidates2;
	std::vector<ConnectedCGRANode> candidates3;
	bool matchFound = false;

	for (int i = 0; i < node->getAncestors().size(); ++i)
	{
		if (mappedLoc->getmappedDFGNode()->getNode() != node->getAncestors()[i]->getNode())
		{
			temp = findNode(node->getAncestors()[i]->getNode());
			assert(temp->getMappedLoc() != NULL);
			candidates2 = getConnectedCGRANodes(temp);
			//					candidates2 = temp->getMappedLoc()->getConnectedNodes();
			//					eraseAlreadyMappedNodes(&candidates2);

			candidateNumbers->push_back(std::make_pair(node->getAncestors()[i]->getNode(), candidates2.size()));

			for (int j = 0; j < candidates.size(); ++j)
			{
				for (int k = 0; k < candidates2.size(); ++k)
				{
					if (candidates[j].node == candidates2[k].node)
					{
						candidates[j].cost = candidates[j].cost + candidates2[k].cost;
						matchFound = true;
						break;
					}
				}
				if (matchFound)
				{
					candidates3.push_back(candidates[j]);
				}
				matchFound = false;
			}
			candidates = candidates3;
		}
	}
	eraseAlreadyMappedNodes(&candidates);
	return candidates;
}

void DFG::eraseAlreadyMappedNodes(std::vector<ConnectedCGRANode> *candidates)
{
	std::vector<ConnectedCGRANode>::iterator it = candidates->begin();

	while (it != candidates->end())
	{
		if (it->node->getmappedDFGNode() != NULL)
		{
			//			LLVM_DEBUG(dbgs() << "eraseAlreadyMappedNodes :: Already mapped = (" << it->node->getT() << ","
			//																	  << it->node->getY() << ","
			//																	  << it->node->getX() << ")\n";
			it = candidates->erase(it);
		}
		else
		{
			++it;
		}
	}
}

void DFG::backTrack(int nodeSeq)
{
	dfgNode *temp = NodeList[nodeSeq];
	dfgNode *anc;
	//	LLVM_DEBUG(dbgs() << "Backtrack : NodeSeq=" << nodeSeq << "\n";

	if (temp->getMappedLoc() != NULL)
	{
		//		LLVM_DEBUG(dbgs() << "(t,y,x) = (" << temp->getMappedLoc()->getT() << "," << temp->getMappedLoc()->getY() << "," << temp->getMappedLoc()->getX() << "\n";
		temp->getMappedLoc()->setMappedDFGNode(NULL);
		temp->setMappedLoc(NULL);
	}

	for (int i = 0; i < temp->getAncestors().size(); ++i)
	{
		anc = temp->getAncestors()[i];

		std::vector<CGRANode *>::iterator it = anc->getRoutingLocs()->begin();

		while (it != anc->getRoutingLocs()->end())
		{
			//			LLVM_DEBUG(dbgs() << "BackTrack :: erasing routing=(" << (*it)->getT() << ","
			//													   << (*it)->getY() << ","
			//													   << (*it)->getX() << ")\n";
			(*it)->setMappedDFGNode(NULL);
			it = anc->getRoutingLocs()->erase(it);
		}
	}
}

std::vector<ConnectedCGRANode> DFG::ExpandCandidatesAddingRoutingNodes(
		std::vector<std::pair<Instruction *, int>> *candidateNumbers)
{

	dfgNode *temp;
	std::vector<ConnectedCGRANode> candidates2;
	std::vector<std::pair<Instruction *, int>> candidateNumbersTemp;
	std::sort(candidateNumbers->begin(), candidateNumbers->end(), ValueComparer());

	//	int conMat[currCGRA->getMII()*currCGRA->getYdim()*currCGRA->getXdim()][currCGRA->getMII()*currCGRA->getYdim()*currCGRA->getXdim()];
	std::vector<std::vector<int>> conMat;

	for (int i = 0; i < currCGRA->getMII() * currCGRA->getYdim() * currCGRA->getXdim(); ++i)
	{
		std::vector<int> temp;
		for (int j = 0; j < currCGRA->getMII() * currCGRA->getYdim() * currCGRA->getXdim(); ++j)
		{
			temp.push_back(0);
		}
		conMat.push_back(temp);
	}

	std::vector<ConnectedCGRANode> phyCand;

	for (int t = 0; t < currCGRA->getMII(); ++t)
	{
		for (int y = 0; y < currCGRA->getYdim(); ++y)
		{
			for (int x = 0; x < currCGRA->getXdim(); ++x)
			{
				conMat[getConMatIdx(t, y, x)][getConMatIdx(t, y, x)] = 1;

				phyCand = currCGRA->getCGRANode(t, y, x)->getConnectedNodes();
				for (int i = 0; i < phyCand.size(); ++i)
				{
					if (phyCand[i].node->getmappedDFGNode() == NULL)
					{
						conMat[getConMatIdx(t, y, x)][getConMatIdx(phyCand[i].node->getT(), phyCand[i].node->getY(), phyCand[i].node->getX())] = 1;
					}
				}
			}
		}
	}

	for (int i = 0; i < candidateNumbers->size(); ++i)
	{
		temp = findNode((*candidateNumbers)[i].first);

		candidates2 = getConnectedCGRANodes(temp);
		std::sort(candidates2.begin(), candidates2.end(), CostComparer());

		for (int j = 0; j < candidates2.size(); ++j)
		{

			//					LLVM_DEBUG(dbgs() << "candidates2 = (" << candidates2[j].node->getT() << ","
			//							   	   	   	   	    << candidates2[j].node->getY() << ","
			//												<< candidates2[j].node->getX() << ")\n";
		}
	}
}

std::vector<ConnectedCGRANode> DFG::getConnectedCGRANodes(dfgNode *node)
{
	assert(node->getMappedLoc() != NULL);

	std::vector<ConnectedCGRANode> candidates;
	std::vector<ConnectedCGRANode> candidates2;
	candidates = node->getMappedLoc()->getConnectedNodes();
	bool found = false;

	int routingCost;
	for (int i = 0; i < node->getRoutingLocs()->size(); ++i)
	{
		found = false;
		routingCost = (*node->getRoutingLocs())[i]->getT() - node->getMappedLoc()->getT();
		if (routingCost < 0)
		{
			routingCost = routingCost + currCGRA->getMII();
		}

		candidates2 = (*node->getRoutingLocs())[i]->getConnectedNodes();

		for (int j = 0; j < candidates2.size(); ++j)
		{
			candidates2[j].cost = candidates2[j].cost + routingCost;

			found = false;
			for (int k = 0; k < candidates.size(); ++k)
			{
				if (candidates2[j].node == candidates[k].node)
				{
					found = true;
				}
			}

			if (!found)
			{
				candidates.push_back(candidates2[j]);
			}
		}
	}
	eraseAlreadyMappedNodes(&candidates);
	return candidates;
}

int DFG::getConMatIdx(int t, int y, int x)
{
	return t * currCGRA->getYdim() * currCGRA->getXdim() + y * currCGRA->getXdim() + x;
}

std::vector<ConnectedCGRANode> DFG::FindCandidateCGRANodes(dfgNode *node)
{
	dfgNode *temp;
	std::vector<ConnectedCGRANode> candidates;
	std::vector<std::pair<Instruction *, int>> candidateNumbers;
	std::vector<std::pair<Instruction *, int>> candidateNumbersLvl2;

	assert(currCGRA != NULL);

	if (node->getAncestors().size() == 0)
	{
		for (int t = 0; t < currCGRA->getMII(); ++t)
		{
			for (int y = 0; y < currCGRA->getYdim(); ++y)
			{
				for (int x = 0; x < currCGRA->getXdim(); ++x)
				{
					if (currCGRA->getCGRANode(t, y, x)->getmappedDFGNode() == NULL)
					{
						ConnectedCGRANode tempCGRANode(currCGRA->getCGRANode(t, y, x), 0, "startNodes");
						candidates.push_back(tempCGRANode);
					}
				}
			}
		}
		return candidates;
	}
	else if (node->getAncestors().size() == 1)
	{
		temp = node->getAncestors()[0];
		assert(temp->getMappedLoc() != NULL);

		candidates = getConnectedCGRANodes(temp);
		//		candidates = temp->getMappedLoc()->getConnectedNodes();

		//		for (int i = 0; i < candidates.size(); ++i) {
		//			if(candidates[i].node->getmappedDFGNode() != NULL){
		//				candidates.erase(candidates.begin() + i);
		//			}
		//		}
		eraseAlreadyMappedNodes(&candidates);

		return candidates;
	}
	else
	{ //(node->getAncestors().size() > 1
		temp = node->getAncestors()[0];

		if (temp->getMappedLoc() == NULL)
		{
			LLVM_DEBUG(dbgs() << "Unmapped ancestor : " << temp->getIdx() << "\n");
		}

		assert(temp->getMappedLoc() != NULL);

		if (temp->getMappedLoc()->getmappedDFGNode() == NULL)
		{
			LLVM_DEBUG(dbgs() << "Unmapped ancestor : " << temp->getIdx() << "\n");
		}

		assert(temp->getMappedLoc()->getmappedDFGNode() != NULL);
		candidateNumbers.clear();
		candidates = searchCandidates(temp->getMappedLoc(), node, &candidateNumbers);

		//		LLVM_DEBUG(dbgs() << "FindCandidateCGRANodes :: candidates.size()=" << candidates.size() << "\n";

		std::vector<ConnectedCGRANode> candidates2;

		if (candidates.size() == 0)
		{
			std::sort(candidateNumbers.begin(), candidateNumbers.end(), ValueComparer());
			for (int i = 0; i < candidateNumbers.size(); ++i)
			{
				temp = findNode(candidateNumbers[i].first);
				candidates2 = getConnectedCGRANodes(temp);
				//				candidates2 = temp->getMappedLoc()->getConnectedNodes();
				//				eraseAlreadyMappedNodes(&candidates2);
				std::sort(candidates2.begin(), candidates2.end(), CostComparer());

				for (int j = 0; j < candidates2.size(); ++j)
				{
					//					LLVM_DEBUG(dbgs() << "candidates2 = (" << candidates2[j].node->getT() << ","
					//							   	   	   	   	    << candidates2[j].node->getY() << ","
					//												<< candidates2[j].node->getX() << ")\n";
				}

				//search one-level with routing nodes
				for (int j = 0; j < candidates2.size(); ++j)
				{
					candidates2[j].node->setMappedDFGNode(temp);
					candidates = searchCandidates(candidates2[j].node, node, &candidateNumbersLvl2);
					if (candidates.size() == 0)
					{
						candidates2[j].node->setMappedDFGNode(NULL);
					}
					else
					{
						//						LLVM_DEBUG(dbgs() << "FindCandidateCGRANodes :: Routing node added=(" << candidates2[j].node->getT() << ","
						//																				   << candidates2[j].node->getY() << ","
						//																				   << candidates2[j].node->getX() << ")\n";
						temp->getRoutingLocs()->push_back(candidates2[j].node);
						break;
					}
				}
				if (candidates.size() != 0)
				{
					break;
				}
			}
		}

		if (candidates.size() == 0)
		{
			LLVM_DEBUG(dbgs() << "No Candidates found for NODE_ID:" << node->getIdx() << "\n");
			LLVM_DEBUG(dbgs() << "Need to backtrack...\n");
			return candidates;
		}

		std::sort(candidates.begin(), candidates.end(), CostComparer());
		return candidates;
	}
}

void DFG::MapCGRA_EMS(int XDim, int YDim, std::string mapfileName)
{
	mappingOutFile.open(mapfileName.c_str());
	clock_t begin = clock();
	int MII = ceil((float)NodeList.size() / ((float)XDim * (float)YDim));
	findMaxRecDist();
	//	conMat = getConMat();
	conMatArr.push_back(getConMat());
	printConMat(conMatArr[0]);

	LLVM_DEBUG(dbgs() << "MapCGRAsa:: Resource Constrained MII = " << MII << "\n");
	LLVM_DEBUG(dbgs() << "MapCGRAsa:: Recurrence Constrained MII = " << getMaxRecDist() << "\n");
	mappingOutFile << "MapCGRAsa:: Number of nodes = " << NodeList.size() << "\n";
	mappingOutFile << "MapCGRAsa:: Resource Constrained MII = " << MII << "\n";
	mappingOutFile << "MapCGRAsa:: Recurrence Constrained MII = " << getMaxRecDist() << "\n";

	MII = std::max(MII, getMaxRecDist());

	astar = new AStar(&mappingOutFile, MII, this);

	//Sanity Check
	dfgNode *node;
	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		if (node->getAncestors().size() > 5)
		{
			LLVM_DEBUG(dbgs() << "Cannot map applications that have Fan in nodes more than 5\n");
			return;
		}
	}

	while (1)
	{
		if (MapCGRA_EMS_ASAPLevel(MII, XDim, YDim))
		{
			clock_t end = clock();
			double elapsed_time = double(end - begin) / CLOCKS_PER_SEC;
			mappingOutFile << "Duration :: " << elapsed_time << "\n";
			mappingOutFile << "MapCGRAsa :: Mapping success with MII = " << MII << "\n";

			std::map<CGRANode *, std::vector<CGRAEdge>> *cgraEdgesPtr = currCGRA->getCGRAEdges();
			std::vector<CGRAEdge> connections;
			int count = 0;

			mappingOutFile << "Reg Connections Available after use :: \n";

			for (int t = 0; t < currCGRA->getMII(); ++t)
			{
				for (int y = 0; y < currCGRA->getYdim(); ++y)
				{
					for (int x = 0; x < currCGRA->getXdim(); ++x)
					{
						connections = (*cgraEdgesPtr)[currCGRA->getCGRANode(t, y, x)];
						for (int c = 0; c < connections.size(); ++c)
						{
							if ((connections[c].Dst->getY() == y) && (connections[c].Dst->getX() == x))
							{
								count++;
							}
						}
						mappingOutFile << "(" << t << "," << y << "," << x << ")"
								<< "=" << count << "\n";
						count = 0;
					}
				}
			}
			break;
		}

		delete (currCGRA);
		clearMapping();
		MII++;
	}
}

bool DFG::MapCGRA_EMS_ASAPLevel(int MII, int XDim, int YDim)
{
	currCGRA = new CGRA(MII, XDim, YDim, REGS_PER_NODE);

	LLVM_DEBUG(dbgs() << "STARTING MAPASAP with MII = " << MII << "with maxASAPLevel = " << maxASAPLevel << "\n");
	mappingOutFile << "STARTING MAPASAP with MII = " << MII << "with maxASAPLevel = " << maxASAPLevel << "\n";

	std::map<CGRANode *, std::vector<CGRAEdge>>::iterator edgeIt;
	for (edgeIt = currCGRA->getCGRAEdges()->begin();
			edgeIt != currCGRA->getCGRAEdges()->end();
			edgeIt++)
	{
		mappingOutFile << "DEBUG:: Node=" << edgeIt->first->getName() << ", NumberOfEdges=" << edgeIt->second.size() << std::endl;
	}

	//	currCGRA->getCGRAEdges()

	//	std::map<dfgNode*,std::vector<CGRANode*> > nodeDestMap;
	std::map<dfgNode *, std::vector<std::pair<CGRANode *, int>>> nodeDestMap;
	std::map<CGRANode *, std::vector<dfgNode *>> destNodeMap;
	std::map<dfgNode *, std::map<CGRANode *, int>> nodeDestCostMap;

	std::map<dfgNode *, std::vector<std::pair<CGRANode *, int>>>::iterator nodeDestMapIt;
	std::map<CGRANode *, std::vector<dfgNode *>>::iterator destNodeMapIt;

	std::vector<dfgNode *> currLevelNodes;
	dfgNode *node;
	int mappedRealTime;
	dfgNode *phiChild;
	dfgNode *parent;
	CGRANode *parentExt;

	CGRANode *end;
	Port endPort;

	//----Data Structures for Sorting the destination nodes based on routing --- (1)
	std::map<std::pair<CGRANode *, Port>, std::pair<CGRANode *, Port>> cameFrom;
	std::map<CGRANode *, int> costSoFar;
	CGRANode *cnode;
	std::pair<CGRANode *, CGRANode *> path;
	int cost;

	std::vector<nodeWithCost> nodesWithCost;
	//	std::vector<nodeWithCost> globalNodesWithCost;
	// End of --- (1)

	for (int level = 0; level <= maxASAPLevel; ++level)
	{

		globalNodesWithCost.clear();
		currLevelNodes.clear();
		nodeDestMap.clear();
		destNodeMap.clear();
		nodeDestCostMap.clear();

		LLVM_DEBUG(dbgs() << "level = " << level << "\n");

		for (int j = 0; j < NodeList.size(); ++j)
		{
			node = NodeList[j];

			if (node->getASAPnumber() == level)
			{
				currLevelNodes.push_back(node);
			}
		}

		LLVM_DEBUG(dbgs() << "numOfNodes = " << currLevelNodes.size() << "\n");

		for (int i = 0; i < currLevelNodes.size(); ++i)
		{
			node = currLevelNodes[i];
			int ll = 0;
			int el = INT32_MAX;
			for (int j = 0; j < node->getAncestors().size(); ++j)
			{
				parent = node->getAncestors()[j];

				//every parent should be mapped
				if (parent->getMappedLoc() == NULL)
				{
					LLVM_DEBUG(dbgs() << "parent :: nodeIdx=" << parent->getIdx() << ", ASAP=" << parent->getASAPnumber() << "\n");
					//					parent->getNode()->dump();
				}
				assert(parent->getMappedLoc() != NULL);

				if (parent->getmappedRealTime() > ll)
				{
					if (parent->getmappedRealTime() % MII != parent->getMappedLoc()->getT())
					{
						LLVM_DEBUG(dbgs() << "getMappedRealTime assertion is going to fail!\n");
						LLVM_DEBUG(dbgs() << "parent->getmappedRealTime()%MII = " << parent->getmappedRealTime() % MII << "\n");
						LLVM_DEBUG(dbgs() << "parent->getMappedLoc()->getT() = " << parent->getMappedLoc()->getT() << "\n");
					}

					assert(parent->getmappedRealTime() % MII == parent->getMappedLoc()->getT());
					//						ll = parent->getMappedLoc()->getT();
					ll = parent->getmappedRealTime();
				}

				//					if(parent->getmappedRealTime() < el){
				//						assert( parent->getmappedRealTime()%MII == parent->getMappedLoc()->getT() );
				//						el = parent->getMappedLoc()->getT();
				////						el = parent->getmappedRealTime();
				//					}
			}

			if (!node->getRecAncestors().empty())
			{
				LLVM_DEBUG(dbgs() << "RecAnc for Node" << node->getIdx());
				mappingOutFile << "RecAnc for Node" << node->getIdx();
			}

			for (int j = 0; j < node->getRecAncestors().size(); ++j)
			{
				parent = node->getRecAncestors()[j];

				LLVM_DEBUG(dbgs() << " (Id=" << parent->getIdx() << ",rt=" << parent->getmappedRealTime() << ",t=" << parent->getMappedLoc()->getT() << "),");

				mappingOutFile << " (Id=" << parent->getIdx() << ",rt=" << parent->getmappedRealTime() << ",t=" << parent->getMappedLoc()->getT() << "),";

				if (parent->getmappedRealTime() < el)
				{
					assert(parent->getmappedRealTime() % MII == parent->getMappedLoc()->getT());
					el = parent->getmappedRealTime();
				}
			}

			if (!node->getRecAncestors().empty())
			{
				LLVM_DEBUG(dbgs() << "\n");
				mappingOutFile << "\n";
				el = el % MII;
			}

			for (int var = 0; var < MII; ++var)
			{

				if (ll + 1 == el)
				{ //this is only set if reccurence ancestors are found.
					mappingOutFile << "MapASAPLevel breaking since reccurence ancestor found\n";
					break;
				}

				if (!node->getPHIchildren().empty())
				{

					assert(node->getPHIchildren().size() == 1);
					phiChild = node->getPHIchildren()[0];
					assert(phiChild->getMappedLoc() != NULL);
					int y = phiChild->getMappedLoc()->getY();
					int x = phiChild->getMappedLoc()->getX();

					if (currCGRA->getCGRANode((ll + 1) % MII, y, x)->getmappedDFGNode() == NULL)
					{
						nodeDestMap[node].push_back(std::make_pair(currCGRA->getCGRANode((ll + 1) % MII, y, x), (ll + 1)));
						destNodeMap[currCGRA->getCGRANode((ll + 1) % MII, y, x)].push_back(node);
					}
				}
				else
				{

					for (int y = 0; y < YDim; ++y)
					{
						for (int x = 0; x < XDim; ++x)
						{
							if (currCGRA->getCGRANode((ll + 1) % MII, y, x)->getmappedDFGNode() == NULL)
							{
								nodeDestMap[node].push_back(std::make_pair(currCGRA->getCGRANode((ll + 1) % MII, y, x), (ll + 1)));
								destNodeMap[currCGRA->getCGRANode((ll + 1) % MII, y, x)].push_back(node);
							}
						}
					}
				}

				ll = (ll + 1);
			}
			LLVM_DEBUG(dbgs() << "MapASAPLevel:: nodeIdx=" << node->getIdx() << " ,Possible Dests = " << nodeDestMap[node].size() << "\n");
		}

		LLVM_DEBUG(dbgs() << "MapASAPLevel:: Finding dests are done!\n");

		//Sorting the nodeDestMap based routing/affinity costs
		for (nodeDestMapIt = nodeDestMap.begin(); nodeDestMapIt != nodeDestMap.end(); nodeDestMapIt++)
		{
			node = nodeDestMapIt->first;
			nodesWithCost.clear();
			for (int i = 0; i < nodeDestMapIt->second.size(); ++i)
			{
				cnode = nodeDestMapIt->second[i].first;
				mappedRealTime = nodeDestMapIt->second[i].second;

				cost = 0;
				for (int j = 0; j < node->getAncestors().size(); ++j)
				{
					parent = node->getAncestors()[j];
					parentExt = parent->getMappedLoc();
					//					parentExt = currCGRA->getCGRANode((parent->getMappedLoc()->getT() + 1)%(currCGRA->getMII()),parent->getMappedLoc()->getY(),parent->getMappedLoc()->getX());
					path = std::make_pair(parentExt, cnode);

					//Calculate cost for the path
					end = astar->AStarSearchEMS(*(currCGRA->getCGRAEdges()), parentExt, cnode, &cameFrom, &costSoFar, &endPort);
					cost += costSoFar[cnode];
				}
				nodesWithCost.push_back(nodeWithCost(node, cnode, cost, mappedRealTime));
			}
			std::sort(nodesWithCost.begin(), nodesWithCost.end(), LessThanNodeWithCost());
			globalNodesWithCost.push_back(nodesWithCost[0]);
			nodeDestMap[node].clear();
			for (std::vector<nodeWithCost>::iterator it = nodesWithCost.begin();
					it != nodesWithCost.end();
					it++)
			{
				nodeDestMap[node].push_back(std::make_pair(it->cnode, it->mappedRealTime));
			}
			assert(nodeDestMap[node].size() != 0);
			//TODO : Implement affinity cost
		}

		std::sort(globalNodesWithCost.begin(), globalNodesWithCost.end(), LessThanNodeWithCost());

		deadEndReached = false;
		if (!MAPCGRA_EMS_MultDest(&nodeDestMap, &destNodeMap, globalNodesWithCost.begin(), *(currCGRA->getCGRAEdges()), 0))
		{
			return false;
		}

	} // End of ASAP level iterations
	return true;
}

bool DFG::MAPCGRA_EMS_MultDest(std::map<dfgNode *, std::vector<std::pair<CGRANode *, int>>> *nodeDestMap,
		std::map<CGRANode *, std::vector<dfgNode *>> *destNodeMap,
		//							   std::map<dfgNode*,std::vector< std::pair<CGRANode*,int> > >::iterator it,
		const std::vector<nodeWithCost>::iterator it,
		std::map<CGRANode *, std::vector<CGRAEdge>> cgraEdges,
		int index)
{

	std::map<dfgNode *, std::vector<std::pair<CGRANode *, int>>> localNodeDestMap = *nodeDestMap;
	std::map<CGRANode *, std::vector<dfgNode *>> localdestNodeMap = *destNodeMap;

	dfgNode *node;
	dfgNode *otherNode;
	dfgNode *parent;
	CGRANode *cnode;
	std::pair<CGRANode *, int> cnodePair;
	CGRANode *parentExt;
	//	std::vector< std::pair<CGRANode*,int> > possibleDests;
	std::vector<dfgNode *> parents;
	//	std::vector<std::pair<CGRANode*, CGRANode*> > paths;
	std::vector<CGRANode *> dests;
	std::vector<std::pair<CGRANode *, CGRANode *>> pathsNotRouted;
	bool success = false;
	//	int idx = index;

	//	CGRA localCGRA = *inCGRA;
	std::map<CGRANode *, std::vector<CGRAEdge>> localCGRAEdges = cgraEdges;

	node = it->node;
	//	possibleDests = it->second;

	//	if(node == NULL){
	//		LLVM_DEBUG(dbgs() << "Node is NULL...\n";
	//	}else{
	//		LLVM_DEBUG(dbgs() << "Node = \n";
	//		node->getNode()->dump();
	//	}

	//	LLVM_DEBUG(dbgs() << "MapMultiDestRec : Procesing NodeIdx = " << node->getIdx() << " ,PossibleDests = " << (*nodeDestMap)[node].size() << "\n";

	LLVM_DEBUG(dbgs() << "EMSMapMultiDestRec : Procesing NodeIdx = " << node->getIdx());
	LLVM_DEBUG(dbgs() << ", PossibleDests = " << (*nodeDestMap)[node].size());
	LLVM_DEBUG(dbgs() << ", MII = " << currCGRA->getMII());
	LLVM_DEBUG(dbgs() << ", currASAPLevel = " << node->getASAPnumber());
	LLVM_DEBUG(dbgs() << ", NodeProgress = " << index + 1 << "/" << nodeDestMap->size());
	LLVM_DEBUG(dbgs() << "\n");

	for (int i = 0; i < (*nodeDestMap)[node].size(); ++i)
	{
		success = false;

		if ((*nodeDestMap)[node][i].first->getmappedDFGNode() == NULL)
		{
			LLVM_DEBUG(dbgs() << "Possible Dest = "
					<< "(" << (*nodeDestMap)[node][i].first->getT() << ","
					<< (*nodeDestMap)[node][i].first->getY() << ","
					<< (*nodeDestMap)[node][i].first->getX() << "), Index=" << i + 1 << "/" << (*nodeDestMap)[node].size() << "\n");
			cnode = (*nodeDestMap)[node][i].first;
			cnodePair = (*nodeDestMap)[node][i];
			for (int j = 0; j < node->getAncestors().size(); ++j)
			{
				parent = node->getAncestors()[j];
				//				parentExt = currCGRA->getCGRANode(cnode->getT(),parent->getMappedLoc()->getY(),parent->getMappedLoc()->getX());
				parentExt = parent->getMappedLoc();
				//				parentExt = currCGRA->getCGRANode((parent->getMappedLoc()->getT() + 1)%(currCGRA->getMII()),parent->getMappedLoc()->getY(),parent->getMappedLoc()->getX());

				//				LLVM_DEBUG(dbgs() << "Path = "
				//					   << "(" << parentExt->getT() << ","
				//					   	   	  << parentExt->getY() << ","
				//							  << parentExt->getX() << ") to ";
				//
				//				LLVM_DEBUG(dbgs() << "(" << cnode->getT() << ","
				//					   	   	  << cnode->getY() << ","
				//							  << cnode->getX() << ")\n";

				//				paths.push_back(std::make_pair(parentExt,cnode));
				dests.push_back(cnode);
				parents.push_back(parent);
			}

			localCGRAEdges = cgraEdges;
			mappingOutFile << "nodeIdx=" << node->getIdx() << ", placed=(" << cnode->getT() << "," << cnode->getY() << "," << cnode->getX() << ")"
					<< "\n";
			//			astar->EMSRoute(parents,paths,&localCGRAEdges,&pathsNotRouted);
			astar->EMSRoute(node, parents, dests, &localCGRAEdges, &pathsNotRouted, &deadEndReached);
			dests.clear();
			if (!pathsNotRouted.empty())
			{
				for (int j = 0; j < parents.size(); ++j)
				{
					for (int k = 0; k < parents[j]->getRoutingLocs()->size(); ++k)
					{
						(*parents[j]->getRoutingLocs())[k]->setMappedDFGNode(NULL);
					}
					parents[j]->getRoutingLocs()->clear();
				}
				mappingOutFile << "routing failed, clearing edges\n";
				LLVM_DEBUG(dbgs() << "all paths are not routed.\n");
				pathsNotRouted.clear();
				if (deadEndReached)
				{
					return false;
				}
				continue;
			}
			mappingOutFile << "routing success, keeping the current edges\n";
			pathsNotRouted.clear();

			for (int j = 0; j < localdestNodeMap[cnode].size(); ++j)
			{
				otherNode = localdestNodeMap[cnode][j];
				localNodeDestMap[otherNode].erase(std::remove(localNodeDestMap[otherNode].begin(), localNodeDestMap[otherNode].end(), cnodePair), localNodeDestMap[otherNode].end());
			}

			//			it->second.clear();
			//			it->second.push_back(cnodePair);
			localdestNodeMap[cnode].clear();

			LLVM_DEBUG(dbgs() << "Placed = "
					<< "(" << cnode->getT() << "," << cnode->getY() << "," << cnode->getX() << ")\n");
			node->setMappedLoc(cnode);
			cnode->setMappedDFGNode(node);
			node->setMappedRealTime(cnodePair.second);

			std::vector<nodeWithCost>::iterator itlocal;
			itlocal = it;
			itlocal++;
			//			it++;

			//			idx++;

			if (index + 1 < nodeDestMap->size())
			{
				if (!EMSSortNodeDest(&localNodeDestMap, localCGRAEdges, index + 1))
				{
					LLVM_DEBUG(dbgs() << "&& EMSSortNodeDest Done and Some nodes does not have destination retrying...\n");
					success = false;
				}
				else
				{
					itlocal = globalNodesWithCost.begin() + index + 1;
					LLVM_DEBUG(dbgs() << "&& EMSSortNodeDest Done \n");
					success = MAPCGRA_EMS_MultDest(
							&localNodeDestMap,
							&localdestNodeMap,
							itlocal,
							localCGRAEdges,
							index + 1);
				}
			}
			else
			{
				LLVM_DEBUG(dbgs() << "nodeDestMap end reached..\n");
				*nodeDestMap = localNodeDestMap;
				*destNodeMap = localdestNodeMap;
				currCGRA->setCGRAEdges(localCGRAEdges);
				success = true;
			}

			if (deadEndReached)
			{
				success = false;
				break;
			}

			if (success)
			{
				(*nodeDestMap)[node].clear();
				(*nodeDestMap)[node].push_back(cnodePair);
				break;
			}
			else
			{
				LLVM_DEBUG(dbgs() << "MapMultiDestRec : fails next possible destination\n");
				node->setMappedLoc(NULL);
				cnode->setMappedDFGNode(NULL);
				mappingOutFile << std::to_string(index + 1) << " :: mapping failed, therefore trying mapping again for index=" << std::to_string(index) << "\n";
			}
		}
	}

	if (success)
	{
	}
	return success;
}

void DFG::clearMapping()
{
	dfgNode *node;
	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		node->setMappedLoc(NULL);
		node->setMappedRealTime(-1);
		node->getRoutingLocs()->clear();
	}
}

TreePath DFG::createTreePath(dfgNode *parent, CGRANode *dest)
{
	TreePath tp;
	dfgNode *child;
	CGRANode *ParentExt;
	CGRANode *cnode;
	int MII = currCGRA->getMII();

	//Initial node, AKA the source
	assert(parent->getMappedLoc() != NULL);

	if (parent->getMappedLoc()->getPEType() == MEM)
	{
		ParentExt = currCGRA->getCGRANode((parent->getMappedLoc()->getT() + 2) % MII, parent->getMappedLoc()->getY(), parent->getMappedLoc()->getX());
		tp.sourceSCpathTdimLengths[ParentExt] = 2;
	}
	else
	{
		ParentExt = currCGRA->getCGRANode((parent->getMappedLoc()->getT() + 1) % MII, parent->getMappedLoc()->getY(), parent->getMappedLoc()->getX());
		tp.sourceSCpathTdimLengths[ParentExt] = 1;
	}
	tp.sources.push_back(ParentExt);
	tp.sourcePorts[ParentExt] = TILE;
	tp.sourcePaths[ParentExt] = (std::make_pair(parent, parent));
	tp.sourceSCpathLengths[ParentExt] = 0;

	//	if(parent->getIdx() == 22){
	//		LLVM_DEBUG(dbgs() << "createTreePath::Parent=" << parent->getIdx() << "\n";
	//		LLVM_DEBUG(dbgs() << "createTreePath::ParentExt=" << ParentExt->getName() << "\n";
	//	}

	assert(parent != NULL);

	tp.dest = dest;
	bool foundParentTreeBasedRoutingLocs = false;

	for (int i = 0; i < parent->getChildren().size(); ++i)
	{
		child = parent->getChildren()[i];
		if (child->getMappedLoc() != NULL)
		{
			foundParentTreeBasedRoutingLocs = false;
			if ((*child->getTreeBasedRoutingLocs()).find(parent) != (*child->getTreeBasedRoutingLocs()).end())
			{
				for (int j = 0; j < (*child->getTreeBasedRoutingLocs())[parent].size(); ++j)
				{
					cnode = (*child->getTreeBasedRoutingLocs())[parent][j]->cnode;
					tp.sources.push_back(cnode);
					tp.sourcePorts[cnode] = (*child->getTreeBasedRoutingLocs())[parent][j]->lastPort;

					//					LLVM_DEBUG(dbgs() << "getTreeBasedRoutingLocs read :: ";
					//					LLVM_DEBUG(dbgs() << ", currNode=" << child->getIdx();
					//					LLVM_DEBUG(dbgs() << ", currParent=" << parent->getIdx();
					//					LLVM_DEBUG(dbgs() << ", cnode=" << cnode->getName();
					//					LLVM_DEBUG(dbgs() << ", port=" << getCGRA()->getPortName(tp.sourcePorts[cnode]) << "\n";
					//					LLVM_DEBUG(dbgs() << ", pathLength=" << (*child->getTreeBasedRoutingLocs())[parent][j]->SCpathLength << "\n";
					assert(std::string("INV").compare(getCGRA()->getPortName(tp.sourcePorts[cnode])) != 0);

					tp.sourceSCpathLengths[cnode] = (*child->getTreeBasedRoutingLocs())[parent][j]->SCpathLength;
					tp.sourceSCpathTdimLengths[cnode] = (*child->getTreeBasedRoutingLocs())[parent][j]->TdimPathLength;
					if (cnode == ParentExt)
					{
						tp.sourcePaths[cnode] = (std::make_pair(parent, parent));
					}
					else
					{
						tp.sourcePaths[cnode] = (std::make_pair(parent, child));
					}
					foundParentTreeBasedRoutingLocs = true;
				}
			}

			if ((*child->getTreeBasedGoalLocs()).find(parent) != (*child->getTreeBasedGoalLocs()).end())
			{
				for (int j = 0; j < (*child->getTreeBasedGoalLocs())[parent].size(); ++j)
				{
					cnode = (*child->getTreeBasedGoalLocs())[parent][j]->cnode;
					tp.sources.push_back(cnode);
					tp.sourcePorts[cnode] = (*child->getTreeBasedGoalLocs())[parent][j]->lastPort;
					assert(std::string("INV").compare(getCGRA()->getPortName(tp.sourcePorts[cnode])) != 0);
					tp.sourceSCpathLengths[cnode] = (*child->getTreeBasedGoalLocs())[parent][j]->SCpathLength;
					tp.sourceSCpathTdimLengths[cnode] = (*child->getTreeBasedGoalLocs())[parent][j]->TdimPathLength;
					if (cnode == ParentExt)
					{
						tp.sourcePaths[cnode] = (std::make_pair(parent, parent));
					}
					else
					{
						tp.sourcePaths[cnode] = (std::make_pair(parent, child));
					}
					foundParentTreeBasedRoutingLocs = true;
				}
			}

			if (!foundParentTreeBasedRoutingLocs)
			{
				LLVM_DEBUG(dbgs() << "foundParentTreeBasedRoutingLocs is false!\n");
				LLVM_DEBUG(dbgs() << "child=" << child->getIdx() << "\n");
				LLVM_DEBUG(dbgs() << "childLoc=" << child->getMappedLoc()->getName() << "\n");
				LLVM_DEBUG(dbgs() << "parent=" << parent->getIdx() << "\n");
			}

			assert(tp.sourcePaths[cnode].first != NULL);
			assert(tp.sourcePaths[cnode].second != NULL);
		}
	}

	return tp;
}

//int DFG::findUtilTreeRoutingLocs(CGRANode* cnode, dfgNode* currNode) {
//	dfgNode* node;
//	std::map<dfgNode*,std::vector<std::pair<CGRANode*,Port> >>::iterator treeRoutingLocsIt;
//	std::vector<std::pair<CGRANode*,Port> > cnodes;
//	int util = 0;
//
//	for (int i = 0; i < NodeList.size(); ++i) {
//		node = NodeList[i];
//
//		if( (node->getMappedLoc() != NULL) || (node == currNode)/*&&(node->getASAPnumber() <= currNode->getASAPnumber())*/){
//
//			if(node->getMappedLoc() == cnode){
//				continue;
//			}
//
//			for(treeRoutingLocsIt = node->getTreeBasedRoutingLocs()->begin();
//				treeRoutingLocsIt != node->getTreeBasedRoutingLocs()->end();
//				treeRoutingLocsIt++){
//
//				cnodes = treeRoutingLocsIt->second;
//				if(std::find(cnodes.begin(),cnodes.end(),cnode) != cnodes.end()){
//					util++;
//				}
//
//
//			}
//		}
//	}
//	return util;
//}

void DFG::printOutSMARTRoutes()
{
	dfgNode *node;
	dfgNode *parent;
	dfgNode *origNode;
	dfgNode *origParent;

	std::vector<std::ofstream> outFiles(currCGRA->getYdim() * currCGRA->getXdim());
	std::ofstream *currOutFile;
	CGRANode *cnode;
	CGRANode *routingCnode;
	CGRANode *parentExt;
	std::map<int, std::vector<std::string>> logEntries;
	bool noRouting = true;
	int MII = currCGRA->getMII();
	std::pair<dfgNode *, dfgNode *> sourcePath;

	int pT = -1;
	int pY = -1;
	int pX = -1;

	//Sanity check
	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		if (node->getMappedLoc() == NULL)
		{
			LLVM_DEBUG(dbgs() << "printOutSMARTRoutes :: All the nodes are not mapped!\n");
			return;
		}
	}

	for (int y = 0; y < currCGRA->getYdim(); ++y)
	{
		for (int x = 0; x < currCGRA->getXdim(); ++x)
		{
			std::string tempOutFileName = name + "_SMRT_" + std::to_string(y) + "_" + std::to_string(x) + ".log";
			outFiles[convertToPhyLoc(y, x)].open(tempOutFileName.c_str());
		}
	}

	int routeStart = 0;
	int k;
	//Generate Log Entries
	for (int t = 0; t < currCGRA->getMII(); ++t)
	{
		for (int y = 0; y < currCGRA->getYdim(); ++y)
		{
			for (int x = 0; x < currCGRA->getXdim(); ++x)
			{
				cnode = currCGRA->getCGRANode(t, y, x);
				if (cnode->getmappedDFGNode() != NULL)
				{
					node = cnode->getmappedDFGNode();
					origNode = node;
					if (node->getMappedLoc() == cnode)
					{ // node is not just a routing location
						for (int i = 0; i < node->getAncestors().size(); ++i)
						{
							parent = node->getAncestors()[i];
							origParent = parent;

							if (parent->getMappedLoc()->getPEType() == MEM)
							{
								parentExt = currCGRA->getCGRANode((parent->getMappedLoc()->getT() + 2) % MII, parent->getMappedLoc()->getY(), parent->getMappedLoc()->getX());
							}
							else
							{
								parentExt = currCGRA->getCGRANode((parent->getMappedLoc()->getT() + 1) % MII, parent->getMappedLoc()->getY(), parent->getMappedLoc()->getX());
							}

							routeStart = 0;
							std::string strEntry;
							routingCnode = node->getMappedLoc();
							LLVM_DEBUG(dbgs() << "ParentExt=" << parentExt->getName() << "\n");
							LLVM_DEBUG(dbgs() << "FinalDest=" << routingCnode->getName() << "\n");
							LLVM_DEBUG(dbgs() << "FinalDestNodeIdx=" << node->getIdx() << "\n");
							LLVM_DEBUG(dbgs() << "FinalParentNodeIdx=" << parent->getIdx() << "\n");
							LLVM_DEBUG(dbgs() << "ParentsSize=" << node->getAncestors().size() << "\n");
							do
							{
								assert(parent->getMappedLoc() != NULL);
								assert(node->getMappedLoc() != NULL);
								//							strEntry = node->getMappedLoc()->getNameWithOutTime();
								//							routingCnode = node->getMappedLoc();
								//							noRouting = true;
								for (int j = routeStart; j < node->getMergeRoutingLocs()[parent].size(); ++j)
								{
									LLVM_DEBUG(dbgs() << "routePath = " << node->getMergeRoutingLocs()[parent][j]->cnode->getName() << "\n");
									//									if(routingCnode->getT() != node->getMergeRoutingLocs()[parent][j]->getT()){
									//										if(!noRouting){
									//											pT = routingCnode->getT();
									//											pY = routingCnode->getY();
									//											pX = routingCnode->getX();
									//
									//											logEntries[convertToPhyLoc(pT,pY,pX)].push_back(strEntry);
									//										}
									//										routingCnode = node->getMergeRoutingLocs()[parent][j];
									//										noRouting = true;
									//										strEntry =  routingCnode->getNameWithOutTime() + " <-- " ;
									//									}else{
									routingCnode = node->getMergeRoutingLocs()[parent][j]->cnode;
									//										noRouting = false;
									strEntry = strEntry + routingCnode->getName() + " <-- ";
									nodeRouteMap[origNode][origParent].push_back(routingCnode);
									//									}
								}

								//								if(!noRouting){
								//									pT = routingCnode->getT();
								//									pY = routingCnode->getY();
								//									pX = routingCnode->getX();
								//
								//									logEntries[convertToPhyLoc(pT,pY,pX)].push_back(strEntry);
								//								}

								sourcePath = (*node->getSourceRoutingPath())[parent];
								node = sourcePath.second;
								parent = sourcePath.first;
								//								parentExt = currCGRA->getCGRANode((parent->getMappedLoc()->getT()+1)%MII,parent->getMappedLoc()->getY(),parent->getMappedLoc()->getX());

								if (node != parent)
								{
									for (k = 0; k < node->getMergeRoutingLocs()[parent].size(); ++k)
									{
										LLVM_DEBUG(dbgs() << "pathNode =" << node->getMergeRoutingLocs()[parent][k]->cnode->getName() << "\n");
										if (node->getMergeRoutingLocs()[parent][k]->cnode == routingCnode)
										{
											routeStart = k + 1;
											break;
										}
									}
									LLVM_DEBUG(dbgs() << "routingCnode = " << routingCnode->getName() << "\n");
									LLVM_DEBUG(dbgs() << "sourcePathNodeIdx = " << node->getIdx() << "\n");
									LLVM_DEBUG(dbgs() << "sourcePathParentIdx = " << parent->getIdx() << "\n");
									LLVM_DEBUG(dbgs() << "sourcePathParentExtLoc = " << parentExt->getName() << "\n");
									LLVM_DEBUG(dbgs() << "sourcePathNodeLoc = " << node->getMappedLoc()->getName() << "\n");
									LLVM_DEBUG(dbgs() << "sourcePathParentLoc = " << parent->getMappedLoc()->getName() << "\n");

									LLVM_DEBUG(dbgs() << "node->getMergeRoutingLocs()[parent].size() = " << node->getMergeRoutingLocs()[parent].size() << "\n");

									assert(routeStart == k + 1);
								}

							} while (node != parent);
							assert(routingCnode == parentExt);
							LLVM_DEBUG(dbgs() << "%% Path Mapping Done ! \n");

							//							if(!noRouting){
							pT = cnode->getT();
							pY = cnode->getY();
							pX = cnode->getX();

							logEntries[convertToPhyLoc(pT, pY, pX)].push_back(strEntry);
							//							}

							node = cnode->getmappedDFGNode();
						}
					}
					else
					{ // node is just a routing location, this will not happen in SMART based routing
						logEntries[convertToPhyLoc(t, y, x)].push_back("ROUTING ONLY");
					}
				}
				else
				{
					logEntries[convertToPhyLoc(t, y, x)].push_back("NIL");
				}
			}
		}
	}

	//Writeout Log Entries
	for (int t = 0; t < currCGRA->getMII(); ++t)
	{
		for (int y = 0; y < currCGRA->getYdim(); ++y)
		{
			for (int x = 0; x < currCGRA->getXdim(); ++x)
			{
				for (int i = 0; i < logEntries[convertToPhyLoc(t, y, x)].size(); ++i)
				{
					outFiles[convertToPhyLoc(y, x)] << std::to_string(t) << "," << logEntries[convertToPhyLoc(t, y, x)][i] << std::endl;
				}
			}
		}
	}

	for (int i = 0; i < outFiles.size(); ++i)
	{
		outFiles[i].close();
	}

	std::ofstream pathLengthFile;
	std::string pathLengthFileName = name + "_routelengthstats.csv";
	pathLengthFile.open(pathLengthFileName.c_str());

	//Print Header
	pathLengthFile << "Node,Parent,Route,RouteLength,dT(RouteLength)/MII" << std::endl;

	std::map<dfgNode *, std::map<dfgNode *, std::vector<CGRANode *>>>::iterator nodeIt;
	std::map<dfgNode *, std::vector<CGRANode *>>::iterator ParentIt;

	for (nodeIt = nodeRouteMap.begin();
			nodeIt != nodeRouteMap.end();
			nodeIt++)
	{

		node = nodeIt->first;
		for (ParentIt = nodeRouteMap[node].begin();
				ParentIt != nodeRouteMap[node].end();
				ParentIt++)
		{

			parent = ParentIt->first;

			pathLengthFile << std::to_string(node->getIdx()) << ",";
			pathLengthFile << std::to_string(parent->getIdx()) << ",";

			assert(!nodeRouteMap[node][parent].empty());
			int endTime = nodeRouteMap[node][parent][0]->getT();
			int revisits = 0;

			for (int i = 0; i < nodeRouteMap[node][parent].size(); ++i)
			{
				pathLengthFile << nodeRouteMap[node][parent][i]->getNameSp() << "<--";

				if (i != 0)
				{
					if (nodeRouteMap[node][parent][i]->getT() == endTime)
					{
						if (nodeRouteMap[node][parent][i - 1]->getT() != nodeRouteMap[node][parent][i]->getT())
						{
							revisits++;
						}
					}
				}
			}
			pathLengthFile << ",";

			pathLengthFile << nodeRouteMap[node][parent].size() << ",";
			pathLengthFile << revisits << std::endl;
			//			pathLengthFile << nodeRouteMap[node][parent].size()/currCGRA->getMII() << std::endl;
		}
		pathLengthFile << std::endl;
	}
	pathLengthFile.close();
	analyzeRTpaths();
}

int DFG::convertToPhyLoc(int t, int y, int x)
{
	return t * currCGRA->getYdim() * currCGRA->getXdim() + y * currCGRA->getXdim() + x;
}

int DFG::convertToPhyLoc(int y, int x)
{
	return y * currCGRA->getXdim() + x;
}

//This function should be called on the incremented pointers, i.e. Nodes upto index are mapped
//This function should reorder nodeDestMap, i.e. nodes that are not yet mapped considering the tree based routing
bool DFG::EMSSortNodeDest(
		std::map<dfgNode *, std::vector<std::pair<CGRANode *, int>>> *nodeDestMap,
		std::map<CGRANode *, std::vector<CGRAEdge>> cgraEdges,
		int index)
{

	//Nodes upto index are mapped
	std::vector<nodeWithCost>::iterator it = globalNodesWithCost.begin() + index;
	std::vector<nodeWithCost>::iterator localtItPrev;
	std::vector<nodeWithCost>::iterator localtItForward;
	dfgNode *node = it->node;
	int affCost = -1;
	int routeCost = -1;
	std::vector<nodeWithCost> localNodesWithCost;
	std::map<dfgNode *, std::vector<std::pair<CGRANode *, int>>> newNodeDestMap;
	CGRANode *cnode;
	int mappedRealTime = -1;
	bool result = true;

	//Loop through mapped nodes
	for (localtItPrev = it - 1; localtItPrev != it; localtItPrev++)
	{
		//Assumption : All nodes are mapped until this point
		assert(localtItPrev->node->getMappedLoc() != NULL);
		LLVM_DEBUG(dbgs() << "localtItPrev, NodeIdx = " << localtItPrev->node->getIdx() << "\n");

		for (localtItForward = it; localtItForward != globalNodesWithCost.end(); localtItForward++)
		{
			LLVM_DEBUG(dbgs() << "localtItForward, NodeIdx = " << localtItForward->node->getIdx() << "\n");
			if (localtItForward->node->getASAPnumber() == 16)
			{
				LLVM_DEBUG(dbgs() << "getAffinityCost started.\n");
			}
			affCost = getAffinityCost(localtItForward->node, localtItPrev->node);
			//TODO :removing affcost for now, add it later
			//			affCost = 0;
			if (localtItForward->node->getASAPnumber() == 16)
			{
				LLVM_DEBUG(dbgs() << "getAffinityCost done.\n");
				LLVM_DEBUG(dbgs() << "(*nodeDestMap)[localtItForward->node].size = " << (*nodeDestMap)[localtItForward->node].size() << "\n");
			}

			localNodesWithCost.clear();
			for (int j = 0; j < (*nodeDestMap)[localtItForward->node].size(); ++j)
			{
				if (localtItForward->node->getASAPnumber() == 16)
				{
					LLVM_DEBUG(dbgs() << "(*nodeDestMap)[localtItForward->node], j = " << j << "\n");
				}

				cnode = (*nodeDestMap)[localtItForward->node][j].first;
				mappedRealTime = (*nodeDestMap)[localtItForward->node][j].second;

				routeCost = getStaticRoutingCost(localtItForward->node, cnode, cgraEdges);
				if (routeCost == INT_MAX)
				{   //Not Routed
					//					localNodesWithCost.push_back(nodeWithCost(localtItForward->node,cnode,INT_MAX,mappedRealTime));
					//					localNodesWithCost[localNodesWithCost.size()-1].affinityCost = 0;
					//					localNodesWithCost[localNodesWithCost.size()-1].routingCost = routeCost;
				}
				else
				{
					affCost = affCost * getDistCGRANodes(cnode, localtItPrev->node->getMappedLoc());
					localNodesWithCost.push_back(nodeWithCost(localtItForward->node, cnode, routeCost + affCost, mappedRealTime));
					localNodesWithCost[localNodesWithCost.size() - 1].affinityCost = affCost;
					localNodesWithCost[localNodesWithCost.size() - 1].routingCost = routeCost;
				}
			}
			//			assert(localNodesWithCost.size() == (*nodeDestMap)[localtItForward->node].size());
			if (localNodesWithCost.size() == 0)
			{
				result = false;
			}
			else
			{
				std::sort(localNodesWithCost.begin(), localNodesWithCost.end(), LessThanNodeWithCost());
				(*nodeDestMap)[localtItForward->node].clear();
				for (int j = 0; j < localNodesWithCost.size(); ++j)
				{
					(*nodeDestMap)[localtItForward->node].push_back(std::make_pair(localNodesWithCost[j].cnode, localNodesWithCost[j].mappedRealTime));
				}
				assert(localNodesWithCost.size() != 0);
				localtItForward->cost = localNodesWithCost[0].cost;
				LLVM_DEBUG(dbgs() << "routeCost=" << localNodesWithCost[0].routingCost << ", affCost=" << localNodesWithCost[0].affinityCost << ", Total=" << localNodesWithCost[0].cost << "\n");
			}
		}
	}
	std::sort(it, globalNodesWithCost.end(), LessThanNodeWithCost());
	return result;
}

int DFG::getDistCGRANodes(CGRANode *a, CGRANode *b)
{
	int aX = a->getX();
	int aY = a->getY();
	int aT = a->getT();

	int bX = b->getX();
	int bY = b->getY();
	int bT = b->getT();

	return abs(aX - bX) + abs(aY - bY) + abs(aT - bT);
}

void DFG::printConMat(std::vector<std::vector<unsigned char>> conMat)
{
	LLVM_DEBUG(dbgs() << "Printing ConMat....\n");
	for (int i = 0; i < conMat.size(); ++i)
	{
		for (int j = 0; j < conMat[i].size(); ++j)
		{
			LLVM_DEBUG(dbgs() << (int)conMat[i][j] << " ");
		}
		LLVM_DEBUG(dbgs() << "\n");
	}
	LLVM_DEBUG(dbgs() << "Done Printing ConMat....\n");
}

int DFG::getStaticRoutingCost(dfgNode *node, CGRANode *dest, std::map<CGRANode *, std::vector<CGRAEdge>> Edges)
{
	dfgNode *parent;
	TreePath tp;
	CGRANode *end;
	Port endPort;

	CGRANode *cnode;
	Port currPort;
	std::pair<CGRANode *, Port> currNodePortPair;
	int currCost = 0;
	int bestCost = INT_MAX;
	CGRANode *bestSource = NULL;

	int cost = 0;

	std::map<std::pair<CGRANode *, Port>, std::pair<CGRANode *, Port>> cameFrom;
	std::map<std::pair<CGRANode *, Port>, std::pair<CGRANode *, Port>>::iterator cameFromIt;
	std::map<CGRANode *, int> costSoFar;

	std::vector<CGRAEdge *> tempCGRAEdges;

	for (int i = 0; i < node->getAncestors().size(); ++i)
	{
		parent = node->getAncestors()[i];
		tp = createTreePath(parent, dest);

		bestSource = NULL;
		for (int j = 0; j < tp.sources.size(); ++j)
		{
			end = astar->AStarSearchEMS(Edges, tp.sources[j], dest, &cameFrom, &costSoFar, &endPort);
			if (end != dest)
			{
				continue;
			}

			//traverse the path
			cnode = dest;
			for (cameFromIt = cameFrom.begin(); cameFromIt != cameFrom.end(); cameFromIt++)
			{
				if (cameFromIt->first.first == cnode)
				{
					currPort = cameFromIt->first.second;
					break;
				}
			}
			currNodePortPair = std::make_pair(cnode, currPort);

			currCost = 0;
			while (cnode != tp.sources[j])
			{
				if ((cnode->getX() == cameFrom[currNodePortPair].first->getX()) &&
						(cnode->getY() == cameFrom[currNodePortPair].first->getY()))
				{
					currCost++;
				}
				else
				{
					if (cnode == dest)
					{
					}
					else
					{
						currCost += currCGRA->getMII();
					}
				}
				//				currCost += (currCGRA->getCGRANode(0,1,1)->originalEdgesSize - (*currCGRA->getCGRAEdges())[cnode].size());
				cnode = cameFrom[currNodePortPair].first;
			}

			if (currCost < bestCost)
			{
				bestCost = currCost;
				bestSource = tp.sources[j];
			}
		}

		if (bestSource == NULL)
		{
			//			LLVM_DEBUG(dbgs() << "getStaticRoutingCost :: routing ParentIdx=" << parent->getIdx();
			//			LLVM_DEBUG(dbgs() << ", placed=" << parent->getMappedLoc()->getName();
			//			LLVM_DEBUG(dbgs() << " to nodeIdx" << node->getIdx() << ", triedToBePlaced=" << dest->getName();
			//			LLVM_DEBUG(dbgs() << " FAILED! \n";
			return INT_MAX;
		}
		cost = cost + bestCost;
	}
	return cost;
}

MemOp DFG::isMemoryOp(dfgNode *node)
{

	LoadInst *Ld = dyn_cast<LoadInst>(node->getNode());
	StoreInst *St = dyn_cast<StoreInst>(node->getNode());

	if (!St && !Ld)
	{
		return INVALID;
	}

	if (Ld)
	{
		if (!Ld->isSimple())
		{
			return INVALID;
		}
		else
		{
			return LOAD;
		}
	}

	if (St)
	{
		if (!St->isSimple())
		{
			return INVALID;
		}
		else
		{
			return STORE;
		}
	}

	return INVALID;
}

#ifndef XMLCheckResult
#define XMLCheckResult(a_eResult)           \
		if (a_eResult != tinyxml2::XML_SUCCESS) \
		{                                       \
			printf("Error: %i\n", a_eResult);   \
			return a_eResult;                   \
		}
#endif

#ifndef XMLCheckNULL
#define XMLCheckNULL(xmlNode) \
		if (xmlNode == NULL)      \
		return tinyxml2::XML_ERROR_FILE_READ_ERROR;
#endif

int DFG::readXML(std::string fileName)
{
	tinyxml2::XMLDocument inputDFG;
	std::vector<dfgNode *> nodelist;

	int totalNumberOps = -1;
	int opManualCount = 0;
	int totalNumberEdges = -1;
	int edgeManualCount = 0;

	tinyxml2::XMLError err;
	tinyxml2::XMLElement *nextElem;
	tinyxml2::XMLElement *inElem1;
	tinyxml2::XMLElement *inElem2;
	tinyxml2::XMLNode *pRoot;
	int ancNumber = -1;
	int childNumber = -1;

	int tempInt1 = -1;
	int tempInt2 = -1;
	const char *tempchararr;
	//	std::string tempString;

	LLVM_DEBUG(dbgs() << "Reading xml input file : " << fileName << "\n");
	err = inputDFG.LoadFile(fileName.c_str());
	XMLCheckResult(err);

	pRoot = inputDFG.FirstChild();
	XMLCheckNULL(pRoot);
	LLVM_DEBUG(dbgs() << pRoot->Value() << "\n");

	LLVM_DEBUG(dbgs() << "Reading OPs\n");
	nextElem = pRoot->FirstChildElement("OPs");
	XMLCheckNULL(nextElem);

	LLVM_DEBUG(dbgs() << "Reading OP-number\n");
	nextElem = nextElem->FirstChildElement("OP-number");
	XMLCheckNULL(nextElem);
	err = nextElem->QueryIntText(&totalNumberOps);
	XMLCheckResult(err);

	nextElem = nextElem->NextSiblingElement("OP");
	XMLCheckNULL(nextElem);

	while (nextElem != NULL)
	{
		dfgNode *node = new dfgNode(this);
		LLVM_DEBUG(dbgs() << "Reading OP," << opManualCount << "\n");

		inElem1 = nextElem->FirstChildElement("ID");
		XMLCheckNULL(inElem1);
		err = inElem1->QueryIntText(&tempInt1);
		XMLCheckResult(err);
		node->setIdx(tempInt1);

		inElem1 = nextElem->FirstChildElement("OP-type");
		XMLCheckNULL(inElem1);
		tempchararr = inElem1->GetText();
		std::string tempString = tempchararr;
		LLVM_DEBUG(dbgs() << tempchararr << "\n");
		node->setNameType(tempString);

		//		LLVM_DEBUG(dbgs() << "Reading In-edge-number : ";
		//		inElem1 = nextElem->FirstChildElement("In-edge-number");
		//		XMLCheckNULL(inElem1);
		//		err = inElem1->QueryIntText(&tempInt1);
		//		XMLCheckResult(err);
		//		ancNumber = tempInt1;
		//		LLVM_DEBUG(dbgs() << ancNumber << "\n";
		//
		//		if(ancNumber > 0){
		//			//traverse to in-edges
		//			inElem1 = nextElem->FirstChildElement("In-edges");
		//			XMLCheckNULL(inElem1);
		//
		//			inElem1 = inElem1->FirstChildElement("Edge");
		//			XMLCheckNULL(inElem1);
		//			for (int i = 0; i < ancNumber; ++i) {
		//				LLVM_DEBUG(dbgs() << "Read in-edge \n";
		//				err = inElem1->QueryIntText(&tempInt1);
		//				XMLCheckResult(err);
		//				node->InEdgesIdx.push_back(tempInt1);
		//
		//				inElem1 = inElem1->NextSiblingElement("Edge");
		//			}
		//		}
		//
		//		LLVM_DEBUG(dbgs() << "Reading Out-edge-number : \n";
		//		inElem1 = nextElem->FirstChildElement("Out-edge-number");
		//		XMLCheckNULL(inElem1);
		//		err = inElem1->QueryIntText(&tempInt1);
		//		XMLCheckResult(err);
		//		childNumber = tempInt1;
		//
		//		if(childNumber > 0){
		//			//traverse to in-edges
		//			inElem1 = nextElem->FirstChildElement("Out-edges");
		//			XMLCheckNULL(inElem1);
		//
		//			inElem1 = inElem1->FirstChildElement("Edge");
		//			XMLCheckNULL(inElem1);
		//			for (int i = 0; i < ancNumber; ++i) {
		//				err = inElem1->QueryIntText(&tempInt1);
		//				XMLCheckResult(err);
		//				node->OutEdgesIdx.push_back(tempInt1);
		//
		//				inElem1 = inElem1->NextSiblingElement("Edge");
		//			}
		//		}
		nextElem = nextElem->NextSiblingElement("OP");
		opManualCount++;
		nodelist.push_back(node);
	}

	assert(opManualCount == totalNumberOps);

	//EDGES
	LLVM_DEBUG(dbgs() << "Reading EDGEs\n");
	nextElem = pRoot->FirstChildElement("EDGEs");
	XMLCheckNULL(nextElem);

	LLVM_DEBUG(dbgs() << "Reading Edge-number\n");
	nextElem = nextElem->FirstChildElement("Edge-number");
	XMLCheckNULL(nextElem);
	err = nextElem->QueryIntText(&totalNumberEdges);
	XMLCheckResult(err);

	nextElem = nextElem->NextSiblingElement("EDGE");
	XMLCheckNULL(nextElem);

	while (nextElem != NULL)
	{
		LLVM_DEBUG(dbgs() << "Reading Edge, " << edgeManualCount << "\n");
		inElem1 = nextElem->FirstChildElement("Start-OP");
		XMLCheckNULL(inElem1);
		err = inElem1->QueryIntText(&tempInt1);
		XMLCheckResult(err);
		inElem1 = nextElem->FirstChildElement("End-OP");
		XMLCheckNULL(inElem1);
		err = inElem1->QueryIntText(&tempInt2);
		XMLCheckResult(err);

		for (int i = 0; i < nodelist.size(); ++i)
		{
			if (nodelist[i]->getIdx() == tempInt1)
			{
				for (int j = 0; j < nodelist.size(); ++j)
				{
					if (nodelist[j]->getIdx() == tempInt2)
					{
						nodelist[i]->addChildNode(nodelist[j]);
						nodelist[j]->addAncestorNode(nodelist[i]);

						Edge temp;
						temp.setID(this->getEdges().size());
						std::ostringstream ss;
						ss << std::dec << nodelist[i]->getIdx() << "_to_" << nodelist[j]->getIdx();
						temp.setName(ss.str());
						temp.setType(EDGE_TYPE_DATA);
						temp.setSrc(nodelist[i]);
						temp.setDest(nodelist[j]);
						this->InsertEdge(temp);
					}
				}
			}
		}
		nextElem = nextElem->NextSiblingElement("EDGE");
		edgeManualCount++;
	}

	assert(edgeManualCount == totalNumberEdges);
	this->setNodes(nodelist);
	this->setName(fileName);
	return 0;
}

int DFG::printREGIMapOuts()
{
	std::ofstream nodeFile;
	std::ofstream edgeFile;
	dfgNode *node;
	Edge *edge;
	std::string fName;
	std::hash<std::string> strHash;
	std::hash<const char *> charArrHash;

	//Printing Nodes
	fName = name + "_REGIMAP_nodefile.txt";
	nodeFile.open(fName.c_str());
	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];

		if (node->getNode() != NULL)
		{
			nodeFile << std::to_string(node->getIdx()) << "\t";
			nodeFile << std::to_string((int)charArrHash(node->getNode()->getOpcodeName())) << "\t";
			nodeFile << node->getNode()->getOpcodeName() << std::endl;
		}
		else
		{
			nodeFile << std::to_string(node->getIdx()) << "\t";
			nodeFile << std::to_string((int)strHash(node->getNameType())) << "\t";
			nodeFile << node->getNameType() << std::endl;
		}
	}
	nodeFile.close();

	//Printing Edges
	fName = name + "_REGIMAP_edgefile.txt";
	edgeFile.open(fName.c_str());
	for (int i = 0; i < edgeList.size(); ++i)
	{
		edge = &edgeList[i];

		//currenlty assuming all the edges are data
		assert(edge->getType() == EDGE_TYPE_DATA);

		edgeFile << std::to_string(edge->getSrc()->getIdx()) << "\t";
		edgeFile << std::to_string(edge->getDest()->getIdx()) << "\t";
		edgeFile << "0"
				<< "\t";
		edgeFile << "TRU" << std::endl;
	}
	edgeFile.close();

	return 0;
}

int DFG::printTurns()
{
	dfgNode *node;
	dfgNode *parent;
	CGRANode *cnode;
	CGRANode *nextCnode;

	int xdiff;
	int ydiff;

	std::map<dfgNode *, std::vector<pathData *>> parentRouteMap;
	std::map<dfgNode *, std::vector<pathData *>>::iterator parentRouteMapIt;

	enum TurnDirs
	{
		NORTH,
		EAST,
		WEST,
		SOUTH,
		TILE
	};
	std::map<CGRANode *, std::map<TurnDirs, int>> CGRANodeTurnStatsMap;

	for (int t = 0; t < currCGRA->getMII(); ++t)
	{
		for (int y = 0; y < currCGRA->getYdim(); ++y)
		{
			for (int x = 0; x < currCGRA->getXdim(); ++x)
			{
				CGRANodeTurnStatsMap[currCGRA->getCGRANode(t, y, x)][NORTH] = 0;
				CGRANodeTurnStatsMap[currCGRA->getCGRANode(t, y, x)][EAST] = 0;
				CGRANodeTurnStatsMap[currCGRA->getCGRANode(t, y, x)][WEST] = 0;
				CGRANodeTurnStatsMap[currCGRA->getCGRANode(t, y, x)][SOUTH] = 0;
				CGRANodeTurnStatsMap[currCGRA->getCGRANode(t, y, x)][TILE] = 0;
			}
		}
	}

	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		parentRouteMap = node->getMergeRoutingLocs();

		for (parentRouteMapIt = parentRouteMap.begin();
				parentRouteMapIt != parentRouteMap.end();
				++parentRouteMapIt)
		{

			parent = parentRouteMapIt->first;

			if (parentRouteMap[parent].size() > 0)
			{
				CGRANodeTurnStatsMap[parentRouteMap[parent][0]->cnode][TILE]++;
			}
			else
			{
				continue;
			}

			for (int j = 1; j < parentRouteMap[parent].size(); ++j)
			{
				cnode = parentRouteMap[parent][j]->cnode;
				nextCnode = parentRouteMap[parent][j - 1]->cnode;

				if (cnode->getT() == nextCnode->getT())
				{ //SMART Routes
					xdiff = nextCnode->getX() - cnode->getX();
					ydiff = nextCnode->getY() - cnode->getY();

					//either one should not be zero
					assert((xdiff != 0) || (ydiff != 0));

					if (ydiff > 0)
					{
						assert(xdiff == 0);
						CGRANodeTurnStatsMap[cnode][SOUTH]++;
					}
					else if (ydiff == 0)
					{
						if (xdiff > 0)
						{
							CGRANodeTurnStatsMap[cnode][EAST]++;
						}
						else
						{ // xdiff < 0
							CGRANodeTurnStatsMap[cnode][WEST]++;
						}
					}
					else
					{ //ydiff < 0
						assert(xdiff == 0);
						CGRANodeTurnStatsMap[cnode][NORTH]++;
					}
				}
				else
				{
					if (std::find(cnode->regAllocation[nextCnode].begin(), cnode->regAllocation[nextCnode].end(), findEdge(parent, node)) == cnode->regAllocation[nextCnode].end())
					{
						cnode->regAllocation[nextCnode].push_back(findEdge(parent, node));
					}
				}
			}
		}
	}

	std::ofstream outFile;
	std::string fName = name + "_TURNSTATS.csv";
	outFile.open(fName.c_str());

	outFile << "Cycle,Y,X,NORTH,EAST,WEST,SOUTH,TILE" << std::endl;
	for (int y = 0; y < currCGRA->getYdim(); ++y)
	{
		for (int x = 0; x < currCGRA->getXdim(); ++x)
		{
			for (int t = 0; t < currCGRA->getMII(); ++t)
			{
				cnode = currCGRA->getCGRANode(t, y, x);
				outFile << cnode->getName() << ",";
				outFile << std::to_string(CGRANodeTurnStatsMap[cnode][NORTH]) << ",";
				outFile << std::to_string(CGRANodeTurnStatsMap[cnode][EAST]) << ",";
				outFile << std::to_string(CGRANodeTurnStatsMap[cnode][WEST]) << ",";
				outFile << std::to_string(CGRANodeTurnStatsMap[cnode][SOUTH]) << ",";
				outFile << std::to_string(CGRANodeTurnStatsMap[cnode][TILE]) << std::endl;
			}
		}
	}
	outFile.close();

	//calling Reg stats
	printRegStats();

	return 0;
}

int DFG::printRegStats()
{

	std::ofstream outFileRegStats;
	std::ofstream outFileRegToggle;

	std::string fName = name + "_REGSTATS.csv";
	outFileRegStats.open(fName.c_str());
	fName = name + "_REGTOGGLE.csv";
	outFileRegToggle.open(fName.c_str());

	std::map<CGRANode *, std::vector<Edge *>>::iterator it;

	CGRANode *cnode;
	CGRANode *nextCnode;
	std::map<int, Edge *> regAlloc;
	std::map<int, int> regToggle;
	std::vector<Edge *> tempEdgeVec;
	std::vector<Edge *>::iterator tempEdgeVecIt;

	int freeSlots;
	int totalToggles = NodeList.size(); //This is for output register of the ALU
	const int maxToggles = currCGRA->getXdim() * currCGRA->getYdim() * currCGRA->getMII() * (currCGRA->getRegsPerNode() + 1);

	for (int y = 0; y < currCGRA->getYdim(); ++y)
	{
		for (int x = 0; x < currCGRA->getXdim(); ++x)
		{
			outFileRegToggle << currCGRA->getCGRANode(0, y, x)->getNameWithOutTime() << ",";

			//Init regAlloc
			for (int i = 0; i < currCGRA->getRegsPerNode(); ++i)
			{
				regAlloc[i] = NULL;
				regToggle[i] = 0;
			}

			for (int t = 0; t < currCGRA->getMII(); ++t)
			{
				cnode = currCGRA->getCGRANode(t, y, x);
				outFileRegStats << cnode->getName() << ",";

				for (it = cnode->regAllocation.begin();
						it != cnode->regAllocation.end();
						it++)
				{

					nextCnode = it->first;
					if (nextCnode->getT() != cnode->getT())
					{
						tempEdgeVec = it->second;

						if (tempEdgeVec.size() > 4)
						{
							LLVM_DEBUG(dbgs() << "init tempEdgeVec.size() =" << tempEdgeVec.size() << "\n");
							for (int i = 0; i < tempEdgeVec.size(); ++i)
							{
								LLVM_DEBUG(dbgs() << tempEdgeVec[i]->getName() << "\n");
							}
						}

						freeSlots = 0;
						for (int i = 0; i < currCGRA->getRegsPerNode(); ++i)
						{
							if (regAlloc[i] != NULL)
							{
								tempEdgeVecIt = std::find(tempEdgeVec.begin(), tempEdgeVec.end(), regAlloc[i]);
								if (tempEdgeVecIt != tempEdgeVec.end())
								{
									tempEdgeVec.erase(tempEdgeVecIt);
								}
								else
								{
									regAlloc[i] = NULL;
									freeSlots++;
								}
							}
							else
							{
								freeSlots++;
							}
						}

						if (freeSlots < tempEdgeVec.size())
						{
							LLVM_DEBUG(dbgs() << "freeSlots=" << freeSlots << ",tempEdgeVec.size()=" << tempEdgeVec.size() << "\n");
						}
						assert(freeSlots >= tempEdgeVec.size());

						for (int i = 0; i < tempEdgeVec.size(); ++i)
						{
							for (int j = 0; j < currCGRA->getRegsPerNode(); ++j)
							{
								if (regAlloc[j] == NULL)
								{
									regAlloc[j] = tempEdgeVec[i];
									regToggle[j]++;
									totalToggles++;
									break;
								}
							}
						}

						for (int i = 0; i < it->second.size(); ++i)
						{
							outFileRegStats << it->second[i]->getName();
							for (int j = 0; j < currCGRA->getRegsPerNode(); ++j)
							{
								if (regAlloc[j] == it->second[i])
								{
									outFileRegStats << "-R" << std::to_string(j) << ",";
									break;
								}
							}
						}
						//						outFile << it->second->getName() << ",";
					}
				}
				outFileRegStats << std::endl;
			}

			for (int i = 0; i < currCGRA->getRegsPerNode(); ++i)
			{
				outFileRegToggle << std::to_string(regToggle[i]) << ",";
			}
			outFileRegToggle << std::endl;
		}
	}

	outFileRegToggle << std::endl;
	outFileRegToggle << "TOTAL Toggles," << std::to_string(totalToggles) << std::endl;
	outFileRegToggle << "Max Toggles," << std::to_string(maxToggles) << std::endl;
	outFileRegToggle << "Toggle Percentage," << std::to_string((double(totalToggles) * 100.0) / double(maxToggles)) << std::endl;

	outFileRegStats.close();
	outFileRegToggle.close();
	return 0;
}

int DFG::sortPossibleDests(
		std::vector<std::pair<CGRANode *, int>> *possibleDests)
{
}

int DFG::removeRedEdgesPHI()
{
	dfgNode *node;
	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		if (node->getNode() == NULL)
			continue;

		if (dyn_cast<PHINode>(node->getNode()))
		{
			for (dfgNode *parent : node->getAncestors())
			{
				if (node->getNameType().compare("CMERGE") != 0)
				{
					parent->removeChild(node);
					node->removeAncestor(parent);
					removeEdge(findEdge(parent, node));
				}
			}
		}
	}
}

int DFG::addCMERGEtoSELECT()
{
	dfgNode *node;
	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		if (node->getNode() == NULL)
			continue;

		if (SelectInst *SEL = dyn_cast<SelectInst>(node->getNode()))
		{
			LLVM_DEBUG(SEL->dump());
			LLVM_DEBUG(dbgs() << "node ancestors = " << node->getAncestors().size() << "\n");
			//			assert(node->getAncestors().size()==3);

			int condVal;
			dfgNode *condNode = NULL;
			//			if(ConstantInt *CI = dyn_cast<ConstantInt>(SEL->getCondition())){
			//				condVal = CI->getSExtValue();
			//			}
			if (dyn_cast<Instruction>(SEL->getCondition()))
			{
				condNode = findNode(cast<Instruction>(SEL->getCondition()));
				LLVM_DEBUG(cast<Instruction>(SEL->getCondition())->dump());
				assert(condNode != NULL);
				assert(std::find(node->getAncestors().begin(), node->getAncestors().end(), condNode) != node->getAncestors().end());
			}
			else
			{
				assert(false);
			}

			int trueVal;
			dfgNode *trueNode = NULL;
			if (ConstantInt *CI = dyn_cast<ConstantInt>(SEL->getTrueValue()))
			{
				trueVal = CI->getSExtValue();
			}
			else if (ConstantFP *FP = dyn_cast<ConstantFP>(SEL->getTrueValue()))
			{
				trueVal = 0; // 2019 work
			}
			else if (dyn_cast<Instruction>(SEL->getTrueValue()))
			{
				trueNode = findNode(cast<Instruction>(SEL->getTrueValue()));
				LLVM_DEBUG(cast<Instruction>(SEL->getTrueValue())->dump());
				assert(trueNode != NULL);
				assert(std::find(node->getAncestors().begin(), node->getAncestors().end(), trueNode) != node->getAncestors().end());
			}
			else
			{
				assert(false);
			}

			int falseVal;
			dfgNode *falseNode = NULL;
			if (ConstantInt *CI = dyn_cast<ConstantInt>(SEL->getFalseValue()))
			{
				falseVal = CI->getSExtValue();
			}
			else if (ConstantFP *FP = dyn_cast<ConstantFP>(SEL->getFalseValue()))
			{
				falseVal = 0; // 2019 work
			}
			else if (dyn_cast<Instruction>(SEL->getFalseValue()))
			{
				falseNode = findNode(cast<Instruction>(SEL->getFalseValue()));
				LLVM_DEBUG(cast<Instruction>(SEL->getFalseValue())->dump());
				assert(falseNode != NULL);
				assert(std::find(node->getAncestors().begin(), node->getAncestors().end(), falseNode) != node->getAncestors().end());
			}
			else
			{
				assert(false);
			}

			condNode->removeChild(node);
			node->removeAncestor(condNode);
			removeEdge(findEdge(condNode, node));

			if (trueNode)
			{
				trueNode->removeChild(node);
				node->removeAncestor(trueNode);
				removeEdge(findEdge(trueNode, node));
				node->addCMergeParent(condNode, trueNode, -1, true);
			}
			else
			{
				node->addCMergeParent(condNode, trueNode, trueVal, true);
			}

			dfgNode *notnode = new dfgNode(this);
			notnode->setIdx(this->getNodesPtr()->size());
			this->getNodesPtr()->push_back(notnode);
			notnode->BB = node->BB;
			notnode->setNameType("XORNOT");

			condNode->addChildNode(notnode);
			notnode->addAncestorNode(condNode);

			if (falseNode)
			{
				falseNode->removeChild(node);
				node->removeAncestor(falseNode);
				removeEdge(findEdge(falseNode, node));
				node->addCMergeParent(notnode, falseNode, -1, true);
			}
			else
			{
				node->addCMergeParent(notnode, falseNode, falseVal, true);
			}
		}
	}
	return 0;
}

int DFG::printJUMPLHeader(std::ofstream &binFile,
		std::ofstream &binOpNameFile)
{

	binOp currBinOp;

	currBinOp.opcode = HyCUBEInsBinary[JUMPL];
	currBinOp.outMap[PRED] = 0b111;
	currBinOp.outMap[OP1] = 0b111;
	currBinOp.outMap[OP2] = 0b111;
	currBinOp.outMap[NORTH] = 0b111;
	currBinOp.outMap[EAST] = 0b111;
	currBinOp.outMap[SOUTH] = 0b111;
	currBinOp.outMap[WEST] = 0b111;
	currBinOp.regwen = 0;
	currBinOp.regbypass = 0;
	currBinOp.tregwen = 0;

	uint32_t constant = 1;										 //PC
	constant = (constant << 5) | (currCGRA->getMII() & 0b11111); //LE
	constant = (constant << 5) | 1;
	currBinOp.constant = constant;
	currBinOp.constantValid = 1;
	currBinOp.npb = 0;

	binFile << std::to_string(0) << std::endl;
	binOpNameFile << std::to_string(0) << std::endl;

	for (int j = 0; j < currCGRA->getYdim(); ++j)
	{
		for (int k = 0; k < currCGRA->getXdim(); ++k)
		{
			binFile << "Y=" << std::to_string(j) << " X=" << std::to_string(k) << ",";
			binOpNameFile << "Y=" << std::to_string(j) << " X=" << std::to_string(k) << ",";

			//Print Binary
			binFile << std::setfill('0');
			binFile << std::setw(1) << std::bitset<1>(currBinOp.npb);			//<< ",";
			binFile << std::setw(1) << std::bitset<1>(currBinOp.constantValid); //<< ",";
			binFile << std::setw(27) << std::bitset<27>(currBinOp.constant);	//<< ",";
			binFile << std::setw(5) << std::bitset<5>(currBinOp.opcode);		//<< "," ;
			binFile << std::setw(4) << std::bitset<4>(currBinOp.regwen);		//<< ",";
			binFile << std::bitset<1>(currBinOp.tregwen);						//<< ",";
			binFile << std::setw(4) << std::bitset<4>(currBinOp.regbypass);		//<< ",";
			binFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[PRED]);  //<< ",";
			binFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[OP2]);   //<< ",";
			binFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[OP1]);   //<< ",";
			binFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[NORTH]); //<< ",";
			binFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[WEST]);  //<< ",";
			binFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[SOUTH]); //<< ",";
			binFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[EAST]) << std::endl;

			//Print Binary with OpName
			binOpNameFile << std::setfill('0');
			binOpNameFile << std::setw(1) << std::bitset<1>(currBinOp.npb);			  //<< ",";
			binOpNameFile << std::setw(1) << std::bitset<1>(currBinOp.constantValid); //<< ",";
			binOpNameFile << std::setw(27) << std::bitset<27>(currBinOp.constant);	//<< ",";
			binOpNameFile << std::setw(5) << std::bitset<5>(currBinOp.opcode);		  //<< "," ;
			binOpNameFile << std::setw(4) << std::bitset<4>(currBinOp.regwen);		  //<< ",";
			binOpNameFile << std::bitset<1>(currBinOp.tregwen);						  //<< ",";
			binOpNameFile << std::setw(4) << std::bitset<4>(currBinOp.regbypass);	 //<< ",";
			binOpNameFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[PRED]);  //<< ",";
			binOpNameFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[OP2]);   //<< ",";
			binOpNameFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[OP1]);   //<< ",";
			binOpNameFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[NORTH]); //<< ",";
			binOpNameFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[WEST]);  //<< ",";
			binOpNameFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[SOUTH]); //<< ",";
			binOpNameFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[EAST]) << ",";
			binOpNameFile << HyCUBEInsStrings[JUMPL] << std::endl;
		}
	}

	binFile << std::endl;
	binOpNameFile << std::endl;
}

int DFG::printMapping()
{
	dfgNode *node;
	dfgNode *parent;
	CGRANode *cnode;
	CGRANode *PrevCnode;
	CGRANode *PrevPrevCnode;
	std::vector<CGRAEdge> cgraEdges;
	std::vector<CGRAEdge> cgraEdges_t;
	std::vector<CGRAEdge> cgraEdges_t2;
	std::map<Port, Edge *> portDfgEdgeMap;

	std::vector<Port> portOrder = {NORTH, EAST, WEST, SOUTH, R0, R1, R2, R3};
	std::vector<Port> insPortOrder = {TREG, R0, R1, R2, R3, PRED, OP1, OP2, NORTH, EAST, WEST, SOUTH};
	std::map<CGRANode *, std::map<Port, Port>> XBarMap;

	//Check all nodes are mapped
	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		assert(node->getMappedLoc() != NULL);
	}

	std::ofstream mapFile;
	std::string mapFileName = name + "_mapFile.csv";
	mapFile.open(mapFileName.c_str());

	std::ofstream insFile;
	std::string insFileName = name + "_insFile.csv";
	insFile.open(insFileName.c_str());

	std::ofstream binFile;
	std::string binFileName = name + "_binFile.trc";
	binFile.open(binFileName.c_str());

	std::ofstream binOpNameFile;
	std::string binOpNameFileFileName = name + "_binOpNameFile.trc";
	binOpNameFile.open(binOpNameFileFileName.c_str());

	//Print Header
	mapFile << "Time,";
	insFile << "Time,";
	//	binFile << "Time,";
	for (int j = 0; j < currCGRA->getYdim(); ++j)
	{
		for (int k = 0; k < currCGRA->getXdim(); ++k)
		{
			mapFile << "Y=" << std::to_string(j) << " X=" << std::to_string(k) << ",";
			mapFile << "NORTH,EAST,WEST,SOUTH,R0,R1,R2,R3,";

			insFile << "Y=" << std::to_string(j) << " X=" << std::to_string(k) << ",";
			insFile << "TREG,R0,R1,R2,R3,PRED,OP1,OP2,NORTH,EAST,WEST,SOUTH,";

			//			binFile << "Y=" << std::to_string(j) << " X=" << std::to_string(k) << ",";
		}
	}
	binFile << "NPB,CONSTVALID,CONST,OPCODE,REGWEN,TREGWEN,REGBYPASS,PRED,OP1,OP2,NORTH,WEST,SOUTH,EAST";
	insFile << std::endl;
	mapFile << std::endl;
	binFile << std::endl;

	std::map<int, std::map<int, uint64_t>> constantValidMaskMap;

	printJUMPLHeader(binFile, binOpNameFile);

	//Print Data
	std::map<dfgNode *, std::set<dfgNode *>> parentInfo;
	for (int i = 0; i < currCGRA->getMII(); ++i)
	{
		mapFile << std::to_string(i) << ",";
		insFile << std::to_string(i) << ",";
		binFile << std::to_string(i + 1) << std::endl;
		binOpNameFile << std::to_string(i + 1) << std::endl;
		for (int j = 0; j < currCGRA->getYdim(); ++j)
		{
			for (int k = 0; k < currCGRA->getXdim(); ++k)
			{
				cnode = currCGRA->getCGRANode(i, j, k);
				node = cnode->getmappedDFGNode();
				if (node != NULL)
				{
					mapFile << std::to_string(node->getIdx()) << "::Parents<";
					for (int l = 0; l < node->getAncestors().size(); ++l)
					{
						parent = node->getAncestors()[l];
						mapFile << std::to_string(parent->getIdx()) << ":";
					}
					mapFile << ">,";
					insFile << HyCUBEInsStrings[node->getFinalIns()] << ",";
				}
				else
				{
					insFile << "NA,";
					mapFile << "NA,";
				}
				binFile << "Y=" << std::to_string(j) << " X=" << std::to_string(k) << ",";
				binOpNameFile << "Y=" << std::to_string(j) << " X=" << std::to_string(k) << ",";
				//CGRAEdges
				PrevCnode = cnode;
				PrevPrevCnode = currCGRA->getCGRANode((i - 1 + 3 * currCGRA->getMII()) % currCGRA->getMII(), j, k);
				cnode = currCGRA->getCGRANode((i + 1) % currCGRA->getMII(), j, k);
				cgraEdges = (*currCGRA->getCGRAEdges())[cnode];
				cgraEdges_t = currCGRA->getCGRAEdgesWithDest(cnode);

				for (int l = 0; l < cgraEdges.size(); ++l)
				{
					if (cgraEdges[l].mappedDFGEdge != NULL)
					{
						portDfgEdgeMap[cgraEdges[l].SrcPort] = cgraEdges[l].mappedDFGEdge;

						if (PrevPrevCnode->getPEType() == MEM)
						{
							if (cgraEdges[l].mappedDFGEdge->getSrc() == PrevPrevCnode->getmappedDFGNode())
							{
								XBarMap[PrevCnode][cgraEdges[l].SrcPort] = TILE;
							}
							else
							{
								for (int m = 0; m < cgraEdges_t.size(); ++m)
								{
									if (cgraEdges_t[m].mappedDFGEdge->getSrc() == cgraEdges[l].mappedDFGEdge->getSrc())
									{
										XBarMap[PrevCnode][cgraEdges[l].SrcPort] = cgraEdges_t[m].DstPort;
									}
								}
							}
						}
						else
						{
							if (cgraEdges[l].mappedDFGEdge->getSrc() == PrevCnode->getmappedDFGNode())
							{
								XBarMap[PrevCnode][cgraEdges[l].SrcPort] = TILE;
							}
							else
							{
								for (int m = 0; m < cgraEdges_t.size(); ++m)
								{
									if (cgraEdges_t[m].mappedDFGEdge->getSrc() == cgraEdges[l].mappedDFGEdge->getSrc())
									{
										XBarMap[PrevCnode][cgraEdges[l].SrcPort] = cgraEdges_t[m].DstPort;
									}
								}
							}
						}
					}
				}

				// TODO :: ADD a similiar if strucuture to MEM nodes where I need to check PrevPrevCnode instead of PrevCnode
				if (cnode->getmappedDFGNode() != NULL)
				{
					if (PrevCnode->getmappedDFGNode() != NULL)
					{
						for (int l = 0; l < cnode->getmappedDFGNode()->getAncestors().size(); ++l)
						{
							if (PrevCnode->getmappedDFGNode() == cnode->getmappedDFGNode()->getAncestors()[l])
							{
								dfgNode *currCnodeNode = cnode->getmappedDFGNode();
								dfgNode *cnodeParent = cnode->getmappedDFGNode()->getAncestors()[l];
								parentInfo[currCnodeNode].insert(cnodeParent);
								if (PrevCnode->getmappedDFGNode()->getFinalIns() == NOP)
								{

									//How can a NOP be a parent of someone?
									assert(false);

									//									if(XBarMap[PrevCnode].find(PRED) == XBarMap[PrevCnode].end()){
									//										XBarMap[PrevCnode][PRED] = TILE;
									//									}
									//									else if(XBarMap[PrevCnode].find(OP1) == XBarMap[PrevCnode].end()){
									//										XBarMap[PrevCnode][OP1] = TILE;
									//									}
									//									else if(XBarMap[PrevCnode].find(OP2) == XBarMap[PrevCnode].end()){
									//										XBarMap[PrevCnode][OP2] = TILE;
									//									}
									//									else{
									//										assert(false);
									//									}
								}
								else
								{

									bool found = false;
									if (currCnodeNode->parentClassification.find(0) !=
											currCnodeNode->parentClassification.end())
									{
										if (currCnodeNode->parentClassification[0] == cnodeParent)
										{
											assert(XBarMap[PrevCnode].find(PRED) == XBarMap[PrevCnode].end());
											XBarMap[PrevCnode][PRED] = TILE;
											found = true;
										}
									}
									else
									{
										XBarMap[PrevCnode][PRED] = INV;
									}

									if (currCnodeNode->parentClassification.find(1) !=
											currCnodeNode->parentClassification.end())
									{
										if (currCnodeNode->parentClassification[1] == cnodeParent)
										{
											assert(XBarMap[PrevCnode].find(OP1) == XBarMap[PrevCnode].end());
											XBarMap[PrevCnode][OP1] = TILE;
											found = true;
										}
									}
									else
									{
										XBarMap[PrevCnode][OP1] = INV;
									}

									if (currCnodeNode->parentClassification.find(2) !=
											currCnodeNode->parentClassification.end())
									{
										if (currCnodeNode->parentClassification[2] == cnodeParent)
										{
											assert(XBarMap[PrevCnode].find(OP2) == XBarMap[PrevCnode].end());
											XBarMap[PrevCnode][OP2] = TILE;
											found = true;
										}
									}
									else
									{
										XBarMap[PrevCnode][OP2] = INV;
									}

									LLVM_DEBUG(dbgs() << "printMapping : currNode = " << currCnodeNode->getIdx() << "\n");
									LLVM_DEBUG(dbgs() << "printMapping : currParent = " << cnodeParent->getIdx() << "\n");
									assert(found);

									//									if((PrevCnode->getmappedDFGNode()->getFinalIns() == CMP) ||
									//									   (PrevCnode->getmappedDFGNode()->getFinalIns() == BR)	){
									//										XBarMap[PrevCnode][PRED] = TILE;
									//									}
									//									else if(XBarMap[PrevCnode].find(OP1) == XBarMap[PrevCnode].end()){
									//										XBarMap[PrevCnode][OP1] = TILE;
									//									}
									//									else if(XBarMap[PrevCnode].find(OP2) == XBarMap[PrevCnode].end()){
									//										XBarMap[PrevCnode][OP2] = TILE;
									//									}
									//									else{
									//										LLVM_DEBUG(dbgs() << "UNCOMMENT here : assertion failes here truly!\n";
									////										assert(false);
									//									}
								}
							}
						}
					}
				}

				//adding the similiar structure
				if (PrevPrevCnode->getPEType() == MEM)
				{
					if (cnode->getmappedDFGNode() != NULL)
					{
						if (PrevPrevCnode->getmappedDFGNode() != NULL)
						{
							for (int l = 0; l < cnode->getmappedDFGNode()->getAncestors().size(); ++l)
							{
								if (PrevPrevCnode->getmappedDFGNode() == cnode->getmappedDFGNode()->getAncestors()[l])
								{

									dfgNode *currCnodeNode = cnode->getmappedDFGNode();
									dfgNode *cnodeParent = cnode->getmappedDFGNode()->getAncestors()[l];
									parentInfo[currCnodeNode].insert(cnodeParent);
									if (PrevPrevCnode->getmappedDFGNode()->getFinalIns() == NOP)
									{

										//How can a NOP be a parent of someone?
										assert(false);

										//									if(XBarMap[PrevCnode].find(PRED) == XBarMap[PrevCnode].end()){
										//										XBarMap[PrevCnode][PRED] = TILE;
										//									}
										//									else if(XBarMap[PrevCnode].find(OP1) == XBarMap[PrevCnode].end()){
										//										XBarMap[PrevCnode][OP1] = TILE;
										//									}
										//									else if(XBarMap[PrevCnode].find(OP2) == XBarMap[PrevCnode].end()){
										//										XBarMap[PrevCnode][OP2] = TILE;
										//									}
										//									else{
										//										assert(false);
										//									}
									}
									else
									{

										bool found = false;
										if (currCnodeNode->parentClassification.find(0) !=
												currCnodeNode->parentClassification.end())
										{
											if (currCnodeNode->parentClassification[0] == cnodeParent)
											{
												assert(XBarMap[PrevCnode].find(PRED) == XBarMap[PrevCnode].end());
												XBarMap[PrevCnode][PRED] = TILE;
												found = true;
											}
										}
										else
										{
											XBarMap[PrevCnode][PRED] = INV;
										}

										if (currCnodeNode->parentClassification.find(1) !=
												currCnodeNode->parentClassification.end())
										{
											if (currCnodeNode->parentClassification[1] == cnodeParent)
											{
												assert(XBarMap[PrevCnode].find(OP1) == XBarMap[PrevCnode].end());
												XBarMap[PrevCnode][OP1] = TILE;
												found = true;
											}
										}
										else
										{
											XBarMap[PrevCnode][OP1] = INV;
										}

										if (currCnodeNode->parentClassification.find(2) !=
												currCnodeNode->parentClassification.end())
										{
											if (currCnodeNode->parentClassification[2] == cnodeParent)
											{
												assert(XBarMap[PrevCnode].find(OP2) == XBarMap[PrevCnode].end());
												XBarMap[PrevCnode][OP2] = TILE;
												found = true;
											}
										}
										else
										{
											XBarMap[PrevCnode][OP2] = INV;
										}

										LLVM_DEBUG(dbgs() << "printMapping : currNode = " << currCnodeNode->getIdx() << "\n");
										LLVM_DEBUG(dbgs() << "printMapping : currParent = " << cnodeParent->getIdx() << "\n");

										if (!found)
										{
											LLVM_DEBUG(dbgs() << "Node : \n");
											if (currCnodeNode->getNode())
											{
												LLVM_DEBUG(currCnodeNode->getNode()->dump());
											}
											LLVM_DEBUG(dbgs() << "Parent : \n");
											if (cnodeParent->getNode())
											{
												LLVM_DEBUG(cnodeParent->getNode()->dump());
											}
											else if (cnodeParent->getNameType().compare("OUTOutLoopLOAD") == 0)
											{
												LLVM_DEBUG(OutLoopNodeMapReverse[cnodeParent]->dump());
											}
										}
										//TODO : DAC18
										//										assert(found);

										//									if((PrevCnode->getmappedDFGNode()->getFinalIns() == CMP) ||
										//									   (PrevCnode->getmappedDFGNode()->getFinalIns() == BR)	){
										//										XBarMap[PrevCnode][PRED] = TILE;
										//									}
										//									else if(XBarMap[PrevCnode].find(OP1) == XBarMap[PrevCnode].end()){
										//										XBarMap[PrevCnode][OP1] = TILE;
										//									}
										//									else if(XBarMap[PrevCnode].find(OP2) == XBarMap[PrevCnode].end()){
										//										XBarMap[PrevCnode][OP2] = TILE;
										//									}
										//									else{
										//										LLVM_DEBUG(dbgs() << "UNCOMMENT here : assertion failes here truly!\n";
										////										assert(false);
										//									}
									}
								}
							}
						}
					}
				}

				//removing other edges if edges with dest as current node is found
				cgraEdges_t2.clear();
				for (int m = 0; m < cgraEdges_t.size(); ++m)
				{

					if (cnode->getmappedDFGNode() == NULL)
						break;
					if (cnode->getmappedDFGNode()->isParent(cgraEdges_t[m].mappedDFGEdge->getSrc()))
					{
						cgraEdges_t2.push_back(cgraEdges_t[m]);
					}

					//					if(cgraEdges_t[m].mappedDFGEdge->getDest() == cnode->getmappedDFGNode()){
					//
					//					}
				}

				if (!cgraEdges_t2.empty())
				{
					cgraEdges_t = cgraEdges_t2;
				}

				for (int m = 0; m < cgraEdges_t.size(); ++m)
				{
					dfgNode *currCnodeNode = cnode->getmappedDFGNode();

					if (currCnodeNode == NULL)
					{
						continue;
					}

					if (parentInfo[currCnodeNode].find(cgraEdges_t[m].mappedDFGEdge->getSrc()) != parentInfo[currCnodeNode].end())
					{
						continue;
					}

					bool parentFound = false;
					for (dfgNode *parent : currCnodeNode->getAncestors())
					{
						currCGRA->printCGRAEdge(cgraEdges_t[m]);
						if (parent == cgraEdges_t[m].mappedDFGEdge->getSrc())
						{
							parentFound = true;
						}
					}
					//					bool parentFound = std::find(currCnodeNode->getAncestors().begin(),
					//												 currCnodeNode->getAncestors().end(),
					//												 cgraEdges_t[m].mappedDFGEdge->getSrc())
					//									   != currCnodeNode->getAncestors().end();
					if (parentFound)
					{
						LLVM_DEBUG(dbgs() << "currCnodeNode=" << currCnodeNode->getIdx() << "\n");
						LLVM_DEBUG(dbgs() << "Parents=");
						for (dfgNode *parent : currCnodeNode->getAncestors())
						{
							LLVM_DEBUG(dbgs() << parent->getIdx() << ",");
						}
						LLVM_DEBUG(dbgs() << "\n");
						LLVM_DEBUG(dbgs() << "Parent=" << cgraEdges_t[m].mappedDFGEdge->getSrc()->getIdx() << "\n");
					}

					if ((cgraEdges_t[m].mappedDFGEdge->getDest() == cnode->getmappedDFGNode()) || parentFound)
					{

						dfgNode *cnodeParent = cgraEdges_t[m].mappedDFGEdge->getSrc();
						dfgNode *cnodeChild = cgraEdges_t[m].mappedDFGEdge->getDest();
						parentInfo[currCnodeNode].insert(cnodeParent);

						if (cgraEdges_t[m].mappedDFGEdge->getSrc()->getFinalIns() == NOP)
						{

							assert(false);
							//Nothing should be routed from a NOP instruction

							//							if(XBarMap[PrevCnode].find(PRED) == XBarMap[PrevCnode].end()){
							//								XBarMap[PrevCnode][PRED] = cgraEdges_t[m].DstPort;
							//							}
							//							else if(XBarMap[PrevCnode].find(OP1) == XBarMap[PrevCnode].end()){
							//								XBarMap[PrevCnode][OP1] = cgraEdges_t[m].DstPort;
							//							}
							//							else if(XBarMap[PrevCnode].find(OP2) == XBarMap[PrevCnode].end()){
							//								XBarMap[PrevCnode][OP2] = cgraEdges_t[m].DstPort;
							//							}
							//							else{
							//								assert(false);
							//							}
						}
						else
						{

							LLVM_DEBUG(dbgs() << "printMapping : currNode = " << currCnodeNode->getIdx() << "\n");
							LLVM_DEBUG(dbgs() << "printMapping : currParent = " << cnodeParent->getIdx() << "\n");
							LLVM_DEBUG(dbgs() << "printMapping : PrevCnode = " << PrevCnode->getName() << "\n");

							bool found = false;
							if (currCnodeNode->parentClassification.find(0) !=
									currCnodeNode->parentClassification.end())
							{
								if (currCnodeNode->parentClassification[0] == cnodeParent)
								{
									this->getCGRA()->printCGRAEdge(cgraEdges_t[m]);
									assert(XBarMap[PrevCnode].find(PRED) == XBarMap[PrevCnode].end());
									XBarMap[PrevCnode][PRED] = cgraEdges_t[m].DstPort;
									LLVM_DEBUG(dbgs() << "PRED : " << cgraEdges_t[m].DstPort << "\n");
									found = true;
								}
							}
							else
							{
								XBarMap[PrevCnode][PRED] = INV;
							}

							if (currCnodeNode->parentClassification.find(1) !=
									currCnodeNode->parentClassification.end())
							{
								if (currCnodeNode->parentClassification[1] == cnodeParent)
								{
									this->getCGRA()->printCGRAEdge(cgraEdges_t[m]);
									assert(XBarMap[PrevCnode].find(OP1) == XBarMap[PrevCnode].end());
									XBarMap[PrevCnode][OP1] = cgraEdges_t[m].DstPort;
									LLVM_DEBUG(dbgs() << "I1 : " << cgraEdges_t[m].DstPort << "\n");
									found = true;
								}
							}
							else
							{
								XBarMap[PrevCnode][OP1] = INV;
							}

							if (currCnodeNode->parentClassification.find(2) !=
									currCnodeNode->parentClassification.end())
							{
								if (currCnodeNode->parentClassification[2] == cnodeParent)
								{
									this->getCGRA()->printCGRAEdge(cgraEdges_t[m]);
									assert(XBarMap[PrevCnode].find(OP2) == XBarMap[PrevCnode].end());
									XBarMap[PrevCnode][OP2] = cgraEdges_t[m].DstPort;
									LLVM_DEBUG(dbgs() << "I2 : " << cgraEdges_t[m].DstPort << "\n");
									found = true;
								}
							}
							else
							{
								XBarMap[PrevCnode][OP2] = INV;
							}

							if (!found)
							{
								LLVM_DEBUG(dbgs() << "Node : \n");
								if (currCnodeNode->getNode())
								{
									LLVM_DEBUG(currCnodeNode->getNode()->dump());
								}
								LLVM_DEBUG(dbgs() << "Parent : \n");
								if (cnodeParent->getNode())
								{
									LLVM_DEBUG(cnodeParent->getNode()->dump());
								}
								else if (cnodeParent->getNameType().compare("OUTOutLoopLOAD") == 0)
								{
									LLVM_DEBUG(OutLoopNodeMapReverse[cnodeParent]->dump());
								}
							}

							//TODO : DAC18
							//							assert(found);

							// mapped for hyCUBE instructions
							//							if((cgraEdges_t[m].mappedDFGEdge->getSrc()->getFinalIns() == CMP) ||
							//							   (cgraEdges_t[m].mappedDFGEdge->getSrc()->getFinalIns() == BR)	){
							//								XBarMap[PrevCnode][PRED] = cgraEdges_t[m].DstPort;
							//							}
							//							else if(XBarMap[PrevCnode].find(OP1) == XBarMap[PrevCnode].end()){
							//								XBarMap[PrevCnode][OP1] = cgraEdges_t[m].DstPort;
							//							}
							//							else if(XBarMap[PrevCnode].find(OP2) == XBarMap[PrevCnode].end()){
							//								XBarMap[PrevCnode][OP2] = cgraEdges_t[m].DstPort;
							//							}
							//							else{
							//								LLVM_DEBUG(dbgs() << "UNCOMMENT here : assertion failes here truly!\n";
							////								assert(false);
							//							}
						}
					}
				}

				//File Printing

				for (int l = 0; l < portOrder.size(); ++l)
				{
					if (portDfgEdgeMap.find(portOrder[l]) != portDfgEdgeMap.end())
					{
						mapFile << portDfgEdgeMap[portOrder[l]]->getName() << ",";
					}
					else
					{
						mapFile << "NA,";
					}
				}
				portDfgEdgeMap.clear();

				binOp currBinOp;
				if (node != NULL)
				{
					currBinOp.opcode = HyCUBEInsBinary[node->getFinalIns()];
				}
				else
				{
					currBinOp.opcode = 0;
				}
				//0b111 is used to cut of the output -- maybe switch of the asynchrnous repeater
				currBinOp.outMap[PRED] = 0b111;
				currBinOp.outMap[OP1] = 0b111;
				currBinOp.outMap[OP2] = 0b111;
				currBinOp.outMap[NORTH] = 0b111;
				currBinOp.outMap[EAST] = 0b111;
				currBinOp.outMap[SOUTH] = 0b111;
				currBinOp.outMap[WEST] = 0b111;
				currBinOp.regwen = 0;
				currBinOp.regbypass = 0;
				currBinOp.tregwen = 0;
				currBinOp.constant = 0;
				currBinOp.constantValid = 0;
				currBinOp.npb = 0;

				if (node != NULL)
				{
					if (node->hasConstantVal())
					{
						currBinOp.constant = node->getConstantVal();
						//						currBinOp.outMap[OP2] = node->getConstantVal() >> 27;
						currBinOp.constantValid = 1;
						constantValidMaskMap[j][k] = (constantValidMaskMap[j][k] & (~1) | 1) << 1;
					}
					if (node->getNPB())
					{
						currBinOp.npb = 1;
					}
					LLVM_DEBUG(dbgs() << "CurrNode=" << node->getIdx() << ",placed=" << node->getMappedLoc()->getName() << ",xbarop2=" << currBinOp.outMap[OP2] << "\n");
				}

				for (int l = 0; l < insPortOrder.size(); ++l)
				{
					if (XBarMap[PrevCnode].find(insPortOrder[l]) != XBarMap[PrevCnode].end())
					{
						insFile << currCGRA->getPortName(XBarMap[PrevCnode][insPortOrder[l]]) << ",";
						updateBinOp(&currBinOp, insPortOrder[l], XBarMap[PrevCnode][insPortOrder[l]]);
					}
					else
					{
						insFile << "NA,";
					}
				}

				LLVM_DEBUG(dbgs() << "xbarop2=" << currBinOp.outMap[OP2] << "\n");

				//Print Binary
				binFile << std::setfill('0');
				binFile << std::setw(1) << std::bitset<1>(currBinOp.npb);			//<< ",";
				binFile << std::setw(1) << std::bitset<1>(currBinOp.constantValid); //<< ",";
				binFile << std::setw(27) << std::bitset<27>(currBinOp.constant);	//<< ",";
				binFile << std::setw(5) << std::bitset<5>(currBinOp.opcode);		//<< "," ;
				binFile << std::setw(4) << std::bitset<4>(currBinOp.regwen);		//<< ",";
				binFile << std::bitset<1>(currBinOp.tregwen);						//<< ",";
				binFile << std::setw(4) << std::bitset<4>(currBinOp.regbypass);		//<< ",";
				binFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[PRED]);  //<< ",";
				binFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[OP2]);   //<< ",";
				binFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[OP1]);   //<< ",";
				binFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[NORTH]); //<< ",";
				binFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[WEST]);  //<< ",";
				binFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[SOUTH]); //<< ",";
				binFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[EAST]) << std::endl;

				//Print Binary with OpName
				binOpNameFile << std::setfill('0');
				binOpNameFile << std::setw(1) << std::bitset<1>(currBinOp.npb);			  //<< ",";
				binOpNameFile << std::setw(1) << std::bitset<1>(currBinOp.constantValid); //<< ",";
				binOpNameFile << std::setw(27) << std::bitset<27>(currBinOp.constant);	//<< ",";
				binOpNameFile << std::setw(5) << std::bitset<5>(currBinOp.opcode);		  //<< "," ;
				binOpNameFile << std::setw(4) << std::bitset<4>(currBinOp.regwen);		  //<< ",";
				binOpNameFile << std::bitset<1>(currBinOp.tregwen);						  //<< ",";
				binOpNameFile << std::setw(4) << std::bitset<4>(currBinOp.regbypass);	 //<< ",";
				binOpNameFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[PRED]);  //<< ",";
				binOpNameFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[OP2]);   //<< ",";
				binOpNameFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[OP1]);   //<< ",";
				binOpNameFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[NORTH]); //<< ",";
				binOpNameFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[WEST]);  //<< ",";
				binOpNameFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[SOUTH]); //<< ",";
				binOpNameFile << std::setw(3) << std::bitset<3>(currBinOp.outMap[EAST]) << ",";
				if (node != NULL)
				{
					binOpNameFile << HyCUBEInsStrings[node->getFinalIns()] << std::endl;
				}
				else
				{
					binOpNameFile << "NULL" << std::endl;
				}
			}
		}
		mapFile << std::endl;
		insFile << std::endl;
		binFile << std::endl;
	}

	//Check whether all the parents are accounted for
	for (std::pair<dfgNode *, std::set<dfgNode *>> unit : parentInfo)
	{
		dfgNode *node = unit.first;
		if (node->getAncestors().size() != parentInfo[node].size())
		{
			LLVM_DEBUG(dbgs() << "This node :" << node->getIdx() << "'s parents are not accounted for\n");
			LLVM_DEBUG(dbgs() << "Accounted parents : ");
			for (dfgNode *par : parentInfo[node])
			{
				LLVM_DEBUG(dbgs() << par->getIdx() << ",");
			}
			LLVM_DEBUG(dbgs() << "\n");
		}
		assert(node->getAncestors().size() == parentInfo[node].size());
	}

	//	LLVM_DEBUG(dbgs() << "parentInfo.size() =" << parentInfo.size() << "\n";
	//	LLVM_DEBUG(dbgs() << "NodeList.size() =" << NodeList.size() << "\n";
	//	assert(parentInfo.size()==NodeList.size());

	binFile << std::endl;
	binOpNameFile << std::endl;

	//	for (int j = 0; j < currCGRA->getYdim(); ++j) {
	//		for (int k = 0; k < currCGRA->getXdim(); ++k) {
	//			binFile << "Y=" << j << ",X=" << k << "ContantValidMask=" << std::setw(32) << std::bitset<32>(constantValidMaskMap[j][k]) << std::endl;
	//			binOpNameFile << "Y=" << j << ",X=" << k << "ContantValidMask=" << std::setw(32) << std::bitset<32>(constantValidMaskMap[j][k]) << std::endl;
	//		}
	//	}

	mapFile.close();
	insFile.close();
	binFile.close();
	binOpNameFile.close();
}

int DFG::updateBinOp(binOp *binOpIns, Port outPort, Port inPort)
{

	switch (outPort)
	{
	case R0:
		if (inPort != R0)
		{
			binOpIns->regwen = binOpIns->regwen | 0b1000;
		}
		break;
	case R1:
		if (inPort != R1)
		{
			binOpIns->regwen = binOpIns->regwen | 0b0001;
		}
		break;
	case R2:
		if (inPort != R2)
		{
			binOpIns->regwen = binOpIns->regwen | 0b0100;
		}
		break;
	case R3:
		if (inPort != R3)
		{
			binOpIns->regwen = binOpIns->regwen | 0b0010;
		}
		break;
	case NORTH:
	case EAST:
	case WEST:
	case SOUTH:
	case OP1:
	case OP2:
	case PRED:
		switch (inPort)
		{
		case R0:
			binOpIns->regbypass = binOpIns->regbypass | 0b0100;
		case NORTH:
			binOpIns->outMap[outPort] = 0b011;
			break;
		case R1:
			binOpIns->regbypass = binOpIns->regbypass | 0b0001;
		case EAST:
			binOpIns->outMap[outPort] = 0b000;
			break;
		case R2:
			binOpIns->regbypass = binOpIns->regbypass | 0b0010;
		case WEST:
			binOpIns->outMap[outPort] = 0b010;
			break;
		case R3:
			binOpIns->regbypass = binOpIns->regbypass | 0b1000;
		case SOUTH:
			binOpIns->outMap[outPort] = 0b001;
			break;
		case TILE:
			binOpIns->outMap[outPort] = 0b100;
			break;
		case TREG:
			binOpIns->outMap[outPort] = 0b101;
			break;
		case INV:
			binOpIns->outMap[outPort] = 0b111;
			break;
		default:
			assert(false);
			break;
		}
		break;
		case TREG:
			if (inPort != TREG)
			{
				binOpIns->tregwen = 0b1;
			}
			break;
		default:
			assert(false);
			break;
	}
}

int DFG::printCongestionInfo()
{
	std::map<int, std::vector<dfgNode *>> nodeMapASAPLevels;
	std::map<int, double> regEdgeCountASAPLevels;
	dfgNode *node;
	dfgNode *child;
	int childASAPLevel;
	bool sanity = false;

	for (int i = 0; i < maxASAPLevel; ++i)
	{
		regEdgeCountASAPLevels[i] = 0;
	}

	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		nodeMapASAPLevels[node->getASAPnumber()].push_back(node);

		for (int j = 0; j < node->getChildren().size(); ++j)
		{
			child = node->getChildren()[j];
			childASAPLevel = child->getASAPnumber();
			for (int k = node->getASAPnumber() + 1; k < childASAPLevel; ++k)
			{
				regEdgeCountASAPLevels[k]++;
			}
		}
	}

	int MII = (int)ceil((double)NodeList.size() / ((double)currCGRA->getXdim() * (double)currCGRA->getYdim()));
	std::map<int, std::map<int, int>> nodeMapEst;
	std::map<int, int> totalNodeCountMapEst;

	std::map<int, std::map<int, double>> edgeMapEst;
	std::map<int, double> totalEdgeCountMapEst;

	int cgraNodePerLevel = currCGRA->getXdim() * currCGRA->getYdim();

	int II = MII;
	int t;
	int T;

	for (int i = 0; i < II; ++i)
	{
		totalNodeCountMapEst[i] = 0;
		nodeMapEst[i][-1] = -1;
	}

	int mapLevel = 0;
	int nodeNumberToBePlaced = 0;
	for (int i = 0; i < maxASAPLevel; ++i)
	{
		LLVM_DEBUG(dbgs() << "printCongestionInfo :: ASAPLevel=" << i << "\n");
		t = mapLevel % II;
		T = mapLevel / II;

		if (totalNodeCountMapEst[t] + nodeMapASAPLevels[i].size() <= cgraNodePerLevel)
		{
			totalNodeCountMapEst[t] += nodeMapASAPLevels[i].size();
			assert(nodeMapEst[t].find(T) == nodeMapEst[t].end());
			nodeMapEst[t][T] = nodeMapASAPLevels[i].size();
			edgeMapEst[t][T] = regEdgeCountASAPLevels[i];
			mapLevel++;
			nodeNumberToBePlaced = 0;
		}
		else
		{
			nodeNumberToBePlaced = totalNodeCountMapEst[t] + nodeMapASAPLevels[i].size() - cgraNodePerLevel;
			LLVM_DEBUG(dbgs() << "first nodeNumberToBePlaced=" << nodeNumberToBePlaced << "\n");
			totalNodeCountMapEst[t] = cgraNodePerLevel;
			assert(nodeMapEst[t].find(T) == nodeMapEst[t].end());
			nodeMapEst[t][T] = cgraNodePerLevel - totalNodeCountMapEst[t];
			edgeMapEst[t][T] = (regEdgeCountASAPLevels[i] * (double)nodeMapEst[t][T]) / (double)nodeMapASAPLevels[i].size();
			assert(nodeNumberToBePlaced > 0);

			while (nodeNumberToBePlaced != 0)
			{
				LLVM_DEBUG(dbgs() << "while nodeNumberToBePlaced=" << nodeNumberToBePlaced << "\n");
				mapLevel++;
				t = mapLevel % II;
				T = mapLevel / II;
				if (totalNodeCountMapEst[t] + nodeNumberToBePlaced <= cgraNodePerLevel)
				{
					totalNodeCountMapEst[t] += nodeNumberToBePlaced;
					assert(nodeMapEst[t].find(T) == nodeMapEst[t].end());
					nodeMapEst[t][T] = nodeNumberToBePlaced;
					edgeMapEst[t][T] = (regEdgeCountASAPLevels[i] * (double)nodeMapEst[t][T]) / (double)nodeMapASAPLevels[i].size();
					mapLevel++;
					nodeNumberToBePlaced = 0;
				}
				else
				{
					nodeNumberToBePlaced = totalNodeCountMapEst[t] + nodeNumberToBePlaced - cgraNodePerLevel;
					LLVM_DEBUG(dbgs() << "while else nodeNumberToBePlaced=" << nodeNumberToBePlaced << "\n");
					totalNodeCountMapEst[t] = cgraNodePerLevel;
					assert(nodeMapEst[t].find(T) == nodeMapEst[t].end());
					nodeMapEst[t][T] = cgraNodePerLevel - totalNodeCountMapEst[t];
					edgeMapEst[t][T] = (regEdgeCountASAPLevels[i] * (double)nodeMapEst[t][T]) / (double)nodeMapASAPLevels[i].size();
				}

				//sanitycheck
				sanity = false;
				for (int i = 0; i < II; ++i)
				{
					assert(totalNodeCountMapEst[i] <= cgraNodePerLevel);
					if (totalNodeCountMapEst[i] < cgraNodePerLevel)
					{
						sanity = true;
					}
				}
				if (!sanity)
				{
					LLVM_DEBUG(dbgs() << "printCongestionInfo is crazy!\n");
					exit(-1);
				}
			}
		}
	}
	LLVM_DEBUG(dbgs() << "printCongestionInfo :: ASAPLevels done.\n");

	std::ofstream outFile;
	std::string outFileName = name + "_congestinfo.txt";
	outFile.open(outFileName.c_str());
	std::map<int, int>::iterator it;

	for (int i = 0; i < II; ++i)
	{
		LLVM_DEBUG(dbgs() << "t=" << std::to_string(i));
		outFile << "t=" << std::to_string(i);

		for (int j = 0; j < nodeMapEst[i].size() - 1; ++j)
		{
			assert(nodeMapEst[i].find(j) != nodeMapEst[i].end());
			LLVM_DEBUG(dbgs() << "," << std::to_string(nodeMapEst[i][j]));
			outFile << "," << std::to_string(nodeMapEst[i][j]);
		}
		LLVM_DEBUG(dbgs() << "\n");
		outFile << std::endl;
	}

	LLVM_DEBUG(dbgs() << "Edges\n");
	outFile << "Edges" << std::endl;

	for (int i = 0; i < II; ++i)
	{
		LLVM_DEBUG(dbgs() << "t=" << std::to_string(i));
		outFile << "t=" << std::to_string(i);

		for (int j = 0; j < edgeMapEst[i].size() - 1; ++j)
		{
			assert(edgeMapEst[i].find(j) != edgeMapEst[i].end());
			LLVM_DEBUG(dbgs() << "," << std::to_string(edgeMapEst[i][j]));
			outFile << "," << std::to_string(edgeMapEst[i][j]);
		}
		LLVM_DEBUG(dbgs() << "\n");
		outFile << std::endl;
	}

	outFile.close();

	LLVM_DEBUG(dbgs() << "printCongestionInfo done\n");
	return 0;
}

int DFG::handleMEMops()
{
	dfgNode *node;
	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		if (node->getNode() != NULL)
		{
			if (node->getNode()->mayReadOrWriteMemory())
			{
				node->setIsMemOp(true);
			}
		}
		else
		{
			if ((node->getNameType().compare("LOAD") == 0) ||
					(node->getNameType().compare("STORE") == 0) ||
					(node->getNameType().compare("LOOPSTART") == 0) ||
					(node->getNameType().compare("STORESTART") == 0) ||
					(node->getNameType().compare("LOOPEXIT") == 0) ||
					(node->getNameType().compare("OutLoopSTORE") == 0) ||
					(node->getNameType().compare("OutLoopLOAD") == 0))
			{
				node->setIsMemOp(true);
			}
		}
	}
	return 0;
}

int DFG::getMEMOpsToBePlaced()
{
	int memOpsToBePlaced = 0;
	dfgNode *node;
	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		if (node->getMappedLoc() == NULL)
		{
			if (node->getIsMemOp())
			{
				memOpsToBePlaced++;
			}
		}
	}
	return memOpsToBePlaced;
}

int DFG::getOutLoopMEMOps()
{
	int outloopMEMOps = 0;
	for (dfgNode *node : NodeList)
	{
		if (node->getNameType().compare("OutLoopLOAD") == 0 ||
				node->getNameType().compare("OutLoopSTORE") == 0)
		{
			outloopMEMOps++;
		}
	}
	return outloopMEMOps;
}

DFG::DFG(std::string name, std::map<Loop *, std::string> *lnPtr)
{
	this->name = name;
	this->loopNamesPtr = lnPtr;

	HyCUBEInsStrings[NOP] = "NOP";
	HyCUBEInsBinary[NOP] = 0 | (0b00000);

	HyCUBEInsStrings[ADD] = "ADD";
	HyCUBEInsBinary[ADD] = 0 | (0b00001);

	HyCUBEInsStrings[SUB] = "SUB";
	HyCUBEInsBinary[SUB] = 0 | (0b00010);

	HyCUBEInsStrings[MUL] = "MUL";
	HyCUBEInsBinary[MUL] = 0 | (0b00011);

	HyCUBEInsStrings[SEXT] = "SEXT";
	HyCUBEInsBinary[SEXT] = 0 | (0b00100);

	HyCUBEInsStrings[DIV] = "DIV";
	HyCUBEInsBinary[DIV] = 0 | (0b00101);

	HyCUBEInsStrings[LS] = "LS";
	HyCUBEInsBinary[LS] = 0 | (0b01000);

	HyCUBEInsStrings[RS] = "RS";
	HyCUBEInsBinary[RS] = 0 | (0b01001);

	HyCUBEInsStrings[ARS] = "ARS";
	HyCUBEInsBinary[ARS] = 0 | (0b01010);

	HyCUBEInsStrings[AND] = "AND";
	HyCUBEInsBinary[AND] = 0 | (0b01011);

	HyCUBEInsStrings[OR] = "OR";
	HyCUBEInsBinary[OR] = 0 | (0b01100);

	HyCUBEInsStrings[XOR] = "XOR";
	HyCUBEInsBinary[XOR] = 0 | (0b01101);

	HyCUBEInsStrings[SELECT] = "SELECT";
	HyCUBEInsBinary[SELECT] = 0 | (0b10000);

	HyCUBEInsStrings[CMERGE] = "CMERGE";
	HyCUBEInsBinary[CMERGE] = 0 | (0b10001);

	HyCUBEInsStrings[CMP] = "CMP";
	HyCUBEInsBinary[CMP] = 0 | (0b10010);

	HyCUBEInsStrings[CLT] = "CLT";
	HyCUBEInsBinary[CLT] = 0 | (0b10011);

	HyCUBEInsStrings[BR] = "BR";
	HyCUBEInsBinary[BR] = 0 | (0b10100);

	HyCUBEInsStrings[CGT] = "CGT";
	HyCUBEInsBinary[CGT] = 0 | (0b10101);

	HyCUBEInsStrings[LOADCL] = "LOADCL";
	HyCUBEInsBinary[LOADCL] = 0 | (0b10110);

	HyCUBEInsStrings[MOVCL] = "MOVCL";
	HyCUBEInsBinary[MOVCL] = 0 | (0b10111);

	HyCUBEInsStrings[Hy_LOAD] = "LOAD";
	HyCUBEInsBinary[Hy_LOAD] = 0 | (0b11000);

	HyCUBEInsStrings[Hy_LOADH] = "LOADH";
	HyCUBEInsBinary[Hy_LOADH] = 0 | (0b11001);

	HyCUBEInsStrings[Hy_LOADB] = "LOADB";
	HyCUBEInsBinary[Hy_LOADB] = 0 | (0b11010);

	HyCUBEInsStrings[Hy_STORE] = "STORE";
	HyCUBEInsBinary[Hy_STORE] = 0 | (0b11011);

	HyCUBEInsStrings[Hy_STOREH] = "STOREH";
	HyCUBEInsBinary[Hy_STOREH] = 0 | (0b11100);

	HyCUBEInsStrings[Hy_STOREB] = "STOREB";
	HyCUBEInsBinary[Hy_STOREB] = 0 | (0b11101);

	HyCUBEInsStrings[JUMPL] = "JUMPL";
	HyCUBEInsBinary[JUMPL] = 0 | (0b11110);

	HyCUBEInsStrings[MOVC] = "MOVC";
	HyCUBEInsBinary[MOVC] = 0 | (0b11111);
}

int DFG::nameNodes()
{
	dfgNode *node;

	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];

		if (node->getNode() != NULL)
		{
			switch (node->getNode()->getOpcode())
			{
			case Instruction::Add:
			case Instruction::FAdd:
				node->setFinalIns(ADD);
				break;
			case Instruction::Sub:
			case Instruction::FSub:
				node->setFinalIns(SUB);
				break;
			case Instruction::Mul:
			case Instruction::FMul:
				node->setFinalIns(MUL);
				break;
			case Instruction::UDiv:
			case Instruction::SDiv:
			case Instruction::FDiv:
				node->setFinalIns(DIV);
				break;
			case Instruction::URem:
			case Instruction::SRem:
			case Instruction::FRem:
				LLVM_DEBUG(dbgs() << "REM operations are not implemented\n");
				assert(false);
				break;
			case Instruction::Shl:
				node->setFinalIns(LS);
				break;
			case Instruction::LShr:
				node->setFinalIns(RS);
				break;
			case Instruction::AShr:
				node->setFinalIns(ARS);
				break;
			case Instruction::And:
				node->setFinalIns(AND);
				break;
			case Instruction::Or:
				node->setFinalIns(OR);
				break;
			case Instruction::Xor:
				node->setFinalIns(XOR);
				break;
			case Instruction::Load:
				if (node->getTypeSizeBytes() >= 4)
				{ //TODO : remove this, we dont support 8 byte data structs
					node->setFinalIns(Hy_LOAD);
				}
				else if (node->getTypeSizeBytes() == 2)
				{
					node->setFinalIns(Hy_LOADH);
				}
				else if (node->getTypeSizeBytes() == 1)
				{
					node->setFinalIns(Hy_LOADB);
				}
				else if (node->getNode()->getType()->isPointerTy())
				{ //pointer is a 32 bit address
					node->setFinalIns(Hy_LOAD);
				}
				else
				{
					LLVM_DEBUG(node->getNode()->dump());
					LLVM_DEBUG(dbgs() << "OutLoopLOAD size = " << node->getTypeSizeBytes() << "\n");
					if (node->getNode()->getType() == Type::getDoubleTy(node->getNode()->getContext()))
					{
						node->setFinalIns(Hy_LOAD);
					}
					if (node->getNode()->getType() == Type::getDoublePtrTy(node->getNode()->getContext()))
					{
						node->setFinalIns(Hy_LOAD);
					}
					else
					{
						assert(0);
					}
				}
				break;
			case Instruction::Store:
				if (node->getTypeSizeBytes() >= 4)
				{ //TODO : remove this, we dont support 8 byte data structs
					node->setFinalIns(Hy_STORE);
				}
				else if (node->getTypeSizeBytes() == 2)
				{
					node->setFinalIns(Hy_STOREH);
				}
				else if (node->getTypeSizeBytes() == 1)
				{
					node->setFinalIns(Hy_STOREB);
				}
				else if (node->getNode()->getType()->isPointerTy() || node->getNode()->getOperand(0)->getType()->isPointerTy())
				{ //pointer is a 32 bit address
					node->setFinalIns(Hy_STORE);
				}
				else
				{
					LLVM_DEBUG(node->getNode()->dump());
					LLVM_DEBUG(dbgs() << "TypeSize : " << node->getTypeSizeBytes() << "\n");
					assert(0);
				}
				break;
			case Instruction::GetElementPtr:
				node->setFinalIns(ADD);
				{
					GetElementPtrInst *GEP = cast<GetElementPtrInst>(node->getNode());
					assert(GEPOffsetMap.find(GEP) != GEPOffsetMap.end());

					if (node->getGEPbaseAddr() != -1)
					{
						node->setGEPbaseAddr(node->getGEPbaseAddr() + GEPOffsetMap[GEP]);
					}
					else
					{
						node->setGEPbaseAddr(GEPOffsetMap[GEP]);
					}
					node->setConstantVal(node->getGEPbaseAddr());
				}
				break;

				break;
			case Instruction::Trunc:
			{
				TruncInst *TI = cast<TruncInst>(node->getNode());
				//					node->setConstantVal(TI->getDestTy()->getIntegerBitWidth()/8);
				double destBitWidthDbl = (double)(TI->getDestTy()->getIntegerBitWidth());
				double bitMaskDbl = pow(2, destBitWidthDbl) - 1;
				uint32_t bitMask = ((uint32_t)bitMaskDbl);
				node->setConstantVal(bitMask);
				node->setFinalIns(AND);
			}
			break;
			case Instruction::SExt:
			{
				SExtInst *SI = cast<SExtInst>(node->getNode());
				uint32_t srcByteWidth = SI->getSrcTy()->getIntegerBitWidth() / 8;
				uint32_t destByteWidth = SI->getDestTy()->getIntegerBitWidth() / 8;
				uint32_t constOperand = (srcByteWidth << 16) | destByteWidth;
				node->setConstantVal(constOperand);
				node->setFinalIns(SEXT);
			}
			break;
			case Instruction::ZExt:
				node->setFinalIns(OR);
				node->setConstantVal(0);
				break;
			case Instruction::SIToFP:
			case Instruction::FPExt:
			case Instruction::FPTrunc:
			case Instruction::FPToUI:
			case Instruction::FPToSI:
			case Instruction::UIToFP:
			case Instruction::PtrToInt:
			case Instruction::IntToPtr:
			case Instruction::BitCast:
			case Instruction::AddrSpaceCast:
				LLVM_DEBUG(node->getNode()->dump());
				//					assert(0);
				//2019 work
				node->setFinalIns(OR);
				node->setConstantVal(0);
				break;
			case Instruction::PHI:
			case Instruction::Select:
				node->setFinalIns(SELECT);
				break;
			case Instruction::Br:
			{
				BranchInst *BRI = cast<BranchInst>(node->getNode());
				if (BRI->isUnconditional())
				{
					node->setFinalIns(OR);
					node->setConstantVal(1);
				}
				else
				{
					node->setFinalIns(OR);
					node->setConstantVal(0);
				}
			}
			break;
			case Instruction::ICmp:
			{
				LLVM_DEBUG(dbgs() << "NameNodes::Node=" << node->getIdx() << ",CMP=");
				CmpInst *CI = cast<CmpInst>(node->getNode());
				LLVM_DEBUG(CI->dump());
				switch (CI->getPredicate())
				{
				case CmpInst::ICMP_SLT:
				case CmpInst::ICMP_ULT:
					LLVM_DEBUG(dbgs() << "LT\n");
					node->setFinalIns(CLT);
					break;
				case CmpInst::ICMP_SGT:
				case CmpInst::ICMP_UGT:
					LLVM_DEBUG(dbgs() << "GT\n");
					node->setFinalIns(CGT);
					break;
				case CmpInst::ICMP_EQ:
					LLVM_DEBUG(dbgs() << "EQ\n");
					node->setFinalIns(CMP);
					break;
				default:
					assert(false);
					break;
				}
			}
			break;
			case Instruction::FCmp:
				LLVM_DEBUG(dbgs() << "NameNodes::Node=" << node->getIdx() << ",FCMP\n");
				node->setFinalIns(CMP);
				//2019 work
				//					assert(false);
				break;
			default:
				LLVM_DEBUG(dbgs() << "The Op :" << node->getNode()->getOpcodeName() << " that I thought would not be in the compiled code\n");
				LLVM_DEBUG(node->getNode()->dump());
				assert(false);
				break;
			}
		}
		else
		{
			if (node->getNameType().compare("LOAD") == 0)
			{
				if (node->getTypeSizeBytes() == 4)
				{
					node->setFinalIns(Hy_LOAD);
				}
				else if (node->getTypeSizeBytes() == 2)
				{
					node->setFinalIns(Hy_LOADH);
				}
				else if (node->getTypeSizeBytes() == 1)
				{
					node->setFinalIns(Hy_LOADB);
				}
				else
				{
					assert(0);
				}
			}
			else if (node->getNameType().compare("STORE") == 0)
			{
				if (node->getTypeSizeBytes() == 4)
				{
					node->setFinalIns(Hy_STORE);
				}
				else if (node->getTypeSizeBytes() == 2)
				{
					node->setFinalIns(Hy_STOREH);
				}
				else if (node->getTypeSizeBytes() == 1)
				{
					node->setFinalIns(Hy_STOREB);
				}
				else
				{
					assert(0);
				}
			}
			else if (node->getNameType().compare("NORMAL") == 0)
			{
				node->setFinalIns(ADD);
			}
			else if (node->getNameType().compare("CTRLBrOR") == 0)
			{
				node->setFinalIns(OR);
			}
			else if (node->getNameType().compare("SELECTPHI") == 0)
			{
				node->setFinalIns(SELECT);
			}
			else if (node->getNameType().compare("OutLoopSTORE") == 0)
			{
				if (node->getTypeSizeBytes() == 4)
				{
					node->setFinalIns(Hy_STORE);
				}
				else if (node->getTypeSizeBytes() == 2)
				{
					node->setFinalIns(Hy_STOREH);
				}
				else if (node->getTypeSizeBytes() == 1)
				{
					node->setFinalIns(Hy_STOREB);
				}
				else
				{
					LLVM_DEBUG(OutLoopNodeMapReverse[node]->dump());
					if (OutLoopNodeMapReverse[node]->getType()->isDoubleTy())
					{
						//TODO : to make it compatible with double
						node->setFinalIns(Hy_STORE);
					}
					else
					{
						assert(0);
					}
				}
				node->setConstantVal(node->getoutloopAddr());
			}
			else if (node->getNameType().compare("OutLoopLOAD") == 0)
			{
				if (node->getTypeSizeBytes() == 4)
				{
					node->setFinalIns(Hy_LOAD);
				}
				else if (node->getTypeSizeBytes() == 2)
				{
					node->setFinalIns(Hy_LOADH);
				}
				else if (node->getTypeSizeBytes() == 1)
				{
					node->setFinalIns(Hy_LOADB);
				}
				else
				{
					LLVM_DEBUG(OutLoopNodeMapReverse[node]->dump());
					LLVM_DEBUG(dbgs() << "OutLoopLOAD size = " << node->getTypeSizeBytes() << "\n");
					if (OutLoopNodeMapReverse[node]->getType()->isDoubleTy())
					{
						//TODO : to make it compatible with double
						node->setFinalIns(Hy_LOAD);
					}
					else
					{
						assert(0);
					}
				}
				node->setConstantVal(node->getoutloopAddr());
			}
			else if (node->getNameType().compare("CMERGE") == 0)
			{
				node->setFinalIns(CMERGE);
			}
			else if (node->getNameType().compare("LOOPSTART") == 0)
			{
				//				node->setFinalIns(BR);
				node->setFinalIns(Hy_LOADB);
#ifdef ARCHI_16BIT
				node->setConstantVal(MEM_SIZE - 1);
#else
				node->setConstantVal(MEM_SIZE - 2);
#endif
			}
			else if (node->getNameType().compare("LOADSTART") == 0)
			{
				node->setFinalIns(Hy_LOADB);
#ifdef ARCHI_16BIT
				node->setConstantVal(MEM_SIZE - 1);
#else
				node->setConstantVal(MEM_SIZE - 2);
#endif
			}
			else if (node->getNameType().compare("STORESTART") == 0)
			{
				node->setFinalIns(Hy_STOREB);
#ifdef ARCHI_16BIT
				node->setConstantVal(MEM_SIZE - 1);
#else
				node->setConstantVal(MEM_SIZE - 2);
#endif		
			}
			else if (node->getNameType().compare("LOOPEXIT") == 0)
			{
				node->setFinalIns(Hy_STOREB);
#ifdef ARCHI_16BIT
				node->setConstantVal(MEM_SIZE  - 2);
#else
				node->setConstantVal(MEM_SIZE/2  - 1);
#endif	
			}
			else if (node->getNameType().compare("MOVC") == 0)
			{
				node->setFinalIns(MOVC);
			}
			else if (node->getNameType().compare("XORNOT") == 0)
			{
				node->setFinalIns(XOR);
				node->setConstantVal(1);
			}
			else if (node->getNameType().compare("ORZERO") == 0)
			{
				node->setFinalIns(OR);
				node->setConstantVal(0);
			}
			else if (node->getNameType().compare("GEPLEFTSHIFT") == 0)
			{
				node->setFinalIns(LS);
			}
			else if (node->getNameType().compare("GEPADD") == 0)
			{
				node->setFinalIns(ADD);
			}
			else if (node->getNameType().compare("MASKAND") == 0)
			{
				node->setFinalIns(AND);
			}
			else if (node->getNameType().compare("PREDAND") == 0)
			{
				node->setFinalIns(AND);
			}
			else if (node->getNameType().compare("TRIGMERGE") == 0)
			{
				node->setFinalIns(SELECT);
			}
			else
			{
				LLVM_DEBUG(dbgs() << "Unknown custom node \n");
				assert(false);
			}
		}

		hyCUBEInsHist[node->getFinalIns()]++;
	}
}

int DFG::checkSanity()
{
	dfgNode *node;
	//Sanity Check
	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		if (node->getAncestors().size() > 3)
		{
			LLVM_DEBUG(dbgs() << "More than 3 ancestors, NodeIdx=" << node->getIdx());
			if (node->getNode() != NULL)
			{
				LLVM_DEBUG(node->getNode()->dump());
			}
			LLVM_DEBUG(dbgs() << "\n");
		}
		assert(NodeList[i]->getAncestors().size() <= 3);
	}
}

std::string DFG::getArchName(ArchType arch)
{
	switch (arch)
	{
	case RegXbar:
		return "RegXBar";
		break;
	case RegXbarTREG:
		return "RegXbarTREG";
		break;
	case DoubleXBar:
		return "DoubleXBar";
		break;
	case LatchXbar:
		return "LatchXbar";
		break;
	case StdNOC:
		return "StdNOC";
		break;
	case NoNOC:
		return "NoNOC";
		break;
	case ALL2ALL:
		return "ALL2ALL";
		break;
	default:
		return "UnNamed Arch";
		break;
	}
}

bool DFG::MapASAPLevelUnWrapped(int MII, int XDim, int YDim, ArchType arch)
{

	return true;
}

int DFG::handlePHINodes(std::set<BasicBlock *> LoopBB)
{
	LLVM_DEBUG(dbgs() << "handlePHINodes started!\n");
	dfgNode *node;
	std::map<dfgNode *, std::vector<const BasicBlock *>> processedPhiNodes;

	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		for (int j = 0; j < node->PHIchildren.size(); ++j)
		{
			assert(node->PHIchildren[j] != NULL);
			LLVM_DEBUG(node->PHIchildren[j]->dump());
			assert(findNode(node->PHIchildren[j]) != NULL);

			LLVM_DEBUG(node->PHIchildren[j]->dump());

			if (PHINode *phiIns = dyn_cast<PHINode>(node->PHIchildren[j]))
			{
				for (int k = 0; k < phiIns->getNumIncomingValues(); ++k)
				{
					BasicBlock *bb = phiIns->getIncomingBlock(k);
					if (bb != node->BB)
					{
						continue;
					}
					LLVM_DEBUG(dbgs() << "handlePHINodes adding nodes...\n");
					BasicBlock::iterator instIter = --bb->end();
					Instruction *brIns = &*instIter;
					assert(brIns->getOpcode() == Instruction::Br);

					dfgNode *brNode = findNode(brIns);
					findNode(node->PHIchildren[j])->addCMergeParent(node, brNode);
					processedPhiNodes[findNode(node->PHIchildren[j])].push_back(node->BB);
				}
			}
			else
			{
				assert(0);
			}

			//			node->addPHIChildNode(findNode(node->PHIchildren[j]));
			//			findNode(node->PHIchildren[j])->addPHIAncestorNode(node);
		}
	}

	LLVM_DEBUG(dbgs() << "second loop\n");

	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		if (node->getNode() == NULL)
			continue;
		if (PHINode *phiIns = dyn_cast<PHINode>(node->getNode()))
		{
			LLVM_DEBUG(phiIns->dump());
			LLVM_DEBUG(dbgs() << "incomingblocks:" << phiIns->getNumIncomingValues() << "\n");

			for (int k = 0; k < phiIns->getNumIncomingValues(); ++k)
			{
				BasicBlock *bb = phiIns->getIncomingBlock(k);

				if (processedPhiNodes.find(node) != processedPhiNodes.end())
				{
					if (std::find(processedPhiNodes[node].begin(),
							processedPhiNodes[node].end(),
							bb) != processedPhiNodes[node].end())
					{
						continue;
					}
				}

				LLVM_DEBUG(dbgs() << "handlePHINodes adding nodes in second loop...\n");

				BasicBlock::iterator instIter = --bb->end();
				Instruction *brIns = &*instIter;
				LLVM_DEBUG(brIns->dump());
				//				assert(LoopBB.find(brIns->getParent())!=LoopBB.end());
				assert(brIns->getOpcode() == Instruction::Br);
				dfgNode *brNode = findNode(brIns);
				//				assert(brNode!=NULL);

				if (ConstantInt *CI = dyn_cast<ConstantInt>(phiIns->getIncomingValueForBlock(bb)))
				{
					assert(CI->getBitWidth() <= 32);

					if (LoopBB.find(brIns->getParent()) == LoopBB.end())
					{
						node->addCMergeParent(brIns, CI->getSExtValue());
					}
					else
					{
						assert(brNode != NULL);
						node->addCMergeParent(brNode, NULL, CI->getSExtValue());
					}

				} //TODO : DAC18
				else if (ConstantFP *CF = dyn_cast<ConstantFP>(phiIns->getIncomingValueForBlock(bb)))
				{
					//					assert(CI->getBitWidth() <= 32);

					if (LoopBB.find(brIns->getParent()) == LoopBB.end())
					{
						node->addCMergeParent(brIns, (int)(CF->getValueAPF().convertToFloat()));
					}
					else
					{
						assert(brNode != NULL);
						node->addCMergeParent(brNode, NULL, (int)(CF->getValueAPF().convertToFloat()));
					}

				} //TODO : DAC18
				else if (UndefValue *UND = dyn_cast<UndefValue>(phiIns->getIncomingValueForBlock(bb)))
				{

					if (LoopBB.find(brIns->getParent()) == LoopBB.end())
					{
						node->addCMergeParent(brIns, 0);
					}
					else
					{
						assert(brNode != NULL);
						node->addCMergeParent(brNode, NULL, 0);
					}
				}
				else if (Instruction *phiData = dyn_cast<Instruction>(phiIns->getIncomingValueForBlock(bb)))
				{
					if (LoopBB.find(phiData->getParent()) == LoopBB.end())
					{
						//						node->addLoadParent(phiData);
						//						assert(OutLoopNodeMap[phiData]!=NULL);
						//						dfgNode* phiDataNode=OutLoopNodeMap[phiData];
						//						node->addCMergeParent(brNode,phiDataNode);

						if (LoopBB.find(brIns->getParent()) == LoopBB.end())
						{
							node->addCMergeParent(brIns, phiData);
						}
						else
						{
							node->addCMergeParent(brNode, phiData);
						}
					}
					else
					{
						assert(findNode(phiData) != NULL);
						//						assert(brNode!=NULL);
						dfgNode *phiDataNode = findNode(phiData);
						//						node->addCMergeParent(brNode,phiDataNode);

						if (LoopBB.find(brIns->getParent()) == LoopBB.end())
						{
							node->addCMergeParent(brIns, phiDataNode);
						}
						else
						{
							node->addCMergeParent(brNode, phiDataNode);
						}
					}
				}
				else if (sizeArrMap.find(phiIns->getIncomingValueForBlock(bb)->getName().str()) != sizeArrMap.end())
				{

					std::string ptrName = phiIns->getIncomingValueForBlock(bb)->getName().str();

					if (LoopBB.find(brIns->getParent()) == LoopBB.end())
					{
						node->addCMergeParent(brIns, allocatedArraysMap[ptrName]);
					}
					else
					{
						assert(brNode != NULL);
						node->addCMergeParent(brNode, NULL, allocatedArraysMap[ptrName]);
					}
				}
				else
				{
					for (std::pair<std::string, int> pair : sizeArrMap)
					{
						LLVM_DEBUG(dbgs() << pair.first << ":" << pair.second << "\n");
					}
					LLVM_DEBUG(dbgs() << "sizeARRMap size =" << sizeArrMap.size() << "\n");
					LLVM_DEBUG(phiIns->getIncomingValueForBlock(bb)->dump());
					assert(0);
				}
			}
		}
	}

	bool loopStartFound = false;
	for (std::pair<BasicBlock *, BasicBlock *> pair : this->loopentryBB)
	{
		BasicBlock *bb = pair.first;
		BasicBlock *entrybb = pair.second;
		BasicBlock::iterator instIter = --bb->end();
		Instruction *brIns = &*instIter;
		assert(brIns->getOpcode() == Instruction::Br);

		if (LoopStartMap.find(brIns) != LoopStartMap.end())
		{
			continue;
		}

		dfgNode *tempLoopStartNode;
		tempLoopStartNode = new dfgNode(this);
		if (LoopStartMap.empty())
		{
			tempLoopStartNode->setIdx(getNodesPtr()->size());
			getNodesPtr()->push_back(tempLoopStartNode);
		}
		else
		{
			tempLoopStartNode->setIdx(10000);
		}
		tempLoopStartNode->setNameType("LOOPSTART");
		LLVM_DEBUG(dbgs() << "Adding loopstart.\n");
		LLVM_DEBUG(brIns->dump());
		tempLoopStartNode->BB = entrybb;
		LoopStartMap[brIns] = tempLoopStartNode;

		std::vector<dfgNode *> leafNodes;
		for (int i = 0; i < NodeList.size(); ++i)
		{
			node = NodeList[i];
			if (node->getNameType().compare("LOOPSTART") == 0)
			{
				continue;
			}
			if (node->getNameType().compare("OutLoopLOAD") == 0)
			{
				continue;
			}
			if (node->getNameType().compare("MOVC") == 0)
			{
				continue;
			}
			if (node->BB != entrybb)
			{
				continue;
			}
			if (node->getAncestors().empty() && node->getPHIancestors().empty())
			{
				leafNodes.push_back(node);
			}
		}

		for (dfgNode *leafNode : leafNodes)
		{
			tempLoopStartNode->addChildNode(leafNode);
			leafNode->addAncestorNode(tempLoopStartNode);
		}
	}

	LLVM_DEBUG(dbgs() << "handlePHINodes DONE!\n");
	return 0;
}

TreePath DFG::createTreePathPHIDest(dfgNode *node, CGRANode *dest, CGRANode *phiChildDest)
{
	TreePath tp;
	dfgNode *child;
	CGRANode *NodeExt;
	CGRANode *cnode;
	int MII = currCGRA->getMII();

	assert(!node->getPHIchildren().empty());

	if (dest->getPEType() == MEM)
	{
		NodeExt = currCGRA->getCGRANode((dest->getT() + 2) % MII, dest->getY(), dest->getX());
		tp.sourceSCpathTdimLengths[NodeExt] = 2;
	}
	else
	{
		NodeExt = currCGRA->getCGRANode((dest->getT() + 1) % MII, dest->getY(), dest->getX());
		tp.sourceSCpathTdimLengths[NodeExt] = 1;
	}
	tp.sources.push_back(NodeExt);
	tp.sourcePorts[NodeExt] = TILE;
	tp.sourcePaths[NodeExt] = (std::make_pair(node, node));
	tp.sourceSCpathLengths[NodeExt] = 0;

	assert(node != NULL);

	tp.dest = phiChildDest;
	bool foundParentTreeBasedRoutingLocs = false;

	return tp;
}

void DFG::addPHIChildEdges()
{
	dfgNode *node;
	dfgNode *phiChildNode;

	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		if (!node->PHIchildren.empty())
		{
			for (int j = 0; j < node->PHIchildren.size(); ++j)
			{
				phiChildNode = findNode(node->PHIchildren[j]);

				Edge temp;
				temp.setID(getEdges().size());

				std::ostringstream ss;
				ss << std::dec << node->getIdx() << "_to_" << phiChildNode->getIdx();
				temp.setName(ss.str());
				temp.setType(EDGE_TYPE_PHI);
				temp.setSrc(node);
				temp.setDest(phiChildNode);

				InsertEdge(temp);
			}
		}
	}
}

void DFG::addPHIParents()
{
	dfgNode *node;
	dfgNode *phiChild;

	std::vector<dfgNode *> ancestors;
	std::vector<dfgNode *>::iterator searchParent;
	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		for (int j = 0; j < node->getPHIchildren().size(); ++j)
		{
			phiChild = node->getPHIchildren()[j];
			ancestors = phiChild->getAncestors();
			LLVM_DEBUG(dbgs() << "\n addPHIParents : child : " << phiChild->getIdx() << "\n");
			LLVM_DEBUG(dbgs() << "addPHIParents : anc : " << node->getIdx() << "\n");
			LLVM_DEBUG(dbgs() << "ancestors.size= : " << ancestors.size() << "\n");
			searchParent = std::find(ancestors.begin(), ancestors.end(), node);
			if (searchParent != ancestors.end())
				LLVM_DEBUG(dbgs() << "searchParent : " << (*searchParent)->getIdx() << "\n");
			if (searchParent == ancestors.end())
			{
				LLVM_DEBUG(dbgs() << "ifaddPHIParents : child : " << phiChild->getIdx() << "\n");
				LLVM_DEBUG(dbgs() << "ifaddPHIParents : anc : " << node->getIdx() << "\n");
				phiChild->addAncestorNode(node, EDGE_TYPE_PHI);
				node->addChildNode(phiChild, EDGE_TYPE_PHI);
			}
		}
	}
}

dfgNode *DFG::findNodeMappedLoc(CGRANode *cnode)
{
	dfgNode *node;

	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		if (node->getMappedLoc() == cnode)
		{
			return node;
		}
	}
	return NULL;
}

void DFG::partitionFuncDFG(DFG *funcDFG, std::vector<DFG *> dfgVectorPtr)
{
}

void DFG::GEPInvestigate(Function &F, Loop *L, std::map<std::string, int> *sizeArrMap)
{

	LLVMContext &Ctx = F.getContext();
	std::set<Value *> handledGEPs;

	//LoopPreHeader Printf marker insertion
	BasicBlock *loopPH = L->getLoopPreheader();
	BasicBlock::iterator instIterPH = loopPH->begin();
	Instruction *loopPHStartIns = &*instIterPH;

	IRBuilder<> builder(loopPHStartIns);
	builder.SetInsertPoint(loopPH, loopPH->getFirstInsertionPt());

	Value *printfSTARTstr = builder.CreateGlobalStringPtr((*this->loopNamesPtr)[L] + ":------------LOOP START--------------\n");
	Value *printfENDstr = builder.CreateGlobalStringPtr((*this->loopNamesPtr)[L] + ":------------LOOP END----------------\n");
	assert(this->loopNamesPtr->find(L) != this->loopNamesPtr->end());
	Value *loopName = builder.CreateGlobalStringPtr((*this->loopNamesPtr)[L]);

	//	Constant* printfMARKER = F.getParent()->getOrInsertFunction(
	//					  "printf",
	//					  FunctionType::getVoidTy(Ctx),
	//					  Type::getInt8PtrTy(Ctx),
	//					  NULL);

	auto loopStartFn = F.getParent()->getOrInsertFunction(
			"loopStart",
			FunctionType::getVoidTy(Ctx),
			Type::getInt8PtrTy(Ctx));

	builder.CreateCall(loopStartFn, {loopName});

	//the entry location for instrumentation code
	BasicBlock *loopHeader = L->getHeader();
	BasicBlock::iterator instIter = loopHeader->begin();
	Instruction *loopStartIns = &*instIter;

	SmallVector<BasicBlock *, 8> loopExitBlocksSV;
	L->getExitBlocks(loopExitBlocksSV);

	std::set<BasicBlock *> s;
	for (unsigned i = 0; i < loopExitBlocksSV.size(); ++i)
		s.insert(loopExitBlocksSV[i]);
	std::vector<BasicBlock *> loopExitBlocks;
	loopExitBlocks.assign(s.begin(), s.end());

	std::vector<Instruction *> loopExitInsVec;
	assert(loopExitBlocks.size() != 0);
	for (int i = 0; i < loopExitBlocks.size(); ++i)
	{
		BasicBlock *LoopExitBB = loopExitBlocks[i];
		//		BasicBlock::iterator instIter = --LoopExitBB->end();
		BasicBlock::iterator instIter = LoopExitBB->begin();
		LLVM_DEBUG(dbgs() << "LoopExit Blocks : \n");
		LLVM_DEBUG(LoopExitBB->dump());

		if (std::find(loopExitInsVec.begin(), loopExitInsVec.end(), &*instIter) == loopExitInsVec.end())
		{
			loopExitInsVec.push_back(&*instIter);
		}
	}

	//	IRBuilder<> builder(loopStartIns);
	//	builder.SetInsertPoint(loopStartIns);
	builder.SetInsertPoint(loopHeader, loopHeader->getFirstInsertionPt());
	auto clearPrintedArrs = F.getParent()->getOrInsertFunction(
			"clearPrintedArrs", FunctionType::getVoidTy(Ctx));

	//	for (int i = 0; i < loopExitInsVec.size(); ++i) {
	//		builder.SetInsertPoint(loopExitInsVec[i]);
	//		builder.CreateCall(clearPrintedArrs);
	//	}

	for (int i = 0; i < loopExitBlocks.size(); ++i)
	{
		builder.SetInsertPoint(loopExitBlocks[i], --loopExitBlocks[i]->end());
		builder.CreateCall(clearPrintedArrs);
	}

	dfgNode *node;
	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];

		if (node->getNameType().compare("OutLoopLOAD") == 0)
		{
			if (!node->isTransferedByHost())
			{
				continue;
			}
			LLVM_DEBUG(dbgs() << "OutLoopLoad found!!\n");
			Value *printfstr = builder.CreateGlobalStringPtr("OutLoopLoadNode:%d,val=%d,addr=%d\n");
			assert(OutLoopNodeMapReverse[node] != NULL);
			LLVM_DEBUG(OutLoopNodeMapReverse[node]->dump());
			Value *loadVal = OutLoopNodeMapReverse[node];
			Value *nodeIdx = ConstantInt::get(Type::getInt32Ty(Ctx), node->getIdx());
			Value *addrVal = ConstantInt::get(Type::getInt32Ty(Ctx), node->getoutloopAddr());
			//			Value* args[] = {printfstr,nodeIdx,loadVal,addrVal};

			//			Constant* printf = F.getParent()->getOrInsertFunction(
			//							  "printf",
			//							  FunctionType::getVoidTy(Ctx),
			//							  Type::getInt8PtrTy(Ctx),
			//							  Type::getInt32Ty(Ctx),
			//							  loadVal->getType(),
			//							  Type::getInt32Ty(Ctx),
			//							  NULL);

			auto outloopReportFn = F.getParent()->getOrInsertFunction(
					"outloopValueReport",
					FunctionType::getVoidTy(Ctx),
					Type::getInt32Ty(Ctx), //nodeIdx
					loadVal->getType(),	//value
					Type::getInt32Ty(Ctx), //addr
					Type::getInt8Ty(Ctx),  //isLoad
					Type::getInt8Ty(Ctx),  //isHostTrans
					Type::getInt8Ty(Ctx)   //size
			);

			Value *isLoad = ConstantInt::get(Type::getInt8Ty(Ctx), 1);
			Value *isHostTrans;
			Value *size = ConstantInt::get(Type::getInt8Ty(Ctx), node->getTypeSizeBytes());
			if (node->isTransferedByHost())
			{
				isHostTrans = ConstantInt::get(Type::getInt8Ty(Ctx), 1);
			}
			else
			{
				isHostTrans = ConstantInt::get(Type::getInt8Ty(Ctx), 0);
			}
			Value *args[] = {nodeIdx, loadVal, addrVal, isLoad, isHostTrans, size};
			for (int i = 0; i < loopExitBlocks.size(); ++i)
			{
				builder.SetInsertPoint(loopExitBlocks[i], --loopExitBlocks[i]->end());
				//				builder.CreateCall(printf,args);
				builder.CreateCall(outloopReportFn, args);
				//				LLVM_DEBUG(dbgs() << "its a load\n";
				//				loopExitBlocks[i]->dump();
			}
		}

		if (node->getNameType().compare("OutLoopSTORE") == 0)
		{
			if (!node->isTransferedByHost())
			{
				continue;
			}
			Value *printfstr = builder.CreateGlobalStringPtr("OutLoopStoreNode:%d,val=%d,addr=%d\n");
			assert(node->getAncestors().size() == 1);
			Value *StoreVal = (node->getAncestors())[0]->getNode();
			Value *nodeIdx = ConstantInt::get(Type::getInt32Ty(Ctx), node->getIdx());
			Value *addrVal = ConstantInt::get(Type::getInt32Ty(Ctx), node->getoutloopAddr());
			//			Value* args[] = {printfstr,nodeIdx,StoreVal,addrVal};

			//			Constant* printf = F.getParent()->getOrInsertFunction(
			//							  "printf",
			//							  FunctionType::getVoidTy(Ctx),
			//							  Type::getInt8PtrTy(Ctx),
			//							  Type::getInt32Ty(Ctx),
			//							  StoreVal->getType(),
			//							  Type::getInt32Ty(Ctx),
			//							  NULL);

			auto outloopReportFn = F.getParent()->getOrInsertFunction(
					"outloopValueReport",
					FunctionType::getVoidTy(Ctx),
					Type::getInt32Ty(Ctx), //nodeIdx
					StoreVal->getType(),   //value
					Type::getInt32Ty(Ctx), //addr
					Type::getInt8Ty(Ctx),  //isLoad
					Type::getInt8Ty(Ctx),  //isHostTrans
					Type::getInt8Ty(Ctx)   //size
			);

			Value *isLoad = ConstantInt::get(Type::getInt8Ty(Ctx), 0);
			Value *isHostTrans;
			Value *size = ConstantInt::get(Type::getInt8Ty(Ctx), node->getTypeSizeBytes());
			if (node->isTransferedByHost())
			{
				isHostTrans = ConstantInt::get(Type::getInt8Ty(Ctx), 1);
			}
			else
			{
				isHostTrans = ConstantInt::get(Type::getInt8Ty(Ctx), 0);
			}
			Value *args[] = {nodeIdx, StoreVal, addrVal, isLoad, isHostTrans, size};
			for (int i = 0; i < loopExitBlocks.size(); ++i)
			{
				builder.SetInsertPoint(loopExitBlocks[i], --loopExitBlocks[i]->end());
				//				builder.CreateCall(printf,args);
				builder.CreateCall(outloopReportFn, args);
				//				LLVM_DEBUG(dbgs() << "its a store\n";
				//				OutLoopNodeMapReverse[node]->dump();
				//				loopExitBlocks[i]->dump();
			}
		}

		if (node->getNode() == NULL)
		{
			continue;
		}
		Instruction *ins = node->getNode();

		//		for (auto &B : F){
		//			BasicBlock* BB = dyn_cast<BasicBlock>(&B);
		//			for(auto &I : *BB){
		//
		//			Instruction* ins = &I;

		//			LLVM_DEBUG(dbgs() << "GEPInvestigate node found\n";

		if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(ins))
		{
			const DataLayout DL = ins->getParent()->getParent()->getParent()->getDataLayout();

			//number of elements
			LLVM_DEBUG(dbgs() << "Pointer operand = " << GEP->getPointerOperand()->getName() << "\n");
			LLVM_DEBUG(GEP->dump());
			Type *T = GEP->getSourceElementType();

			LLVM_DEBUG(dbgs() << "S/A/I/P=" << T->isStructTy() << "/");
			LLVM_DEBUG(dbgs() << T->isArrayTy() << "/");
			LLVM_DEBUG(dbgs() << T->isIntegerTy() << "/");
			LLVM_DEBUG(dbgs() << T->isPointerTy() << "\n");

			if (dyn_cast<StructType>(T))
			{

				if (handledGEPs.find(GEP->getPointerOperand()) != handledGEPs.end())
				{
					continue;
				}

				StructType *ST = dyn_cast<StructType>(T);
				LLVM_DEBUG(dbgs() << "StructType=" << ST->getName() << "\n");

				//				//Insert a call to our function
				//				builder(loopStartIns);
				//				builder.SetInsertPoint(loopStartIns);
				builder.SetInsertPoint(loopHeader, loopHeader->getFirstInsertionPt());
				//				builder.SetInsertPoint(loopHeader,++builder.GetInsertPoint());

				Value *old = GEP->getPointerOperand();
				Value *bitcastedPtr = builder.CreateBitCast(old, Type::getInt8PtrTy(Ctx));
				int size = DL.getTypeAllocSize(ST);

				(*sizeArrMap)[GEP->getPointerOperand()->getName().str()] = size;

				if (allocatedArraysMap.find(GEP->getPointerOperand()->getName().str()) == allocatedArraysMap.end())
				{
					dfgNode *memopChild = node->getChildren()[0];
					assert(memopChild->getLeftAlignedMemOp() != 0);
					if (memopChild->getLeftAlignedMemOp() == 1)
					{
						node->setGEPbaseAddr(arrayAddrPtrLeft);
						LLVM_DEBUG(dbgs() << "NodeIdx:" << node->getIdx() << ",addr=" << arrayAddrPtrLeft << ",size=" << size << "\n");
						allocatedArraysMap[GEP->getPointerOperand()->getName().str()] = arrayAddrPtrLeft;
						arrayAddrPtrLeft += size;
					}
					else
					{
						node->setGEPbaseAddr(arrayAddrPtrRight);
						LLVM_DEBUG(dbgs() << "NodeIdx:" << node->getIdx() << ",addr=" << arrayAddrPtrRight << ",size=" << size << "\n");
						allocatedArraysMap[GEP->getPointerOperand()->getName().str()] = arrayAddrPtrRight;
						arrayAddrPtrRight += size;
					}
				}
				else
				{
					node->setGEPbaseAddr(allocatedArraysMap[GEP->getPointerOperand()->getName().str()]);
					LLVM_DEBUG(dbgs() << "NodeIdx:" << node->getIdx() << ",addr=" << allocatedArraysMap[GEP->getPointerOperand()->getName().str()] << ",size=" << size << "\n");
				}

				auto printArrFunc = F.getParent()->getOrInsertFunction(
						"printArr",
						FunctionType::getVoidTy(Ctx),
						Type::getInt8PtrTy(Ctx),
						Type::getInt8PtrTy(Ctx),
						Type::getInt32Ty(Ctx),
						Type::getInt8Ty(Ctx),
						Type::getInt32Ty(Ctx));

				Value *st_name = builder.CreateGlobalStringPtr(ST->getName());
				Value *argsi[] = {st_name,
						bitcastedPtr,
						ConstantInt::get(Type::getInt32Ty(Ctx), size),
						ConstantInt::get(Type::getInt8Ty(Ctx), 1),
						ConstantInt::get(Type::getInt32Ty(Ctx), node->getGEPbaseAddr())};
				Value *argso[] = {st_name,
						bitcastedPtr,
						ConstantInt::get(Type::getInt32Ty(Ctx), size),
						ConstantInt::get(Type::getInt8Ty(Ctx), 0),
						ConstantInt::get(Type::getInt32Ty(Ctx), node->getGEPbaseAddr())};
				//				builder.CreateCall(printArrFunc,args);

				builder.CreateCall(printArrFunc, argsi);

				//				for (int i = 0; i < loopExitInsVec.size(); ++i) {
				//					builder.SetInsertPoint(loopExitInsVec[i]);
				//					builder.CreateCall(printArrFunc,argso);
				//				}

				for (int i = 0; i < loopExitBlocks.size(); ++i)
				{
					builder.SetInsertPoint(loopExitBlocks[i], --loopExitBlocks[i]->end());
					builder.CreateCall(printArrFunc, argso);
				}

				//experiment
				//				LLVM_DEBUG(dbgs() << "Experiment, arraytype size = " << AT->getArrayNumElements() << "\n";
			}
			else
			{
				if (dyn_cast<ArrayType>(T))
				{

					if (handledGEPs.find(GEP->getPointerOperand()) != handledGEPs.end())
					{
						continue;
					}

					ArrayType *AT = dyn_cast<ArrayType>(T);
					LLVM_DEBUG(dbgs() << "ArrayType=" << AT->getArrayNumElements() << "\n");
					LLVM_DEBUG(dbgs() << "Size = " << DL.getTypeAllocSize(AT) << "\n");

					//					//Insert a call to our function
					//					IRBuilder<> builder(loopStartIns);
					//					builder.SetInsertPoint(loopStartIns);
					builder.SetInsertPoint(loopHeader, loopHeader->getFirstInsertionPt());

					Value *old = GEP->getPointerOperand();
					Value *bitcastedPtr = builder.CreateBitCast(old, Type::getInt8PtrTy(Ctx));
					int size = DL.getTypeAllocSize(AT);

					(*sizeArrMap)[GEP->getPointerOperand()->getName().str()] = size;
					dfgNode *memopChild = node->getChildren()[0];
					assert(memopChild->getLeftAlignedMemOp() != 0);
					if (memopChild->getLeftAlignedMemOp() == 1)
					{
						node->setGEPbaseAddr(arrayAddrPtrLeft);
						LLVM_DEBUG(dbgs() << "NodeIdx:" << node->getIdx() << ",addr=" << arrayAddrPtrLeft << ",size=" << size << "\n");
						arrayAddrPtrLeft += size;
					}
					else
					{
						node->setGEPbaseAddr(arrayAddrPtrRight);
						LLVM_DEBUG(dbgs() << "NodeIdx:" << node->getIdx() << ",addr=" << arrayAddrPtrRight << ",size=" << size << "\n");
						arrayAddrPtrRight += size;
					}

					auto printArrFunc = F.getParent()->getOrInsertFunction(
							"printArr",
							FunctionType::getVoidTy(Ctx),
							Type::getInt8PtrTy(Ctx),
							Type::getInt8PtrTy(Ctx),
							Type::getInt32Ty(Ctx),
							Type::getInt8Ty(Ctx),
							Type::getInt32Ty(Ctx));

					Value *st_name = builder.CreateGlobalStringPtr(GEP->getPointerOperand()->getName());
					Value *argsi[] = {st_name,
							bitcastedPtr,
							ConstantInt::get(Type::getInt32Ty(Ctx), size),
							ConstantInt::get(Type::getInt8Ty(Ctx), 1),
							ConstantInt::get(Type::getInt32Ty(Ctx), node->getGEPbaseAddr())};
					Value *argso[] = {st_name,
							bitcastedPtr,
							ConstantInt::get(Type::getInt32Ty(Ctx), size),
							ConstantInt::get(Type::getInt8Ty(Ctx), 0),
							ConstantInt::get(Type::getInt32Ty(Ctx), node->getGEPbaseAddr())};

					//Add a call in the begginning of the loop
					builder.CreateCall(printArrFunc, argsi);

					//					LLVM_DEBUG(dbgs() << "loopExitInsVec.size()=" << loopExitInsVec.size() << "\n";
					//					for (int i = 0; i < loopExitInsVec.size(); ++i) {
					//						loopExitInsVec[i]->dump();
					//						builder.SetInsertPoint(loopExitInsVec[i]);
					//						builder.CreateCall(printArrFunc,argso);
					//					}

					for (int i = 0; i < loopExitBlocks.size(); ++i)
					{
						builder.SetInsertPoint(loopExitBlocks[i], --loopExitBlocks[i]->end());
						builder.CreateCall(printArrFunc, argso);
					}
				}
				else
				{
					assert(dyn_cast<IntegerType>(T));
					IntegerType *IT = dyn_cast<IntegerType>(T);

					//					IRBuilder<> builder(GEP);
					builder.SetInsertPoint(GEP);
					//					builder.SetInsertPoint(loopHeader,++builder.GetInsertPoint());

					Value *old = GEP->getPointerOperand();
					Value *addr = GEP;
					Value *bitcastedPtr = builder.CreateBitCast(old, Type::getInt8PtrTy(Ctx));
					int size = DL.getTypeAllocSize(IT);

					auto reportDynArrSize = F.getParent()->getOrInsertFunction(
							"reportDynArrSize", FunctionType::getVoidTy(Ctx),
							Type::getInt8PtrTy(Ctx),
							Type::getInt8PtrTy(Ctx),
							Type::getInt32Ty(Ctx),
							Type::getInt32Ty(Ctx));

					Value *st_name = builder.CreateGlobalStringPtr(GEP->getPointerOperand()->getName());
					Value *args[] = {st_name,
							bitcastedPtr,
							GEP->getOperand(1),
							ConstantInt::get(Type::getInt32Ty(Ctx),
									size)};
					builder.CreateCall(reportDynArrSize, args);

					std::string ptrName = GEP->getPointerOperand()->getName().str();

					//checking for user defined input sizes
					if (sizeArrMap->find(ptrName) != sizeArrMap->end())
					{
						//						builder.SetInsertPoint(loopStartIns);
						//						builder.SetInsertPoint(loopHeader,++builder.GetInsertPoint());
						int size = (*sizeArrMap)[ptrName];

						if (allocatedArraysMap.find(ptrName) == allocatedArraysMap.end())
						{
							dfgNode *memopChild = node->getChildren()[0];
							assert(memopChild->getLeftAlignedMemOp() != 0);
							if (memopChild->getLeftAlignedMemOp() == 1)
							{
								node->setGEPbaseAddr(arrayAddrPtrLeft);
								LLVM_DEBUG(dbgs() << "NodeIdx:" << node->getIdx() << ",addr=" << arrayAddrPtrLeft << ",size=" << size << "\n");
								allocatedArraysMap[ptrName] = arrayAddrPtrLeft;
								arrayAddrPtrLeft += size;
							}
							else
							{
								node->setGEPbaseAddr(arrayAddrPtrRight);
								LLVM_DEBUG(dbgs() << "NodeIdx:" << node->getIdx() << ",addr=" << arrayAddrPtrRight << ",size=" << size << "\n");
								allocatedArraysMap[ptrName] = arrayAddrPtrRight;
								arrayAddrPtrRight += size;
							}
						}
						else
						{
							node->setGEPbaseAddr(allocatedArraysMap[ptrName]);
							LLVM_DEBUG(dbgs() << "NodeIdx:" << node->getIdx() << ",addr=" << allocatedArraysMap[ptrName] << ",size=" << size << "\n");
						}

						auto printArrFunc = F.getParent()->getOrInsertFunction(
								"printArr",
								FunctionType::getVoidTy(Ctx),
								Type::getInt8PtrTy(Ctx),
								Type::getInt8PtrTy(Ctx),
								Type::getInt32Ty(Ctx),
								Type::getInt8Ty(Ctx),
								Type::getInt32Ty(Ctx));

						Value *argsi[] = {st_name,
								bitcastedPtr,
								ConstantInt::get(Type::getInt32Ty(Ctx), size),
								ConstantInt::get(Type::getInt8Ty(Ctx), 1),
								ConstantInt::get(Type::getInt32Ty(Ctx), node->getGEPbaseAddr())};
						Value *argso[] = {st_name,
								bitcastedPtr,
								ConstantInt::get(Type::getInt32Ty(Ctx), size),
								ConstantInt::get(Type::getInt8Ty(Ctx), 0),
								ConstantInt::get(Type::getInt32Ty(Ctx), node->getGEPbaseAddr())};
						//Add a call in the begginning of the loop
						builder.CreateCall(printArrFunc, argsi);
						//
						//						LLVM_DEBUG(dbgs() << "loopExitInsVec.size()=" << loopExitInsVec.size() << "\n";
						//						for (int i = 0; i < loopExitInsVec.size(); ++i) {
						//							loopExitInsVec[i]->dump();
						//							builder.SetInsertPoint(loopExitInsVec[i]);
						//							builder.CreateCall(printArrFunc,argso);
						//						}

						for (int i = 0; i < loopExitBlocks.size(); ++i)
						{
							builder.SetInsertPoint(loopExitBlocks[i], --loopExitBlocks[i]->end());
							builder.CreateCall(printArrFunc, argso);
						}
					}
					else
					{
						LLVM_DEBUG(dbgs() << "Please provide sizes for the arrayptr : " << ptrName << "\n");
						assert(0);
					}
				}
			}

			LLVM_DEBUG(dbgs() << "GEPInvestigate : instrument code added!\n");
			handledGEPs.insert(GEP->getPointerOperand());
		}

		//		} // for(auto &I : *BB){
		//	} //for (auto &B : F){

	} //Nodelist

	auto printDynArrSize = F.getParent()->getOrInsertFunction(
			"printDynArrSize", FunctionType::getVoidTy(Ctx));

	auto loopEndFn = F.getParent()->getOrInsertFunction(
			"loopEnd", FunctionType::getVoidTy(Ctx), Type::getInt8PtrTy(Ctx));

	//	for (int i = 0; i < loopExitInsVec.size(); ++i) {
	//		builder.SetInsertPoint(loopExitInsVec[i]);
	//		builder.CreateCall(clearPrintedArrs);
	//		builder.CreateCall(loopEndFn,{loopName});
	//	}

	for (int i = 0; i < loopExitBlocks.size(); ++i)
	{
		builder.SetInsertPoint(loopExitBlocks[i], --loopExitBlocks[i]->end());
		builder.CreateCall(clearPrintedArrs);
		builder.CreateCall(loopEndFn, {loopName});
	}

} //End of GEPInvestigate Function

//2017 New version for mapping unit

void DFG::GEPInvestigate(Function &F, std::map<std::string, int> *sizeArrMap)
{

	LLVMContext &Ctx = F.getContext();
	std::set<Value *> handledGEPs;

	//LoopPreHeader Printf marker insertion
	BasicBlock *loopPH = L->getLoopPreheader();
	BasicBlock::iterator instIterPH = loopPH->begin();
	Instruction *loopPHStartIns = &*instIterPH;

	IRBuilder<> builder(loopPHStartIns);
	builder.SetInsertPoint(loopPH, loopPH->getFirstInsertionPt());

	Value *printfSTARTstr = builder.CreateGlobalStringPtr((*this->loopNamesPtr)[L] + ":------------LOOP START--------------\n");
	Value *printfENDstr = builder.CreateGlobalStringPtr((*this->loopNamesPtr)[L] + ":------------LOOP END----------------\n");
	assert(this->loopNamesPtr->find(L) != this->loopNamesPtr->end());
	Value *loopName = builder.CreateGlobalStringPtr((*this->loopNamesPtr)[L]);

	//	Constant* printfMARKER = F.getParent()->getOrInsertFunction(
	//					  "printf",
	//					  FunctionType::getVoidTy(Ctx),
	//					  Type::getInt8PtrTy(Ctx),
	//					  NULL);

	auto loopStartFn = F.getParent()->getOrInsertFunction(
			"loopStart",
			FunctionType::getVoidTy(Ctx),
			Type::getInt8PtrTy(Ctx));

	builder.CreateCall(loopStartFn, {loopName});

	//the entry location for instrumentation code
	BasicBlock *loopHeader = L->getHeader();
	BasicBlock::iterator instIter = loopHeader->begin();
	Instruction *loopStartIns = &*instIter;

	SmallVector<BasicBlock *, 8> loopExitBlocksSV;
	L->getExitBlocks(loopExitBlocksSV);

	std::set<BasicBlock *> s;
	for (unsigned i = 0; i < loopExitBlocksSV.size(); ++i)
		s.insert(loopExitBlocksSV[i]);
	std::vector<BasicBlock *> loopExitBlocks;
	loopExitBlocks.assign(s.begin(), s.end());

	std::vector<Instruction *> loopExitInsVec;
	assert(loopExitBlocks.size() != 0);
	for (int i = 0; i < loopExitBlocks.size(); ++i)
	{
		BasicBlock *LoopExitBB = loopExitBlocks[i];
		//		BasicBlock::iterator instIter = --LoopExitBB->end();
		BasicBlock::iterator instIter = LoopExitBB->begin();
		LLVM_DEBUG(dbgs() << "LoopExit Blocks : \n");
		LLVM_DEBUG(LoopExitBB->dump());

		if (std::find(loopExitInsVec.begin(), loopExitInsVec.end(), &*instIter) == loopExitInsVec.end())
		{
			loopExitInsVec.push_back(&*instIter);
		}
	}

	//	IRBuilder<> builder(loopStartIns);
	//	builder.SetInsertPoint(loopStartIns);
	builder.SetInsertPoint(loopHeader, loopHeader->getFirstInsertionPt());
	auto clearPrintedArrs = F.getParent()->getOrInsertFunction(
			"clearPrintedArrs", FunctionType::getVoidTy(Ctx));

	//	for (int i = 0; i < loopExitInsVec.size(); ++i) {
	//		builder.SetInsertPoint(loopExitInsVec[i]);
	//		builder.CreateCall(clearPrintedArrs);
	//	}

	for (int i = 0; i < loopExitBlocks.size(); ++i)
	{
		builder.SetInsertPoint(loopExitBlocks[i], --loopExitBlocks[i]->end());
		builder.CreateCall(clearPrintedArrs);
	}

	dfgNode *node;
	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];

		if (node->getNameType().compare("OutLoopLOAD") == 0)
		{
			if (!node->isTransferedByHost())
			{
				continue;
			}
			LLVM_DEBUG(dbgs() << "OutLoopLoad found!!\n");
			Value *printfstr = builder.CreateGlobalStringPtr("OutLoopLoadNode:%d,val=%d,addr=%d\n");
			assert(OutLoopNodeMapReverse[node] != NULL);
			LLVM_DEBUG(OutLoopNodeMapReverse[node]->dump());
			Value *loadVal = OutLoopNodeMapReverse[node];
			Value *nodeIdx = ConstantInt::get(Type::getInt32Ty(Ctx), node->getIdx());
			Value *addrVal = ConstantInt::get(Type::getInt32Ty(Ctx), node->getoutloopAddr());
			//			Value* args[] = {printfstr,nodeIdx,loadVal,addrVal};

			//			Constant* printf = F.getParent()->getOrInsertFunction(
			//							  "printf",
			//							  FunctionType::getVoidTy(Ctx),
			//							  Type::getInt8PtrTy(Ctx),
			//							  Type::getInt32Ty(Ctx),
			//							  loadVal->getType(),
			//							  Type::getInt32Ty(Ctx),
			//							  NULL);

			auto outloopReportFn = F.getParent()->getOrInsertFunction(
					"outloopValueReport",
					FunctionType::getVoidTy(Ctx),
					Type::getInt32Ty(Ctx), //nodeIdx
					loadVal->getType(),	//value
					Type::getInt32Ty(Ctx), //addr
					Type::getInt8Ty(Ctx),  //isLoad
					Type::getInt8Ty(Ctx),  //isHostTrans
					Type::getInt8Ty(Ctx)   //size
			);

			Value *isLoad = ConstantInt::get(Type::getInt8Ty(Ctx), 1);
			Value *isHostTrans;
			Value *size = ConstantInt::get(Type::getInt8Ty(Ctx), node->getTypeSizeBytes());
			if (node->isTransferedByHost())
			{
				isHostTrans = ConstantInt::get(Type::getInt8Ty(Ctx), 1);
			}
			else
			{
				isHostTrans = ConstantInt::get(Type::getInt8Ty(Ctx), 0);
			}
			Value *args[] = {nodeIdx, loadVal, addrVal, isLoad, isHostTrans, size};
			for (int i = 0; i < loopExitBlocks.size(); ++i)
			{
				builder.SetInsertPoint(loopExitBlocks[i], --loopExitBlocks[i]->end());
				//				builder.CreateCall(printf,args);
				builder.CreateCall(outloopReportFn, args);
				//				LLVM_DEBUG(dbgs() << "its a load\n";
				//				loopExitBlocks[i]->dump();
			}
		}

		if (node->getNameType().compare("OutLoopSTORE") == 0)
		{
			if (!node->isTransferedByHost())
			{
				continue;
			}
			Value *printfstr = builder.CreateGlobalStringPtr("OutLoopStoreNode:%d,val=%d,addr=%d\n");
			assert(node->getAncestors().size() == 1);
			Value *StoreVal = (node->getAncestors())[0]->getNode();
			Value *nodeIdx = ConstantInt::get(Type::getInt32Ty(Ctx), node->getIdx());
			Value *addrVal = ConstantInt::get(Type::getInt32Ty(Ctx), node->getoutloopAddr());
			//			Value* args[] = {printfstr,nodeIdx,StoreVal,addrVal};

			//			Constant* printf = F.getParent()->getOrInsertFunction(
			//							  "printf",
			//							  FunctionType::getVoidTy(Ctx),
			//							  Type::getInt8PtrTy(Ctx),
			//							  Type::getInt32Ty(Ctx),
			//							  StoreVal->getType(),
			//							  Type::getInt32Ty(Ctx),
			//							  NULL);

			auto outloopReportFn = F.getParent()->getOrInsertFunction(
					"outloopValueReport",
					FunctionType::getVoidTy(Ctx),
					Type::getInt32Ty(Ctx), //nodeIdx
					StoreVal->getType(),   //value
					Type::getInt32Ty(Ctx), //addr
					Type::getInt8Ty(Ctx),  //isLoad
					Type::getInt8Ty(Ctx),  //isHostTrans
					Type::getInt8Ty(Ctx)   //size
			);

			Value *isLoad = ConstantInt::get(Type::getInt8Ty(Ctx), 0);
			Value *isHostTrans;
			Value *size = ConstantInt::get(Type::getInt8Ty(Ctx), node->getTypeSizeBytes());
			if (node->isTransferedByHost())
			{
				isHostTrans = ConstantInt::get(Type::getInt8Ty(Ctx), 1);
			}
			else
			{
				isHostTrans = ConstantInt::get(Type::getInt8Ty(Ctx), 0);
			}
			Value *args[] = {nodeIdx, StoreVal, addrVal, isLoad, isHostTrans, size};
			for (int i = 0; i < loopExitBlocks.size(); ++i)
			{
				builder.SetInsertPoint(loopExitBlocks[i], --loopExitBlocks[i]->end());
				//				builder.CreateCall(printf,args);
				builder.CreateCall(outloopReportFn, args);
				//				LLVM_DEBUG(dbgs() << "its a store\n";
				//				OutLoopNodeMapReverse[node]->dump();
				//				loopExitBlocks[i]->dump();
			}
		}

		if (node->getNode() == NULL)
		{
			continue;
		}
		Instruction *ins = node->getNode();

		//		for (auto &B : F){
		//			BasicBlock* BB = dyn_cast<BasicBlock>(&B);
		//			for(auto &I : *BB){
		//
		//			Instruction* ins = &I;

		//			LLVM_DEBUG(dbgs() << "GEPInvestigate node found\n";

		if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(ins))
		{
			const DataLayout DL = ins->getParent()->getParent()->getParent()->getDataLayout();

			//number of elements
			LLVM_DEBUG(dbgs() << "Pointer operand = " << GEP->getPointerOperand()->getName() << "\n");
			LLVM_DEBUG(GEP->dump());
			Type *T = GEP->getSourceElementType();

			LLVM_DEBUG(dbgs() << "S/A/I/P=" << T->isStructTy() << "/");
			LLVM_DEBUG(dbgs() << T->isArrayTy() << "/");
			LLVM_DEBUG(dbgs() << T->isIntegerTy() << "/");
			LLVM_DEBUG(dbgs() << T->isPointerTy() << "\n");

			if (dyn_cast<StructType>(T))
			{

				if (handledGEPs.find(GEP->getPointerOperand()) != handledGEPs.end())
				{
					continue;
				}

				StructType *ST = dyn_cast<StructType>(T);
				LLVM_DEBUG(dbgs() << "StructType=" << ST->getName() << "\n");

				//				//Insert a call to our function
				//				builder(loopStartIns);
				//				builder.SetInsertPoint(loopStartIns);
				builder.SetInsertPoint(loopHeader, loopHeader->getFirstInsertionPt());
				//				builder.SetInsertPoint(loopHeader,++builder.GetInsertPoint());

				Value *old = GEP->getPointerOperand();
				Value *bitcastedPtr = builder.CreateBitCast(old, Type::getInt8PtrTy(Ctx));
				int size = DL.getTypeAllocSize(ST);

				(*sizeArrMap)[GEP->getPointerOperand()->getName().str()] = size;

				if (allocatedArraysMap.find(GEP->getPointerOperand()->getName().str()) == allocatedArraysMap.end())
				{
					dfgNode *memopChild = node->getChildren()[0];
					assert(memopChild->getLeftAlignedMemOp() != 0);
					if (memopChild->getLeftAlignedMemOp() == 1)
					{
						node->setGEPbaseAddr(arrayAddrPtrLeft);
						LLVM_DEBUG(dbgs() << "NodeIdx:" << node->getIdx() << ",addr=" << arrayAddrPtrLeft << ",size=" << size << "\n");
						allocatedArraysMap[GEP->getPointerOperand()->getName().str()] = arrayAddrPtrLeft;
						arrayAddrPtrLeft += size;
					}
					else
					{
						node->setGEPbaseAddr(arrayAddrPtrRight);
						LLVM_DEBUG(dbgs() << "NodeIdx:" << node->getIdx() << ",addr=" << arrayAddrPtrRight << ",size=" << size << "\n");
						allocatedArraysMap[GEP->getPointerOperand()->getName().str()] = arrayAddrPtrRight;
						arrayAddrPtrRight += size;
					}
				}
				else
				{
					node->setGEPbaseAddr(allocatedArraysMap[GEP->getPointerOperand()->getName().str()]);
					LLVM_DEBUG(dbgs() << "NodeIdx:" << node->getIdx() << ",addr=" << allocatedArraysMap[GEP->getPointerOperand()->getName().str()] << ",size=" << size << "\n");
				}

				auto printArrFunc = F.getParent()->getOrInsertFunction(
						"printArr",
						FunctionType::getVoidTy(Ctx),
						Type::getInt8PtrTy(Ctx),
						Type::getInt8PtrTy(Ctx),
						Type::getInt32Ty(Ctx),
						Type::getInt8Ty(Ctx),
						Type::getInt32Ty(Ctx));

				Value *st_name = builder.CreateGlobalStringPtr(ST->getName());
				Value *argsi[] = {st_name,
						bitcastedPtr,
						ConstantInt::get(Type::getInt32Ty(Ctx), size),
						ConstantInt::get(Type::getInt8Ty(Ctx), 1),
						ConstantInt::get(Type::getInt32Ty(Ctx), node->getGEPbaseAddr())};
				Value *argso[] = {st_name,
						bitcastedPtr,
						ConstantInt::get(Type::getInt32Ty(Ctx), size),
						ConstantInt::get(Type::getInt8Ty(Ctx), 0),
						ConstantInt::get(Type::getInt32Ty(Ctx), node->getGEPbaseAddr())};
				//				builder.CreateCall(printArrFunc,args);

				builder.CreateCall(printArrFunc, argsi);

				//				for (int i = 0; i < loopExitInsVec.size(); ++i) {
				//					builder.SetInsertPoint(loopExitInsVec[i]);
				//					builder.CreateCall(printArrFunc,argso);
				//				}

				for (int i = 0; i < loopExitBlocks.size(); ++i)
				{
					builder.SetInsertPoint(loopExitBlocks[i], --loopExitBlocks[i]->end());
					builder.CreateCall(printArrFunc, argso);
				}

				//experiment
				//				LLVM_DEBUG(dbgs() << "Experiment, arraytype size = " << AT->getArrayNumElements() << "\n";
			}
			else
			{
				if (dyn_cast<ArrayType>(T))
				{

					if (handledGEPs.find(GEP->getPointerOperand()) != handledGEPs.end())
					{
						continue;
					}

					ArrayType *AT = dyn_cast<ArrayType>(T);
					LLVM_DEBUG(dbgs() << "ArrayType=" << AT->getArrayNumElements() << "\n");
					LLVM_DEBUG(dbgs() << "Size = " << DL.getTypeAllocSize(AT) << "\n");

					//					//Insert a call to our function
					//					IRBuilder<> builder(loopStartIns);
					//					builder.SetInsertPoint(loopStartIns);
					builder.SetInsertPoint(loopHeader, loopHeader->getFirstInsertionPt());

					Value *old = GEP->getPointerOperand();
					Value *bitcastedPtr = builder.CreateBitCast(old, Type::getInt8PtrTy(Ctx));
					int size = DL.getTypeAllocSize(AT);

					(*sizeArrMap)[GEP->getPointerOperand()->getName().str()] = size;
					dfgNode *memopChild = node->getChildren()[0];
					assert(memopChild->getLeftAlignedMemOp() != 0);
					if (memopChild->getLeftAlignedMemOp() == 1)
					{
						node->setGEPbaseAddr(arrayAddrPtrLeft);
						LLVM_DEBUG(dbgs() << "NodeIdx:" << node->getIdx() << ",addr=" << arrayAddrPtrLeft << ",size=" << size << "\n");
						arrayAddrPtrLeft += size;
					}
					else
					{
						node->setGEPbaseAddr(arrayAddrPtrRight);
						LLVM_DEBUG(dbgs() << "NodeIdx:" << node->getIdx() << ",addr=" << arrayAddrPtrRight << ",size=" << size << "\n");
						arrayAddrPtrRight += size;
					}

					auto printArrFunc = F.getParent()->getOrInsertFunction(
							"printArr",
							FunctionType::getVoidTy(Ctx),
							Type::getInt8PtrTy(Ctx),
							Type::getInt8PtrTy(Ctx),
							Type::getInt32Ty(Ctx),
							Type::getInt8Ty(Ctx),
							Type::getInt32Ty(Ctx));

					Value *st_name = builder.CreateGlobalStringPtr(GEP->getPointerOperand()->getName());
					Value *argsi[] = {st_name,
							bitcastedPtr,
							ConstantInt::get(Type::getInt32Ty(Ctx), size),
							ConstantInt::get(Type::getInt8Ty(Ctx), 1),
							ConstantInt::get(Type::getInt32Ty(Ctx), node->getGEPbaseAddr())};
					Value *argso[] = {st_name,
							bitcastedPtr,
							ConstantInt::get(Type::getInt32Ty(Ctx), size),
							ConstantInt::get(Type::getInt8Ty(Ctx), 0),
							ConstantInt::get(Type::getInt32Ty(Ctx), node->getGEPbaseAddr())};

					//Add a call in the begginning of the loop
					builder.CreateCall(printArrFunc, argsi);

					//					LLVM_DEBUG(dbgs() << "loopExitInsVec.size()=" << loopExitInsVec.size() << "\n";
					//					for (int i = 0; i < loopExitInsVec.size(); ++i) {
					//						loopExitInsVec[i]->dump();
					//						builder.SetInsertPoint(loopExitInsVec[i]);
					//						builder.CreateCall(printArrFunc,argso);
					//					}

					for (int i = 0; i < loopExitBlocks.size(); ++i)
					{
						builder.SetInsertPoint(loopExitBlocks[i], --loopExitBlocks[i]->end());
						builder.CreateCall(printArrFunc, argso);
					}
				}
				else
				{
					assert(dyn_cast<IntegerType>(T));
					IntegerType *IT = dyn_cast<IntegerType>(T);

					//					IRBuilder<> builder(GEP);
					builder.SetInsertPoint(GEP);
					//					builder.SetInsertPoint(loopHeader,++builder.GetInsertPoint());

					Value *old = GEP->getPointerOperand();
					Value *addr = GEP;
					Value *bitcastedPtr = builder.CreateBitCast(old, Type::getInt8PtrTy(Ctx));
					int size = DL.getTypeAllocSize(IT);

					auto reportDynArrSize = F.getParent()->getOrInsertFunction(
							"reportDynArrSize", FunctionType::getVoidTy(Ctx),
							Type::getInt8PtrTy(Ctx),
							Type::getInt8PtrTy(Ctx),
							Type::getInt32Ty(Ctx),
							Type::getInt32Ty(Ctx));

					Value *st_name = builder.CreateGlobalStringPtr(GEP->getPointerOperand()->getName());
					Value *args[] = {st_name,
							bitcastedPtr,
							GEP->getOperand(1),
							ConstantInt::get(Type::getInt32Ty(Ctx),
									size)};
					builder.CreateCall(reportDynArrSize, args);

					std::string ptrName = GEP->getPointerOperand()->getName().str();

					//checking for user defined input sizes
					if (sizeArrMap->find(ptrName) != sizeArrMap->end())
					{
						//						builder.SetInsertPoint(loopStartIns);
						//						builder.SetInsertPoint(loopHeader,++builder.GetInsertPoint());
						int size = (*sizeArrMap)[ptrName];

						if (allocatedArraysMap.find(ptrName) == allocatedArraysMap.end())
						{
							dfgNode *memopChild = node->getChildren()[0];
							assert(memopChild->getLeftAlignedMemOp() != 0);
							if (memopChild->getLeftAlignedMemOp() == 1)
							{
								node->setGEPbaseAddr(arrayAddrPtrLeft);
								LLVM_DEBUG(dbgs() << "NodeIdx:" << node->getIdx() << ",addr=" << arrayAddrPtrLeft << ",size=" << size << "\n");
								allocatedArraysMap[ptrName] = arrayAddrPtrLeft;
								arrayAddrPtrLeft += size;
							}
							else
							{
								node->setGEPbaseAddr(arrayAddrPtrRight);
								LLVM_DEBUG(dbgs() << "NodeIdx:" << node->getIdx() << ",addr=" << arrayAddrPtrRight << ",size=" << size << "\n");
								allocatedArraysMap[ptrName] = arrayAddrPtrRight;
								arrayAddrPtrRight += size;
							}
						}
						else
						{
							node->setGEPbaseAddr(allocatedArraysMap[ptrName]);
							LLVM_DEBUG(dbgs() << "NodeIdx:" << node->getIdx() << ",addr=" << allocatedArraysMap[ptrName] << ",size=" << size << "\n");
						}

						auto printArrFunc = F.getParent()->getOrInsertFunction(
								"printArr",
								FunctionType::getVoidTy(Ctx),
								Type::getInt8PtrTy(Ctx),
								Type::getInt8PtrTy(Ctx),
								Type::getInt32Ty(Ctx),
								Type::getInt8Ty(Ctx),
								Type::getInt32Ty(Ctx));

						Value *argsi[] = {st_name,
								bitcastedPtr,
								ConstantInt::get(Type::getInt32Ty(Ctx), size),
								ConstantInt::get(Type::getInt8Ty(Ctx), 1),
								ConstantInt::get(Type::getInt32Ty(Ctx), node->getGEPbaseAddr())};
						Value *argso[] = {st_name,
								bitcastedPtr,
								ConstantInt::get(Type::getInt32Ty(Ctx), size),
								ConstantInt::get(Type::getInt8Ty(Ctx), 0),
								ConstantInt::get(Type::getInt32Ty(Ctx), node->getGEPbaseAddr())};
						//Add a call in the begginning of the loop
						builder.CreateCall(printArrFunc, argsi);
						//
						//						LLVM_DEBUG(dbgs() << "loopExitInsVec.size()=" << loopExitInsVec.size() << "\n";
						//						for (int i = 0; i < loopExitInsVec.size(); ++i) {
						//							loopExitInsVec[i]->dump();
						//							builder.SetInsertPoint(loopExitInsVec[i]);
						//							builder.CreateCall(printArrFunc,argso);
						//						}

						for (int i = 0; i < loopExitBlocks.size(); ++i)
						{
							builder.SetInsertPoint(loopExitBlocks[i], --loopExitBlocks[i]->end());
							builder.CreateCall(printArrFunc, argso);
						}
					}
					else
					{
						LLVM_DEBUG(dbgs() << "Please provide sizes for the arrayptr : " << ptrName << "\n");
						assert(0);
					}
				}
			}

			LLVM_DEBUG(dbgs() << "GEPInvestigate : instrument code added!\n");
			handledGEPs.insert(GEP->getPointerOperand());
		}

		//		} // for(auto &I : *BB){
		//	} //for (auto &B : F){

	} //Nodelist

	auto printDynArrSize = F.getParent()->getOrInsertFunction(
			"printDynArrSize", FunctionType::getVoidTy(Ctx));

	auto loopEndFn = F.getParent()->getOrInsertFunction(
			"loopEnd", FunctionType::getVoidTy(Ctx), Type::getInt8PtrTy(Ctx));

	//	for (int i = 0; i < loopExitInsVec.size(); ++i) {
	//		builder.SetInsertPoint(loopExitInsVec[i]);
	//		builder.CreateCall(clearPrintedArrs);
	//		builder.CreateCall(loopEndFn,{loopName});
	//	}

	for (int i = 0; i < loopExitBlocks.size(); ++i)
	{
		builder.SetInsertPoint(loopExitBlocks[i], --loopExitBlocks[i]->end());
		builder.CreateCall(clearPrintedArrs);
		builder.CreateCall(loopEndFn, {loopName});
	}

} //End of GEPInvestigate Function

void DFG::AssignOutLoopAddr()
{

	dfgNode *node;
	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		if ((node->getNameType().compare("OutLoopLOAD") == 0) ||
				(node->getNameType().compare("OutLoopSTORE") == 0))
		{

			assert(node->getMappedLoc() != NULL);

			int bytewidth;
			if (dyn_cast<IntegerType>(OutLoopNodeMapReverse[node]->getType()))
			{
				bytewidth = OutLoopNodeMapReverse[node]->getType()->getIntegerBitWidth() / 8;
			}
			else if (dyn_cast<ArrayType>(OutLoopNodeMapReverse[node]->getType()) ||
					dyn_cast<StructType>(OutLoopNodeMapReverse[node]->getType()) ||
					dyn_cast<PointerType>(OutLoopNodeMapReverse[node]->getType()))
			{
				bytewidth = 4;
			}
			else
			{
				assert(0 && "should not come here");
			}

			if (node->getMappedLoc()->getY() <= 1)
			{
				outloopAddrPtrLeft = outloopAddrPtrLeft - bytewidth;

				if (bytewidth == 4)
				{
					outloopAddrPtrLeft = outloopAddrPtrLeft & (~0b11);
				}
				else if (bytewidth == 2)
				{
					outloopAddrPtrLeft = outloopAddrPtrLeft & (~0b1);
				}

				node->setoutloopAddr(outloopAddrPtrLeft);
				LLVM_DEBUG(dbgs() << "NodeIdx:" << node->getIdx() << ",bytewidth=" << bytewidth << ",addr=" << outloopAddrPtrLeft << "\n");
			}
			else if (node->getMappedLoc()->getY() >= 2)
			{
				outloopAddrPtrRight = outloopAddrPtrRight - bytewidth;

				if (bytewidth == 4)
				{
					outloopAddrPtrRight = outloopAddrPtrRight & (~0b11);
				}
				else if (bytewidth == 2)
				{
					outloopAddrPtrRight = outloopAddrPtrRight & (~0b1);
				}

				node->setoutloopAddr(outloopAddrPtrRight);
				LLVM_DEBUG(dbgs() << "NodeIdx:" << node->getIdx() << ",bytewidth=" << bytewidth << ",addr=" << outloopAddrPtrRight << "\n");
			}
			else
			{
				assert(false);
			}
		}
	}
}

int DFG::phiselectInsert()
{
}

int DFG::PlaceMacro(DFG *mappedDFG, int XDim, int YDim, ArchType arch)
{
	if (this->currCGRA != NULL)
	{
		assert(this->currCGRA->getMapped() == false);
	}
	int II = 0; //this will be one inititally once it gets into the following while loop
	bool macroPlaceSuccess = false;

	while (macroPlaceSuccess == false)
	{
		II++;
		delete currCGRA;
		currCGRA = new CGRA(II, XDim, YDim, REGS_PER_NODE, arch);
		macroPlaceSuccess = currCGRA->PlaceMacro(mappedDFG);
	}

	return II;
}

int DFG::classifyParents()
{
	dfgNode *node;
	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];

		LLVM_DEBUG(dbgs() << "classifyParent::currentNode = " << node->getIdx() << "\n");
		LLVM_DEBUG(dbgs() << "Parents : ");
		for (dfgNode *parent : node->getAncestors())
		{
			LLVM_DEBUG(dbgs() << parent->getIdx() << ",");
		}
		LLVM_DEBUG(dbgs() << "\n");

		for (dfgNode *parent : node->getAncestors())
		{
			Instruction *ins;
			Value *parentIns;

			if (node->getNode() != NULL)
			{
				ins = node->getNode();
			}
			else
			{
				if (node->getNameType().compare("XORNOT") == 0)
				{
					assert(node->parentClassification.find(1) == node->parentClassification.end());
					node->parentClassification[1] = parent;
					continue;
				}
				if (node->getNameType().compare("ORZERO") == 0)
				{
					assert(node->parentClassification.find(1) == node->parentClassification.end());
					node->parentClassification[1] = parent;
					continue;
				}
				if (node->getNameType().compare("GEPLEFTSHIFT") == 0)
				{
					if (parent->getNode() != NULL)
					{
						parentIns = parent->getNode();
						if (BranchInst *BRI = dyn_cast<BranchInst>(parentIns))
						{
							node->parentClassification[0] = parent;
							continue;
						}
						else if (CmpInst *CMPI = dyn_cast<CmpInst>(parentIns))
						{
							node->parentClassification[0] = parent;
							continue;
						}
					}
					assert(node->parentClassification.find(1) == node->parentClassification.end());
					node->parentClassification[1] = parent;
					continue;
				}
				if (node->getNameType().compare("MASKAND") == 0)
				{
					assert(node->parentClassification.find(1) == node->parentClassification.end());
					node->parentClassification[1] = parent;
					continue;
				}
				if (node->getNameType().compare("PREDAND") == 0)
				{
					assert(node->parentClassification.find(1) == node->parentClassification.end());
					node->parentClassification[1] = parent;
					continue;
				}
				if (node->getNameType().compare("CTRLBrOR") == 0)
				{
					if (node->parentClassification.find(1) == node->parentClassification.end())
					{
						node->parentClassification[1] = parent;
					}
					else
					{
						assert(node->parentClassification.find(2) == node->parentClassification.end());
						node->parentClassification[2] = parent;
					}
					continue;
				}
				if (node->getNameType().compare("GEPADD") == 0)
				{
					if (node->parentClassification.find(1) == node->parentClassification.end())
					{
						node->parentClassification[1] = parent;
					}
					else
					{
						assert(node->parentClassification.find(2) == node->parentClassification.end());
						node->parentClassification[2] = parent;
					}
					continue;
				}
				if (node->getNameType().compare("STORESTART") == 0)
				{
					assert(parent->getNode() == NULL);
					if (parent->getNameType().compare("MOVC") == 0)
					{
						node->parentClassification[1] = parent;
					}
					else if (parent->getNameType().compare("LOOPSTART") == 0)
					{
						node->parentClassification[0] = parent;
					}
					else if (parent->getNameType().compare("PREDAND") == 0)
					{
						node->parentClassification[0] = parent;
					}
					else
					{
						assert(false);
					}
					continue;
				}
				if (node->getNameType().compare("LOOPEXIT") == 0)
				{
					if (parent->getNode() == NULL)
					{
						if (parent->getNameType().compare("MOVC") == 0)
						{
							assert(node->parentClassification.find(1) == node->parentClassification.end());
							node->parentClassification[1] = parent;
						}
						else if (parent->getNameType().compare("CTRLBrOR") == 0)
						{
							assert(node->parentClassification.find(0) == node->parentClassification.end());
							node->parentClassification[0] = parent;
						}
						else
						{
							assert(false);
						}
					}
					else
					{
						assert(node->parentClassification.find(0) == node->parentClassification.end());
						node->parentClassification[0] = parent;
					}
					continue;
				}
				if (node->getNameType().compare("OutLoopLOAD") == 0)
				{
					//do nothing
					continue;
				}
				if (node->getNameType().compare("OutLoopSTORE") == 0)
				{
					assert(node->hasConstantVal());

					//check for predicate parent
					if (parent->getNode() != NULL)
					{
						parentIns = parent->getNode();
						if (BranchInst *BRI = dyn_cast<BranchInst>(parentIns))
						{
							node->parentClassification[0] = parent;
							continue;
						}
						else if (CmpInst *CMPI = dyn_cast<CmpInst>(parentIns))
						{
							node->parentClassification[0] = parent;
							continue;
						}
					}
					else
					{
						if (parent->getNameType().compare("CTRLBrOR") == 0)
						{
							node->parentClassification[0] = parent;
							continue;
						}
					}

					//if it comes to this, it has to be a data parent.
					node->parentClassification[1] = parent;
					continue;
				}
				if (node->getNameType().compare("CMERGE") == 0)
				{

					Value *parentIns = NULL;
					if (parent->getNameType().compare("OutLoopLOAD") == 0)
					{
						parentIns = OutLoopNodeMapReverse[parent];
					}
					else
					{
						parentIns = parent->getNode();
					}

					assert(node->getChildren().size() == 1);
					dfgNode *cmergeChild = node->getChildren()[0];
					assert(cmergeChild->getNode() != NULL);
					Instruction *condIns;
					if (SelectInst *SLI = dyn_cast<SelectInst>(cmergeChild->getNode()))
					{
						condIns = dyn_cast<Instruction>(SLI->getCondition());
						assert(condIns);
					}

					if (parentIns != NULL)
					{
						if ((dyn_cast<BranchInst>(parentIns)) || (dyn_cast<CmpInst>(parentIns)))
						{
							assert(node->parentClassification.find(0) == node->parentClassification.end());
							node->parentClassification[0] = parent;
						}
						else if (parentIns == condIns)
						{
							assert(node->parentClassification.find(0) == node->parentClassification.end());
							node->parentClassification[0] = parent;
						}
						else
						{
							if (node->parentClassification.find(1) != node->parentClassification.end())
							{
								LLVM_DEBUG(parentIns->dump());
							}
							assert(node->parentClassification.find(1) == node->parentClassification.end());
							node->parentClassification[1] = parent;
						}
					}
					else
					{
						if ((parent->getNameType().compare("XORNOT") == 0))
						{
							assert(node->parentClassification.find(0) == node->parentClassification.end());
							node->parentClassification[0] = parent;
						}
						else if ((parent->getNameType().compare("ORZERO") == 0))
						{
							assert(node->parentClassification.find(0) == node->parentClassification.end());
							node->parentClassification[0] = parent;
						}
						else if (parent->getNameType().compare("LOOPSTART") == 0)
						{
							assert(node->parentClassification.find(0) == node->parentClassification.end());
							node->parentClassification[0] = parent;
							node->setNPB(true);
						}
						else if (parent->getNameType().compare("PREDAND") == 0)
						{
							assert(node->parentClassification.find(0) == node->parentClassification.end());
							node->parentClassification[0] = parent;
							node->setNPB(true);
						}
						else
						{
							LLVM_DEBUG(dbgs() << "Parent :" << parent->getIdx() << ", classified as I1\n");
							assert(node->parentClassification.find(1) == node->parentClassification.end());
							node->parentClassification[1] = parent;
						}
					}

					//						if(node->parentClassification.find(1) == node->parentClassification.end()){
					//							node->parentClassification[1]=parent;
					//						}
					//						else{
					//							assert(node->parentClassification.find(2) == node->parentClassification.end());
					//							node->parentClassification[2]=parent;
					//						}
					continue;
				}
			}

			if (dyn_cast<PHINode>(ins) || dyn_cast<SelectInst>(ins))
			{
				if (parent->getNode() != NULL)
					LLVM_DEBUG(parent->getNode()->dump());
				LLVM_DEBUG(dbgs() << "node : " << node->getIdx() << "\n");
				LLVM_DEBUG(dbgs() << "Parent : " << parent->getIdx() << "\n");
				LLVM_DEBUG(dbgs() << "Parent NameType : " << parent->getNameType() << "\n");

				if (parent->getNode())
				{
					if (dyn_cast<BranchInst>(parent->getNode()))
					{
						node->parentClassification[0] = parent;
						continue;
					}
				}

				assert(parent->getNameType().compare("CMERGE") == 0);
				if (node->parentClassification.find(1) == node->parentClassification.end())
				{
					node->parentClassification[1] = parent;
				}
				else if (node->parentClassification.find(2) == node->parentClassification.end())
				{
					//						assert(node->parentClassification.find(2) == node->parentClassification.end());
					node->parentClassification[2] = parent;
				}
				else
				{
					LLVM_DEBUG(dbgs() << "Extra parent : " << parent->getIdx() << ",Nametype = " << parent->getNameType() << ",par_idx = " << node->parentClassification.size() + 1 << "\n");
					node->parentClassification[node->parentClassification.size() + 1] = parent;
				}
				continue;
			}

			if (dyn_cast<BranchInst>(ins))
			{
				if (node->parentClassification.find(1) == node->parentClassification.end())
				{
					node->parentClassification[1] = parent;
				}
				else
				{
					assert(node->parentClassification.find(2) == node->parentClassification.end());
					node->parentClassification[2] = parent;
				}
				continue;
			}

			if (parent->getNode() != NULL)
			{
				parentIns = parent->getNode();
				if (BranchInst *BRI = dyn_cast<BranchInst>(parentIns))
				{
					node->parentClassification[0] = parent;
					continue;
				}
				//					else if(CmpInst * CMPI = dyn_cast<CmpInst>(parentIns)){
				//						node->parentClassification[0]=parent;
				//						continue;
				//					}
			}
			else
			{
				if (parent->getNameType().compare("CTRLBrOR") == 0)
				{
					node->parentClassification[0] = parent;
					continue;
				}
				if (parent->getNameType().compare("PREDAND") == 0)
				{
					node->parentClassification[0] = parent;
					continue;
				}
				//					if(parent->getNameType().compare("GEPLEFTSHIFT")==0){
				//						assert(node->parentClassification.find(1)==node->parentClassification.end());
				//						node->parentClassification[1]=parent;
				//						continue;
				//					}
				if (parent->getNameType().compare("GEPLEFTSHIFT") == 0)
				{
					assert(parent->getAncestors().size() < 3);
					node->parentClassification[1] = parent;
					continue;
				}
				if (parent->getNameType().compare("MASKAND") == 0)
				{
					assert(parent->getAncestors().size() == 1);
					parentIns = parent->getAncestors()[0]->getNode();
				}
				if (parent->getNameType().compare("OutLoopLOAD") == 0)
				{
					parentIns = OutLoopNodeMapReverse[parent];
				}
				if (parent->getNameType().compare("CMERGE") == 0)
				{
					if (node->parentClassification.find(1) == node->parentClassification.end())
					{
						node->parentClassification[1] = parent;
					}
					else if (node->parentClassification.find(2) == node->parentClassification.end())
					{
						//						assert(node->parentClassification.find(2) == node->parentClassification.end());
						node->parentClassification[2] = parent;
					}
					else
					{
						LLVM_DEBUG(dbgs() << "Extra parent : " << parent->getIdx() << ",Nametype = " << parent->getNameType() << ",par_idx = " << node->parentClassification.size() + 1 << "\n");
						node->parentClassification[node->parentClassification.size() + 1] = parent;
					}
					continue;
				}
			}
			//Only ins!=NULL and non-PHI and non-BR will reach here
			node->parentClassification[findOperandNumber(node, ins, parentIns)] = parent;
		}
	}
}

int DFG::findOperandNumber(dfgNode *node, Instruction *child, Value *parent)
{
	//	parent->dump();
	//	child->dump();

	if (LoadInst *LDI = dyn_cast<LoadInst>(child))
	{
		assert(parent == LDI->getPointerOperand());
		return 2;
	}
	else if (StoreInst *STI = dyn_cast<StoreInst>(child))
	{
		if (parent == STI->getPointerOperand())
		{
			return 2;
		}
		else if (parent == STI->getValueOperand())
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(child))
	{
		if (node->getAncestors().size() > 1)
		{
			LLVM_DEBUG(dbgs() << "node :");
			LLVM_DEBUG(child->dump());
			for (dfgNode *parentNode : node->getAncestors())
			{
				if (parentNode->getNode())
				{
					LLVM_DEBUG(dbgs() << "parent :");
					LLVM_DEBUG(parentNode->getNode()->dump());
				}
				else
				{
					LLVM_DEBUG(dbgs() << "parent :" << parentNode->getDFSIdx() << "," << parentNode->getNameType() << "\n");
				}
			}

			for (int i = 0; i < child->getNumOperands(); ++i)
			{
				LLVM_DEBUG(child->getOperand(i)->dump());
				if (child->getOperand(i) == parent)
				{
					return i + 1;
				}
			}
		}
		//TODO : please remove this
		assert(node->getAncestors().size() == 1);
		return 1;
	}
	else
	{
		if (child->getNumOperands() == 3)
		{
			for (int i = 0; i < 3; ++i)
			{
				if (Value *chkIns = (child->getOperand(i)))
				{
					if (parent == chkIns)
					{
						return i;
					}
				}
				else
				{
					//					LLVM_DEBUG(dbgs() << "child :\n";
					LLVM_DEBUG(child->dump());
					//					LLVM_DEBUG(dbgs() << "parent :\n";
					LLVM_DEBUG(parent->dump());
					//					LLVM_DEBUG(dbgs() << child->getOperand(i)->getName() << "\n";
					assert(i != 0);
					ConstantInt *CI = cast<ConstantInt>(child->getOperand(i));
					if (i == 1)
					{
						assert(parent == (child->getOperand(2)));
						return 2;
					}
					else
					{ // i==2
						assert(parent == (child->getOperand(1)));
						return 1;
					}
				}
			}
		}
		else if (child->getNumOperands() == 2)
		{
			for (int i = 0; i < 2; ++i)
			{
				if (Value *chkIns = (child->getOperand(i)))
				{
					if (parent == chkIns)
					{
						return i + 1;
					}
				}
				else
				{
					//					ConstantInt* CI = cast<ConstantInt>(child->getOperand(i));
					//					assert(node->hasConstantVal());
					if (i == 0)
					{
						assert(parent == (child->getOperand(1)));
						return 1;
					}
					else
					{ // i==1
						assert(parent == (child->getOperand(0)));
						return 0;
					}
				}
			}
		}
		else if (child->getNumOperands() == 1)
		{
			assert(parent == (child->getOperand(0)));
			return 1;
		}
		else
		{
			assert(false);
		}
	}
}

int DFG::treatFalsePaths()
{
	dfgNode *node;
	int NOTsadded = 0;
	for (int i = 0; i < NodeList.size(); i++)
	{
		node = NodeList[i];
		for (dfgNode *parent : node->getAncestors())
		{
			if (parent->getNode() != NULL)
			{
				if (BranchInst *BRI = dyn_cast<BranchInst>(parent->getNode()))
				{
					if (node->getNameType().compare("CTRLBrOR") == 0)
					{
						if (BRI->isConditional())
						{
							if (node->BB == BRI->getSuccessor(1))
							{
								parent->removeChild(node);
								node->removeAncestor(parent);
								removeEdge(findEdge(parent, node));

								dfgNode *temp = new dfgNode(this);
								temp->setNameType("XORNOT");
								temp->setIdx(NodeList.size());
								temp->BB = node->BB;
								NodeList.push_back(temp);

								parent->addChildNode(temp);
								temp->addAncestorNode(parent);

								temp->addChildNode(node);
								node->addAncestorNode(temp);
								NOTsadded++;
							}
						}
					}
					else
					{
						if (BRI->isConditional())
						{
							if (node->BB == BRI->getSuccessor(1))
							{
								node->setNPB(true);
							}
						}
					}
				}
			}
		}
	}
	LLVM_DEBUG(dbgs() << "treatFalsePaths:: NOTsadded = " << NOTsadded << "\n");
	return 0;
}

int DFG::insertshiftGEPs()
{
	LLVM_DEBUG(dbgs() << "insertshiftGEPs started!\n");
	dfgNode *node;
	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];

		if (node->getNode() != NULL)
		{
			if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(node->getNode()))
			{

				int elementSize = 0;
				LLVM_DEBUG(GEP->dump());
				for (dfgNode *child : node->getChildren())
				{
					if (child->getNode() != NULL)
					{
						if (child->getNode()->mayReadOrWriteMemory())
						{
							elementSize = child->getTypeSizeBytes() / 4;
							LLVM_DEBUG(dbgs() << "GEP element size = " << child->getTypeSizeBytes() / 4 << "\n");
							break;
						}
					}
				}

				std::set<dfgNode *> non_control_parents;
				for (dfgNode *parent : node->getAncestors())
				{
					if (parent->getNameType() == "CTRLBrOR")
					{
						continue;
					}
					else if (parent->getNode() != NULL)
					{
						Instruction *parentIns = parent->getNode();
						if (BranchInst *BRI = dyn_cast<BranchInst>(parentIns))
						{
							continue;
						}
						else if (CmpInst *CMPI = dyn_cast<CmpInst>(parentIns))
						{
							continue;
						}
					}
					non_control_parents.insert(parent);
				}
				assert(non_control_parents.size() <= 2);

				if (non_control_parents.size() == 2)
				{
					dfgNode *tempADD = new dfgNode(this);
					tempADD->setNameType("GEPADD");
					tempADD->setIdx(NodeList.size());
					NodeList.push_back(tempADD);
					tempADD->BB = node->BB;

					for (dfgNode *parent : non_control_parents)
					{
						parent->removeChild(node);
						node->removeAncestor(parent);
						removeEdge(findEdge(parent, node));

						parent->addChildNode(tempADD);
						tempADD->addAncestorNode(parent);
					}

					if (elementSize > 1)
					{
						dfgNode *temp = new dfgNode(this);
						temp->setNameType("GEPLEFTSHIFT");
						temp->setIdx(NodeList.size());
						NodeList.push_back(temp);
						temp->setConstantVal(Log2_32(elementSize));
						temp->BB = node->BB;

						tempADD->addChildNode(temp);
						temp->addAncestorNode(tempADD);

						temp->addChildNode(node);
						node->addAncestorNode(temp);
					}
				}
				else
				{
					assert(non_control_parents.size() == 1);
					if (elementSize > 1)
					{
						dfgNode *temp = new dfgNode(this);
						temp->setNameType("GEPLEFTSHIFT");
						temp->setIdx(NodeList.size());
						NodeList.push_back(temp);
						temp->setConstantVal(Log2_32(elementSize));
						temp->BB = node->BB;

						for (dfgNode *parent : node->getAncestors())
						{
							parent->removeChild(node);
							node->removeAncestor(parent);
							removeEdge(findEdge(parent, node));

							parent->addChildNode(temp);
							temp->addAncestorNode(parent);
						}
						temp->addChildNode(node);
						node->addAncestorNode(temp);
					}
				}
			}
		}
	}
	LLVM_DEBUG(dbgs() << "insertshiftGEPs ended!\n");
}

int DFG::partitionMemNodes()
{
	dfgNode *node;
	std::map<Value *, std::vector<dfgNode *>> pointerMemInsMap;
	std::vector<Value *> pointerOperandVec;

	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		if (node->getNode() != NULL)
		{
			if (LoadInst *LDI = dyn_cast<LoadInst>(node->getNode()))
			{

				if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(LDI->getPointerOperand()))
				{
					//					GetElementPtrInst* GEP = cast<GetElementPtrInst>(LDI->getPointerOperand());
					pointerMemInsMap[GEP->getPointerOperand()].push_back(node);
				}
				else
				{
					pointerMemInsMap[LDI->getPointerOperand()].push_back(node);
				}
			}

			if (StoreInst *STI = dyn_cast<StoreInst>(node->getNode()))
			{

				if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(STI->getPointerOperand()))
				{
					//					GetElementPtrInst* GEP = cast<GetElementPtrInst>(STI->getPointerOperand());
					pointerMemInsMap[GEP->getPointerOperand()].push_back(node);
				}
				else
				{
					pointerMemInsMap[STI->getPointerOperand()].push_back(node);
				}
			}
		}
	}

	int sum = 0;
	for (std::pair<Value *, std::vector<dfgNode *>> pair : pointerMemInsMap)
	{
		sum += pair.second.size();
		pointerOperandVec.push_back(pair.first);
	}

	int n = pointerOperandVec.size();

	bool dp[n + 1][sum + 1];
	std::map<int, std::map<int, std::vector<Value *>>> dpValueMap;

	//first column is true :: 0 sum is possible with all elements
	for (int i = 0; i <= n; ++i)
	{
		dp[i][0] = true;
	}

	// Initialize top row, except dp[0][0], as false. With
	// 0 elements, no other sum except 0 is possible
	for (int j = 1; j <= sum; j++)
	{
		dp[0][j] = false;
	}

	// Fill the partition table in bottom up manner
	for (int i = 1; i <= n; i++)
	{
		for (int j = 1; j <= sum; j++)
		{
			// If i'th element is excluded
			dp[i][j] = dp[i - 1][j];
			if (dp[i - 1][j])
			{
				dpValueMap[i][j] = dpValueMap[i - 1][j];
			}

			// If i'th element is included
			int ithElement = pointerMemInsMap[pointerOperandVec[i - 1]].size();
			if (ithElement <= j)
			{
				dp[i][j] |= dp[i - 1][j - ithElement];
				if (dp[i - 1][j - ithElement])
				{
					dpValueMap[i][j] = dpValueMap[i - 1][j - ithElement];
					dpValueMap[i][j].push_back(pointerOperandVec[i - 1]);
				}
			}
		}
	}

	// Initialize difference of two sums.
	int diff = INT_MAX;

	// Find the largest j such that dp[n][j]
	// is true where j loops from sum/2 t0 0
	int minsumpart = 0;
	for (int j = sum / 2; j >= 0; j--)
	{
		// Find the
		if (dp[n][j] == true)
		{
			diff = sum - 2 * j;
			minsumpart = j;
			break;
		}
	}

	LLVM_DEBUG(dbgs() << "Left side pointers : \n");
	for (Value *pointerOp : dpValueMap[n][minsumpart])
	{
		LLVM_DEBUG(dbgs() << "(" << pointerOp->getName() << "," << pointerMemInsMap[pointerOp].size() << ");");
		for (dfgNode *node : pointerMemInsMap[pointerOp])
		{
			node->setLeftAlignedMemOp(1);
		}
		pointerOperandVec.erase(std::remove(pointerOperandVec.begin(), pointerOperandVec.end(), pointerOp), pointerOperandVec.end());
	}
	LLVM_DEBUG(dbgs() << "\n");

	LLVM_DEBUG(dbgs() << "Right side pointers : \n");
	for (Value *pointerOp : pointerOperandVec)
	{
		LLVM_DEBUG(dbgs() << "(" << pointerOp->getName() << "," << pointerMemInsMap[pointerOp].size() << ");");
		for (dfgNode *node : pointerMemInsMap[pointerOp])
		{
			node->setLeftAlignedMemOp(2);
		}
	}
	LLVM_DEBUG(dbgs() << "\n");

	return 0;
}

int DFG::handlestartstop()
{
	dfgNode *node;

	// Loop *L = this->getLoop();
	// SmallVector<BasicBlock *, 8> loopExitBlocks;
	// L->getExitBlocks(loopExitBlocks);

	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		if (node->getNode() == NULL)
		{
			if (node->getNameType().compare("LOOPSTART") == 0)
			{
				//				assert(false);
			}
		}
	}

	bool startFound = false;
	bool exitFound = false;

	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];

		if (startFound & exitFound)
		{
			break;
		}

		//LoopStart
		if (node->getNode() == NULL)
		{
			if (node->getNameType().compare("LOOPSTART") == 0)
			{
				startFound = true;
				//				dfgNode* loadStart = new dfgNode(this);
				//				loadStart->setIdx(this->getNodesPtr()->size());
				//				this->getNodesPtr()->push_back(loadStart);
				//				loadStart->setNameType("LOADSTART");
				//				loadStart->BB = node->BB;

				node->setLeftAlignedMemOp(2);

				dfgNode *storeStart = new dfgNode(this);
				storeStart->setIdx(this->getNodesPtr()->size());
				this->getNodesPtr()->push_back(storeStart);
				storeStart->setNameType("STORESTART");
				storeStart->BB = node->BB;
				storeStart->setLeftAlignedMemOp(2);
				storeStart->setNPB(false);

				node->addChildNode(storeStart);
				storeStart->addAncestorNode(node);

				dfgNode *movone = new dfgNode(this);
				movone->setIdx(this->getNodesPtr()->size());
				this->getNodesPtr()->push_back(movone);
				movone->setNameType("MOVC");
				movone->BB = node->BB;
				movone->setConstantVal(0);

				movone->addChildNode(storeStart);
				storeStart->addAncestorNode(movone);

				//				for (dfgNode* child : node->getChildren()) {
				//					node->removeChild(child);
				//					child->removeAncestor(node);
				//					removeEdge(findEdge(node,child));
				//
				//					loadStart->addChildNode(child);
				//					child->addAncestorNode(loadStart);
				//				}
			}
		}
		else
		{ //LoopExit
			//			if(std::find(loopExitBlocks.begin(),loopExitBlocks.end(),node->getNode()->getParent())!=loopExitBlocks.end()){
			//
			//				if(!dyn_cast<BranchInst>(node->getNode())){
			//					continue;
			//				}
			//
			//				//current node belong to a loop exit basic block
			//				dfgNode* loopexit = new dfgNode(this);
			//				loopexit->setIdx(this->getNodesPtr()->size());
			//				this->getNodesPtr()->push_back(loopexit);
			//				loopexit->setNameType("LOOPEXIT");
			//				loopexit->BB = node->BB;
			//				loopexit->setLeftAlignedMemOp(1);
			//
			//				assert(node->getChildren().empty());
			//				for(dfgNode* anc : node->getAncestors()){
			//					anc->removeChild(node);
			//					node->removeAncestor(anc);
			//					removeEdge(findEdge(anc,node));
			//
			//					anc->addChildNode(loopexit);
			//					loopexit->addAncestorNode(anc);
			//				}
			//
			//				dfgNode* movone = new dfgNode(this);
			//				movone->setIdx(this->getNodesPtr()->size());
			//				this->getNodesPtr()->push_back(movone);
			//				movone->setNameType("MOVC");
			//				movone->BB = node->BB;
			//				movone->setConstantVal(1);
			//
			//				movone->addChildNode(loopexit);
			//				loopexit->addAncestorNode(movone);
			//			}

			//checking
			//			assert(false); // please comment below to remove this assertion

			//			if(std::find(loopExitBlocks.begin(),loopExitBlocks.end(),node->getNode()->g)!=loopExitBlocks.end()){

			// if (BranchInst *BRI = dyn_cast<BranchInst>(node->getNode()))
			// {

			// 	for (int j = 0; j < BRI->getNumSuccessors(); ++j)
			// 	{
			// 		if (std::find(loopExitBlocks.begin(),
			// 					  loopExitBlocks.end(),
			// 					  BRI->getSuccessor(j)) != loopExitBlocks.end())
			// 		{

			// 			//current node belong to a loop exit basic block
			// 			dfgNode *loopexit = new dfgNode(this);
			// 			loopexit->setIdx(this->getNodesPtr()->size());
			// 			this->getNodesPtr()->push_back(loopexit);
			// 			loopexit->setNameType("LOOPEXIT");
			// 			loopexit->BB = BRI->getSuccessor(j);
			// 			loopexit->setLeftAlignedMemOp(1);

			// 			node->addChildNode(loopexit);
			// 			loopexit->addAncestorNode(node);

			// 			dfgNode *movone = new dfgNode(this);
			// 			movone->setIdx(this->getNodesPtr()->size());
			// 			this->getNodesPtr()->push_back(movone);
			// 			movone->setNameType("MOVC");
			// 			movone->BB = node->BB;
			// 			movone->setConstantVal(1);

			// 			movone->addChildNode(loopexit);
			// 			loopexit->addAncestorNode(movone);
			// 			exitFound = true;
			// 		}
			// 	}
			// }

			//			}
		}
	}
}

int DFG::handlestartstop_munit(std::vector<munitTransition> bbTrans)
{

	std::vector<dfgNode *> startNodes;
	std::map<dfgNode *, BasicBlock *> comingFromBBMap;
	std::map<dfgNode *, std::vector<BasicBlock *>> exitNodesWithSucc;
	int exitNodesWithSuccCount = 0;

	for (std::pair<Instruction *, dfgNode *> pair : LoopStartMap)
	{
		startNodes.push_back(pair.second);
		comingFromBBMap[pair.second] = pair.first->getParent();
	}

	dfgNode *realStartNode = NULL;
	for (int i = 0; i < NodeList.size(); ++i)
	{
		dfgNode *node = NodeList[i];
		if (node->getNode() == NULL)
		{
			if (node->getNameType().compare("LOOPSTART") == 0)
			{
				assert(realStartNode == NULL);
				realStartNode = node;
			}
		}
		else
		{
			if (BranchInst *BRI = dyn_cast<BranchInst>(node->getNode()))
			{
				for (int j = 0; j < BRI->getNumSuccessors(); ++j)
				{
					if (std::find(loopBB.begin(),
							loopBB.end(),
							BRI->getSuccessor(j)) == loopBB.end())
					{
						//successor does not belong to the loop
						//						exitNodes.push_back(node);
						LLVM_DEBUG(BRI->dump());
						LLVM_DEBUG(dbgs() << "Successor BB : " << BRI->getSuccessor(j)->getName() << "\n");
						exitNodesWithSucc[node].push_back(BRI->getSuccessor(j));
						exitNodesWithSuccCount++;
					}
				}
			}
		}
	}
	assert(realStartNode != NULL);

	LLVM_DEBUG(dbgs() << "munitName = " << this->name << "\n");
	LLVM_DEBUG(dbgs() << "startNodes size =" << startNodes.size() << "\n");
	LLVM_DEBUG(dbgs() << "loopentryBB size =" << loopentryBB.size() << "\n");

	LLVM_DEBUG(dbgs() << "exitNodesWithSucc size =" << exitNodesWithSuccCount << "\n");
	LLVM_DEBUG(dbgs() << "loopexitBB size =" << loopexitBB.size() << "\n");

	assert(startNodes.size() == this->loopentryBB.size());
	assert(loopexitBB.size() == exitNodesWithSuccCount);

	std::vector<dfgNode *> connecttoRealStart;
	for (dfgNode *startNode : startNodes)
	{
		dfgNode *cusBitMsk = new dfgNode(this);
		cusBitMsk->setIdx(this->getNodesPtr()->size());
		this->getNodesPtr()->push_back(cusBitMsk);
		cusBitMsk->setNameType("PREDAND");
		cusBitMsk->BB = realStartNode->BB;

		bool found = false;
		uint32_t BBActiveTransBit;
		for (munitTransition munitTrans : bbTrans)
		{
			if (munitTrans.srcBB == comingFromBBMap[startNode] &&
					munitTrans.destBB == startNode->BB)
			{
				BBActiveTransBit = 1 << munitTrans.id;
				found = true;
				break;
			}
		}
		assert(found);
		cusBitMsk->setConstantVal(BBActiveTransBit);

		//make connections
		for (dfgNode *startNodeChild : startNode->getChildren())
		{
			startNode->removeChild(startNodeChild);
			startNodeChild->removeAncestor(startNode);
			removeEdge(findEdge(startNode, startNodeChild));

			cusBitMsk->addChildNode(startNodeChild);
			startNodeChild->addAncestorNode(cusBitMsk);
		}

		connecttoRealStart.push_back(cusBitMsk);
		//		assert(realStartNode->getNameType().compare("LOOPSTART")==0);
		//		realStartNode->addChildNode(cusBitMsk);
		//		cusBitMsk->addAncestorNode(realStartNode);
	}

	for (dfgNode *connectNode : connecttoRealStart)
	{
		assert(realStartNode->getNameType().compare("LOOPSTART") == 0);
		realStartNode->addChildNode(connectNode);
		connectNode->addAncestorNode(realStartNode);
	}

	//	uint32_t BBActiveWord = 0;
	//	for(BasicBlock* bb : loopentryBB){
	//		int bbIdx = (*bbids)[bb];
	//		int singlebit = 1 << bbIdx;
	//		BBActiveWord = BBActiveWord | singlebit;
	//	}
	//make connection for one mapping unit
	uint32_t BBActiveTransWord = 0;
	for (dfgNode *startNode : startNodes)
	{
		bool found = false;
		uint32_t BBActiveTransBit;

		for (munitTransition munitTrans : bbTrans)
		{
			if (munitTrans.srcBB == comingFromBBMap[startNode] &&
					munitTrans.destBB == startNode->BB)
			{
				BBActiveTransBit = 1 << munitTrans.id;
				BBActiveTransWord = BBActiveTransWord | BBActiveTransBit;
				found = true;
				break;
			}
		}
		assert(found);
	}

	dfgNode *cusZeroBitMsk = new dfgNode(this);
	cusZeroBitMsk->setIdx(this->getNodesPtr()->size());
	this->getNodesPtr()->push_back(cusZeroBitMsk);
	cusZeroBitMsk->setNameType("PREDAND");
	cusZeroBitMsk->BB = realStartNode->BB;
	cusZeroBitMsk->setConstantVal(~BBActiveTransWord);

	dfgNode *storeStart = new dfgNode(this);
	storeStart->setIdx(this->getNodesPtr()->size());
	this->getNodesPtr()->push_back(storeStart);
	storeStart->setNameType("STORESTART");
	storeStart->BB = realStartNode->BB;
	storeStart->setLeftAlignedMemOp(2);

	cusZeroBitMsk->addChildNode(storeStart);
	storeStart->addAncestorNode(cusZeroBitMsk);

	assert(realStartNode->getNameType().compare("LOOPSTART") == 0);
	realStartNode->addChildNode(cusZeroBitMsk);
	cusZeroBitMsk->addAncestorNode(realStartNode);

	for (std::pair<dfgNode *, std::vector<BasicBlock *>> pair : exitNodesWithSucc)
	{
		int count = 0;
		for (BasicBlock *succBasicBlock : pair.second)
		{
			dfgNode *node = pair.first;
			assert(pair.second.size() < 3);
			dfgNode *loopexit = new dfgNode(this);
			loopexit->setIdx(this->getNodesPtr()->size());
			this->getNodesPtr()->push_back(loopexit);
			loopexit->setNameType("LOOPEXIT");
			loopexit->BB = pair.second[0]; //just putting one bb out of the loop coz it doesnt really matter
			loopexit->setLeftAlignedMemOp(1);
			if (count == 1)
			{
				loopexit->setNPB(true);
			}
			node->addChildNode(loopexit);
			loopexit->addAncestorNode(node);

			dfgNode *movnextbb = new dfgNode(this);
			movnextbb->setIdx(this->getNodesPtr()->size());
			this->getNodesPtr()->push_back(movnextbb);
			movnextbb->setNameType("MOVC");
			movnextbb->BB = node->BB;

			bool found = false;
			uint32_t BBActiveTransBit = 0;
			for (munitTransition munitTrans : bbTrans)
			{
				if (munitTrans.srcBB == node->BB &&
						munitTrans.destBB == succBasicBlock)
				{
					BBActiveTransBit = 1 << munitTrans.id;
					found = true;
					break;
				}
			}
			movnextbb->setConstantVal(BBActiveTransBit);
			movnextbb->addChildNode(loopexit);
			loopexit->addAncestorNode(movnextbb);
			count++;
		}
	}
}

int DFG::nonGEPLoadStorecheck()
{
	dfgNode *node;

	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];

		if (node->getNode() == NULL)
			continue;
		if (node->getNode()->mayReadOrWriteMemory() != true)
			continue;

		const DataLayout DL = node->getNode()->getParent()->getParent()->getParent()->getDataLayout();

		//only comes to here when there are loads and nodes
		bool GEPfound = false;
		for (dfgNode *parent : node->getAncestors())
		{
			if (parent->getNode() == NULL)
				continue;
			if (dyn_cast<GetElementPtrInst>(parent->getNode()))
			{
				GEPfound = true;
				break;
			}
		}
		if (GEPfound)
			continue;

		Type *T;

		if (LoadInst *LDI = dyn_cast<LoadInst>(node->getNode()))
		{
			T = LDI->getPointerOperand()->getType();
			if (dyn_cast<Instruction>(LDI->getPointerOperand()))
			{
				continue;
			}
		}
		else if (StoreInst *STI = dyn_cast<StoreInst>(node->getNode()))
		{
			T = STI->getPointerOperand()->getType();
			if (dyn_cast<Instruction>(STI->getPointerOperand()))
			{
				continue;
			}
		}
		else
		{
			assert(false);
		}

		LLVM_DEBUG(node->getNode()->dump());
		int Size = DL.getTypeAllocSize(T);
		LLVM_DEBUG(dbgs() << "Size=" << Size << ",");

		if (PointerType *PT = dyn_cast<PointerType>(T))
		{
			LLVM_DEBUG(dbgs() << "ElementSize=" << DL.getTypeAllocSize(PT->getElementType()) << ",");
			//			PT->is
		}

		LLVM_DEBUG(dbgs() << "S/A/I/P=" << T->isStructTy() << "/");
		LLVM_DEBUG(dbgs() << T->isArrayTy() << "/");
		LLVM_DEBUG(dbgs() << T->isIntegerTy() << "/");
		LLVM_DEBUG(dbgs() << T->isPointerTy() << "\n");
	}

	assert(false);
}

int DFG::addMaskLowBitInstructions()
{
	for (dfgNode *node : NodeList)
	{
		if (node->getNode() != NULL)
		{
			if (node->getNode()->getOpcode() == Instruction::Shl)
			{
				int byteWidth = node->getNode()->getType()->getIntegerBitWidth() / 8;
				if (byteWidth == 2 || byteWidth == 1)
				{
					dfgNode *mls = new dfgNode(this);
					mls->setIdx(this->getNodesPtr()->size());
					this->getNodesPtr()->push_back(mls);
					mls->setNameType("MASKAND");
					if (byteWidth == 2)
					{
						mls->setConstantVal(0xffff);
					}
					else
					{ //byteWidth == 1
						mls->setConstantVal(0xff);
					}
					mls->BB = node->BB;

					dfgNode *child;
					for (int i = 0; i < node->getChildren().size(); ++i)
					{
						child = node->getChildren()[i];

						node->removeChild(child);
						child->removeAncestor(node);
						removeEdge(findEdge(node, child));

						node->addChildNode(mls);
						mls->addAncestorNode(node);

						mls->addChildNode(child);
						child->addAncestorNode(mls);
					}
				}
				else
				{
					assert(byteWidth == 4);
				}
			}
		}
	}
}

int DFG::addBreakLongerPaths()
{
	dfgNode *node;

	for (int i = 0; i < NodeList.size(); ++i)
	{
		node = NodeList[i];
		for (dfgNode *child : node->getChildren())
		{
			int ASAPdistance = child->getASAPnumber() - node->getASAPnumber();
			int internalASAPdist = ASAPdistance / 4;
			if (ASAPdistance > 5 && internalASAPdist > 0)
			{
				LLVM_DEBUG(dbgs() << "breaking long paths adding op:OR 0 between "
						<< ": node=" << node->getIdx() << "and node=" << child->getIdx() << "\n");

				dfgNode *orzero1 = new dfgNode(this);
				orzero1->setIdx(this->getNodesPtr()->size());
				this->getNodesPtr()->push_back(orzero1);
				orzero1->BB = node->BB;
				orzero1->setNameType("ORZERO");

				orzero1->setASAPnumber(node->getASAPnumber() + internalASAPdist);

				dfgNode *orzero2 = new dfgNode(this);
				orzero2->setIdx(this->getNodesPtr()->size());
				this->getNodesPtr()->push_back(orzero2);
				orzero2->BB = node->BB;
				orzero2->setNameType("ORZERO");

				orzero2->setASAPnumber(orzero1->getASAPnumber() + internalASAPdist);

				dfgNode *orzero3 = new dfgNode(this);
				orzero3->setIdx(this->getNodesPtr()->size());
				this->getNodesPtr()->push_back(orzero3);
				orzero3->BB = node->BB;
				orzero3->setNameType("ORZERO");

				orzero3->setASAPnumber(orzero2->getASAPnumber() + internalASAPdist);

				orzero3->setALAPnumber(child->getALAPnumber() - 1);
				orzero2->setALAPnumber(orzero3->getALAPnumber() - 1);
				orzero1->setALAPnumber(orzero2->getALAPnumber() - 1);

				node->removeChild(child);
				child->removeAncestor(node);

				node->addChildNode(orzero1);
				orzero1->addAncestorNode(node);

				orzero1->addChildNode(orzero2);
				orzero2->addAncestorNode(orzero1);

				orzero2->addChildNode(orzero3);
				orzero3->addAncestorNode(orzero2);

				orzero3->addChildNode(child);
				child->addAncestorNode(orzero3);
			}
		}
	}
}

int DFG::analyzeRTpaths()
{
	assert(!nodeRouteMap.empty());
	LLVM_DEBUG(dbgs() << "analyzeRTpaths :: begin\n");

	for (dfgNode *node : NodeList)
	{
		if (node->getAncestors().empty())
			continue;
		if (node->getNode() != NULL)
		{
			if (dyn_cast<PHINode>(node->getNode()))
			{
				continue;
			}
		}
		for (dfgNode *parent : node->getAncestors())
		{
			if (parent->getNameType().compare("OutLoopLOAD") == 0 ||
					parent->getNameType().compare("MOVC") == 0)
			{
				continue;
			}

			if (std::find(node->getPHIancestors().begin(),
					node->getPHIancestors().begin(),
					parent) != node->getPHIancestors().end())
			{
				continue;
			}

			//analyzing distance in time
			int distanceDt = 1; //it should be atleast 1
			int parent_tplus1_2 = parent_tplus1_2 = (parent->getMappedLoc()->getT() + 1) % currCGRA->getMII();
			if (parent->getMappedLoc()->getPEType() == MEM)
			{
				distanceDt = 2;
				parent_tplus1_2 = (parent->getMappedLoc()->getT() + 2) % currCGRA->getMII();
			}

			LLVM_DEBUG(dbgs() << "path size = " << nodeRouteMap[node][parent].size() << "\n");

			int parent_y = parent->getMappedLoc()->getY();
			int parent_x = parent->getMappedLoc()->getX();

			CGRANode *startingCnode = currCGRA->getCGRANode(parent_tplus1_2, parent_y, parent_x);

			for (int i = 1; i < nodeRouteMap[node][parent].size(); ++i)
			{

				//this vector is going from destination to source
				CGRANode *curr = nodeRouteMap[node][parent][i];
				CGRANode *next = nodeRouteMap[node][parent][i - 1];

				if (curr->getT() != next->getT())
				{
					LLVM_DEBUG(dbgs() << "curr=" << curr->getNameSp() << ",next=" << next->getNameSp() << "\n");
					distanceDt++;
				}

				if (curr == startingCnode)
				{
					break;
				}
			}

			int distanceRT = node->getmappedRealTime() - parent->getmappedRealTime();

			if (distanceDt != distanceRT)
			{
				LLVM_DEBUG(dbgs() << "the path from " << parent->getMappedLoc()->getNameSp() << ",Node=" << parent->getIdx());
				LLVM_DEBUG(dbgs() << " to " << node->getMappedLoc()->getNameSp() << ",Node=" << node->getIdx());
				LLVM_DEBUG(dbgs() << " RealDistance=" << distanceRT << " and MappedDistance=" << distanceDt << "\n");
			}

			assert(distanceRT % currCGRA->getMII() == distanceDt % currCGRA->getMII());
		}
	}
	LLVM_DEBUG(dbgs() << "analyzeRTpaths :: end\n");
}

std::map<BasicBlock *, std::set<BasicBlock *>> DFG::checkMutexBBs()
{
	assert(NodeList.size() > 0);
	dfgNode *firstNode = NodeList[0];
	SmallVector<std::pair<const BasicBlock *, const BasicBlock *>, 8> Result;
	FindFunctionBackedges(*(firstNode->BB->getParent()), Result);
	LLVM_DEBUG(dbgs() << "Number of Backedges = " << Result.size() << "\n");

	std::map<BasicBlock *, std::set<BasicBlock *>> mutexBBs;

	for (BasicBlock *BB1 : loopBB)
	{
		for (BasicBlock *BB2 : loopBB)
		{
			if (std::find(BBSuccBasicBlocks[BB1].begin(), BBSuccBasicBlocks[BB1].end(), BB2) == BBSuccBasicBlocks[BB1].end())
			{
				if (std::find(BBSuccBasicBlocks[BB2].begin(), BBSuccBasicBlocks[BB2].end(), BB1) == BBSuccBasicBlocks[BB2].end())
				{
					// coming here means BB2 is not a successor of BB1
					// coming here means BB1 is not a successor of BB2
					// then its mutually exclusive
					mutexBBs[BB1].insert(BB2);
					mutexBBs[BB2].insert(BB1);
				}
			}
		}
	}

	for (std::pair<BasicBlock *, std::set<BasicBlock *>> pair : mutexBBs)
	{
		LLVM_DEBUG(dbgs() << "BB=" << pair.first->getName() << ":");
		for (BasicBlock *BB2 : pair.second)
		{
			LLVM_DEBUG(dbgs() << BB2->getName() << ",");
		}
		LLVM_DEBUG(dbgs() << "\n");
	}

	return mutexBBs;
}

int DFG::printHyCUBEInsHist()
{
	LLVM_DEBUG(dbgs() << "Hist--------------------\n");
	for (std::pair<HyCUBEIns, int> pair : hyCUBEInsHist)
	{
		LLVM_DEBUG(dbgs() << "Ins:" << HyCUBEInsStrings[pair.first] << "," << pair.second << "\n");
	}
}

void DFG::printNewDFGXML()
{
	std::string fileName = name + "_DFG.xml";
	std::ofstream xmlFile;
	xmlFile.open(fileName.c_str());

	printREGIMapfiles();
	addPHIParents();
	classifyParents();
	MergeCMerge();

	//    insertMOVC();
	//	scheduleASAP();
	//	scheduleALAP();
	//	balanceASAPALAP();
	//				  LoopDFG.addBreakLongerPaths();
	CreateSchList();

	std::map<BasicBlock *, std::set<BasicBlock *>> mBBs = checkMutexBBs();
	std::map<std::string, std::set<std::string>> mBBs_str;

	std::map<int, std::vector<dfgNode *>> asaplevelNodeList;
	for (dfgNode *node : NodeList)
	{
		asaplevelNodeList[node->getASAPnumber()].push_back(node);
	}

	std::map<dfgNode *, std::string> nodeBBModified;
	for (dfgNode *node : NodeList)
	{
		nodeBBModified[node] = node->BB->getName().str();
	}

	for (dfgNode *node : NodeList)
	{
		int cmergeParentCount = 0;
		std::set<std::string> mutexBBs;
		for (dfgNode *parent : node->getAncestors())
		{
			if (HyCUBEInsStrings[parent->getFinalIns()] == "CMERGE")
			{
				nodeBBModified[parent] = nodeBBModified[parent] + "_" + std::to_string(node->getIdx()) + "_" + std::to_string(cmergeParentCount);
				mutexBBs.insert(nodeBBModified[parent]);
				cmergeParentCount++;
			}
		}
		for (std::string bb_str1 : mutexBBs)
		{
			for (std::string bb_str2 : mutexBBs)
			{
				if (bb_str2 == bb_str1)
					continue;
				mBBs_str[bb_str1].insert(bb_str2);
			}
		}
	}

	xmlFile << "<MutexBB>\n";
	for (std::pair<BasicBlock *, std::set<BasicBlock *>> pair : mBBs)
	{
		BasicBlock *first = pair.first;
		xmlFile << "<BB1 name=\"" << first->getName().str() << "\">\n";
		for (BasicBlock *second : pair.second)
		{
			xmlFile << "\t<BB2 name=\"" << second->getName().str() << "\"/>\n";
		}
		xmlFile << "</BB1>\n";
	}
	for (std::pair<std::string, std::set<std::string>> pair : mBBs_str)
	{
		std::string first = pair.first;
		xmlFile << "<BB1 name=\"" << first << "\">\n";
		for (std::string second : pair.second)
		{
			xmlFile << "\t<BB2 name=\"" << second << "\"/>\n";
		}
		xmlFile << "</BB1>\n";
	}
	xmlFile << "</MutexBB>\n";

	xmlFile << "<DFG count=\"" << NodeList.size() << "\">\n";

	for (dfgNode *node : NodeList)
	{
		//	for (int i = 0; i < maxASAPLevel; ++i) {
		//		for(dfgNode* node : asaplevelNodeList[i]){
		xmlFile << "<Node idx=\"" << node->getIdx() << "\"";
		xmlFile << "ASAP=\"" << node->getASAPnumber() << "\"";

		//		    if(node->getNameType() == "OutLoopLOAD") {
		//		    	xmlFile << "OutLoopLOAD=\"1\"";
		//		    }
		//		    else{
		//		    	xmlFile << "OutLoopLOAD=\"0\"";
		//		    }
		//
		//		    if(node->getNameType() == "OutLoopSTORE") {
		//		    	xmlFile << "OutLoopSTORE=\"1\"";
		//		    }
		//		    else{
		//		    	xmlFile << "OutLoopSTORE=\"0\"";
		//		    }

		//		    xmlFile << "BB=\"" << node->BB->getName().str() << "\"";
		xmlFile << "BB=\"" << nodeBBModified[node] << "\"";
		if (node->hasConstantVal())
		{
			xmlFile << "CONST=\"" << node->getConstantVal() << "\"";
		}
		xmlFile << ">\n";

		xmlFile << "<OP>";
		if ((node->getNameType() == "OutLoopLOAD") || (node->getNameType() == "OutLoopSTORE"))
		{
			xmlFile << "O";
		}
		xmlFile << HyCUBEInsStrings[node->getFinalIns()] << "</OP>\n";
		//			xmlFile << "<OP>" << HyCUBEInsStrings[node->getFinalIns()] << "</OP>\n";

		xmlFile << "<Inputs>\n";
		for (dfgNode *parent : node->getAncestors())
		{
			//			xmlFile << "\t<Input idx=\"" << parent->getIdx() << "\" type=\"DATA\"/>\n";
			xmlFile << "\t<Input idx=\"" << parent->getIdx() << "\"/>\n";
		}
		//		for(dfgNode* parentPHI : node->getPHIancestors()){
		//			xmlFile << "\t<Input idx=\"" << parentPHI->getIdx() << "\" type=\"PHI\"/>\n";
		//		}
		xmlFile << "</Inputs>\n";

		xmlFile << "<Outputs>\n";
		for (dfgNode *child : node->getChildren())
		{
			//			xmlFile << "\t<Output idx=\"" << child->getIdx() <<"\" type=\"DATA\"/>\n";
			xmlFile << "\t<Output idx=\"" << child->getIdx() << "\" ";

			if (child->parentClassification[0] == node)
			{
				xmlFile << "type=\"P\"/>\n";
			}
			else if (child->parentClassification[1] == node)
			{
				xmlFile << "type=\"I1\"/>\n";
			}
			else if (child->parentClassification[2] == node)
			{
				xmlFile << "type=\"I2\"/>\n";
			}
			else
			{
				bool found = false;
				for (std::pair<int, dfgNode *> pair : child->parentClassification)
				{
					if (pair.second == node)
					{
						xmlFile << "type=\"I2\"/>\n";
						found = true;
						break;
					}
				}

				assert(found);
			}
		}
		//		for(dfgNode* phiChild : node->getPHIchildren()){
		//			xmlFile << "\t<Output idx=\"" << phiChild->getIdx() << "\" type=\"PHI\"/>\n";
		//		}
		xmlFile << "</Outputs>\n";

		xmlFile << "<RecParents>\n";
		for (dfgNode *recParent : node->getRecAncestors())
		{
			xmlFile << "\t<RecParent idx=\"" << recParent->getIdx() << "\"";
			xmlFile << " type=\"" << node->RecAncestorType[recParent->getNode()] << "\"";
			xmlFile << "/>\n";
		}
		xmlFile << "</RecParents>\n";

		xmlFile << "</Node>\n\n";
	}
	//		}
	//	}

	xmlFile << "</DFG>\n";
	xmlFile.close();
}

void DFG::printREGIMapfiles()
{
	std::map<HyCUBEIns, int> regimapInsIdxMap;
	regimapInsIdxMap[NOP] = 32;
	regimapInsIdxMap[ADD] = 0;
	regimapInsIdxMap[SUB] = 1;
	regimapInsIdxMap[MUL] = 2;
	regimapInsIdxMap[SEXT] = 4;
	regimapInsIdxMap[DIV] = 3;
	regimapInsIdxMap[LS] = 4;
	regimapInsIdxMap[RS] = 5;
	regimapInsIdxMap[ARS] = regimapInsIdxMap[RS];
	regimapInsIdxMap[AND] = 6;
	regimapInsIdxMap[OR] = 7;
	regimapInsIdxMap[XOR] = 8;
	regimapInsIdxMap[SELECT] = 29;
	regimapInsIdxMap[CMERGE] = regimapInsIdxMap[OR];
	regimapInsIdxMap[CMP] = 10;
	regimapInsIdxMap[CLT] = 12;
	regimapInsIdxMap[BR] = 31;
	regimapInsIdxMap[CGT] = 9;

	regimapInsIdxMap[Hy_LOAD] = 19;
	regimapInsIdxMap[Hy_LOADH] = 19;
	regimapInsIdxMap[Hy_LOADB] = 19;
	regimapInsIdxMap[Hy_STORE] = 21;
	regimapInsIdxMap[Hy_STOREH] = 21;
	regimapInsIdxMap[Hy_STOREB] = 21;
	regimapInsIdxMap[JUMPL] = regimapInsIdxMap[BR];
	regimapInsIdxMap[LOADCL] = regimapInsIdxMap[Hy_LOAD];
	regimapInsIdxMap[MOVCL] = regimapInsIdxMap[OR];
	regimapInsIdxMap[MOVC] = regimapInsIdxMap[OR];

	//NOP,ADD,SUB,MUL,SEXT,DIV,LS,RS,ARS,AND,OR,XOR,SELECT,CMERGE,CMP,CLT,BR,CGT,LOADCL,MOVCL,Hy_LOAD,Hy_LOADH,Hy_LOADB,Hy_STORE,Hy_STOREH,Hy_STOREB,JUMPL,MOVC

	std::string nodefileName = name + "_REGI_node.txt";
	std::string edgefileName = name + "_REGI_edge.txt";

	std::ofstream nodeFile;
	nodeFile.open(nodefileName.c_str());

	std::ofstream edgeFile;
	edgeFile.open(edgefileName.c_str());

	int additionalDataNodeStartIdx = 1000;

	for (dfgNode *node : NodeList)
	{
		if (regimapInsIdxMap[node->getFinalIns()] == 19 || regimapInsIdxMap[node->getFinalIns()] == 21)
		{
			nodeFile << node->getIdx() << "\t" << regimapInsIdxMap[node->getFinalIns()] << "\n";
			nodeFile << additionalDataNodeStartIdx << "\t" << regimapInsIdxMap[node->getFinalIns()] + 1 << "\n";

			if (regimapInsIdxMap[node->getFinalIns()] == 19)
			{
				edgeFile << node->getIdx() << "\t" << additionalDataNodeStartIdx << "\tLRE\n";
			}
			else
			{
				assert(regimapInsIdxMap[node->getFinalIns()] == 21);
				edgeFile << node->getIdx() << "\t" << additionalDataNodeStartIdx << "\tSRE\n";
			}

			for (dfgNode *child : node->getChildren())
			{
				edgeFile << additionalDataNodeStartIdx << "\t" << child->getIdx() << "\tTRU\n";
			}
			for (dfgNode *child : node->getPHIchildren())
			{
				edgeFile << additionalDataNodeStartIdx << "\t" << child->getIdx() << "\tPRE\n";
			}
			additionalDataNodeStartIdx++;
		}
		else
		{
			nodeFile << node->getIdx() << "\t" << regimapInsIdxMap[node->getFinalIns()] << "\n";
			for (dfgNode *child : node->getChildren())
			{
				edgeFile << node->getIdx() << "\t" << child->getIdx() << "\tTRU\n";
			}
			for (dfgNode *child : node->getPHIchildren())
			{
				edgeFile << node->getIdx() << "\t" << child->getIdx() << "\tPRE\n";
			}
		}
	}

	nodeFile.close();
	edgeFile.close();
}

void DFG::MergeCMerge()
{

	std::set<dfgNode *> delNodes;

	for (dfgNode *node : NodeList)
	{
		if (node->getNameType().compare("CMERGE") == 0)
		{
			assert(node->parentClassification.find(0) != node->parentClassification.end());
			//			assert(node->parentClassification.find(1)!=node->parentClassification.end());
			if (node->parentClassification.find(1) == node->parentClassification.end())
			{
				continue; // no data parent
			}

			dfgNode *predParent = node->parentClassification[0];
			dfgNode *dataParent = node->parentClassification[1];

			if (dataParent->getAncestors().size() > 0)
			{
				if (dataParent->parentClassification.find(0) != dataParent->parentClassification.end())
				{
					continue;
				}
			}

			if (dataParent->getChildren().size() == 1)
			{
				predParent->removeChild(node);
				node->removeAncestor(predParent);
				removeEdge(findEdge(predParent, node));

				dataParent->removeChild(node);
				node->removeAncestor(dataParent);
				removeEdge(findEdge(dataParent, node));

				delNodes.insert(node);

				predParent->addChildNode(dataParent);
				dataParent->addAncestorNode(predParent);
				dataParent->parentClassification[0] = predParent;

				std::vector<dfgNode *> children = node->getChildren();
				for (dfgNode *child : children)
				{
					dataParent->addChildNode(child);
					child->addAncestorNode(dataParent);

					LLVM_DEBUG(dbgs() << "parentClassification size = " << child->parentClassification.size() << "\n");
					//					if(child->parentClassification[0]==node){
					//						child->parentClassification[0]=dataParent;
					//					}
					//					else if(child->parentClassification[1]==node){
					//						child->parentClassification[1]=dataParent;
					//					}
					//					else if(child->parentClassification[2]==node){
					//						child->parentClassification[2]=dataParent;
					//					}
					//					else{
					//						assert(false);
					//					}
					bool found = false;
					for (std::pair<int, dfgNode *> pair : child->parentClassification)
					{
						if (pair.second == node)
						{
							child->parentClassification[pair.first] = dataParent;
							found = true;
							break;
						}
					}
					assert(found);

					node->removeChild(child);
					child->removeAncestor(node);
					removeEdge(findEdge(node, child));
				}
			}
		}
	}

	for (dfgNode *del : delNodes)
	{
		NodeList.erase(std::find(NodeList.begin(), NodeList.end(), del));
	}
}

void DFG::insertMOVC()
{
	int beforeNodeListSize = NodeList.size();
	std::vector<dfgNode *> tobeadded;

	for (dfgNode *node : NodeList)
	{
		if (node->hasConstantVal())
		{
			dfgNode *temp = new dfgNode(this);
			temp->setNameType("MOVC");
			temp->setIdx(NodeList.size() + tobeadded.size());
			temp->BB = node->BB;
			temp->setFinalIns(MOVC);
			temp->setConstantVal(node->getConstantVal());
			tobeadded.push_back(temp);

			temp->addChildNode(node);
			node->addAncestorNode(temp);
			node->parentClassification[2] = temp;
		}
	}

	for (dfgNode *newNode : tobeadded)
	{
		NodeList.push_back(newNode);
	}

	LLVM_DEBUG(dbgs() << "NodeList Size was increased : from=" << beforeNodeListSize << ", to=" << NodeList.size() << "\n");
}

void DFG::removeDisconnectedNodes()
{

	std::set<dfgNode *> rn;

	for (dfgNode *n : NodeList)
	{
		bool noAncestors = true;
		bool ps_edges_found = false;

		std::vector<dfgNode *> ancestors = n->getAncestors();
		for (dfgNode *anc : ancestors)
		{
			if (findEdge(anc, n)->getType() == EDGE_TYPE_PS)
			{
				ps_edges_found = true;
			}
			else
			{
				noAncestors = false;
			}
		}

		if (ps_edges_found && noAncestors)
		{
			for (dfgNode *anc : ancestors)
			{
				anc->removeChild(n);
				n->removeAncestor(anc);
			}
		}

		if (noAncestors && n->getChildren().size() == 0)
		{
			rn.insert(n);
		}
	}

	for (dfgNode *n : rn)
	{
		LLVM_DEBUG(dbgs() << "Removing Node = " << n->getIdx() << "\n");
		NodeList.erase(std::remove(NodeList.begin(), NodeList.end(), n), NodeList.end());
	}
}

std::unordered_set<dfgNode *> DFG::getLineage(dfgNode *n)
{

	std::unordered_set<dfgNode *> lin;

	std::queue<dfgNode *> q;
	q.push(n);
	lin.insert(n);

	while (!q.empty())
	{
		dfgNode *head = q.front();
		q.pop();
		for (dfgNode *anc : head->getAncestors())
		{
			if (head->ancestorBackEdgeMap[anc])
				continue;
			if (lin.find(anc) == lin.end())
			{
				q.push(anc);
				lin.insert(anc);
			}
		}
	}

	LLVM_DEBUG(dbgs() << "lineage = ");
	for (dfgNode *n : lin)
		LLVM_DEBUG(dbgs() << n->getIdx() << ",");
	LLVM_DEBUG(dbgs() << "\n");

	return lin;
}

void populateStructOffset(StructType *ST, std::unordered_map<int, int> &structOffset, const DataLayout &DL)
{
	int prev_offset = 0;
	int ctr = 0;
	for (auto type_el : ST->elements())
	{
		LLVM_DEBUG(dbgs() << "\t\t");
		LLVM_DEBUG(type_el->dump());
		structOffset[ctr] = prev_offset;
		prev_offset += DL.getTypeAllocSize(type_el);
		ctr++;
	}
}

int DFG::CalculateGEPBaseAddr(GetElementPtrInst *GEP)
{
	// dfgNode* gep_node = findNode(GEP); assert(gep_node);
	int base_addr = 0;
	const DataLayout DL = GEP->getParent()->getParent()->getParent()->getDataLayout();

	int total_size = -1;

	PointerType *PT = cast<PointerType>(GEP->getPointerOperand()->getType());
	if (StructType *ST = dyn_cast<StructType>(PT->getElementType()))
	{
		LLVM_DEBUG(dbgs() << "\t");
		LLVM_DEBUG(ST->dump());
		total_size = DL.getTypeAllocSize(ST);
		// LLVM_DEBUG(dbgs() << "\t\t sample element 2 = " ; ST->getElementType(2)->dump();
	}
	else if (ArrayType *AT = dyn_cast<ArrayType>(PT->getElementType()))
	{
		LLVM_DEBUG(dbgs() << "\t");
		LLVM_DEBUG(AT->dump());
		total_size = DL.getTypeAllocSize(AT);
	}
	else
	{
		//no further offset calculation is required.
		return base_addr;
	}

	int offset = 0;

	if (ConstantInt *cv_main_offset = dyn_cast<ConstantInt>(GEP->getOperand(1)))
	{
		int cv_main_offest_int = cv_main_offset->getSExtValue();
		offset = cv_main_offest_int * total_size;
		Type *currType = GEP->getSourceElementType();

		if (GEP->getNumOperands() > 1)
		{
			for (int i = 2; i < GEP->getNumOperands(); i++)
			{
				LLVM_DEBUG(dbgs() << "currType = ");
				LLVM_DEBUG(currType->dump());
				if (ConstantInt *CV = dyn_cast<ConstantInt>(GEP->getOperand(i)))
				{
					int cv_int = CV->getSExtValue();
					LLVM_DEBUG(dbgs() << "\t offset_idx" << i << "=" << cv_int << "\n");

					if (StructType *ST = dyn_cast<StructType>(currType))
					{
						LLVM_DEBUG(dbgs() << "ST Type!.\n");
						std::unordered_map<int, int> structOffset;
						populateStructOffset(ST, structOffset, DL);
						offset += structOffset[cv_int];
						currType = ST->getElementType(cv_int);
					}
					else if (ArrayType *AT = dyn_cast<ArrayType>(currType))
					{
						LLVM_DEBUG(dbgs() << "AT Type!.\n");
						int element_size = DL.getTypeAllocSize(AT->getElementType());
						offset += cv_int * element_size;
						currType = AT->getElementType();
					}
					else
					{
						LLVM_DEBUG(dbgs() << "scalar Type.\n");
						assert(i == GEP->getNumOperands() - 1);
					}
				}
			}
		}
	}

	LLVM_DEBUG(dbgs() << "\t offset = " << offset << "\n");

	return offset;
}

void DFG::GEPBaseAddrCheck(Function &F)
{

	for (auto &bb : F)
	{
		for (auto &ins : bb)
		{
			if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(&ins))
			{
				LLVM_DEBUG(dbgs() << "GEP = ");
				LLVM_DEBUG(GEP->dump());
				int offset = CalculateGEPBaseAddr(GEP);
				GEPOffsetMap[GEP] = offset;
				LLVM_DEBUG(dbgs() << "[dfg.cpp][GEPBaseAddrCheck] offset: " << offset << "\n");
			}
		}
	}
}

void checkGEPDerivatives(Instruction *root, GetElementPtrInst *GEP, std::unordered_map<Value *, GetElementPtrInst *> &deriv)
{
	for (User *u : root->users())
	{
		if(u == GEP) continue;
		if (Instruction *ins = dyn_cast<Instruction>(u))
		{
			if (ins->mayReadOrWriteMemory())
				continue;
			if (deriv.find(ins) != deriv.end())
				continue;

			LLVM_DEBUG(dbgs() << "\t value = ");
			LLVM_DEBUG(ins->dump());
			deriv[ins] = GEP;
			checkGEPDerivatives(ins, GEP, deriv);
		}
	}
}

//int DFG::getMUnitTransID(BasicBlock *src, BasicBlock *dest)
//{
//
//	for (munitTransition mut : munitTransitionsALL)
//	{
//		if (src == mut.srcBB && dest == mut.destBB)
//		{
//			return mut.id;
//		}
//	}
//	assert(false);
//}

/*
 *
 * Outputs:
 * 1) outer_vals
 *
 * contains values loaded from outerLoopStore/outerLoopLoads. These loads are scalar
 * load/stores corresponds to live in and out values such as outer loop induction variables
 * Eg:
 *
 *  %i.022 = phi i32 [ 0, %entry ], [ %inc8, %for.end ] (%i.022 is an induction variable passed in
 * as a live in variable)
 *
 *  %add = add nsw i32 %mul, %sum.021 (%add is a temp variable generated within loop and passed out
 *  as a live out variable)
 *
 * 2) mem_ptrs
 *
 * contains pointer operands of array loads/stores (??). Pointer operand is a GEP instruction.
 *
 * Eg: Value name:arrayidx4,
 * GEP =   %arrayidx4 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* @A, i32 0, i32 %i.022, i32 %j.020
 *
 * 3) acc
 *
 * contains the number of memory accesses for each array/ scalars.
 * Value represent the array base pointer or scalar pointer (??)
 * Eg:
 * base_ptr:A, accesses = 1 (array)
 * base_ptr:x, accesses = 1 (array)
 * base_ptr:add, accesses = 1 (outloopstore scalar)
 * base_ptr:i.022, accesses = 1 (outloopload scalar)
 *
 *
 * */

void DFG::getTransferVariables(std::unordered_set<Value *> &outer_vals,
		std::unordered_map<Value *, GetElementPtrInst *> &mem_ptrs,
		std::unordered_map<Value *, int> &acc,
		Function &F)
{

	assert(OutLoopNodeMap.size() == OutLoopNodeMapReverse.size());

	for (auto it = OutLoopNodeMapReverse.begin(); it != OutLoopNodeMapReverse.end(); it++)
	{
		LLVM_DEBUG(dbgs() << "onode = " << it->first->getIdx()  << "nametype = " << it->first->getNameType() << "\n");
		Value *val = it->second;
		outer_vals.insert(val);

		if(it->first->getNameType().compare("OutLoopSTORE")){
			outVals_inorout[val]=1;//load
		}else{
			outVals_inorout[val]=0;//store
		}
		acc[val] = acc[val] + 1;
	}

	/*
	 * gep_derivatives:
	 *
	 * Value: pointer operand of array loads/stores (??). Pointer operand is a GEP instruction.
	 * GetElementPtrInst: corresponding GEP instruction
	 * */

	std::unordered_map<Value *, GetElementPtrInst *> gep_derivatives;

	for (auto &bb : F)
	{
		for (auto &ins : bb)
		{
			if (gep_derivatives.find(&ins) != gep_derivatives.end())
				continue;
			if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(&ins))
			{
				LLVM_DEBUG(dbgs() << "searching for GEP = ");
				LLVM_DEBUG(GEP->dump());
				gep_derivatives[GEP] = GEP;
				checkGEPDerivatives(GEP, GEP, gep_derivatives);
			}
		}
	}
	/*
	 * sizeArrMap:
	 *
	 * Contains the array sizes passed from function annotations.
	 *
	 * Eg:
	 * __attribute__((annotate("fr:128,fi:128")))
	 * int fix_fft(short fr[], short fi[], short m1, short inverse){
	 * */
	for (dfgNode *node : NodeList)
	{
		if (node->getNode())
		{
			if (LoadInst *LDI = dyn_cast<LoadInst>(node->getNode()))
			{
				LLVM_DEBUG(dbgs() << "LOAD TRANSFER = ");
				LLVM_DEBUG(LDI->dump());
				Value *ld_ptr = LDI->getPointerOperand();

				if(sizeArrMap.find(ld_ptr->getName().str()) != sizeArrMap.end()){
					acc[LDI->getPointerOperand()] = acc[LDI->getPointerOperand()] + 1;
				}
				else{
					LLVM_DEBUG(dbgs()<<"ld_ptr:" );
					LLVM_DEBUG(ld_ptr->dump());
					assert(gep_derivatives.find(ld_ptr) != gep_derivatives.end());
					mem_ptrs[ld_ptr] = gep_derivatives[ld_ptr];
					acc[gep_derivatives[ld_ptr]->getPointerOperand()] = acc[gep_derivatives[ld_ptr]->getPointerOperand()] + 1;
				}

			}
			if (StoreInst *STI = dyn_cast<StoreInst>(node->getNode()))
			{
				LLVM_DEBUG(dbgs() << "STORE TRANSFER = ");
				LLVM_DEBUG(STI->dump());
				Value *st_ptr = STI->getPointerOperand();

				if(sizeArrMap.find(st_ptr->getName().str()) != sizeArrMap.end()){
					acc[STI->getPointerOperand()] = acc[STI->getPointerOperand()] + 1;
				}
				else{
					assert(gep_derivatives.find(st_ptr) != gep_derivatives.end());
					mem_ptrs[st_ptr] = gep_derivatives[st_ptr];
					acc[gep_derivatives[st_ptr]->getPointerOperand()] = acc[gep_derivatives[st_ptr]->getPointerOperand()] + 1;
				}

			}
		}
	}


	// for(auto it = OutLoopNodeMapReverse.begin(); it != OutLoopNodeMapReverse.end(); it++){
	// 	dfgNode* on = it->first;
	// 	acc[it->second] = 1;
	// }

	LLVM_DEBUG(dbgs()<<"\nOutvals_inorout contains: \n" );
	for (auto it = outVals_inorout.begin(); it != outVals_inorout.end(); it++)
	{
		Value* outvl = it->first;
		LLVM_DEBUG(dbgs() << "outVal:");
		LLVM_DEBUG(outvl->dump());
		LLVM_DEBUG(dbgs() << "\n");
		LLVM_DEBUG(dbgs() << "load(1)/store(0):"<<it->second <<"\n");
	}

}
/*
 * Set base pointer names of DFG nodes
 * Record sizes of each array/ scalar
 *
 * base_ptr:i.022, size = 4
 * base_ptr:add, size = 4
 * base_ptr:A, size = 64
 * base_ptr:x, size = 16
 *
 *
 * */
void DFG::SetBasePointers(std::unordered_set<Value *> &outer_vals,
		std::unordered_map<Value *, GetElementPtrInst *> &mem_ptrs, std::map<dfgNode*,Value*> &OLNodesWithPtrTyUsage, Function &F)
{


	LLVM_DEBUG(dbgs()<<"array_pointer_sizes contains: \n" );
	for (auto it = array_pointer_sizes.begin(); it != array_pointer_sizes.end(); it++)
	{
		std::string base_ptr = it->first;
		int size = it->second;
		LLVM_DEBUG(dbgs() << "base_ptr:" << base_ptr << ", size = " << size << "\n");
	}

	LLVM_DEBUG(dbgs() << "OutLoopNodeMapReverse size = " << OutLoopNodeMapReverse.size() << "\n");
	LLVM_DEBUG(dbgs() << "OutLoopNodeMap size = " << OutLoopNodeMap.size() << "\n");

	for (dfgNode *node : NodeList)
	{

		if (OutLoopNodeMapReverse.find(node) != OutLoopNodeMapReverse.end())
		{
			assert(outer_vals.find(OutLoopNodeMapReverse[node]) != outer_vals.end());
			LLVM_DEBUG(dbgs() << "Outer LOOP value :: ");
			LLVM_DEBUG(OutLoopNodeMapReverse[node]->dump());
			LLVM_DEBUG(dbgs() << "Outer LOOP name = " << OutLoopNodeMapReverse[node]->getName() << "\n");
			LLVM_DEBUG(dbgs() << "Outer LOOP node idx = " << node->getIdx() << "\n");
			node->setArrBasePtr(OutLoopNodeMapReverse[node]->getName());
			/*
			 *   %arrayidx = gep , []* @sum, ... //arrayidx is the address of sum[i]
			 *
			 *   for.body
			 *
			 *      store i32 %x, i32* %arrayidx    // store to address arrayidx
			 *
			 *
			 * Detect out loop nodes with pointer type usage in the loop body
			 * These nodes require special treatment in instrumentation
			 * because we need to record the local memory address, not the memory address
			 * generated in the ./final at runtime*/
			for (User *u : OutLoopNodeMapReverse[node]->users())
			{
				LLVM_DEBUG(dbgs() << "Users: ");
				LLVM_DEBUG(u->dump());
				LLVM_DEBUG(dbgs() << "\n");

				for (int i = 0; i < u->getNumOperands(); ++i)
				{
					if (u->getOperand(i) ==  OutLoopNodeMapReverse[node] && u->getOperand(i)->getType()->isPointerTy())
					{
						LLVM_DEBUG(dbgs() << "Have pointer type user\n");
						OLNodesWithPtrTyUsage[node]=OutLoopNodeMapReverse[node];
					}
				}

			}

			DataLayout DL = F.getParent()->getDataLayout();
			array_pointer_sizes[OutLoopNodeMapReverse[node]->getName()] = DL.getTypeAllocSize(OutLoopNodeMapReverse[node]->getType());
		}
		else if (node->getNode() && node->getNode()->mayReadOrWriteMemory())
		{
			if (LoadInst *LDI = dyn_cast<LoadInst>(node->getNode()))
			{
				Value *pointer = LDI->getPointerOperand();
				if(sizeArrMap.find(pointer->getName().str()) != sizeArrMap.end()){
					array_pointer_sizes[pointer->getName().str()] = sizeArrMap[pointer->getName().str()];
					node->setArrBasePtr(pointer->getName().str());
				}
				else{
					assert(mem_ptrs.find(pointer) != mem_ptrs.end());

					std::string base_ptr_name = mem_ptrs[pointer]->getPointerOperand()->getName();
					node->setArrBasePtr(base_ptr_name);

					if (sizeArrMap.find(base_ptr_name) != sizeArrMap.end())
					{
						array_pointer_sizes[base_ptr_name] = sizeArrMap[base_ptr_name];
					}
					else
					{
						DataLayout DL = LDI->getParent()->getParent()->getParent()->getDataLayout();
						PointerType *PT = cast<PointerType>(mem_ptrs[pointer]->getPointerOperand()->getType());
						array_pointer_sizes[base_ptr_name] = DL.getTypeAllocSize(PT->getElementType());
					}
				}
			}
			else if (StoreInst *STI = dyn_cast<StoreInst>(node->getNode()))
			{
				Value *pointer = STI->getPointerOperand();
				if(sizeArrMap.find(pointer->getName().str()) != sizeArrMap.end()){
					array_pointer_sizes[pointer->getName().str()] = sizeArrMap[pointer->getName().str()];
					node->setArrBasePtr(pointer->getName().str());
				}
				else{
					assert(mem_ptrs.find(pointer) != mem_ptrs.end());

					std::string base_ptr_name = mem_ptrs[pointer]->getPointerOperand()->getName();
					node->setArrBasePtr(base_ptr_name);

					if (sizeArrMap.find(base_ptr_name) != sizeArrMap.end())
					{
						array_pointer_sizes[base_ptr_name] = sizeArrMap[base_ptr_name];
					}
					else
					{
						DataLayout DL = STI->getParent()->getParent()->getParent()->getDataLayout();
						PointerType *PT = cast<PointerType>(mem_ptrs[pointer]->getPointerOperand()->getType());
						array_pointer_sizes[base_ptr_name] = DL.getTypeAllocSize(PT->getElementType());
					}
				}
			}
			else
			{
				//if the instruction could read or write memory it should be a load
				// or a store instruction.
				LLVM_DEBUG(node->getNode()->dump());
				assert(false);
			}
		}
		else if (node->getNode() && isa<GetElementPtrInst>(node->getNode()))
		{
			GetElementPtrInst *GEP = cast<GetElementPtrInst>(node->getNode());
			std::string base_ptr_name = GEP->getPointerOperand()->getName();
			node->setArrBasePtr(base_ptr_name);

			if (sizeArrMap.find(base_ptr_name) != sizeArrMap.end())
			{
				array_pointer_sizes[base_ptr_name] = sizeArrMap[base_ptr_name];
			}
			else
			{
				DataLayout DL = GEP->getParent()->getParent()->getParent()->getDataLayout();
				PointerType *PT = cast<PointerType>(GEP->getPointerOperand()->getType());
				array_pointer_sizes[base_ptr_name] = DL.getTypeAllocSize(PT->getElementType());
			}
		}


	}
	LLVM_DEBUG(dbgs() << "Outloop node instructions with pointer type usage in the loop body \n");
	for (auto i : OLNodesWithPtrTyUsage)
	{

		Value* ptr = i.second;
		LLVM_DEBUG(ptr->dump());
		LLVM_DEBUG(dbgs() << "\n");

	}

	LLVM_DEBUG(dbgs()<<"array_pointer_sizes contains: \n" );
	for (auto it = array_pointer_sizes.begin(); it != array_pointer_sizes.end(); it++)
	{
		std::string base_ptr = it->first;
		int size = it->second;
		LLVM_DEBUG(dbgs() << "base_ptr:" << base_ptr << ", size = " << size << "\n");
	}


}
/*
 * Instrument the code to save live in and live out data for simulation
 *
 *
 * */
void DFG::InstrumentInOutVars(Function &F, std::unordered_map<Value *, int> mem_accesses, std::map<dfgNode*,Value*> &OLNodesWithPtrTyUsage, std::unordered_map<Value *, int> &spm_base_address){

	LLVM_DEBUG(dbgs() << "Outloop node instructions with pointer type usage in the loop body \n");
	for (auto i : OLNodesWithPtrTyUsage)
	{

		Value* ptr = i.second;
		LLVM_DEBUG(ptr->dump());
		LLVM_DEBUG(dbgs() << "\n");

	}
	LLVMContext &Ctx = F.getContext();

	//Instrumentation functions

	//this function should be called at the beginning of the loop.
	auto loopStartFn = F.getParent()->getOrInsertFunction(
			"loopStart",
			FunctionType::getVoidTy(Ctx),
			Type::getInt8PtrTy(Ctx) //loopname
	);

	//this should be called at the end of the loop
	auto clearPrintedArrs = F.getParent()->getOrInsertFunction(
			"clearPrintedArrs",
			FunctionType::getVoidTy(Ctx));

	auto printArrFunc = F.getParent()->getOrInsertFunction(
			"printArr",
			FunctionType::getVoidTy(Ctx),
			Type::getInt8PtrTy(Ctx),
			Type::getInt8PtrTy(Ctx),
			Type::getInt32Ty(Ctx),
			Type::getInt8Ty(Ctx),
			Type::getInt32Ty(Ctx));

	auto reportDynArrSize = F.getParent()->getOrInsertFunction(
			"reportDynArrSize", FunctionType::getVoidTy(Ctx),
			Type::getInt8PtrTy(Ctx),
			Type::getInt8PtrTy(Ctx),
			Type::getInt32Ty(Ctx),
			Type::getInt32Ty(Ctx));

	auto printDynArrSize = F.getParent()->getOrInsertFunction(
			"printDynArrSize", FunctionType::getVoidTy(Ctx));

	auto loopEndFn = F.getParent()->getOrInsertFunction(
			"loopEnd", FunctionType::getVoidTy(Ctx), Type::getInt8PtrTy(Ctx));

	//these should be called prior to the loop start/end to report all the variables that are being accessed in the loop
	auto live_in_report_FN = F.getParent()->getOrInsertFunction(
			"LiveInReport",
			FunctionType::getVoidTy(Ctx),
			Type::getInt8PtrTy(Ctx), //variable name
			Type::getInt8PtrTy(Ctx), //variable data
			Type::getInt32Ty(Ctx) //size
	);
	auto live_in_report_FN2 = F.getParent()->getOrInsertFunction(
			"LiveInReport2",
			FunctionType::getVoidTy(Ctx),
			Type::getInt8PtrTy(Ctx), //variable name
			Type::getInt32PtrTy(Ctx), //variable data
			Type::getInt32Ty(Ctx) //size
	);

	auto live_in_report_PtrTypeUsage = F.getParent()->getOrInsertFunction(
			"LiveInReportPtrTypeUsage",
			FunctionType::getVoidTy(Ctx),
			Type::getInt8PtrTy(Ctx), //variable name
			Type::getInt8PtrTy(Ctx),//base addr var name
			Type::getInt32Ty(Ctx), //addr offset
			Type::getInt32Ty(Ctx) //size
	);

	auto live_out_report_FN = F.getParent()->getOrInsertFunction(
			"LiveOutReport",
			FunctionType::getVoidTy(Ctx),
			Type::getInt8PtrTy(Ctx), //variable name
			Type::getInt8PtrTy(Ctx), // variable data
			Type::getInt32Ty(Ctx) //size
	);

	auto live_in_report_intvar_FN = F.getParent()->getOrInsertFunction(
			"LiveInReportIntermediateVar",
			FunctionType::getVoidTy(Ctx),
			Type::getInt8PtrTy(Ctx), //variable name
			Type::getInt32Ty(Ctx) //variable data
	);

	auto live_out_report_intvar_FN = F.getParent()->getOrInsertFunction(
			"LiveOutReportIntermediateVar",
			FunctionType::getVoidTy(Ctx),
			Type::getInt8PtrTy(Ctx), //variable name
			Type::getInt32Ty(Ctx) //variable data
	);

	//Insertion of loop start functions to all entries
	for(auto trans : loopentryBB){
		BasicBlock* entryBB = trans.first;
		IRBuilder<> builder(entryBB->getTerminator());

		Value *loopName = builder.CreateGlobalStringPtr(this->kernelname);
		builder.CreateCall(loopStartFn,{loopName});
	}



	// Insertion of reporting functions at entry and exit basicblocks
	for(auto it = mem_accesses.begin(); it != mem_accesses.end(); it++){
		IRBuilder<> builder(NodeList[0]->getNode());

		Value* ptr = it->first;
		LLVM_DEBUG(dbgs() << "ptr_name = " << ptr->getName() << "\n");
		LLVM_DEBUG(ptr->dump());


		assert(array_pointer_sizes.find(ptr->getName()) != array_pointer_sizes.end());
		int size = array_pointer_sizes[ptr->getName()];

		LLVM_DEBUG(dbgs() << "size = " << size << "\n");

		Value* ptr_name_val = builder.CreateGlobalStringPtr(ptr->getName());
		Value *size_val = ConstantInt::get(Type::getInt32Ty(Ctx), size);
		Value *size_val2 = ConstantInt::get(Type::getInt32Ty(Ctx), size/4);
		bool isOLNodewithPtrTyUsage = false;
		for(auto trans : loopentryBB){
			BasicBlock* entryBB = trans.first;
			builder.SetInsertPoint(entryBB->getTerminator());

			for (auto i : OLNodesWithPtrTyUsage)
			{
				/*Instrument the out-loop-nodes with pointer type usage in the loop body.
				 * These nodes require special treatment in instrumentation
				 * because we need to record the local memory address, not the memory address
				 * generated in the ./final at runtime
				 * Works only for single dim arrays (required to flatten mem access in multi-dim arrays)
				 *
				 * */
				Value* ptrs = i.second;
				//				ptrs->dump();
				if(ptrs == ptr){
					GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(ptrs);
//					LLVM_DEBUG(dbgs() << "GEP operand 2 =  ";
					LLVM_DEBUG(GEP->getOperand(2)->dump());
					LLVM_DEBUG(GEP->getOperand(0)->dump());
//					LLVM_DEBUG(dbgs() << "array base = " << GEP->getOperand(0)->getName() << "\n";
					Value* gepop0 = builder.CreateGlobalStringPtr(GEP->getOperand(0)->getName());//base ptr of array
					Value * gepop2= GEP->getOperand(2);// array offset
//					LLVM_DEBUG(dbgs() << "\n";

					Value* ptr_spm_address_val = builder.CreateGlobalStringPtr(std::to_string(spm_base_address[GEP->getOperand(0)]));

					Value* bitcastedptr = builder.CreateIntCast(gepop2, Type::getInt32Ty(Ctx),true);
					builder.CreateCall(live_in_report_PtrTypeUsage,{ptr_name_val,ptr_spm_address_val,bitcastedptr,size_val});
					isOLNodewithPtrTyUsage = true;
					break;
				}
				isOLNodewithPtrTyUsage = false;
			}

			if(isOLNodewithPtrTyUsage == true){isOLNodewithPtrTyUsage = false;}
			else if(ptr->getType()->isPointerTy()){
				Value* bitcastedptr = builder.CreateBitCast(it->first, Type::getInt8PtrTy(Ctx));
				builder.CreateCall(live_in_report_FN,{ptr_name_val,bitcastedptr,size_val});
			//	Value* bitcastedptr2 = builder.CreateBitCast(it->first, Type::getInt32PtrTy(Ctx));
			//	builder.CreateCall(live_in_report_FN2,{ptr_name_val,bitcastedptr2,size_val2});
			}
			else{
				LLVM_DEBUG(dbgs() << "type = "); LLVM_DEBUG(ptr->getType()->dump());
				if(outVals_inorout.find(ptr)!=outVals_inorout.end()){
					if(outVals_inorout[ptr]==1){//outLoopLoad
						LLVM_DEBUG(dbgs() << "Adding live_in_report_intermediatevariable() call for OutloopLoad\n");
						Value* bitcastedptr = builder.CreateIntCast(it->first, Type::getInt32Ty(Ctx),true);
						builder.CreateCall(live_in_report_intvar_FN,{ptr_name_val,bitcastedptr});
					}
				}else{
					Value* bitcastedptr = builder.CreateIntCast(it->first, Type::getInt32Ty(Ctx),true);
					builder.CreateCall(live_in_report_intvar_FN,{ptr_name_val,bitcastedptr});
				}
			}

		}

		for(auto trans : loopexitBB){
			BasicBlock* exitBB = trans.second;
			builder.SetInsertPoint(exitBB->getTerminator());
			if(ptr->getType()->isPointerTy()){
				Value* bitcastedptr = builder.CreateBitCast(it->first, Type::getInt8PtrTy(Ctx));
				builder.CreateCall(live_out_report_FN,{ptr_name_val,bitcastedptr,size_val});
			}
			else{
				LLVM_DEBUG(dbgs() << "type = "); LLVM_DEBUG(ptr->getType()->dump());

				if(outVals_inorout.find(ptr)!=outVals_inorout.end()){
					if(outVals_inorout[ptr]==0){//outLoopStore
						LLVM_DEBUG(dbgs() << "Adding live_out_report_intermediatevariable() call for OutloopStore\n");
						for (auto &I : *exitBB){

							LLVM_DEBUG(I.dump());
							Value *val = dyn_cast<Value>(&I);
							LLVM_DEBUG(val->dump());
							if(I.getOperand(0)==ptr){
								LLVM_DEBUG(dbgs() << "lcssa use\n");
//								ptr_name_val = builder.CreateGlobalStringPtr(val->getName());
								Value* bitcastedptr = builder.CreateIntCast(val, Type::getInt32Ty(Ctx),true);
								builder.CreateCall(live_out_report_intvar_FN,{ptr_name_val,bitcastedptr});
							}
						}


						for (User *u : ptr->users()){
							LLVM_DEBUG(dbgs() << "Users: ");
							LLVM_DEBUG(u->dump());
							LLVM_DEBUG(dbgs() << "\n");
						}
//						Value* bitcastedptr = builder.CreateIntCast(it->first, Type::getInt32Ty(Ctx),true);
//						builder.CreateCall(live_out_report_intvar_FN,{ptr_name_val,bitcastedptr});
					}
				}else{
					Value* bitcastedptr = builder.CreateIntCast(it->first, Type::getInt32Ty(Ctx),true);
					builder.CreateCall(live_out_report_intvar_FN,{ptr_name_val,bitcastedptr});
				}
			}
		}

	}

	//Insertion of loop end function to all exits
	for(auto trans : loopexitBB){
		BasicBlock* exitBB = trans.second;
		IRBuilder<> builder(exitBB->getTerminator());

		Value *loopName = builder.CreateGlobalStringPtr(kernelname);
		builder.CreateCall(loopEndFn,{loopName});
	}

}

void DFG::UpdateSPMAllocation(std::unordered_map<Value *, int> &spm_base_address,
		std::unordered_map<Value *, SPM_BANK> &spm_base_allocation,
		std::unordered_map<Value *, GetElementPtrInst *> &arr_ptrs)
{
	std::ofstream mem_alloc_txt;
	mem_alloc_txt.open(this->kernelname + "_mem_alloc.txt");
	mem_alloc_txt << "var_name,base_addr\n";
	std::string var_name;
	int base_addr;
	std::unordered_map<std::string, int> base_address_map;
	for (dfgNode *node : NodeList)
	{
		if (OutLoopNodeMapReverse.find(node) != OutLoopNodeMapReverse.end() && OutLoopNodeMapReverse[node])
		{
			assert(spm_base_address.find(OutLoopNodeMapReverse[node]) != spm_base_address.end());
			LLVM_DEBUG(dbgs() << "SetOutLoopLOAD/STORE BaseAddresses :: setting outerloop value=" << OutLoopNodeMapReverse[node]->getName() << " to " << spm_base_address[OutLoopNodeMapReverse[node]] << "\n");
			node->setConstantVal(spm_base_address[OutLoopNodeMapReverse[node]]);
			node->setoutloopAddr(spm_base_address[OutLoopNodeMapReverse[node]]);//this is assigned to constval in Namenodes()
//			LLVM_DEBUG(dbgs() << "Const val: " << node->getConstantVal() << "/n";
			assert(spm_base_allocation.find(OutLoopNodeMapReverse[node]) != spm_base_allocation.end());
			if (spm_base_allocation[OutLoopNodeMapReverse[node]] == BANK0)
			{
				node->setLeftAlignedMemOp(1);
			}
			else
			{
				node->setLeftAlignedMemOp(2);
			}
			LLVM_DEBUG(dbgs() << "\t pointer " << OutLoopNodeMapReverse[node]->getName() << "\n");
			LLVM_DEBUG(dbgs() << "\t to address " << spm_base_address[OutLoopNodeMapReverse[node]] << "\n");
			var_name = OutLoopNodeMapReverse[node]->getName();
			base_addr = spm_base_address[OutLoopNodeMapReverse[node]];
			base_address_map[var_name]=base_addr;
		}
		else if (node->getNode())
		{
			if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(node->getNode()))
			{
				Value *gep_pointer = GEP->getPointerOperand();
				if (spm_base_address.find(gep_pointer) != spm_base_address.end())
				{
					node->setGEPbaseAddr(spm_base_address[gep_pointer]);
					LLVM_DEBUG(dbgs() << "SetNewGEPBaseAddresses :: setting GEP base address for=");
					LLVM_DEBUG(GEP->dump());
					LLVM_DEBUG(dbgs() << "\t pointer " << gep_pointer->getName() << "\n");
					LLVM_DEBUG(dbgs() << "\t to address " << spm_base_address[gep_pointer] << "\n");
					var_name = gep_pointer->getName();
					base_addr = spm_base_address[gep_pointer];
					base_address_map[var_name]=base_addr;
					//					mem_alloc_txt << var_name<<","<< base_addr<<"\n";
				}
			}
			else if (LoadInst *LDI = dyn_cast<LoadInst>(node->getNode()))
			{
				Value *ptr = LDI->getPointerOperand();
				LLVM_DEBUG(ptr->dump());

				if(arr_ptrs.find(ptr) == arr_ptrs.end()){
					LLVM_DEBUG(dbgs() << "\t pointer  " << ptr->getName() << "\n");
					LLVM_DEBUG(dbgs() << "\t to address " << spm_base_address[ptr] << "\n");
				}

				assert(arr_ptrs.find(ptr) != arr_ptrs.end());
				Value *gep_ptr = arr_ptrs[ptr]->getPointerOperand();
				assert(spm_base_allocation.find(gep_ptr) != spm_base_allocation.end());

				if (spm_base_allocation[gep_ptr] == BANK0)
				{
					node->setLeftAlignedMemOp(1);
				}
				else
				{
					node->setLeftAlignedMemOp(2);
				}

				LLVM_DEBUG(dbgs() << "\t pointer  " << gep_ptr->getName() << "\n");
				LLVM_DEBUG(dbgs() << "\t to address " << spm_base_address[gep_ptr] << "\n");
				var_name = gep_ptr->getName();
				base_addr = spm_base_address[gep_ptr];
				base_address_map[var_name]=base_addr;
			}
			else if (StoreInst *STI = dyn_cast<StoreInst>(node->getNode()))
			{
				Value *ptr = STI->getPointerOperand();
				LLVM_DEBUG(ptr->dump());
				assert(arr_ptrs.find(ptr) != arr_ptrs.end());
				Value *gep_ptr = arr_ptrs[ptr]->getPointerOperand();
				assert(spm_base_allocation.find(gep_ptr) != spm_base_allocation.end());

				if (spm_base_allocation[gep_ptr] == BANK0)
				{
					node->setLeftAlignedMemOp(1);
				}
				else
				{
					node->setLeftAlignedMemOp(2);
				}

				LLVM_DEBUG(dbgs() << "\t pointer " << gep_ptr->getName() << "\n");
				LLVM_DEBUG(dbgs() << "\t to address " << spm_base_address[gep_ptr] << "\n");
				var_name = gep_ptr->getName();
				base_addr = spm_base_address[gep_ptr];
				base_address_map[var_name]=base_addr;
			}
		}
	}

	LLVM_DEBUG(dbgs() << "\n\nName nodes begin\n");
	nameNodes();
	LLVM_DEBUG(dbgs() << "\nName nodes end\n\n");

	LLVM_DEBUG(dbgs() << "\n\n Writing mem alloc begin \n");

	for(auto i = base_address_map.begin(); i != base_address_map.end(); i++){
		LLVM_DEBUG(dbgs() << "\t pointer " << i->first<<","<< i->second<<"\n");
		mem_alloc_txt << i->first<<","<< i->second<<"\n";
	}
#ifdef ARCHI_16BIT
		mem_alloc_txt << "loopstart" <<","<<(MEM_SIZE - 1)<<"\n";
		mem_alloc_txt << "storestart" <<","<<(MEM_SIZE - 1)<<"\n";
		mem_alloc_txt << "loopend" <<","<<(MEM_SIZE - 2)<<"\n";
#else
		mem_alloc_txt << "loopend" <<","<<(MEM_SIZE/2 - 1)<<"\n";
//		mem_alloc_txt << "storestart" <<","<<(MEM_SIZE/2 - 1)<<"\n";
		mem_alloc_txt << "loopstart" <<","<<(MEM_SIZE - 2)<<"\n";
#endif
	mem_alloc_txt.close();
	LLVM_DEBUG(dbgs() << "\n Writing mem alloc end\n\n");
	//std::cout << "Data layout generated ("<<this->kernelname<<"_mem_alloc.txt) \n";
	// assert(false);
}

int DFG::insertshiftGEPsCorrect(){

	std::unordered_set<dfgNode *> newNodes;

	for(dfgNode* node : NodeList){
		if(node->getNode()){
			if(GetElementPtrInst* GEP = dyn_cast<GetElementPtrInst>(node->getNode())){
				DataLayout DL = GEP->getParent()->getParent()->getParent()->getDataLayout();
				int array_size = DL.getTypeAllocSize(GEP->getSourceElementType());

				/* Find total number of elements in array */
				Type *T = cast<PointerType>(GEP->getPointerOperandType())->getElementType();
				int no_of_elements;

				if (isa<ArrayType>(GEP->getSourceElementType())){
					no_of_elements = cast<ArrayType>(T)->getNumElements();
					LLVM_DEBUG(dbgs() << "is an array\n");
				}else{
					no_of_elements = 1;
					LLVM_DEBUG(dbgs()<< "not an array\n");
				}

				int size = array_size/no_of_elements;

				LLVM_DEBUG(dbgs() << "total array size=" << array_size << " no_of_elements=" << no_of_elements  << " element size(bytes)=" << size << "\n" );
				LLVM_DEBUG(GEP->dump());

				if (size > 1)
				{
					for(dfgNode* anc : node->getAncestors()){
						//ignore predicates
						if(anc->childConditionalMap[node] != UNCOND) continue;

						dfgNode *temp;
						temp = new dfgNode(this);
						temp->setNameType("GEPLEFTSHIFT");
						temp->setIdx(NodeList.size() + newNodes.size());
						newNodes.insert(temp);
#ifdef ARCHI_16BIT				
						temp->setConstantVal(Log2_32(size)-1);
#else
						temp->setConstantVal(Log2_32(size));
#endif

						temp->BB = node->BB;
						LLVM_DEBUG(dbgs() << "adding node...\n");

						anc->addChildNode(temp);
						temp->addAncestorNode(anc);


						temp->addChildNode(node);
						node->addAncestorNode(temp);

						anc->removeChild(node);
						node->removeAncestor(anc);
					}



				}

			}
		}
	}

	for (dfgNode *n : newNodes)
	{
		NodeList.push_back(n);
	}

	// assert(false);
}
