rm *_looptrace.log
rm *_loopinfo.log
rm *_munittrace.log
rm memtraces/*
clang -D CGRA_COMPILER -target i386-unknown-linux-gnu -c -emit-llvm -O2 -fno-tree-vectorize -fno-inline   -fno-slp-vectorize -fno-unroll-loops invert_matrix_general.c -S -o invert_matrix_general.ll
opt -gvn -mem2reg -memdep -memcpyopt -lcssa -loop-simplify -licm -loop-deletion -indvars -simplifycfg -mergereturn -indvars invert_matrix_general.ll -o invert_matrix_general_gvn.ll
#opt -load ~/manycore/cgra_dfg/buildeclipse/skeleton/libSkeletonPass.so -fn $1 -ln $2 -ii $3 -skeleton invert_matrix_general_gvn.ll -S -o invert_matrix_general_gvn_instrument.ll

opt -load ../../build/skeleton/libSkeletonPass.so -fn $1 -skeleton invert_matrix_general_gvn.ll -S -o invert_matrix_general_gvn_instrument.ll

# clang -target i386-unknown-linux-gnu -c -emit-llvm -S ../../skeleton/instrumentation/instrumentation.cpp -o instrumentation.ll

# llvm-link invert_matrix_general_gvn_instrument.ll instrumentation.ll -o final.ll

# llc -filetype=obj final.ll -o final.o
# clang++ -m32 final.o -o final
