

clang -D CGRA_COMPILER -target i386-unknown-linux-gnu -c -emit-llvm -O2 -fno-tree-vectorize -fno-inline -fno-unroll-loops $1.c -S -o $1.ll
opt -gvn -mem2reg -memdep -memcpyopt -lcssa -loop-simplify -licm -loop-deletion -indvars -simplifycfg -mergereturn -indvars $1.ll -o $1_gvn.ll
#opt -load ~/manycore/cgra_dfg/buildeclipse/skeleton/libSkeletonPass.so -fn $1 -ln $2 -ii $3 -skeleton integer_fft_gvn.ll -S -o integer_fft_gvn_instrument.ll

opt -load ./build/skeleton/libSkeletonPass.so -fn $2 -skeleton $1_gvn.ll -S -o $1_gvn_instrument.ll -nobanks ${3:-2}  -banksize ${4:-2048}

clang -target i386-unknown-linux-gnu -c -emit-llvm -S ./skeleton/instrumentation/instrumentation.cpp -o instrumentation.ll