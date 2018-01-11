vec4 checker_color()
{
    bool cx = fract(gl_FragCoord.x / 20.0) < 0.5;
    bool cy = fract(gl_FragCoord.y / 20.0) < 0.5;
    return cx && !cy || !cx && cy
        ? vec4(0.6, 0.6, 0.6, 1.0)
        : vec4(0.5, 0.5, 0.5, 1.0);
}

// TODO: lol. just try shader storage buffer objects or texture buffer objects
uvec3 alloc_range(layout(r32ui) uimage2D alloc_pointer, uvec4 heap_info, uint size)
{
    const uint max_startx = heap_info[1] - size;
    uint start;
    uint startx = max_startx;
    while (startx >= max_startx)
    {
        // TODO: Try using atomic counter ops if you manage to get a capable
        // machine
        start = imageAtomicAdd(alloc_pointer, ivec2(0), size);
        startx = start & heap_info[2];
    }
    uint starty = start >> heap_info[3];
    return uvec3(startx, starty, size);
}

uint pack_range(uint startx, uint starty, uint count)
{
    return startx | (starty << 14) | (count << 27);
}

uint pack_range(uvec3 range)
{
    return pack_range(range[0], range[1], range[2]);
}

uvec3 unpack_range(uint range)
{
    return uvec3(range & 0x3FFF, (range >> 14) & 0x1FFF, range >> 27);
}

float min_component(vec3 u)
{
    return min(u.x, min(u.y, u.z));
}

float max_component(vec4 u)
{
    return max(max(u.x, u.y), max(u.z, u.w));
}

bool all_positive(vec3 u)
{
    // TODO: Check if non-SIMD would be faster
    return dot(min(u, 0.0), vec3(1.0)) == 0.0;
}
