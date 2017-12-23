#include "prefix.h"
#include <iostream>
#include "opengl.h"
#include "files.h"
#include "math.h"
#include "scene.h"

using namespace hiab;

int main(int argc, char** argv)
{
    add_file_search_prefix("../src/shaders");
    add_file_search_prefix("../obj");

    GLFWwindow* window;

    if (!glfwInit())
        return 1;

    int window_width = 800, window_height = 600;
    window = glfwCreateWindow(window_width, window_height, "Hiab", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    try
    {
        Scene scene;
        int obj_count = load_scene_objects(&scene, "bunny");

        GLuint object_program = gl_link_program(
            "scene_object_v", "scene_object_f");
        GLint object_camera_uniform = gl_get_uniform_location(
            object_program, "camera");
        GLint object_transform_uniform = gl_get_uniform_location(
            object_program, "transform");
        GLint object_position_attrib = gl_get_attrib_location(
            object_program, "position");
        GLint object_normal_attrib = gl_get_attrib_location(
            object_program, "normal");
        GLint object_uv_attrib = gl_get_attrib_location(
            object_program, "uv");

        GLuint counter_buffer;
        GLuint counter = 0;
        glGenBuffers(1, &counter_buffer);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, counter_buffer);
        glBufferData(
            GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &counter, GL_DYNAMIC_DRAW);

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            glViewport(0, 0, window_width, window_height);
            glClearColor(0.05f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);

            mat4f projection = mat4f::perspective_aov(
                window_width, window_height, 0.25, 50, 0.5f * PI);
            mat4f view = eye4f().translate(0, 0, -3);
            mat4f camera = projection * view;

            mat4f transform = eye4f()
                .rotate_x(2 * (float)glfwGetTime())
                .rotate_z(0.5f * (float)glfwGetTime());

            counter = 0;
            glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, counter_buffer);
            glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &counter);
            glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, counter_buffer);

            glUseProgram(object_program);
            glUniformMatrix4fv(object_camera_uniform, 1, GL_TRUE, camera.p());
            glUniformMatrix4fv(object_transform_uniform, 1, GL_TRUE, transform.p());
            glEnableVertexAttribArray(object_position_attrib);
            glEnableVertexAttribArray(object_normal_attrib);
            for (SceneObject const* object : scene.objects)
            {
                glBindBuffer(GL_ARRAY_BUFFER, object->buffers.positions);
                glVertexAttribPointer(
                    object_position_attrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
                glBindBuffer(GL_ARRAY_BUFFER, object->buffers.normals);
                glVertexAttribPointer(
                    object_normal_attrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

                glDrawArrays(GL_TRIANGLES, 0, object->vertex_count);
            }
            glDisableVertexAttribArray(object_position_attrib);
            glDisableVertexAttribArray(object_normal_attrib);

            glfwSwapBuffers(window);

            glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(counter), (void*)&counter);
            std::cout << "Rendered fragments: " << counter << std::endl;
        }

        clear_scene(&scene);
        glDeleteProgram(object_program);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    glfwTerminate();
    return 0;
}
