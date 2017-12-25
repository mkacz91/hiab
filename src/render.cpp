#include "render.h"
#include "scene.h"
#include "shaders.h"
#include "math.h"

#include <iostream>

namespace hiab {

void init_renderer(Renderer* r)
{
    r->framebuffer_width = r->framebuffer_height = 0;
    r->framebuffer_size_changed = true;

    r->programs.object = new ObjectProgram;
    r->programs.heads = new HeadsProgram;

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

    glGenTextures(
        Renderer::TEXTURE_COUNT, reinterpret_cast<GLuint*>(&r->textures));

    glBindTexture(GL_TEXTURE_2D, r->textures.heads);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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

void set_framebuffer_size(Renderer* r, int width, int height)
{
    r->framebuffer_size_changed =
        r->framebuffer_size_changed ||
        r->framebuffer_width != width || r->framebuffer_height != height;
    r->framebuffer_width = width;
    r->framebuffer_height = height;
}

void apply_framebuffer_size_changes(Renderer* r)
{
    if (!r->framebuffer_size_changed)
        return;
    r->framebuffer_size_changed = false;
    int width = r->framebuffer_width, height = r->framebuffer_height;

    glBindTexture(GL_TEXTURE_2D, r->textures.heads);
    glTexImage2D(GL_TEXTURE_2D, 0,
        GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, r->framebuffers.clear_heads);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
        r->textures.heads, 0);
}

void render(Renderer* r, Scene const* scene)
{
    float t = (float)glfwGetTime();

    apply_framebuffer_size_changes(r);
    glViewport(0, 0, r->framebuffer_width, r->framebuffer_height);

    glBindFramebuffer(GL_FRAMEBUFFER, r->framebuffers.clear_heads);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.05f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    mat4f projection = mat4f::perspective_aov(
        r->framebuffer_width, r->framebuffer_height,
        0.25, 50, 0.5f * PI);
    mat4f view = eye4f().translate(0, 0, -3);
    mat4f camera = projection * view;
    mat4f transform = eye4f()
        .scale(0.02f)
        .rotate_x(2 * t)
        .rotate_z(0.5f * t);

    GLuint counter = 0;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, r->buffers.counter);
    glBufferData(
        GL_ATOMIC_COUNTER_BUFFER, sizeof(counter), &counter,
        GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, r->buffers.counter);
    glBindImageTexture(
        0, r->textures.heads, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

    glUseProgram(r->programs.object->id);
    {
        auto program = r->programs.object;
        glUniformMatrix4fv(program->camera, 1, GL_TRUE, camera.p());
        glUniformMatrix4fv(program->transform, 1, GL_TRUE, transform.p());
        glEnableVertexAttribArray(program->position);
        glEnableVertexAttribArray(program->normal);
        for (SceneObject const* object : scene->objects)
        {
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

    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->textures.heads);

    glUseProgram(r->programs.heads->id);
    {
        auto program = r->programs.heads;
        glUniform1i(program->heads, 0);
        glEnableVertexAttribArray(program->position);
        glBindBuffer(GL_ARRAY_BUFFER, r->buffers.viewport_vertices);
        glVertexAttribPointer(
            program->position, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDisableVertexAttribArray(program->position);
    }

    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, r->buffers.counter);
    glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(counter), (void*)&counter);
    std::cout << "Rendered fragments: " << counter << std::endl;
}

} // namespace hiab;
