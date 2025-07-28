cmake_minimum_required(VERSION 3.0)

project(NuDock VERSION 0.0.1 LANGUAGES CXX)


include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Will create compile_commands.json for autocompleting in vim
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
#include(${CMAKE_SOURCE_DIR}/externals/CMakeLists.txt)

add_library(nudock STATIC 
  nudock.cpp
)

target_compile_definitions(nudock
  PUBLIC
    VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    VERSION_MINOR=${PROJECT_VERSION_MINOR}
    VERSION_PATCH=${PROJECT_VERSION_PATCH}
    VERSION="${PROJECT_VERSION}"
)

target_include_directories(nudock
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/externals/json-schema-validator/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/externals/json/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/externals/cpp-httplib>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
)

set(SCHEMAS_DIR "${CMAKE_INSTALL_INCLUDEDIR}/nudock/schemas")
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/nudock_config.hpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/nudock_config.hpp
  @ONLY
)
add_compile_definitions(SCHEMAS_DIR="${CMAKE_INSTALL_INCLUDEDIR}/nudock/schemas")
add_definitions(-DSCHEMAS_DIR="${SCHEMAS_DIR}")

install(FILES nudock.hpp ${CMAKE_CURRENT_BINARY_DIR}/nudock_config.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nudock)

# Install should also move the schemas folder with the json schemas
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/schemas/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nudock/schemas
  FILES_MATCHING PATTERN "*.json"
)

install(TARGETS nudock EXPORT nudockTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT nudockTargets
  FILE NuDockTargets.cmake
  NAMESPACE NuDock::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NuDock
)

## Make the install directory available for the tests
set(NuDock_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/cmake/NuDock" CACHE PATH "NuDock CMake package directory" FORCE)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/NuDockConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/NuDockConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/NuDockConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NuDock
)

# Install the config files
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/NuDockConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/NuDockConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NuDock
)