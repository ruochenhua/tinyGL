﻿#version 330 core

layout(location=0) in vec3 in_pos;
layout(location=1) in vec3 in_normal;
layout(location=2) in vec2 in_texcoord;
layout(location=3) in vec3 in_tangent;
layout(location=4) in vec3 in_bitangent;

// out vec3 normal_world;
out vec3 frag_pos;
out vec3 frag_normal;
out vec2 frag_uv;
out mat3 TBN;
out vec3 out_tangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform mat3 normal_model_mat;


void main(){
    gl_Position = proj * view * model * vec4(in_pos, 1.0);
    frag_pos = (model * vec4(in_pos, 1.0)).xyz;

    // 法线没有位移，不需要w向量，且还需要一些特殊处理来处理不等比缩放时带来的问题
    frag_normal = normalize(mat3(transpose(inverse(normal_model_mat))) * in_normal);
    frag_uv = in_texcoord;
    
    vec3 tangent = in_tangent;
    out_tangent = tangent;
    
    vec3 T = normalize(vec3(model*vec4(in_tangent, 0.0)));
    vec3 B = normalize(vec3(model*vec4(in_bitangent, 0.0)));
    vec3 N = normalize(vec3(model*vec4(frag_normal, 0.0)));
    
    TBN = mat3(T, B, N);

    //ShadowCoord = depth_bias_mvp * vec4(vertexPosition_modelspace, 1);
}