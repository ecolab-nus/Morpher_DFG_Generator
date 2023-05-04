//function that extracts the value from the matrix to be flattend while also including padding
int8_t im2col_get_pixel(int8_t *im, int height, int width,
                        int row, int col, int channel, int pad_l, int pad_r, int pad_u, int pad_d, int8_t pad_value)
{
    //subtract padding size left and upper
    row -= pad_l;
    col -= pad_u;
    //if position is in padding regime, return padding value else return value from matrix
    if (row < 0 || col < 0 ||
        row >= height || col >= width) return pad_value;
    return im[col + width*(row + height*channel)];
}
//From Berkeley Vision's Caffe!
//https://github.com/BVLC/caffe/blob/master/LICENSE


void im2col(int8_t* data_im,
  int channels,  int height,  int width, int ksize,  int stride,
  int8_t* data_col, int pad_l, int pad_r, int pad_u, int pad_d, int8_t pad_value) 
{
    int c,h,w;
    int height_col = (height +pad_d + pad_u - ksize) / stride + 1;
    //printf("height_col: %d \n", height_col);
    int width_col = (width +pad_l + pad_r - ksize) / stride + 1;
    //printf("width_col: %d \n", width_col);
    int channels_col = channels * ksize * ksize;
    //printf("channels_col: %d \n", channels_col);
    for (c = 0; c < channels_col; ++c) {
        int w_offset = c % ksize;
        //printf("w_offset: %d \n", w_offset);//0,1,2
        int h_offset = ((int) (c / ksize)) % ksize;
        //printf("h_offset: %d \n", h_offset);//0,1,2
        int c_im = c / ksize / ksize;//0-7
        //printf("c_im: %d \n", c_im);
        //printf("c: %d \n", c);
        //printf("c*height_col*width_col: %d \n", c*height_col*width_col);
        for (h = 0; h < height_col; ++h) {
            for (w = 0; w < width_col; ++w) {
                int im_row = h_offset + h * stride;
                int im_col = w_offset + w * stride;
                int col_index = (c*height_col*width_col)+ w + h*width_col;
                //printf("col_index: %d \n", col_index);
                data_col[col_index] = im2col_get_pixel(data_im, height, width,
                        im_row, im_col, c_im, pad_l, pad_r, pad_u, pad_d, pad_value);
            }
        }
    }
    //printf("IM2COL Done\n");
    //printf("data_col[1000]: %d \n", data_col[1000]);
    //printf("data_col[2000]: %d \n", data_col[2000]);
}


