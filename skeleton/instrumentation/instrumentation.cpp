#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include <map>
#include <vector>
#include <set>
#include <sstream> 

#include "instrumentation.h"

using namespace std;

static std::map<std::string,int> dynArrSizes;
static std::map<std::string,uint8_t*> dynArrPointers;
static std::map<std::string,std::vector<uint8_t> > dynByteArrCopy; 
static std::set<std::string> printedArrs;

static FILE* currentFile;
static std::map<std::string,FILE*> currentFiles;

static FILE* loopInvTraceFile;

//static int loopRunIdx=0;
static std::map<std::string,int> loopRunIdx;
static std::map<std::string,int> LoopInsCount;

typedef struct{
	uint8_t pre_data;
	uint8_t post_data;
	std::string name;
} AddrDataTuple;

static std::map<uint32_t,AddrDataTuple> data;

void printArr(const char* name, uint8_t* arr, int size, uint8_t io, uint32_t addr){

	std::string str(name);
	if(printedArrs.find(str)!=printedArrs.end()){
		//returning this has been print prior to the loop once.
		return;
 	}

	std::cout << "printArr:" ; 

	if(io==1){
		std::cout << "#####INPUT######\n";
	}
	else{
        std::cout << "#####OUTPUT######\n";
	}

	std::cout << "ArrName=" << name << ",ArrSize=" << size << ",Addr=" << addr <<  "\n";
	for (int i = 0; i < size; ++i)
	{
		if(io==1){
			AddrDataTuple tuple;
			tuple.name = str;
			tuple.pre_data=arr[i];
			data[addr+i]=tuple;
		}
		else{
			data[addr+i].post_data=arr[i];
		}
		std::cout << (int)arr[i] << ",";
	}
	std::cout << "\n";
	printedArrs.insert(str);
}

void reportDynArrSize(const char* name, uint8_t* arr, uint32_t idx_i, int size){
	std::string str(name);
	uint64_t idx = size*(idx_i+1);

	dynArrPointers[str]=arr;
	//std::cout << "Ptr=" << (uint64_t)arr << "\n";

	if(dynArrSizes.find(str)!=dynArrSizes.end()){
		if(idx > dynArrSizes[str]){
			for (int i = dynArrSizes[str]; i < idx; ++i)
			{
				//std::cout << "Arr[i]=" << (int)arr[i] << "Size=" << idx << "\n";
				dynByteArrCopy[str].push_back(arr[i]);
			}
			dynArrSizes[str]=idx;
		}
	}
	else{
		for (int i = 0; i < idx; ++i)
			{
				//std::cout << "Arr[i]=" << (int)arr[i] << "Size=" << idx << "\n";
				dynByteArrCopy[str].push_back(arr[i]);
			}
		dynArrSizes[str]=idx;
	}

}

void printDynArrSize(){
	std::string name;
	uint64_t maxIdx;
	std::map<string,int>::iterator dynArrSizesIt;

	for (dynArrSizesIt=dynArrSizes.begin(); dynArrSizesIt!=dynArrSizes.end(); dynArrSizesIt++)
	{
		name=dynArrSizesIt->first;
		maxIdx=dynArrSizesIt->second;

		std::cout << "printDynArrSize" << "\n";
		std::cout << "ArrName=" << name << ",ArrSize=" << maxIdx << "\n";
		for (int i = 0; i < maxIdx; ++i)
		{
			std::cout << (int)dynByteArrCopy[name][i] << ",";
		}
		std::cout << "\n";

	}
	dynArrSizes.clear();
	dynByteArrCopy.clear();

	std::cout << "\n";
	std::cout << "----------------------------------------------------------\n";
	std::cout << "----------------------------------------------------------\n";
	std::cout << "----------------------------------------------------------\n";
	printedArrs.clear();
}

void clearPrintedArrs(){
	// std::cout << "### OUTPUTS ###" << "\n";
	printedArrs.clear();
}

void outloopValueReport(uint32_t nodeIdx, uint32_t value, uint32_t addr, uint8_t isLoad, uint8_t isHostTrans,uint8_t size){
	std::stringstream ss;
	uint8_t* bytePtr = (uint8_t*)&value;

	if(isLoad==1){
		cout << "OutLoopLoadNode:" << nodeIdx << ",val=" << value << ",addr=" << addr << ",host=" << isHostTrans <<"\n";
		ss << "LoadNode:" << nodeIdx;
	}
	else{
		cout << "OutLoopStoreNode:" << nodeIdx << ",val=" << value << ",addr=" << addr << ",host=" << isHostTrans <<"\n";
		ss << "StoreNode:" << nodeIdx;
	}

	for (int i = 0; i < size; ++i) {
		AddrDataTuple tuple;
		tuple.name = ss.str();

		if(isLoad==1){
			tuple.pre_data = bytePtr[i];
			tuple.post_data = bytePtr[i];
		}
		else{
			tuple.pre_data = 0;
			tuple.post_data = bytePtr[i];
		}

		data[addr+i]=tuple;
	}
}

void PrintData(std::string ln){
	std::map<uint32_t,AddrDataTuple>::iterator dataIt;
	fprintf(currentFiles[ln],"addr,pre-run-data,post-run-data\n");
	for (dataIt = data.begin(); dataIt != data.end(); ++dataIt) {
		fprintf(currentFiles[ln],"%d,%d,%d\n",dataIt->first,dataIt->second.pre_data,dataIt->second.post_data);
	}
}


void loopStart(const char* loopName){
	std::string ln(loopName);
	std::stringstream ss;

	if(loopRunIdx.find(ln)==loopRunIdx.end()){
		loopRunIdx[ln]=0;
	}

	ss << "memtraces/loop_"  << loopName  << "_" << loopRunIdx[ln] << ".txt";
	assert(currentFiles[ln] = fopen(ss.str().c_str(),"w"));
	cout << ln << "------------LOOP START--------------\n";
}

void loopEnd(const char* loopName){
	std::string ln(loopName);
	PrintData(ln);
	fclose(currentFiles[ln]);
	loopRunIdx[ln]++;
	cout << ln <<"------------LOOP END----------------\n";
}

void loopTraceOpen(const char* fnName){
	std::string fnNameStr(fnName);
	fnNameStr = fnNameStr + "_looptrace.log";
	loopInvTraceFile = fopen(fnNameStr.c_str(), "a");
	fprintf(loopInvTraceFile,"call %s\n",fnName);
}

void loopTraceClose(){
	fclose(loopInvTraceFile);
}

static int currentExecutedIns=0;

void reportExecInsCount(int count){
	currentExecutedIns+=count;
}

void loopInvoke(const char* loopName){
	std::string ln(loopName);
	std::string loopNumber = ln.substr(2);
	int loopNumberInt = atoi(loopNumber.c_str());

	std::stringstream ss;
	for (int i = loopNumber.size(); i < 3; ++i) {
		loopNumberInt = loopNumberInt * 10;
	}
	ss << loopNumber;
	fprintf(loopInvTraceFile,"%s,START,%d\n",ss.str().c_str(),currentExecutedIns);
//	cout << "LoopNumber" << loopNumber << "\n";
}

void loopInvokeEnd(const char* loopName){
	std::string ln(loopName);
	std::string loopNumber = ln.substr(2);
	int loopNumberInt = atoi(loopNumber.c_str());

	std::stringstream ss;
	for (int i = loopNumber.size(); i < 3; ++i) {
		loopNumberInt = loopNumberInt * 10;
	}
	ss << loopNumber;
	fprintf(loopInvTraceFile,"%s,END,%d\n",ss.str().c_str(),currentExecutedIns);
}

void loopInsUpdate(const char* name, int insCount){
	std::string nameStr(name);
	LoopInsCount[nameStr]+=insCount;
}

void loopInsClear(const char* name){
	std::string nameStr(name);
	LoopInsCount[nameStr]=0;
}

