include(ExternalProject)
find_package(Git REQUIRED)

# Yaml-CPP
ExternalProject_Add(
    yaml-cpp
    PREFIX ${CMAKE_BINARY_DIR}/ext/yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    CMAKE_ARGS
        -DYAML_CPP_BUILD_TOOLS=False
        -DBUILD_SHARED_LIBS=ON
    )

add_library(libyaml-cpp IMPORTED SHARED GLOBAL)
add_dependencies(libyaml-cpp yaml-cpp)

ExternalProject_Get_Property(yaml-cpp source_dir binary_dir)
set_target_properties(libyaml-cpp PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/libyaml-cpp.so")

set(YAML_INCLUDE_DIR ${source_dir}/include CACHE INTERNAL "./ext/yaml-cpp/include")

# spdlog
ExternalProject_Add(
    spdlog
    PREFIX ${CMAKE_BINARY_DIR}/ext/spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    )

ExternalProject_Get_Property(spdlog source_dir)
set(SPDLOG_INCLUDE_DIR ${source_dir}/include CACHE INTERNAL "./ext/spdlog/include")

# Catch
ExternalProject_Add(
    catch
    PREFIX ${CMAKE_BINARY_DIR}/ext/catch
    GIT_REPOSITORY https://github.com/philsquared/Catch.git
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    )

ExternalProject_Get_Property(catch source_dir)
set(CATCH_INCLUDE_DIR ${source_dir}/single_include CACHE INTERNAL "./ext/catch/include")

##

function(create_syntastic_config)
    get_directory_property(DIRECTORIES INCLUDE_DIRECTORIES)
    list(REMOVE_DUPLICATES DIRECTORIES)

    set(CONFIG_FILE "${CMAKE_SOURCE_DIR}/.syntastic_cpp_config")
    file(REMOVE "${CONFIG_FILE}")

    foreach(DIRECTORY IN LISTS DIRECTORIES)
        file(APPEND "${CONFIG_FILE}" "-I${DIRECTORY}\n")
    endforeach()

    set(FLAGS ${CMAKE_CXX_FLAGS})
    separate_arguments(FLAGS)

    foreach(FLAG IN LISTS FLAGS)
        file(APPEND "${CONFIG_FILE}" "${FLAG}\n")
    endforeach()
endfunction()
