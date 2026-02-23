#pragma once

#include "Error.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>

struct fastoml_node;

namespace Fastoml {

enum class NodeKind {
    Table = 1,
    Array = 2,
    String = 3,
    Int = 4,
    Float = 5,
    Bool = 6,
    DateTime = 7,
    Date = 8,
    Time = 9,
    Unknown = 255,
};

class NodeView {
public:
    NodeView() = default;
    explicit NodeView(const fastoml_node* node) noexcept;

    [[nodiscard]] auto valid() const noexcept -> bool;
    [[nodiscard]] auto kind() const -> NodeKind;
    [[nodiscard]] auto size() const -> std::size_t;
    [[nodiscard]] auto get(std::string_view key) const -> Result<NodeView>;

    [[nodiscard]] auto asBool() const -> Result<bool>;
    [[nodiscard]] auto asInt64() const -> Result<std::int64_t>;
    [[nodiscard]] auto asDouble() const -> Result<double>;
    [[nodiscard]] auto asStringView() const -> Result<std::string_view>;

    template <typename T>
    [[nodiscard]] auto as() const -> Result<T> {
        if constexpr (std::is_same_v<T, bool>) {
            auto value = asBool();
            if (!value) {
                return makeUnexpected<T>(value.error());
            }
            return *value;
        }

        if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
            auto value = asInt64();
            if (!value) {
                return makeUnexpected<T>(value.error());
            }

            const auto raw = *value;
            if constexpr (std::is_signed_v<T>) {
                if (raw < static_cast<std::int64_t>((std::numeric_limits<T>::lowest)()) ||
                    raw > static_cast<std::int64_t>((std::numeric_limits<T>::max)())) {
                    return makeUnexpected<T>(
                        Error{ErrorCode::Overflow, "Integer conversion overflow while reading node."});
                }
            } else {
                if (raw < 0) {
                    return makeUnexpected<T>(
                        Error{ErrorCode::Overflow, "Integer conversion overflow while reading node."});
                }
                if (static_cast<std::uint64_t>(raw) >
                    static_cast<std::uint64_t>((std::numeric_limits<T>::max)())) {
                    return makeUnexpected<T>(
                        Error{ErrorCode::Overflow, "Integer conversion overflow while reading node."});
                }
            }
            return static_cast<T>(raw);
        }

        if constexpr (std::is_floating_point_v<T>) {
            auto value = asDouble();
            if (!value) {
                return makeUnexpected<T>(value.error());
            }
            return static_cast<T>(*value);
        }

        if constexpr (std::is_same_v<T, std::string_view>) {
            return asStringView();
        }

        if constexpr (std::is_same_v<T, std::string>) {
            auto value = asStringView();
            if (!value) {
                return makeUnexpected<T>(value.error());
            }
            return std::string(*value);
        }

        return makeUnexpected<T>(
            Error{ErrorCode::UnsupportedType, "Requested type is not supported by NodeView::as()."});
    }

    [[nodiscard]] auto raw() const noexcept -> const fastoml_node*;

private:
    const fastoml_node* node_ = nullptr;
};

} // namespace Fastoml
