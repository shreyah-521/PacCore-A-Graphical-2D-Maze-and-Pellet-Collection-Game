cmake_minimum_required(VERSION 3.20)

project(PacCore CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(raylib QUIET)

if(NOT raylib_FOUND)
    include(FetchContent)

    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.0
    )

    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(raylib)
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(PacCore ${SOURCES})

target_include_directories(PacCore PRIVATE include)

target_link_libraries(PacCore raylib)

add_custom_command(TARGET PacCore POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:PacCore>/assets
)
