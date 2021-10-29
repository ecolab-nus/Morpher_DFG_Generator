rm *_looptrace.log
rm *_loopinfo.log
rm *_munittrace.log
rm memtraces/*
clang -D CGRA_COMPILER -target i386-unknown-linux-gnu -c -emit-llvm -O2 -fno-tree-vectorize -fno-slp-vectorize  -mllvm -inline-threshold=100 -fno-unroll-loops libpicojpeg.c -S -o libpicojpeg.ll
opt -gvn -mem2reg -memdep -memcpyopt -lcssa -loop-simplify -licm -loop-deletion -indvars -simplifycfg -mergereturn -indvars -always-inline libpicojpeg.ll -o libpicojpeg_gvn.ll
#opt -load ~/manycore/cgra_dfg/buildeclipse/skeleton/libSkeletonPass.so -fn $1 -ln $2 -ii $3 -skeleton libpicojpeg_gvn.ll -S -o libpicojpeg_gvn_instrument.ll

opt -load ../../build/skeleton/libSkeletonPass.so -fn $1 -skeleton libpicojpeg_gvn.ll -S -o libpicojpeg_gvn_instrument.ll

#clang -target i386-unknown-linux-gnu -c -emit-llvm -S ../../skeleton/instrumentation/instrumentation.cpp -o instrumentation.ll

#llvm-link libpicojpeg_gvn_instrument.ll instrumentation.ll -o final.ll

#llc -filetype=obj final.ll -o final.o
#clang++ -m32 final.o -o final