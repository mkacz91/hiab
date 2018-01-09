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
TracePreview* trace_preview = nullptr;
mat4f captured_camera = eye4f();
int trace_iterations = 100;

void set_flying_around(bool value);
void apply_camera_movement();
bool trace_preview_enabled();
void set_trace_preview(bool enabled);

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
        {
            box3f bounds;
            bounds.clear();
            for (auto const* object : scene.objects)
                bounds.expand(object->bounds);
            mat4f transform = get_box_mapping_to_symunit(bounds);
            for (auto* object : scene.objects)
                object->transform = transform;
        }

        glfwSetKeyCallback(window, on_key);
        glfwSetMouseButtonCallback(window, on_mouse_button);
        glfwSetCursorPosCallback(window, on_mouse_move);

        set_camera_clip_planes(&camera, 0.5f, 5.0f);
        move_camera(&camera, { 0, 0, 3 });

        init_scene_time(&scene, glfwGetTime());

        while (!glfwWindowShouldClose(window))
        {
            if (trace_preview_enabled())
            {
                render_trace_preview(&renderer, trace_preview, &camera);
            }
            else
            {
                render_frustum(&renderer, captured_camera, &camera);
                render_scene(&renderer, &scene, &camera);
            }
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

    translation *= 3.0f * scene.dt;
    move_camera(&camera, translation);
}

bool trace_preview_enabled()
{
    return trace_preview != nullptr;
}

void set_trace_preview(bool enabled)
{
    if (trace_preview_enabled() == enabled)
        return;

    if (enabled)
    {
        trace_preview = init_trace_preview(&renderer, &camera);
        captured_camera = get_camera_matrix(&camera);
    }
    else
    {
        delete trace_preview;
        trace_preview = nullptr;
    }
}

void set_trace_iterations(int value)
{
    value = clamp(value, 0, 200);
    if (trace_iterations == value)
        return;

    trace_iterations = value;
    if (trace_preview_enabled())
        trace_preview->iterations = trace_iterations;

    std::cout << "Trace iterations: " << trace_iterations << std::endl;
}

void on_key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch (key)
    {
        case GLFW_KEY_SPACE:
            if (action == GLFW_PRESS)
                camera_panning = !camera_panning;
            break;

        case GLFW_KEY_R:
            if (action == GLFW_PRESS)
                set_trace_preview(!trace_preview_enabled());
            break;

        case GLFW_KEY_I:
        case GLFW_KEY_O:
            if (action == GLFW_PRESS)
            {
                int delta = key == GLFW_KEY_I ? 1 : -1;
                set_trace_iterations(trace_iterations + delta);
            }
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
    set_renderer_viewport(&renderer, { 0, 0, width, height });
    set_camera_viewport(&camera, width, height);
}
