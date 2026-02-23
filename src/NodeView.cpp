#include "NodeView.hpp"

#include "detail/CInterop.hpp"

#include <string>
#include <utility>

namespace {

auto toNodeKind(fastoml_node_kind kind) -> Fastoml::NodeKind {
    switch (kind) {
    case FASTOML_NODE_TABLE:
        return Fastoml::NodeKind::Table;
    case FASTOML_NODE_ARRAY:
        return Fastoml::NodeKind::Array;
    case FASTOML_NODE_STRING:
        return Fastoml::NodeKind::String;
    case FASTOML_NODE_INT:
        return Fastoml::NodeKind::Int;
    case FASTOML_NODE_FLOAT:
        return Fastoml::NodeKind::Float;
    case FASTOML_NODE_BOOL:
        return Fastoml::NodeKind::Bool;
    case FASTOML_NODE_DATETIME:
        return Fastoml::NodeKind::DateTime;
    case FASTOML_NODE_DATE:
        return Fastoml::NodeKind::Date;
    case FASTOML_NODE_TIME:
        return Fastoml::NodeKind::Time;
    default:
        return Fastoml::NodeKind::Unknown;
    }
}

} // namespace

namespace Fastoml {

NodeView::NodeView(const fastoml_node* node) noexcept : node_(node) {
}

auto NodeView::valid() const noexcept -> bool {
    return node_ != nullptr;
}

auto NodeView::kind() const -> NodeKind {
    if (node_ == nullptr) {
        return NodeKind::Unknown;
    }
    return toNodeKind(fastoml_node_kindof(node_));
}

auto NodeView::size() const -> std::size_t {
    if (node_ == nullptr) {
        return 0u;
    }

    const auto nodeKind = fastoml_node_kindof(node_);
    if (nodeKind == FASTOML_NODE_TABLE) {
        return static_cast<std::size_t>(fastoml_table_size(node_));
    }
    if (nodeKind == FASTOML_NODE_ARRAY) {
        return static_cast<std::size_t>(fastoml_array_size(node_));
    }
    return 0u;
}

auto NodeView::get(std::string_view key) const -> Result<NodeView> {
    if (node_ == nullptr) {
        return makeUnexpected<NodeView>(Error{ErrorCode::InvalidState, "Cannot read a null node."});
    }
    if (fastoml_node_kindof(node_) != FASTOML_NODE_TABLE) {
        return makeUnexpected<NodeView>(Error{ErrorCode::Type, "Node is not a table."});
    }
    if (key.empty()) {
        return makeUnexpected<NodeView>(Error{ErrorCode::InvalidPath, "Table key must not be empty."});
    }

    auto keySlice = detail::toSlice(key);
    if (!keySlice) {
        return makeUnexpected<NodeView>(keySlice.error());
    }

    const auto* child = fastoml_table_get(node_, *keySlice);
    if (child == nullptr) {
        auto message = std::string("Key not found in table: ");
        message += key;
        return makeUnexpected<NodeView>(Error{ErrorCode::KeyNotFound, std::move(message)});
    }

    return NodeView(child);
}

auto NodeView::asBool() const -> Result<bool> {
    if (node_ == nullptr) {
        return makeUnexpected<bool>(Error{ErrorCode::InvalidState, "Cannot read a null node."});
    }

    int value = 0;
    const auto status = fastoml_node_as_bool(node_, &value);
    if (status != FASTOML_OK) {
        return makeUnexpected<bool>(detail::toError(status, nullptr, "Failed to read bool value"));
    }
    return value != 0;
}

auto NodeView::asInt64() const -> Result<std::int64_t> {
    if (node_ == nullptr) {
        return makeUnexpected<std::int64_t>(Error{ErrorCode::InvalidState, "Cannot read a null node."});
    }

    std::int64_t value = 0;
    const auto status = fastoml_node_as_int(node_, &value);
    if (status != FASTOML_OK) {
        return makeUnexpected<std::int64_t>(detail::toError(status, nullptr, "Failed to read int value"));
    }
    return value;
}

auto NodeView::asDouble() const -> Result<double> {
    if (node_ == nullptr) {
        return makeUnexpected<double>(Error{ErrorCode::InvalidState, "Cannot read a null node."});
    }

    double value = 0.0;
    const auto status = fastoml_node_as_float(node_, &value);
    if (status != FASTOML_OK) {
        return makeUnexpected<double>(detail::toError(status, nullptr, "Failed to read float value"));
    }
    return value;
}

auto NodeView::asStringView() const -> Result<std::string_view> {
    if (node_ == nullptr) {
        return makeUnexpected<std::string_view>(Error{ErrorCode::InvalidState, "Cannot read a null node."});
    }

    fastoml_slice slice;
    const auto status = fastoml_node_as_slice(node_, &slice);
    if (status != FASTOML_OK) {
        return makeUnexpected<std::string_view>(
            detail::toError(status, nullptr, "Failed to read string-like value"));
    }
    return std::string_view(slice.ptr, slice.len);
}

auto NodeView::raw() const noexcept -> const fastoml_node* {
    return node_;
}

} // namespace Fastoml
