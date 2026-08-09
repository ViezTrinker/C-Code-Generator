# CMake dependency bootstrap for Graphical C Code Generator.
# Prefer vendored / local trees, otherwise FetchContent downloads SFML once.
# nlohmann_json is always taken from third_party/nlohmann/json.hpp in this repo.
include_guard(GLOBAL)

include(FetchContent)

# --- nlohmann_json (vendored single header) ---------------------------------
if(NOT TARGET nlohmann_json::nlohmann_json)
   add_library(cgen_nlohmann_json INTERFACE)
   add_library(nlohmann_json::nlohmann_json ALIAS cgen_nlohmann_json)
   target_include_directories(cgen_nlohmann_json INTERFACE
      "${CMAKE_CURRENT_SOURCE_DIR}/third_party"
   )
endif()

# --- SFML 3 -----------------------------------------------------------------
set(CGEN_SFML_TAG "3.0.2" CACHE STRING "SFML git tag / version to fetch")
set(CGEN_SFML_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/sfml")

option(CGEN_USE_SYSTEM_SFML "Prefer a system/vcpkg SFML install if available" OFF)

set(CGEN_SFML_READY FALSE)

if(CGEN_USE_SYSTEM_SFML)
   find_package(SFML 3 COMPONENTS System Window Graphics QUIET)
   if(SFML_FOUND OR TARGET SFML::Graphics)
      set(CGEN_SFML_READY TRUE)
      message(STATUS "Using system SFML")
   endif()
endif()

if(NOT CGEN_SFML_READY)
   if(EXISTS "${CGEN_SFML_LOCAL_DIR}/CMakeLists.txt")
      message(STATUS "Using vendored SFML at ${CGEN_SFML_LOCAL_DIR}")
      set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
      set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
      set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
      set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
      add_subdirectory("${CGEN_SFML_LOCAL_DIR}" "${CMAKE_BINARY_DIR}/_sfml" EXCLUDE_FROM_ALL)
      set(CGEN_SFML_READY TRUE)
   endif()
endif()

if(NOT CGEN_SFML_READY)
   message(STATUS "SFML not found locally; downloading ${CGEN_SFML_TAG} via FetchContent (first configure needs network)")
   set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
   set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
   set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
   set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)

   FetchContent_Declare(
      SFML
      GIT_REPOSITORY https://github.com/SFML/SFML.git
      GIT_TAG "${CGEN_SFML_TAG}"
      GIT_SHALLOW TRUE
      SYSTEM
      EXCLUDE_FROM_ALL
   )
   FetchContent_MakeAvailable(SFML)
   set(CGEN_SFML_READY TRUE)
endif()

if(NOT TARGET SFML::Graphics)
   message(FATAL_ERROR "SFML::Graphics target was not created. Check cmake/Dependencies.cmake.")
endif()
