#include "glfw/platform_factory.hpp"

#include "platform_manager.hpp"
#include "core_manager.hpp"

#include "shader.hpp"

enum Action
{
    Exit
};

typedef struct Vertex
{
    glm::vec2 pos;
    glm::vec3 col;
} Vertex;

static const Vertex vertices[3] =
{
{ { -0.6f, -0.4f }, { 1.f, 0.f, 0.f } },
{ {  0.6f, -0.4f }, { 0.f, 1.f, 0.f } },
{ {   0.f,  0.6f }, { 0.f, 0.f, 1.f } }
};

static const char* vertex_shader_text =
"#version 330\n"
"uniform mat4 MVP;\n"
"layout (location = 0) in vec2 vPos;\n"
"layout (location = 1) in vec3 vCol;\n"
"out vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vCol;\n"
"}\n";

static const char* fragment_shader_text =
"#version 330\n"
"in vec3 color;\n"
"out vec4 fragment;\n"
"void main()\n"
"{\n"
"    fragment = vec4(color, 1.0);\n"
"}\n";

int main()
{
    CoreManager      core_manager;
    PlatformManager& platform_manager = PlatformManager::instance();

    core_manager.init();
    platform_manager.init(new glfw::PlatformFactory(), { "Template", 800, 600 });

    gainput::InputManager input_manager;
    input_manager.SetDisplaySize(800, 600);

    const gainput::DeviceId keyboard_id = input_manager.CreateDevice<gainput::InputDeviceKeyboard>();

    gainput::InputMap input_map(input_manager);
    input_map.MapBool(Action::Exit, keyboard_id, gainput::KeyEscape);

    Shader vert_shader;

    const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    const GLint vpos_location = 0;
    const GLint vcol_location = 1;
    const GLint mvp_location  = glGetUniformLocation(program, "MVP");

    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, pos));
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, col));

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    auto angle = 0.0f;
    auto speed = 60.0f;

    while (platform_manager.is_active())
    {
        core_manager.update();
        input_manager.Update();

        if (input_map.GetBoolWasDown(Action::Exit))
        {
            platform_manager.shutdown();
        }

        auto width  = window::Screen::width();
        auto height = window::Screen::height();
        auto ratio  = window::Screen::ratio();

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        angle += speed * Time::delta_time();

        glm::vec3 a = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::mat4 m = glm::rotate(glm::mat4(1.0f), glm::radians(angle), a);
        glm::mat4 p = glm::ortho(-ratio, ratio, -1.0f, 1.0f, 1.0f, -1.0f);
        glm::mat4 mvp = p * m;

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &mvp);

        glBindVertexArray(vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        platform_manager.update();
    }

    core_manager.release();
    platform_manager.release();

    exit(EXIT_SUCCESS);
}