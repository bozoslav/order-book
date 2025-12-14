cmake_minimum_required(VERSION 3.10)

project(OrderBook)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

add_executable(OrderBook 
    src/main.cpp 
    src/OrderBook.cpp
)

#[[ for recording in instruments
if (APPLE) #cmake -DCMAKE_BUILD_TYPE=Debug ..
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(OrderBook PRIVATE -g)
    endif()

    find_program(DSYMUTIL_PROGRAM dsymutil)
    if (DSYMUTIL_PROGRAM)
        add_custom_command(
            TARGET OrderBook
            POST_BUILD
            # The command runs dsymutil on the newly built executable.
            COMMAND ${DSYMUTIL_PROGRAM} "$<TARGET_FILE:OrderBook>"
            COMMENT "Generating dSYM bundle for OrderBook"
        )
    endif()
endif()
]]#

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")