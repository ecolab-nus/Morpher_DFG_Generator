# Microspeech

You can change the filename according to the label you want to run the model for in read_test_data_param() function. 
There are four possible output labels: 1, 2, 3 and 4. 


To compile:
gcc -w microspeech_int16_test.c -o micro -lpthread 
Change "path_to_dir" on line 874 and line 886 accordingly.


Compile with FPGA emulation test:
gcc -w microspeech_int16_live.c -lft4222 -lftd2xx -Wl,-rpath /usr/local/lib -o microlive -lpthread  
