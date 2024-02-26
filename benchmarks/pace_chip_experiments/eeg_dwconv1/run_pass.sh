rm *_looptrace.log
rm *_loopinfo.log
rm *_munittrace.log
rm memtraces/*
set -e
echo $1
clang -D CGRA_COMPILER  -c -emit-llvm -O3 -fno-vectorize -fno-slp-vectorize -fno-tree-vectorize -fno-inline -fno-unroll-loops -m32 $1 -S -o microspeech.ll
opt -gvn -mem2reg -memdep -memcpyopt -lcssa -loop-simplify -licm -disable-slp-vectorization -loop-deletion -indvars -simplifycfg -mergereturn -indvars microspeech.ll -S -o microspeech_opt.ll
#opt -load ~/manycore/cgra_dfg/buildeclipse/skeleton/libSkeletonPass.so -fn $1 -ln $2 -ii $3 -skeleton integer_fft_gvn.ll -S -o integer_fft_gvn_instrument.ll

opt -load ../../../build/src/libdfggenPass.so -fn conv_main -nobanks 2 -banksize 8192 -skeleton microspeech_opt.ll -S -o microspeech_opt_instrument.ll

clang  -m32 -c -emit-llvm -S ../../../src/instrumentation/instrumentation.cpp -o instrumentation.ll

llvm-link microspeech_opt_instrument.ll instrumentation.ll -o final.ll

llc -filetype=obj final.ll -o final.o
clang++ -m32  final.o -o final
./final 1> final_log.txt 2> final_err_log.txt


function morpher_mapper() {
    local MORPHER_MAPPER_PATH=~/Morpher1/Morpher_CGRA_Mapper/
    local json_arch=${MORPHER_MAPPER_PATH}/json_arch/hycube_original_updatemem.json
    local update_mem_alloc_python=${MORPHER_MAPPER_PATH}/update_mem_alloc.py

    local mem_alloc_txt_file=$1_mem_alloc.txt
    python $update_mem_alloc_python $json_arch $mem_alloc_txt_file 8192 2 hycube_original_mem.json
    ${MORPHER_MAPPER_PATH}/build/src/cgra_xml_mapper -d $1_PartPredDFG.xml -j hycube_original_mem.json -i 0 -t HyCUBE_4REG -m 0
}

#morpher_mapper conv_main
