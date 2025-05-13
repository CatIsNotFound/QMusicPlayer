#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "TagLib::tag" for configuration "Release"
set_property(TARGET TagLib::tag APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(TagLib::tag PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/libtag.dll.a"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/libtag.dll"
  )

list(APPEND _cmake_import_check_targets TagLib::tag )
list(APPEND _cmake_import_check_files_for_TagLib::tag "${_IMPORT_PREFIX}/lib/libtag.dll.a" "${_IMPORT_PREFIX}/bin/libtag.dll" )

# Import target "TagLib::tag_c" for configuration "Release"
set_property(TARGET TagLib::tag_c APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(TagLib::tag_c PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/libtag_c.dll.a"
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "TagLib::tag"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/libtag_c.dll"
  )

list(APPEND _cmake_import_check_targets TagLib::tag_c )
list(APPEND _cmake_import_check_files_for_TagLib::tag_c "${_IMPORT_PREFIX}/lib/libtag_c.dll.a" "${_IMPORT_PREFIX}/bin/libtag_c.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
