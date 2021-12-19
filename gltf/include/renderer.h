//
// Created by koerriva on 2021/12/19.
//

#ifndef GLTFVIEWER_RENDERER_H
#define GLTFVIEWER_RENDERER_H

#include "glm/glm.hpp"

using namespace glm;

class Camera{
public:
    Camera() = default;
    ~Camera() = default;

    mat4 GetViewMatrix();
    mat4 GetProjection();

private:
    vec3 position{0.f,0.f,-5.f};
    vec3 target{0.f};
    vec3 up{0.f,1.0f,0.0f};
    float z_near = 0.1f;
    float z_far = 1000.f;
};

typedef struct {
    uint32_t pid;
    vec4 baseColor{1.f};
    float metallic = 0;
    float roughness = 0;
} material;

typedef struct {
    uint32_t vao;
    int vertices_count = 0;
    int normal_count = 0;
    int texcoord0_count = 0;
    int indices_count = 0;
    material material{};
} mesh;

typedef struct {
    mesh meshes[1024]{};
    uint32_t meshes_size = 0;
} model;

enum render_mode {
    shader,wireframe
};

class Renderer{
public:
    Renderer();
    ~Renderer() = default;

    void Render(Camera* camera,model* model);
    void SetRenderMode(render_mode mode);

    static int LoadModel(const char *filename,model* model);
    static uint32_t LoadBaseShader();

private:
    render_mode m_mode;
    float delta = 1.f/60.f;
    float game_time = 0.f;
};

#endif //GLTFVIEWER_RENDERER_H
