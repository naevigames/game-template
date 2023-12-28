#include "glfw/platform_factory.hpp"

#include "platform_manager.hpp"
#include "core_manager.hpp"

#include "screen.hpp"
#include "file.hpp"

#include "gl/shader.hpp"
#include "gl/buffer.hpp"

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

    auto vert_source = File::read<char>("../simple_vert.glsl");
    auto frag_source = File::read<char>("../simple_frag.glsl");

    gl::ShaderStage vert_stage;
    vert_stage.create(GL_VERTEX_SHADER);
    vert_stage.source(vert_source);

    gl::ShaderStage frag_stage;
    frag_stage.create(GL_FRAGMENT_SHADER);
    frag_stage.source(frag_source);

    gl::Shader shader;
    shader.create();
    shader.attach(vert_stage);
    shader.attach(frag_stage);
    shader.compile();

    const GLint vpos_location = 0;
    const GLint vcol_location = 1;
    const GLint mvp_location  = 0;

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

        auto width  = Screen::width();
        auto height = Screen::height();
        auto ratio  = Screen::ratio();

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        angle += speed * Time::delta_time();

        glm::vec3 a = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::mat4 m = glm::rotate(glm::mat4(1.0f), glm::radians(angle), a);
        glm::mat4 p = glm::ortho(-ratio, ratio, -1.0f, 1.0f, 1.0f, -1.0f);
        glm::mat4 mvp = p * m;

        shader.bind();
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &mvp);

        glBindVertexArray(vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        platform_manager.update();
    }

    core_manager.release();
    platform_manager.release();

    exit(EXIT_SUCCESS);
}