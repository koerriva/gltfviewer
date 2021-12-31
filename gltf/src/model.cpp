//
// Created by koerriva on 2021/12/30.
//
#include <iostream>
#include <unordered_map>
#include "renderer.h"
#include "glm/ext.hpp"
#include "glad/glad.h"
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

int Assets::LoadModel(const char *filename, model_t* model) {
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

        int offset = position_accessor->offset + position_accessor->buffer_view->offset;
        void* vertices_buffer = (uint8_t*)(position_accessor->buffer_view->buffer->data)+offset;
        int vertices_num = position_accessor->count;
        int vertices_size = position_accessor->buffer_view->size;

        offset = normal_accessor->offset + normal_accessor->buffer_view->offset;
        void* normal_buffer = (uint8_t*)(normal_accessor->buffer_view->buffer->data)+offset;
        int normal_num = normal_accessor->count;
        int normal_size = normal_accessor->buffer_view->size;

        void* texcoord0_buffer;
        int texcoord0_num;
        int texcoord0_size;
        if(texcoord_accessor){
            offset = texcoord_accessor->offset + texcoord_accessor->buffer_view->offset;
            texcoord0_buffer = (uint8_t*)(texcoord_accessor->buffer_view->buffer->data)+offset;
            texcoord0_num = texcoord_accessor->count;
            texcoord0_size = texcoord_accessor->buffer_view->size;
        }

        offset = indices_accessor->offset + indices_accessor->buffer_view->offset;
        void* indices_buffer = (uint8_t*)(indices_accessor->buffer_view->buffer->data)+offset;
        int indices_num = indices_accessor->count;
        int indices_size = indices_accessor->buffer_view->size;
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
        }

        model->meshes[model->meshes_size++] = mesh;
    }

    cgltf_free(data);
    return 1;
}

int Assets::LoadAnimateModel(const char *filename, model_t* model) {
    cgltf_options options{};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, filename, &data);
    if (result == cgltf_result_success)
    {
        cgltf_load_buffers(&options,data,filename);

        std::cout << "Success load : " << filename << std::endl;
    }else{
        std::cout << "Can't find file : " << filename << std::endl;
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
        model->transform.position = vec3(0.0);
        model->transform.rotation = quat(1.0,0.0,0.0,0.0);
        model->transform.scale = vec3(1.0);

        if(cnode->has_translation){
            model->transform.position = make_vec3(cnode->translation);
        }
        if(cnode->has_rotation){
            model->transform.rotation = make_quat(cnode->rotation);
        }
        if(cnode->has_scale){
            model->transform.scale = make_vec3(cnode->scale);
        }

        model->meshes_size = cmesh->primitives_count;
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
                if(strcmp(attr.name,"POSITION")==0){
                    position_accessor = attr.data;
                    continue;
                }
            }

            int offset = position_accessor->offset + position_accessor->buffer_view->offset;
            void* vertices_buffer = (uint8_t*)(position_accessor->buffer_view->buffer->data)+offset;
            int vertices_num = position_accessor->count;
            int vertices_size = position_accessor->buffer_view->size;

            offset = indices_accessor->offset + indices_accessor->buffer_view->offset;
            void* indices_buffer = (uint8_t*)(indices_accessor->buffer_view->buffer->data)+offset;
            int indices_num = indices_accessor->count;
            int indices_size = indices_accessor->buffer_view->size;

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
            model->meshes[j] = mesh;
        }
    }

    model->animation_count = data->animations_count;
    for (int i = 0; i < data->animations_count; ++i) {
        cgltf_animation* canimation = &data->animations[i];
        std::cout << "Animation : " << std::endl;

        animation_t anim;
        anim.channel_count = canimation->channels_count;
        for (int j = 0; j < canimation->channels_count; ++j) {
            cgltf_animation_channel* channel = &canimation->channels[j];

            cgltf_accessor* input = channel->sampler->input;
            cgltf_accessor* output = channel->sampler->output;

            anim.channels[j].keyframe_count = input->count;
            anim.channels[j].interpolation = channel->sampler->interpolation;

            for (int k = 0; k < input->count; ++k) {
                keyframe_t keyframe = anim.channels[j].keyframe[k];

                keyframe.time = *((float*)((uint8_t*)input->buffer_view->buffer->data + input->offset + input->buffer_view->offset) + k);

                if(channel->target_path==cgltf_animation_path_type_translation){
                    anim.channels[j].has_translation = true;
                    keyframe.translation = make_vec3((float*)((uint8_t *)(output->buffer_view->buffer->data)+output->offset+output->buffer_view->offset) + k*3);
                }
                if(channel->target_path==cgltf_animation_path_type_rotation){
                    anim.channels[j].has_rotation = true;
                    keyframe.rotation = make_quat((float*)((uint8_t *)(output->buffer_view->buffer->data)+output->offset+output->buffer_view->offset) + k*4);
                }
                if(channel->target_path==cgltf_animation_path_type_scale){
                    anim.channels[j].has_scale = true;
                    keyframe.scale = make_vec3((float*)((uint8_t *)(output->buffer_view->buffer->data)+output->offset+output->buffer_view->offset) + k*3);
                }

                anim.channels[j].keyframe[k] = keyframe;
            }

        }

        model->animations[i] = anim;
    }

    model->skeleton_count = data->skins_count;
    for (int i = 0; i < data->skins_count; ++i) {
        cgltf_skin* cskin = &data->skins[i];
        std::cout << "Skin : " << std::endl;

        skeleton_t skeleton;
        skeleton.joints_count = cskin->joints_count;
        for (int j = 0; j < cskin->joints_count; ++j) {
            cgltf_node* cjoint = cskin->joints[j];
            skeleton.joins[j].id = j;
            transform_t transform;
            if(cjoint->has_translation){
                transform.position += make_vec3(cjoint->translation);
            }
            if(cjoint->has_rotation){
                transform.rotation = make_quat(cjoint->rotation);
            }
            if(cjoint->has_scale){
                transform.scale = make_vec3(cjoint->scale);
            }
            skeleton.joins[j].transform = transform;
        }

        cgltf_accessor * accessor = cskin->inverse_bind_matrices;
        for (int j = 0; j < accessor->count; ++j) {
//            float* buffer
        }
    }

    cgltf_free(data);
    return 1;
}