#pragma once
#include "common.h"

namespace tinyGL
{
class TGAImage;
class CRenderObj
{
public:
	virtual std::vector<float> GetVertices() const;
	virtual std::vector<float> GetTextureCoords() const;
	virtual std::vector<float> GetNormals() const;
	virtual std::vector<unsigned int> GetIndices() const;
	virtual TGAImage* GetTextureImage() const;

	SRenderInfo GetRenderInfo() {return m_RenderInfo;}
	
protected:
	virtual void GenerateRenderInfo() = 0;
	
	std::vector<glm::vec3> m_vVertex;
	std::vector<glm::vec3> m_vNormal;
	std::vector<glm::vec2> m_vTexCoord;

	TGAImage* m_pDiffuseTex = nullptr;

	std::vector<unsigned int> m_vIndex;

	SRenderInfo m_RenderInfo;

	int ImportObj(const std::string& model_path);
	TGAImage* LoadTexture(const std::string& texture_path);
};
}