add_library(Engine STATIC)
set_target_properties(Engine PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
)

find_package(Vulkan)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(slang CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

# meshoptimizer
FetchContent_Declare(
    meshoptimizer
    GIT_REPOSITORY https://github.com/zeux/meshoptimizer
    GIT_TAG v1.1.1
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/meshoptimizer
)
FetchContent_MakeAvailable(meshoptimizer)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_SOURCES})
target_sources(Engine PRIVATE ${ENGINE_SOURCES})
target_include_directories(Engine 
	PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/compressonator/include
)
target_precompile_headers(Engine PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/Source/gepch.h>")
target_compile_definitions(
    Engine PRIVATE
    GLFW_INCLUDE_VULKAN
    $<$<CONFIG:Debug>:_DEBUG>
)
if(WIN32)
target_link_libraries(Engine
	PUBLIC
		glfw
        glm::glm-header-only
        EnTT::EnTT
        imgui::imgui
		Vulkan::Vulkan
        assimp::assimp
        meshoptimizer
        slang::slang
        yaml-cpp
        GPUOpen::VulkanMemoryAllocator
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/compressonator/lib/compressonator_MT_DLL.lib
)
else()
target_link_libraries(Engine
	PUBLIC
		glfw
        glm::glm-header-only
        EnTT::EnTT
		Vulkan::Vulkan
        imgui::imgui
        assimp::assimp
        meshoptimizer
        slang::slang
        yaml-cpp
        GPUOpen::VulkanMemoryAllocator
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/compressonator/lib/libCMP_Compressonator.a
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/compressonator/lib/libCMP_Core.a
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/compressonator/lib/libCMP_Core_AVX.a
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/compressonator/lib/libCMP_Core_AVX512.a
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/compressonator/lib/libCMP_Core_SSE.a
)
endif()