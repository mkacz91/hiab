#version 420

layout (binding = 0) uniform atomic_uint counter;

in vec3 frag_normal;
in vec2 frag_uv;

void main()
{
  atomicCounterIncrement(counter);
  gl_FragColor = vec4(0.5 * (normalize(frag_normal) + 1.0), 1.0);
}
