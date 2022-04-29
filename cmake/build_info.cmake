# PRINT BUILD INFORMATION
message(STATUS "\nConfiguring build in: ${CMAKE_BINARY_DIR}")

status("")
status("Build configuration for Fuel3D project ========================================")

	if(WIN32)
		set(_username $ENV{USERNAME})
	else()
		set(_username $ENV{USER})
	endif()

# ========================== version control ==========================

status("")
status("  Version control:")
status("    Fuel3D Package Version:" "${F3D_PACKAGE_VERSION}")
status("    branch:" "${_GIT_BRANCH_NAME}")
status("    revision:" "${_GIT_COMMIT_HASH}")
status("    local changes:" "${IS_DIRTY}")
status("    CI build ID:" "${CI_BUILD_ID}")
status("    CI build number:" "${CI_BUILD_NUMBER}")

# ========================== build platform ==========================
status("")
status("  Platform:")
if(NOT CMAKE_VERSION VERSION_LESS 2.8.11 AND NOT BUILD_INFO_SKIP_TIMESTAMP)
  string(TIMESTAMP TIMESTAMP "" UTC)
  if(TIMESTAMP)
    status("    Timestamp:"    ${TIMESTAMP})
  endif()
endif()
status("    Host:"             ${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_VERSION} ${CMAKE_HOST_SYSTEM_PROCESSOR} (${_username}))
if(CMAKE_CROSSCOMPILING)
  status("    Target:"         ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION} ${CMAKE_SYSTEM_PROCESSOR})
endif()
status("    CMake:"            ${CMAKE_VERSION})
status("    CMake generator:"  ${CMAKE_GENERATOR})
status("    CMake build tool:" ${CMAKE_BUILD_TOOL})
if(MSVC)
  status("    MSVC:"           ${MSVC_VERSION})
endif()
if(CMAKE_GENERATOR MATCHES Xcode)
  status("    Xcode:"          ${XCODE_VERSION})
endif()
if(NOT CMAKE_GENERATOR MATCHES "Xcode|Visual Studio")
  status("    Configuration:"  ${CMAKE_BUILD_TYPE})
endif()

# ========================== C/C++ options ==========================
if(CMAKE_CXX_COMPILER_VERSION)
  set(F3D_COMPILER_STR "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} (ver ${CMAKE_CXX_COMPILER_VERSION})")
elseif(CMAKE_COMPILER_IS_CLANGCXX)
  set(F3D_COMPILER_STR "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} (ver ${CMAKE_CLANG_REGEX_VERSION})")
elseif(CMAKE_COMPILER_IS_GNUCXX)
  set(F3D_COMPILER_STR "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} (ver ${CMAKE_GCC_REGEX_VERSION})")
else()
  set(F3D_COMPILER_STR "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}")
endif()
string(STRIP "${F3D_COMPILER_STR}" F3D_COMPILER_STR)

status("")
status("  C/C++:")
status("    C++ Compiler:"           ${F3D_COMPILER_STR})
status("    C++ flags (Release):"    ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE})
status("    C++ flags (RelWithDebInfo):"    ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
status("    C++ flags (Debug):"      ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG})
if(CMAKE_COMPILER_IS_GNUCC)
  status("    C++ flags (Coverage):"   ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_COVERAGE})
endif()
if(WIN32)
  status("    Linker flags (Release):" ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_RELEASE})
  status("    Linker flags (Debug):"   ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
else()
  status("    Linker flags (Release):" ${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS_RELEASE})
  status("    Linker flags (Debug):"   ${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
endif()

status("")
status("  CUDA:")
status("    CUDA Compiler:"                   ${F3D_CUDA_COMPILER})
status("    CUDA Compiler Version:"           ${F3D_CUDA_COMPILER_VERSION})
status("")
status("  Options:")
status("    Python Bindings:"                 ${F3D_PYTHON_BINDINGS})