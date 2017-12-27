#version 420

layout (binding = 0) uniform atomic_uint node_alloc_pointer;
layout (binding = 0, rgba32ui) uniform restrict writeonly uimage2D nodes;
layout (binding = 1, r32ui) uniform restrict uimage2D heads;

uniform uvec4 heap_info;

in vec3 frag_normal;
in vec2 frag_uv;

out vec4 color;

void main()
{
    color = vec4(0.5 * (normalize(frag_normal) + 1.0), 1.0);

    uint head = atomicCounterIncrement(node_alloc_pointer);
    if (head < heap_info[0])
    {
        uint headx = head & heap_info[2];
        uint heady = head >> heap_info[3];
        head = headx | (heady << 16);
        uint next = imageAtomicExchange(heads, ivec2(gl_FragCoord), head);

        uint udepth = floatBitsToUint(gl_FragCoord.z);
        uint ucolor = packUnorm4x8(color);

        imageStore(nodes, ivec2(headx, heady), uvec4(
            udepth, 0, ucolor, next));
    }
}
