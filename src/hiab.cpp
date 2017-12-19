#include "opengl.h"

int main(int argc, char** argv)
{
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

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.7f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
