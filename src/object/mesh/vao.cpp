#include "vao.hpp"
#include "../../gl.hpp"
#include <boost/hana.hpp>

namespace {

template<typename Object, typename Field>
[[nodiscard]] auto offset_of(const Object& object, const Field& field) noexcept
    -> size_t {
    return static_cast<size_t>(
        // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<uintptr_t>(std::addressof(field))
        - reinterpret_cast<uintptr_t>(std::addressof(object))
        // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    );
}

} // namespace

namespace object::mesh {

VAO::VAO(const VBO& vbo) {
    glGenVertexArrays(1, &this->id);

    this->bind();
    vbo.bind();

    constexpr auto VERTEX{Vertex{}};
    auto index{uint32_t{0}};

    boost::hana::for_each(
        boost::hana::accessors<Vertex>(),
        [&VERTEX, &index](auto accessor) -> void {
            const auto& field{boost::hana::second(accessor)(VERTEX)};

            using Attribute = std::remove_cvref_t<decltype(field)>;

            // I'd like to compare with GL_FLOAT instead
            static_assert(std::same_as<typename Attribute::Value, float>);

            glVertexAttribPointer(
                index,
                Attribute::DIMENSION,
                GL_FLOAT,
                GL_FALSE,
                sizeof(Vertex),
                // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
                // NOLINTBEGIN(performance-no-int-to-ptr)
                reinterpret_cast<const void*>(offset_of(VERTEX, field))
                // NOLINTEND(performance-no-int-to-ptr)
                // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
            );

            glEnableVertexAttribArray(index);

            ++index;
        }
    );
}

VAO::VAO(VAO&& other) noexcept
    : id{std::exchange(other.id, 0)} {}

VAO::~VAO() {
    if (this->id != 0) {
        glDeleteVertexArrays(1, &this->id);
    }
}

auto VAO::operator=(VAO&& other) noexcept -> VAO& {
    if (this != &other) {
        glDeleteVertexArrays(1, &this->id);
        this->id = std::exchange(other.id, 0);
    }

    return *this;
}

auto VAO::bind() const -> void {
    glBindVertexArray(this->id);
}

auto VAO::unbind() -> void {
    glBindVertexArray(0);
}

} // namespace object::mesh
