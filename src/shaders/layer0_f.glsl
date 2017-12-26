#version 420

const int MAX_LAYER_COUNT = 16;

float depths[MAX_LAYER_COUNT];
uint colors[MAX_LAYER_COUNT];

uniform usampler2D nodes;
uniform usampler2D heads;

out vec4 color;

void main()
{
    uint pnode = texelFetch(heads, ivec2(gl_FragCoord), 0).r;
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

    int k = 1;
    if (k < layer_count)
        color = unpackUnorm4x8(colors[k]);
    else
        color = vec4(0);
}
