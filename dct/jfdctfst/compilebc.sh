opt -gvn -loop-simplify -simplifycfg jfdctfst.bc -o jfdctfst_gvn.bc
opt -load ../../buildeclipse/skeleton/libSkeletonPass.so -skeleton jfdctfst_gvn.bc
