add_library(Tools STATIC
    include/Core.h
    include/UUID.h
    include/Flags.h
    include/Memory/Allocator.h
    include/Memory/PlatformMemoryAlloc.h
    include/Memory/Ref.h
    include/Memory/Memory.h
    src/Memory/Allocator.cpp
)

set_target_properties(Tools PROPERTIES 
    CXX_STANDARD 20 
    CXX_STANDARD_REQUIRED ON
)

if(DEFINED ENV{GLASS_ENGINE_DIR})
    target_compile_definitions(Tools PUBLIC GE_DIR="$ENV{GLASS_ENGINE_DIR}")
    message(STATUS "Glass Engine Directory: $ENV{GLASS_ENGINE_DIR}")
endif()

target_include_directories(Tools PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/include
)