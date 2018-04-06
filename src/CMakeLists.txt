# Add core library: rl_engine
file(GLOB_RECURSE SRC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
list(REMOVE_ITEM SRC_FILES "main.cpp")

add_library(rl_engine SHARED ${SRC_FILES})
target_link_libraries(rl_engine
    libyaml-cpp
    pthread)

# Add main executable: rl_markets
add_executable(rl_markets main.cpp)
target_link_libraries(rl_markets rl_engine)

# Include/Link dependencies:
find_package(Boost COMPONENTS program_options system filesystem REQUIRED)
if(${Boost_FOUND})
    include_directories(${Boost_INCLUDE_DIRS})

    target_link_libraries(rl_engine ${Boost_LIBRARIES})
endif(${Boost_FOUND})
