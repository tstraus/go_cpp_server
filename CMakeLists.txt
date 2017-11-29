cmake_minimum_required(VERSION 3.0)
project(go_cpp_server)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(Tcmalloc)
find_package(Threads)
find_package(OpenSSL)

if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

if (MSVC)
    set(Boost_USE_STATIC_LIBS "On")
    find_package( Boost 1.52 COMPONENTS system thread regex REQUIRED )
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y -pedantic -Wextra")
    find_package( Boost 1.52 COMPONENTS system thread REQUIRED )
endif()

include_directories(
    ${Boost_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include
)

set(SOURCE
    source/main.cpp
)

add_executable(${PROJECT_NAME}
    ${SOURCE}
)

target_link_libraries(${PROJECT_NAME}
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)