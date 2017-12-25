#include "shaders.h"

namespace hiab {

ShaderProgram::ShaderProgram(GLuint id) : id(id) { }

ShaderProgram::ShaderProgram(
    string const& vertex_shader_name, string const& fragment_shader_name) :
    ShaderProgram(gl_link_program(vertex_shader_name, fragment_shader_name))
{ }

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(id);
}

#define load_uniform(name) name = gl_get_uniform_location(id, #name);
#define load_attrib(name) name = gl_get_attrib_location(id, #name);

ObjectProgram::ObjectProgram() :
    ShaderProgram("scene_object_v", "scene_object_f")
{
    load_uniform(camera);
    load_uniform(transform);
    load_attrib(position);
    load_attrib(normal);
    load_attrib(uv);
}

HeadsProgram::HeadsProgram()
    : ShaderProgram("position4_v", "heads_f")
{
    load_uniform(heads);
    load_attrib(position);
}

} // namespace hiab
