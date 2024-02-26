
#define TVM_EXPORTS

#include "tvm_header.h"
#include <math.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C"
#endif

int8_t data[5640];
int8_t kernel[60];
int8_t dwconv1[2256];// tvm target: c -keys=cpu 

TVM_DLL int32_t conv_main(void* args, int32_t* arg_type_ids, int32_t num_args, void* out_ret_value, int32_t* out_ret_tcode, void* resource_handle) {
  void* arg_data = (((TVMValue*)args)[0].v_handle);
  int32_t arg_data_code = arg_type_ids[0];
  void* arg_kernel = (((TVMValue*)args)[1].v_handle);
  int32_t arg_kernel_code = arg_type_ids[1];
  void* arg_dwconv1 = (((TVMValue*)args)[2].v_handle);
  int32_t arg_dwconv1_code = arg_type_ids[2];
  // void* data = (((DLTensor*)arg_data)[0].data);
  void* arg_data_shape = (((DLTensor*)arg_data)[0].shape);
  void* arg_data_strides = (((DLTensor*)arg_data)[0].strides);
  int32_t dev_id = (((DLTensor*)arg_data)[0].device.device_id);
  // void* kernel = (((DLTensor*)arg_kernel)[0].data);
  void* arg_kernel_shape = (((DLTensor*)arg_kernel)[0].shape);
  void* arg_kernel_strides = (((DLTensor*)arg_kernel)[0].strides);
  // void* dwconv1 = (((DLTensor*)arg_dwconv1)[0].data);
  void* arg_dwconv1_shape = (((DLTensor*)arg_dwconv1)[0].shape);
  void* arg_dwconv1_strides = (((DLTensor*)arg_dwconv1)[0].strides);
  if (!(arg_data_strides == NULL)) {
  }
  if (!(arg_kernel_strides == NULL)) {
  }
  if (!(arg_dwconv1_strides == NULL)) {
  }
  for (int32_t ow_outer = 0; ow_outer < 4; ++ow_outer) {

    int32_t mod_12 = 0;
    for (int32_t ow_inner_c_fused_m_fused = 0; ow_inner_c_fused_m_fused < 564; ++ow_inner_c_fused_m_fused) {
      #ifdef CGRA_COMPILER  
      please_map_me();
      #endif
      int32_t cse_var_3 = mod_12;
      //  (ow_inner_c_fused_m_fused % 12);
      int32_t cse_var_2 = ((ow_outer * 564) + ow_inner_c_fused_m_fused);
      int32_t cse_var_1 = ((ow_outer * 282) + (ow_inner_c_fused_m_fused >> 1));
      ((int8_t*)dwconv1)[cse_var_2] = (int8_t)0;
      ((int8_t*)dwconv1)[cse_var_2] = (((int8_t*)dwconv1)[cse_var_2] + (((int8_t*)data)[cse_var_1] * ((int8_t*)kernel)[cse_var_3]));
      ((int8_t*)dwconv1)[cse_var_2] = (((int8_t*)dwconv1)[cse_var_2] + (((int8_t*)data)[(cse_var_1 + 1128)] * ((int8_t*)kernel)[(cse_var_3 + 12)]));
      ((int8_t*)dwconv1)[cse_var_2] = (((int8_t*)dwconv1)[cse_var_2] + (((int8_t*)data)[(cse_var_1 + 2256)] * ((int8_t*)kernel)[(cse_var_3 + 24)]));
      ((int8_t*)dwconv1)[cse_var_2] = (((int8_t*)dwconv1)[cse_var_2] + (((int8_t*)data)[(cse_var_1 + 3384)] * ((int8_t*)kernel)[(cse_var_3 + 36)]));
      ((int8_t*)dwconv1)[cse_var_2] = (((int8_t*)dwconv1)[cse_var_2] + (((int8_t*)data)[(cse_var_1 + 4512)] * ((int8_t*)kernel)[(cse_var_3 + 48)]));
      if (mod_12 + 1 == 12) {
        mod_12 = 0;
      } else {
        mod_12++;
      }
    }
  }
  return 0;
}

// CodegenC: NOTE: Auto-generated entry function
#ifdef __cplusplus
extern "C"
#endif
TVM_DLL int32_t __tvm_conv_main__(void* args, int* arg_type_ids, int num_args, void* out_ret_value, int* out_ret_tcode, void* resource_handle) {
  return conv_main(args, arg_type_ids, num_args, out_ret_value, out_ret_tcode, resource_handle);
}

int main() {
    // data
    
    int64_t shape_data[3] = {5,188,6};
    //int8_t data[5640];
    DLDataType type_data;
    type_data.code = (uint8_t)kDLInt;
    type_data.bits = 8;
    type_data.lanes = 1;
    read_int8_data("dwconv1_data.txt", data, 5640);
    DLTensor dlt_data;
    create_dl_tensor(&dlt_data, data, 3, type_data, shape_data);
    TVMValue v_data;
    v_data.v_handle = &dlt_data;
    
    // kernel
    
    int64_t shape_kernel[4] = {5,1,6,2};
    //int8_t kernel[60];
    DLDataType type_kernel;
    type_kernel.code = (uint8_t)kDLInt;
    type_kernel.bits = 8;
    type_kernel.lanes = 1;
    read_int8_data("dwconv1_kernel.txt", kernel, 60);
    DLTensor dlt_kernel;
    create_dl_tensor(&dlt_kernel, kernel, 4, type_kernel, shape_kernel);
    TVMValue v_kernel;
    v_kernel.v_handle = &dlt_kernel;
    
    // dwconv1
    
    int64_t shape_dwconv1[4] = {1,188,6,2};
    //int8_t dwconv1[2256];
    DLDataType type_dwconv1;
    type_dwconv1.code = (uint8_t)kDLInt;
    type_dwconv1.bits = 8;
    type_dwconv1.lanes = 1;
    
    DLTensor dlt_dwconv1;
    create_dl_tensor(&dlt_dwconv1, dwconv1, 4, type_dwconv1, shape_dwconv1);
    TVMValue v_dwconv1;
    v_dwconv1.v_handle = &dlt_dwconv1;
    

    TVMValue args[3] = {v_data, v_kernel, v_dwconv1};
    int32_t fake[] = {0,0,0};
    conv_main(args, fake, 3, NULL, NULL, NULL);
    
    // write out tensor
    if (write_int8_data("dwconv1_output.txt", dwconv1, 2256) != 0) {
      printf("write data failed");
      return -1;
    }
    
    
    return 0;
}
        