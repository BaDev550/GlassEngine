add_library(Engine SHARED)
set_target_properties(Engine PROPERTIES 
    CXX_STANDARD 20 
    CXX_STANDARD_REQUIRED ON
)

find_package(assimp CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
FetchContent_Declare(
    meshoptimizer
    GIT_REPOSITORY https://github.com/zeux/meshoptimizer
    GIT_TAG v1.1.1
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/meshoptimizer
)
FetchContent_MakeAvailable(meshoptimizer)

target_include_directories(Engine PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/Application/include
    ${CMAKE_SOURCE_DIR}/Tools/include
)
target_compile_definitions(Engine PRIVATE GE_EXPORT)

file(GLOB_RECURSE engine_sources ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${engine_sources})
target_sources(Engine PRIVATE ${engine_sources})
target_link_libraries(Engine 
    PUBLIC
        Tools
        Application
        EnTT::EnTT
        meshoptimizer
        assimp::assimp
        yaml-cpp::yaml-cpp
        glm::glm-header-only
)