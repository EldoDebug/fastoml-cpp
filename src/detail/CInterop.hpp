#pragma once

#include "Error.hpp"
#include "Options.hpp"

#include <fastoml.h>

#include <string_view>

namespace Fastoml::detail {

[[nodiscard]] auto toErrorCode(fastoml_status status) -> ErrorCode;
[[nodiscard]] auto toError(fastoml_status status, const fastoml_error* error, std::string_view context) -> Error;

[[nodiscard]] auto toFastomlOptions(const ParseOptions& options) -> fastoml_options;
[[nodiscard]] auto toFastomlBuilderOptions(const BuilderOptions& options) -> fastoml_builder_options;
[[nodiscard]] auto toFastomlSerializeOptions(const SerializeOptions& options) -> fastoml_serialize_options;

[[nodiscard]] auto toSlice(std::string_view value) -> Result<fastoml_slice>;

} // namespace Fastoml::detail
