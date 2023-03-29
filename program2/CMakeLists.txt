cmake_minimum_required(VERSION 3.2)

project(program2)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED)

if(Boost_FOUND)
    include_directories (SYSTEM ${Boost_INCLUDE_DIR})
else()
    message(STATUS "'Boost' cannot be found.")
endif()

include_directories(${Boost_INCLUDE_DIRS})
 
add_executable(program2 src/server.cpp src/server.h src/main.cpp)
 
target_link_libraries(program2 ${Boost_LIBRARIES})