add_library(Engine STATIC)
set_target_properties(Engine PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
)

find_package(Vulkan)

# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.9
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw
)
FetchContent_MakeAvailable(glfw)
# Spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.17.0
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog
)
FetchContent_MakeAvailable(spdlog)
# imgui
set(IMGUI_IMPL_OPENGL_LOADER_GLAD OFF CACHE BOOL "" FORCE)
set(IMGUI_IMPL_OPENGL_LOADER_GL3W OFF CACHE BOOL "" FORCE)
set(IMGUI_IMPL_OPENGL_LOADER_GLEW OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui
)
FetchContent_MakeAvailable(imgui)
if(NOT TARGET imgui)
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
    )
    target_include_directories(imgui PUBLIC 
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui PRIVATE glfw Vulkan::Vulkan)
endif()
# vma
FetchContent_Declare(
    vma
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG v3.3.0
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/vma
)
FetchContent_MakeAvailable(vma)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_SOURCES})
target_sources(Engine PRIVATE ${ENGINE_SOURCES})
target_include_directories(Engine 
	PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}/Source
)
target_precompile_headers(Engine PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/Source/gepch.h>")

target_link_libraries(Engine
	PUBLIC
		glfw
        spdlog
		Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator
)