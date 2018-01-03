#pragma once

#include "prefix.h"
#include "opengl.h"

namespace hiab {

struct ShaderProgram
{
    GLuint id;

    ShaderProgram(GLuint id);
    ShaderProgram(
        string const& vertex_shader_name, string const& fragment_shader_name);
    ShaderProgram(ShaderProgram const& other) = delete;
    ~ShaderProgram();
};

struct ObjectProgram : public ShaderProgram
{
    GLint camera;
    GLint transform;
    GLint heap_info;
    GLint position;
    GLint normal;
    GLint uv;

    ObjectProgram();
};

struct Layer0Program : public ShaderProgram
{
    GLint heads;
    GLint nodes;
    GLint position;
    GLint heap_info;

    Layer0Program();
};

struct HeadsProgram : public ShaderProgram
{
    GLint heads;
    GLint viewport_size;
    GLint position;

    HeadsProgram();
};

struct TracePreviewProgram : public ShaderProgram
{
    GLint array_ranges;
    GLint depth_arrays;
    GLint viewport_to_bake_view;
    GLint bake_projection;
    GLint bake_nearz;
    GLint viewport_position;

    TracePreviewProgram();
};

struct FrustumProgram : public ShaderProgram
{
    GLint in_camera;
    GLint out_camera;
    GLint position;

    FrustumProgram();
};

struct DownsampleProgram : public ShaderProgram
{
    GLint array_ranges;
    GLint base_level;
    GLint viewport_position;

    DownsampleProgram();
};

} // namespace hiab
