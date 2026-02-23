#pragma once

#include <cstdint>

namespace Fastoml {

struct ParseOptions {
    bool validateOnly = false;
    bool disableSimd = false;
    bool trustUtf8 = false;
    std::uint32_t maxDepth = 256u;
};

struct BuilderOptions {
    std::uint32_t maxDepth = 256u;
};

struct SerializeOptions {
    bool finalNewline = true;
};

} // namespace Fastoml
