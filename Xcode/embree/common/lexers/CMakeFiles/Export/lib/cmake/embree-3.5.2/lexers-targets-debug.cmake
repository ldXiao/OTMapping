#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "lexers" for configuration "Debug"
set_property(TARGET lexers APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(lexers PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "sys;math"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/liblexers.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS lexers )
list(APPEND _IMPORT_CHECK_FILES_FOR_lexers "${_IMPORT_PREFIX}/lib/liblexers.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
