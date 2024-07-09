#ifndef REMOVE_AGI_H
#define REMOVE_AGI_H

#include "llvm/Analysis/CFG.h"
#include <algorithm>
#include <queue>
#include <map>
#include <set>
#include <vector>

void RemoveAGIExter()
{
#ifdef REMOVE_AGI
	cerr <<"Experimental version with no AGI instructions. Don't use REMOVE_AGI directive in normal compilation"<< endl;
	int baseline_node_count = NodeList.size();
	removeAGI();
	int post_removal_node_count = NodeList.size();
	LLVM_DEBUG(dbgs() << "-------------------------------------BASELINE NODE COUNT = " << baseline_node_count << "\n");
	LLVM_DEBUG(dbgs() << "-------------------------------------POST REMOVAL NODE COUNT = " << post_removal_node_count << "\n");

	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][scheduleASAP begin]\n");
	scheduleASAP();
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][scheduleASAP end]\n\n");
	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][scheduleALAP begin]\n");
	scheduleALAP();
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][scheduleALAP end]\n\n");

	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][nameNodes begin]\n");
	nameNodes();
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][nameNodes end]\n\n");

	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][classifyParents begin]\n");
	classifyParents();
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][classifyParents end]\n\n");
	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][removeDisconnectedNodes begin]\n");
	removeDisconnetedNodes();
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][removeDisconnectedNodes end]\n\n");
//
//	changeTypeofSingleSourceCompNodes();
//	removeDisconnetedNodes();
	printDOT(this->name + "_AGIremovedDFG.dot");
	printNewDFGXML();


#else
	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][GEPBaseAddrCheck begin]\n");
	GEPBaseAddrCheck(F);
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][GEPBaseAddrCheck end]\n\n");
	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][nameNodes begin]\n");
	nameNodes();
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][nameNodes end]\n\n");
	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][classifyParents begin]\n");
	classifyParents();
	// RemoveInductionControlLogic();

	// RemoveBackEdgePHIs();
	// removeOutLoopLoad();
	//	RemoveConstantCMERGEs(); Originally on morpher
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][classifyParents end]\n\n");
	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][removeDisconnectedNodes begin]\n");
	//removeDisconnectedNodes();
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][removeDisconnectedNodes end]\n\n");
	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][addOrphanPseudoEdges begin]\n");
	addOrphanPseudoEdges();
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][addOrphanPseudoEdges end]\n\n");
	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][addRecConnsAsPseudo begin]\n");
	addRecConnsAsPseudo();
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][addRecConnsAsPseudo end]\n\n");
	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][changeTypeofSingleSourceCompNodes begin]\n");
	// printDOT(this->name + "_PartPredDFG.dot");
	// printNewDFGXML();
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][changeTypeofSingleSourceCompNodes end]\n\n");
	LLVM_DEBUG(dbgs() << "\n[DFGPartPred.cpp][removeDisconnectedNodes begin]\n");


	//function removeDisconnetedNodes() should be called at last, otherwise same idx would be reused
	//when new instructions are added
	removeDisconnetedNodes();
	LLVM_DEBUG(dbgs() << "[DFGPartPred.cpp][removeDisconnectedNodes end]\n\n");
#endif
}

#endif