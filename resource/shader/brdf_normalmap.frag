﻿#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"
#include "/common/brdf_common.glsl"

layout(std140, binding=1) uniform LIGHT_INFO_UBO {
	ivec4 has_dir_light;
    DirectionalLight directional_light;
	ivec4 point_light_count;
    PointLight point_lights[POINT_LIGHT_MAX];
} light_info_ubo;

out vec4 FragColor;

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 frag_uv;
in mat3 TBN;

layout(std140, binding=0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 cam_pos;
} matrix_ubo;

uniform vec3 albedo;    // color
uniform float specular_factor;
uniform float metallic;
uniform float roughness;
uniform float ao;

uniform sampler2D diffuse_texture;
uniform sampler2D specular_texture;
uniform sampler2D normal_texture;
uniform sampler2D diffuse_roughness_tex;

vec3 GetAlbedo()
{
    float texture_size = textureSize(diffuse_texture, 0).x;
    if(texture_size > 1.0)
    {
        vec3 texture_albedo = texture(diffuse_texture, frag_uv).rgb;
        // sRGB空间转换到线性空间
        return pow(texture_albedo, vec3(2.2));
    }

    return albedo;
}

vec3 GetNormal()
{
    float texture_size = textureSize(normal_texture, 0).x;
    if(texture_size > 1.0)
    {
        vec3 texture_normal = texture(normal_texture, frag_uv).rgb;
        // 从[0,1]映射到[-1,1]
        vec3 tex_normal = normalize(texture_normal*2.0 - 1.0);
        return normalize(TBN * tex_normal);
    }

    return frag_normal;
}

vec3 GetSpecular()
{
    float texture_size = textureSize(specular_texture, 0).x;
    if(texture_size > 1.0)
    {
        return texture(specular_texture, frag_uv).rgb;
    }

    return vec3(1);
}

float GetRoughness()
{
    float texture_size = textureSize(diffuse_roughness_tex, 0).x;
    if(texture_size > 1.0)
    {
        return texture(diffuse_roughness_tex, frag_uv).r;
    }

    return roughness;
}



vec3 CalcLight(vec3 light_color, vec3 to_light_dir, vec3 normal, vec3 view)
{
    BRDFMaterial material;
    material.roughness = GetRoughness();
    material.metallic = metallic;
    material.albedo = GetAlbedo();
    material.specular_factor = specular_factor;
    material.ao = ao;

    return CalcLight_BRDF(light_color, to_light_dir, normal, view, material);
}

// calculate color causes by directional light
vec3 CalcDirLight(DirectionalLight dir_light, vec3 normal, vec3 view)
{
    vec3 light_color = dir_light.light_color.xyz;
    vec3 to_light_dir = -dir_light.light_dir.xyz;

    return CalcLight(light_color, to_light_dir, normal, view);
}

vec3 CalcPointLight(PointLight point_light, vec3 normal, vec3 view, vec3 in_frag_pos)
{
    vec3 light_color = point_light.light_color.xyz;
    vec3 light_dir = normalize(point_light.light_pos.xyz - in_frag_pos);

    vec3 point_light_color = CalcLight(light_color, light_dir, normal, view);

    float distance = length(point_light.light_pos.xyz - in_frag_pos);
    
    float attenuation = 1.0 / (distance * distance);	//衰减和点光源的参数可控，这里先简单弄个
    return point_light_color * attenuation;
}

void main()
{
    vec3 view = normalize(matrix_ubo.cam_pos - frag_pos);
    vec3 obj_normal = GetNormal();

    vec3 dir_light_color = vec3(0,0,0);
	if(light_info_ubo.has_dir_light.x > 0)
	{
		dir_light_color = CalcDirLight(light_info_ubo.directional_light, obj_normal, view);
	}

    vec3 point_light_color = vec3(0,0,0);
    for(int i = 0; i <  min(light_info_ubo.point_light_count.x,4); ++i)
    {
        point_light_color += CalcPointLight(light_info_ubo.point_lights[i], obj_normal, view, frag_pos);
    }

    vec3 ambient = vec3(0.03)*GetAlbedo()*ao;
    vec3 color = ambient + dir_light_color + point_light_color;
    // 伽马校正（Reinhard）
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));
    //FragColor = vec4((obj_normal+1)/2, 1.0);
    FragColor = vec4(color, 1.0);
}