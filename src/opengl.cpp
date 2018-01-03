#include "opengl.h"
#include <unordered_map>
#include <sstream>
#include "files.h"

namespace hiab {

std::unordered_map<GLuint, string> gl_shader_names;

gl_exception::gl_exception(string const& message) :
    runtime_error(message)
{ }

gl_exception::gl_exception(string const& message, GLenum error_code) :
    runtime_error(message + " " + paren(gl_enum_string(error_code))),
    m_error_code(error_code)
{ }

GLenum gl_exception::error_code() const { return m_error_code; }

string gl_exception::error_string() const { return gl_enum_string(error_code()); }

void gl_load_preprocessed_shader_source(
    string const& name, std::ostringstream& ostr, string& line)
{
    auto fin = open_file_for_reading(name + ".glsl");
    while (std::getline(fin, line))
    {
        if (string_starts_with(line, "#include "))
        {
            string include_name = line.substr(9);
            gl_load_preprocessed_shader_source(include_name, ostr, line);
        }
        else
        {
            ostr << line << '\n';
        }
    }
}

string gl_load_preprocessed_shader_source(string const& name)
{
    std::ostringstream ostr;
    string line;
    gl_load_preprocessed_shader_source(name, ostr, line);
    return ostr.str();
}

GLuint gl_load_shader(const string& name, GLenum shader_type)
{
    auto source = gl_load_preprocessed_shader_source(name);

    // Create shader object
    GLuint shader = glCreateShader(shader_type);
    if (shader == 0)
        throw gl_exception("Unable to create new shader", glGetError());

    // Upload source and compile
    char const* source_data = source.c_str();
    int source_size = (int)source.length();
    glShaderSource(shader, 1, &source_data, &source_size);
    glCompileShader(shader);

    int compile_status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
    if (compile_status != GL_TRUE)
    {
        const int max_log_length = 255;
        char log[max_log_length + 1];
        glGetShaderInfoLog(shader, max_log_length, nullptr, (char*)&log);
        glDeleteShader(shader);
        throw gl_exception("Shader " + squote(name) + ", compilation error: " + log);
    }

    gl_shader_names[shader] = name;
    return shader;
}

GLuint gl_load_vertex_shader(const string& name)
{
    return gl_load_shader(name, GL_VERTEX_SHADER);
}

GLuint gl_load_fragment_shader(const string& name)
{
    return gl_load_shader(name, GL_FRAGMENT_SHADER);
}

GLuint gl_link_program(GLuint vertex_shader, GLuint fragment_shader)
{
    // Create program
    GLuint program = glCreateProgram();
    if (program == 0)
        throw gl_exception("Unable to create new program");

    // Attach shaders
    gl_if_error (
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
    ) {
        glDeleteProgram(program);
        throw gl_exception(
            "Unable to attach shaders " + squote(gl_shader_name(vertex_shader)) + ", " +
            squote(gl_shader_name(fragment_shader)) + ": "+ gl_enum_string(error)
        );
    }

    // Link program
    glLinkProgram(program);
    int link_status;
    glGetProgramiv(program, GL_LINK_STATUS, &link_status);
    if (link_status != GL_TRUE)
    {
        const int max_log_length = 255;
        char log[max_log_length + 1];
        glGetProgramInfoLog(program, max_log_length, nullptr, (char*)&log);
        glDeleteProgram(program);
        throw gl_exception(
            "Unable to link shaders " + squote(gl_shader_name(vertex_shader)) + ", " +
            squote(gl_shader_name(fragment_shader)) + ": " + log
        );
    }

    return program;
}

GLuint gl_link_program(const string& vertex_shader_name, const string& fragment_shader_name)
{
    GLuint vertex_shader = 0;
    GLuint fragment_shader = 0;
    try
    {
        vertex_shader = gl_load_vertex_shader(vertex_shader_name);
        fragment_shader = gl_load_fragment_shader(fragment_shader_name);
        GLuint program = gl_link_program(vertex_shader, fragment_shader);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return program;
    }
    catch (...)
    {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        throw;
    }
}

#define gl_get_location(glFunction) \
    gl_if_error (GLint location = glFunction(program, name)) \
    { \
        throw gl_exception( \
            string("Error during ") + #glFunction + "(" + \
            to_string(program) + ", " + quote(name) + ")"); \
    } \
    return location

GLint gl_get_uniform_location(GLuint program, const char* name)
{
    gl_get_location(glGetUniformLocation);
}

GLint gl_get_attrib_location(GLuint program, const char* name)
{
    gl_get_location(glGetAttribLocation);
}

#undef gl_get_location

string gl_shader_name(GLuint shader)
{
    auto result_it = gl_shader_names.find(shader);
    return result_it != gl_shader_names.end() ? result_it->second : "unknown_shader";
}

string gl_enum_string(GLenum value)
{
#   define gl_enum_string_case(constant_name) case constant_name: return #constant_name
    switch (value)
    {
        gl_enum_string_case(GL_NO_ERROR);
        gl_enum_string_case(GL_INVALID_ENUM);
        gl_enum_string_case(GL_INVALID_VALUE);
        gl_enum_string_case(GL_INVALID_OPERATION);
        gl_enum_string_case(GL_OUT_OF_MEMORY);
        gl_enum_string_case(GL_FRAMEBUFFER_COMPLETE);
        gl_enum_string_case(GL_FRAMEBUFFER_UNDEFINED);
        gl_enum_string_case(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
        gl_enum_string_case(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
        gl_enum_string_case(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
        gl_enum_string_case(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
        gl_enum_string_case(GL_FRAMEBUFFER_UNSUPPORTED);
        gl_enum_string_case(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);
        gl_enum_string_case(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS); 

        default: return "Unknown GL constant " + to_string(value);
    }
#   undef gl_enum_string_case
}

} // namespace hiab
