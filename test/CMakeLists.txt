file(GLOB_RECURSE TEST_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

include_directories(${CATCH_INCLUDE_DIR})

# add_catch_test(<target> <sources>...)
function(add_catch_test target)
    add_executable(${target} main.cpp ${ARGN})
    target_link_libraries(${target}
        rl_engine)

    add_test(${target} ${CMAKE_BINARY_DIR}/${target})
endfunction()

add_catch_test(do_tests ${TEST_FILES})
