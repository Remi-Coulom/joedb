# Development overlay: build the current joedb worktree. Replace this with a
# vcpkg_from_github() source block after the packaging changes are released.
get_filename_component(SOURCE_PATH "${CURRENT_PORT_DIR}/../.." ABSOLUTE)

if(VCPKG_TARGET_IS_WINDOWS)
 vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
endif()

vcpkg_check_features(
 OUT_FEATURE_OPTIONS FEATURE_OPTIONS
 FEATURES
  asio   JOEDB_WITH_ASIO
  brotli JOEDB_WITH_BROTLI
  curl   JOEDB_WITH_CURL
  ssh    JOEDB_WITH_SSH
)

set(JOEDB_OPTIONS
 -DBUILD_TESTING=OFF
 -DJOEDB_BUILD_BENCHMARKS=OFF
 -DJOEDB_BUILD_EXAMPLES=OFF
 -DJOEDB_BUILD_FUZZERS=OFF
 -DJOEDB_BUILD_TOOLS=OFF
 -DJOEDB_ENABLE_CPACK=OFF
 -DJOEDB_WITH_WEBSOCKETS=OFF
 ${FEATURE_OPTIONS}
)

if(VCPKG_CROSSCOMPILING)
 list(APPEND JOEDB_OPTIONS
  "-DJOEDBC_EXECUTABLE=${CURRENT_HOST_INSTALLED_DIR}/tools/${PORT}/joedbc${VCPKG_HOST_EXECUTABLE_SUFFIX}"
 )
endif()

vcpkg_cmake_configure(
 SOURCE_PATH "${SOURCE_PATH}/compcmake"
 OPTIONS ${JOEDB_OPTIONS}
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/joedb)
vcpkg_copy_pdbs()

if(HOST_TRIPLET STREQUAL TARGET_TRIPLET)
 vcpkg_copy_tools(TOOL_NAMES joedbc AUTO_CLEAN)
else()
 file(APPEND "${CURRENT_PACKAGES_DIR}/share/joedb/joedbConfig.cmake"
"\nif(NOT TARGET joedb::joedbc)
 add_executable(joedb::joedbc IMPORTED)
 set_target_properties(joedb::joedbc PROPERTIES
  IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/../../../${HOST_TRIPLET}/tools/${PORT}/joedbc${VCPKG_HOST_EXECUTABLE_SUFFIX}\"
 )
endif()
")
endif()

file(REMOVE_RECURSE
 "${CURRENT_PACKAGES_DIR}/debug/include"
 "${CURRENT_PACKAGES_DIR}/debug/share"
)

file(INSTALL "${CURRENT_PORT_DIR}/usage"
 DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
