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
    c.positions.reserve(shape.mesh.indices.size());
    for (auto& index : shape.mesh.indices)
        c.positions.push_back(view_as<vec3f>(&c.attrib.vertices[3 * index.vertex_index]));
    int face_count = (int)c.positions.size() / 3;
    int vertex_count = 3 * face_count;
    if (face_count == 0)
        return nullptr;

    c.normals.clear();
    c.normals.reserve(shape.mesh.indices.size());
    if (shape.mesh.indices[0].normal_index != -1)
    {
        for (auto& index : shape.mesh.indices)
            c.normals.push_back(view_as<vec3f>(&c.attrib.normals[3 * index.normal_index]));
    }
    else
    {
        auto position = c.positions.begin();
        while (position != c.positions.end())
        {
            vec3f normal = face_normal(*position++, *position++, *position++);
            c.normals.push_back(normal);
            c.normals.push_back(normal);
            c.normals.push_back(normal);
        }
    }

    c.uvs.clear();
    bool uvs_present = shape.mesh.indices[0].texcoord_index != -1;
    if (uvs_present)
    {
        c.uvs.reserve(shape.mesh.indices.size());
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
    object->bounds = get_bounds(c.positions);
    object->transform.load_identity();

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

void init_scene_time(Scene* scene, double t)
{
    scene->double_time = t;
    scene->time = (float)t;
    scene->dt = 1.0f / 30.0f;
}

void advance_scene_time(Scene* scene, double t)
{
    double prev_time = scene->double_time;
    scene->double_time = t;
    scene->time = (float)t;
    scene->dt = max((float)(t - prev_time), 0.0f);
}

void advance_scene_time(Scene* scene);

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

void move_camera(Camera* camera, vec3f translation)
{
    if (translation == vec3f { 0, 0, 0 })
        return;
    mat4f rotation = eye4f()
        .rotate_x(camera->angle.x)
        .rotate_y(camera->angle.y);
    camera->position += rotation.transform_v(translation) / camera->scale;;
}

void rotate_camera(Camera* camera, vec2f rotation)
{
    camera->angle += rotation;
    camera->angle.y = fmod(camera->angle.y , TWO_PI);
    camera->angle.x = clamp(camera->angle.x, -QUARTER_PI, QUARTER_PI);
}

void set_camera_viewport(Camera* camera, int width, int height)
{
    camera->aspect = float(max(width, 1)) / float(max(height, 1));
}

void set_camera_clip_planes(Camera* camera, float near, float far)
{
    constexpr float MARGIN = 0.1f;
    camera->near = max(near, MARGIN);
    camera->far = max(far, camera->near + MARGIN);
}

void apply_camera_view_matrix(mat4f& matrix, Camera const* camera)
{
    matrix
        .translate(-camera->position)
        .rotate_y(-camera->angle.y)
        .rotate_x(-camera->angle.x)
        .scale(camera->scale);
}

void apply_inverse_camera_view_matrix(mat4f& matrix, Camera const* camera)
{
    matrix
        .scale(1.0f / camera->scale)
        .rotate_x(camera->angle.x)
        .rotate_y(camera->angle.y)
        .translate(camera->position);
}

void apply_camera_projection_matrix(mat4f& matrix, Camera const* camera)
{
    matrix.apply(mat4f::perspective_aov(
        camera->aspect, camera->near, camera->far, camera->aov));
}

mat4f get_camera_matrix(Camera const* camera)
{
    mat4f matrix;
    matrix.load_identity();
    apply_camera_view_matrix(matrix, camera);
    apply_camera_projection_matrix(matrix, camera);
    return matrix;
}

} // namespace hiab
