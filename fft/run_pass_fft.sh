#clang -Xclang -load -Xclang buildeclipse/skeleton/libSkeletonPass.so mibench_telecom/telecomm/FFT/fftmisc.c mibench_telecom/telecomm/FFT/fourierf.c mibench_telecom/telecomm/FFT/main.c

#clang -S ../mibench_telecom/telecomm/FFT/fftmisc.c ../mibench_telecom/telecomm/FFT/fourierf.c ../mibench_telecom/telecomm/FFT/main.c

#clang -S fftmisc.ll fourierf.ll main.ll -o fft.ll

rm *.log .
opt -load ../buildeclipse/skeleton/libSkeletonPass.so -skeleton fourierf_gvn.ll
