# Visual Wake Word Application

The Visual Wake Work Application shall detect people in images in a lightweight and efficient manner such that the application can be deployed on a microcontroller. The following image shows the steps that have been necessary to implement this application.

![Roadmap](https://github.com/melina2200/Research-Internship-NUS/blob/main/VWW-Application/img/roadmap.png?raw=true)

**(1)** In the first step the model had to be trained. I used the [COCO Dataset](https://cocodataset.org/#home) for training. The model is based on the [MobileNets](https://arxiv.org/pdf/1704.04861.pdf) model. The [TrainPersonDetect Jupyter Notebook](TrainPersonDetect.ipynb) contains the script to train the model for the Person Detection Algorithm. I used it in Google Colab. Maybe there need to be done some modifications if you run it as a jupyter notebook. The script is based on the Training of the [Person Detection Example of the TF Lite Micro Repo](https://github.com/tensorflow/tflite-micro/tree/main/tensorflow/lite/micro/examples/person_detection).

The final ouput of the trained model can be found in the TrainedModel folders for [298.000 Iterations](Trained-Model-298000Iter) and for [1.000.000 Iterations](Trained-Model-1000000Iter). These models have been trained with an input image size of 96x96 Pixels (grayscale).

**(2)** In the second step I had to extract the trained model weight, the bias and the quantization parameters that are needed for the implementation in C of this network.[The TVMScripts folder](TVMScripts) contains all scripts used for extracting model parameters from trained TFLite models with the help of TVM. To run these scripts it is necessary to install TVM, more information in the ReadMe File within that folder.

**(3)** In the third steo I implemented the model in nativeC code. This step is necessary to be able to modify the layer functions for a simpler acceleration with the Hycube CGRA. [The C Implementation folder](CImplementation) contains the implemetation of Visual Wake Word Application in C, all model parameters needed have been extracted from the Trained Model (1000000 Iterations) and stored in header files. 

**(4)** In the next step both implementations (TFLite and nativeC) are deployed onto the Manuca Microcontroller with the help of the [mbedOS Framework](https://os.mbed.com/mbed-os/). 

**(5)** In the future the model shall be accelerated with the HyCube CGRA to achieve even faster and more efficient results. 


## Other Files

[The NodeRepresentation](NodeRepresentation) contains the Layer-Node Representation of the Visual Wake Word Network

The [Person Detection Algorithm Excel File](Person_Detection_Algorithm.xlsx) contains detailed information about the layer structure of the implemented model.


## VWW Model
The Visual Wake Word Model for Person Detection is based on the [MobileNets](https://arxiv.org/pdf/1704.04861.pdf) model and makes use of depthwise seperable convolution. In this process the 'normal' convolution is seperated in two convolution steps, the depthwise and the pointwise convolution. This separation divides a convolution kernel into two smaller kernels leading to a reduction in multiplications and therefore computational complexity. In the example given below instead of doing 3x3x3 = 27 multiplications we will now do 3x3 = 9 multiplications in the first step and them 3 in the second, leading to a total of 12 multiplications.

![SeparableConv](https://github.com/melina2200/Research-Internship-NUS/blob/main/VWW-Application/img/separableConv.png?raw=true)


**TODO**: Add detailed layer description of DepthwiseConv, AvgPooling, Softmax and describe Quantization
