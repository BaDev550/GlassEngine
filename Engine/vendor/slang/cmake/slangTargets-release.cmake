#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "slang::slang" for configuration "Release"
set_property(TARGET slang::slang APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(slang::slang PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/slang-compiler.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/binary/slang-compiler.dll"
  )

list(APPEND _cmake_import_check_targets slang::slang )
list(APPEND _cmake_import_check_files_for_slang::slang "${_IMPORT_PREFIX}/lib/slang-compiler.lib" "${_IMPORT_PREFIX}/binary/slang-compiler.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
