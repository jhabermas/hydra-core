############################################################
#
# Generate the repo hash, local machine and timestamp info at compile time.
# Store the info in a header that can be included in the executable code.
#
############################################################

include("GetGitRevisionDescription")
get_git_head_revision(_GIT_BRANCH_NAME _GIT_COMMIT_HASH)
git_commit_height(_GIT_COMMIT_HEIGHT)
git_local_changes(_GIT_LOCAL_CHANGES)
if(_GIT_LOCAL_CHANGES STREQUAL "CLEAN")
    set(IS_DIRTY "false")
else()
    set(IS_DIRTY "true")
endif()

function(GET_F3D_VERSION _F3D_VERSION_OUT)
    if (DEFINED ENV{F3D_PACKAGE_VERSION})
        set(${_F3D_VERSION_OUT} $ENV{F3D_PACKAGE_VERSION} PARENT_SCOPE)
        message(STATUS "Using version from environment variable")
    else()
        find_package(Python REQUIRED COMPONENTS Interpreter)
        execute_process(
            COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/devops/scripts/fuel3d.py --working-folder ${CMAKE_SOURCE_DIR} version
            OUTPUT_VARIABLE _F3D_VERSION
            ERROR_VARIABLE VERSION_ERROR
        )
        if (VERSION_ERROR)
            message(STATUS "Error getting version from Python/Git: ${VERSION_ERROR}. using default")
            set(${_F3D_VERSION_OUT} "0.0.0-dev" PARENT_SCOPE)
        else()

            message(STATUS "Using computed version from Python/Git ${_F3D_PACKAGE_VERSION}")
            string(STRIP ${_F3D_VERSION} _F3D_PACKAGE_VERSION)
            set(${_F3D_VERSION_OUT} ${_F3D_PACKAGE_VERSION} PARENT_SCOPE)
        endif()
    endif()
endfunction()

get_f3d_version(F3D_PACKAGE_VERSION)
# CI Build Versions
SET(CI_BUILD_ID "development build" CACHE STRING "Build ID Set from the CI Pipeline")
SET(CI_BUILD_NUMBER "development build" CACHE STRING "Build Number Set from the CI Pipeline")
STRING(TIMESTAMP TIMESTAMP "%s" UTC)
STRING(SUBSTRING ${_GIT_COMMIT_HASH} 0 7 _GIT_COMMIT_HASH_ABBREV)
SET(VERSION_FILENAME "${CMAKE_SOURCE_DIR}/include/hydra/version.h")
if (TARGET f3d_compute_cuda)
    set(F3D_WITH_CUDA "true")
else()
    set(F3D_WITH_CUDA "false")
endif()
