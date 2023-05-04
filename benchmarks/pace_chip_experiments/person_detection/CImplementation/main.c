
#include "bias.h"
#include "layer_functions.h"
#include "image.h"
#include "weights.h"
#include "im2col.h"
#include "quant_params.h"
#include "combinedLayers.h"


//#include <inttypes.h>

int main() {
    //while(1){
        //matrices used to store the results of a layer after the convolution
        int8_t OUTPUT_MATRIX_int8[48*48*16];//48x48x16
        //matrices used to store the flattend input after the im2col function (only for depthwise Conv Layers)

        input_conv_layer(OUTPUT_MATRIX_int8, IMAGE_PERSON, WEIGHT_MATRIX1, 1, CHANNELS1, 3, IMAGE_DIM, 0, 1, 0, 1, -2, bias1, multiply1, add1, shift1, 2);
        printf("Layer 1 Done\n");

        depthwise_conv_layer(OUTPUT_MATRIX_int8, WEIGHT_MATRIX2, CHANNELS1, CHANNELS2, 3, 48, 1, 1, 1, 1, -128, bias2, multiply2, add2, shift2, 1);
        printf("Layer 2 Done\n");

        complete_pointwise_conv_layer(OUTPUT_MATRIX_int8,WEIGHT_MATRIX3, CHANNELS_IN3, CHANNELS_OUT3, 48, bias3, multiply3, add3,  shift3);
        printf("Layer 3 Done\n");

        depthwise_conv_layer(OUTPUT_MATRIX_int8, WEIGHT_MATRIX4, CHANNELS_OUT3, CHANNELS4, 3, 48, 0, 1, 0, 1, -128, bias4, multiply4, add4, shift4, 2);
        printf("Layer 4 Done\n");

        complete_pointwise_conv_layer(OUTPUT_MATRIX_int8,WEIGHT_MATRIX5, CHANNELS_IN5, CHANNELS_OUT5, 24, bias5, multiply5, add5,  shift5);
        printf("Layer 5 Done\n");

        depthwise_conv_layer(OUTPUT_MATRIX_int8, WEIGHT_MATRIX6, CHANNELS_OUT5, CHANNELS6, 3, 24, 1, 1, 1, 1, -128, bias6, multiply6, add6, shift6, 1);
        printf("Layer 6 Done\n");

        complete_pointwise_conv_layer(OUTPUT_MATRIX_int8,WEIGHT_MATRIX7, CHANNELS_IN7, CHANNELS_OUT7, 24, bias7, multiply7, add7,  shift7);
        printf("Layer 7 Done\n");

        depthwise_conv_layer(OUTPUT_MATRIX_int8, WEIGHT_MATRIX8, CHANNELS_OUT7, CHANNELS8, 3, 24, 0, 1, 0, 1, -128, bias8, multiply8, add8, shift8, 2);
        printf("Layer 8 Done\n");

        complete_pointwise_conv_layer(OUTPUT_MATRIX_int8,WEIGHT_MATRIX9, CHANNELS_IN9, CHANNELS_OUT9, 12, bias9, multiply9, add9,  shift9);
        printf("Layer 9 Done\n");

        depthwise_conv_layer(OUTPUT_MATRIX_int8, WEIGHT_MATRIX10, CHANNELS_OUT9, CHANNELS10, 3, 12, 1, 1, 1, 1, -128, bias10, multiply10, add10, shift10, 1);
        printf("Layer 10 Done\n");

        complete_pointwise_conv_layer(OUTPUT_MATRIX_int8,WEIGHT_MATRIX11, CHANNELS_IN11, CHANNELS_OUT11, 12, bias11, multiply11, add11,  shift11);
        printf("Layer 11 Done\n");

        depthwise_conv_layer(OUTPUT_MATRIX_int8, WEIGHT_MATRIX12, CHANNELS_OUT11, CHANNELS12, 3, 12, 0, 1, 0, 1, -128, bias12, multiply12, add12, shift12, 2);
        printf("Layer 12 Done\n");

        complete_pointwise_conv_layer(OUTPUT_MATRIX_int8,WEIGHT_MATRIX13, CHANNELS_IN13, CHANNELS_OUT13, 6, bias13, multiply13, add13,  shift13);
        printf("Layer 13 Done\n");

        depthwise_conv_layer(OUTPUT_MATRIX_int8, WEIGHT_MATRIX14, CHANNELS_OUT13, CHANNELS14, 3, 6, 1, 1, 1, 1, -128, bias14, multiply14, add14, shift14, 1);
        printf("Layer 14 Done\n");

        complete_pointwise_conv_layer(OUTPUT_MATRIX_int8,WEIGHT_MATRIX15, CHANNELS_IN15, CHANNELS_OUT15, 6, bias15, multiply15, add15,  shift15);
        printf("Layer 15 Done\n");

        depthwise_conv_layer(OUTPUT_MATRIX_int8, WEIGHT_MATRIX16, CHANNELS_OUT15, CHANNELS16, 3, 6, 1, 1, 1, 1, -128, bias16, multiply16, add16, shift16, 1);
        printf("Layer 16 Done\n");

        complete_pointwise_conv_layer(OUTPUT_MATRIX_int8,WEIGHT_MATRIX17, CHANNELS_IN17, CHANNELS_OUT17, 6, bias17, multiply17, add17,  shift17);
        printf("Layer 17 Done\n");

        depthwise_conv_layer(OUTPUT_MATRIX_int8, WEIGHT_MATRIX18, CHANNELS_OUT17, CHANNELS18, 3, 6, 1, 1, 1, 1, -128, bias18, multiply18, add18, shift18, 1);
        printf("Layer 18 Done\n");

        complete_pointwise_conv_layer(OUTPUT_MATRIX_int8,WEIGHT_MATRIX19, CHANNELS_IN19, CHANNELS_OUT19, 6, bias19, multiply19, add19,  shift19);
        printf("Layer 19 Done\n");

        depthwise_conv_layer(OUTPUT_MATRIX_int8, WEIGHT_MATRIX20, CHANNELS_OUT19, CHANNELS20, 3, 6, 1, 1, 1, 1, -128, bias20, multiply20, add20, shift20, 1);
        printf("Layer 20 Done\n");

        complete_pointwise_conv_layer(OUTPUT_MATRIX_int8,WEIGHT_MATRIX21, CHANNELS_IN21, CHANNELS_OUT21, 6, bias21, multiply21, add21,  shift21);
        printf("Layer 21 Done\n");

        depthwise_conv_layer(OUTPUT_MATRIX_int8, WEIGHT_MATRIX22, CHANNELS_OUT21, CHANNELS22, 3, 6, 1, 1, 1, 1, -128, bias22, multiply22, add22, shift22, 1);
        printf("Layer 22 Done\n");

        complete_pointwise_conv_layer(OUTPUT_MATRIX_int8,WEIGHT_MATRIX23, CHANNELS_IN23, CHANNELS_OUT23, 6, bias23, multiply23, add23,  shift23);
        printf("Layer 23 Done\n");

        depthwise_conv_layer(OUTPUT_MATRIX_int8, WEIGHT_MATRIX24, CHANNELS_OUT23, CHANNELS24, 3, 6, 0, 1, 0, 1, -128, bias24, multiply24, add24, shift24, 2);
        printf("Layer 24 Done\n");

        complete_pointwise_conv_layer(OUTPUT_MATRIX_int8,WEIGHT_MATRIX25, CHANNELS_IN25, CHANNELS_OUT25, 3, bias25, multiply25, add25,  shift25);
        printf("Layer 25 Done\n");


        depthwise_conv_layer(OUTPUT_MATRIX_int8, WEIGHT_MATRIX26, CHANNELS_OUT25, CHANNELS26, 3, 3, 1, 1, 1, 1, -128, bias26, multiply26, add26, shift26, 1);
        printf("Layer 26 Done\n");

        layer27(OUTPUT_MATRIX_int8, WEIGHT_MATRIX27, CHANNELS_IN27, CHANNELS_OUT27, 3, bias27, multiply27, add27,  shift27);

        printf("Layer 27 Done\n");

        avg_pool_layer(OUTPUT_MATRIX_int8, 9, 256, OUTPUT_MATRIX_int8);
        printf("Layer 28 Done\n");

        final_conv_layer(OUTPUT_MATRIX_int8,WEIGHT_MATRIX29, CHANNELS_IN29, CHANNELS_OUT29, 1, bias29, multiply29, add29,  shift29);
        printf("Layer 29 Done\n");

        printf("output at 0: %d \n",OUTPUT_MATRIX_int8[0]);
        printf("output at 1: %d \n",OUTPUT_MATRIX_int8[1]);

        softmax_and_output(OUTPUT_MATRIX_int8,2);
    //}
    return 0;
}

