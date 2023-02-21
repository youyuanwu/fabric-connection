set(Boost_USE_STATIC_LIBS ON) # use static boost
find_package(Boost REQUIRED COMPONENTS unit_test_framework )

file(GLOB SOURCES
*_test.cpp
)

# file(GLOB_RECURSE HEADER_SOURCES
# ${CMAKE_SOURCE_DIR}/include/boost/winasio/http/*.hpp
# )

# message(STATUS "debug ${HEADER_SOURCES}")

# strip file extension
foreach(test_file ${SOURCES})
    get_filename_component(test_name ${test_file} NAME_WE)
    add_executable(${test_name} ${test_file} ${HEADER_SOURCES})
    target_include_directories(${test_name} 
      PRIVATE .
      ../src # private headers
    )
    
    #target_compile_definitions(${test_name} PRIVATE WINASIO_LOG) # enable logging
    
    target_link_libraries(${test_name} PRIVATE Boost::unit_test_framework Boost::disable_autolinking 
      fabric_connection
      fabric_internal_ext
      fabric_ext
      winasio
    )
    set_property(TARGET ${test_name} PROPERTY CXX_STANDARD 20) # for semaphore
    add_test(NAME ${test_name} COMMAND ${test_name} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endforeach()