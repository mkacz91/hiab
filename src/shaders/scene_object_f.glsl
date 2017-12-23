#version 330

in vec3 frag_normal;
in vec2 frag_uv;

void main()
{
  gl_FragColor = vec4(0.5 * (normalize(frag_normal) + 1.0), 1.0);
}
