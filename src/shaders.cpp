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
    load_uniform(heap_info);
    load_attrib(position);
    load_attrib(normal);
    load_attrib(uv);
}

Layer0Program::Layer0Program() :
    ShaderProgram("position4_v", "layer0_f")
{
    load_uniform(nodes);
    load_uniform(heads);
    load_uniform(heap_info);
    load_attrib(position);
}

HeadsProgram::HeadsProgram()
    : ShaderProgram("position4_v", "heads_f")
{
    load_uniform(heads);
    load_uniform(viewport_size);
    load_attrib(position);
}

TracePreviewProgram::TracePreviewProgram()
    : ShaderProgram("trace_preview_v", "trace_preview_f")
{
    load_uniform(array_ranges);
    load_uniform(depth_arrays);
    load_uniform(viewport_to_bake_view);
    load_uniform(bake_projection);
    load_uniform(bake_nearz);
    load_attrib(viewport_position);
}

FrustumProgram::FrustumProgram()
    : ShaderProgram("frustum_v", "varying4_f")
{
    load_uniform(in_camera);
    load_uniform(out_camera);
    load_attrib(position);
}

DownsampleProgram::DownsampleProgram()
    : ShaderProgram("downsample_v", "downsample_f")
{
    load_uniform(array_ranges);
    load_uniform(heap_info);
    load_uniform(coord_adjust);
    load_attrib(viewport_position);
}

} // namespace hiab
