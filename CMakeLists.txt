cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

# Rename `template_cpp17` to your project name (e.g. `my_project`)
project(zennube LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)

add_subdirectory(external/sdl2)
add_subdirectory(external/sdl2_image)

file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

find_package(SDL2 REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(SailC++ CONFIG REQUIRED)


target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} fmt::fmt SAIL::sail-c++)
