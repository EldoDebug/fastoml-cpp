#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <utility>

namespace Fastoml {

enum class ErrorCode {
    Ok = 0,
    Syntax,
    Utf8,
    DuplicateKey,
    Type,
    OutOfMemory,
    Overflow,
    Depth,
    KeyNotFound,
    InvalidPath,
    InvalidState,
    UnsupportedType,
};

struct Error {
    ErrorCode code = ErrorCode::Ok;
    std::string message;
    std::uint32_t byteOffset = 0u;
    std::uint32_t line = 0u;
    std::uint32_t column = 0u;
};

template <typename T>
using Result = std::expected<T, Error>;

template <typename T>
[[nodiscard]] inline auto makeUnexpected(Error error) -> std::unexpected<Error> {
    return std::unexpected<Error>(std::move(error));
}

} // namespace Fastoml
