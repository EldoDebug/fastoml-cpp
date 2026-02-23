#include "Fastoml.hpp"

#include <iostream>

auto main() -> int {
    auto builder = Fastoml::Builder::create();
    if (!builder) {
        std::cerr << "builder creation failed: " << builder.error().message << '\n';
        return 1;
    }

    auto root = builder->root();
    if (!root.valid()) {
        std::cerr << "builder root is invalid\n";
        return 1;
    }

    auto server = root.table("server");
    if (!server) {
        std::cerr << "create table failed: " << server.error().message << '\n';
        return 1;
    }

    if (!server->set("host", "0.0.0.0")) {
        std::cerr << "set host failed\n";
        return 1;
    }
    if (!server->set("port", 3000)) {
        std::cerr << "set port failed\n";
        return 1;
    }
    if (!server->set("timeoutSeconds", 2.25)) {
        std::cerr << "set timeoutSeconds failed\n";
        return 1;
    }
    if (!server->set("enabled", true)) {
        std::cerr << "set enabled failed\n";
        return 1;
    }

    auto output = builder->toToml();
    if (!output) {
        std::cerr << "serialize failed: " << output.error().message << '\n';
        return 1;
    }

    std::cout << *output;
    return 0;
}

