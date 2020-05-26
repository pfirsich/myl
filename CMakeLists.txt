cmake_minimum_required(VERSION 3.0.0)

project(myl)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${myl_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(SRC
  componentfile.cpp
  debugsystem.cpp
  ecs.cpp
  fieldtype.cpp
  lua/lua.cpp
  modules/timer.cpp
  modules/window.cpp
  modules/input.cpp
  util.cpp
  main.cpp
)
list(TRANSFORM SRC PREPEND src/)

include_directories(deps/implot)
set(IMPLOT_SRC deps/implot/implot.cpp deps/implot/implot_demo.cpp)

list(APPEND SRC ${IMPLOT_SRC})

include_directories(include)

include_directories(deps/sol2/single/include)

include_directories(deps/glm)

find_package(spdlog CONFIG REQUIRED)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

set(IMGUI_DIR deps/imgui)
set(IMGUI_SFML_FIND_SFML ON)
set(BUILD_SHARED_LIBS ON)
add_subdirectory(deps/imgui-sfml)
set(BUILD_SHARED_LIBS OFF)

include_directories(deps/tomlplusplus/include)

include(luajit)

add_executable(myl ${SRC})
target_link_libraries(myl spdlog::spdlog)
target_link_libraries(myl libluajit)
target_link_libraries(myl sfml-graphics sfml-window sfml-system)
target_link_libraries(myl ImGui-SFML::ImGui-SFML)