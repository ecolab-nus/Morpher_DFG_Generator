opt -gvn -loop-simplify -simplifycfg jidctflt.bc -o jidctflt_gvn.bc
opt -load ../../buildeclipse/skeleton/libSkeletonPass.so -skeleton jidctflt_gvn.bc
