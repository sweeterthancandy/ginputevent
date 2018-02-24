
cmake_minimum_required(VERSION 3.7)

project(ginputevent)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-std=c++14")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-private-field")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-braced-scalar-init")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-field-initializers")

find_package(Boost REQUIRED system filesystem COMPONENTS)

include_directories(${Boost_INCLUDE_DIRS})
include_directories( include )



aux_source_directory(src lib_src)
add_library(ginput SHARED ${lib_src}) 
target_link_libraries( ginput pthread ${Boost_LIBRARIES} )

add_executable( linux_event_debugger app/linux_events.cpp)
target_link_libraries( linux_event_debugger ginput  pthread ${Boost_LIBRARIES})

add_executable( TestMouse  app/TestMouse.cpp)
target_link_libraries( TestMouse ginput pthread ${Boost_LIBRARIES})

add_executable( TestKeyboard  app/TestKeyboard.cpp)
target_link_libraries( TestKeyboard ginput pthread ${Boost_LIBRARIES})


