
#This Script can be used to convert the input image from a signed decimal representation to a hex representation such that it can directly used with TFLite and mbedOS

#This function converts a signed decimal value(val) to a hex string with nbits
def tohex(val, nbits):
  return hex((val + (1 << nbits)) % (1 << nbits))

num_of_bits = 8
file_org = 'image.h' #file to read from
file_new = 'image_hex.h' #file to write to


#get file content
with open(file_org) as f:
    lines = f.readlines()
    f.close()


#iterate over content
with open(file_new, 'w') as f:
    for line in lines:
        if '{' in line: #only look at lines that conatain hex values
            posStart = line.find('{')
            posEnd = line.find('}')
            value_string = line[posStart+1: posEnd]
            values = value_string.split(',')
            newLine = line[:posStart+1]
            print(newLine)
            for value in values:#iterate over all values representing the input image
                value = int(value)
                hexValue = tohex(value, num_of_bits)
                newLine += hexValue + ', '
            newLine = newLine[:-1]+'};'
            f.write(newLine)
            print(newLine)
        else: f.write(line)
    f.close()
        

