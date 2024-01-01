#version 460

layout (location = 0) in vec2 vPos;
layout (location = 1) in vec3 vCol;

layout (binding = 0, std140) uniform u_matrices
{
    mat4 MVP;
};

out vec3 color;

void main()
{
    gl_Position = MVP * vec4(vPos, 0.0, 1.0);
    color = vCol;
}