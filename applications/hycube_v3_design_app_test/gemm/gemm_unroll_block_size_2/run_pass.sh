rm *_looptrace.log
rm *_loopinfo.log
rm *_munittrace.log
rm memtraces/*
clang -D CGRA_COMPILER -target i386-unknown-linux-gnu -c -emit-llvm -O2 -fno-vectorize -fno-slp-vectorize -fno-tree-vectorize -fno-inline -fno-unroll-loops gemm.c -S -o gemm.ll
opt -gvn -mem2reg -memdep -memcpyopt -lcssa -loop-simplify -licm -disable-slp-vectorization -loop-deletion -indvars -simplifycfg -mergereturn -indvars gemm.ll -S -o gemm_opt.ll
#opt -load ~/manycore/cgra_dfg/buildeclipse/skeleton/libSkeletonPass.so -fn $1 -ln $2 -ii $3 -skeleton integer_fft_gvn.ll -S -o integer_fft_gvn_instrument.ll

opt -load ../../../../build/skeleton/libSkeletonPass.so -fn $1 -skeleton gemm_opt.ll -S -o gemm_opt_instrument.ll

clang -target i386-unknown-linux-gnu -c -emit-llvm -S ../../../../skeleton/instrumentation/instrumentation.cpp -o instrumentation.ll

llvm-link gemm_opt_instrument.ll instrumentation.ll -o final.ll

llc -filetype=obj final.ll -o final.o
clang++ -m32 final.o -o final
