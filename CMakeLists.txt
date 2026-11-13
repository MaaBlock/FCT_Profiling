cmake_minimum_required(VERSION 3.22.1)
project(FCT_Profiling LANGUAGES CXX)

include(platform/platform.cmake)
include(config/copy_headers.cmake)
include(config/set_utf8_compiler_encoding.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(DEFINED FCT_ENABLE_TRACY AND NOT DEFINED FCT_ENABLE_PROFILING)
    option(FCT_ENABLE_PROFILING "Enable FCT_Profiling (Tracy backend)" "${FCT_ENABLE_TRACY}")
else()
    option(FCT_ENABLE_PROFILING "Enable FCT_Profiling (Tracy backend)" ON)
endif()
set(FCT_ENABLE_TRACY "${FCT_ENABLE_PROFILING}" CACHE BOOL "Legacy alias for FCT_ENABLE_PROFILING" FORCE)
option(FCT_TRACY_ON_DEMAND "Enable on-demand profiling for Tracy" ON)

if(FCT_ENABLE_PROFILING)
    set(TRACY_ENABLE ON CACHE BOOL "Enable profiling" FORCE)
    if(FCT_TRACY_ON_DEMAND)
        set(TRACY_ON_DEMAND ON CACHE BOOL "On-demand profiling" FORCE)
    else()
        set(TRACY_ON_DEMAND OFF CACHE BOOL "On-demand profiling" FORCE)
    endif()
    set(TRACY_STATIC OFF CACHE BOOL "Build Tracy as shared library" FORCE)

    if(NOT TARGET TracyClient)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/tracy/CMakeLists.txt")
            add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/tracy" "${CMAKE_CURRENT_BINARY_DIR}/tracy")
        else()
            include(FetchContent)
            FetchContent_Declare(
                tracy
                GIT_REPOSITORY https://github.com/MaaBlock/tracy.git
                GIT_TAG master
            )
            FetchContent_MakeAvailable(tracy)
        endif()
    endif()
endif()

if(NOT TARGET FCT_Base)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../FCT_Base/CMakeLists.txt")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../FCT_Base" "${CMAKE_CURRENT_BINARY_DIR}/FCT_Base")
    else()
        message(FATAL_ERROR "FCT_Profiling requires FCT_Base target")
    endif()
endif()

file(GLOB_RECURSE FCT_PROFILING_HEADERS
    LIST_DIRECTORIES false
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)
list(REMOVE_ITEM FCT_PROFILING_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/FCT_Profiling.h")

file(GLOB_RECURSE FCT_PROFILING_SOURCES
    LIST_DIRECTORIES false
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(FCT_Profiling STATIC
    ${FCT_PROFILING_SOURCES}
    ${FCT_PROFILING_HEADERS}
)

set_utf8_for_target(FCT_Profiling)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/FCT_Profiling")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/FCT_Profiling.h")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/FCT_Profiling.h" "${CMAKE_CURRENT_SOURCE_DIR}/include/FCT_Profiling.h" COPYONLY)
endif()

foreach(header_file ${FCT_PROFILING_HEADERS})
    file(RELATIVE_PATH relative_path "${CMAKE_CURRENT_SOURCE_DIR}/src" "${header_file}")
    get_filename_component(header_dir "${relative_path}" DIRECTORY)
    if(header_dir)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/FCT_Profiling/${header_dir}")
    endif()
    configure_file("${header_file}" "${CMAKE_CURRENT_SOURCE_DIR}/include/FCT_Profiling/${relative_path}" COPYONLY)
endforeach()

add_copy_target_headers(FCT_Profiling "${FCT_PROFILING_HEADERS}")

target_include_directories(FCT_Profiling PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(FCT_Profiling PUBLIC FCT_Base)

if(FCT_ENABLE_PROFILING)
    target_link_libraries(FCT_Profiling PUBLIC Tracy::TracyClient)
    target_compile_definitions(FCT_Profiling PUBLIC FCT_PROFILING_ENABLED=1)
else()
    target_compile_definitions(FCT_Profiling PUBLIC FCT_PROFILING_ENABLED=0)
endif()

function(target_link_fct_profiling_libraries target_name)
    if(NOT TARGET ${target_name})
        message(FATAL_ERROR "Target ${target_name} does not exist")
    endif()

    target_link_libraries(${target_name} FCT_Profiling)

    if(WIN32 AND FCT_ENABLE_PROFILING AND TARGET TracyClient AND NOT TRACY_STATIC)
        get_target_property(_target_type ${target_name} TYPE)
        if(_target_type STREQUAL "EXECUTABLE" OR _target_type STREQUAL "SHARED_LIBRARY")
            get_target_property(_already_copied ${target_name} _FCT_PROFILING_DLL_COPIED)
            if(NOT _already_copied)
                set_target_properties(${target_name} PROPERTIES _FCT_PROFILING_DLL_COPIED TRUE)
                add_custom_command(TARGET ${target_name} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:TracyClient>
                    $<TARGET_FILE_DIR:${target_name}>
                    COMMENT "Copying TracyClient.dll for ${target_name}"
                )
            endif()
        endif()
    endif()
endfunction()
