//
// Created by koerriva on 2021/12/29.
//
#include <iostream>
#include "renderer.h"
#include "glad/glad.h"

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


unsigned Shader::LoadBaseShader(){
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

uint32_t Shader::LoadAnimateShader() {
    const char* vert_shader_source = R"(
    #version 430
    layout (location = 0) in vec3 position;
    layout (location = 1) in vec3 normal;
    layout (location = 2) in vec2 texcoord;
    layout (location = 3) in uvec4 a_joint;
    layout (location = 4) in vec4 a_weight;

    uniform float time;
    uniform mat4 u_jointMat[64];
    uniform int hasSkin;

    uniform mat4 P;
    uniform mat4 V;
    uniform mat4 M;

    out vec2 v_TexCoord;

    void main(){
        mat4 modelMat = M;
        if(hasSkin==1){
            mat4 skinMat =
                a_weight.x * u_jointMat[a_joint.x] +
                a_weight.y * u_jointMat[a_joint.y] +
                a_weight.z * u_jointMat[a_joint.z] +
                a_weight.w * u_jointMat[a_joint.w];
            modelMat = skinMat;
        }

        gl_Position = P*V*modelMat*vec4(position,1.0);
        v_TexCoord = texcoord;
    }
)";
    const char* frag_shader_source = R"(
    #version 430

    in vec2 v_TexCoord;

    out vec4 FragColor;
    uniform vec4 base_color;

    uniform int hasBaseColorTexture;
    uniform sampler2D baseColorTexture;

    void main(){
        vec4 diffuse = vec4(1);
        if(hasBaseColorTexture==1){
            diffuse = texture(baseColorTexture,v_TexCoord);
        }
        FragColor = vec4(base_color*diffuse);
    }
)";

    auto vertexShader = CreateShader(vert_shader_source,GL_VERTEX_SHADER);
    auto fragmentShader = CreateShader(frag_shader_source,GL_FRAGMENT_SHADER);
    uint32_t program = CreateProgram(vertexShader,fragmentShader);
    return program;
}
