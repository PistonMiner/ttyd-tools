cmake_minimum_required(VERSION 3.15)
project(elf2rel)

set(CMAKE_CXX_STANDARD 17)

find_package( Boost REQUIRED COMPONENTS program_options )
include_directories( ${Boost_INCLUDE_DIRS} )

include_directories(.)
include_directories(elfio)

add_executable(elf2rel
    elf2rel.cpp)
    
target_link_libraries(elf2rel ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_REGEX_LIBRARY})

