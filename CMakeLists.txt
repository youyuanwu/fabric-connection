cmake_minimum_required(VERSION 3.14)

project(fabric-connection)

if(NOT fabric_connection_LocalDevMode)
  # import winext
  message(STATUS "feching winasio")
  include(FetchContent)
  FetchContent_Declare(winasio
    GIT_REPOSITORY https://github.com/youyuanwu/winasio.git
    GIT_TAG ebde72a3f8aaa72bc90b917ec495017470f0f888)
  FetchContent_GetProperties(winasio)
  if(NOT winasio_POPULATED)
    FetchContent_Populate(winasio)
    # add subdir is outside and after fmt targets.
  endif()
else()
  # winasio repo must be located parallel to wingrpc
  message(STATUS "Using winasio LocalDevMode. This should not be used in CI.")
  set(winasio_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../winasio)
  set(winasio_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/local/winasio)
endif(NOT fabric_connection_LocalDevMode)

# format
include(${winasio_SOURCE_DIR}/cmake/clang-format.cmake)
if(WINASIO_CI_FORMAT)
    message(STATUS "Only added format target.")
    return()
endif()

add_subdirectory(${winasio_SOURCE_DIR} ${winasio_BINARY_DIR} EXCLUDE_FROM_ALL)

message(STATUS "fetching service-fabric-cpp")

include(FetchContent)
FetchContent_Declare(service_fabric_cpp
    GIT_REPOSITORY https://github.com/youyuanwu/service-fabric-cpp.git
    GIT_TAG 7c88a7023969189c21ab43933f1d904961df4d2e
)
FetchContent_GetProperties(service_fabric_cpp)
if(NOT service_fabric_cpp_POPULATED)
    FetchContent_Populate(service_fabric_cpp)
    add_subdirectory(${service_fabric_cpp_SOURCE_DIR} ${service_fabric_cpp_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)

# coverage
find_program(OpenCppCoverage_exe
    NAMES OpenCppCoverage.exe
)
if(OpenCppCoverage_exe)
    message(STATUS "coverage tool found: ${OpenCppCoverage_exe}")
    # coverage tool only recognizes windows style path, backslash.
    file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" PWD_WIN_PATH)
    add_custom_target(coverage
        COMMAND ${OpenCppCoverage_exe} --quiet --export_type cobertura:cobertura.xml --cover_children 
        --sources "${PWD_WIN_PATH}\\src" --sources "${PWD_WIN_PATH}\\include" --modules "${PWD_WIN_PATH}"
        -- ctest -C Debug --test-dir build --repeat until-pass:3 --timeout 30
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
else()
    message(STATUS "coverage tool not found: ${OpenCppCoverage_exe}")
endif()
