#include <stdlib.h>
#include <stdio.h>
//#include <iostream>

// IF targeted at arm include DUMP
#ifdef __arm__
#define DUMP
#endif

#ifdef DUMP
#include <m5op.h>
#endif

#ifndef DUMP
#include <pthread.h>
#endif

#define OUTPUT

#define TAG 0
//using namespace std;

//#define SIZE 1025
#define SIZE 513
#define NUM 1

//int *S;
int S[SIZE];
//int *T;
int T[SIZE];
int DTW[SIZE+1][SIZE+1];

int minimum(int a, int b, int c)
{
	int min = 65535;
	if(a<b)
	{
		min = a;	
	}else{
		min = b;	
	}
	if(min>c)
	{
		min = c;
	}
	return min;
}

int minimum2(int a, int b)
{
	int min = 65535;
	if(a<b)
	{
		min = a;	
	}else{
		min = b;	
	}
	return min;
}

int maximum(int a, int b, int c)
{
	int max = 0;
	if(a<b)
	{
		max = b;	
	}else{
		max = a;	
	}
	if(max<c)
	{
		max = c;
	}
	return max;
}

int maximum2(int a, int b)
{
	int max = 0;
	if(a<b)
	{
		max = b;	
	}else{
		max = a;	
	}
	return max;
}

void output()
{
    int i;
    int j;
	for(i=0;i<SIZE+1;++i)
	{
		for(j=0;j<SIZE+1;++j)
		{
			//if(j==SIZE)
				//cout<<DTW[i][j]<<"\t";
		}
		//cout<<endl;
	}
}

int absolute(int v)
{
    if(v<0)
    {
        v = -v;
    }
    return v;
}

void get_data(int len, int **sa, int **ta)
{
    char filename_std[256] = {"input_dtw/input_std"};
    char filename_smp[256] = {"input_dtw/input_smp"};
    int base_fname_length = 18;

    //    char rank_str[20];
    //    sprintf(rank_str, "%d", rank);

    //    for(i = 0; rank_str[i] != '\0'; i++)
    //        filename[base_fname_length + i] = rank_str[i];
    filename_std[base_fname_length + 1] = '\0';

    FILE *fp = fopen(filename_std, "r");
    if(fp == NULL)
    {
        printf("\ncannot open file \"%s\".\n", filename_std);
        exit(1);
    }

    int *data = (int*)malloc(sizeof(int) * len);
    int length = len;
    int *p = data;
    float t = 0;
    float ign = 0;
    while (len--) {
        fscanf(fp, "%f,%f", &t, &ign);
        *p = (int)t;
        //        printf("core %d std value read = %d\n", rank, *p);
        //scanf("%f", p);
        //scanf("%" DATA_FMT, p);
        p++;
    }
    fclose(fp);
    *sa = data;

    fp = fopen(filename_smp, "r");
    if(fp == NULL)
    {
        printf("\ncannot open file \"%s\".\n", filename_smp);
        exit(1);
    }
    len = length;
    data = (int*)malloc(sizeof(int) * len);
    p = data;
    t = 0;
    ign = 0;
    while (len--) {
        fscanf(fp, "%f,%f", &t, &ign);
        *p = (int)t;
        //scanf("%f", p);
        //scanf("%" DATA_FMT, p);
        //        printf("core %d smp value read = %d\n", rank, *p);
        p++;
    }
    fclose(fp);
    *ta = data;
}

void initialize(int hasW)
{
    int temp_s[SIZE];// = {1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7};
    int i;
	for(i=0;i<SIZE;++i)
	{
		temp_s[i] = i+1;
	}
	//cout <<"S:\t";
	for(i=0;i<SIZE;++i)
	{
		S[i] = temp_s[i];	
		//cout << temp_s[i] << "\t";
	}
	int temp_t[SIZE];// = {2,3,4,5,6,7,8,9,11,3,4,5,6,7,8,9,10};
	for(i=0; i<SIZE; ++i)
	{
		temp_t[i] = i+3;
	}
	//cout <<"\nT:\t";
    int j;
	for(j=0;j<SIZE;++j)
	{
		T[j] = temp_t[j];	
		//cout << temp_t[j] << "\t";
	}

//    get_data(SIZE, &S, &T);

    
	for(i=1;i<SIZE+1;++i)
	{
        for(j=1;j<SIZE+1;++j)
        {
		    DTW[i][j] = -1;
        }
	}

//	cout << "\n-------------------------------------------------------" << endl;
	for(i=1;i<SIZE+1;++i)
	{
		DTW[i][0] = 65535;
		DTW[0][i] = 65535;
	}

	for(j=1;j<SIZE+1;++j)
	{
		DTW[j][1] = 65535;
		DTW[1][j] = absolute(S[0] - T[j-1]);
	}
	
//	output();

	if(hasW == 1)
	{
		for(i=0;i<SIZE+1;++i)
		{
			for(j=0;j<SIZE+1;++j)
			{	
				DTW[i][j] = 65535;
			}
		}
	}
	DTW[0][0] = 0;
}

int cost[(SIZE-1)/NUM];
int j;

void DTW_runloop(int x, int i){
         cost[x] = absolute(S[i]-T[j+x]); 

         	int min = 65535;
	        if(DTW[i-1][j+x]<DTW[i][j+x])
	        {
		        min = DTW[i-1][j+x];	
	        }else{
		        min = DTW[i][j+x];	
	        }
	        if(min>DTW[i][j-1+x])
	        {
		        min = DTW[i][j-1+x];
	        }
         //DTW[i+1][j+1+x] = cost[x] + minimum(DTW[i-1][j+x], DTW[i][j+x], DTW[i][j-1+x]);
        DTW[i+1][j+1+x] = cost[x] + min;
}

void DTW_run(int id)
{

    j = (SIZE-1)/NUM*id + 1;
    int i;
    for(i = 1; i < SIZE; ++i)
    {
//        for(int x=0; x<(SIZE-1)/NUM; ++x)
//        {
//        }
        
        int x;
        for(x=0; x<(SIZE-1)/NUM; ++x)
        {
            DTW_runloop(x,i);
        }
    }
}


int main(int argc, char **argv)
{

	//cout<<"================= next, DTW variation=============="<<endl;

//for(int iter = 0; iter<100; ++iter)
//{
    initialize(0);

#ifdef DUMP
	m5_dump_stats(0, 0);
	m5_reset_stats(0, 0);
#endif

	DTW_run(0);
//}

#ifdef DUMP
	m5_dump_stats(0, 0);
	m5_reset_stats(0, 0);
#endif

#ifdef OUTPUT
	output();
#endif
	return 0;
}
