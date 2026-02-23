# fastoml-cpp

A modern C++23 wrapper for [fastoml](https://github.com/EldoDebug/fastoml) — a high-performance TOML parser.

## Features

- **Read** — Parse TOML strings and access values via dot-notation paths
- **Write** — Programmatically build TOML documents with a fluent builder API
- **Struct Mapping** — Bidirectional `struct ↔ TOML` conversion with compile-time field references
- **Validation** — Validate TOML syntax with detailed error locations (line, column, byte offset)
- **Zero-cost Abstractions** — `FixedString`, `StaticPathRef`, and constexpr field mappings

## Requirements

- C++23
- CMake 3.24+

The underlying `fastoml` C library is tracked as a git submodule at `external/fastoml`.

## Integration

### CMake FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
  fastoml_cpp
  GIT_REPOSITORY https://github.com/EldoDebug/fastoml-cpp.git
  GIT_TAG main
  GIT_SUBMODULES_RECURSE TRUE
)
FetchContent_MakeAvailable(fastoml_cpp)

target_link_libraries(your_target PRIVATE fastoml-cpp::fastoml-cpp)
```

If you use `SOURCE_DIR` instead of `GIT_REPOSITORY`, ensure `external/fastoml` is initialized in that source tree.

## Quick Start

### Read

Parse a TOML string and extract values with dot-notation paths.

```cpp
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

    std::cout << "host: " << *host << '\n';
    std::cout << "port: " << *port << '\n';
    std::cout << "timeoutSeconds: " << *timeout << '\n';
    std::cout << "enabled: " << (*enabled ? "true" : "false") << '\n';
}
```

### Write

Build a TOML document programmatically and serialize it to a string.

```cpp
#include "Fastoml.hpp"

#include <iostream>

auto main() -> int {
    auto builder = Fastoml::Builder::create();
    if (!builder) {
        std::cerr << "builder creation failed: " << builder.error().message << '\n';
        return 1;
    }

    auto root = builder->root();
    auto server = root.table("server");
    if (!server) {
        std::cerr << "create table failed: " << server.error().message << '\n';
        return 1;
    }

    server->set("host", "0.0.0.0");
    server->set("port", 3000);
    server->set("timeoutSeconds", 2.25);
    server->set("enabled", true);

    auto output = builder->toToml();
    if (!output) {
        std::cerr << "serialize failed: " << output.error().message << '\n';
        return 1;
    }

    std::cout << *output;
    // [server]
    // host = "0.0.0.0"
    // port = 3000
    // timeoutSeconds = 2.25
    // enabled = true
}
```

### Struct Mapping (Static References)

Map C++ structs to TOML with compile-time field references for automatic serialization and deserialization.

```cpp
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

// Define compile-time field references
constexpr auto SERVER_HOST_REF = Fastoml::field<"host">(&ServerConfig::host);
constexpr auto SERVER_PORT_REF = Fastoml::field<"port">(&ServerConfig::port);
constexpr auto SERVER_TIMEOUT_REF = Fastoml::field<"timeoutSeconds">(&ServerConfig::timeoutSeconds);
constexpr auto SERVER_ENABLED_REF = Fastoml::field<"enabled">(&ServerConfig::enabled);
constexpr auto APP_SERVER_REF = Fastoml::field<"server">(&AppConfig::server);

// Register models
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

    // TOML -> struct
    auto config = Fastoml::parseAs<AppConfig>(input);
    if (!config) {
        std::cerr << "parseAs failed: " << config.error().message << '\n';
        return 1;
    }

    std::cout << "host=" << config->server.host
              << ", port=" << config->server.port << '\n';

    // Modify and serialize back: struct -> TOML
    config->server.host = "0.0.0.0";
    config->server.port = 3000;

    auto encoded = Fastoml::toToml(*config);
    if (!encoded) {
        std::cerr << "toToml failed: " << encoded.error().message << '\n';
        return 1;
    }

    std::cout << *encoded;
}
```

## API Overview

| Function / Class | Description |
|---|---|
| `Fastoml::parse(toml, options)` | Parse a TOML string into a `Document` |
| `Fastoml::validate(toml, options)` | Validate TOML syntax without building a document |
| `Document::get(dotPath)` | Access a value by dot-notation path (e.g. `"server.host"`) |
| `Document::ref<"path">()` | Access a value via compile-time path reference |
| `NodeView::as<T>()` | Convert a node to `bool`, `int64_t`, `double`, `string_view`, etc. |
| `NodeView::kind()` | Get the node type (`Table`, `Array`, `String`, `Int`, `Float`, `Bool`, ...) |
| `Builder::create(options)` | Create a new TOML document builder |
| `NodeBuilder::set(key, value)` | Set a key-value pair on a table |
| `NodeBuilder::table(key)` / `array(key)` | Create a nested table or array |
| `NodeBuilder::push(value)` | Append a value to an array |
| `Builder::toToml(options)` | Serialize the built document to a TOML string |
| `Fastoml::parseAs<T>(toml)` | Parse TOML directly into a struct |
| `Fastoml::toToml(value)` | Serialize a struct to a TOML string |
| `FASTOML_CPP_MODEL(Type, ...)` | Register a struct for automatic TOML conversion |

All fallible operations return `Fastoml::Result<T>` (`std::expected<T, Fastoml::Error>`).

## Building

If you clone this repository directly, initialize submodules first:

```bash
git submodule update --init --recursive
```

```bash
cmake -B build -G Ninja
cmake --build build
```

To disable building examples:

```bash
cmake -B build -G Ninja -DFASTOML_CPP_BUILD_EXAMPLES=OFF
```


