#include "app.hpp"
#include "../models/sphere.hpp"
#include "core/panic.hpp"

auto App::instance() -> App& {
    static auto instance{App{}};
    return instance;
}

auto App::init_opengl() -> void {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    this->window = glfwCreateWindow(800, 600, "ZPG", nullptr, nullptr);

    if (window == nullptr) {
        glfwTerminate();
        core::panic("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (gladLoadGL(static_cast<GLADloadfunc>(glfwGetProcAddress)) == 0) {
        core::panic("GLAD initialization failed");
    }
}

auto App::create_shaders() -> void {
    // Sphere
    {
        auto sphere_shader_program{object::shader::ShaderProgram{}};

        sphere_shader_program.compile(
            "shaders/basic.vert", object::shader::ShaderType::Value::Vertex
        );
        sphere_shader_program.compile(
            "shaders/green.frag", object::shader::ShaderType::Value::Fragment
        );

        sphere_shader_program.link();

        this->shader_programs.emplace(
            "sphere", std::move(sphere_shader_program)
        );
    }

    // Square
    {
        auto square_shader_program{object::shader::ShaderProgram{}};

        square_shader_program.compile(
            "shaders/basic.vert", object::shader::ShaderType::Value::Vertex
        );
        square_shader_program.compile(
            "shaders/red.frag", object::shader::ShaderType::Value::Fragment
        );

        square_shader_program.link();

        this->shader_programs.emplace(
            "square", std::move(square_shader_program)
        );
    }
}

auto App::create_models() -> void {
    // Sphere
    {
        auto sphere{object::Object{
            object::mesh::Mesh::from_raw_data(
                SPHERE, object::mesh::MeshDrawMode::Value::Triangles
            ),
            &this->shader_programs.at("sphere")
        }};

        this->objects.emplace_back(std::move(sphere));
    }

    // Square
    {
        auto square{object::Object{
            object::mesh::Mesh{
                std::array{
                    object::mesh::Vertex{
                        {-0.5f, -0.5f, 1.0f}, {1.0f, 0.0f, 0.0f}
                    },
                    object::mesh::Vertex{
                        {0.5f, -0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}
                    },
                    object::mesh::Vertex{
                        {-0.5f, 0.5f, 1.0f}, {0.0f, 0.0f, 1.0f}
                    },
                    object::mesh::Vertex{{0.5f, 0.5f, 1.0f}, {1.0f, 1.0f, 0.0f}}
                },
                object::mesh::MeshDrawMode::Value::TriangleStrip
            },
            &this->shader_programs.at("square")
        }};

        this->objects.emplace_back(std::move(square));
    }
}

auto App::run() const -> void {
    while (glfwWindowShouldClose(window) == 0) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (const auto& object : this->objects) {
            object.draw();
        }

        glfwSwapBuffers(this->window);
        glfwPollEvents();
    }
}

App::~App() {
    if (this->window != nullptr) {
        glfwDestroyWindow(this->window);
    }
    glfwTerminate();
}
