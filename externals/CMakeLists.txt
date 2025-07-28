
# Get nlohmann_json
find_package(nlohmann_json)
if(nlohmann_json_FOUND)
    message("Found nlohmann_json")
elseif(EXISTS "${PROJECT_SOURCE_DIR}/externals/json")
    message("Did not find nlohmann_json. We will compile submodule instead.")

    set(JSON_Install ON CACHE INTERNAL "")
    add_subdirectory(externals/json)
    install(TARGETS nlohmann_json::nlohmann_json EXPORT nudockTargets)
else()
    message("Did not find nlohmann_json locally. We will fetch it directly from github instead")

    # Adds json directory
    include(FetchContent)
    FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
    FetchContent_MakeAvailable(json)
    set(JSON_Install ON CACHE INTERNAL "")
    install(TARGETS nlohmann_json::nlohmann_json EXPORT nudockTargets)
endif(nlohmann_json_FOUND)

# Get cpp-httplib 
find_package(cpp-httplib)
if(cpp-httplib_FOUND)
    message("Found cpp-httplib")
elseif(EXISTS "${PROJECT_SOURCE_DIR}/externals/cpp-httplib")
    message("Did not find cpp-httplib. We will compile submodule instead.")

    # Adds cpp-httplib directory
    add_subdirectory(externals/cpp-httplib)
    install(TARGETS httplib EXPORT nudockTargets)
else()
    message("Did not find cpp-httplib locally. We will fetch it directly from github instead")

    # Adds cpp-httplib directory
    include(FetchContent)
    FetchContent_Declare(cpp-httplib URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.20.0.tar.gz)
    FetchContent_MakeAvailable(cpp-httplib)
    install(TARGETS httplib EXPORT nudockTargets)
endif(cpp-httplib_FOUND)

find_package(nlohmann_json_schema_validator)

if(nlohmann_json_schema_validator_FOUND)
    message("Found nlohmann_json_schema_validator")
elseif(EXISTS "${PROJECT_SOURCE_DIR}/externals/json-schema-validator")
    message("Did not find nlohmann_json_schema_validator. We will compile submodule instead.")  

    # Not actually installing it, we only need the headers
    add_subdirectory(externals/json-schema-validator EXCLUDE_FROM_ALL)
    set(JSON_VALIDATOR_INSTALL ON CACHE INTERNAL "")
    set(JSON_VALIDATOR_SHARED_LIBS ON)
else()
    message("Did not find nlohmann_json_schema_validator locally. We will fetch it directly from github instead")   

    # Adds json-schema-validator directory
    include(FetchContent)
    FetchContent_Declare(json-schema-validator URL https://github.com/pboettch/json-schema-validator/archive/refs/tags/2.3.0.tar.gz)
    FetchContent_MakeAvailable(json-schema-validator)
    set(JSON_VALIDATOR_INSTALL ON CACHE INTERNAL "")
    set(JSON_VALIDATOR_SHARED_LIBS ON)
endif(nlohmann_json_schema_validator_FOUND)
