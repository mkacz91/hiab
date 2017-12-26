#version 420

uniform usampler2D heads;

out vec4 color;

void main()
{
    uvec4 value = texelFetch(heads, ivec2(gl_FragCoord), 0);
    color = unpackUnorm4x8(value.r);
}
