#pragma once

#include <array>
#include <cstddef>
#include <string_view>

namespace Fastoml {

template <std::size_t N>
struct FixedString {
    std::array<char, N> value{};

    constexpr FixedString(const char (&text)[N]) {
        for (std::size_t i = 0; i < N; ++i) {
            value[i] = text[i];
        }
    }

    [[nodiscard]] constexpr auto view() const -> std::string_view {
        static_assert(N > 0, "FixedString must not be empty.");
        return std::string_view(value.data(), N - 1u);
    }
};

template <FixedString Path>
struct StaticPathRef {
    static constexpr auto literal = Path;

    [[nodiscard]] static constexpr auto view() -> std::string_view {
        return literal.view();
    }
};

template <FixedString Path>
[[nodiscard]] constexpr auto pathRef() -> StaticPathRef<Path> {
    return {};
}

} // namespace Fastoml
