//
// Created by koerriva on 2021/12/19.
//

#ifndef GLTFVIEWER_RENDERER_H
#define GLTFVIEWER_RENDERER_H

#include <vector>
#include "glm/glm.hpp"
#include "glm/ext.hpp"

using namespace glm;

class Camera{
public:
    Camera() = default;
    ~Camera() = default;

    mat4 GetViewMatrix();
    mat4 GetProjection();

private:
    vec3 position{-5.f,0.f,5.f};
    vec3 target{0.f};
    vec3 up{0.f,1.0f,0.0f};
    float z_near = 0.1f;
    float z_far = 1000.f;
};

typedef struct transform_t {
    vec3 position{0};
    quat rotation{1,0,0,0};
    vec3 scale{1};
} transform_t;

typedef struct material_t {
    vec4 baseColor{1.f};
    int hasBaseColorTexture = 0;
    uint32_t baseColorTexture=0;
} material_t;

typedef struct mesh_t {
    uint32_t vao{};
    int vertices_count = 0;
    int indices_count = 0;
    material_t material{};
} mesh_t;

typedef struct keyframe_t {
    float time = 0;
    vec3 translation{0};
    quat rotation{1,0,0,0};
    vec3 scale{1};
} keyframe_t;

#define MAX_KEYFRAME_COUNT 120

typedef struct channel_t {
    int keyframe_count = 0;
    keyframe_t keyframe[120]{};
    int interpolation = 0;//0-liber,1-step,2-cubic
    bool has_translation = false;
    bool has_rotation = false;
    bool has_scale = false;
    struct object_t* target = nullptr;
    transform_t origin;
} channel_t;

typedef struct animation_t {
    int channel_count = 0;
    channel_t channels[40]{};
    char name[45]{0};
} animation_t;

#define MAX_JOINT_COUNT 64
typedef struct skeleton_t {
    int joints_count = 0;
    object_t* joints[MAX_JOINT_COUNT] = {nullptr};
    mat4 inverse_bind_matrices[MAX_JOINT_COUNT]{};
} skeleton_t;

typedef struct model_t {
    mesh_t meshes[10];
    uint32_t mesh_count=0;
} model_t;

typedef struct object_t {
    int id=-1;
    char name[45] = {0};

    transform_t transform;
    int animated = 0;
    transform_t animated_transform;

    int jointed = 0;
    mat4 jointed_matrices = mat4{1.0f};

    int has_model = 0;
    model_t * model = nullptr;

    int has_skin = 0;
    skeleton_t * skeleton = nullptr;

    int has_camera = 0;
    Camera* camera = nullptr;

    int has_animation = 0;
    int animation_count=0;
    animation_t animations[20];
    class Animator* animator = nullptr;

    int children_count = 0;
    object_t* children[1024]={nullptr};
    object_t* parent = nullptr;
} object_t;

typedef struct scene_t {
    int object_count = 0;
    object_t objects[1024];

    int model_count = 0;
    object_t * models[1024]={nullptr};

    int root_count = 0;
    object_t * roots[1024]={nullptr};

    uint32_t shader = 0;
    Camera* camera = nullptr;
} scene_t;

mat4 getLocalTransform(object_t* object);
mat4 getGlobalTransform(object_t* object);

enum render_mode {
    shader,wireframe
};

class Renderer{
public:
    Renderer();
    ~Renderer() = default;

    void Render(scene_t* scene);
    void SetRenderMode(render_mode mode);
private:
    render_mode m_mode;
    float delta = 1.f/60.f;
    float game_time = 0.f;
};

class Shader {
public:
    static uint32_t LoadBaseShader();
    static uint32_t LoadAnimateShader();
};

class Assets {
public:
    static object_t * LoadAnimateModel(const char *filename, scene_t* scene);
    static uint32_t LoadTexture(ivec3 shape,ivec2 filter,ivec2 warp,const unsigned char *buffer);
};

class Animator{
#define MAX_ANIMATION_COUNT 10
#define MAX_CHANNEL_COUNT 5
private:
    float currTime[MAX_ANIMATION_COUNT] = {0};
    keyframe_t * prevFrame[MAX_ANIMATION_COUNT][MAX_CHANNEL_COUNT] = {};
    keyframe_t * nextFrame[MAX_ANIMATION_COUNT][MAX_CHANNEL_COUNT] = {};

    object_t * model;
    bool playing = false;
    const char* playingAnimation = "";
public:
    explicit Animator(struct object_t * _model);
    void Update(float delta);
    void Play();
    void Play(const char* name);
    void Pause();
    void Stop();

    bool IsPlaying() const;
};
#endif //GLTFVIEWER_RENDERER_H
