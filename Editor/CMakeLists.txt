add_executable(Editor
    src/main.cpp
)
set_target_properties(Editor PROPERTIES 
    CXX_STANDARD 20 
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(Editor PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/Application/include
    ${CMAKE_SOURCE_DIR}/Tools/include
)

target_link_libraries(Editor 
    PUBLIC
        Application
)