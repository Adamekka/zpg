#pragma once

#include "imported_model.hpp"
#include <expected>
#include <string>

namespace object::importer {

class ModelImporter final {
  public:
    ModelImporter() = delete;

    [[nodiscard]] static auto import(const std::filesystem::path& path)
        -> std::expected<ImportedModel, std::string>;
};

} // namespace object::importer
