#version 420

uniform mat4 viewport_to_bake_view;

in vec2 viewport_position;

out vec3 eye_ray_origin;
out vec3 eye_ray_direction;

void main()
{
    gl_Position = vec4(viewport_position, 0.0, 1.0);
    eye_ray_origin =
        vec3(viewport_to_bake_view * vec4(0.0, 0.0, 0.0, 1.0));
    eye_ray_direction =
        vec3(viewport_to_bake_view * vec4(viewport_position, -1.0, 0.0));
}
