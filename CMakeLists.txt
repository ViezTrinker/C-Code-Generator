cmake_minimum_required(VERSION 3.20)
project(c_code_generator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Dependencies.cmake")

option(CGEN_BUILD_TESTS "Build GoogleTest unit tests" ON)

set(CGEN_CORE_HEADERS
   include/model/block_type.h
   include/model/c_type.h
   include/model/edge.h
   include/model/graph_align.h
   include/model/graph_clipboard.h
   include/model/graph_document.h
   include/model/graph_layout.h
   include/model/graph_validator.h
   include/model/node.h
   include/model/port.h
   include/model/result.h
   include/codegen/c_codegen.h
   include/serialize/cgen_serializer.h
   include/gui/document_history.h
   include/gui/block_placement.h
)

set(CGEN_CORE_SOURCES
   src/model/c_type.cpp
   src/model/graph_align.cpp
   src/model/graph_clipboard.cpp
   src/model/graph_document.cpp
   src/model/graph_layout.cpp
   src/model/graph_validator.cpp
   src/model/node_factory.cpp
   src/codegen/c_codegen.cpp
   src/serialize/cgen_serializer.cpp
   src/gui/document_history.cpp
   src/gui/block_placement.cpp
)

set(CGEN_APP_HEADERS
   include/build/build_runner.h
   include/build/process_session.h
   include/gui/app.h
   include/gui/canvas_view.h
   include/gui/context_menu.h
   include/gui/log_pane.h
   include/gui/palette.h
   include/gui/property_panel.h
   include/gui/toolbar.h
)

set(CGEN_APP_SOURCES
   src/main.cpp
   src/build/build_runner.cpp
   src/build/process_session.cpp
   src/gui/app.cpp
   src/gui/canvas_view.cpp
   src/gui/context_menu.cpp
   src/gui/log_pane.cpp
   src/gui/palette.cpp
   src/gui/property_panel.cpp
   src/gui/toolbar.cpp
)

add_library(cgen_core STATIC
   ${CGEN_CORE_SOURCES}
   ${CGEN_CORE_HEADERS}
)

target_include_directories(cgen_core PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cgen_core PUBLIC
   nlohmann_json::nlohmann_json
)

add_executable(c_code_generator
   ${CGEN_APP_SOURCES}
   ${CGEN_APP_HEADERS}
)

target_link_libraries(c_code_generator PRIVATE
   cgen_core
   SFML::System
   SFML::Window
   SFML::Graphics
)

if(WIN32)
   target_link_libraries(c_code_generator PRIVATE comdlg32)
   add_custom_command(TARGET c_code_generator POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E
         $<IF:$<BOOL:$<TARGET_RUNTIME_DLLS:c_code_generator>>,copy_if_different,true>
         $<TARGET_RUNTIME_DLLS:c_code_generator>
         $<TARGET_FILE_DIR:c_code_generator>
      COMMAND_EXPAND_LISTS
   )
endif()

if(CGEN_BUILD_TESTS)
   enable_testing()
   add_subdirectory(tests)
endif()
