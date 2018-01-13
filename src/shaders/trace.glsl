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

// TODO: There are still some lone pixels that are somehow being missed.
bool cast_ray_hierarchical(
    vec3 ray_origin, vec3 ray_direction,
    int level, int iterations,
    out vec4 color)
{
    // TODO: Move to outer scope.
    ray_origin = 0.5 * ray_origin + 0.5;
    ray_direction *= 0.5;

    // Clamping the division to avoid infinities. This may result in
    // underestimation of the checked range, but it's ok. Otherwise may yield
    // NaN when multiplied by 0.
    const vec3 inv_direction = clamp(1.0 / ray_direction, MIN_FLOAT, MAX_FLOAT);
    const vec3 direction_sign = sign(ray_direction);
    const vec3 frustum_in = -min(direction_sign, 0.0); // Other way may yeild infinite in_dt.
    const vec3 frustum_out = 1.0 - frustum_in;
    const float default_target_z = 0.5 + 2.0 * (frustum_out.z - 0.5); // Note the margin.
    const vec2 target_bias = frustum_out.xy; // Coincidental equality

    // TODO: Move to outer scope.
    float in_dt = max_component(vec4(
        inv_direction * (frustum_in - ray_origin), 0.0));
    ray_origin += in_dt * ray_direction;

    vec3 p = ray_origin;
    vec2 sample_bias = vec2(0.0); // Helps with p on texel boundary.
    while (iterations > 0 && all_positive((frustum_out - p) * direction_sign))
    {
        level = min(max_level, level);
        vec2 texel_size = level_infos[level].xy;
        vec2 sample_adjust = level_infos[level].zw;

        vec2 sample_p = p.xy + texel_size * sample_bias;
        vec3 target = vec3(
            texel_size * (floor(sample_p / texel_size) + target_bias),
            default_target_z);
        ivec2 array_index = ivec2(0);
        uint packed_range = textureLod(
            array_ranges, sample_adjust * sample_p, float(level))[0];
        if (packed_range != 0u) // TODO: Maybe we can get rid of the branch?
        {
            uvec3 range = unpack_range(packed_range);
            array_index = ivec2(range);
            target.z = texelFetch(depth_arrays, array_index, 0)[0];
        }

        vec3 dts = inv_direction * (target - p);
        float xy_dt = min(dts.x, dts.y);
        float dt = 0.0;
        bool yolo = false;
        if (dts.z <= xy_dt)
        {
            if (level == 0)
            {
                color = texelFetch(color_arrays, array_index, 0);
                return packed_range != 0u; // TODO: Try eliminating this check.
            }
            if (dts.z > 0.0) // TODO: Try eliminating this check.
            {
                dt = dts.z;
                sample_bias = vec2(0.0);
            }
            --level;
        }
        else
        {
            dt = xy_dt;
            vec2 step_mask = sign(xy_dt - dts.xy) + 1.0;
            sample_bias = 0.25 * direction_sign.xy * step_mask;
            ++level;
        }

        p += dt * ray_direction;
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
