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
int GetLocation(material* mat,const char* name){
    auto iter = locations.find(name);
    if(iter==locations.end()){
        int location = glGetUniformLocation(mat->pid,name);
        locations.insert({name,location});
        return location;
    }
    return iter->second;
}

void SetMaterialParam_float(material* mat,const char* name,float value){
    int location = GetLocation(mat,name);
    glUniform1f(location,value);
}
void SetMaterialParam_mat4(material* mat,const char* name,float* value){
    int location = GetLocation(mat,name);
    glUniformMatrix4fv(location,1,GL_FALSE,value);
}
void SetMaterialParam_vec4(material* mat,const char* name,float* value){
    int location = GetLocation(mat,name);
    glUniform4fv(location,1,value);
}

void Renderer::SetRenderMode(render_mode mode) {
    this->m_mode = mode;
}

void Renderer::Render(Camera *camera, model* model) {
    glEnable(GL_DEPTH_TEST);

    auto p = camera->GetProjection();
    auto v = camera->GetViewMatrix();
    auto m = mat4{1};
    m = translate(m,vec3{0,-1.f,0.f});
    m = rotate(m, radians(1.f+game_time),vec3{0.f,1.f,0.f});
    m = scale(m,vec3{1.f});

    glClearColor(0.2f,0.3f,0.634f,0.6778f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    if(m_mode==wireframe){
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    if(m_mode==shader){
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    {

        for (int i = 0; i < model->meshes_size; ++i) {
            mesh * mesh = &model->meshes[i];
            material * mat = &mesh->material;
            glUseProgram(mat->pid);

            SetMaterialParam_float(mat,"time",game_time);
            SetMaterialParam_mat4(mat,"P", value_ptr(p));
            SetMaterialParam_mat4(mat,"V", value_ptr(v));
            SetMaterialParam_mat4(mat,"M", value_ptr(m));

            SetMaterialParam_vec4(mat,"base_color", value_ptr(mat->baseColor));
            SetMaterialParam_float(mat,"metallic",mat->metallic);
            SetMaterialParam_float(mat,"roughness",mat->roughness);

            glBindVertexArray(mesh->vao);
            glDrawElements(GL_TRIANGLES,mesh->indices_count,GL_UNSIGNED_SHORT,nullptr);
//        glBindVertexArray(0);
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
    uint32_t baseShader = Renderer::LoadBaseShader();

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
            mesh.material.pid = baseShader;
        }

        model->meshes[model->meshes_size++] = mesh;
    }

    cgltf_free(data);
    return 1;
}

uint32_t CreateShader(const char* source,uint32_t type){
    unsigned int shader = glCreateShader(type);
    if (type==GL_VERTEX_SHADER){
        glShaderSource(shader,1,&source, nullptr);

    }else if(type==GL_FRAGMENT_SHADER){
        glShaderSource(shader,1,&source, nullptr);
    }else{
        std::cerr << "Unsupported shader " << std::endl;
        exit(-1001);
    }
    glCompileShader(shader);
    int success;
    char info[512];
    glGetShaderiv(shader,GL_COMPILE_STATUS,&success);
    if(!success){
        glGetShaderInfoLog(shader,512, nullptr,info);
        std::cerr << "Compile shader :" << info << std::endl;
        exit(-1002);
    }
    return shader;
}

uint32_t CreateProgram(unsigned vertShader,unsigned fragShader){
    unsigned int program = glCreateProgram();
    glAttachShader(program,vertShader);
    glAttachShader(program,fragShader);
    glLinkProgram(program);
    int success;
    char info[512];
    glGetProgramiv(program,GL_LINK_STATUS, &success);
    if(!success){
        glGetProgramInfoLog(program,512, nullptr,info);
        std::cerr << "Compile program :" << info << std::endl;
        exit(-1002);
    }
    return program;
}

unsigned Renderer::LoadBaseShader(){
    const char* vert_shader_source = R"(
    #version 430

    layout (location = 0) in vec3 position;
    layout (location = 1) in vec3 normal;
    layout (location = 2) in vec2 texcoord;

    uniform float time;

    uniform mat4 P;
    uniform mat4 V;
    uniform mat4 M;

    out vec2 v_TexCoord;
    out vec3 v_WorldPos;
    out vec3 v_Normal;

    void main(){
        gl_Position = P*V*M*vec4(position,1.0);
        v_TexCoord = texcoord;
        v_Normal = mat3(transpose(inverse(M))) * normal;
        v_WorldPos = (M*vec4(position,1.0)).xyz;
    }
)";
    const char* frag_shader_source = R"(
    #version 430
    const float pi = 3.1415926;

    out vec4 FragColor;

    in vec2 v_TexCoord;
    in vec3 v_WorldPos;
    in vec3 v_Normal;

    vec3 light_pos = vec3(2.0,2.0,0.0);
    vec3 light_color = vec3(1.0);
    vec4 sky_color = vec4(0.2f,0.3f,0.634f,0.6778f);

    uniform vec4 base_color;
    uniform float metallic;
    uniform float roughness;

    void main(){
        vec3 light_dir = normalize(light_pos-v_WorldPos);

        vec3 ambient = sky_color.rgb * vec3(0.6);
        vec3 diffuse = max(dot(v_Normal,light_dir),0.0)*light_color;

        FragColor = vec4(base_color.rgb*(ambient+diffuse),base_color.a);
    }
)";

    auto vertexShader = CreateShader(vert_shader_source,GL_VERTEX_SHADER);
    auto fragmentShader = CreateShader(frag_shader_source,GL_FRAGMENT_SHADER);
    uint32_t program = CreateProgram(vertexShader,fragmentShader);
    return program;
}

