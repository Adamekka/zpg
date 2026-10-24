#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <span>
#include <string>

namespace object::material::texture {

class Texture final {
  public:
    Texture(const Texture&) = delete;
    Texture(Texture&&) noexcept;

    ~Texture();

    auto operator=(const Texture&) -> Texture& = delete;
    auto operator=(Texture&&) noexcept -> Texture&;

    [[nodiscard]] static auto load(const std::filesystem::path& path)
        -> std::expected<Texture, std::string>;

    [[nodiscard]] static auto load(std::span<const uint8_t> data)
        -> std::expected<Texture, std::string>;

    [[nodiscard]] static auto solid_color(std::array<uint8_t, 4> color)
        -> Texture;

    auto bind(uint32_t unit) const -> void;
    static auto unbind(uint32_t unit) -> void;

  private:
    uint32_t id{0};

    Texture(const uint8_t* pixels, int32_t width, int32_t height);
};

} // namespace object::material::texture
