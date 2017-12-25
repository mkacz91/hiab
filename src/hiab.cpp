#include "prefix.h"
#include <iostream>
#include "opengl.h"
#include "files.h"
#include "scene.h"
#include "render.h"

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
        load_scene_objects(&scene, "bunny");

        Renderer renderer;
        init_renderer(&renderer);

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            set_framebuffer_size(&renderer, window_width, window_height);
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
