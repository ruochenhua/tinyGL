#pragma once
#include "Shader.h"

namespace tinyGL
{
    class BRDFShader : public Shader
    {
    public:
        BRDFShader() = default;
        
        void SetupData(CMesh& mesh) override;
        void UpdateRenderData(const CMesh& mesh, const SSceneRenderInfo& scene_render_info) override;
    
        virtual void InitDefaultShader() override;
    };

    // brdf_normalmap 就是brdf的变种，很多能力可以和BRDFShader共用
    class BRDFShader_NormalMap : public BRDFShader
    {
    public:
        BRDFShader_NormalMap() = default;

        void SetupData(CMesh& mesh) override;
        void UpdateRenderData(const CMesh& mesh, const SSceneRenderInfo& scene_render_info) override;
        
        virtual void InitDefaultShader() override;
    };

    // brdf_shadowmap 是基于brdf_normalmap的演化
    class BRDFShader_ShadowMap : public BRDFShader_NormalMap
    {
    public:
        BRDFShader_ShadowMap() = default;

        void SetupData(CMesh& mesh) override;
        void UpdateRenderData(const CMesh& mesh, const SSceneRenderInfo& scene_render_info) override;
        
        virtual void InitDefaultShader() override;
    };
}