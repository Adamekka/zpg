#pragma once

namespace object::mesh {

struct Color final {
    using Value = float;
    constexpr static auto DIMENSION{3};

    Value r{1};
    Value g{1};
    Value b{1};

    Color() = default;

    constexpr Color(const Value r, const Value g, const Value b)
        : r{r}
        , g{g}
        , b{b} {}
};

} // namespace object::mesh
