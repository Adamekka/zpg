#include "app.hpp"
#include <cstdio>
#include <print>

auto main() -> int {
    App::instance().init_opengl();
    if (const auto result{App::instance().create_objects()}; !result) {
        std::println(stderr, "{}", result.error());
        return 1;
    }
    App::instance().run();
}
