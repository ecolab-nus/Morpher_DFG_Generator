#ifndef DFGFULLPRED_H
#define DFGFULLPRED_H

//#include "../src/DFGPartPred.h"
#include <morpherdfggen/dfg/DFGPartPred.h>


class DFGFullPred : public DFGPartPred{
    public:
    DFGFullPred(std::string name,std::map<Loop*,std::string>* lnPtr, Loop* l) : DFGPartPred(name,lnPtr,l) {}
    void connectBBTrig();  
    void generateTrigDFGDOT(Function &F);
    void printNewDFGXML();
};


#endif
