#pragma once

#include "prefix.h"
#include "opengl.h"

namespace hiab {

struct Scene;
struct ObjectProgram;

struct RendererBuffers
{
    static constexpr int COUNT = 1;
    GLuint counter = 0;
};

struct Renderer
{
    int framebuffer_width, framebuffer_height;
    ObjectProgram* object_program;
    RendererBuffers buffers;
};

void init_renderer(Renderer* renderer);

void close_renderer(Renderer* renderer);

void set_framebuffer_size(Renderer* r, int width, int height);

void render(Renderer* renderer, Scene const* scene);

} // namespace hiab
