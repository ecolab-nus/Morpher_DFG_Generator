add_library(dfggenPass MODULE
    # List your source files here.
    dfggenPass.cpp
    ../include/morpherdfggen/arch/CGRANode.h
    arch/CGRANode.cpp
    ../include/morpherdfggen/arch/CGRA.h
    arch/CGRA.cpp
    ../include/morpherdfggen/common/edge.h
    common/edge.cpp
    ../include/morpherdfggen/common/dfgnode.h
    common/dfgnode.cpp
    ../include/morpherdfggen/common/dfg.h
    common/dfg.cpp
    ../include/morpherdfggen/common/astar.h
    common/astar.cpp
    ../include/morpherdfggen/util/tinyxml2.h
    util/tinyxml2.cpp
    common/dfgmacro.cpp
    ../include/morpherdfggen/common/dfgmacro.h
    ../include/morpherdfggen/dfg/DFGTrig.h
    dfg/DFGTrig.cpp
    ../include/morpherdfggen/dfg/DFGPartPred.h
    dfg/DFGPartPred.cpp
    ../include/morpherdfggen/dfg/DFGPartPredLight.h
    dfg/DFGPartPredLight.cpp
    ../include/morpherdfggen/dfg/DFGCgraMe.h
    dfg/DFGCgraMe.cpp
    ../include/morpherdfggen/dfg/DFGOpenCGRA.h
    dfg/DFGOpenCGRA.cpp
    ../include/morpherdfggen/dfg/DFGFullPred.h
    dfg/DFGFullPred.cpp
    ../include/morpherdfggen/dfg/DFGDISE.h
    dfg/DFGDISE.cpp
    ../include/morpherdfggen/dfg/DFGBrMap.h
    dfg/DFGBrMap.cpp
    ../include/morpherdfggen/dfg/DFGTrMap.h
    dfg/DFGTrMap.cpp
)


# Use C++11 to compile our pass (i.e., supply -std=c++11).
target_compile_features(dfggenPass PRIVATE cxx_range_for cxx_auto_type)

# LLVM is (typically) built with no C++ RTTI. We need to match that.
set_target_properties(dfggenPass PROPERTIES
    COMPILE_FLAGS "-fno-rtti -ggdb"
)

# Get proper shared-library behavior (where symbols are not necessarily
# resolved when the shared library is linked) on OS X.
if(APPLE)
    set_target_properties(dfggenPass PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
endif(APPLE)
