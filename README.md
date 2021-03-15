# Morpher DFG Generator :  LLVM-based Control Data-Flow Graph (CDFG) generator

CGRAs target loop kernels where the application spends significat fraction of the execution time. Morpher requires the user to annotate the specifica loop kernels to be mapped. Morpher DFG Generator extract the loop in an .xml file. Morpher supports control-divergence (i.e., existence of multiple control flow paths) inside the loop kernel through partial predication.

## Build dependencies:

This version requires LLVM 10.0.0 and JSON libraries. 

### LLVM, clang, polly:

Read https://llvm.org/docs/GettingStarted.html
follow https://github.com/llvm/llvm-project

    git clone https://github.com/llvm/llvm-project.git
    git checkout <correct version> (llvm10.0.0)
    cd llvm-project
    mkdir build
    cd build
    cmake -DLLVM_ENABLE_PROJECTS='polly;clang' -G "Unix Makefiles" ../llvm
    make -j4
    sudo make install

Important points:

    make sure to checkout correct version before building
    better to use gold linker instead of ld if you face memory problem while building: https://stackoverflow.com/questions/25197570/llvm-clang-compile-error-with-memory-exhausted
    don't use release type use default debug version (will take about 70GB disk space)

### JSON:
https://blog.csdn.net/jiaken2660/article/details/105155257

git clone https://github.com/nlohmann/json.git

    mkdir build
    cd build
    cmake ../
    make -j2
    sudo make install

## Build DFG Generator:

    cd Morpher_DFG_Generator
    git checkout stable
    mkdir build
    cd build
    cmake ..
    make all


# Examples:

    cd applications/

    Use python3 virtual environment to run the script https://linoxide.com/linux-how-to/setup-python-virtual-environment-ubuntu/

    python3 -m venv morpher_env (first time)

    source morpher_env/bin/activate

    Set MORPHER_HOME directory as an environment variable (Ex: export MORPHER_HOME=/home/dmd/Workplace/Morphor/github_ecolab_repos)

    python run_morpher_dfg_generator.py


(for multiple SPM banks)

    ./run_pass.sh hello main 2 2048 

    hello: the source code file name  
    main: function name  
    2: the number of SPM banks. This is optional. The default value is 2.  
    2048: the size of each bank. This is optional and the default value is 2048.  

