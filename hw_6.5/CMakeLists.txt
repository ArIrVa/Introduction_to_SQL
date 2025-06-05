cmake_minimum_required(VERSION 3.20)
project(myproj)
set(CMAKE_CXX_STANDARD 17)
add_executable(myproj main.cpp)
add_subdirectory(libpqxx libpqxx-build)

target_link_libraries(myproj PRIVATE pqxx)




