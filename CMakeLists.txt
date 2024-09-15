cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

project(monokl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)

file(GLOB_RECURSE SOURCES src/main.cpp src/*.cpp)

list(REMOVE_ITEM SOURCES ${CMAKE_SOURCE_DIR}/src/main.cpp)
list(INSERT SOURCES 0 ${CMAKE_SOURCE_DIR}/src/main.cpp)

if(WIN32)
  list(INSERT SOURCES 1 ${CMAKE_SOURCE_DIR}/docs/windows.rc)
  message(STATUS "Added Windows executable icon ${SOURCES}")
endif()

add_executable(${PROJECT_NAME} ${SOURCES})

find_package(SDL2 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(SailC++ CONFIG REQUIRED)
find_package(toml11 CONFIG REQUIRED)

# TODO check if release or debug and only activate in release so the console window doesn't appear
if(WIN32)
  target_link_options(${PROJECT_NAME} PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
endif()

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
  $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
  fmt::fmt
  SAIL::sail-c++
  toml11::toml11
)
