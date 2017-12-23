#version 420

uniform mat4 camera;
uniform mat4 transform;

in vec4 position;
in vec3 normal;
in vec2 uv;

out vec3 frag_normal;
out vec2 frag_uv;

void main()
{
  frag_normal = normal;
  frag_uv = uv;
  gl_Position = camera * transform * position;
}
