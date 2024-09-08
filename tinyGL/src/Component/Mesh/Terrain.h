﻿#pragma once
#include "MeshComponent.h"

namespace Kong
{
    // 地形类
    class Terrain : public CMeshComponent
    {
    public:
        Terrain(const string &file_name);
        void Draw();
    private:
        GLuint terrain_vao = GL_NONE;
        GLuint terrain_vbo = GL_NONE;
        GLuint terrain_ebo = GL_NONE;
        
        std::vector<float> height_data;
        std::vector<unsigned int> height_indices;
        float y_scale = 32.0f/256.0f;
        float y_shift = 16.0f;

        unsigned int num_strips = 0;
        unsigned int num_verts_per_strip = 0;
        // 读取高度图
        int ImportTerrain(const string &file_name);
        shared_ptr<Shader> terrain_shader;
    };
}
