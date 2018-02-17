# HyCUBE Compiler : Acquire DFG and Generate RTL Streams

This version is for LLVM 7 
A completely useless LLVM pass.

Build:

    $ export ROOT_DIR=[full path to the cgra_dfg directory]
    $ cd $ROOT_DIR
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ cd ..

Run:

    $ clang -c -emit-llvm -S [source].c -o [source].bc
    $ opt -always-inline -gvn -loop-simplify -simplifycfg [source].bc -o [source]_gvn.bc
    $ opt -load $ROOT_DIR/build/skeleton/libSkeletonPass.so -skeleton -fn [function name] -ln [loop number] [source]_gvn.bc

