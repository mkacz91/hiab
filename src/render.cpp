#include "render.h"
#include "scene.h"
#include "shaders.h"
#include "math.h"

#include <iostream>

namespace hiab {

void init_renderer(Renderer* r)
{
    r->viewport = { 0, 0, 0, 0 };
    r->viewport_changed = true;

    r->avg_layers_per_pixel = 2;

    r->programs.object = new ObjectProgram;
    r->programs.layer0 = new Layer0Program;
    r->programs.heads = new HeadsProgram;
    r->programs.trace_preview = new TracePreviewProgram;
    r->programs.frustum = new FrustumProgram;
    r->programs.downsample = new DownsampleProgram;

    glGenBuffers(
        Renderer::BUFFER_COUNT, reinterpret_cast<GLuint*>(&r->buffers));

    GLfloat viewport_vertices[] =
    {
         0.0f,  3.1f,
        -2.1f, -1.1f,
         2.1f, -1.1f
    };
    glBindBuffer(GL_ARRAY_BUFFER, r->buffers.viewport_vertices);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(viewport_vertices), &viewport_vertices, GL_STATIC_DRAW);

    GLfloat frustum_vertices[] =
    {
        -1, -1, -1, +1, -1, -1,
        -1, -1, +1, +1, -1, +1,
        -1, +1, -1, +1, +1, -1,
        -1, +1, +1, +1, +1, +1,

        -1, -1, -1, -1, +1, -1,
        -1, -1, +1, -1, +1, +1,
        +1, -1, -1, +1, +1, -1,
        +1, -1, +1, +1, +1, +1,

        -1, -1, -1, -1, -1, +1,
        -1, +1, -1, -1, +1, +1,
        +1, -1, -1, +1, -1, +1,
        +1, +1, -1, +1, +1, +1
    };
    glBindBuffer(GL_ARRAY_BUFFER, r->buffers.frustum_vertices);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(frustum_vertices), &frustum_vertices, GL_STATIC_DRAW);

    auto textures = reinterpret_cast<GLuint*>(&r->textures);
    glGenTextures(Renderer::TEXTURE_COUNT, textures);

    for (int i = 0; i < Renderer::TEXTURE_COUNT; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        // Default filtering modes may cause the textures to be incomplete under
        // certain circumstances. Overwrite with safer ones, to spare debugging.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    glGenFramebuffers(
        Renderer::FRAMEBUFFER_COUNT, reinterpret_cast<GLuint*>(&r->framebuffers));
}

void close_renderer(Renderer* r)
{
    auto programs = reinterpret_cast<ShaderProgram**>(&r->programs);
    for (int i = 0; i < Renderer::PROGRAM_COUNT; ++i)
        delete programs[i];

    glDeleteBuffers(
        Renderer::BUFFER_COUNT, reinterpret_cast<GLuint*>(&r->buffers));
    glDeleteTextures(
        Renderer::TEXTURE_COUNT, reinterpret_cast<GLuint*>(&r->textures));
    glDeleteFramebuffers(
        Renderer::FRAMEBUFFER_COUNT, reinterpret_cast<GLuint*>(&r->framebuffers));
}

void set_renderer_viewport(Renderer* r, Viewport viewport)
{
    r->viewport_changed =
        r->viewport.width != viewport.width ||
        r->viewport.height != viewport.height;
    r->viewport = viewport;
}

void apply_viewport_changes(Renderer* r)
{
    if (!r->viewport_changed)
        return;
    r->viewport_changed = false;
    int width = r->viewport.width, height = r->viewport.height;

    glBindTexture(GL_TEXTURE_2D, r->textures.heads);
    glTexImage2D(GL_TEXTURE_2D, 0,
        GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, r->framebuffers.clear_heads);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
        r->textures.heads, 0);

    int min_heap_size = r->avg_layers_per_pixel * width * height;
    int heap_size_exp = 8;
    while (1 << heap_size_exp < min_heap_size)
        ++heap_size_exp;
    int heap_width_exp = (heap_size_exp + 1) / 2;
    int heap_height_exp = heap_size_exp - heap_width_exp;
    int heap_width = 1 << heap_width_exp;
    int heap_height = 1 << heap_height_exp;
    r->heap_info.size = 1 << heap_size_exp;
    r->heap_info.width = heap_width;
    r->heap_info.xmask = ~((~0u) << heap_width_exp);
    r->heap_info.yshift = heap_width_exp;

    glBindTexture(GL_TEXTURE_2D, r->textures.nodes);
    gl_error_guard(glTexImage2D(GL_TEXTURE_2D, 0,
        GL_RGBA32UI, heap_width, heap_height, 0,
        GL_RGBA_INTEGER, GL_UNSIGNED_INT, nullptr));
    glBindTexture(GL_TEXTURE_2D, r->textures.depth_arrays);
    gl_error_guard(glTexImage2D(GL_TEXTURE_2D, 0,
        GL_R32F, heap_width, heap_height, 0,
        GL_RED, GL_FLOAT, nullptr));

    glBindTexture(GL_TEXTURE_2D, r->textures.array_ranges);
    {
        int level = 0;
        while (level < Renderer::REFERENCE_ABUFFER_HIERARCHY_LEVEL_COUNT)
        {
            int level_width = width >> level, level_height = height >> level;
            if (level_width == 0 || level_height == 0)
                break;
            glTexImage2D(
                GL_TEXTURE_2D, level, GL_R32UI, level_width, level_height,
                0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
            ++level;
        }
        r->abuffer_hierarchy_level_count = level;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
            GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level - 1);
    }
}

void render_scene(Renderer* r, Scene const* scene, Camera const* camera)
{
    float t = (float)glfwGetTime();

    apply_viewport_changes(r);
    glViewport(
        r->viewport.x, r->viewport.y, r->viewport.width, r->viewport.height);

    glBindFramebuffer(GL_FRAMEBUFFER, r->framebuffers.clear_heads);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.05f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    mat4f camera_matrix = get_camera_matrix(camera);

    GLuint node_alloc_pointer = 1;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, r->buffers.node_alloc_pointer);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER,
        sizeof(node_alloc_pointer), &node_alloc_pointer, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, r->buffers.node_alloc_pointer);

    glBindImageTexture(
        0, r->textures.nodes, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32UI);
    glBindImageTexture(
        1, r->textures.heads, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

    glUseProgram(r->programs.object->id);
    {
        auto program = r->programs.object;
        glUniformMatrix4fv(program->camera, 1, GL_TRUE, camera_matrix.p());
        glUniform4uiv(program->heap_info, 1, (GLuint*)&r->heap_info);
        glEnableVertexAttribArray(program->position);
        glEnableVertexAttribArray(program->normal);
        for (SceneObject const* object : scene->objects)
        {
            glUniformMatrix4fv(program->transform, 1, GL_TRUE,
                object->transform.p());

            glBindBuffer(GL_ARRAY_BUFFER, object->buffers.positions);
            glVertexAttribPointer(
                program->position, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            glBindBuffer(GL_ARRAY_BUFFER, object->buffers.normals);
            glVertexAttribPointer(
                program->normal, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            glDrawArrays(GL_TRIANGLES, 0, object->vertex_count);
        }
        glDisableVertexAttribArray(program->position);
        glDisableVertexAttribArray(program->normal);
    }

    GLuint array_alloc_pointer = 1;
    glBindTexture(GL_TEXTURE_2D, r->textures.array_alloc_pointer);
    glTexImage2D(GL_TEXTURE_2D, 0,
        GL_R32UI, 1, 1, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &array_alloc_pointer);

    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, r->framebuffers.write_array_ranges);

    glUseProgram(r->programs.layer0->id);
    {
        auto program = r->programs.layer0;
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            r->textures.array_ranges, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, r->textures.nodes);
        glUniform1i(program->nodes, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, r->textures.heads);
        glUniform1i(program->heads, 1);

        glBindImageTexture(0, r->textures.array_alloc_pointer,
            0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
        glBindImageTexture(1, r->textures.depth_arrays,
            0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

        glUniform4uiv(program->heap_info, 1, (GLuint*)&r->heap_info);

        glEnableVertexAttribArray(program->position);
        glBindBuffer(GL_ARRAY_BUFFER, r->buffers.viewport_vertices);
        glVertexAttribPointer(
            program->position, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glDisableVertexAttribArray(program->position);
    }

    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

    glUseProgram(r->programs.downsample->id);
    {
        auto program = r->programs.downsample;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, r->textures.array_ranges);
        glUniform1i(program->array_ranges, 0);

        glBindImageTexture(0, r->textures.array_alloc_pointer,
            0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
        glBindImageTexture(1, r->textures.depth_arrays,
            0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

        glEnableVertexAttribArray(program->viewport_position);
        glBindBuffer(GL_ARRAY_BUFFER, r->buffers.viewport_vertices);
        glVertexAttribPointer(
            program->viewport_position, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        for (int level = 1; level < r->abuffer_hierarchy_level_count; ++level)
        {
            glFramebufferTexture2D(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                r->textures.array_ranges, level);
            int level_width = r->viewport.width >> level;
            int level_height = r->viewport.height >> level;
            glViewport(0, 0, level_width, level_height);
            glUniform1f(program->base_level, float(level - 1));
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        glDisableVertexAttribArray(program->viewport_position);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(
        r->viewport.x, r->viewport.y, r->viewport.width, r->viewport.height);

    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, r->buffers.node_alloc_pointer);
    glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER,
        0, sizeof(node_alloc_pointer), (void*)&node_alloc_pointer);
    std::cout << "Rendered fragments: " << node_alloc_pointer << std::endl;
    std::cout << "Average nodes: " <<
        node_alloc_pointer / float(r->viewport_width * r->viewport_height) << std::endl;
    std::cout << "Dt: " << scene->dt * 1000 << std::endl;
}

TracePreview* init_trace_preview(Renderer const* r, Camera const* camera)
{
    auto preview = new TracePreview;
    apply_camera_view_matrix(
        preview->bake_view.load_identity(), camera);
    apply_camera_projection_matrix(
        preview->bake_projection.load_identity(), camera);
    preview->bake_nearz = -camera->near;
    return preview;
}

void render_trace_preview(
    Renderer* r, TracePreview const* preview, Camera const* camera)
{
    mat4f viewport_to_bake_view;
    {
        viewport_to_bake_view.load_identity();
        float scalex, scaley;
        mat4f::get_perspective_aov_bounds(
            camera->aspect, camera->near, camera->aov, &scalex, &scaley);
        viewport_to_bake_view.scale(scalex, scaley, camera->near);

        apply_inverse_camera_view_matrix(viewport_to_bake_view, camera);
        viewport_to_bake_view.apply(preview->bake_view);
    }

    glUseProgram(r->programs.trace_preview->id);
    {
        auto program = r->programs.trace_preview;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, r->textures.array_ranges);
        glUniform1i(program->array_ranges, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, r->textures.depth_arrays);
        glUniform1i(program->depth_arrays, 1);

        glUniformMatrix4fv(program->viewport_to_bake_view, 1, GL_TRUE,
            viewport_to_bake_view.p());
        glUniformMatrix4fv(program->bake_projection, 1, GL_TRUE,
            preview->bake_projection.p());
        glUniform1f(program->bake_nearz, preview->bake_nearz);

        glEnableVertexAttribArray(program->viewport_position);
        glBindBuffer(GL_ARRAY_BUFFER, r->buffers.viewport_vertices);
        glVertexAttribPointer(
            program->viewport_position, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glDisableVertexAttribArray(program->viewport_position);
    }
}

void render_frustum(Renderer* r, mat4f const& in_camera, Camera const* camera)
{
    glUseProgram(r->programs.frustum->id);
    {
        auto program = r->programs.frustum;

        glUniformMatrix4fv(program->in_camera, 1, GL_TRUE,
            in_camera.p());
        glUniformMatrix4fv(program->out_camera, 1, GL_TRUE,
            get_camera_matrix(camera).p());

        glEnableVertexAttribArray(program->position);
        glBindBuffer(GL_ARRAY_BUFFER, r->buffers.frustum_vertices);
        glVertexAttribPointer(
            program->position, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        glDrawArrays(GL_LINES, 0, 24);

        glDisableVertexAttribArray(program->position);
    }
}

} // namespace hiab;
