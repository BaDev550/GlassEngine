add_library(Application SHARED)
set_target_properties(Application PROPERTIES 
    CXX_STANDARD 20 
    CXX_STANDARD_REQUIRED ON
)

find_package(glfw3 CONFIG REQUIRED)

target_include_directories(Application PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/Tools/include
)
target_compile_definitions(Application PRIVATE GE_EXPORT)

file(GLOB_RECURSE app_sources ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${app_sources})
target_sources(Application PRIVATE ${app_sources})
target_link_libraries(Application 
    PUBLIC
        Tools
        glfw
)