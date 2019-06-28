cmake_minimum_required(VERSION 2.6)

project(lockfree)

set(CMAKE_CXX_COMPILER g++)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/test_cases
    )

#add_subdirectory("${PROJECT_SOURCE_DIR}/include")
add_subdirectory(${PROJECT_SOURCE_DIR}/test_cases)

set(CMAKE_CXX_FLAGS " -std=c++11 -Wformat=0 ")

set(SRC_LIST "lockfree.cpp")

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} gtest pthread)