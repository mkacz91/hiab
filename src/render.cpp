#include "render.h"
#include "scene.h"
#include "shaders.h"
#include "math.h"

#include <iostream>

namespace hiab {

void init_renderer(Renderer* r)
{
    r->framebuffer_width = r->framebuffer_height = 0;
    r->object_program = new ObjectProgram;
    glGenBuffers(RendererBuffers::COUNT, reinterpret_cast<GLuint*>(&r->buffers));
}

void close_renderer(Renderer* r)
{
    delete r->object_program;
    glDeleteBuffers(RendererBuffers::COUNT, reinterpret_cast<GLuint*>(&r->buffers));
}

void set_framebuffer_size(Renderer* r, int width, int height)
{
    r->framebuffer_width = width;
    r->framebuffer_height = height;
}

void render(Renderer* r, Scene const* scene)
{
    float t = (float)glfwGetTime();

    glViewport(0, 0, r->framebuffer_width, r->framebuffer_height);
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

    glUseProgram(r->object_program->id);
    {
        auto program = r->object_program;
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

    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, r->buffers.counter);
    glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(counter), (void*)&counter);
    std::cout << "Rendered fragments: " << counter << std::endl;
}

} // namespace hiab;
