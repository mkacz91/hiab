#include "scene.h"
#include <vector>
#include <tinyobj.h>
#include "opengl.h"
#include "math.h"
#include "files.h"

namespace to = tinyobj;

namespace hiab {

struct create_scene_object_closure
{
    string const& filename;
    to::attrib_t const& attrib;
    std::vector<to::material_t> const& materials;
    std::vector<vec3f> positions;
    std::vector<vec3f> normals;
    std::vector<vec2f> uvs;
};

SceneObject* create_scene_object(create_scene_object_closure& c, to::shape_t& shape)
{
    c.positions.clear();
    for (auto& index : shape.mesh.indices)
        c.positions.push_back(view_as<vec3f>(&c.attrib.vertices[3 * index.vertex_index]));
    int face_count = (int)c.positions.size() / 3;
    int vertex_count = 3 * face_count;
    if (face_count == 0)
        return nullptr;

    c.normals.clear();
    if (shape.mesh.indices[0].normal_index != -1)
    {
        for (auto& index : shape.mesh.indices)
            c.normals.push_back(view_as<vec3f>(&c.attrib.normals[3 * index.normal_index]));
    }
    else
    {
        // TODO: Compute normals from faces
        throw not_implemented_here;
    }

    c.uvs.clear();
    bool uvs_present = shape.mesh.indices[0].texcoord_index != -1;
    if (uvs_present)
    {
        for (auto& index : shape.mesh.indices)
            c.uvs.push_back(view_as<vec2f>(&c.attrib.texcoords[2 * index.texcoord_index]));
    }

    glGetError();

    SceneObjectBuffers buffers;
    glGenBuffers(uvs_present ? 3 : 2, reinterpret_cast<GLuint*>(&buffers));
    glBindBuffer(GL_ARRAY_BUFFER, buffers.positions);
    glBufferData(
        GL_ARRAY_BUFFER, vertex_count * sizeof(vec3f),
        reinterpret_cast<void const*>(c.positions.data()), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.normals);
    glBufferData(
        GL_ARRAY_BUFFER, vertex_count * sizeof(vec3f),
        reinterpret_cast<void const*>(c.normals.data()), GL_STATIC_DRAW);
    if (uvs_present)
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffers.uvs);
        glBufferData(
            GL_ARRAY_BUFFER, vertex_count * sizeof(vec2f),
            reinterpret_cast<void const*>(c.uvs.data()), GL_STATIC_DRAW);
    }

    GLenum gl_error = glGetError();
    if (gl_error != GL_NO_ERROR)
        throw gl_exception("Unable to create scene object buffers.", gl_error);

    auto object = new SceneObject;
    object->name = shape.name.empty() ? c.filename : c.filename + "/" + shape.name;
    object->vertex_count = vertex_count;
    object->buffers = buffers;

    return object;
}

int load_scene_objects(Scene* scene, string const& name)
{
    auto path = get_file_path(join_path(name, name) + ".obj");
    if (path.empty())
        return 0;

    to::attrib_t attrib;
    std::vector<to::shape_t> shapes;
    std::vector<to::material_t> materials;
    string error_message;
    bool load_succeeded = tinyobj::LoadObj(
        &attrib, &shapes, &materials, &error_message, path.c_str());
    if (!load_succeeded)
        throw file_error(path, error_message);

    int prev_object_count = (int)scene->objects.size();
    create_scene_object_closure c = { name, attrib, materials };
    for (auto& shape : shapes)
    {
        auto object = create_scene_object(c, shape);
        if (object)
            scene->objects.push_back(object);
    }

    return (int)scene->objects.size() - prev_object_count;
}

void clear_scene(Scene* scene)
{
    for (auto object : scene->objects)
    {
        glDeleteBuffers(
            SceneObjectBuffers::COUNT, reinterpret_cast<GLuint const*>(&object->buffers));
        delete object;
    }
    scene->objects.clear();
}

} // namespace hiab
