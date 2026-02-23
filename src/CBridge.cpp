#include "detail/CInterop.hpp"

#include <limits>
#include <string>

namespace Fastoml::detail {

auto toErrorCode(fastoml_status status) -> ErrorCode {
    switch (status) {
    case FASTOML_OK:
        return ErrorCode::Ok;
    case FASTOML_ERR_SYNTAX:
        return ErrorCode::Syntax;
    case FASTOML_ERR_UTF8:
        return ErrorCode::Utf8;
    case FASTOML_ERR_DUP_KEY:
        return ErrorCode::DuplicateKey;
    case FASTOML_ERR_TYPE:
        return ErrorCode::Type;
    case FASTOML_ERR_OOM:
        return ErrorCode::OutOfMemory;
    case FASTOML_ERR_OVERFLOW:
        return ErrorCode::Overflow;
    case FASTOML_ERR_DEPTH:
        return ErrorCode::Depth;
    default:
        return ErrorCode::InvalidState;
    }
}

auto toError(fastoml_status status, const fastoml_error* error, std::string_view context) -> Error {
    Error out;
    out.code = toErrorCode(status);
    out.message = std::string(context);
    if (status != FASTOML_OK) {
        out.message += ": ";
        out.message += fastoml_status_string(status);
    }

    if (error != nullptr) {
        out.byteOffset = error->byte_offset;
        out.line = error->line;
        out.column = error->column;
    }

    return out;
}

auto toFastomlOptions(const ParseOptions& options) -> fastoml_options {
    fastoml_options out;
    fastoml_options_default(&out);
    out.flags = 0u;

    if (options.validateOnly) {
        out.flags |= FASTOML_PARSE_VALIDATE_ONLY;
    }
    if (options.disableSimd) {
        out.flags |= FASTOML_PARSE_DISABLE_SIMD;
    }
    if (options.trustUtf8) {
        out.flags |= FASTOML_PARSE_TRUST_UTF8;
    }
    out.max_depth = options.maxDepth;
    return out;
}

auto toFastomlBuilderOptions(const BuilderOptions& options) -> fastoml_builder_options {
    fastoml_builder_options out;
    fastoml_builder_options_default(&out);
    out.max_depth = options.maxDepth;
    return out;
}

auto toFastomlSerializeOptions(const SerializeOptions& options) -> fastoml_serialize_options {
    fastoml_serialize_options out;
    fastoml_serialize_options_default(&out);
    if (!options.finalNewline) {
        out.flags &= ~FASTOML_SERIALIZE_FINAL_NEWLINE;
    }
    return out;
}

auto toSlice(std::string_view value) -> Result<fastoml_slice> {
    if (value.size() > static_cast<std::size_t>((std::numeric_limits<std::uint32_t>::max)())) {
        return makeUnexpected<fastoml_slice>(
            Error{ErrorCode::Overflow, "Input text exceeds fastoml_slice length limit."});
    }

    fastoml_slice slice;
    slice.ptr = value.data();
    slice.len = static_cast<std::uint32_t>(value.size());
    return slice;
}

} // namespace Fastoml::detail
