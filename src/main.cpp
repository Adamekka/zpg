#include "app.hpp"

auto main() -> int {
    App::instance().init_opengl();

    App::instance().create_shaders();
    App::instance().create_models();

    App::instance().run();
}
