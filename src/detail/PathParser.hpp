#pragma once

#include "Error.hpp"

#include <string_view>
#include <vector>

namespace Fastoml::detail {

[[nodiscard]] auto splitDotPath(std::string_view path) -> Result<std::vector<std::string_view>>;

} // namespace Fastoml::detail
