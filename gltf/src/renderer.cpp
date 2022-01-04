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
        }

        mat4 M{1.f};
        M = calcTransform(M,model->transform);
        SetMaterialParam_mat4(model->shader,"M", value_ptr(M));

        SetMaterialParam_int(model->shader,"baseColorTexture",0);

        for (int j = 0; j < model->mesh_count; ++j) {
            mesh * mesh = &model->meshes[j];
            material * mat = &mesh->material;

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

Animator::Animator(struct model_t *_model):model(_model) {
    origin_transform = _model->transform;
    for (int i = 0; i < MAX_CHANNEL_COUNT; ++i) {
        currTime[i] = 0;
        prevFrame[i] = nullptr;
        nextFrame[i] = nullptr;
    }
}

void Animator::Play() {
    playing = true;
    std::cout << "Play Animation ..." << std::endl;
}

void Animator::Pause() {
    playing = false;
    std::cout << "Pause Animation ..." << std::endl;
}

void Animator::Stop() {
    playing = false;
    std::cout << "Stop Animation ..." << std::endl;
    model->transform = origin_transform;
    for (int i = 0; i < MAX_CHANNEL_COUNT; ++i) {
        currTime[i] = 0;
        prevFrame[i] = nullptr;
        nextFrame[i] = nullptr;
    }
}

void Animator::Update(float delta) {
    if(!playing)return;
    if(model->animation_count>0){
        animation_t * animation = model->animations;

        curr_transform = origin_transform;

        for (int i = 0; i < animation->channel_count; ++i) {
            keyframe_t* base_keyframe = animation->channels[i].keyframe;
            int keyframe_count = animation->channels[i].keyframe_count;

            for (int j = 0; j < keyframe_count; ++j) {
                keyframe_t* keyframe = base_keyframe+j;
                if(keyframe->time<=currTime[i]){
                    prevFrame[i] = keyframe;
                }
            }

            bool lastKeyframe = true;
            for (int k = 0; k < keyframe_count; ++k) {
                keyframe_t* keyframe = base_keyframe+k;
                if(keyframe->time>currTime[i]){
                    nextFrame[i] = keyframe;
                    lastKeyframe = false;
                    break;
                }
            }

            if(lastKeyframe){
                currTime[i]=0.0;
                nextFrame[i] = base_keyframe;
            }

            float interp = (currTime[i] - prevFrame[i]->time)/(nextFrame[i]->time-prevFrame[i]->time);

            if(animation->channels[i].has_translation){
                vec3 translation = mix(prevFrame[i]->translation,nextFrame[i]->translation,interp);
                curr_transform.position = origin_transform.position + translation;
            }

            if(animation->channels[i].has_rotation){
                quat origin = origin_transform.rotation;

                quat prev = prevFrame[i]->rotation;
                quat next = nextFrame[i]->rotation;

                quat rotation = slerp(prev, next,interp);
                curr_transform.rotation = origin * rotation;
            }

            if(animation->channels[i].has_scale){
                vec3 scale = mix(prevFrame[i]->scale,nextFrame[i]->scale,interp);
                curr_transform.scale = origin_transform.scale * scale;
            }

            currTime[i] += 0.001f;
        }

        model->transform = curr_transform;
    }
}

bool Animator::IsPlaying() const {
    return playing;
}

mat4 calcTransform(transform_t transform){
    return calcTransform(mat4{1.0},transform);
}

mat4 calcTransform(mat4 mat,transform_t transform){
    mat4 M = mat;

    mat4 T = translate(M,transform.position);
    mat4 R = mat4_cast(transform.rotation);
    mat4 S = scale(M,transform.scale);

    M = T*R*S;
    return M;
}
