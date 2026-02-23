#include "Builder.hpp"

#include "detail/CInterop.hpp"

#include <cstring>
#include <fastoml.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Fastoml {

struct NodeBuilder::Context {
    std::unique_ptr<fastoml_builder, decltype(&fastoml_builder_destroy)> builder{nullptr, &fastoml_builder_destroy};
};

struct Builder::Impl {
    std::shared_ptr<NodeBuilder::Context> context;
};

NodeBuilder::NodeBuilder(std::weak_ptr<Context> context, fastoml_value* value) noexcept
    : context_(std::move(context)), value_(value) {
}

auto NodeBuilder::valid() const noexcept -> bool {
    const auto context = context_.lock();
    return context != nullptr && context->builder != nullptr && value_ != nullptr;
}

auto NodeBuilder::setValue(std::string_view key, fastoml_value* value) -> Result<NodeBuilder> {
    if (!valid()) {
        return makeUnexpected<NodeBuilder>(Error{ErrorCode::InvalidState, "Builder node is not initialized."});
    }
    if (value == nullptr) {
        return makeUnexpected<NodeBuilder>(
            Error{ErrorCode::OutOfMemory, "Failed to allocate value while setting table entry."});
    }
    if (key.empty()) {
        return makeUnexpected<NodeBuilder>(Error{ErrorCode::InvalidPath, "Table key must not be empty."});
    }

    auto slice = detail::toSlice(key);
    if (!slice) {
        return makeUnexpected<NodeBuilder>(slice.error());
    }

    const auto status = fastoml_builder_table_set(value_, *slice, value);
    if (status != FASTOML_OK) {
        return makeUnexpected<NodeBuilder>(detail::toError(status, nullptr, "Failed to set table value"));
    }
    return NodeBuilder(context_, value_);
}

auto NodeBuilder::pushValue(fastoml_value* value) -> Result<NodeBuilder> {
    if (!valid()) {
        return makeUnexpected<NodeBuilder>(Error{ErrorCode::InvalidState, "Builder node is not initialized."});
    }
    if (value == nullptr) {
        return makeUnexpected<NodeBuilder>(
            Error{ErrorCode::OutOfMemory, "Failed to allocate value while appending array entry."});
    }

    const auto status = fastoml_builder_array_push(value_, value);
    if (status != FASTOML_OK) {
        return makeUnexpected<NodeBuilder>(detail::toError(status, nullptr, "Failed to append array value"));
    }
    return NodeBuilder(context_, value_);
}

auto NodeBuilder::set(std::string_view key, bool value) -> Result<NodeBuilder> {
    auto context = context_.lock();
    if (context == nullptr || context->builder == nullptr || value_ == nullptr) {
        return makeUnexpected<NodeBuilder>(Error{ErrorCode::InvalidState, "Builder node is not initialized."});
    }
    auto* entry = fastoml_builder_new_bool(context->builder.get(), value ? 1 : 0);
    return setValue(key, entry);
}

auto NodeBuilder::set(std::string_view key, std::int64_t value) -> Result<NodeBuilder> {
    auto context = context_.lock();
    if (context == nullptr || context->builder == nullptr || value_ == nullptr) {
        return makeUnexpected<NodeBuilder>(Error{ErrorCode::InvalidState, "Builder node is not initialized."});
    }
    auto* entry = fastoml_builder_new_int(context->builder.get(), value);
    return setValue(key, entry);
}

auto NodeBuilder::set(std::string_view key, double value) -> Result<NodeBuilder> {
    auto context = context_.lock();
    if (context == nullptr || context->builder == nullptr || value_ == nullptr) {
        return makeUnexpected<NodeBuilder>(Error{ErrorCode::InvalidState, "Builder node is not initialized."});
    }
    auto* entry = fastoml_builder_new_float(context->builder.get(), value);
    return setValue(key, entry);
}

auto NodeBuilder::set(std::string_view key, std::string_view value) -> Result<NodeBuilder> {
    auto context = context_.lock();
    if (context == nullptr || context->builder == nullptr || value_ == nullptr) {
        return makeUnexpected<NodeBuilder>(Error{ErrorCode::InvalidState, "Builder node is not initialized."});
    }

    auto slice = detail::toSlice(value);
    if (!slice) {
        return makeUnexpected<NodeBuilder>(slice.error());
    }

    auto* entry = fastoml_builder_new_string(context->builder.get(), *slice);
    return setValue(key, entry);
}

auto NodeBuilder::set(std::string_view key, const char* value) -> Result<NodeBuilder> {
    if (value == nullptr) {
        return makeUnexpected<NodeBuilder>(
            Error{ErrorCode::InvalidState, "Input string pointer must not be null."});
    }
    return set(key, std::string_view(value, std::strlen(value)));
}

auto NodeBuilder::table(std::string_view key) -> Result<NodeBuilder> {
    auto context = context_.lock();
    if (context == nullptr || context->builder == nullptr || value_ == nullptr) {
        return makeUnexpected<NodeBuilder>(Error{ErrorCode::InvalidState, "Builder node is not initialized."});
    }

    auto* table = fastoml_builder_new_table(context->builder.get());
    auto setResult = setValue(key, table);
    if (!setResult) {
        return makeUnexpected<NodeBuilder>(setResult.error());
    }
    return NodeBuilder(context, table);
}

auto NodeBuilder::array(std::string_view key) -> Result<NodeBuilder> {
    auto context = context_.lock();
    if (context == nullptr || context->builder == nullptr || value_ == nullptr) {
        return makeUnexpected<NodeBuilder>(Error{ErrorCode::InvalidState, "Builder node is not initialized."});
    }

    auto* array = fastoml_builder_new_array(context->builder.get());
    auto setResult = setValue(key, array);
    if (!setResult) {
        return makeUnexpected<NodeBuilder>(setResult.error());
    }
    return NodeBuilder(context, array);
}

auto NodeBuilder::push(bool value) -> Result<NodeBuilder> {
    auto context = context_.lock();
    if (context == nullptr || context->builder == nullptr || value_ == nullptr) {
        return makeUnexpected<NodeBuilder>(Error{ErrorCode::InvalidState, "Builder node is not initialized."});
    }
    auto* entry = fastoml_builder_new_bool(context->builder.get(), value ? 1 : 0);
    return pushValue(entry);
}

auto NodeBuilder::push(std::int64_t value) -> Result<NodeBuilder> {
    auto context = context_.lock();
    if (context == nullptr || context->builder == nullptr || value_ == nullptr) {
        return makeUnexpected<NodeBuilder>(Error{ErrorCode::InvalidState, "Builder node is not initialized."});
    }
    auto* entry = fastoml_builder_new_int(context->builder.get(), value);
    return pushValue(entry);
}

auto NodeBuilder::push(double value) -> Result<NodeBuilder> {
    auto context = context_.lock();
    if (context == nullptr || context->builder == nullptr || value_ == nullptr) {
        return makeUnexpected<NodeBuilder>(Error{ErrorCode::InvalidState, "Builder node is not initialized."});
    }
    auto* entry = fastoml_builder_new_float(context->builder.get(), value);
    return pushValue(entry);
}

auto NodeBuilder::push(std::string_view value) -> Result<NodeBuilder> {
    auto context = context_.lock();
    if (context == nullptr || context->builder == nullptr || value_ == nullptr) {
        return makeUnexpected<NodeBuilder>(Error{ErrorCode::InvalidState, "Builder node is not initialized."});
    }

    auto slice = detail::toSlice(value);
    if (!slice) {
        return makeUnexpected<NodeBuilder>(slice.error());
    }

    auto* entry = fastoml_builder_new_string(context->builder.get(), *slice);
    return pushValue(entry);
}

auto NodeBuilder::push(const char* value) -> Result<NodeBuilder> {
    if (value == nullptr) {
        return makeUnexpected<NodeBuilder>(
            Error{ErrorCode::InvalidState, "Input string pointer must not be null."});
    }
    return push(std::string_view(value, std::strlen(value)));
}

auto NodeBuilder::pushTable() -> Result<NodeBuilder> {
    auto context = context_.lock();
    if (context == nullptr || context->builder == nullptr || value_ == nullptr) {
        return makeUnexpected<NodeBuilder>(Error{ErrorCode::InvalidState, "Builder node is not initialized."});
    }

    auto* table = fastoml_builder_new_table(context->builder.get());
    auto pushResult = pushValue(table);
    if (!pushResult) {
        return makeUnexpected<NodeBuilder>(pushResult.error());
    }
    return NodeBuilder(context, table);
}

auto NodeBuilder::pushArray() -> Result<NodeBuilder> {
    auto context = context_.lock();
    if (context == nullptr || context->builder == nullptr || value_ == nullptr) {
        return makeUnexpected<NodeBuilder>(Error{ErrorCode::InvalidState, "Builder node is not initialized."});
    }

    auto* array = fastoml_builder_new_array(context->builder.get());
    auto pushResult = pushValue(array);
    if (!pushResult) {
        return makeUnexpected<NodeBuilder>(pushResult.error());
    }
    return NodeBuilder(context, array);
}

Builder::Builder(std::unique_ptr<Impl> impl) noexcept : impl_(std::move(impl)) {
}

Builder::~Builder() = default;

Builder::Builder(Builder&& other) noexcept : impl_(std::move(other.impl_)) {
}

auto Builder::operator=(Builder&& other) noexcept -> Builder& {
    if (this == &other) {
        return *this;
    }

    impl_ = std::move(other.impl_);
    return *this;
}

auto Builder::create(BuilderOptions options) -> Result<Builder> {
    auto fastOptions = detail::toFastomlBuilderOptions(options);
    std::unique_ptr<fastoml_builder, decltype(&fastoml_builder_destroy)> rawBuilder(
        fastoml_builder_create(&fastOptions), &fastoml_builder_destroy);
    if (rawBuilder == nullptr) {
        return makeUnexpected<Builder>(
            Error{ErrorCode::OutOfMemory, "Failed to create fastoml builder instance."});
    }

    auto impl = std::make_unique<Builder::Impl>();
    impl->context = std::make_shared<NodeBuilder::Context>();
    impl->context->builder = std::move(rawBuilder);

    return Builder(std::move(impl));
}

auto Builder::isValid() const noexcept -> bool {
    return impl_ != nullptr && impl_->context != nullptr && impl_->context->builder != nullptr;
}

auto Builder::root() -> NodeBuilder {
    if (!isValid()) {
        return {};
    }
    return NodeBuilder(impl_->context, fastoml_builder_root(impl_->context->builder.get()));
}

auto Builder::toToml(SerializeOptions options) const -> Result<std::string> {
    if (!isValid()) {
        return makeUnexpected<std::string>(Error{ErrorCode::InvalidState, "Builder is not initialized."});
    }

    const auto* rootValue = fastoml_builder_root(impl_->context->builder.get());
    if (rootValue == nullptr) {
        return makeUnexpected<std::string>(Error{ErrorCode::InvalidState, "Builder root node is null."});
    }

    const auto fastOptions = detail::toFastomlSerializeOptions(options);

    std::size_t textLength = 0u;
    auto status = fastoml_serialize_to_buffer(rootValue, &fastOptions, nullptr, 0u, &textLength);
    if (status != FASTOML_OK) {
        return makeUnexpected<std::string>(
            detail::toError(status, nullptr, "Failed to estimate serialized TOML size"));
    }

    std::vector<char> buffer(textLength + 1u, '\0');
    status = fastoml_serialize_to_buffer(rootValue, &fastOptions, buffer.data(), buffer.size(), &textLength);
    if (status != FASTOML_OK) {
        return makeUnexpected<std::string>(
            detail::toError(status, nullptr, "Failed to serialize TOML document"));
    }

    return std::string(buffer.data(), textLength);
}

} // namespace Fastoml
