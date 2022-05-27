cmake_minimum_required(VERSION 3.0)

project(optionalcpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)

add_executable(test_${PROJECT_NAME} tests/tests.cpp)
target_link_libraries(test_${PROJECT_NAME} ${PROJECT_NAME})

target_include_directories(test_${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/submodules/Catch2/include
)
