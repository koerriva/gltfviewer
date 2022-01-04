//
// Created by koerriva on 2021/12/30.
//
#include <iostream>
#include <unordered_map>
#include "renderer.h"
#include "glm/ext.hpp"
#include "glad/glad.h"

#define TINYGLTF_USE_CPP14
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

int Assets::LoadModel(const char *filename, model_t* model) {
    using namespace tinygltf;

    Model cmodel;
    TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = loader.LoadASCIIFromFile(&cmodel, &err, &warn, filename);
//bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, argv[1]); // for binary glTF(.glb)

    if (!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty()) {
        printf("Err: %s\n", err.c_str());
    }

    if (!ret) {
        printf("Failed to parse glTF\n");
        return -1;
    }

    std::cout << "Upload BaseShader ..." << std::endl;
    uint32_t baseShader = Shader::LoadBaseShader();
    model->shader = baseShader;

    auto& cmesh = cmodel.meshes[0];

    for (auto& primitive : cmesh.primitives) {
        mesh mesh;

        std::cout << "Upload Mesh ..." << std::endl;

        glGenVertexArrays(1,&mesh.vao);
        glBindVertexArray(mesh.vao);

        uint32_t vbo = 0;


        std::cout << "Mesh : " << cmesh.name << std::endl;

        Accessor* position_accessor = nullptr;
        Accessor* normal_accessor = nullptr;
        Accessor* texcoord_accessor = nullptr;
        Accessor* indices_accessor = nullptr;

        for (const auto& attr : primitive.attributes) {
            std::cout << "Attr L : " << attr.first << std::endl;
            if(attr.first=="POSITION"){
                std::cout << "Attr : " << attr.first << std::endl;
                position_accessor = &cmodel.accessors[attr.second];
                continue;
            }
            if(attr.first=="NORMAL"){
                std::cout << "Attr : " << attr.first << std::endl;
                normal_accessor = &cmodel.accessors[attr.second];
                continue;
            }
            if(attr.first=="TEXCOORD_0"){
                std::cout << "Attr : " << attr.first << std::endl;
                texcoord_accessor = &cmodel.accessors[attr.second];
                continue;
            }
        }

        BufferView* bufferView = nullptr;
        Buffer* buffer = nullptr;
        if(position_accessor){
            bufferView = &cmodel.bufferViews[position_accessor->bufferView];
            buffer = &cmodel.buffers[bufferView->buffer];

            int offset = position_accessor->byteOffset + bufferView->byteOffset;

            //vertices
            glGenBuffers(1,&vbo);
            glBindBuffer(GL_ARRAY_BUFFER,vbo);
            glBufferData(GL_ARRAY_BUFFER,bufferView->byteLength,buffer->data.data()+offset,GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);
        }
        if(normal_accessor){
            bufferView = &cmodel.bufferViews[normal_accessor->bufferView];
            buffer = &cmodel.buffers[bufferView->buffer];

            int offset = normal_accessor->byteOffset + bufferView->byteOffset;

            //normals
            glGenBuffers(1,&vbo);
            glBindBuffer(GL_ARRAY_BUFFER,vbo);
            glBufferData(GL_ARRAY_BUFFER,bufferView->byteLength,buffer->data.data()+offset,GL_STATIC_DRAW);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);
        }
        if(texcoord_accessor){
            bufferView = &cmodel.bufferViews[texcoord_accessor->bufferView];
            buffer = &cmodel.buffers[bufferView->buffer];

            int offset = texcoord_accessor->byteOffset + bufferView->byteOffset;

            //normals
            glGenBuffers(1,&vbo);
            glBindBuffer(GL_ARRAY_BUFFER,vbo);
            glBufferData(GL_ARRAY_BUFFER,bufferView->byteLength,buffer->data.data()+offset,GL_STATIC_DRAW);
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,2*sizeof(float),nullptr);
        }
        if(indices_accessor){
            bufferView = &cmodel.bufferViews[indices_accessor->bufferView];
            buffer = &cmodel.buffers[bufferView->buffer];

            int offset = indices_accessor->byteOffset + bufferView->byteOffset;

            uint32_t ebo = 0 ;
            glGenBuffers(1,&ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,bufferView->byteLength,buffer->data.data()+offset,GL_STATIC_DRAW);
        }
        glBindVertexArray(0);

        if(primitive.material>=0){
            Material* cmat = &cmodel.materials[primitive.material];
            std::cout << "Upload Material ..." << std::endl;
            auto& baseColor = cmat->pbrMetallicRoughness.baseColorFactor;
            mesh.material.baseColor = make_vec4(baseColor.data());
        }

        model->meshes[model->mesh_count++] = mesh;
    }

    return 1;
}

int Assets::LoadAnimateModel(const char *filename, model_t* model) {
    using namespace tinygltf;

    Model cmodel;
    TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = loader.LoadASCIIFromFile(&cmodel, &err, &warn, filename);
//bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, argv[1]); // for binary glTF(.glb)

    if (!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty()) {
        printf("Err: %s\n", err.c_str());
    }

    if (!ret) {
        printf("Failed to parse glTF\n");
        return -1;
    }

    std::cout << "Upload AnimateShader ..." << std::endl;
    uint32_t shader = Shader::LoadAnimateShader();
    model->shader = shader;

    for (auto& cnode:cmodel.nodes) {
        if(cnode.mesh<0)continue;

        Mesh* cmesh = &cmodel.meshes[cnode.mesh];
        model->transform.position = vec3(0.0);
        model->transform.rotation = quat(1.0,0.0,0.0,0.0);
        model->transform.scale = vec3(1.0);

        if(!cnode.translation.empty()){
            model->transform.position = make_vec3(cnode.translation.data());
        }
        if(!cnode.translation.empty()){
            model->transform.rotation = make_quat(cnode.rotation.data());
        }
        if(!cnode.scale.empty()){
            model->transform.scale = make_vec3(cnode.scale.data());
        }

        model->mesh_count = 0;
        for (auto& primitive:cmesh->primitives) {
            mesh mesh;
            std::cout << "Upload Mesh ..." << std::endl;

            glGenVertexArrays(1,&mesh.vao);
            glBindVertexArray(mesh.vao);

            uint32_t vbo = 0;

            Accessor* position_accessor = nullptr;
            Accessor* normal_accessor = nullptr;
            Accessor* texcoord_accessor = nullptr;
            Accessor* indices_accessor = &cmodel.accessors[primitive.indices];

            for (auto& attr:primitive.attributes) {
                if(attr.first=="POSITION"){
                    position_accessor = &cmodel.accessors[attr.second];
                    continue;
                }
                if(attr.first=="NORMAL"){
                    normal_accessor = &cmodel.accessors[attr.second];
                    continue;
                }
                if(attr.first=="TEXCOORD_0"){
                    texcoord_accessor = &cmodel.accessors[attr.second];
                    continue;
                }
            }

            if(position_accessor){
                BufferView* bufferView = &cmodel.bufferViews[position_accessor->bufferView];
                Buffer* buffer = &cmodel.buffers[bufferView->buffer];

                int offset = position_accessor->byteOffset + bufferView->byteOffset;
                int data_count = position_accessor->count;
                int byte_size = bufferView->byteLength;

                //vertices
                glGenBuffers(1,&vbo);
                glBindBuffer(GL_ARRAY_BUFFER,vbo);
                glBufferData(GL_ARRAY_BUFFER,byte_size,buffer->data.data()+offset,GL_STATIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);
            }

            if(normal_accessor){
                BufferView* bufferView = &cmodel.bufferViews[normal_accessor->bufferView];
                Buffer* buffer = &cmodel.buffers[bufferView->buffer];

                int offset = normal_accessor->byteOffset + bufferView->byteOffset;
                int data_count = normal_accessor->count;
                int byte_size = bufferView->byteLength;

                //vertices
                glGenBuffers(1,&vbo);
                glBindBuffer(GL_ARRAY_BUFFER,vbo);
                glBufferData(GL_ARRAY_BUFFER,byte_size,buffer->data.data()+offset,GL_STATIC_DRAW);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);
            }

            if(texcoord_accessor){
                BufferView* bufferView = &cmodel.bufferViews[texcoord_accessor->bufferView];
                Buffer* buffer = &cmodel.buffers[bufferView->buffer];

                int offset = texcoord_accessor->byteOffset + bufferView->byteOffset;
                int data_count = texcoord_accessor->count;
                int byte_size = bufferView->byteLength;

                //texcoord
                glGenBuffers(1,&vbo);
                glBindBuffer(GL_ARRAY_BUFFER,vbo);
                glBufferData(GL_ARRAY_BUFFER,byte_size,buffer->data.data()+offset,GL_STATIC_DRAW);
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,2*sizeof(float),nullptr);
            }

            if(indices_accessor){
                BufferView* bufferView = &cmodel.bufferViews[indices_accessor->bufferView];
                Buffer* buffer = &cmodel.buffers[bufferView->buffer];

                int offset = indices_accessor->byteOffset + bufferView->byteOffset;
                int data_count = indices_accessor->count;
                int byte_size = bufferView->byteLength;

                mesh.indices_count = data_count;

                uint32_t ebo = 0 ;
                glGenBuffers(1,&ebo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER,byte_size,buffer->data.data()+offset,GL_STATIC_DRAW);
            }

            glBindVertexArray(0);

            if(primitive.material>-1){
                Material* cmat = &cmodel.materials[primitive.material];
                TextureInfo baseColorTextureInfo = cmat->pbrMetallicRoughness.baseColorTexture;
                if(baseColorTextureInfo.index>-1){
                    mesh.material.hasBaseColorTexture = 1;
                    Texture* baseColorTexture = &cmodel.textures[baseColorTextureInfo.index];
                    Sampler* sampler = &cmodel.samplers[baseColorTexture->sampler];
                    Image* image = &cmodel.images[baseColorTexture->source];

                    std::string& img_type = image->mimeType;

                    ivec3 shape{image->width,image->height,image->component};
                    int min_filter = sampler->minFilter;
                    int mag_filter = sampler->magFilter;
                    int warp_s = sampler->wrapS;
                    int warp_t = sampler->wrapT;

                    mesh.material.baseColorTexture = Assets::LoadTexture(shape,{min_filter,mag_filter},{warp_s,warp_t},image->image.data());
                }

                mesh.material.baseColor = make_vec4(cmat->pbrMetallicRoughness.baseColorFactor.data());
            }


            model->meshes[model->mesh_count++] = mesh;
        }
    }

    model->animation_count = 0;
    for (auto& canimation:cmodel.animations) {
        std::cout << "Animation : " << canimation.name << std::endl;

        animation_t anim;
        anim.channel_count = canimation.channels.size();
        int j=0;
        for (auto& cchannel:canimation.channels) {
            auto& channel = anim.channels[j];
            AnimationSampler* sampler = &canimation.samplers[cchannel.sampler];
            channel.interpolation = 0;
            if(sampler->interpolation=="STEP"){
                channel.interpolation = 1;
            }
            if(sampler->interpolation=="CUBIC"){
                channel.interpolation = 2;
            }

            Accessor* input = &cmodel.accessors[sampler->input];
            BufferView* inputBufferView = &cmodel.bufferViews[input->bufferView];
            Buffer* inputBuffer = &cmodel.buffers[inputBufferView->buffer];

            Accessor* output = &cmodel.accessors[sampler->output];
            BufferView* outputBufferView = &cmodel.bufferViews[output->bufferView];
            Buffer* outputBuffer = &cmodel.buffers[outputBufferView->buffer];

            channel.keyframe_count = input->count;
            for (int k = 0; k < input->count; ++k) {
                auto& keyframe = channel.keyframe[k];

                keyframe.time = *((float*)(inputBuffer->data.data() + input->byteOffset + inputBufferView->byteOffset) + k);

                if(cchannel.target_path=="translation"){
                    channel.has_translation = true;
                    keyframe.translation = make_vec3((float*)(outputBuffer->data.data()+output->byteOffset+outputBufferView->byteOffset) + k*3);
                }
                if(cchannel.target_path=="rotation"){
                    channel.has_rotation = true;
                    keyframe.rotation = make_quat((float*)(outputBuffer->data.data()+output->byteOffset+outputBufferView->byteOffset) + k*4);
                }
                if(cchannel.target_path=="scale"){
                    channel.has_scale = true;
                    keyframe.scale = make_vec3((float*)(outputBuffer->data.data()+output->byteOffset+outputBufferView->byteOffset) + k*3);
                }

                channel.keyframe[k] = keyframe;
            }

            j++;
        }

        model->animations[model->animation_count++] = anim;
    }

    model->skeleton_count = 0;
//    for (int i = 0; i < data->skins_count; ++i) {
//        cgltf_skin* cskin = &data->skins[i];
//        std::cout << "Skin : " << std::endl;
//
//        skeleton_t skeleton;
//        skeleton.joints_count = cskin->joints_count;
//        for (int j = 0; j < cskin->joints_count; ++j) {
//            cgltf_node* cjoint = cskin->joints[j];
//            skeleton.joins[j].id = j;
//            transform_t transform;
//            if(cjoint->has_translation){
//                transform.position += make_vec3(cjoint->translation);
//            }
//            if(cjoint->has_rotation){
//                transform.rotation = make_quat(cjoint->rotation);
//            }
//            if(cjoint->has_scale){
//                transform.scale = make_vec3(cjoint->scale);
//            }
//            skeleton.joins[j].transform = transform;
//        }
//
//        cgltf_accessor * accessor = cskin->inverse_bind_matrices;
//        for (int j = 0; j < accessor->count; ++j) {
////            float* buffer
//        }
//    }

    return 1;
}

uint32_t Assets::LoadTexture(ivec3 shape,ivec2 filter,ivec2 warp,const unsigned char *buffer) {
    int width = shape.x,height=shape.y,comp=shape.z;

    uint32_t texture;

    char buff[100]={0};
    sprintf(buff,"Image Info width=%d, height=%d, channels=%d\0",width,height,comp);
    std::cout << buff << std::endl;

    glGenTextures(1,&texture);
    //生成纹理
    glBindTexture(GL_TEXTURE_2D,texture);
    //为当前绑定的纹理对象设置环绕、过滤方式
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, warp.x>-1?warp.x:GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, warp.y>-1?warp.y:GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter.x>-1?filter.x:GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter.y>-1?filter.y:GL_LINEAR);
    if(comp==4){
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,buffer);
    }else if(comp==3){
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,buffer);
    }else{
        std::cerr << "Image Format Unsupported" << std::endl;
    }
    glGenerateMipmap(GL_TEXTURE_2D);
    return texture;
}