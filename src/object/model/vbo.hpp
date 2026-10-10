#pragma once

#include <cstdint>
#include <span>

namespace object::model {

class VBO final {
  public:
    explicit VBO(std::span<const float> data);

    VBO(const VBO&) = delete;
    VBO(VBO&&) noexcept;

    ~VBO();

    auto operator=(const VBO&) -> VBO& = delete;
    auto operator=(VBO&&) noexcept -> VBO&;

    auto bind() const -> void;
    static auto unbind() -> void;

  private:
    uint32_t id{0};
};

} // namespace object::model
