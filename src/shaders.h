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
    GLint position;

    HeadsProgram();
};

} // namespace hiab
