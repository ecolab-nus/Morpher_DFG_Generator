opt -gvn -loop-simplify -simplifycfg aes.bc -o aes_gvn.bc
opt -load ../buildeclipse/skeleton/libSkeletonPass.so -skeleton aes_gvn.bc
