add_library(GlassRenderer SHARED)
set_target_properties(GlassRenderer PROPERTIES 
    CXX_STANDARD 20 
    CXX_STANDARD_REQUIRED ON
)

find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

find_package(Vulkan)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

target_include_directories(GlassRenderer PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/Application/include
    ${CMAKE_SOURCE_DIR}/Tools/include
)

file(GLOB_RECURSE gengine_sources ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${gengine_sources})
target_sources(GlassRenderer PRIVATE ${gengine_sources})
target_link_libraries(GlassRenderer 
    PUBLIC
        Tools
        Engine
        Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator
        glm::glm-header-only
)