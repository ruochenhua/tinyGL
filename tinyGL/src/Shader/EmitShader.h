#pragma once
#include "Shader.h"

namespace tinyGL
{
    // 简单发光shader
    class EmitShader : public Shader
    {
    public:
        EmitShader() = default;
        void SetupData(CMesh& mesh) override;
        void UpdateRenderData(const CMesh& mesh,
            const glm::mat4& actor_model_mat,
            const SSceneRenderInfo& scene_render_info) override;
    
        virtual void InitDefaultShader() override;
    };
}