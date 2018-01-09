#version 420

uniform usampler2D array_ranges;

layout(binding = 0, r32ui) uniform restrict uimage2D array_alloc_pointer;
layout(binding = 1, r32f ) uniform restrict image2D depth_arrays;
uniform uvec4 heap_info;

noperspective in vec2 coords;

out uint packed_array_range;

#include utils_f

void main()
{
    float min_z = 2.0;
    uvec4 ranges = textureGather(array_ranges, coords);
    for (int i = 0; i < 4; ++i)
    {
        uint range = ranges[i];
        if (range == 0u)
            continue;
        ivec2 coords = ivec2(unpack_range(range));
        float z = imageLoad(depth_arrays, coords)[0];
        min_z = min(z, min_z);
    }

    if (min_z == 2.0)
    {
        packed_array_range = 0;
        return;
    }

    uint layer_count = 1;
    uvec3 array_range = alloc_range(array_alloc_pointer, heap_info, layer_count);

    imageStore(depth_arrays, ivec2(array_range),
        vec4(min_z, 0.0, 0.0, 0.0));

    packed_array_range = pack_range(array_range);
}
