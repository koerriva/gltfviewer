//
// Created by koerriva on 2021/12/19.
//

#include <iostream>
#include <unordered_map>
#include "renderer.h"
#include "glm/ext.hpp"
#include "glad/glad.h"

mat4 Camera::GetViewMatrix() {
    return lookAt(position, target,up);
}

mat4 Camera::GetProjection() {
    return perspective(radians(60.f),16.f/9.f,z_near,z_far);
}

Renderer::Renderer() {

}

static std::unordered_map<std::string,int> locations;
int GetLocation(uint32_t pid,const char* name){
    auto iter = locations.find(name);
    if(iter==locations.end()){
        int location = glGetUniformLocation(pid,name);
        locations.insert({name,location});
        return location;
    }
    return iter->second;
}

void SetMaterialParam_float(uint32_t pid,const char* name,float value){
    int location = GetLocation(pid,name);
    glUniform1f(location,value);
}
void SetMaterialParam_int(uint32_t pid,const char* name,int value){
    int location = GetLocation(pid,name);
    glUniform1i(location,value);
}
void SetMaterialParam_mat4(uint32_t pid,const char* name,float* value){
    int location = GetLocation(pid,name);
    glUniformMatrix4fv(location,1,GL_FALSE,value);
}
void SetMaterialParam_vec4(uint32_t pid,const char* name,float* value){
    int location = GetLocation(pid,name);
    glUniform4fv(location,1,value);
}

void Renderer::SetRenderMode(render_mode mode) {
    this->m_mode = mode;
}

void Renderer::Render(Camera *camera, model_t* models,size_t size) {
    glEnable(GL_DEPTH_TEST);

    auto p = camera->GetProjection();
    auto v = camera->GetViewMatrix();

    glClearColor(0.2f,0.3f,0.634f,0.6778f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    if(m_mode==wireframe){
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    if(m_mode==shader){
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    for (size_t i = 0; i < size; ++i) {
        model_t* model = models+i;

        glUseProgram(model->shader);
        SetMaterialParam_float(model->shader,"time",game_time);
        SetMaterialParam_mat4(model->shader,"P", value_ptr(p));
        SetMaterialParam_mat4(model->shader,"V", value_ptr(v));

        bool is_Playing_Animation = false;
        if(model->animator){
            ((Animator*) model->animator)->Update(delta);
            is_Playing_Animation = ((Animator*) model->animator)->IsPlaying();

            SetMaterialParam_int(model->shader,"hasSkin",model->has_skin);
            if(model->has_skin){
                skeleton_t* skeleton = &model->skeleton;
                for (int j = 0; j < skeleton->joints_count; ++j) {
                    mat4 mat = skeleton->inverse_bind_matrices[j];
                    std::string name = "u_jointMat["+ std::to_string(j)+"]";
                    mat = calcTransform(mat4{1.0f},skeleton->joins[j].transform);
                    SetMaterialParam_mat4(model->shader,name.c_str(), value_ptr(mat));
                }
            }
        }

        mat4 M{1.f};
        M = calcTransform(M,model->transform);
        SetMaterialParam_mat4(model->shader,"M", value_ptr(M));

        SetMaterialParam_int(model->shader,"baseColorTexture",0);

        for (int j = 0; j < model->mesh_count; ++j) {
            mesh_t * mesh = &model->meshes[j];
            material_t * mat = &mesh->material;

            SetMaterialParam_vec4(model->shader,"base_color", value_ptr(mat->baseColor));

            SetMaterialParam_int(model->shader,"hasBaseColorTexture",mat->hasBaseColorTexture);
            if(mat->hasBaseColorTexture==1){
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D,mat->baseColorTexture);
            }

            glBindVertexArray(mesh->vao);
            glDrawElements(GL_TRIANGLES,mesh->indices_count,GL_UNSIGNED_SHORT,nullptr);
            glBindVertexArray(0);
        }
        glUseProgram(0);
    }

    game_time += delta;
}

mat4 calcTransform(mat4 mat,transform_t transform){
    mat4 M = mat;

    mat4 T = translate(M,transform.position);
    mat4 R = mat4_cast(transform.rotation);
    mat4 S = scale(M,transform.scale);

    M = T*R*S;
    return M;
}
