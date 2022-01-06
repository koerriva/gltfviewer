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

void Renderer::Render(scene_t* scene) {
    glEnable(GL_DEPTH_TEST);

    Camera* camera = scene->camera;

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

    glUseProgram(scene->shader);

    //animation
    for (int i = 0; i < scene->root_count; ++i) {
        object_t* object = scene->roots[i];
        if(object->has_animation==1){
            object->animator->Update(delta);
        }
    }

    for (size_t i = 0; i < scene->model_count; ++i) {
        object_t* object = scene->models[i];
        model_t* model = object->model;

        SetMaterialParam_float(scene->shader,"time",game_time);
        SetMaterialParam_mat4(scene->shader,"P", value_ptr(p));
        SetMaterialParam_mat4(scene->shader,"V", value_ptr(v));

        SetMaterialParam_int(scene->shader,"hasSkin",object->has_skin);
        if(object->has_skin){
            skeleton_t* skeleton = object->skeleton;
            for (int j = 0; j < skeleton->joints_count; ++j) {
                mat4 inverse_mat = skeleton->inverse_bind_matrices[j];
                mat4 joint_mat = getGlobalTransform(skeleton->joins[j]);
                joint_mat = joint_mat*inverse_mat;

                std::string name = "u_jointMat["+ std::to_string(j)+"]";
                SetMaterialParam_mat4(scene->shader,name.c_str(), value_ptr(joint_mat));
            }
        }

        mat4 M = getGlobalTransform(object);
        SetMaterialParam_mat4(scene->shader,"M", value_ptr(M));

        SetMaterialParam_int(scene->shader,"baseColorTexture",0);

        for (int j = 0; j < model->mesh_count; ++j) {
            mesh_t * mesh = &model->meshes[j];
            material_t * mat = &mesh->material;

            SetMaterialParam_vec4(scene->shader,"base_color", value_ptr(mat->baseColor));

            SetMaterialParam_int(scene->shader,"hasBaseColorTexture",mat->hasBaseColorTexture);
            if(mat->hasBaseColorTexture==1){
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D,mat->baseColorTexture);
            }

            glBindVertexArray(mesh->vao);
            if(mesh->indices_count>0){
                glDrawElements(GL_TRIANGLES,mesh->indices_count,GL_UNSIGNED_SHORT,nullptr);
            }else{
                glDrawArrays(GL_TRIANGLES,0,mesh->vertices_count);
            }
            glBindVertexArray(0);
        }
    }

    glUseProgram(0);

    game_time += delta;
}

mat4 calcTransform(transform_t* transform){
    mat4 M{1.0f};
    mat4 T = translate(M,transform->position);
    mat4 R = mat4_cast(transform->rotation);
    mat4 S = scale(M,transform->scale);
    M = T*R*S;
    return M;
}

mat4 getLocalTransform(object_t* object){
    return calcTransform(&object->transform);
}

mat4 getGlobalTransform(object_t* object){

    transform_t * transform = &(object->animated?object->animated_transform:object->transform);
    mat4 local = calcTransform(transform);
    mat4 global = mat4{1.0f};

    std::vector<transform_t*> parent_transforms;
    object_t* parent_object = object->parent;
    int depth = 0;

    while (parent_object){
        parent_transforms.push_back(&(parent_object->animated?parent_object->animated_transform:parent_object->transform));

        parent_object = parent_object->parent;
        depth++;
    }

    for (int i = depth; i >0; i--) {
        global *= calcTransform(parent_transforms[i-1]);
    }

    return global * local;
}
