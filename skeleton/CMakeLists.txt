add_library(SkeletonPass MODULE
    # List your source files here.
    Skeleton.cpp
    CGRANode.h
    CGRANode.cpp
    CGRA.h
    CGRA.cpp
    edge.h
    edge.cpp
    dfgnode.h
    dfgnode.cpp
    dfg.h
    dfg.cpp
    astar.h
    astar.cpp
    tinyxml2.h
    tinyxml2.cpp
    dfgmacro.cpp
    dfgmacro.h
    DFGTrig.h
    DFGTrig.cpp
    DFGPartPred.h
    DFGPartPred.cpp
)


# Use C++11 to compile our pass (i.e., supply -std=c++11).
target_compile_features(SkeletonPass PRIVATE cxx_range_for cxx_auto_type)

# LLVM is (typically) built with no C++ RTTI. We need to match that.
set_target_properties(SkeletonPass PROPERTIES
    COMPILE_FLAGS "-fno-rtti -ggdb"
)

# Get proper shared-library behavior (where symbols are not necessarily
# resolved when the shared library is linked) on OS X.
if(APPLE)
    set_target_properties(SkeletonPass PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
endif(APPLE)
