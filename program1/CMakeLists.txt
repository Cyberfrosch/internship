cmake_minimum_required(VERSION 3.2)

project(program1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost COMPONENTS locale REQUIRED)

if(Boost_FOUND)
    include_directories (SYSTEM ${Boost_INCLUDE_DIR})
else()
    message(STATUS "'Boost' cannot be found.")
endif()

include_directories(${Boost_INCLUDE_DIRS})
 
add_executable(program1 src/client.cpp src/client.h src/main.cpp)
 
target_link_libraries(program1 ${Boost_LIBRARIES})