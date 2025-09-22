cmake_minimum_required(VERSION 3.14)
project(docker-cpp VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force position independent code for all targets
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Use FetchContent for dependencies
include(FetchContent)

# Fetch RapidJSON (header-only library)
FetchContent_Declare(
    rapidjson
    GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
    GIT_TAG master
)

# Configure RapidJSON to not build anything except headers
set(RAPIDJSON_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_THIRDPARTY_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(rapidjson)

# Fetch and build libcurl
FetchContent_Declare(
    curl
    GIT_REPOSITORY https://github.com/curl/curl.git
    GIT_TAG curl-8_4_0
)

# Configure curl build options
set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CURL_STATICLIB ON CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(CURL_USE_OPENSSL ON CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(curl)

# Note: libcurl is an alias, the actual target is libcurl_static or CURL::libcurl
# We'll handle PIC through the global setting

# Source files
set(SOURCES docker.cpp)
set(HEADERS docker.h)

# Create shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Set library properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${HEADERS}"
)

# Include directories
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${rapidjson_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    INTERFACE
        $<BUILD_INTERFACE:${curl_SOURCE_DIR}/include>
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        libcurl
)

# Compiler flags (removed -fPIC since we set CMAKE_POSITION_INDEPENDENT_CODE globally)
# target_compile_options removed as it's handled by CMAKE_POSITION_INDEPENDENT_CODE

# Define CURL_STATICLIB for static linking
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        CURL_STATICLIB
)

# Create test executable
add_executable(test test.cpp)
target_link_libraries(test ${PROJECT_NAME})

# Add example subdirectory
add_subdirectory(example)

# Installation
include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Generate and install CMake config files
install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)