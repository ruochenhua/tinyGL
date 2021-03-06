#include "model.h"
#include "OBJ_Loader.h"
#include "tgaimage.h"

using namespace glm;
using namespace tinyGL;
int CModel::ImportObj(const std::string& model_path, const std::string& diffuse_tex_coord)
{
	objl::Loader obj_loader;

	bool loadout = obj_loader.LoadFile(model_path);
	if (!loadout)
		return -1;

	auto load_meshes = obj_loader.LoadedMeshes;
	auto load_vertices = obj_loader.LoadedVertices;
	auto load_materials = obj_loader.LoadedMaterials;

	m_vIndex = obj_loader.LoadedIndices;

	size_t vertex_count = load_vertices.size();

	m_vVertex.clear();
	m_vVertex.resize(vertex_count);

	m_vTexCoord.clear();
	m_vTexCoord.resize(vertex_count);

	m_vNormal.clear();
	m_vNormal.resize(vertex_count);

	for (size_t i = 0; i < m_vVertex.size(); ++i)
	{
		auto &pos = load_vertices[i].Position;
		m_vVertex[i] = vec3(pos.X, pos.Y, pos.Z);

		auto tex_coord = load_vertices[i].TextureCoordinate;
		m_vTexCoord[i] = vec2(tex_coord.X, tex_coord.Y);

		auto normal = load_vertices[i].Normal;
		m_vNormal[i] = vec3(normal.X, normal.Y, normal.Z);
	}

	if (!diffuse_tex_coord.empty())
	{
		if (m_pDiffuseTex)
		{
			delete m_pDiffuseTex;
			m_pDiffuseTex = nullptr;
		}

		m_pDiffuseTex = new TGAImage;
		m_pDiffuseTex->read_tga_file(diffuse_tex_coord.c_str());
	}

	return 0;
}
