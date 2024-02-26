// tvm target: c -keys=cpu 
#define TVM_EXPORTS

#include "tvm_header.h"
#include <math.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C"
#endif
int8_t data[1575];
int8_t kernel[768];
int8_t conv2d_nhwc[5640];
TVM_DLL int32_t conv_main(void* args, int32_t* arg_type_ids, int32_t num_args, void* out_ret_value, int32_t* out_ret_tcode, void* resource_handle) {
  void* arg_data = (((TVMValue*)args)[0].v_handle);
  int32_t arg_data_code = arg_type_ids[0];
  void* arg_kernel = (((TVMValue*)args)[1].v_handle);
  int32_t arg_kernel_code = arg_type_ids[1];
  void* arg_conv2d_nhwc = (((TVMValue*)args)[2].v_handle);
  int32_t arg_conv2d_nhwc_code = arg_type_ids[2];
  //int32_t* data = (((DLTensor*)arg_data)[0].data);
  void* arg_data_shape = (((DLTensor*)arg_data)[0].shape);
  void* arg_data_strides = (((DLTensor*)arg_data)[0].strides);
  int32_t dev_id = (((DLTensor*)arg_data)[0].device.device_id);
  //int32_t* kernel = (((DLTensor*)arg_kernel)[0].data);
  void* arg_kernel_shape = (((DLTensor*)arg_kernel)[0].shape);
  void* arg_kernel_strides = (((DLTensor*)arg_kernel)[0].strides);
  //conv2d_nhwc = (((DLTensor*)arg_conv2d_nhwc)[0].data);

  //int32_t* conv2d_nhwc = (((DLTensor*)arg_conv2d_nhwc)[0].data);
  //int conv2d_nhwc[11280]; 
  void* arg_conv2d_nhwc_shape = (((DLTensor*)arg_conv2d_nhwc)[0].shape);
  void* arg_conv2d_nhwc_strides = (((DLTensor*)arg_conv2d_nhwc)[0].strides);
  for (int32_t w_outer = 0; w_outer < 4; ++w_outer) {
    for (int32_t w_inner_h_fused_oc_fused_rh_fused_rw_outer_fused_init = 0; w_inner_h_fused_oc_fused_rh_fused_rw_outer_fused_init < 11280; ++w_inner_h_fused_oc_fused_rh_fused_rw_outer_fused_init) {
      ((int32_t*)conv2d_nhwc)[((((((w_inner_h_fused_oc_fused_rh_fused_rw_outer_fused_init % 240) / 48) * 1128) + (w_outer * 282)) + ((w_inner_h_fused_oc_fused_rh_fused_rw_outer_fused_init / 240) * 6)) + ((w_inner_h_fused_oc_fused_rh_fused_rw_outer_fused_init % 48) >> 3))] = (int32_t)0;
    }
    int32_t tmp_cse_var_6 = 0, tmp_cse_var_5 = 0;
    int32_t c240 = 0, i_240 = 0;
    int32_t c48 = 0, i_48 = 0;
    //int32_t w_outer = 0;
    int32_t w_inner_h_fused_oc_fused_rh_fused_rw_outer_fused = 0;
    // map this loop to cgra
    for (; w_inner_h_fused_oc_fused_rh_fused_rw_outer_fused < 11280; ++w_inner_h_fused_oc_fused_rh_fused_rw_outer_fused) {
      #ifdef CGRA_COMPILER
      please_map_me();
      #endif
      int32_t cse_var_5 = i_240;
      int32_t cse_var_4 = i_48;

      int32_t cse_var_3 = (tmp_cse_var_6 * 16);
      int32_t cse_var_2 = ((((cse_var_4 * 315) + (w_outer * 47)) + ((w_inner_h_fused_oc_fused_rh_fused_rw_outer_fused & 7) * 16)) + cse_var_5);
      int32_t cse_var_1 = ((((cse_var_4 * 1128) + (w_outer * 282)) + (cse_var_5 * 6)) + (tmp_cse_var_6 >> 3));
      //int32_t sum = cse_var_3 + cse_var_2 + cse_var_1 + w_inner_h_fused_oc_fused_rh_fused_rw_outer_fused;
      //conv2d_nhwc[w_inner_h_fused_oc_fused_rh_fused_rw_outer_fused] =conv2d_nhwc[w_inner_h_fused_oc_fused_rh_fused_rw_outer_fused] + w_inner_h_fused_oc_fused_rh_fused_rw_outer_fused;
      conv2d_nhwc[cse_var_1] = conv2d_nhwc[cse_var_1] + data[cse_var_2] * kernel[cse_var_3];
      conv2d_nhwc[cse_var_1] = conv2d_nhwc[cse_var_1] + data[(cse_var_2 + 1)] * kernel[(cse_var_3 + 1)];
      conv2d_nhwc[cse_var_1] = conv2d_nhwc[cse_var_1] + data[(cse_var_2 + 2)] * kernel[(cse_var_3 + 2)];
      conv2d_nhwc[cse_var_1] = conv2d_nhwc[cse_var_1] + data[(cse_var_2 + 3)] * kernel[(cse_var_3 + 3)];
      conv2d_nhwc[cse_var_1] = conv2d_nhwc[cse_var_1] + data[(cse_var_2 + 4)] * kernel[(cse_var_3 + 4)];
      conv2d_nhwc[cse_var_1] = conv2d_nhwc[cse_var_1] + data[(cse_var_2 + 5)] * kernel[(cse_var_3 + 5)];
      conv2d_nhwc[cse_var_1] = conv2d_nhwc[cse_var_1] + data[(cse_var_2 + 6)] * kernel[(cse_var_3 + 6)];
      conv2d_nhwc[cse_var_1] = conv2d_nhwc[cse_var_1] + data[(cse_var_2 + 7)] * kernel[(cse_var_3 + 7)];
      conv2d_nhwc[cse_var_1] = conv2d_nhwc[cse_var_1] + data[(cse_var_2 + 8)] * kernel[(cse_var_3 + 8)];
      conv2d_nhwc[cse_var_1] = conv2d_nhwc[cse_var_1] + data[(cse_var_2 + 9)] * kernel[(cse_var_3 + 9)];
      conv2d_nhwc[cse_var_1] = conv2d_nhwc[cse_var_1] + data[(cse_var_2 + 10)] * kernel[(cse_var_3 + 10)];
      conv2d_nhwc[cse_var_1] = conv2d_nhwc[cse_var_1] + data[(cse_var_2 + 11)] * kernel[(cse_var_3 + 11)];
      conv2d_nhwc[cse_var_1] = conv2d_nhwc[cse_var_1] + data[(cse_var_2 + 12)] * kernel[(cse_var_3 + 12)];
      conv2d_nhwc[cse_var_1] = conv2d_nhwc[cse_var_1] + data[(cse_var_2 + 13)] * kernel[(cse_var_3 + 13)];
      conv2d_nhwc[cse_var_1] = conv2d_nhwc[cse_var_1] + data[(cse_var_2 + 14)] * kernel[(cse_var_3 + 14)];
      conv2d_nhwc[cse_var_1] = conv2d_nhwc[cse_var_1] + data[(cse_var_2 + 15)] * kernel[(cse_var_3 + 15)];

      if (c240 + 1==240) {
        i_240++;
        c240 = 0;
      } else {
        c240++;
      }

      if (tmp_cse_var_6+1 == 48) {
        tmp_cse_var_6 = 0;
      } else {
        tmp_cse_var_6++;
      }

      if (c48+1 == 48) {
        i_48++;
        c48=0;
      } else {
        c48++;
      }

      if (tmp_cse_var_5+1 == 240) {
        tmp_cse_var_5 = 0;
        i_48=0;
        c48=0;
      } else {
        tmp_cse_var_5++;
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
    
    int32_t shape_data[3] = {5,315,1};
    //int32_t data[1575];
    DLDataType type_data;
    type_data.code = (uint32_t)kDLInt;
    type_data.bits = 8;
    type_data.lanes = 1;
    read_int8_data("data.txt", data, 1575);
    DLTensor dlt_data;
    create_dl_tensor(&dlt_data, data, 3, type_data, shape_data);
    TVMValue v_data;
    v_data.v_handle = &dlt_data;
    
    // kernel
    
    int32_t shape_kernel[4] = {6,1,128,1};
    //int32_t kernel[768];
    DLDataType type_kernel;
    type_kernel.code = (uint32_t)kDLInt;
    type_kernel.bits = 8;
    type_kernel.lanes = 1;
    read_int8_data("kernel.txt", kernel, 768);
    DLTensor dlt_kernel;
    create_dl_tensor(&dlt_kernel, kernel, 4, type_kernel, shape_kernel);
    TVMValue v_kernel;
    v_kernel.v_handle = &dlt_kernel;
    
    // output
    
    int32_t shape_output[3] = {5,188,6};
    //int32_t output[5640];
    DLDataType type_output;
    type_output.code = (uint32_t)kDLInt;
    type_output.bits = 8;
    type_output.lanes = 1;
    
    DLTensor dlt_output;
    create_dl_tensor(&dlt_output, conv2d_nhwc, 3, type_output, shape_output);
    TVMValue v_output;
    v_output.v_handle = &dlt_output;
    

    TVMValue args[3] = {v_data, v_kernel, v_output};
    int32_t fake[] = {0,0,0};
    conv_main(args, fake, 3, NULL, NULL, NULL);
    
    // write out tensor
    if (write_int8_data("output.txt", conv2d_nhwc, 5640) != 0) {
      printf("write data failed");
      return -1;
    }
    
    return 0;
}
        
