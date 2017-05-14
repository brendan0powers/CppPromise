cmake_minimum_required(VERSION 3.4)

project(CppPromise)

#global project settings
set(CMAKE_INCLUDE_CURRENT_DIR ON)   #Ensure includes from the current dir work
set(CMAKE_AUTOMOC ON)               #handle Qt moc automatically
set(CMAKE_CXX_STANDARD 11)          #C++11 support is requried
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5Core)
find_package(Qt5Network)

include_directories(include)

#Pull in the Catch library. It's a header only library, so checking
#out the it repo is enough
include(ExternalProject)
ExternalProject_Add(CatchLibExt
    PREFIX ${CMAKE_BINARY_DIR}/ext/Catch
    GIT_REPOSITORY https://github.com/philsquared/Catch.git
    GIT_TAG v1.9.3
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    )
add_library(CatchLib INTERFACE)
add_dependencies(CatchLib CatchLibExt)
target_include_directories(CatchLib INTERFACE ${CMAKE_BINARY_DIR}/ext/Catch/src/CatchLibExt/single_include/)


set(SOURCES
    tests/main.cpp
    tests/PromiseHttp.cpp
    tests/PromiseHttp.h
    include/Promise.h
    )
add_executable(TestApp WIN32 ${SOURCES})
target_link_libraries(TestApp Qt5::Core Qt5::Network CatchLib)
