#pragma once

#include "prefix.h"
#include "opengl.h"
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
};

struct Scene
{
    std::vector<SceneObject*> objects;
};

int load_scene_objects(Scene* scene, string const& name);

void clear_scene(Scene* scene);

} // namespace hiab
