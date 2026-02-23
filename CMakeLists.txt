cmake_minimum_required(VERSION 3.24)

project(fastoml-cpp LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(FASTOML_CPP_BUILD_EXAMPLES "Build fastoml-cpp examples" ON)

include(FetchContent)

set(FASTOML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FASTOML_BENCH OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  fastoml
  GIT_REPOSITORY https://github.com/EldoDebug/fastoml.git
  GIT_TAG 0a6c7d767b529afc7bc21f70fc9d0724d8b750cf
)
FetchContent_MakeAvailable(fastoml)

file(
  GLOB FASTOML_CPP_HEADERS
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

file(
  GLOB_RECURSE FASTOML_CPP_SOURCES
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

add_library(fastoml-cpp ${FASTOML_CPP_HEADERS} ${FASTOML_CPP_SOURCES})
add_library(fastoml-cpp::fastoml-cpp ALIAS fastoml-cpp)

target_link_libraries(fastoml-cpp PUBLIC fastoml::fastoml)
target_include_directories(fastoml-cpp PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_features(fastoml-cpp PUBLIC cxx_std_23)

if(FASTOML_CPP_BUILD_EXAMPLES)
  add_subdirectory(example)
endif()
