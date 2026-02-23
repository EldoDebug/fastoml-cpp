#include "Fastoml.hpp"

#include <iostream>

auto main() -> int {
    constexpr auto text = R"(
[server]
host = "127.0.0.1"
port = 8080
timeoutSeconds = 1.5
enabled = true
)";

    auto document = Fastoml::parse(text);
    if (!document) {
        std::cerr << "parse failed: " << document.error().message << '\n';
        return 1;
    }

    auto hostNode = document->get("server.host");
    auto portNode = document->get("server.port");
    auto timeoutNode = document->get("server.timeoutSeconds");
    auto enabledNode = document->get("server.enabled");
    if (!hostNode || !portNode || !timeoutNode || !enabledNode) {
        std::cerr << "required keys are missing\n";
        return 1;
    }

    auto host = hostNode->as<std::string_view>();
    auto port = portNode->as<std::int64_t>();
    auto timeout = timeoutNode->as<double>();
    auto enabled = enabledNode->as<bool>();
    if (!host || !port || !timeout || !enabled) {
        std::cerr << "type conversion failed\n";
        return 1;
    }

    std::cout << "host: " << *host << '\n';
    std::cout << "port: " << *port << '\n';
    std::cout << "timeoutSeconds: " << *timeout << '\n';
    std::cout << "enabled: " << (*enabled ? "true" : "false") << '\n';
    return 0;
}

