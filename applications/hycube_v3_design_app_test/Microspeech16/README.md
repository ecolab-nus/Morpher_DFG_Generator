# Microspeech

To compile:
gcc -w microspeech.c -o micro -lpthread

You can change the filename according to the label you want to run the model for in read_test_data_param() function. 
There are four possible output labels: 1, 2, 3 and 4. 


To compile:
gcc -w microspeech_int16.c -o micro -lpthread 
Change "path_to_dir" on line 848 and line 860 accordingly.  
