#pragma once

#include "prefix.h"
#include "opengl.h"

namespace hiab {

struct Scene;
struct ObjectProgram;
struct HeadsProgram;

struct Renderer
{
    int framebuffer_width, framebuffer_height;
    bool framebuffer_size_changed;

    struct
    {
        ObjectProgram* object;
        HeadsProgram* heads;
    } programs;
    static constexpr int PROGRAM_COUNT =
        sizeof(Renderer::programs) / sizeof(void*);

    struct
    {
        GLuint viewport_vertices;
        GLuint counter;
    } buffers;
    static constexpr int BUFFER_COUNT =
        sizeof(Renderer::buffers) / sizeof(GLuint);

    struct
    {
        GLuint heads;
    } textures;
    static constexpr int TEXTURE_COUNT =
        sizeof(Renderer::textures) / sizeof(GLuint);

    struct
    {
        GLuint clear_heads;
    } framebuffers;
    static constexpr int FRAMEBUFFER_COUNT =
        sizeof(Renderer::framebuffers) / sizeof(GLuint);
};

void init_renderer(Renderer* renderer);

void close_renderer(Renderer* renderer);

void set_framebuffer_size(Renderer* r, int width, int height);

void render(Renderer* renderer, Scene const* scene);

} // namespace hiab
