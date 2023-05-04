# Visual Wake Word Application - C Implementation

The Visual Wake Work Application shall detect people in images in a lightweight and efficient manner such that the application can be deployed on a microcontroller. The following image shows the steps that have been necessary to implement this application. This folder contains the implementation of this application in C.


The [person_detect.c](person_detect.c) script contains a more extended/ an earlier version of the VWW implementation. It is not optimized for minimal memory usage rather than a refernece implementation.

Use the following commands to compile and run the script
```
gcc -w person_detect.c -o person_detect  

./person_detect
```

The main.c(main.c) script contains the functionally identical implementaion of the VWW application but the code has been optimized to minimize memory usage.

Use the following command to compile the script
```
gcc -w main.c -o main -lm -lstdc++

./main
```

The weights, bias and quantization parameters are stored in the respective header files.

The [image.h](image.h) header file contains the perprocessed input image data for a picture of a person and a house (=not a person) which can be used for testing purposes. Additionally the input.h(input.h) header file contains the person input image after the im2col function (padding and flatting input) which has also been used for testing purposes. The [test-dataset](test-dataset) folder contains a python script which automatically loops over images in a folder, preprocess them and stores the result in respective header files. The current script has been used with the val2017 image folder of the COCO dataset but can be adapted to other image folders.

