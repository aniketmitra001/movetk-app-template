cmake_minimum_required(VERSION 3.11)

project(movetk-app
        LANGUAGES CXX
        VERSION 0.1
        )

# set path to custom cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
# Add OS specific tools.
if(UNIX)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/unix")
elseif(MSVC)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/msvc")
endif()

# include common tools and workarounds
include(tools)
# run cmake with -DCMAKE_PREFIX_PATH=path-to-installed-cmake-dir-of-movetk
find_package(movetk CONFIG REQUIRED)

find_package(Boost REQUIRED COMPONENTS iostreams log)
find_package(Threads REQUIRED)
find_package(GDAL 2.2 REQUIRED)
find_package(GeographicLib REQUIRED)
find_package(movetk REQUIRED)


message(STATUS "movetk_include ${movetk_INCLUDE_DIR}")

include_directories(movetk-app
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        ${GDAL_INCLUDE_DIR}
        ${GeographicLib_INCLUDE_DIR}
        ${movetk_INCLUDE_DIR})


add_executable(process_trajectories process_trajectories.cpp)

target_compile_definitions(process_trajectories PUBLIC -DBOOST_LOG_DYN_LINK=1
         -DWITH_HERE_PROBE=${WITH_HERE_PROBE})


target_link_libraries(process_trajectories
        PUBLIC
        Threads::Threads
        Boost::log
        Boost::iostreams
        ${GDAL_LIBRARY}
        ${GeographicLib_LIBRARIES}
        c2d::movetk)
