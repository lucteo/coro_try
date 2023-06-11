cmake_minimum_required(VERSION 3.17.0)

# Define the project
project(myproj-dev LANGUAGES CXX)

if (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup(TARGETS)
else ()
    message(ERROR "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif ()

# Set the version of the project
project(myproj-dev VERSION "${CONAN_PACKAGE_VERSION}")

# Print CMake configuration
message(STATUS "System           : ${CMAKE_SYSTEM}")
message(STATUS "System name      : ${CMAKE_SYSTEM_NAME}")
message(STATUS "System ver       : ${CMAKE_SYSTEM_VERSION}")
message(STATUS "Conan compiler   : ${CONAN_SETTINGS_COMPILER} ${CONAN_SETTINGS_COMPILER_VERSION}")
message(STATUS "Conan libcxx     : ${CONAN_SETTINGS_COMPILER_LIBCXX}")
message(STATUS)

# The main library
add_library(myproj INTERFACE)

# Set the version and current build date
set(myproj_VERSION "${PROJECT_VERSION}")
set(myproj_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
string(TIMESTAMP myproj_BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP myproj_BUILD_YEAR "%Y")

message(STATUS "Library ver      : ${myproj_VERSION}")
message(STATUS "Build date       : ${myproj_BUILD_DATE}")
message(STATUS "Build year       : ${myproj_BUILD_YEAR}")
message(STATUS)

# Set library version
set_target_properties(myproj PROPERTIES
                      VERSION "${myproj_VERSION}"
                      SOVERSION "${myproj_VERSION_MAJOR}")

# Declare the public include directories
target_include_directories(myproj INTERFACE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
                           $<INSTALL_INTERFACE:include>
                           )

# Ensure that we link with the threading library
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)
target_link_libraries(myproj INTERFACE Threads::Threads)
target_link_libraries(myproj INTERFACE CONAN_PKG::boost)
target_link_libraries(myproj INTERFACE CONAN_PKG::tracy-interface)

# Use C++20 standard
target_compile_features(myproj INTERFACE cxx_std_20)

# Use Tracy
target_compile_definitions(myproj INTERFACE USE_TRACY=1)

# Turn all warnings
target_compile_options(myproj INTERFACE
                       $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
                       -Wall>
                       $<$<CXX_COMPILER_ID:MSVC>:
                       /W4>)
# template backtrace limit
target_compile_options(myproj INTERFACE
                       $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
                       -ftemplate-backtrace-limit=0>
                       )
# Proper use of libc++
if (CONAN_SETTINGS_COMPILER_LIBCXX STREQUAL "libc++")
    target_compile_options(myproj INTERFACE -stdlib=libc++)
    target_link_options(myproj INTERFACE -lc++)
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
    add_compile_options(-Wno-non-template-friend)
    add_compile_options(-fcoroutines)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
endif ()

add_library(myproj::myproj ALIAS myproj)


add_executable(boostctx_threading_example async/profiling.cpp main.cpp)
target_link_libraries(boostctx_threading_example myproj)
target_link_libraries(boostctx_threading_example CONAN_PKG::boost)
target_link_libraries(boostctx_threading_example CONAN_PKG::tracy-interface)


