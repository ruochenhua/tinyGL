#include "Actor.h"

#include "Component/Mesh/MeshComponent.h"
#include "Component/LightComponent.h"
#include "Component/Mesh/BoxShape.h"
#include "glm/gtc/random.hpp"
#include "glm/gtx/euler_angles.hpp"
using namespace tinyGL;
using namespace glm;

AActor::~AActor()
{
    for(auto& component : components)
    {
        component.reset();
    }
    
    components.clear();
    component_cache.clear();
}

void AActor::AddComponent(std::shared_ptr<CComponent> component)
{
    components.push_back(component);
}

void AActor::Update(double delta)
{
    // actor 更新调用
    // todo：先处理一下点光源和box mesh之间的颜色同步的问题吧，具体其他的还没想好
    auto point_light_comp = GetComponent<CPointLightComponent>();
    auto box_comp = GetComponent<CBoxShape>();
    if(point_light_comp && box_comp)
    {
        box_comp->mesh_list[0].m_RenderInfo.material.albedo = glm::vec4(point_light_comp->light_color, 1.0);
    }
}

glm::mat4 AActor::GetModelMatrix() const
{
    mat4 model = mat4(1.0);
    model = translate(model, location);
    model *= eulerAngleXYZ(rotation.x, rotation.y, rotation.z);
    model = glm::scale(model, scale);

    return model;
}

mat4 AActor::GenInstanceModelMatrix() const
{
	mat4 model = mat4(1.0);

	vec3 ins_location, ins_rotation, ins_scale;
	ins_location = glm::linearRand(instancing_info.location_min, instancing_info.location_max);
	ins_rotation = glm::linearRand(instancing_info.rotation_min, instancing_info.rotation_max);
	ins_scale	 = glm::linearRand(instancing_info.scale_min, instancing_info.scale_max);

	model = translate(model, ins_location);
	model *= eulerAngleXYZ(ins_rotation.x, ins_rotation.y, ins_rotation.z);
	model = glm::scale(model, ins_scale);
	
	return model;
}

void AActor::InitInstancingData()
{
	unsigned count = instancing_info.count;
	if(count == 0)
	{
		return;
	}
		
	instancing_model_mat.resize(count+1);
	
	instancing_model_mat[0] = GetModelMatrix();
	for(unsigned i = 1; i <= count; ++i)
	{
		instancing_model_mat[i] = GenInstanceModelMatrix();
	}
}

void AActor::BindInstancingToMesh(weak_ptr<CMeshComponent> mesh_comp)
{
	auto mesh_ptr = mesh_comp.lock();

	CMesh& mesh = mesh_ptr->mesh_list[0];
	auto& render_info = mesh.m_RenderInfo;
	glGenBuffers(1, &render_info.instance_buffer);
	
	glBindBuffer(GL_ARRAY_BUFFER, render_info.instance_buffer);
	render_info.instance_count = instancing_info.count+1;
	glBufferData(GL_ARRAY_BUFFER, render_info.instance_count * sizeof(glm::mat4), &instancing_model_mat[0], GL_STATIC_DRAW);
	
	glBindVertexArray(render_info.vertex_array_id);
	GLsizei vec4_size = sizeof(glm::vec4);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)0);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(vec4_size));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(vec4_size*2));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(vec4_size*3));
	
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
	
	glBindVertexArray(0);
}

const mat4& AActor::GetInstancingModelMat(unsigned idx) const
{
	if(idx >= instancing_model_mat.size())
	{
		return mat4(1.0);
	}

	return instancing_model_mat[idx];
}

void AActor::BeginPlay()
{
    // 每个component也调用一下
    for(auto& component : components)
    {
        component->BeginPlay();
    }
    
    // 在加载完后需要做一些事情
    // 这里先处理一下instancing的东西，其他的后面再加
    // auto transform_comp = GetComponent<CTransformComponent>();
    // if(transform_comp)
    // {
    //     return;
    // }
    //todo: instancing
    if(instancing_info.count > 0)
    {
        InitInstancingData();
        auto mesh_comp = GetComponent<CMeshComponent>();
        if(mesh_comp)
        {
			BindInstancingToMesh(mesh_comp);
        }
    }
}
