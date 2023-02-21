set (CMAKE_CXX_STANDARD 20)

# connection lib
add_library(fabric_connection STATIC
  connection.cpp
  ../include/fabricconnection/connection.hpp
  listener.cpp
  listener.hpp
  namedpipe_server.hpp
  namedpipe_server.cpp
  listenerinternal.hpp
  chunk.cpp
  chunk.hpp
  msgconv.cpp
  msgconv.hpp
)

target_include_directories(fabric_connection 
  PUBLIC ../include
  PRIVATE .
)

target_link_libraries(fabric_connection PUBLIC 
  # FabricTransport 
  fabric_internal_headers # requires fabrictransport.h
  PRIVATE winasio
  fabric_ext
  fabric_internal_ext
)

# asio winsock headers requires this
target_compile_definitions(fabric_connection
  PUBLIC WIN32_LEAN_AND_MEAN=1
)