cmake_minimum_required(VERSION 3.10)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("GlassEngine")

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
endif(WIN32)