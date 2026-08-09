# Unit tests (GoogleTest via third_party/googletest submodule).

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(
   "${CMAKE_SOURCE_DIR}/third_party/googletest"
   "${CMAKE_BINARY_DIR}/_googletest"
   EXCLUDE_FROM_ALL
)

set(CGEN_TEST_SOURCES
   test_block_type.cpp
   test_c_type.cpp
   test_c_codegen.cpp
   test_cgen_serializer.cpp
   test_document_history.cpp
   test_graph_document.cpp
   test_result.cpp
)

add_executable(cgen_unit_tests
   ${CGEN_TEST_SOURCES}
)

target_link_libraries(cgen_unit_tests PRIVATE
   cgen_core
   GTest::gtest_main
)

# Forward slashes avoid Windows escape issues inside the string macro.
file(TO_CMAKE_PATH "${CMAKE_SOURCE_DIR}" CGEN_SOURCE_DIR_NORMALIZED)
target_compile_definitions(cgen_unit_tests PRIVATE
   CGEN_SOURCE_DIR="${CGEN_SOURCE_DIR_NORMALIZED}"
)

include(GoogleTest)
gtest_discover_tests(cgen_unit_tests
   WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
   DISCOVERY_TIMEOUT 60
)
