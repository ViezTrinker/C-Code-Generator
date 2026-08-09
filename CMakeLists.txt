cmake_minimum_required(VERSION 3.20)
project(c_code_generator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Dependencies.cmake")

set(PROJECT_HEADERS
   include/model/block_type.h
   include/model/c_type.h
   include/model/edge.h
   include/model/graph_document.h
   include/model/node.h
   include/model/port.h
   include/model/result.h
   include/codegen/c_codegen.h
   include/serialize/cgen_serializer.h
   include/build/build_runner.h
   include/build/process_session.h
   include/gui/app.h
   include/gui/canvas_view.h
   include/gui/log_pane.h
   include/gui/palette.h
   include/gui/property_panel.h
   include/gui/toolbar.h
)

set(PROJECT_SOURCES
   src/main.cpp
   src/model/c_type.cpp
   src/model/graph_document.cpp
   src/model/node_factory.cpp
   src/codegen/c_codegen.cpp
   src/serialize/cgen_serializer.cpp
   src/build/build_runner.cpp
   src/build/process_session.cpp
   src/gui/app.cpp
   src/gui/canvas_view.cpp
   src/gui/log_pane.cpp
   src/gui/palette.cpp
   src/gui/property_panel.cpp
   src/gui/toolbar.cpp
)

add_executable(c_code_generator
   ${PROJECT_SOURCES}
   ${PROJECT_HEADERS}
)

target_include_directories(c_code_generator PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(c_code_generator PRIVATE
   SFML::System
   SFML::Window
   SFML::Graphics
   nlohmann_json::nlohmann_json
)

if(WIN32)
   target_link_libraries(c_code_generator PRIVATE comdlg32)
   # TARGET_RUNTIME_DLLS is empty for fully static links; skip copy in that case.
   add_custom_command(TARGET c_code_generator POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E
         $<IF:$<BOOL:$<TARGET_RUNTIME_DLLS:c_code_generator>>,copy_if_different,true>
         $<TARGET_RUNTIME_DLLS:c_code_generator>
         $<TARGET_FILE_DIR:c_code_generator>
      COMMAND_EXPAND_LISTS
   )
endif()
