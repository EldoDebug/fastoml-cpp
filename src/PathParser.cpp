#include "detail/PathParser.hpp"

namespace Fastoml::detail {

auto splitDotPath(std::string_view path) -> Result<std::vector<std::string_view>> {
    std::vector<std::string_view> parts;
    if (path.empty()) {
        return parts;
    }

    std::size_t begin = 0u;
    while (begin <= path.size()) {
        const auto end = path.find('.', begin);
        const auto count = (end == std::string_view::npos) ? path.size() - begin : end - begin;
        if (count == 0u) {
            return makeUnexpected<std::vector<std::string_view>>(
                Error{ErrorCode::InvalidPath, "Dot path contains an empty segment."});
        }

        parts.emplace_back(path.data() + begin, count);
        if (end == std::string_view::npos) {
            break;
        }
        begin = end + 1u;
    }

    return parts;
}

} // namespace Fastoml::detail
