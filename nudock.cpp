#include "nudock.hpp"

NuDock::NuDock(bool _debug, const std::string& _default_schemas_location)
  : m_debug(_debug), m_default_schemas_location(_default_schemas_location)
{
}

json NuDock::load_json_file(const std::string_view& _path)
{
  std::ifstream file(_path.data());
  if (!file.is_open()) {
    throw std::runtime_error("Could not open file: " + std::string(_path));
  }

  json j;
  try {
    j = json::parse(file);
  } catch (const nlohmann::json::parse_error& e) {
    throw std::runtime_error("JSON parsing error: " + std::string(e.what()));
  }

  return j;
}

void NuDock::register_response(const std::string& _request,
                               HandlerFunction _handler_function,
                               const std::string_view& _schema_path)
{
  std::string schema_path = _schema_path.empty() ? m_default_schemas_location + _request + ".schema.json" : std::string(_schema_path);

  // Check if the request name is valid
  if (_request.empty()) {
    std::cerr << DEBUG() << "Request name is empty!" << std::endl;
    return;
  }
  if (m_request_handlers.count(_request)) {
    std::cerr << DEBUG() << "Request handler for \"" << _request << "\" already exists!" << std::endl;
    return;
  }
  if (_schema_path.empty()) {
    std::cerr << DEBUG() << "Schema path is empty!" << std::endl;
    return;
  }

  // Create the schema validator for a specific request
  json schema = load_json_file(_schema_path);
  SchemaValidator validator;
  validator.schema = schema["properties"];

  // Request validator
  validator.request_validator = std::make_shared<json_validator>();
  validator.request_validator->set_root_schema(schema["properties"]["request"]);

  // Response validator
  validator.response_validator = std::make_shared<json_validator>();
  validator.response_validator->set_root_schema(schema["properties"]["response"]);

  // Move the validator to the internally held map
  m_schema_validator[_request] = std::move(validator);

  // Add the request handler function
  m_request_handlers[_request] = std::move(_handler_function);
}

bool NuDock::validate_start(const json& _message)
{
  if (!_message.contains("version")) {
    std::cerr << DEBUG() << "Received /validate_start request without provided \"version\" entry! We will crash. Full request received:" << std::endl;
    std::cerr << DEBUG() << _message.dump() << std::endl;
    return false;
  }

  if (_message["version"] != m_version) {
    std::cerr << DEBUG() << "Received request from client with version: " << _message["version"] << ", this server's version is " << m_version << std::endl;
    return false;
  }
  else {
    std::cout << "Internal version: " << m_version << " external version: " << _message["version"] << std::endl;
  }

  return true;
}

void NuDock::start_server()
{
  m_version = "0.0.1";
  if (m_client || m_server) {
    std::cerr << DEBUG() << "Client or server already started" << std::endl;
    return;
  }

  m_debug_prefix = "Server";
  // Create the server instance
  m_server = std::make_unique<httplib::Server>();

  if (!m_server->is_valid()){
    std::cerr << DEBUG() << "Server is not valid" << std::endl;
    return;
  }

  // Checks the served does upon receiving "validate_start" message: checks
  // clients version against its own, crashes if needed, but not before
  // sending an appropriate response.
  m_server->Post("/validate_start", [&](const httplib::Request& req, httplib::Response& res) {
    try {
      std::cout << "Server received request for /validate_start" << std::endl;

      // deserialize
      json req_json = json::parse(req.body);
      bool validated = validate_start(req_json);
      std::cout << DEBUG() << "Server validated, sending validation response to the client to validate it" << std::endl;

      json res_json;
      res_json["version"] = m_version;

      res.set_content(res_json.dump(), "application/json");

      if (!validated) {
        m_server->stop();
      }
    }
    catch (const std::exception& e) {
      std::cout << DEBUG() << "Exception caught: \"" << e.what() << "\" Setting response to 400" << std::endl;
      ERROR_RESPONSE(res, e.what());
    }
  });

  // Listen to the registered requests
  for (const auto& [request_name, handler]: m_request_handlers) {
    m_server->Post(request_name.c_str(), [request_name, handler, this](const httplib::Request& req, httplib::Response& res) {
      try {
        m_request_counter++;
        // Validating the request
        json req_json = json::parse(req.body);
        try {
          m_schema_validator[request_name].request_validator->validate(req_json, m_err);
        }
        catch (const std::exception& e) {
          std::cout << DEBUG() << "Validating the request with name \"" << request_name << "\" failed! Here is why: " << e.what() << std::endl;
          std::cout << DEBUG() << " -- Expected format : " << m_schema_validator[request_name].schema["request"].dump() << std::endl;
          std::cout << DEBUG() << " -- Request received: " << req_json.dump() << std::endl;
          std::cout << DEBUG() << " -- Aborting" << std::endl;
          ERROR_RESPONSE(res, "Server request validation failed: " + std::string(e.what()));
        }

        // Getting the response
        json response_json = handler(req_json);

        // Validating the response
        try {
          m_schema_validator[request_name].response_validator->validate(response_json, m_err);
        }
        catch (const std::exception& e) {
          std::cout << DEBUG() << "Validating the response failed! Here is why: " << e.what() << std::endl;
          std::cout << DEBUG() << "Expected format: " << m_schema_validator[request_name].schema["response"].dump() << std::endl;
          std::cout << DEBUG() << "Response given : " << response_json.dump() << std::endl;
          std::cout << DEBUG() << "Aborting" << std::endl;
          ERROR_RESPONSE(res, "Server response validation failed: " + std::string(e.what()));
        }


        // Sending the response back to the client
        res.set_content(response_json.dump(), "application/json");
        std::cout << DEBUG() << "Request counter: " << m_request_counter << std::endl;
      } 
      catch (const std::exception& e) {
        std::cout << DEBUG() << "Exception caught for request \"" << request_name << "\" : \"" << e.what() << "\" Setting response to 400" << std::endl;
        ERROR_RESPONSE(res, e.what());
      }
    });
  }

  m_server->Post(R"(/.*)", [&](const httplib::Request& req, httplib::Response& res) {
    const std::string& path = req.path;
    if (m_request_handlers.count(path)) {
        // Let the registered handler deal with it.
        return;
    }

    json err = {
        {"error", "Unknown request title: " + path}
    };
    res.status = 404;
    res.set_content(err.dump(2), "application/json");
  });

  std::cout << DEBUG() << "Registered requests handlers: " << std::endl;
  for (const auto& request_name: m_request_handlers) {
    std::cout << DEBUG() << request_name.first << std::endl;
  }

  std::cout << DEBUG() << "VERSION: " << m_version << " started" << std::endl;

  m_server->listen("localhost", 8080);
}

void NuDock::start_client()
{
  m_version = "0.0.1";
  if (m_client || m_server) {
    std::cerr << DEBUG() << "Client or server already started" << std::endl;
    return;
  }

  m_debug_prefix = "Client";
  std::cout << "Starting the client" << std::endl;
  m_client = std::make_unique<httplib::Client>("localhost", 8080);
  std::cout << "Client started!" << std::endl;

  json req_json_validate;
  req_json_validate["version"] = m_version;

  auto res = m_client->Post("/validate_start", req_json_validate.dump(), "application/json");
  if (res && res->status == 200) {
    auto res_json = json::parse(res->body);
    validate_start(res_json);
    std::cout << DEBUG() << "Client validated!" << std::endl;
  }
  else {
    std::cerr << DEBUG() << "Client failed to validate!" << std::endl;
    std::cerr << DEBUG() << " -- The message was: " << req_json_validate.dump() << std::endl;
    std::cerr << DEBUG() << "Request failed with status: " << (res ? res->status : 0) << " and error: " << res.error() << std::endl;
    throw res.error();
  }

  std::cout << DEBUG() << "VERSION: " << m_version << " started" << std::endl;
}

void NuDock::send_request(const std::string& _request, const json& _message)
{
  m_request_counter++;
  if (!m_client) {
    std::cerr << DEBUG() << "Client needs to be started first!" << std::endl;
    std::abort();
  }

  if (_request.empty()) {
    std::cerr << DEBUG() << "Request name is empty!" << std::endl;
    std::abort();
  }

  httplib::Result res;
  try{
    res = m_client->Post(_request, _message.dump(), "application/json");
  } catch (const std::exception& e) {
    std::cerr << DEBUG() << "Exception caught while sending request: " << e.what() << std::endl;
    std::abort();
  }

  if (res && res->status == 200) {
    auto res_json = json::parse(res->body);
    std::cout << DEBUG() << "Received response: " << res_json << " from Server" << std::endl;
    std::cout << DEBUG() << "Request counter: " << m_request_counter << std::endl;
  } else {
    std::cerr << DEBUG() << "Request failed with status: " << (res ? res->status : 0) 
              << ", error: \"" << (res ? res->body : "") 
              << "\", message: " << _message.dump() << std::endl;
    std::cout << DEBUG() << "Request counter: " << m_request_counter << std::endl;
    std::abort();
  }
}
