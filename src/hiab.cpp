#include "prefix.h"
#include <iostream>
#include "opengl.h"
#include "files.h"

using namespace hiab;

int main(int argc, char** argv)
{
    add_file_search_prefix("../src/shaders");

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
        float positions[] =
        {
            -1, -1,
            1, -1,
            0, 1
        };
        GLuint vertices;
        glGenBuffers(1, &vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vertices);
        glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

        GLuint program = gl_link_program("position4_v", "uniform4_f");
        GLint color_uniform = gl_get_uniform_location(program, "color");
        GLint position_attrib = gl_get_attrib_location(program, "position");

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            glViewport(0, 0, window_width, window_height);
            glClearColor(0.7f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(program);
            glUniform4f(color_uniform, 0.2f, 0.2f, 0.7f, 1.0f);
            glEnableVertexAttribArray(position_attrib);
            glBindBuffer(GL_ARRAY_BUFFER, vertices);
            glVertexAttribPointer(position_attrib, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glDisableVertexAttribArray(position_attrib);

            glfwSwapBuffers(window);
        }

        glDeleteProgram(program);
        glDeleteBuffers(1, &vertices);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    glfwTerminate();
    return 0;
}
