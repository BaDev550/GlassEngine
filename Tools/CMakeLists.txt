add_library(Tools STATIC
    include/Core.h
    include/UUID.h
    include/Flags.h
    include/Memory/Allocator.h
    include/Memory/PlatformMemoryAlloc.h
    include/Memory/Ref.h
    include/Memory/Memory.h
    src/Memory/Allocator.cpp
 "include/Logger.h")

set_target_properties(Tools PROPERTIES 
    CXX_STANDARD 20 
    CXX_STANDARD_REQUIRED ON
)

find_package(glm CONFIG REQUIRED)

if(DEFINED ENV{GLASS_ENGINE_DIR})
    target_compile_definitions(Tools PUBLIC GE_DIR="$ENV{GLASS_ENGINE_DIR}")
    message(STATUS "Glass Engine Directory: $ENV{GLASS_ENGINE_DIR}")
endif()

target_include_directories(Tools PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Tools
	PUBLIC
        glm::glm-header-only
)