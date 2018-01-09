// Retrieves the intersection of ray, defined by `ray_origin` and
// `ray_direction`, with the scene. (TODO: describe how scene is defined)
bool cast_ray(
    vec3 ray_origin, vec3 ray_direction,
    usampler2D array_ranges, sampler2D depth_arrays,
    int iterations,
    out vec4 color)
{
    // Clamping the division to avoid infinities. This may result in
    // underestimation of the checked range, but it's ok. Otherwise may yield
    // NaN when multiplied by 0.
    vec3 inv_direction = clamp(1.0 / ray_direction, -3.0e38, +3.0e38);

    // Compute the params `t0` and `t1` where the ray enters and leaves scene
    // frustum, respectively. The frustum is a cube in current coordinate
    // system.
    vec3 positive_intersections = inv_direction * (+1.0 - ray_origin);
    vec3 negative_intersections = inv_direction * (-1.0 - ray_origin);
    vec3 t0s = min(positive_intersections, negative_intersections);
    vec3 t1s = max(positive_intersections, negative_intersections);
    float t0 = max(max(t0s.x, t0s.y), t0s.z);
    float t1 = min(min(t1s.x, t1s.y), t1s.z);
    t0 = max(0.0, t0);
    if (t0 >= t1)
        return false;

    vec3 p = ray_origin + t0 * ray_direction;
    vec3 dp = (t1 - t0) / float(iterations) * ray_direction;

    p = 0.5 + 0.5 * p;
    dp *= 0.5;

    while (iterations --> 0)
    {
        p += dp;
        uint array_range = textureLod(array_ranges, p.xy, 0.0)[0];
        if (array_range == 0u)
            continue;
        ivec2 array_index = ivec2(unpack_range(array_range));
        float z = texelFetch(depth_arrays, array_index, 0)[0];
        if (z < p.z && z > p.z - 0.01)
        {
            color = vec4(abs(p.z - z) * 100, 0, 0, 1);
            return true;
        }
    }

    return false;
}

bool cast_ray_hierarchical(
    vec3 ray_origin, vec3 ray_direction,
    int level, int iterations,
    out vec4 color)
{
    // TODO: get rid of this
    ray_origin = 0.5 * ray_origin + 0.5;
    ray_direction *= 0.5;

    // Clamping the division to avoid infinities. This may result in
    // underestimation of the checked range, but it's ok. Otherwise may yield
    // NaN when multiplied by 0.
    vec3 inv_direction = clamp(1.0 / ray_direction, -3.0e38, +3.0e38);

    // TODO: Find a better way to ensure proper stop choice
    vec2 stop_bias = max(sign(ray_direction.xy), -0.2) + 0.1;
    float zstop = max(sign(ray_direction.z), 0.0);
    vec3 p0 = ray_origin;
    while (iterations > 0)
    {
        level = min(max_level, level);
        vec2 texel_size = level_infos[level].xy;
        vec2 coord_adjust = level_infos[level].zw;

        vec3 next_stop = vec3(
            texel_size * floor(p0.xy / texel_size + stop_bias), zstop);
        float dt = min_component(inv_direction * (next_stop - p0));
        vec3 p1 = p0 + dt * ray_direction;
        // TODO: Embed 0.5 in coord_adjust
        // TODO: Try eliminating clamping
        vec2 coord = clamp(0.5 * coord_adjust * (p0.xy + p1.xy), 0.0, 1.0);
        uint packed_range = textureLod(array_ranges, coord, float(level))[0];
        bool hit = false;
        float z;
        uvec3 range;
        // TODO: try using special value at 0 to avoid branching
        if (packed_range != 0u)
        {
            range = unpack_range(packed_range);
            z = texelFetch(depth_arrays, ivec2(range), 0)[0];
            hit = p1.z >= z || p1.z >= z;
        }
        if (hit)
        {
            if (level == 0)
            {
                color = texelFetch(color_arrays, ivec2(range), 0);
                return true;
            }
            --level;
            // TODO: Move p0 to z-plane intersection
        }
        else
        {
            p0 = p1;
            ++level;
        }
        --iterations;
    }

    return false;
}

// If `origin.z > clipz`, the origin is moved along the positive `direction`
// onto the specified Z-plane and result is `true`. Result is `false` if such
// operation is not possible due to `direction` being parallel to or pointing
// away from the plane.
bool clip_ray_z(
    inout vec3 origin, in vec3 direction, in float clipz)
{
    float dz = clipz - origin.z;
    if (dz >= 0.0)
        return true;
    if (direction.z > -0.001)
        return false;
    origin += (dz / direction.z) * direction;
    return true;
}

// Transforms a cartesian ray `origin + t * direction` using a `matrix`,
// correctly handling the projective component of the transformation.
//
// The result is mapped back into the cartesian space using homogeneous divide,
// so caution is required for `origin` not to be transformed into a point at
// infinity. In particular, if applying a standard projection matrix, make sure
// that `origin.z != 0`. You can use, for example, use `clip_ray_z` to clip
// against the projection near plane.
void perspective_transform_ray(
     in mat4 matrix, inout vec3 origin, inout vec3 direction)
{
    vec4 origin_h = matrix * vec4(origin, 1.0);
    vec4 direction_h = matrix * vec4(direction, 0.0);
    origin = origin_h.xyz / origin_h.w;
    direction = direction_h.xyz - direction_h.w * origin;
}
