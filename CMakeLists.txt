cmake_minimum_required(VERSION 3.20)

project("GlassEngine")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

include(FetchContent)
add_subdirectory(Engine)
add_subdirectory(Editor)

function(copy_file_to_bin TARGET_NAME FILE_PATH)
    if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
        # temp
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Editor")
    endif()

    if(NOT EXISTS "${FILE_PATH}")
        message(WARNING "file not exists, file name: ${FILE_PATH}")
        return()
    endif()

    get_filename_component(FILE_NAME "${FILE_PATH}" NAME)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${FILE_PATH}"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/${FILE_NAME}"
        COMMENT "copy: ${FILE_NAME}"
    )
endfunction()

if(WIN32)
    copy_file_to_bin(Editor ${CMAKE_CURRENT_SOURCE_DIR}/vendor/compressonator/binary/Compressonator_MT_DLL.dll)
    copy_file_to_bin(Editor ${CMAKE_CURRENT_SOURCE_DIR}/vendor/aftermath/lib/GFSDK_Aftermath_Lib.x64.dll)
else()
    copy_file_to_bin(Editor ${CMAKE_CURRENT_SOURCE_DIR}/vendor/aftermath/lib/libGFSDK_Aftermath_Lib.x64.so)
endif(WIN32)