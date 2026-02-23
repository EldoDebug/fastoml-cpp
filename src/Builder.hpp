#pragma once

#include "Error.hpp"
#include "Options.hpp"

#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

struct fastoml_value;

namespace Fastoml {

class Builder;

class NodeBuilder {
public:
    NodeBuilder() = default;

    [[nodiscard]] auto valid() const noexcept -> bool;

    [[nodiscard]] auto set(std::string_view key, bool value) -> Result<NodeBuilder>;
    [[nodiscard]] auto set(std::string_view key, std::int64_t value) -> Result<NodeBuilder>;
    [[nodiscard]] auto set(std::string_view key, double value) -> Result<NodeBuilder>;
    [[nodiscard]] auto set(std::string_view key, std::string_view value) -> Result<NodeBuilder>;

    [[nodiscard]] auto set(std::string_view key, const char* value) -> Result<NodeBuilder>;

    template <typename T>
    [[nodiscard]] auto set(std::string_view key, T value) -> Result<NodeBuilder>
        requires(std::is_integral_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, std::int64_t>)
    {
        if constexpr (std::is_signed_v<T>) {
            return set(key, static_cast<std::int64_t>(value));
        }

        if (value > static_cast<T>((std::numeric_limits<std::int64_t>::max)())) {
            return makeUnexpected<NodeBuilder>(
                Error{ErrorCode::Overflow, "Unsigned integer conversion overflow while building node."});
        }
        return set(key, static_cast<std::int64_t>(value));
    }

    [[nodiscard]] auto table(std::string_view key) -> Result<NodeBuilder>;
    [[nodiscard]] auto array(std::string_view key) -> Result<NodeBuilder>;

    [[nodiscard]] auto push(bool value) -> Result<NodeBuilder>;
    [[nodiscard]] auto push(std::int64_t value) -> Result<NodeBuilder>;
    [[nodiscard]] auto push(double value) -> Result<NodeBuilder>;
    [[nodiscard]] auto push(std::string_view value) -> Result<NodeBuilder>;
    [[nodiscard]] auto push(const char* value) -> Result<NodeBuilder>;
    [[nodiscard]] auto pushTable() -> Result<NodeBuilder>;
    [[nodiscard]] auto pushArray() -> Result<NodeBuilder>;

    template <typename T>
    [[nodiscard]] auto push(T value) -> Result<NodeBuilder>
        requires(std::is_integral_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, std::int64_t>)
    {
        if constexpr (std::is_signed_v<T>) {
            return push(static_cast<std::int64_t>(value));
        }

        if (value > static_cast<T>((std::numeric_limits<std::int64_t>::max)())) {
            return makeUnexpected<NodeBuilder>(
                Error{ErrorCode::Overflow, "Unsigned integer conversion overflow while building node."});
        }
        return push(static_cast<std::int64_t>(value));
    }

private:
    struct Context;
    std::weak_ptr<Context> context_;
    fastoml_value* value_ = nullptr;

    NodeBuilder(std::weak_ptr<Context> context, fastoml_value* value) noexcept;

    [[nodiscard]] auto setValue(std::string_view key, fastoml_value* value) -> Result<NodeBuilder>;
    [[nodiscard]] auto pushValue(fastoml_value* value) -> Result<NodeBuilder>;

    friend class Builder;
};

class Builder {
public:
    Builder() = default;
    ~Builder();

    Builder(Builder&& other) noexcept;
    auto operator=(Builder&& other) noexcept -> Builder&;

    Builder(const Builder&) = delete;
    auto operator=(const Builder&) -> Builder& = delete;

    [[nodiscard]] static auto create(BuilderOptions options = {}) -> Result<Builder>;

    [[nodiscard]] auto isValid() const noexcept -> bool;
    [[nodiscard]] auto root() -> NodeBuilder;
    [[nodiscard]] auto toToml(SerializeOptions options = {}) const -> Result<std::string>;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    explicit Builder(std::unique_ptr<Impl> impl) noexcept;
};

} // namespace Fastoml
