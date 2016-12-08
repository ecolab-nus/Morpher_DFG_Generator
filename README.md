# HyCUBE Compiler : Acquire DFG and Generate RTL Streams

A completely useless LLVM pass.

Build:
    $ export ROOT\_DIR=[full path to the cgra\_dfg directory]
    $ cd $ROOT\_DIR
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ cd ..

Run:

    $ clang -c -emit-llvm -S [source].c -o [source].bc
    $ opt -always-inline -gvn -loop-simplify -simplifycfg [source].bc -o [source]_gvn.bc
    $ opt -load $ROOT\_DIR/build/skeleton/libSkeletonPass.so -skeleton -fn [function name] -ln [loop number] [source].bc

