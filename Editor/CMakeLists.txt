add_executable(Editor)
set_target_properties(Editor PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
)

file(GLOB_RECURSE EDITOR_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${EDITOR_SOURCES})
target_sources(Editor PRIVATE ${EDITOR_SOURCES})
target_include_directories(Editor 
	PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(Editor
	PUBLIC
		Engine
)