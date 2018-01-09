#version 420

#define HIERARCHICAL

const int MAX_ABUFFER_LEVELS = 8;

uniform usampler2D array_ranges;
uniform sampler2D depth_arrays;
uniform sampler2D color_arrays;
uniform mat4 bake_projection;
uniform float bake_nearz;
uniform int iterations; // TODO: check if uniform vs constant makes difference

uniform vec4 level_infos[MAX_ABUFFER_LEVELS];
uniform int max_level;

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

#ifndef HIERARCHICAL
    if (!cast_ray(
            ray_origin, ray_direction,
            array_ranges, depth_arrays,
            iterations,
            color))
#else
    if (!cast_ray_hierarchical(
            ray_origin, ray_direction,
            6, iterations,
            color))
#endif
        color = checker_color();
}
