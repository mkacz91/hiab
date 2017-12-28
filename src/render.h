#pragma once

#include "prefix.h"
#include "opengl.h"

namespace hiab {

struct Scene;
struct Camera;
struct ObjectProgram;
struct Layer0Program;
struct HeadsProgram;

struct HeapInfo
{
    GLuint size;
    GLuint width;
    GLuint xmask;
    GLuint yshift;
};

struct Renderer
{
    int viewport_width, viewport_height;
    bool viewport_changed;
    int avg_layers_per_pixel;

    HeapInfo heap_info;

    struct
    {
        ObjectProgram* object;
        Layer0Program* layer0;
        HeadsProgram* heads;
    } programs;
    static constexpr int PROGRAM_COUNT =
        sizeof(Renderer::programs) / sizeof(void*);

    struct
    {
        GLuint viewport_vertices;
        GLuint node_alloc_pointer;
    } buffers;
    static constexpr int BUFFER_COUNT =
        sizeof(Renderer::buffers) / sizeof(GLuint);

    struct
    {
        GLuint nodes;
        GLuint heads;
        GLuint array_alloc_pointer;
        GLuint array_ranges;
        GLuint depth_arrays;
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

void set_renderer_viewport(Renderer* renderer, int width, int height);

void render(Renderer* renderer, Scene const* scene, Camera const* camera);

} // namespace hiab
