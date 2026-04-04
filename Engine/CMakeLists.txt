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

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_SOURCES})
target_sources(Engine PRIVATE ${ENGINE_SOURCES})
target_include_directories(Engine 
	PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(Engine
	PUBLIC
		glfw
        spdlog
		Vulkan::Vulkan
)