#include "Document.hpp"

#include "detail/CInterop.hpp"
#include "detail/PathParser.hpp"

#include <fastoml.h>

#include <memory>
#include <string>
#include <utility>

namespace Fastoml {

namespace {

using ParserPtr = std::unique_ptr<fastoml_parser, decltype(&fastoml_parser_destroy)>;

} // namespace

struct Document::Impl {
    std::string source;
    ParserPtr parser{nullptr, &fastoml_parser_destroy};
    const fastoml_document* document = nullptr;
};

Document::Document(std::unique_ptr<Impl> impl) noexcept : impl_(std::move(impl)) {
}

Document::~Document() = default;

Document::Document(Document&& other) noexcept = default;

auto Document::operator=(Document&& other) noexcept -> Document& = default;

auto Document::isValid() const noexcept -> bool {
    return impl_ != nullptr && impl_->document != nullptr;
}

auto Document::root() const -> Result<NodeView> {
    if (!isValid()) {
        return makeUnexpected<NodeView>(Error{ErrorCode::InvalidState, "Document has no parsed root node."});
    }

    const auto* rootNode = fastoml_doc_root(impl_->document);
    if (rootNode == nullptr) {
        return makeUnexpected<NodeView>(Error{ErrorCode::InvalidState, "Parsed document returned a null root node."});
    }
    return NodeView(rootNode);
}

auto Document::get(std::string_view dotPath) const -> Result<NodeView> {
    auto rootNode = root();
    if (!rootNode) {
        return makeUnexpected<NodeView>(rootNode.error());
    }

    if (dotPath.empty()) {
        return *rootNode;
    }

    auto parts = detail::splitDotPath(dotPath);
    if (!parts) {
        return makeUnexpected<NodeView>(parts.error());
    }

    const fastoml_node* current = rootNode->raw();
    for (const auto segment : *parts) {
        if (fastoml_node_kindof(current) != FASTOML_NODE_TABLE) {
            return makeUnexpected<NodeView>(
                Error{ErrorCode::Type, "Path traversal requires table nodes for each segment."});
        }

        auto key = detail::toSlice(segment);
        if (!key) {
            return makeUnexpected<NodeView>(key.error());
        }

        current = fastoml_table_get(current, *key);
        if (current == nullptr) {
            auto message = std::string("Key not found in table: ");
            message += segment;
            return makeUnexpected<NodeView>(Error{ErrorCode::KeyNotFound, std::move(message)});
        }
    }

    return NodeView(current);
}

auto parse(std::string_view toml, ParseOptions options) -> Result<Document> {
    auto fastOptions = detail::toFastomlOptions(options);
    fastOptions.flags &= ~FASTOML_PARSE_VALIDATE_ONLY;

    ParserPtr parser(fastoml_parser_create(&fastOptions), &fastoml_parser_destroy);
    if (parser == nullptr) {
        return makeUnexpected<Document>(
            Error{ErrorCode::OutOfMemory, "Failed to create fastoml parser instance."});
    }

    auto impl = std::make_unique<Document::Impl>();
    impl->source = std::string(toml);
    impl->parser = std::move(parser);

    const fastoml_document* parsedDocument = nullptr;
    fastoml_error parseError{};
    const auto status = fastoml_parse(
        impl->parser.get(), impl->source.data(), impl->source.size(), &parsedDocument, &parseError);
    if (status != FASTOML_OK) {
        return makeUnexpected<Document>(detail::toError(status, &parseError, "Parse failed"));
    }

    if (parsedDocument == nullptr) {
        return makeUnexpected<Document>(Error{ErrorCode::InvalidState, "fastoml returned a null parsed document."});
    }

    impl->document = parsedDocument;
    return Document(std::move(impl));
}

auto validate(std::string_view toml, ParseOptions options) -> Result<void> {
    auto fastOptions = detail::toFastomlOptions(options);
    fastOptions.flags |= FASTOML_PARSE_VALIDATE_ONLY;

    ParserPtr parser(fastoml_parser_create(&fastOptions), &fastoml_parser_destroy);
    if (parser == nullptr) {
        return makeUnexpected<void>(Error{ErrorCode::OutOfMemory, "Failed to create fastoml parser instance."});
    }

    fastoml_error parseError{};
    const auto source = std::string(toml);
    const auto status = fastoml_validate(parser.get(), source.data(), source.size(), &parseError);
    if (status != FASTOML_OK) {
        return makeUnexpected<void>(detail::toError(status, &parseError, "Validation failed"));
    }

    return {};
}

} // namespace Fastoml
