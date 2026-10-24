#pragma once

#include <cstdint>
#include <filesystem>
#include <variant>
#include <vector>

namespace object::importer {

struct ImportedTexture final {
    std::variant<std::filesystem::path, std::vector<uint8_t>> source;
};

} // namespace object::importer
