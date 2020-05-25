cmake_minimum_required(VERSION 3.0.0)

project(myl)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${myl_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(SRC
  componentfile.cpp
  fieldtype.cpp
  lua/lua.cpp
  modules/timer.cpp
  modules/window.cpp
  modules/input.cpp
  main.cpp
)
list(TRANSFORM SRC PREPEND src/)

include_directories(include)

include_directories(deps/sol2/single/include)

include_directories(deps/glm)

find_package(spdlog CONFIG REQUIRED)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

include_directories(deps/tomlplusplus/include)

include(luajit)

add_executable(myl ${SRC})
target_link_libraries(myl spdlog::spdlog libluajit sfml-graphics sfml-window sfml-system)