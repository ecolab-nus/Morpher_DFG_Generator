void depthwise_conv_layer(int8_t* matrix,int8_t* weights, const int inp_channels, const int outp_channels, const int weight_dim, const int input_dim,
    int pad_l, int pad_r, int pad_u, int pad_d, int8_t pad_value, int32_t* bias, int64_t* multiply, int64_t* add, int64_t* shift, int stride)
{
    int8_t INPUT_MATRIX[input_dim* input_dim* inp_channels* weight_dim* weight_dim];
    int32_t OUTPUT_MATRIX[input_dim* input_dim* inp_channels];

    im2col(matrix, inp_channels, input_dim,input_dim, weight_dim, stride,INPUT_MATRIX, pad_l,pad_r,pad_u,pad_d,pad_value);
    const int input_dim_new = input_dim/stride;
    printf("input_dim_new: %d \n",input_dim_new);
    conv_layer(INPUT_MATRIX, weights,outp_channels, weight_dim*weight_dim, input_dim_new*input_dim_new, OUTPUT_MATRIX, inp_channels);
    quantize_conv_layer(OUTPUT_MATRIX,weights,outp_channels, weight_dim*weight_dim, input_dim_new*input_dim_new,128); 
    add_bias(OUTPUT_MATRIX, bias, input_dim_new*input_dim_new, outp_channels);
    requantize_conv(OUTPUT_MATRIX,matrix, input_dim_new*input_dim_new, outp_channels, multiply, add, shift, 0);

}

void input_conv_layer(int8_t* output,int8_t* input,int8_t* weights, const int inp_channels, const int outp_channels, const int weight_dim, const int input_dim,
    int pad_l, int pad_r, int pad_u, int pad_d, int8_t pad_value, int32_t* bias, int64_t* multiply, int64_t* add, int64_t* shift, int stride)
{
    int8_t INPUT_MATRIX[48*48*9];
    int32_t OUTPUT_MATRIX[48*48*8];
    im2col(input, inp_channels, input_dim,input_dim, weight_dim, stride,INPUT_MATRIX, pad_l,pad_r,pad_u,pad_d,pad_value);
    const int input_dim_new = input_dim/stride;
    printf("input_dim_new: %d \n",input_dim_new);
    conv_layer(INPUT_MATRIX, weights,outp_channels, weight_dim*weight_dim, input_dim_new*input_dim_new, OUTPUT_MATRIX, inp_channels);
    quantize_conv_layer(OUTPUT_MATRIX,weights,outp_channels, weight_dim*weight_dim, input_dim_new*input_dim_new,2); 
    add_bias(OUTPUT_MATRIX, bias, input_dim_new*input_dim_new, outp_channels);
    requantize_conv(OUTPUT_MATRIX,output, input_dim_new*input_dim_new, outp_channels, multiply, add, shift, 0);


}

void complete_pointwise_conv_layer(int8_t* matrix,int8_t* weights, const int inp_channels, const int outp_channels , const int input_dim, 
    int32_t* bias, int64_t* multiply, int64_t* add, int64_t* shift)
    {
        int32_t OUTPUT_MATRIX[input_dim*input_dim*outp_channels];
        pointwise_conv_layer(matrix, weights,inp_channels,outp_channels, input_dim*input_dim, OUTPUT_MATRIX);
        quantize_conv_layer(OUTPUT_MATRIX,weights,outp_channels, inp_channels, input_dim*input_dim,128); 
        add_bias(OUTPUT_MATRIX, bias,input_dim*input_dim, outp_channels);
        requantize_conv(OUTPUT_MATRIX,matrix, input_dim*input_dim, outp_channels, multiply, add, shift, 0);
    }

void layer27(int8_t* matrix,int8_t* weights, const int inp_channels, const int outp_channels , const int input_dim, 
    int32_t* bias, int64_t* multiply, int64_t* add, int64_t* shift)
    {
        int32_t OUTPUT_MATRIX[input_dim*input_dim*outp_channels];
        int32_t OUTPUT_MATRIX_SMALL[input_dim*input_dim*(outp_channels/4)];
        pointwise_conv_layer(matrix,WEIGHT_MATRIX27_1,inp_channels,outp_channels/4, input_dim*input_dim, OUTPUT_MATRIX_SMALL);
        for(int i=0; i<input_dim*input_dim; i++)
        {
            for(int j=0; j< (outp_channels/4); j++)
            {
                OUTPUT_MATRIX[j*input_dim*input_dim+i] = OUTPUT_MATRIX_SMALL[j*input_dim*input_dim+i];
            } 
        }
        pointwise_conv_layer(matrix,WEIGHT_MATRIX27_2,inp_channels,outp_channels/4, input_dim*input_dim, OUTPUT_MATRIX_SMALL);
        for(int i=0; i<input_dim*input_dim; i++)
        {
            for(int j=0; j< (outp_channels/4); j++)
            {
                OUTPUT_MATRIX[input_dim*input_dim*(outp_channels/4)+j*input_dim*input_dim+i] = OUTPUT_MATRIX_SMALL[j*input_dim*input_dim+i];
            } 
        }
        pointwise_conv_layer(matrix,WEIGHT_MATRIX27_3,inp_channels,outp_channels/4, input_dim*input_dim, OUTPUT_MATRIX_SMALL);
        for(int i=0; i<input_dim*input_dim; i++)
        {
            for(int j=0; j< (outp_channels/4); j++)
            {
                OUTPUT_MATRIX[2*input_dim*input_dim*(outp_channels/4)+j*input_dim*input_dim+i] = OUTPUT_MATRIX_SMALL[j*input_dim*input_dim+i];
            } 
        }
        pointwise_conv_layer(matrix,WEIGHT_MATRIX27_4,inp_channels,outp_channels/4, input_dim*input_dim, OUTPUT_MATRIX_SMALL);
        for(int i=0; i<input_dim*input_dim; i++)
        {
            for(int j=0; j< (outp_channels/4); j++)
            {
                OUTPUT_MATRIX[3*input_dim*input_dim*(outp_channels/4)+j*input_dim*input_dim+i] = OUTPUT_MATRIX_SMALL[j*input_dim*input_dim+i];
            } 
        }
        quantize_conv_layer(OUTPUT_MATRIX,weights,outp_channels, inp_channels, input_dim*input_dim,128); 
        add_bias(OUTPUT_MATRIX, bias,input_dim*input_dim, outp_channels);
        requantize_conv(OUTPUT_MATRIX,matrix, input_dim*input_dim, outp_channels, multiply, add, shift, 0);
    }

void final_conv_layer(int8_t* matrix,int8_t* weights, const int inp_channels, const int outp_channels , const int input_dim, 
    int32_t* bias, int64_t* multiply, int64_t* add, int64_t* shift)
    {
        int32_t OUTPUT_MATRIX[input_dim*input_dim*outp_channels];
        pointwise_conv_layer(matrix, weights,inp_channels,outp_channels, input_dim*input_dim, OUTPUT_MATRIX);
        quantize_conv_layer(OUTPUT_MATRIX,weights,outp_channels, inp_channels, input_dim*input_dim,128); 
        add_bias(OUTPUT_MATRIX, bias,input_dim*input_dim, outp_channels);
        requantize_conv(OUTPUT_MATRIX,matrix, input_dim*input_dim, outp_channels, multiply, add, shift, 1);
    }