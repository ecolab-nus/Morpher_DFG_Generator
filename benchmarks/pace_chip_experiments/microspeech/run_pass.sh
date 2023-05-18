rm *_looptrace.log
rm *_loopinfo.log
rm *_munittrace.log
rm memtraces/*
clang -D CGRA_COMPILER  -c -emit-llvm -O2 -fno-vectorize -fno-slp-vectorize -fno-tree-vectorize -fno-inline -fno-unroll-loops microspeech_int16_test.c -S -o microspeech.ll
opt -gvn -mem2reg -memdep -memcpyopt -lcssa -loop-simplify -licm -disable-slp-vectorization -loop-deletion -indvars -simplifycfg -mergereturn -indvars microspeech.ll -S -o microspeech_opt.ll
#opt -load ~/manycore/cgra_dfg/buildeclipse/skeleton/libSkeletonPass.so -fn $1 -ln $2 -ii $3 -skeleton integer_fft_gvn.ll -S -o integer_fft_gvn_instrument.ll

opt -load ../../../build/src/libdfggenPass.so -fn $1 -nobanks $2 -banksize $3 -skeleton microspeech_opt.ll -S -o microspeech_opt_instrument.ll

clang  -c -emit-llvm -S ../../../src/instrumentation/instrumentation.cpp -o instrumentation.ll

llvm-link microspeech_opt_instrument.ll instrumentation.ll -o final.ll

llc -filetype=obj final.ll -o final.o
clang++  final.o -o final
./final 1> final_log.txt 2> final_err_log.txt