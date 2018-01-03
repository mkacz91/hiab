#pragma once

#include "prefix.h"
#include "opengl.h"
#include "math.h"

namespace hiab {

struct Scene;
struct Camera;
struct ObjectProgram;
struct Layer0Program;
struct HeadsProgram;
struct TracePreviewProgram;
struct FrustumProgram;
struct DownsampleProgram;

struct HeapInfo
{
    GLuint size;
    GLuint width;
    GLuint xmask;
    GLuint yshift;
};

struct Viewport
{
    int x, y, width, height;
};

struct Renderer
{
    static constexpr int REFERENCE_ABUFFER_HIERARCHY_LEVEL_COUNT = 4;

    Viewport viewport;
    bool viewport_changed;
    int avg_layers_per_pixel;
    int abuffer_hierarchy_level_count;

    HeapInfo heap_info;

    struct
    {
        ObjectProgram* object;
        Layer0Program* layer0;
        HeadsProgram* heads;
        TracePreviewProgram* trace_preview;
        FrustumProgram* frustum;
        DownsampleProgram* downsample;
    } programs;
    static constexpr int PROGRAM_COUNT =
        sizeof(Renderer::programs) / sizeof(void*);

    struct
    {
        GLuint viewport_vertices;
        GLuint node_alloc_pointer;
        GLuint frustum_vertices;
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
        GLuint write_array_ranges;
    } framebuffers;
    static constexpr int FRAMEBUFFER_COUNT =
        sizeof(Renderer::framebuffers) / sizeof(GLuint);
};

struct TracePreview
{
    mat4f bake_view;
    mat4f bake_projection;
    float bake_nearz;
};

void init_renderer(Renderer* renderer);

void close_renderer(Renderer* renderer);

void set_renderer_viewport(Renderer* renderer, Viewport viewport);

void render_scene(Renderer* renderer, Scene const* scene, Camera const* camera);

TracePreview* init_trace_preview(
    Renderer const* renderer, Camera const* camera);

void render_trace_preview(
    Renderer* renderer, TracePreview const* preview, Camera const* camera);

void render_frustum(
    Renderer* renderer, mat4f const& in_camera, Camera const* camera);

} // namespace hiab
