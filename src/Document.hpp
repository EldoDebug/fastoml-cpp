#pragma once

#include "NodeView.hpp"
#include "Options.hpp"
#include "PathRef.hpp"

#include <memory>
#include <string_view>

namespace Fastoml {

class Document {
public:
    Document() = default;
    ~Document();

    Document(Document&& other) noexcept;
    auto operator=(Document&& other) noexcept -> Document&;

    Document(const Document&) = delete;
    auto operator=(const Document&) -> Document& = delete;

    [[nodiscard]] auto isValid() const noexcept -> bool;
    [[nodiscard]] auto root() const -> Result<NodeView>;
    [[nodiscard]] auto get(std::string_view dotPath) const -> Result<NodeView>;

    template <FixedString Path>
    [[nodiscard]] auto ref() const -> Result<NodeView> {
        return get(Path.view());
    }

    template <auto Ref>
    [[nodiscard]] auto ref() const -> Result<NodeView>
        requires requires { Ref.view(); }
    {
        return get(Ref.view());
    }

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    explicit Document(std::unique_ptr<Impl> impl) noexcept;

    friend auto parse(std::string_view toml, ParseOptions options) -> Result<Document>;
};

[[nodiscard]] auto parse(std::string_view toml, ParseOptions options = {}) -> Result<Document>;
[[nodiscard]] auto validate(std::string_view toml, ParseOptions options = {}) -> Result<void>;

} // namespace Fastoml
