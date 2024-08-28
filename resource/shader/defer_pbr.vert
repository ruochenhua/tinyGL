#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"

layout(location=0) in vec3 in_pos;
layout(location=1) in vec3 in_normal;
layout(location=2) in vec2 in_texcoord;

layout(std140, binding=0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 cam_pos;
} matrix_ubo;

out vec2 TexCoords;

void main(){
    mat4 model = matrix_ubo.model;
    gl_Position = vec4(in_pos, 1.0);
    TexCoords = in_texcoord;
}