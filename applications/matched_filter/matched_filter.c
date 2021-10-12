    // matched filter (correlation filter)    
    // H[] holds the samples to match, 
    // M = H.length (number of samples in H)
    // ip[] holds ADC sampled input data (length > nPts + M )
    // op[] is matched filter output buffer
    // nPts is the length of the required output data 
int H[1000];
int ip[1000];
int op[1000];
#define nPts 100
#define M 100
    void corrFilter_1()
    {  
      int sum = 0;
      for (int j=0; j < nPts; j++)
      {
        sum = 0;
        for (int i = 0; i < M; i=i+50)
        {    
          #ifdef CGRA_COMPILER
           please_map_me();
           #endif
          sum += (H[i]) * (ip[j+i]);
          sum += (H[i+1]) * (ip[j+i+1]);
          sum += (H[i+2]) * (ip[j+i+2]);
          sum += (H[i+3]) * (ip[j+i+3]);
          sum += (H[i+4]) * (ip[j+i+4]);
          sum += (H[i+5]) * (ip[j+i+5]);
          sum += (H[i+6]) * (ip[j+i+6]);
          sum += (H[i+7]) * (ip[j+i+7]);
          sum += (H[i+8]) * (ip[j+i+8]);
          sum += (H[i+9]) * (ip[j+i+9]);
          sum += (H[i+10]) * (ip[j+i+10]);
          sum += (H[i+11]) * (ip[j+i+11]);
          sum += (H[i+12]) * (ip[j+i+12]);
          sum += (H[i+13]) * (ip[j+i+13]);
          sum += (H[i+14]) * (ip[j+i+14]);
          sum += (H[i+15]) * (ip[j+i+15]);
          sum += (H[i+16]) * (ip[j+i+16]);
          sum += (H[i+17]) * (ip[j+i+17]);
          sum += (H[i+18]) * (ip[j+i+18]);
          sum += (H[i+19]) * (ip[j+i+19]);
          sum += (H[i+20]) * (ip[j+i+20]);
          sum += (H[i+21]) * (ip[j+i+21]);
          sum += (H[i+22]) * (ip[j+i+22]);
          sum += (H[i+23]) * (ip[j+i+23]);
          sum += (H[i+24]) * (ip[j+i+24]);
          sum += (H[i+25]) * (ip[j+i+25]);
          sum += (H[i+26]) * (ip[j+i+26]);
          sum += (H[i+27]) * (ip[j+i+27]);
          sum += (H[i+28]) * (ip[j+i+28]);
          sum += (H[i+29]) * (ip[j+i+29]);
          sum += (H[i+30]) * (ip[j+i+30]);
          sum += (H[i+31]) * (ip[j+i+31]);
          sum += (H[i+32]) * (ip[j+i+32]);
          sum += (H[i+33]) * (ip[j+i+33]);
          sum += (H[i+34]) * (ip[j+i+34]);
          sum += (H[i+35]) * (ip[j+i+35]);
          sum += (H[i+36]) * (ip[j+i+36]);
          sum += (H[i+37]) * (ip[j+i+37]);
          sum += (H[i+38]) * (ip[j+i+38]);
          sum += (H[i+39]) * (ip[j+i+39]);
          sum += (H[i+40]) * (ip[j+i+40]);
          sum += (H[i+41]) * (ip[j+i+41]);
          sum += (H[i+42]) * (ip[j+i+42]);
          sum += (H[i+43]) * (ip[j+i+43]);
          sum += (H[i+44]) * (ip[j+i+44]);
          sum += (H[i+45]) * (ip[j+i+45]);
          sum += (H[i+46]) * (ip[j+i+46]);
          sum += (H[i+47]) * (ip[j+i+47]);
          sum += (H[i+48]) * (ip[j+i+48]);
          sum += (H[i+49]) * (ip[j+i+49]);
        }
        op[j] = sum;      
      }
    }

// void corrFilter_2(int *H, int M, int *ip, int *op, int nPts)
//     {  
//       int sum = 0;
//       for (int j=0; j < nPts; j++)
//       {
//         sum = 0;
//         for (int i = 0; i < M; i++)
//         {    
//           #ifdef CGRA_COMPILER
//            please_map_me();
//            #endif
//           sum += (H[i]) * (ip[j+i]);
//         }
//         op[j] = sum;      
//       }

//       int j = 0; 
//       int i = 0;
//       for (int ij = 0; ij < nPts*M ; ij++){

//           #ifdef CGRA_COMPILER
//            please_map_me();
//            #endif
//           sum += (H[i]) * (ip[j+i]);

//         if(++i == M){
//           op[j] = sum;  
//           i=0;
//           ++j;
//         }

//       }
//     }
