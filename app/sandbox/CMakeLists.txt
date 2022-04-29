cmake_minimum_required(VERSION 3.13)
project(sandbox)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(sandbox "")
add_subdirectory(src)

if (MSVC)
target_compile_options(sandbox
    PRIVATE
        $<$<OR:$<CONFIG:RelWithDebInfo>,$<CONFIG:Debug>>:/Z7>
)
endif()

target_link_libraries(sandbox
        PRIVATE
            Boost::date_time
            Boost::program_options
            CONAN_PKG::simdjson
            CONAN_PKG::openssl
            CONAN_PKG::certify
)

target_include_directories(sandbox
        PRIVATE
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

set_property(TARGET sandbox PROPERTY FOLDER "app")

install(
    TARGETS
        sandbox
    RUNTIME DESTINATION bin
)
