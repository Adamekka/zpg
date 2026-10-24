#include "texture.hpp"
#include "../../../core/assert.hpp"
#include "../../../core/defer.hpp"
#include "../../../gl.hpp"
#include <limits>
#include <stb/stb_image.h>

namespace object::material::texture {

Texture::Texture(
    const uint8_t* const pixels, const int32_t width, const int32_t height
) {
    core::assert_ne(pixels, nullptr);
    core::assert_that(width > 0);
    core::assert_that(height > 0);

    glGenTextures(1, &this->id);
    this->bind(0);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixels
    );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

auto Texture::load(const std::filesystem::path& path)
    -> std::expected<Texture, std::string> {
    auto width{int32_t{}};
    auto height{int32_t{}};

    auto* const data{
        stbi_load(path.c_str(), &width, &height, nullptr, STBI_rgb_alpha)
    };

    defer({ stbi_image_free(data); });

    if (data == nullptr) {
        auto message{std::string{"Failed to decode texture '"}};
        message.append(path).append("'");

        if (const auto* const reason{stbi_failure_reason()};
            reason != nullptr) {
            message.append(": ").append(reason);
        }

        return std::unexpected{std::move(message)};
    }

    return Texture{data, width, height};
}

auto Texture::load(const std::span<const uint8_t> data)
    -> std::expected<Texture, std::string> {
    if (data.empty()) {
        return std::unexpected{"Encoded texture data is empty"};
    }

    if (data.size()
        > static_cast<size_t>(std::numeric_limits<int32_t>::max())) {
        return std::unexpected{
            "Encoded texture data exceeds stb_image's size limit"
        };
    }

    auto width{int32_t{}};
    auto height{int32_t{}};
    auto* const pixels{stbi_load_from_memory(
        data.data(),
        static_cast<int32_t>(data.size()),
        &width,
        &height,
        nullptr,
        STBI_rgb_alpha
    )};

    defer({ stbi_image_free(pixels); });

    if (pixels == nullptr) {
        auto message{std::string{"Failed to decode encoded texture"}};

        if (const auto* const reason{stbi_failure_reason()};
            reason != nullptr) {
            message.append(": ").append(reason);
        }

        return std::unexpected{std::move(message)};
    }

    return Texture{pixels, width, height};
}

auto Texture::solid_color(const std::array<uint8_t, 4> color) -> Texture {
    return Texture{color.data(), 1, 1};
}

Texture::Texture(Texture&& other) noexcept
    : id{std::exchange(other.id, 0)} {}

Texture::~Texture() {
    if (this->id != 0) {
        glDeleteTextures(1, &this->id);
    }
}

auto Texture::operator=(Texture&& other) noexcept -> Texture& {
    if (this != &other) {
        glDeleteTextures(1, &this->id);
        this->id = std::exchange(other.id, 0);
    }

    return *this;
}

auto Texture::bind(const uint32_t unit) const -> void {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, this->id);
}

auto Texture::unbind(const uint32_t unit) -> void {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace object::material::texture
