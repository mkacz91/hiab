#version 420

uniform mat4 in_camera;
uniform mat4 out_camera;

in vec4 position;

out vec4 color;

void main()
{
    gl_Position = out_camera * inverse(in_camera) * position;
    color = 0.5 * position + 0.5;
}
