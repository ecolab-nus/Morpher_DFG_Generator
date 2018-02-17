#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

void printArr(const char* name, uint8_t* arr, int size, uint8_t io, uint32_t addr);
void reportDynArrSize(const char* name, uint8_t* arr, uint32_t idx_i, int size);
void printDynArrSize();
void clearPrintedArrs();
void loopEnd(const char* loopName);
void loopStart(const char* loopName);
void outloopValueReport(uint32_t nodeIdx, uint32_t value, uint32_t addr, uint8_t isLoad, uint8_t isHostTrans, uint8_t size);

void loopTraceOpen(const char* fnName);
void loopTraceClose();
void loopInvoke(const char* loopName);
void loopInvokeEnd(const char* loopName);
void loopInsUpdate(const char* name, int insCount);

void loopBBInsUpdate(const char* loopName, const char* BBName, int insCount);
void loopBBMappingUnitUpdate(const char* BBName, const char* munitName);

void loopInsClear(const char* name);
void loopBBInsClear();

void updateLoopPreHeader(const char* loopName, const char* preheaderBB);

void reportExecInsCount(int count);
void recordUncondMunitTransition(const char* srcBB, const char* destBB);
void recordCondMunitTransition(const char* srcBB, const char* destBB1, const char* destBB2, int condition);


#ifdef __cplusplus
}
#endif
