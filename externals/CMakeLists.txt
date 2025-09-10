
# Get nlohmann_json
find_package(nlohmann_json)
if(nlohmann_json_FOUND)
    message("Found nlohmann_json")
elseif(EXISTS "${PROJECT_SOURCE_DIR}/externals/json")
    message("Did not find nlohmann_json. We will compile submodule instead.")

    set(JSON_Install ON CACHE INTERNAL "")
    add_subdirectory(externals/json)
    install(TARGETS nlohmann_json EXPORT nudockTargets)
else()
    message("Did not find nlohmann_json locally. We will fetch it directly from github instead")

    # Adds json directory
    include(FetchContent)
    FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
    FetchContent_MakeAvailable(json)
    set(JSON_Install ON CACHE INTERNAL "")
    install(TARGETS nlohmann_json EXPORT nudockTargets)
endif(nlohmann_json_FOUND)

# Get cpp-httplib 
find_package(cpp-httplib)
if(cpp-httplib_FOUND)
    message("Found cpp-httplib")
#elseif(EXISTS "${PROJECT_SOURCE_DIR}/externals/cpp-httplib")
#    message("Did not find cpp-httplib. We will compile submodule instead.")
#
#    # Adds cpp-httplib directory
#    set(HTTPLIB_INSTALL OFF)
#    add_subdirectory(externals/cpp-httplib EXCLUDE_FROM_ALL)
#    set_property(TARGET httplib APPEND PROPERTY
#        INTERFACE_INCLUDE_DIRECTORIES $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
#    )
#    install(TARGETS httplib 
#        EXPORT nudockTargets
#        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/
#        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/
#        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/
#    )
else()
    message("Did not find cpp-httplib locally. We will fetch it directly from github instead")

    # Adds cpp-httplib directory
    include(FetchContent)
    FetchContent_Declare(httplib URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.20.0.tar.gz)
    set(HTTPLIB_USE_BROTLI_IF_AVAILABLE OFF)
    set(HTTPLIB_USE_ZLIB_IF_AVAILABLE OFF)
    set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF)
    set(HTTPLIB_USE_ZSTD_IF_AVAILABLE OFF)
    set(HTTPLIB_REQUIRE_BROTLI OFF)
    set(HTTPLIB_REQUIRE_OPENSSL OFF)
    set(HTTPLIB_REQUIRE_ZLIB OFF)
    set(HTTPLIB_REQUIRE_ZSTD OFF)
    FetchContent_MakeAvailable(httplib)
    #add_subdirectory(${cpp-httplib_SOURCE_DIR} ${cpp-httplib_BINARY_DIR} EXCLUDE_FROM_ALL)
    set_property(TARGET httplib APPEND PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    install(TARGETS httplib
        EXPORT nudockTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/
    )
endif(cpp-httplib_FOUND)

find_package(nlohmann_json_schema_validator)

if(nlohmann_json_schema_validator_FOUND)
    message("Found nlohmann_json_schema_validator")
elseif(EXISTS "${PROJECT_SOURCE_DIR}/externals/json-schema-validator")
    message("Did not find nlohmann_json_schema_validator. We will compile submodule instead.")  

    # Not actually installing it, we only need the headers
    set(JSON_VALIDATOR_INSTALL OFF)

    add_subdirectory(externals/json-schema-validator EXCLUDE_FROM_ALL)
    set_property(TARGET nlohmann_json_schema_validator APPEND PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>
    )
    install(TARGETS nlohmann_json_schema_validator
        EXPORT nudockTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/
    )
    install(FILES externals/json-schema-validator/src/nlohmann/json-schema.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nlohmann
    )
else()
    message("Did not find nlohmann_json_schema_validator locally. Use git clone --recurse-submodules to get it or we will fetch it directly from github instead")
endif(nlohmann_json_schema_validator_FOUND)
