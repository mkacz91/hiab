#version 420

uniform mat4 bake_projection;
uniform float bake_nearz;

in vec3 eye_ray_origin;
in vec3 eye_ray_direction;

out vec4 color;

#include utils_f
#include trace

void main()
{
    vec3 ray_origin = eye_ray_origin;
    vec3 ray_direction = eye_ray_direction;
    if (!clip_ray_z(ray_origin, ray_direction, bake_nearz))
    {
        color = checker_color();
        return;
    }
    perspective_transform_ray(bake_projection, ray_origin, ray_direction);
    if (!cast_ray(ray_origin, ray_direction, color))
        color = checker_color();
}
