#include "Shader.h"

#include <regex>

#include "BlendShader.h"
#include "BRDFShader.h"
#include "EmitShader.h"
#include "LightComponent.h"
#include "render.h"
#include "ShadowMapShader.h"

using namespace tinyGL;
using namespace glm;

ShaderManager* g_shader_manager = new ShaderManager;
std::set<string> shader_include_set;

Shader::Shader(const SRenderResourceDesc& render_resource_desc)
{
    Init(render_resource_desc.shader_paths);
}

GLuint Shader::LoadShaders(const map<EShaderType, string>& shader_path_map)
{
    vector<GLuint> shader_id_list;
    for(auto& shader_path_pair : shader_path_map)
    {
    	auto shader_type = shader_path_pair.first;
    	auto shader_path = shader_path_pair.second;

    	// EShaderType对照GL_XXXX_SHADER
    	GLuint shader_id = glCreateShader(shader_type);

    	// Read the Shader code from the file
    	std::string shader_code = Engine::ReadFile(shader_path);

    	GLint result = GL_FALSE;
    	int info_log_length;

    	// Compile Shader
    	printf("Compiling shader : %s\n", shader_path.c_str());
    	char const * shader_string_ptr = shader_code.c_str();
    	glShaderSource(shader_id, 1, &shader_string_ptr, NULL);
    	vector<string> include_headers = FindIncludeFiles(shader_code);
    	for(const auto& header : include_headers)
    	{
    		IncludeShader(header);
    	}    	
    	
    	glCompileShader(shader_id);

    	// Check Shader
    	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
    	glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
    	if (info_log_length > 0) {
    		std::vector<char> error_msg(info_log_length + 1);
    		glGetShaderInfoLog(shader_id, info_log_length, NULL, &error_msg[0]);
    		printf("%s\n", &error_msg[0]);
    		assert(0);
    	}
		shader_id_list.push_back(shader_id);
    }

	// Link the program
	printf("Linking program\n");
	GLuint prog_id = glCreateProgram();
    for(auto shader_id : shader_id_list)
    {
    	glAttachShader(prog_id, shader_id);
    }
	glLinkProgram(prog_id);

    GLint result = GL_FALSE;
    int info_log_length;
	// Check the program
	glGetProgramiv(prog_id, GL_LINK_STATUS, &result);
	glGetProgramiv(prog_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0) {
		std::vector<char> prog_error_msg(info_log_length + 1);
		glGetProgramInfoLog(prog_id, info_log_length, NULL, &prog_error_msg[0]);
		printf("%s\n", &prog_error_msg[0]);
		assert(0);
	}

    for(auto shader_id : shader_id_list)
    {
    	glDetachShader(prog_id, shader_id);
		glDeleteShader(shader_id);
    }

	return prog_id;
}

void Shader::IncludeShader(const string& include_path)
{
	// already include
	if(shader_include_set.find(include_path) != shader_include_set.end())
	{
		return;
	}
	
	// include 文件名需要以“/”开头，要不然会报错，为什么？？
	string full_path = CSceneLoader::ToResourcePath("/shader"+include_path);
	string include_content_str = Engine::ReadFile(full_path);
	
	glNamedStringARB(GL_SHADER_INCLUDE_ARB,
	include_path.size(),
	include_path.c_str(),
	include_content_str.size(),
	include_content_str.c_str());
}

std::vector<std::string> Shader::FindIncludeFiles(const string& code_content)
{
	std::regex includeRegex("#include \"(.+)\"");  // Regex for #include statements
	std::vector<std::string> includes;  // Vector to store extracted includes

	// Iterate through lines in the code
	std::istringstream iss(code_content);
	std::string line;
	while (std::getline(iss, line)) {
		std::smatch match;
		// For each line, try to match the #include regex
		if (std::regex_search(line, match, includeRegex)) {
			includes.push_back(match[1]);  // Add matched content to includes
		}
	}

	return includes;
}


void Shader::Init(const map<EShaderType, string>& shader_path_cache)
{
    shader_id = Shader::LoadShaders(shader_path_cache);
}

void Shader::Use() const
{
    assert(shader_id, "Shader not loaded yet!");
    glUseProgram(shader_id);
}

void Shader::UpdateRenderData(const CMesh& mesh, const SSceneRenderInfo& scene_render_info)
{
	auto& render_info = mesh.m_RenderInfo;
    glBindVertexArray(render_info.vertex_array_id);	// 绑定VAO

	// 材质属性
	SetVec4("albedo", render_info.material.albedo);
	SetFloat("specular_factor", render_info.material.specular_factor);
	SetFloat("metallic", render_info.material.metallic);
	SetFloat("roughness", render_info.material.roughness);
	SetFloat("ao", render_info.material.ao);

	/*
	法线矩阵被定义为「模型矩阵左上角3x3部分的逆矩阵的转置矩阵」
	Normal = mat3(transpose(inverse(model))) * aNormal;
	 */
	GLuint null_tex_id = CRender::GetNullTexId();
	glActiveTexture(GL_TEXTURE0);
	GLuint diffuse_tex_id = render_info.diffuse_tex_id != 0 ? render_info.diffuse_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, diffuse_tex_id);

	glActiveTexture(GL_TEXTURE1);
	GLuint specular_map_id = render_info.specular_tex_id != 0 ? render_info.specular_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, specular_map_id);

	glActiveTexture(GL_TEXTURE2);
	GLuint normal_map_id = render_info.normal_tex_id != 0 ? render_info.normal_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, normal_map_id);
}

shared_ptr<Shader> ShaderManager::GetShader(const string& shader_name)
{
	return g_shader_manager->GetShaderFromTypeName(shader_name);
}

shared_ptr<Shader> ShaderManager::GetShaderFromTypeName(const string& shader_name)
{
	auto find_iter = shader_cache.find(shader_name);
	if(find_iter != shader_cache.end())
	{
		return find_iter->second;
	}

	// create shader
	if(shader_name == "brdf")
	{
		auto shader_data = make_shared<BRDFShader>();
		shader_data->InitDefaultShader();
		shader_cache.emplace(shader_name, shader_data);
		return shader_data;
	}
	else if(shader_name == "emit")
	{
		auto shader_data = make_shared<EmitShader>();
		shader_data->InitDefaultShader();
		shader_cache.emplace(shader_name, shader_data);
		return shader_data;
	}
	else if(shader_name == "blend")
	{
		auto shader_data = make_shared<BlendShader>();
		shader_data->InitDefaultShader();
		shader_cache.emplace(shader_name, shader_data);
		return shader_data;
	}
	else if(shader_name == "point_light_shadowmap")
	{
		auto shader_data = make_shared<PointLightShadowMapShader>();
		shader_data->InitDefaultShader();
		shader_cache.emplace(shader_name, shader_data);
		return shader_data;
	}
	else if(shader_name == "directional_light_shadowmap")
	{
		auto shader_data = make_shared<DirectionalLightShadowMapShader>();
		shader_data->InitDefaultShader();
		shader_cache.emplace(shader_name, shader_data);
		return shader_data;
	}
	else
	{
		assert(0, "shader type not supported");
	}

	
	return nullptr;
}
