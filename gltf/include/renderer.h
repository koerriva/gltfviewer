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
    vec3 position{0.f,0.f,5.f};
    vec3 target{0.f};
    vec3 up{0.f,1.0f,0.0f};
    float z_near = 0.1f;
    float z_far = 1000.f;
};

typedef struct material {
    vec4 baseColor{1.f};
} material;

typedef struct mesh {
    uint32_t vao;
    int indices_count = 0;
    material material;
} mesh;

typedef struct keyframe_t {
    float time = 0;
    vec3 translation;
    quat rotation;
    vec3 scale;
} keyframe_t;

typedef struct channel_t {
    int keyframe_count;
    keyframe_t keyframe[60];
    int interpolation;//0-liber,1-step,2-cubic
    bool has_translation;
    bool has_rotation;
    bool has_scale;
} channel_t;

typedef struct animation_t {
    int channel_count = 0;
    channel_t channels[5];
} animation_t;

typedef struct joint_t {
    int id;
    mat4 transform;
} joint_t;

typedef struct skeleton_t {
    int joints_count = 0;
    mat4 inverse_bind_matrices[64];
    joint_t joins[64];
} skeleton_t;

typedef struct model_t {
    uint32_t shader;
    mesh meshes[10];
    uint32_t meshes_size=0;
    mat4 transform{1.0};
    int animation_count=0;
    animation_t animations[20];
    int skeleton_count=0;
    skeleton_t skeleton[20];
} model_t;

enum render_mode {
    shader,wireframe
};

class Renderer{
public:
    Renderer();
    ~Renderer() = default;

    void Render(Camera* camera, model_t* model);
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
    static int LoadModel(const char *filename, model_t* model);
    static int LoadAnimateModel(const char *filename, model_t* model);
};

class Animator{
private:
    float currTime[10] = {0};
    keyframe_t * prevFrame[10] = {};
    keyframe_t * nextFrame[10] = {};

    model_t * model;
    bool playing = false;
public:
    explicit Animator(struct model_t * _model):model(_model){};
    void Update(float delta);
    void Play();
    void Stop();
};
#endif //GLTFVIEWER_RENDERER_H
