#include "prefix.h"
#include <iostream>
#include "opengl.h"
#include "files.h"
#include "scene.h"
#include "render.h"

using namespace hiab;

Scene scene;
Renderer renderer;

void framebuffer_size_changed(GLFWwindow* window, int width, int height);

int main(int argc, char** argv)
{
    add_file_search_prefix("../src/shaders");
    add_file_search_prefix("../obj");

    GLFWwindow* window;

    if (!glfwInit())
        return 1;

    window = glfwCreateWindow(800, 600, "Hiab", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    try
    {
        init_renderer(&renderer);
        {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            framebuffer_size_changed(window, width, height);
            glfwSetFramebufferSizeCallback(window, framebuffer_size_changed);
        }
        load_scene_objects(&scene, "teapot");

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            render(&renderer, &scene);
            glfwSwapBuffers(window);
        }

        clear_scene(&scene);
        close_renderer(&renderer);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_changed(GLFWwindow* window, int width, int height)
{
    std::cout << "Framebuffer size: " << width << " " << height << std::endl;
    set_framebuffer_size(&renderer, width, height);
}
