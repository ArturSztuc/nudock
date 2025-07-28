#include <nudock/nudock.hpp>

json pong(json _request) 
{
  std::cout << "Received request from client: " << _request.dump() << std::endl;
  json response;
  response = "pong";
  return response;
};

json ping(json _request) 
{
  std::cout << "Received request from client: " << _request.dump() << std::endl;
  json response;
  response["message"] = "ping";
  return response;
};

json log_likelihood(json _request)
{

  // nominal values
  double dm32 = 0.0025;
  double th13 = 0.15;
  double th23 = 0.5;
  double dcp = 0.0;

  double sys1 = 0.01;
  double sys2 = 0.02;


  std::cout << "Received request from client: " << _request.dump() << std::endl;

  std::vector<std::string> required_osc_pars = {"Deltam2_32", "Theta13", "Theta23", "DeltaCP"};
  for (const std::string& par: required_osc_pars) {
    if (!_request["osc_pars"].contains(par) || !_request["osc_pars"][par].is_number()) {
      std::cerr << "Missing or invalid osc_par: " << par << std::endl;
      throw std::invalid_argument("Missing or invalid osc_par: " + par);
    }
  }

  for (auto& [key, value] : _request["sys_pars"].items()) {
    if (!value.is_number()) {
      std::cerr << "Invalid sys_param value for key: " << key << std::endl;
      throw std::invalid_argument("Invalid sys_param value for key: " + key);
    }
  }

  double log_prob = 0.0;
  log_prob += std::pow(_request["osc_pars"]["Deltam2_32"].get<double>() - dm32, 2);
  log_prob += std::pow(_request["osc_pars"]["Theta13"].get<double>() - th13, 2);
  log_prob += std::pow(_request["osc_pars"]["Theta23"].get<double>() - th23, 2);
  log_prob += std::pow(_request["osc_pars"]["DeltaCP"].get<double>() - dcp, 2);

  log_prob += std::pow(_request["sys_pars"]["sys1"].get<double>() - sys1, 2);
  log_prob += std::pow(_request["sys_pars"]["sys2"].get<double>() - sys2, 2);

  json response;
  response["log_likelihood"] = log_prob;

  return response;
};

int main()
{
  NuDock dock;
  dock.register_response("/ping", std::bind(pong, std::placeholders::_1));
  dock.register_response("/log_likelihood", std::bind(log_likelihood, std::placeholders::_1));
  dock.start_server();
  return 0;
}
