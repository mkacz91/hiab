#version 420

uniform usampler2D heads;

out vec4 color;

void main()
{
     uint value = texelFetch(heads, ivec2(gl_FragCoord), 0).r;
     color = vec4(float(min(5, value)) / 5.0);
}
