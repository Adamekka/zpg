#include "app.hpp"

auto main() -> int {
    App::instance().init_opengl();
    App::instance().create_objects();
    App::instance().run();
}
