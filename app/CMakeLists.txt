cmake_minimum_required(VERSION 3.10)

# project 名 = 目录名，保持一致即可
get_filename_component(CURRENT_DIR_PATH "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)
get_filename_component(CURRENT_DIR_NAME "${CURRENT_DIR_PATH}" NAME)
project(${CURRENT_DIR_NAME})

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)

file(GLOB_RECURSE CPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE HDR_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_executable(${PROJECT_NAME} WIN32 ${CPP_FILES} ${HDR_FILES})
target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Gui Qt5::Core)
