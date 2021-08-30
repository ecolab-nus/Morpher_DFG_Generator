rm *_looptrace.log
rm *_loopinfo.log
rm *_munittrace.log
rm memtraces/*
clang -D CGRA_COMPILER -target i386-unknown-linux-gnu -c -emit-llvm -O2 -fno-tree-vectorize fix_fft.c -S -o integer_fft.ll
opt -gvn -mem2reg -memdep -memcpyopt -lcssa -loop-simplify -licm -loop-deletion -indvars -simplifycfg -mergereturn -indvars integer_fft.ll -o integer_fft_gvn.ll
#opt -load ~/manycore/cgra_dfg/buildeclipse/skeleton/libSkeletonPass.so -fn $1 -ln $2 -ii $3 -skeleton integer_fft_gvn.ll -S -o integer_fft_gvn_instrument.ll

# opt -load /home/dmd/Workplace/Morphor/hycube-compiler/build/skeleton/libSkeletonPass.so -fn $1 -munit $2 -ii $3 -dx $4 -dy $5 -skeleton integer_fft_gvn.ll -S -o integer_fft_gvn_instrument.ll

# clang -target i386-unknown-linux-gnu -c -emit-llvm -S /home/dmd/Workplace/Morphor/hycube-compiler/skeleton/instrumentation/instrumentation.cpp -o instrumentation.ll
# llvm-link integer_fft_gvn_instrument.ll instrumentation.ll -o final.ll

# llc -filetype=obj final.ll -o final.o
# g++ -m32 final.o -o final


opt -load ../../../../build/skeleton/libSkeletonPass.so -fn $1  -nobanks $2 -banksize $3 -skeleton integer_fft_gvn.ll -S -o integer_fft_gvn_instrument.ll

clang -target i386-unknown-linux-gnu -c -emit-llvm -S ../../../../skeleton/instrumentation/instrumentation.cpp -o instrumentation.ll

llvm-link integer_fft_gvn_instrument.ll instrumentation.ll -o final.ll

llc -filetype=obj final.ll -o final.o
clang++ -m32 final.o -o final
