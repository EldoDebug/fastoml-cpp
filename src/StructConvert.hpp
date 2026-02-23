#pragma once

#include "Builder.hpp"
#include "Document.hpp"
#include "PathRef.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace Fastoml {

template <typename T>
struct Model;

template <typename T>
concept ModelDefined = requires { Model<T>::fields(); };

template <typename Owner, typename Member, FixedString Key>
struct StaticFieldRef {
    using OwnerType = Owner;
    using MemberType = Member;

    Member Owner::*member;

    [[nodiscard]] static constexpr auto key() -> std::string_view {
        return Key.view();
    }
};

template <FixedString Key, typename Owner, typename Member>
[[nodiscard]] consteval auto field(Member Owner::*member) -> StaticFieldRef<Owner, Member, Key> {
    return StaticFieldRef<Owner, Member, Key>{member};
}

namespace detail {

template <typename T>
using Decayed = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename T>
[[nodiscard]] auto decodeNode(const NodeView& node) -> Result<T>;

template <typename T, typename FieldRef>
[[nodiscard]] auto decodeField(const NodeView& tableNode, T& output, const FieldRef& ref) -> Result<void> {
    using Owner = typename FieldRef::OwnerType;
    using Member = typename FieldRef::MemberType;
    using MemberDecayed = Decayed<Member>;

    static_assert(std::is_same_v<Owner, T>, "Field owner type must match model type.");

    if constexpr (std::is_same_v<MemberDecayed, std::string_view>) {
        return makeUnexpected<void>(
            Error{ErrorCode::UnsupportedType, "std::string_view is not supported for decode output fields."});
    }

    auto node = tableNode.get(FieldRef::key());
    if (!node) {
        return makeUnexpected<void>(node.error());
    }

    auto value = decodeNode<MemberDecayed>(*node);
    if (!value) {
        return makeUnexpected<void>(value.error());
    }

    output.*(ref.member) = std::move(*value);
    return {};
}

template <std::size_t Index, typename T, typename Tuple>
[[nodiscard]] auto decodeFields(const NodeView& tableNode, T& output, const Tuple& refs) -> Result<void> {
    if constexpr (Index >= std::tuple_size_v<Tuple>) {
        return {};
    } else {
        auto status = decodeField(tableNode, output, std::get<Index>(refs));
        if (!status) {
            return makeUnexpected<void>(status.error());
        }
        return decodeFields<Index + 1u>(tableNode, output, refs);
    }
}

template <typename T>
[[nodiscard]] auto decodeObject(const NodeView& node) -> Result<T> {
    if (node.kind() != NodeKind::Table) {
        return makeUnexpected<T>(Error{ErrorCode::Type, "Decoded node must be a TOML table."});
    }

    T output{};
    const auto refs = Model<T>::fields();
    auto status = decodeFields<0u>(node, output, refs);
    if (!status) {
        return makeUnexpected<T>(status.error());
    }
    return output;
}

template <typename T>
[[nodiscard]] auto decodeNode(const NodeView& node) -> Result<T> {
    using Value = Decayed<T>;
    if constexpr (ModelDefined<Value>) {
        return decodeObject<Value>(node);
    } else {
        return node.template as<Value>();
    }
}

template <typename T>
[[nodiscard]] auto setScalar(NodeBuilder& table, std::string_view key, const T& value) -> Result<void> {
    using Value = Decayed<T>;
    Result<NodeBuilder> setStatus =
        makeUnexpected<NodeBuilder>(Error{ErrorCode::UnsupportedType, "Type is not serializable to TOML scalar."});

    if constexpr (std::is_same_v<Value, std::string>) {
        setStatus = table.set(key, std::string_view(value));
    } else if constexpr (std::is_same_v<Value, std::string_view>) {
        setStatus = table.set(key, value);
    } else if constexpr (std::is_same_v<Value, const char*>) {
        setStatus = table.set(key, value);
    } else if constexpr (std::is_same_v<Value, bool>) {
        setStatus = table.set(key, value);
    } else if constexpr (std::is_floating_point_v<Value>) {
        setStatus = table.set(key, static_cast<double>(value));
    } else if constexpr (std::is_integral_v<Value>) {
        setStatus = table.set(key, value);
    }

    if (!setStatus) {
        return makeUnexpected<void>(setStatus.error());
    }
    return {};
}

template <typename T>
[[nodiscard]] auto encodeNode(NodeBuilder& tableNode, const T& value) -> Result<void>;

template <typename T, typename FieldRef>
[[nodiscard]] auto encodeField(NodeBuilder& tableNode, const T& source, const FieldRef& ref) -> Result<void> {
    using Owner = typename FieldRef::OwnerType;
    using Member = typename FieldRef::MemberType;
    using MemberDecayed = Decayed<Member>;

    static_assert(std::is_same_v<Owner, T>, "Field owner type must match model type.");

    const auto& memberValue = source.*(ref.member);
    if constexpr (ModelDefined<MemberDecayed>) {
        auto nestedTable = tableNode.table(FieldRef::key());
        if (!nestedTable) {
            return makeUnexpected<void>(nestedTable.error());
        }
        auto nestedNode = *nestedTable;
        return encodeNode(nestedNode, memberValue);
    } else {
        return setScalar(tableNode, FieldRef::key(), memberValue);
    }
}

template <std::size_t Index, typename T, typename Tuple>
[[nodiscard]] auto encodeFields(NodeBuilder& tableNode, const T& source, const Tuple& refs) -> Result<void> {
    if constexpr (Index >= std::tuple_size_v<Tuple>) {
        return {};
    } else {
        auto status = encodeField(tableNode, source, std::get<Index>(refs));
        if (!status) {
            return makeUnexpected<void>(status.error());
        }
        return encodeFields<Index + 1u>(tableNode, source, refs);
    }
}

template <typename T>
[[nodiscard]] auto encodeNode(NodeBuilder& tableNode, const T& value) -> Result<void> {
    using Value = Decayed<T>;
    if constexpr (!ModelDefined<Value>) {
        return makeUnexpected<void>(
            Error{ErrorCode::UnsupportedType, "encodeNode requires a Model<T> specialization."});
    } else {
        const auto refs = Model<Value>::fields();
        return encodeFields<0u>(tableNode, value, refs);
    }
}

} // namespace detail

template <typename T>
[[nodiscard]] auto decode(const Document& document) -> Result<T>
    requires ModelDefined<std::remove_cv_t<std::remove_reference_t<T>>>
{
    auto rootNode = document.root();
    if (!rootNode) {
        return makeUnexpected<T>(rootNode.error());
    }
    return detail::decodeNode<std::remove_cv_t<std::remove_reference_t<T>>>(*rootNode);
}

template <typename T>
[[nodiscard]] auto parseAs(std::string_view toml, ParseOptions options = {}) -> Result<T>
    requires ModelDefined<std::remove_cv_t<std::remove_reference_t<T>>>
{
    auto document = parse(toml, options);
    if (!document) {
        return makeUnexpected<T>(document.error());
    }
    return decode<std::remove_cv_t<std::remove_reference_t<T>>>(*document);
}

template <typename T>
[[nodiscard]] auto toToml(const T& source, SerializeOptions options = {}) -> Result<std::string>
    requires ModelDefined<std::remove_cv_t<std::remove_reference_t<T>>>
{
    auto builder = Builder::create();
    if (!builder) {
        return makeUnexpected<std::string>(builder.error());
    }

    auto root = builder->root();
    if (!root.valid()) {
        return makeUnexpected<std::string>(
            Error{ErrorCode::InvalidState, "Builder root is invalid for struct serialization."});
    }

    auto encodeStatus = detail::encodeNode(root, source);
    if (!encodeStatus) {
        return makeUnexpected<std::string>(encodeStatus.error());
    }

    return builder->toToml(options);
}

} // namespace Fastoml

#define FASTOML_CPP_FIELD(TYPE, MEMBER, KEY_LITERAL) ::Fastoml::field<KEY_LITERAL>(&TYPE::MEMBER)
#define FASTOML_CPP_MODEL(TYPE, ...)                                                                           \
    template <>                                                                                                \
    struct ::Fastoml::Model<TYPE> {                                                                           \
        static constexpr auto fields() {                                                                       \
            return std::make_tuple(__VA_ARGS__);                                                               \
        }                                                                                                      \
    }
