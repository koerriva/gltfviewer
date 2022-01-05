//
// Created by koerriva on 2021/12/30.
//
#include <iostream>
#include <unordered_map>
#include "renderer.h"
#include "glm/ext.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glad/glad.h"

#define TINYGLTF_USE_CPP14
#define TINYGLTF_USE_RAPIDJSON
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

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

    std::unordered_map<int,transform_t*> animate_nodes;
    for (auto& cnode:cmodel.nodes) {
        if(cnode.mesh<0)continue;

        //load skin
        if(cnode.skin!=-1){
            Skin& cskin = cmodel.skins[cnode.skin];

            model->has_skin = 1;
            skeleton_t& skeleton = model->skeleton;

            std::cout << "Skin : " << cskin.name << std::endl;

            for (int i = 0; i < cskin.joints.size(); ++i) {
                skeleton.inverse_bind_matrices[i] = mat4(1.0f);
            }

            skeleton.joints_count = 0;
            for (auto& cjoint_id:cskin.joints) {
                Node& cjoint = cmodel.nodes[cjoint_id];
                joint_t& joint = skeleton.joins[skeleton.joints_count++];

                joint.id = cjoint_id;
                joint.transform.position = vec3(0.0f);
                joint.transform.rotation = quat(1.0f,0.0,0.0,0.0);
                joint.transform.scale = vec3(1.0f);

                if(cjoint.translation.size() == 3){
                    joint.transform.position = make_vec3(cjoint.translation.data());
                }
                if(cjoint.rotation.size() == 4){
                    joint.transform.rotation = make_quat(cjoint.rotation.data());
                }
                if(cjoint.scale.size() == 3){
                    joint.transform.scale = make_vec3(cjoint.scale.data());
                }

                animate_nodes.insert(std::pair(cjoint_id,&joint.transform));
            }

            Accessor * accessor = &cmodel.accessors[cskin.inverseBindMatrices];
            BufferView* bufferView = &cmodel.bufferViews[accessor->bufferView];
            Buffer* buffer = &cmodel.buffers[bufferView->buffer];

            size_t offset = accessor->byteOffset+bufferView->byteOffset;
            size_t data_count = accessor->count;
            auto* ptr = (float*)(buffer->data.data()+offset);
            for (int i = 0; i < data_count; ++i) {
                mat4 mat = make_mat4(ptr+i*16);
//                std::cout << "mat4 " << i << ":" << to_string(mat) << std::endl;
                mat = mat4{1.0f};
                skeleton.inverse_bind_matrices[i] = mat;
            }
        }

        //load mesh
        Mesh* cmesh = &cmodel.meshes[cnode.mesh];
        std::cout << "Upload Mesh : " << cmesh->name << std::endl;
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

        auto idx = std::find(cmodel.nodes.begin(), cmodel.nodes.end(), cnode) - cmodel.nodes.begin();
        animate_nodes.insert(std::pair(idx,&model->transform));

        model->mesh_count = 0;
        for (auto& primitive:cmesh->primitives) {
            mesh_t& mesh = model->meshes[model->mesh_count++];
            std::cout << "Upload Primitive " << std::endl;

            glGenVertexArrays(1,&mesh.vao);
            glBindVertexArray(mesh.vao);

            uint32_t vbo = 0;

            Accessor* position_accessor = nullptr;
            Accessor* normal_accessor = nullptr;
            Accessor* texcoord_accessor = nullptr;
            Accessor* joint_accessor = nullptr;
            Accessor* weight_accessor = nullptr;
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
                if(attr.first=="JOINTS_0"){
                    joint_accessor = &cmodel.accessors[attr.second];
                    continue;
                }
                if(attr.first=="WEIGHTS_0"){
                    weight_accessor = &cmodel.accessors[attr.second];
                    continue;
                }
            }

            if(position_accessor){
                BufferView* bufferView = &cmodel.bufferViews[position_accessor->bufferView];
                Buffer* buffer = &cmodel.buffers[bufferView->buffer];

                int offset = position_accessor->byteOffset + bufferView->byteOffset;
                int data_count = position_accessor->count;
                std::cout << "vertices : " << data_count << std::endl;
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

            if(joint_accessor){
                BufferView* bufferView = &cmodel.bufferViews[joint_accessor->bufferView];
                Buffer* buffer = &cmodel.buffers[bufferView->buffer];

                size_t offset = joint_accessor->byteOffset + bufferView->byteOffset;
                size_t data_count = joint_accessor->count;
                std::cout << "joints : " << data_count << std::endl;
                size_t byte_size = sizeof(uvec4)*data_count;

                std::vector<uint32_t> data;

                if(joint_accessor->componentType==GL_UNSIGNED_SHORT){
                    auto * ptr = (uint16_t*)(buffer->data.data()+offset);
                    for (int i = 0; i < data_count; ++i) {
                        uvec4 indices{*(ptr+i*8),*(ptr+i*8+1),*(ptr+i*8+2),*(ptr+i*8+3)};
                        uvec4 indices2{*(ptr+i*8+4),*(ptr+i*8+5),*(ptr+i*8+6),*(ptr+i*8+7)};
                        std::cout << "j " << i << ":" << to_string(indices) << "|" << to_string(indices2) << std::endl;
                        for (int j = 0; j < 4; ++j) {
                            data.push_back(*(ptr+i*8+j));
                        }
                    }
                }
                if(joint_accessor->componentType==GL_UNSIGNED_BYTE){
                    auto * ptr = (uint8_t*)(buffer->data.data()+offset);
                    for (int i = 0; i < data_count; ++i) {
                        uvec4 indices{*(ptr+i*4),*(ptr+i*4+1),*(ptr+i*4+2),*(ptr+i*4+3)};
                        std::cout << "j " << i << ":" << to_string(indices) << std::endl;
                        for (int j = 0; j < 4; ++j) {
                            data.push_back(*(ptr+i*4+j));
                        }
                    }
                }

                //joints
                glGenBuffers(1,&vbo);
                glBindBuffer(GL_ARRAY_BUFFER,vbo);
                glBufferData(GL_ARRAY_BUFFER,byte_size,data.data(),GL_STATIC_DRAW);
                glEnableVertexAttribArray(3);
                glVertexAttribIPointer(3,4,GL_UNSIGNED_INT,sizeof(uvec4),nullptr);
            }

            if(weight_accessor){
                BufferView* bufferView = &cmodel.bufferViews[weight_accessor->bufferView];
                Buffer* buffer = &cmodel.buffers[bufferView->buffer];

                size_t offset = weight_accessor->byteOffset + bufferView->byteOffset;
                size_t data_count = weight_accessor->count;
                std::cout << "weights : " << data_count << std::endl;
                size_t byte_size = sizeof(vec4)*data_count;

                auto * ptr = (float*)(buffer->data.data()+offset);
                std::vector<float> data;
                for (int i = 0; i < data_count; ++i) {
                    vec4 weights{*(ptr+i*4),*(ptr+i*4+1),*(ptr+i*4+2),*(ptr+i*4+3)};
                    std::cout << "w" << i << ":" << to_string(weights) << std::endl;
                    for (int j = 0; j < 4; ++j) {
                        data.push_back(*(ptr+i*4+j));
                    }
                }

                //weights
                glGenBuffers(1,&vbo);
                glBindBuffer(GL_ARRAY_BUFFER,vbo);
                glBufferData(GL_ARRAY_BUFFER,byte_size,data.data(),GL_STATIC_DRAW);
                glEnableVertexAttribArray(4);
                glVertexAttribPointer(4,4,GL_FLOAT,GL_FALSE,sizeof(vec4),nullptr);
            }

            glBindVertexArray(0);

            mesh.material.baseColor = vec4(1.0f);
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
        }
    }

    model->animation_count = 0;
    for (auto& canimation:cmodel.animations) {
        std::cout << "Animation : " << canimation.name << std::endl;

        animation_t& anim = model->animations[model->animation_count++];
        memset(anim.name,'\0',sizeof(anim.name));
        if(canimation.name.empty()){
            strcpy(anim.name,"default");
        }else{
            strcpy(anim.name,canimation.name.data());
        }
        anim.channel_count = 0;
        for (auto& cchannel:canimation.channels) {
            auto& channel = anim.channels[anim.channel_count++];
            AnimationSampler* sampler = &canimation.samplers[cchannel.sampler];
            channel.interpolation = 0;
            if(sampler->interpolation=="STEP"){
                channel.interpolation = 1;
            }
            if(sampler->interpolation=="CUBIC"){
                channel.interpolation = 2;
            }

            channel.transform = animate_nodes[cchannel.target_node];

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
                keyframe.translation = vec3(0.0f);
                keyframe.rotation = quat(1.0f,0.0f,0.0f,0.0f);
                keyframe.scale = vec3(1.0f);

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
            }
        }
    }

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