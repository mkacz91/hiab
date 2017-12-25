#version 420

layout (binding = 0) uniform atomic_uint counter;
layout (binding = 0, r32ui) uniform restrict uimage2D head;

in vec3 frag_normal;
in vec2 frag_uv;

out vec4 color;

void main()
{
  imageAtomicAdd(head, ivec2(gl_FragCoord), 1);
  atomicCounterIncrement(counter);
  color = vec4(0.5 * (normalize(frag_normal) + 1.0), 1.0);
}
