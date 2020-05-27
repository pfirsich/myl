cmake_minimum_required(VERSION 3.0.0)

project(myl)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${myl_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

set(SRC
  componentfile.cpp
  debugsystem.cpp
  ecs.cpp
  fieldtype.cpp
  lua/lua.cpp
  main.cpp
  modules/imguistyle.cpp
  modules/input.cpp
  modules/timer.cpp
  modules/window.cpp
  util.cpp
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

set(IMGUI_DIR "${CMAKE_CURRENT_LIST_DIR}/deps/imgui")
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
