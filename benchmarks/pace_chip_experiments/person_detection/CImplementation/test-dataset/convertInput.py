#This script can be used to compare results from the C implementation with results from the TVM implementation

from PIL import Image
from matplotlib import pyplot as plt
import numpy as np
import os


def convertImage(img_path):
    resized_image = Image.open(img_path).resize((96, 96))
    #plt.imshow(resized_image)
    #plt.show()
    image_data = np.asarray(resized_image).astype("int8")
    
    # Add a dimension to the image so that we have NHWC format layout
    image_data = np.expand_dims(image_data, axis=0)
    #1,96,96,1
    image_data = (0.21 * image_data[:,:,:,:1]) + (0.72 * image_data[:,:,:,1:2]) + (0.07 * image_data[:,:,:,-1:])
    image_data_int8 = image_data.astype("int8")
    print(np.shape(image_data_int8))
    return image_data_int8

folder_path = os.getcwd()
folder_path += '/val2017'
print(folder_path)

count = 0
for filename in sorted(os.listdir(folder_path)):
    if count > 100: break
    image_path = folder_path +'/'+ filename
    image_int8 = convertImage(image_path)
    image_int8 = np.resize(image_int8, (96,96))
    #print(image_int8)
    #image = Image.open(path)
    print(filename + ' 0')
    txt_file = 'test_arrays/'+str(filename) + '_array.h'
    with open(txt_file, "w") as f:
        f.write('#define IMAGE_DIM  96 \n')
        f.write('extern const int8_t IMAGE[IMAGE_DIM*IMAGE_DIM]= {')
        np.savetxt(f, image_int8.astype("int8"), newline=",",delimiter = ",", fmt='%d')
        f.write('};')
    count += 1
    





