//
// Created by koerriva on 2021/12/19.
//

#include <iostream>
#include <unordered_map>
#include "renderer.h"
#include "glm/ext.hpp"
#include "glad/glad.h"
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

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


float currTime = 0;
keyframe_t * prevFrame = nullptr;
keyframe_t * nextFrame = nullptr;


void Renderer::Render(Camera *camera, model* model) {
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

    {
        glUseProgram(model->shader);
        SetMaterialParam_float(model->shader,"time",game_time);
        SetMaterialParam_mat4(model->shader,"P", value_ptr(p));
        SetMaterialParam_mat4(model->shader,"V", value_ptr(v));
        SetMaterialParam_mat4(model->shader,"M", value_ptr(model->transform));

        if(model->animation_count>0){
            if(currTime>1.0){
                currTime = 0.0;
            }

            animation_t * animation = model->animations;
            for (int i = 0; i < animation->keyframe_count; ++i) {
                keyframe_t* keyframe = animation->keyframe+i;
                if(keyframe->time<=currTime){
                    prevFrame = keyframe;
                }
            }
            for (int i = 0; i < animation->keyframe_count; ++i) {
                keyframe_t* keyframe = animation->keyframe+i;
                if(keyframe->time>currTime){
                    nextFrame = keyframe;
                    break;
                }
            }

            float interp = (currTime- prevFrame->time)/(nextFrame->time-prevFrame->time);
            quat rotation = slerp(prevFrame->rotation,nextFrame->rotation,interp);
            vec3 angle = eulerAngles(rotation);
            mat4 m = model->transform;
            m = rotate(m,angle.x,vec3(1.0,0.0,0.0))
                * rotate(m,angle.y,vec3(0.0,1.0,0.0))
                * rotate(m,angle.z,vec3(0.0,0.0,1.0));
            SetMaterialParam_mat4(model->shader,"M", value_ptr(m));

            currTime += 0.0001;
        }

        for (int i = 0; i < model->meshes_size; ++i) {
            mesh * mesh = &model->meshes[i];
            material * mat = &mesh->material;

            SetMaterialParam_vec4(model->shader,"base_color", value_ptr(mat->baseColor));

            glBindVertexArray(mesh->vao);
            glDrawElements(GL_TRIANGLES,mesh->indices_count,GL_UNSIGNED_SHORT,nullptr);
            glBindVertexArray(0);
        }
    }

    game_time += delta;
}

int Renderer::LoadModel(const char *filename,model* model) {
    cgltf_options options{};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, filename, &data);
    if (result == cgltf_result_success)
    {
        /* TODO make awesome stuff */

        cgltf_load_buffers(&options,data,filename);

        std::cout << "Success load : " << filename << std::endl;
    }else{
        std::cerr << "Can't find file : " << filename << std::endl;
        return 0;
    }

    if(data->meshes_count<1){
        return 0;
    }

    std::cout << "Upload BaseShader ..." << std::endl;
    uint32_t baseShader = Shader::LoadBaseShader();
    model->shader = baseShader;

    cgltf_mesh cmesh = data->meshes[0];
    for (int i = 0; i < cmesh.primitives_count; ++i) {
        mesh mesh;

        std::cout << "Upload Mesh ..." << std::endl;

        glGenVertexArrays(1,&mesh.vao);
        glBindVertexArray(mesh.vao);

        uint32_t vbo = 0;


        std::cout << "Mesh : " << cmesh.name << std::endl;

        cgltf_primitive primitive = cmesh.primitives[i];

        cgltf_accessor* position_accessor = nullptr;
        cgltf_accessor* normal_accessor = nullptr;
        cgltf_accessor* texcoord_accessor = nullptr;
        cgltf_accessor* indices_accessor = primitive.indices;

        for (int i = 0; i < primitive.attributes_count; ++i) {
            auto attr = primitive.attributes[i];
            std::cout << "Attr L : " << attr.name << std::endl;
            if(strcmp(attr.name,"POSITION")==0){
                std::cout << "Attr : " << attr.name << std::endl;
                position_accessor = attr.data;
                continue;
            }
            if(strcmp(attr.name,"NORMAL")==0){
                std::cout << "Attr : " << attr.name << std::endl;
                normal_accessor = attr.data;
                continue;
            }
            if(strcmp(attr.name,"TEXCOORD_0")==0){
                std::cout << "Attr : " << attr.name << std::endl;
                texcoord_accessor = attr.data;
                continue;
            }
        }

        int offset = position_accessor->buffer_view->offset;
        void* vertices_buffer = (uint8_t*)(position_accessor->buffer_view->buffer->data)+offset;
        int vertices_num = position_accessor->count;
        int vertices_size = position_accessor->buffer_view->size;

        offset = normal_accessor->buffer_view->offset;
        void* normal_buffer = (uint8_t*)(normal_accessor->buffer_view->buffer->data)+offset;
        int normal_num = normal_accessor->count;
        int normal_size = normal_accessor->buffer_view->size;

        void* texcoord0_buffer;
        int texcoord0_num;
        int texcoord0_size;
        if(texcoord_accessor){
            offset = texcoord_accessor->buffer_view->offset;
            texcoord0_buffer = (uint8_t*)(texcoord_accessor->buffer_view->buffer->data)+offset;
            texcoord0_num = texcoord_accessor->count;
            texcoord0_size = texcoord_accessor->buffer_view->size;
        }

        offset = indices_accessor->buffer_view->offset;
        void* indices_buffer = (uint8_t*)(indices_accessor->buffer_view->buffer->data)+offset;
        int indices_num = indices_accessor->count;
        int indices_size = indices_accessor->buffer_view->size;

        mesh.vertices_count = vertices_num;
        mesh.normal_count = normal_num;
        mesh.texcoord0_count = texcoord0_num;
        mesh.indices_count = indices_num;

        //vertices
        glGenBuffers(1,&vbo);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,vertices_size,vertices_buffer,GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);

        //normals
        glGenBuffers(1,&vbo);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,normal_size,normal_buffer,GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);

        //texcoords
        if(texcoord_accessor){
            glGenBuffers(1,&vbo);
            glBindBuffer(GL_ARRAY_BUFFER,vbo);
            glBufferData(GL_ARRAY_BUFFER,texcoord0_size,texcoord0_buffer,GL_STATIC_DRAW);
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,2*sizeof(float),nullptr);
        }

        uint32_t ebo = 0 ;
        glGenBuffers(1,&ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices_size,indices_buffer,GL_STATIC_DRAW);

        glBindVertexArray(0);

        if(primitive.material->has_pbr_metallic_roughness){
            std::cout << "Upload Material ..." << std::endl;
            cgltf_material* cmat = primitive.material;
            cgltf_float* baseColor = cmat->pbr_metallic_roughness.base_color_factor;
            mesh.material.baseColor = {baseColor[0],baseColor[1],baseColor[2],baseColor[3]};
            mesh.material.metallic = cmat->pbr_metallic_roughness.metallic_factor;
            mesh.material.roughness = cmat->pbr_metallic_roughness.roughness_factor;
        }

        model->meshes[model->meshes_size++] = mesh;
    }

    cgltf_free(data);
    return 1;
}

int Renderer::LoadAnimateModel(const char *filename,model* model) {
    cgltf_options options{};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, filename, &data);
    if (result == cgltf_result_success)
    {
        cgltf_load_buffers(&options,data,filename);

        std::cout << "Success load : " << filename << std::endl;
    }else{
        std::cerr << "Can't find file : " << filename << std::endl;
        return 0;
    }

    if(data->meshes_count<1){
        return 0;
    }

    std::cout << "Upload AnimateShader ..." << std::endl;
    uint32_t shader = Shader::LoadAnimateShader();
    model->shader = shader;

    for (int i = 0; i < data->nodes_count; ++i) {
        cgltf_node* cnode = &data->nodes[i];
        if(!cnode->mesh)continue;

        cgltf_mesh* cmesh = cnode->mesh;

        if(cnode->has_translation){
            model->transform = translate(model->transform, make_vec3(cnode->translation));
        }
        if(cnode->has_rotation){
            quat r = make_quat(cnode->rotation);
            vec3 angle = eulerAngles(r);
            model->transform = rotate(model->transform,angle.x,vec3(1.0,0.0,0.0));
            model->transform = rotate(model->transform,angle.y,vec3(0.0,1.0,0.0));
            model->transform = rotate(model->transform,angle.z,vec3(0.0,0.0,1.0));
        }
        if(cnode->has_scale){
            model->transform = scale(model->transform, make_vec3(cnode->scale));
        }

        std::cout << "Node Mesh : " << cmesh->name << std::endl;

        for (int j = 0; j < cmesh->primitives_count; ++j) {
            mesh mesh;
            std::cout << "Upload Mesh ..." << std::endl;

            glGenVertexArrays(1,&mesh.vao);
            glBindVertexArray(mesh.vao);

            uint32_t vbo = 0;

            cgltf_primitive primitive = cmesh->primitives[j];

            cgltf_accessor* position_accessor = nullptr;
            cgltf_accessor* indices_accessor = primitive.indices;

            for (int i = 0; i < primitive.attributes_count; ++i) {
                auto attr = primitive.attributes[i];
                std::cout << "Attr L : " << attr.name << std::endl;
                if(strcmp(attr.name,"POSITION")==0){
                    std::cout << "Attr : " << attr.name << std::endl;
                    position_accessor = attr.data;
                    continue;
                }
            }

            int offset = position_accessor->buffer_view->offset;
            void* vertices_buffer = (uint8_t*)(position_accessor->buffer_view->buffer->data)+offset;
            int vertices_num = position_accessor->count;
            int vertices_size = position_accessor->buffer_view->size;

            offset = indices_accessor->buffer_view->offset;
            void* indices_buffer = (uint8_t*)(indices_accessor->buffer_view->buffer->data)+offset;
            int indices_num = indices_accessor->count;
            int indices_size = indices_accessor->buffer_view->size;

            mesh.vertices_count = vertices_num;
            mesh.indices_count = indices_num;

            //vertices
            glGenBuffers(1,&vbo);
            glBindBuffer(GL_ARRAY_BUFFER,vbo);
            glBufferData(GL_ARRAY_BUFFER,vertices_size,vertices_buffer,GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);

            uint32_t ebo = 0 ;
            glGenBuffers(1,&ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices_size,indices_buffer,GL_STATIC_DRAW);

            glBindVertexArray(0);

            mesh.material.baseColor = vec4{1.0};
            model->meshes[model->meshes_size++] = mesh;
        }
    }

    model->animation_count = data->animations_count;
    for (int i = 0; i < data->animations_count; ++i) {
        cgltf_animation* canimation = &data->animations[i];
        std::cerr << "Animation : " << canimation->name << std::endl;

        animation_t anim;
        for (int j = 0; j < canimation->channels_count; ++j) {
            cgltf_animation_channel* channel = &canimation->channels[j];

            cgltf_accessor* input = channel->sampler->input;
            cgltf_accessor* output = channel->sampler->output;

            for (int k = 0; k < input->count; ++k) {
                keyframe_t keyframe;
                keyframe.has_rotation = true;
                keyframe.time = *((float*)((uint8_t*)input->buffer_view->buffer->data + input->offset) + k);
                keyframe.rotation = make_quat((float*)((uint8_t *)(output->buffer_view->buffer->data)+output->offset) + k*4);
                anim.keyframe[anim.keyframe_count++] = keyframe;
            }
        }

        model->animations[i] = anim;
    }

    cgltf_free(data);
    return 1;
}
