#pragma once

#include "prefix.h"
#include "opengl.h"
#include "math.h"
#include <vector>

namespace hiab {

struct SceneObjectBuffers
{
    static constexpr int COUNT = 3;

    GLuint positions = 0;
    GLuint normals = 0;
    GLuint uvs = 0;
};

struct SceneObject
{
    string name;
    int vertex_count;
    SceneObjectBuffers buffers;
    box3f bounds;
    mat4f transform;
};

struct Scene
{
    std::vector<SceneObject*> objects;

    double double_time;
    float time;
    float dt;
};

struct Camera
{
    vec3f position = { 0, 0, 0 };
    vec2f angle = { 0, 0 };
    float scale = 1;
    float aspect = 1;
    float near = 0.5f;
    float far = 10.0f;
    float aov = QUARTER_PI;
};

int load_scene_objects(Scene* scene, string const& name);

void init_scene_time(Scene* scene, double t);

void advance_scene_time(Scene* scene, double t);

void clear_scene(Scene* scene);

void move_camera(Camera* camera, vec3f translation);

void rotate_camera(Camera* camera, vec2f rotation);

void scale_camera(Camera* camera, float factor);

void set_camera_viewport(Camera* camera, int width, int height);

void set_camera_clip_planes(Camera* camera, float near, float far);

void apply_camera_view_matrix(mat4f& matrix, Camera const* camera);

void apply_camera_projection_matrix(mat4f& matrix, Camera const* camera);

mat4f get_camera_matrix(Camera const* camera);

} // namespace hiab
