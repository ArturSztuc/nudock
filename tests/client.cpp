#include <nudock/nudock.hpp>

int main()
{
    NuDock client;
    client.start_client();

    json message;
    message["request"] = "message";
    message["numb"] = 2.12;

    json logl_request;
    logl_request["osc_pars"]["Deltam2_32"] = 0.002;
    logl_request["osc_pars"]["Deltam2_21"] = 0.002;
    logl_request["osc_pars"]["Theta13"] = 0.1;
    logl_request["osc_pars"]["Theta12"] = 0.1;
    logl_request["osc_pars"]["Theta23"] = 0.6;
    logl_request["osc_pars"]["DeltaCP"] = 0.01;
    logl_request["sys_pars"]["sys1"] = 0.01;
    logl_request["sys_pars"]["sys2"] = 0.02;

    while (true) {
        client.send_request("/ping", "Hello! :)");
        client.send_request("/log_likelihood", logl_request);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}
