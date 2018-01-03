#version 420

const int MAX_LAYER_COUNT = 16;

float depths[MAX_LAYER_COUNT];
uint colors[MAX_LAYER_COUNT];

uniform usampler2D nodes;
uniform usampler2D heads;

layout(binding = 0, r32ui) uniform restrict uimage2D array_alloc_pointer;
layout(binding = 1, r32f) uniform restrict writeonly image2D depth_arrays;
uniform uvec4 heap_info;

out uint packed_array_range;

#include utils_f

void main()
{
    uint pnode = texelFetch(heads, ivec2(gl_FragCoord), 0).r;
    if (pnode == 0u)
    {
        // TODO: Benchmark against framebuffer clear and discard
        packed_array_range = 0;
        return;
    }

    int layer_count = 0;
    while (layer_count < MAX_LAYER_COUNT && pnode != 0u)
    {
        uvec4 node = texelFetch(nodes, ivec2(pnode & 0xFFFF, pnode >> 16), 0);
        depths[layer_count] = uintBitsToFloat(node[0]);
        colors[layer_count] = node[2];
        pnode = node[3];
        ++layer_count;
    }

    const int sort_iteration_count = layer_count - 1;
    for (int i = 0; i < sort_iteration_count; ++i)
    {
        float min_depth = depths[i];
        int jbest = i;
        for (int j = i + 1; j < layer_count; ++j)
        {
            if (min_depth > depths[j])
            {
                min_depth = depths[j];
                jbest = j;
            }
        }

        depths[jbest] = depths[i];
        depths[i] = min_depth;

        uint tmp_color = colors[jbest];
        colors[jbest] = colors[i];
        colors[i] = tmp_color;
    }

    uvec3 array_range = alloc_range(
        array_alloc_pointer, heap_info, layer_count);
    for (int i = 0; i < layer_count; ++i)
    {
        ivec2 coords = ivec2(array_range[0] + i, array_range[1]);
        imageStore(depth_arrays, coords, vec4(depths[i], 0, 0, 0));
    }

    packed_array_range = pack_range(array_range);
}
