cmake_minimum_required(VERSION 3.0)

project(NuDock VERSION 0.0.1 LANGUAGES CXX)


include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Will create compile_commands.json for autocompleting in vim
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
include(${CMAKE_SOURCE_DIR}/externals/CMakeLists.txt)

add_library(nudock INTERFACE)

# Add the library version
target_compile_definitions(nudock
  INTERFACE
    VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    VERSION_MINOR=${PROJECT_VERSION_MINOR}
    VERSION_PATCH=${PROJECT_VERSION_PATCH}
    VERSION="${PROJECT_VERSION}"
)

# Include directories for nudock
target_include_directories(nudock
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(nudock
  INTERFACE
    nlohmann_json::nlohmann_json
    httplib::httplib
    nlohmann_json_schema_validator
)

# Link the schema dirs to the library
set(SCHEMAS_DIR "${CMAKE_INSTALL_INCLUDEDIR}/nudock/schemas")
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/nudock_config.hpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/nudock_config.hpp
  @ONLY
)
add_compile_definitions(SCHEMAS_DIR="${CMAKE_INSTALL_INCLUDEDIR}/nudock/schemas")
add_definitions(-DSCHEMAS_DIR="${SCHEMAS_DIR}")

# Install the headers
install(FILES nudock.hpp ${CMAKE_CURRENT_BINARY_DIR}/nudock_config.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nudock)

# Install should also copy the schemas folder with the json schemas
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/schemas/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nudock/schemas
  FILES_MATCHING PATTERN "*.json"
)

install(TARGETS nudock EXPORT nudockTargets)

install(EXPORT nudockTargets
  FILE NuDockTargets.cmake
  NAMESPACE NuDock::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nudock
)

# Make the install directory available for the tests
set(NuDock_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/cmake/nudock " CACHE PATH "NuDock CMake package directory" FORCE)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/NuDockConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/NuDockConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/NuDockConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nudock
)

# Install the config files
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/NuDockConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/NuDockConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nudock
)
