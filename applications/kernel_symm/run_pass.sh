rm *_looptrace.log
rm *_loopinfo.log
rm *_munittrace.log
rm memtraces/*

#clang -cc1 -target i386-unknown-linux-gnu polybench.h -emit-pch -o polybench.h.pch

clang -D CGRA_COMPILER -target i386-unknown-linux-gnu -c -emit-llvm -O2 -fno-tree-vectorize -fno-inline -fno-unroll-loops symm.c -S -o trmm.ll

clang -D CGRA_COMPILER -target i386-unknown-linux-gnu -c -emit-llvm -O2 -fno-tree-vectorize -fno-inline -fno-unroll-loops polybench.c -S -o polybench.ll

opt -gvn -mem2reg -memdep -memcpyopt -lcssa -loop-simplify -licm -loop-deletion -indvars -simplifycfg -mergereturn -indvars trmm.ll -S -o trmm_opt.ll

opt -load ../../build/skeleton/libSkeletonPass.so -fn $1 -nobanks $2 -banksize $3 -skeleton trmm_opt.ll -S -o trmm_opt_instrument.ll

clang -target i386-unknown-linux-gnu -c -emit-llvm -S ../../skeleton/instrumentation/instrumentation.cpp -o instrumentation.ll

llvm-link trmm_opt_instrument.ll instrumentation.ll polybench.ll -o final.ll

llc -filetype=obj final.ll -o final.o
clang++ -m32 final.o -o final
