/**
 * @file nudock.hpp
 * 
 * @brief A small tool for server-client communication using JSON over HTTP.
 * 
 * @todo: Sort out the debugging define, should be more descriptive.
 * @todo: Add schema validation layers on the client side too.
 * @todo: The validation should be a compile-time option? Or better, templated. E.g. NuDock<true> for validation, NuDock<false> for no validation.
 * @todo: All functions should have documentation. More comments.
 * @todo: Refactor to have the http as an option, so we can use this as a library link directly between cross-compiled C++ code.
 * @todo: Remove hard-coded localhost and port, make them configurable. If configurable, could even run on a different machine!
 * @todo: Rethink abort(). 
 * @todo: The version defined in CMakefile should be used here.
 * @todo: Sort out the build system for the external packages!
 * 
 * @date 2025-07-01
 * @author Artur Sztuc (a.sztuc@ucl.ac.uk)
 */

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <memory>
#include <vector>

#include "nudock_config.hpp"

using nlohmann::json;
using nlohmann::json_schema::json_validator;
using HandlerFunction = std::function<json(const json&)>;

// Debugging macro to print debug messages with function name and line number
#define DEBUG() (this->m_debug_prefix + "::" + __func__ + "::L" + std::to_string(__LINE__) + " ")

// Macro to set an error response and stop the server
#define ERROR_RESPONSE(res, message) \
  res.status = 400; \
  res.set_content(message, "text/plain"); \
  m_server->stop(); \
  return;

// SchemaValidator struct to hold request and response validators along with the
// full schema
struct SchemaValidator {
  std::shared_ptr<json_validator> request_validator;
  std::shared_ptr<json_validator> response_validator;
  json schema;
};

// Custom error handler that throws exceptions on validation errors
class custom_throwing_error_handler : public nlohmann::json_schema::error_handler
{
	void error(const json::json_pointer &ptr, const json &instance, const std::string &message) override
	{
    //nlohmann::json_schema::basic_error_handler::error(ptr, instance, message);
    throw std::invalid_argument(std::string("Pointer: \"") + ptr.parent_pointer().to_string() + "\" instance: \"" + instance.dump() + "\"error message: \"" + message + "\"");
	}
};

class NuDock
{
  // Public member functions
  public:
    NuDock(bool _debug=true, 
           const std::string& _default_schemas_location=NUDOCK_SCHEMAS_DIR);

    /// @brief Server: responds to requests from the client
    void start_server();

    /// @brief Client: sends requests to the server and receives answers
    void start_client();

    /**
     * @brief Register the server's response handler for a specific request.
     * 
     * @param _request Request ID name
     * @param _handler_function Function to handle the request, takes json request and returns a json response
     * @param _schema_path Path of the schema file for the request and response validation.
     */
    void register_response(const std::string& _request_name, 
                           HandlerFunction _handler_function,
                           const std::string_view& _schema_path = "");

    /**
     * @brief Function for the client to send a request to the server.
     * 
     * @param _request Request ID name
     * @param _message json object with the request message
     * @return json object with the response from the server
     */
    json send_request(const std::string& _request_name,
                      const json& _message);

  // Private member functions
  private:
    /**
     * @brief Validates the server / client communication.
     * 
     * @param _message Message contains the version of the client, to be compared with the server's version.
     * @return true Communication successful
     * @return false Communication failed
     */
    bool validate_start(const json& _message);

    /**
     * @brief Loads json object from a given file path.
     * 
     * @param _path Path to the json file
     * @return json Json object loaded from the file
     */
    json load_json_file(const std::string_view& _path);

  // Private member data
  private:
    /// @brief version of the server/client.
    /// @todo Make this a compile-time constant, or read from a file.
    std::string m_version = NUDOCK_VERSION;

    /// @brief server object with external experiment
    std::unique_ptr<httplib::Server> m_server;

    /// @brief client requesting responses from external experiment
    std::unique_ptr<httplib::Client> m_client;

    /// @brief map of request names to their handler functions
    std::unordered_map<std::string, HandlerFunction> m_request_handlers;

    /// @brief map of request names to their schema validators
    std::unordered_map<std::string, SchemaValidator> m_schema_validator;

    /// @brief whether we want to print debug messages
    bool m_debug;

    /// @brief string prefix for debugging messages
    std::string m_debug_prefix;

    std::string m_default_schemas_location;

    /// @brief custom error handler for json validation
    custom_throwing_error_handler m_err;

    /// @brief Counter for the number of requests sent / processed
    uint64_t m_request_counter = 0;
};