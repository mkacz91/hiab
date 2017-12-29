#include "prefix.h"
#include <iostream>
#include "math.h"
#include "opengl.h"
#include "files.h"
#include "scene.h"
#include "render.h"

using namespace hiab;

GLFWwindow* window;

Renderer renderer;
Scene scene;
Camera camera;

int framebuffer_width, framebuffer_height;
double mouse_x, mouse_y;
bool flying_around = false;
bool camera_panning = false;

void set_flying_around(bool value);
void apply_camera_movement();

void on_key(GLFWwindow* window, int key, int scancode, int action, int mods);
void on_mouse_button(GLFWwindow* window, int button, int action, int mods);
void on_mouse_move(GLFWwindow* window, double x, double y);
void on_resize(GLFWwindow* window, int width, int height);

int main(int argc, char** argv)
{
    add_file_search_prefix("../src/shaders");
    add_file_search_prefix("../obj");

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
            on_resize(window, width, height);
            glfwSetFramebufferSizeCallback(window, on_resize);
        }
        load_scene_objects(&scene, "teapot");

        glfwSetKeyCallback(window, on_key);
        glfwSetMouseButtonCallback(window, on_mouse_button);
        glfwSetCursorPosCallback(window, on_mouse_move);

        set_camera_clip_planes(&camera, 0.5f, 50.0f);
        move_camera(&camera, { 0, 0, 3 });

        init_scene_time(&scene, glfwGetTime());

        while (!glfwWindowShouldClose(window))
        {
            glViewport(0, 0, framebuffer_width, framebuffer_height);
            render(&renderer, &scene, &camera);
            glfwSwapBuffers(window);

            glfwPollEvents();
            advance_scene_time(&scene, glfwGetTime());
            apply_camera_movement();
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

void set_flying_around(bool value)
{
    if (flying_around == value)
        return;

    flying_around = value;
    if (flying_around)
    {
        glfwGetCursorPos(window, &mouse_x, &mouse_y);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void apply_camera_movement()
{
    if (!flying_around)
        return;

    vec3f translation = { 0, 0, 0 };
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        if (camera_panning)
            translation.y += 1;
        else
            translation.z -= 1;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        if (camera_panning)
            translation.y -= 1;
        else
            translation.z += 1;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        translation.x -= 1;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        translation.x += 1;

    translation *= 6.0f * scene.dt;
    move_camera(&camera, translation);
}

void on_key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch (key)
    {
        case GLFW_KEY_SPACE:
            if (action == GLFW_PRESS)
                camera_panning = !camera_panning;
        break;
        case GLFW_KEY_ESCAPE:
            if (action == GLFW_PRESS)
            {
                if (flying_around)
                    set_flying_around(false);
                else
                    glfwSetWindowShouldClose(window, true);
            }
        break;
    }
}

void on_mouse_button(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        set_flying_around(!flying_around);
}

void on_mouse_move(GLFWwindow* window, double x, double y)
{
    if (!flying_around)
        return;
    float dx = float(x - mouse_x), dy = float(y - mouse_y);
    rotate_camera(&camera,
        { -dy / max(1, framebuffer_height), -dx / max(1, framebuffer_width) });
    mouse_x = x; mouse_y = y;
}

void on_resize(GLFWwindow* window, int width, int height)
{
    std::cout << "Framebuffer size: " << width << " " << height << std::endl;
    framebuffer_width = width;
    framebuffer_height = height;
    set_renderer_viewport(&renderer, width, height);
    set_camera_viewport(&camera, width, height);
}
