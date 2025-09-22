cmake_minimum_required(VERSION 3.14)
project(docker-client-example)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the example executable
add_executable(docker-example main.cpp)

# Link against the docker client library
# If the library is installed system-wide:
# find_package(docker-cpp REQUIRED)
# target_link_libraries(docker-example docker-cpp::docker-cpp)

# For building as part of the main project:
target_link_libraries(docker-example docker-cpp)
target_include_directories(docker-example PRIVATE ${CMAKE_SOURCE_DIR})