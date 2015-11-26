cmake_minimum_required(VERSION 3.3)
project(Mafia)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp network.cpp network.h Player.cpp Player.h Room.cpp Room.h)
add_executable(Mafia ${SOURCE_FILES})

set(Boost_USE_MULTITHREADED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

find_package(Boost COMPONENTS thread date_time filesystem system REQUIRED)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()

if (Boost_FOUND)
    message(status "Boost found!")
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(Mafia ${Boost_LIBRARIES})
endif()