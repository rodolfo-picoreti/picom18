cmake_minimum_required(VERSION 3.0)
include(GNUInstallDirs)

find_package(is REQUIRED)
find_package(ismsgs REQUIRED) #fixme
find_package(prometheus-cpp REQUIRED) #fixme

add_executable(client.bin client.cpp)
target_link_libraries(client.bin PRIVATE is::is)
set_property(TARGET client.bin PROPERTY CXX_STANDARD 14)

add_executable(server.bin server.cpp)
target_link_libraries(server.bin PRIVATE is::is)
set_property(TARGET server.bin PROPERTY CXX_STANDARD 14)

install(
  TARGETS server.bin client.bin
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
