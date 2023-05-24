rm *_looptrace.log
rm *_loopinfo.log
rm *_munittrace.log
rm memtraces/*
clang -D CGRA_COMPILER -target i386-unknown-linux-gnu -c -emit-llvm -O1  -fno-inline-functions -fno-tree-vectorize   ../aesxam.c -S -o kernel.ll
opt -gvn -mem2reg -memdep -memcpyopt -lcssa -loop-simplify -licm -loop-deletion -indvars -simplifycfg -mergereturn -indvars kernel.ll -o kernel_gvn.ll
#opt -load ~/manycore/cgra_dfg/buildeclipse/skeleton/libSkeletonPass.so -fn $1 -ln $2 -ii $3 -skeleton integer_fft_gvn.ll -S -o integer_fft_gvn_instrument.ll
echo "Morpher Pass"
opt -load ../../../../build/src/libdfggenPass.so -fn $1 -skeleton kernel_gvn.ll -S -o kernel_gvn_instrument.ll 
clang -target i386-unknown-linux-gnu -c -emit-llvm -S ../../../../src/instrumentation/instrumentation.cpp -o instrumentation.ll
echo "Linking Instrumentation.cpp"
llvm-link kernel_gvn_instrument.ll instrumentation.ll -o final.ll
echo "Creating Object file"
llc -filetype=obj final.ll -o final.o
clang++ -m32 final.o -o final


