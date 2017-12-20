#pragma once

#include "prefix.h"
#include <glad.h>
#include <GLFW/glfw3.h>

namespace hiab {

class gl_exception : public std::runtime_error
{
public:
    gl_exception(string const& message);
    gl_exception(string const& message, GLenum error_code);

    GLenum error_code() const;
    string error_string() const;

private:
    GLenum m_error_code = GL_NO_ERROR;
};

#define gl_if_error(call) \
    glGetError(); \
    call; \
    for (GLenum error = glGetError(); error != GL_NO_ERROR; error = GL_NO_ERROR)

#define gl_error_guard(call) \
    gl_if_error (call) \
        throw ::hiab::gl_exception("Error during " + #call, error);

GLuint gl_load_shader(const string& name, GLenum shader_type);

GLuint gl_load_vertex_shader(const string& name);

GLuint gl_load_fragment_shader(const string& name);

GLuint gl_link_program(GLuint vertex_shader, GLuint fragment_shader);

GLuint gl_link_program(const string& vertex_shader_name, const string& fragment_shader_name);

GLint gl_get_uniform_location(GLuint program, const char* name);

GLint gl_get_attrib_location(GLuint program, const char* name);

string gl_shader_name(GLuint shader);

string gl_enum_string(GLenum value);

}
