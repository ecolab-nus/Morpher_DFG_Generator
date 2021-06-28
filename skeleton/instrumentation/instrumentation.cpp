#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <fstream>
#include <ostream>
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
static FILE* MUnitInvTraceFile;
static std::string currFnName;

//static int loopRunIdx=0;
static std::map<std::string,int> loopRunIdx;
static std::map<std::string,int> LoopInsCount;
static std::map<std::string,int> BBInsCount;
static std::map<std::string,int> InsInBB;
static std::map<std::string,std::set<std::string> > loopBasicBlocks;
static std::map<std::string,std::string> BBMappingUnitMap;
static std::map<std::string,std::string> loopPreHeaderBBMap;

//recording transitions
                //src Munit           //dest Munit
static std::map<std::string,std::map<std::string,int> > munitTransProfile;


//2018 triggered execution code
struct pathInfo{
	private:
	std::vector<std::string> currentBBPath;
	int currentBBPathCount=0;

	public:
	void clear(){
		currentBBPath.clear();
		currentBBPathCount=0;
	}

	bool insertBB(std::string bbNameStr){
		if(std::find(currentBBPath.begin(),currentBBPath.end(),bbNameStr) != currentBBPath.end()){
			return false;
		}
		//the basic block is not contained
		currentBBPath.push_back(bbNameStr);
		return true;
	}

	void incrementPathCount(){currentBBPathCount++;}
	std::vector<std::string> getBBStrArry(){return currentBBPath;}
	int getPathCount(){return currentBBPathCount;}
};
pathInfo currentPath;
std::map<std::string,std::vector<std::vector<pathInfo>>> loopPathTrace;
std::vector<pathInfo> pathTrace;
std::map<std::vector<std::string>,int> pathsSoFar;
pathInfo trimPreHeader(std::string preheaderBB, pathInfo path);



typedef struct{
	uint8_t pre_data;
	uint8_t post_data;
	std::string name;
} AddrDataTuple;

static std::map<uint32_t,AddrDataTuple> data;


typedef struct{
	vector<uint8_t> pre_data;
	vector<uint8_t> post_data;
	std::string name;
} AddrDataTupleMorpher;

static std::map<std::string,AddrDataTupleMorpher> data_morpher;

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

void PrintDataMorpher(std::string ln){
	fprintf(currentFiles[ln],"var_name,offset,pre-run-data,post-run-data\n");

	for(auto it = data_morpher.begin(); it != data_morpher.end(); it++){
		string var_name = it->first;
		AddrDataTupleMorpher data_tuple = it->second;
		for(int offset = 0; offset < data_tuple.pre_data.size(); offset++){
			fprintf(currentFiles[ln],"%s,%d,%d,%d\n",var_name.c_str(),offset,data_tuple.pre_data[offset],data_tuple.post_data[offset]);
		}
	}
}


void loopStart(const char* loopName){
	std::string ln(loopName);
	std::stringstream ss;
	data_morpher.clear();

	if(loopRunIdx.find(ln)==loopRunIdx.end()){
		loopRunIdx[ln]=0;
	}

	ss << "memtraces/loop_"  << loopName  << "_" << loopRunIdx[ln] << ".txt";
	assert(currentFiles[ln] = fopen(ss.str().c_str(),"w"));
	cout << ln << "------------LOOP START--------------\n";
}

void loopEnd(const char* loopName){
	std::string ln(loopName);
	PrintDataMorpher(ln);
	fclose(currentFiles[ln]);
	loopRunIdx[ln]++;
	cout << ln <<"------------LOOP END----------------\n";
}

void loopTraceOpen(const char* fnName){
	std::string fnNameStr(fnName);
	currFnName = fnNameStr;
	fnNameStr = fnNameStr + "_looptrace.log";
	loopInvTraceFile = fopen(fnNameStr.c_str(), "a");
	assert(loopInvTraceFile!=NULL);

//	std::cout << "loopTrace opened\n";

	std::string MUnitInvTraceFileName = currFnName + "_munittrace.log";
	MUnitInvTraceFile = fopen(MUnitInvTraceFileName.c_str(),"a");

	fprintf(MUnitInvTraceFile,"call begin : %s\n",fnName);
	fprintf(loopInvTraceFile,"call begin : %s\n",fnName);
}

void loopTraceClose(){

	FILE* looptraceinfoFile;
	std::string looptraceinfoFileName = currFnName + "_loopinfo.log";
	looptraceinfoFile = fopen(looptraceinfoFileName.c_str(), "a");

	fprintf(looptraceinfoFile,"BB Profile Begin : \n");
	for(std::pair<std::string,int> pair : BBInsCount){
		fprintf(looptraceinfoFile,"BBName:%s,\t%d \n",pair.first.c_str(),pair.second);
	}
	fprintf(looptraceinfoFile,"BB Profile End. \n");

	fprintf(looptraceinfoFile,"Loop Average Instructions Per Iteration : \n");
	for(std::pair<std::string,std::set<std::string> > pair : loopBasicBlocks){
		std::string loopName = pair.first;
		fprintf(looptraceinfoFile,"LoopName:%s\n",pair.first.c_str());

		int BBInvCount=0;
		for (std::string BBName : pair.second){
			BBInvCount+=BBInsCount[BBName];
		}
		fprintf(looptraceinfoFile,"TotalBBInvCount=%d\n",BBInvCount);

		int maxInvBB=0;
		for (std::string BBName : pair.second){
			float BBInvCountFloat = (float)BBInvCount;
			float BBInsCountFloat = (float)BBInsCount[BBName];
			fprintf(looptraceinfoFile,"\t BBName:%s,BBInsCount:%d,BBInvCount:%d,BBLoopInvRatio:%f\n",
					BBName.c_str(),
					InsInBB[BBName],
					BBInsCount[BBName],
					BBInsCountFloat/BBInvCountFloat);
			maxInvBB = std::max(maxInvBB,BBInsCount[BBName]);
		}

		int estimatedIterations = BBInvCount/maxInvBB;

		float avgLpInstructions=0;
		for (std::string BBName : pair.second){
			float invWeight = (float)BBInsCount[BBName]/(float)maxInvBB;
			avgLpInstructions+=invWeight*(float)InsInBB[BBName];
		}
		fprintf(looptraceinfoFile,"avgLpInstructions=%f\n",avgLpInstructions);
		std::string preHeaderBBName = loopPreHeaderBBMap[loopName];
		fprintf(looptraceinfoFile,"PreHeaderInvokations::BasicBlockName=%s,InvCount=%d\n\n",preHeaderBBName.c_str(),BBInsCount[preHeaderBBName]);
	}
	fprintf(looptraceinfoFile,"Loop Invocations Begin : \n");
	for(std::pair<std::string,std::set<std::string> > pair : loopBasicBlocks){
		std::string loopName = pair.first;
		fprintf(looptraceinfoFile,"%s,%d\n",loopName.c_str(),BBInsCount[loopPreHeaderBBMap[loopName]]);
	}
	fprintf(looptraceinfoFile,"Loop Invocations End.\n");

	fprintf(looptraceinfoFile,"Just Plain Number of Invocations : \n");
	for(std::pair<std::string,std::set<std::string> > pair : loopBasicBlocks){
		for (std::string BBName : pair.second){
			fprintf(looptraceinfoFile,"%s,%d,%s,%s\n",BBName.c_str(),
					                       BBInsCount[BBName],
										   BBMappingUnitMap[BBName].c_str(),
										   pair.first.c_str());
		}
	}

	for(std::pair<std::string,std::map<std::string,int> > pair1 : munitTransProfile){
//		fprintf(loopInvTraceFile,"%s,",pair1.first.c_str());
		for(std::pair<std::string,int> pair2 : pair1.second){
			fprintf(looptraceinfoFile,"%s,%s,%d\n",pair1.first.c_str(),pair2.first.c_str(),pair2.second);
		}
//		fprintf(loopInvTraceFile,"\n");
	}

	fprintf(MUnitInvTraceFile,"call end.\n");
	fprintf(loopInvTraceFile,"call end.\n");

	fprintf(looptraceinfoFile,"call end.\n");
	fprintf(looptraceinfoFile,"************************************************\n");



	fclose(MUnitInvTraceFile);
	fclose(loopInvTraceFile);
	fclose(looptraceinfoFile);
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
	assert(loopInvTraceFile!=NULL);
//    std::cout << "LoopNumber" << loopNumber << "," << ss.str() << "," << currentExecutedIns << "\n";
	fprintf(loopInvTraceFile,"%s,START,%d\n",ss.str().c_str(),currentExecutedIns);
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

	std::stringstream sstrig;
	reportLoopEnd(loopName);
	//2018 Triggered Instruction Work
	for(pathInfo pi : pathTrace){
		sstrig << ln << ",";
		for(std::string bbName : pi.getBBStrArry()){
			sstrig << bbName << "-->";
		}
		sstrig << "," << pi.getPathCount() << "\n";
	}
	fprintf(loopInvTraceFile,"%s",sstrig.str().c_str());
//	pathTrace.clear();
	//---------------------------------

	fprintf(loopInvTraceFile,"%s,END,%d\n",ss.str().c_str(),currentExecutedIns);
}

void loopInsUpdate(const char* name, int insCount){
	std::string nameStr(name);
	LoopInsCount[nameStr]+=insCount;
}

void loopBBInsUpdate(const char* loopName, const char* BBName, int insCount){
	std::string BBnameStr(BBName);
	std::string loopNameStr(loopName);

	if(BBInsCount.find(BBnameStr)==BBInsCount.end()){
		BBInsCount[BBnameStr]=0;
	}
//	BBInsCount[nameStr]+=insCount;
	loopBasicBlocks[loopNameStr].insert(BBnameStr);
	InsInBB[BBnameStr]=insCount;
	BBInsCount[BBnameStr]++;

	//2018 Triggered Instruction Work
	reportNewBBinPath(BBName,loopName);
}

void loopInsClear(const char* name){
	std::string nameStr(name);
	LoopInsCount[nameStr]=0;
}

void loopBBInsClear(){
//	BBInsCount.clear();
}

void updateLoopPreHeader(const char* loopName, const char* preheaderBB){
	std::string loopNameStr(loopName);
	std::string preheaderBBStr(preheaderBB);
	loopPreHeaderBBMap[loopNameStr]=preheaderBBStr;
}

void loopBBMappingUnitUpdate(const char* BBName, const char* munitName){
	std::string BBNameStr(BBName);
	std::string munitNameStr(munitName);

	BBMappingUnitMap[BBNameStr]=munitNameStr;
}

void recordUncondMunitTransition(const char* srcBB, const char* destBB){
	std::string srcMunitStr = "FUNC_BODY";
	std::string destMunitStr = "FUNC_BODY";

	std::string srcBBStr(srcBB);
	std::string destBBStr(destBB);

	if(BBMappingUnitMap.find(srcBBStr)!=BBMappingUnitMap.end() && !srcBBStr.empty()){
		if(!BBMappingUnitMap[srcBBStr].empty()){
			srcMunitStr=BBMappingUnitMap[srcBBStr];
		}
	}

	if(BBMappingUnitMap.find(destBBStr)!=BBMappingUnitMap.end() & !destBBStr.empty()){
		if(!BBMappingUnitMap[destBBStr].empty()){
			destMunitStr=BBMappingUnitMap[destBBStr];
		}
	}

	if(srcMunitStr.compare(destMunitStr)!=0){
		fprintf(MUnitInvTraceFile,"%s,%s\n",srcMunitStr.c_str(),destMunitStr.c_str());
	}

	if(munitTransProfile.find(srcMunitStr)==munitTransProfile.end()){
		munitTransProfile[srcMunitStr][destMunitStr]=1;
	}
	else{
		if(munitTransProfile[srcMunitStr].find(destMunitStr)==munitTransProfile[srcMunitStr].end()){
			munitTransProfile[srcMunitStr][destMunitStr]=1;
		}
		else{
			munitTransProfile[srcMunitStr][destMunitStr]++;
		}
	}
}

void recordCondMunitTransition(const char* srcBB, const char* destBB1, const char* destBB2, int condition){
	std::string srcMunitStr = "FUNC_BODY";
	std::string destMunitStr = "FUNC_BODY";

	std::string srcBBStr(srcBB);
	std::string destBB1Str(destBB1);
	std::string destBB2Str(destBB2);

	std::string destBBStr;
	if(condition==1){
		destBBStr=destBB1Str;
	}
	else{
		destBBStr=destBB2Str;
	}

	if(BBMappingUnitMap.find(srcBBStr)!=BBMappingUnitMap.end() && !srcBBStr.empty()){
		if(!BBMappingUnitMap[srcBBStr].empty()){
			srcMunitStr=BBMappingUnitMap[srcBBStr];
		}
	}

	if(BBMappingUnitMap.find(destBBStr)!=BBMappingUnitMap.end() & !destBBStr.empty()){
		if(!BBMappingUnitMap[destBBStr].empty()){
			destMunitStr=BBMappingUnitMap[destBBStr];
		}
	}
	if(srcMunitStr.compare(destMunitStr)!=0){
		fprintf(MUnitInvTraceFile,"%s,%s\n",srcMunitStr.c_str(),destMunitStr.c_str());
	}

	if(srcMunitStr.compare(destMunitStr)==0){
		return;
	}

	if(munitTransProfile.find(srcMunitStr)==munitTransProfile.end()){
		munitTransProfile[srcMunitStr][destMunitStr]=1;
	}
	else{
		if(munitTransProfile[srcMunitStr].find(destMunitStr)==munitTransProfile[srcMunitStr].end()){
			munitTransProfile[srcMunitStr][destMunitStr]=1;
		}
		else{
			munitTransProfile[srcMunitStr][destMunitStr]++;
		}
	}
}

std::map<std::string,int> functionInsMap;
std::map<std::string,std::map<std::string,int>> BBInsMap;

void reportBBTrace(const char* FName, const char* BBName, int insCount){
	std::string fNameStr(FName);
	std::string BBNameStr(BBName);

	functionInsMap[fNameStr]+=insCount;
	BBInsMap[fNameStr][BBNameStr]+=insCount;
}

void sortandPrintStats(){
	std::ofstream statFile;
	statFile.open("statFile.log");

	statFile << "*****************************************\n";
	for(std::pair<std::string,int> pair : functionInsMap){
		statFile << pair.first << "," << pair.second << "\n";
	}
	statFile << "*****************************************\n";
}



//2018 triggered execution code

pathInfo trimPreHeader(std::string preheaderBB, pathInfo path){
	pathInfo result;
	assert(path.getPathCount() == 0);

	bool preheaderFound = false;

	for(std::string bb : path.getBBStrArry()){
		if(preheaderFound){
			result.insertBB(bb);
		}

		if(bb == preheaderBB){
			preheaderFound = true;
		}
	}

	if(!preheaderFound){
		return path;
	}
	else{
		assert(!result.getBBStrArry().empty());
		return result;
	}
}


void addPath2Profile(pathInfo currentPath, const char* loopName){
	std::string loopNameStr(loopName);
	std::string loopPreHeaderBB = loopPreHeaderBBMap[loopNameStr];
	currentPath = trimPreHeader(loopPreHeaderBB,currentPath);

	currentPath.incrementPathCount();
	assert(!currentPath.getBBStrArry().empty());
	if(pathsSoFar.find(currentPath.getBBStrArry())==pathsSoFar.end()){
		pathsSoFar[currentPath.getBBStrArry()] = pathsSoFar.size();
	}

	if(!pathTrace.empty()){
		pathInfo& lastPath = pathTrace[pathTrace.size()-1];
		if(pathsSoFar[lastPath.getBBStrArry()] == pathsSoFar[currentPath.getBBStrArry()]){
			lastPath.incrementPathCount();
		}
		else{
			pathTrace.push_back(currentPath);
//			currentPath.clear();
		}
	}
	else{
		pathTrace.push_back(currentPath);
//		currentPath.clear();
	}
}

void reportNewBBinPath(const char* bbName, const char* loopName){
	if(currentPath.insertBB(std::string(bbName))){

	}
	else{
		//coming here means its an backedge
		addPath2Profile(currentPath,loopName);
		currentPath.clear();
		assert(currentPath.insertBB(bbName));
	}
}

void reportLoopEnd(const char* loopName){
	addPath2Profile(currentPath,loopName);
	currentPath.clear();
	std::string loopNameStr(loopName);
	loopPathTrace[loopNameStr].push_back(pathTrace);
}


void LiveInReport(const char* varname, uint8_t* value, uint32_t size){
	std::string varname_str(varname);

	cout << "Call LiveInReport var name:" << varname_str << " size:" << (int)size <<"\n";

	for(int i=0; i<size; i++){
		data_morpher[varname_str].pre_data.push_back(value[i]);
		data_morpher[varname_str].post_data.push_back(value[i]);
		//cout << "var name:" << varname_str << ",value:" <<(int)value[i] <<"\n";
	}
}

void LiveInReportPtrTypeUsage(const char* varname,const char* varbaseaddr, uint32_t value, uint32_t size){
	std::string varname_str(varname);
	std::string varbaseaddr_str(varbaseaddr);

	uint32_t baseaddr_int =static_cast<uint32_t>(std::stoul(varbaseaddr_str));// *(uint32_t *)varbaseaddr;
#ifdef ARCHI_16BIT
	uint32_t addr= baseaddr_int+2*value;//4 is to get the byte address
#else
	uint32_t addr= baseaddr_int+4*value;//4 is to get the byte address
#endif
	cout << "Call LiveInReportPtrTypeUsage var name:" << varname_str << " local base address(str):"<< varbaseaddr_str<< " address offset:"<< value<< " local address:"<< addr<<" size:" << (int)size <<"\n";

//	for(int i=0; i<size; i++){
////		data_morpher[varname_str].pre_data.push_back(value[i]);
////		data_morpher[varname_str].post_data.push_back(value[i]);
//		cout << "var name:" << varname_str << ",value:" <<(int)value[i] <<"\n";
//	}
//	std::string varname_str(varname);
	uint8_t* value_ptr = (uint8_t*)&addr;
	for(int i=0; i<4; i++){
		data_morpher[varname_str].pre_data.push_back(value_ptr[i]);
		data_morpher[varname_str].post_data.push_back(value_ptr[i]);
	}
}

void LiveInReport2(const char* varname, uint32_t* value, uint32_t size){
	std::string varname_str(varname);

	cout << "Call LiveInReport2 var name:" << varname_str << " size:" << (int)size <<"\n";

	for(int i=0; i<size; i++){
//		data_morpher[varname_str].pre_data.push_back(value[i]);
//		data_morpher[varname_str].post_data.push_back(value[i]);
		//cout << "var name:" << varname_str << ",value:" <<(int)value[i] <<"\n";
	}
}

void LiveOutReport(const char* varname, uint8_t* value, uint32_t size){
	std::string varname_str(varname);
	cout << "Call LiveOutReport var name:" << varname_str << " size:" << (int)size <<"\n";
	for(int i=0; i<size; i++){
		data_morpher[varname_str].post_data[i] = value[i];
		//cout << "var name:" << varname_str << ",value:" <<(int)value[i] <<"\n";
	}
}


void LiveInReportIntermediateVar(const char* varname, uint32_t value){
	std::string varname_str(varname);
	uint8_t* value_ptr = (uint8_t*)&value;
	for(int i=0; i<4; i++){
		data_morpher[varname_str].pre_data.push_back(value_ptr[i]);
		data_morpher[varname_str].post_data.push_back(value_ptr[i]);
	}
}


void LiveOutReportIntermediateVar(const char* varname, uint32_t value){
	std::string varname_str(varname);
	cout << "Call LiveOutReportIntermediateVar var name:" << varname_str << "Value:" <<value << "\n";
	uint8_t* value_ptr = (uint8_t*)&value;
	for(int i=0; i<4; i++){
		data_morpher[varname_str].pre_data.push_back(0);
		data_morpher[varname_str].post_data.push_back(value_ptr[i]);
//		data_morpher[varname_str].post_data[i] = value_ptr[i];
	}
}








