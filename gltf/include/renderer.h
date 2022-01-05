//
// Created by koerriva on 2021/12/19.
//

#ifndef GLTFVIEWER_RENDERER_H
#define GLTFVIEWER_RENDERER_H

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
    vec3 position{-5.f,2.f,3.f};
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
    int indices_count = 0;
    material_t material{};
} mesh_t;

typedef struct keyframe_t {
    float time = 0;
    vec3 translation{0};
    quat rotation{1,0,0,0};
    vec3 scale{1};
} keyframe_t;

typedef struct channel_t {
    int keyframe_count = 0;
    keyframe_t keyframe[60]{};
    int interpolation = 0;//0-liber,1-step,2-cubic
    bool has_translation = false;
    bool has_rotation = false;
    bool has_scale = false;
    transform_t* transform = nullptr;
} channel_t;

typedef struct animation_t {
    int channel_count = 0;
    channel_t channels[40]{};
    char name[20]{0};
} animation_t;

typedef struct joint_t {
    int id{};
    transform_t transform{};
} joint_t;

#define MAX_JOINT_COUNT 64
typedef struct skeleton_t {
    int joints_count = 0;
    joint_t joins[MAX_JOINT_COUNT];
    mat4 inverse_bind_matrices[MAX_JOINT_COUNT]{};
} skeleton_t;

typedef struct model_t {
    uint32_t shader{};
    mesh_t meshes[10];
    uint32_t mesh_count=0;
    transform_t transform{};
    int animation_count=0;
    animation_t animations[20];
    int has_skin = 0;
    skeleton_t skeleton;
    void * animator{};
} model_t;

mat4 calcTransform(mat4 mat,transform_t transform);

enum render_mode {
    shader,wireframe
};

class Renderer{
public:
    Renderer();
    ~Renderer() = default;

    void Render(Camera* camera, model_t* models, size_t size);
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
    static int LoadAnimateModel(const char *filename, model_t* model);
    static uint32_t LoadTexture(ivec3 shape,ivec2 filter,ivec2 warp,const unsigned char *buffer);
};

class Animator{
#define MAX_ANIMATION_COUNT 10
#define MAX_CHANNEL_COUNT 5
private:
    float currTime[MAX_ANIMATION_COUNT][MAX_CHANNEL_COUNT] = {0};
    keyframe_t * prevFrame[MAX_ANIMATION_COUNT][MAX_CHANNEL_COUNT] = {};
    keyframe_t * nextFrame[MAX_ANIMATION_COUNT][MAX_CHANNEL_COUNT] = {};

    model_t * model;
    transform_t origin_transform[MAX_ANIMATION_COUNT][MAX_CHANNEL_COUNT] = {};
    bool playing = false;
    const char* playingAnimation = nullptr;
public:
    explicit Animator(struct model_t * _model);
    void Update(float delta);
    void Play();
    void Play(const char* name);
    void Pause();
    void Stop();

    bool IsPlaying() const;
};
#endif //GLTFVIEWER_RENDERER_H
