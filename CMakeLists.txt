cmake_minimum_required(VERSION 3.14)

project(
  CLIENT_PROJET_RESEAU
  LANGUAGES CXX
  VERSION 0.0.1
  DESCRIPTION ""
)

option(GUI_CLIENT "build la gui qt" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

include(CPM)

CPMAddPackage(
  NAME fmt
  GITHUB_REPOSITORY "fmtlib/fmt"
  GIT_TAG "8.0.0"
  OPTIONS "FMT_DOC OFF" "FMT_TEST OFF"
  EXCLUDE_FROM_ALL
)

CPMAddPackage(
  NAME ftxui
  GITHUB_REPOSITORY "ArthurSonzogni/FTXUI"
  GIT_TAG "master"
  OPTIONS "FTXUI_BUILD_DOCS OFF" "FTXUI_BUILD_EXAMPLES OFF" "FTXUI_ENABLE_INSTALL OFF"
  EXCLUDE_FROM_ALL
)

add_executable(client src/cli.cpp src/client.cpp)
target_link_libraries(client PUBLIC bluetooth fmt::fmt pthread)

add_executable(tui-client src/tui.cpp src/client.cpp)
target_link_libraries(tui-client PUBLIC bluetooth fmt::fmt ftxui::screen ftxui::dom ftxui::component pthread)

if (GUI_CLIENT)
  add_subdirectory(gui)
endif()
