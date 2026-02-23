#include "Fastoml.hpp"

#include <cstdint>
#include <iostream>
#include <string>

struct ServerConfig {
    std::string host;
    std::int64_t port = 0;
    double timeoutSeconds = 0.0;
    bool enabled = false;
};

struct AppConfig {
    ServerConfig server;
};

constexpr auto SERVER_HOST_REF = Fastoml::field<"host">(&ServerConfig::host);
constexpr auto SERVER_PORT_REF = Fastoml::field<"port">(&ServerConfig::port);
constexpr auto SERVER_TIMEOUT_REF = Fastoml::field<"timeoutSeconds">(&ServerConfig::timeoutSeconds);
constexpr auto SERVER_ENABLED_REF = Fastoml::field<"enabled">(&ServerConfig::enabled);
constexpr auto APP_SERVER_REF = Fastoml::field<"server">(&AppConfig::server);

FASTOML_CPP_MODEL(ServerConfig, SERVER_HOST_REF, SERVER_PORT_REF, SERVER_TIMEOUT_REF, SERVER_ENABLED_REF);
FASTOML_CPP_MODEL(AppConfig, APP_SERVER_REF);

auto main() -> int {
    constexpr auto input = R"(
[server]
host = "127.0.0.1"
port = 8080
timeoutSeconds = 1.25
enabled = true
)";

    auto decoded = Fastoml::parseAs<AppConfig>(input);
    if (!decoded) {
        std::cerr << "parseAs failed: " << decoded.error().message << '\n';
        return 1;
    }

    std::cout << "decoded host=" << decoded->server.host << ", port=" << decoded->server.port
              << ", timeout=" << decoded->server.timeoutSeconds << ", enabled="
              << (decoded->server.enabled ? "true" : "false") << '\n';

    decoded->server.host = "0.0.0.0";
    decoded->server.port = 3000;
    decoded->server.timeoutSeconds = 2.0;
    decoded->server.enabled = false;

    auto encoded = Fastoml::toToml(*decoded);
    if (!encoded) {
        std::cerr << "toToml failed: " << encoded.error().message << '\n';
        return 1;
    }

    std::cout << "\nserialized TOML:\n" << *encoded;
    return 0;
}

