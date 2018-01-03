#version 420

in vec2 viewport_position;

noperspective out vec2 base_coords[4];

void main()
{
    gl_Position = vec4(viewport_position, 0.0, 1.0);
    vec2 coords0 = 0.5 * viewport_position + 0.5;
    base_coords[0] = coords0;
    base_coords[1] = coords0;
    base_coords[2] = coords0;
    base_coords[3] = coords0;
}
