#version 420

uniform vec2 coord_adjust;

in vec2 viewport_position;

noperspective out vec2 coords;

void main()
{
    gl_Position = vec4(viewport_position, 0.0, 1.0);
    coords = (0.5 * viewport_position + 0.5) / coord_adjust;
}
