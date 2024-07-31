#version 330 core
// todo: 支持include，整合到一个地方
struct DirectionalLight
{
    vec3 light_dir;
    vec3 light_color;
};

struct PointLight
{
    vec3 light_pos;
    vec3 light_color;
};

#define POINT_LIGHT_MAX 4
const float PI = 3.14159265359;

out vec4 FragColor;

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 frag_uv;
in mat3 TBN;
in vec4 frag_pos_lightspace;

uniform vec3 cam_pos;

uniform DirectionalLight directional_light;
uniform PointLight point_lights[POINT_LIGHT_MAX];
uniform int point_light_count;

uniform vec3 albedo;    // color
uniform float metallic;
uniform float roughness;
uniform float ao;

uniform sampler2D diffuse_texture;
uniform sampler2D specular_texture;
uniform sampler2D normal_texture;
uniform sampler2D tangent_texture;
uniform sampler2D shadow_map;

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

// 计算阴影
float ShadowCalculation(vec4 pos_light_space, vec3 to_light_dir)
{
    // 转换到-1,1的范围，再转到0,1的范围
    vec3 proj_coords = pos_light_space.xyz / pos_light_space.w;
    proj_coords = proj_coords* 0.5 + 0.5;
    // 超出阴影贴图范围，就只返回0
    if(proj_coords.z > 1.0)
        return 0.0;

//    float closet_depth = texture(shadow_map, proj_coords.xy).r;
    float current_depth = proj_coords.z;
    float bias = max(0.0001 * (1.0 - dot(frag_normal, to_light_dir)), 0.0001);
//    float shadow = (current_depth - bias) > closet_depth ? 1.0 : 0.0;
    // 采用pcf柔和阴影锯齿边界
    // todo: 更多优化方法
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadow_map, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadow_map, proj_coords.xy + vec2(x, y) * texelSize).r;
            shadow += (current_depth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    return shadow;
}
// 完整的Cook-Torrance specular BRDF: DFG / 4*dot(N,V)*dot(N,L)

// 菲涅尔方程F
// Fresnel-Schlick近似法接收一个参数F0，被称为0°入射角的反射率，或者说是直接(垂直)观察表面时有多少光线会被反射。
// 这个参数F0会因为材料不同而不同，而且对于金属材质会带有颜色。
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// note: 根据迪士尼公司给出的观察以及后来被Epic Games公司采用的光照模型，在几何遮蔽函数和法线分布函数中采用粗糙度的平方会让光照看起来更加自然。

// 法线分布函数D
// 从统计学上近似地表示了与某些（半程）向量h取向一致的微平面的比率。
// 当粗糙度很低（也就是说表面很光滑）的时候，与半程向量取向一致的微平面会高度集中在一个很小的半径范围内。
// 由于这种集中性，NDF最终会生成一个非常明亮的斑点。但是当表面比较粗糙的时候，微平面的取向方向会更加的随机
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}
// 几何遮蔽函数G
// 从统计学上近似的求得了微平面间相互遮蔽的比率，这种相互遮蔽会损耗光线的能量。
// 几何函数采用一个材料的粗糙度参数作为输入参数，粗糙度较高的表面其微平面间相互遮蔽的概率就越高。
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1  = GeometrySchlickGGX(NdotV, roughness);
    float ggx2  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 CalcLight(vec3 light_color, vec3 to_light_dir, vec3 normal, vec3 view)
{
    vec3 h = normalize(to_light_dir + view);
    vec3 radiance = light_color;

    float NDF = DistributionGGX(normal, h, roughness);
    float G = GeometrySmith(normal, view, to_light_dir, roughness);
    vec3 F0 = vec3(0.04);
    vec3 obj_albedo = GetAlbedo();
    F0 = mix(F0, obj_albedo, metallic);

    vec3 F = FresnelSchlick(clamp(dot(h, view), 0.0, 1.0), F0);
    //vec3 F = FresnelSchlick(clamp(dot(h, to_light_dir), 0.0, 1.0), F0);

    vec3 numerator = NDF*G*F;
    float demoninator = 4.0 * max(dot(normal, view), 0.0) * max(dot(normal, to_light_dir), 0.0) + 0.0001; //加一个小数避免处于0的情况出现
    vec3 specualr = numerator / demoninator;
    // KS就是菲涅尔的值
    vec3 KS = F;
    // 根据能量守恒定律，KS+KD不大于1
    vec3 KD = vec3(1.0) - KS;
    // 非金属材质才有漫反射量，这里乘以一下(1-metallic)
    KD *= 1.0 - metallic;
    // 取在观测方向上的一个分量
    float NdotL = max(dot(normal, to_light_dir), 0.0);

    // return (KD*obj_albedo / PI)*radiance*NdotL;
    return (KD*obj_albedo / PI + specualr)*radiance*NdotL;
}

// calculate color causes by directional light
vec3 CalcDirLight(DirectionalLight dir_light, vec3 normal, vec3 view)
{
    vec3 light_color = dir_light.light_color;
    vec3 to_light_dir = -dir_light.light_dir;

    float shadow = ShadowCalculation(frag_pos_lightspace, to_light_dir);
    return CalcLight(light_color, to_light_dir, normal, view)  * (1.0 - shadow);
    //return vec3(shadow);
}

vec3 CalcPointLight(PointLight point_light, vec3 normal, vec3 view, vec3 in_frag_pos)
{
    vec3 light_color = point_light.light_color;
    vec3 to_light_dir = normalize(point_light.light_pos - in_frag_pos);

    vec3 point_light_color = CalcLight(light_color, to_light_dir, normal, view);

    float distance = length(point_light.light_pos - in_frag_pos);
    
    float attenuation = 1.0 / (distance * distance);	//衰减和点光源的参数可控，这里先简单弄个
    return point_light_color * attenuation;
}

void main()
{
    vec3 view = normalize(cam_pos - frag_pos);  //to_view
    vec3 obj_normal = GetNormal();

    vec3 dir_light_color = CalcDirLight(directional_light, obj_normal, view);
    vec3 point_light_color = vec3(0,0,0);
    for(int i = 0; i < point_light_count; ++i)
    {
        point_light_color += CalcPointLight(point_lights[i], obj_normal, view, frag_pos);
    }


    vec3 ambient = vec3(0.03)*GetAlbedo()*ao;
    vec3 color = ambient + (dir_light_color + point_light_color);
    // 伽马校正（Reinhard）
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));
    FragColor = vec4(color, 1.0);
//    FragColor = vec4(vec3(1.0-shadow), 1.0);
}