cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW) # The project command manages VERSION variables
endif()
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW) # find_package uses PackageName_ROOT variables.
endif()
if(POLICY CMP0076)
    cmake_policy(SET CMP0076 NEW) # target_sources uses relative paths
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}")
include(cmake/macros/build_info_functions.cmake)

project(Hydra VERSION 0.0.1)

string(REPLACE "." "," PROJECT_VERSION_RC ${PROJECT_VERSION})
if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
else()
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
endif()

conan_basic_setup(TARGETS)

add_definitions("-DBOOST_ALL_DYN_LINK")
add_definitions("-DBOOST_ALL_NO_LIB")

if(WIN32)
  # ensure Unicode friendly APIs are used on Windows
  add_definitions(-DUNICODE -D_UNICODE)

  # minimize windows.h content and fix compilation errors with C++17
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  add_definitions(/bigobj)
endif()

if (MSVC)
  # Use embedded debug info rather than PDB files
  # Use standard C++ exception handling
  set(HYDRA_CXX_MSVC_FLAGS /W4 /EHsc)
  add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${HYDRA_CXX_MSVC_FLAGS}>")
  add_compile_options($<$<AND:$<COMPILE_LANGUAGE:CXX>,$<OR:$<CONFIG:RelWithDebInfo>,$<CONFIG:Debug>>>:/Z7>)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
  set(HYDRA_CXX_ALL_WARNINGS -Wall -Wextra -Wconversion -pedantic)
  add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${HYDRA_CXX_ALL_WARNINGS}>")
  # Debug visualisation for LLVM libc++ strings
  add_compile_options($<$<AND:$<COMPILE_LANGUAGE:CXX>,$<OR:$<CONFIG:RelWithDebInfo>,$<CONFIG:Debug>>>:-fstandalone-debug>)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_DEBUG_POSTFIX _d)

set(HYDRA_TEST_OUTPUT_DIR ${CMAKE_BINARY_DIR}/test_results CACHE PATH "Output folder for test reports")

# ensure test output dir has a trailing slash for gtest commad line
set(HYDRA_TEST_OUTPUT_DIR "${HYDRA_TEST_OUTPUT_DIR}/")
file(MAKE_DIRECTORY ${HYDRA_TEST_OUTPUT_DIR})

find_package(Boost REQUIRED
        log
        program_options
)

add_subdirectory(src)
add_subdirectory(app)

include(cmake/compileTimeVersionInfo.cmake)
include(cmake/build_info.cmake)
include(CMakePackageConfigHelpers)

STRING(TIMESTAMP TIMESTAMP "%s" UTC)
configure_file(
    "${CMAKE_SOURCE_DIR}/src/version.h.in"
    "${VERSION_FILENAME}"
    @ONLY)
message( STATUS "GIT version info written to: ${VERSION_FILENAME}")

configure_package_config_file (cmake/HydraConfig.cmake.in HydraConfig.cmake
  INSTALL_DESTINATION .)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/HydraConfigVersion.cmake
    COMPATIBILITY AnyNewerVersion
)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/HydraConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/HydraConfigVersion.cmake
    DESTINATION lib/cmake/hydra
)
