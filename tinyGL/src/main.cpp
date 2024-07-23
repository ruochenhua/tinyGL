//tiny openGL project
#include "render.h"
#include "message.h"
#include "Engine.h"
#include "Scene.h"

#include "TapBodyManager.h"
#include "utilityshape.h"
using namespace glm;
using namespace std;
using namespace tinyGL;

int main()
{
	// Open a window and create its OpenGL context
	// (In the accompanying source code, this variable is global for simplicity)
	// 构建渲并初始化染类
	CRender* render = new CRender;
	render->Init();
	
	// string model_path = RESOURCE_PATH + "diablo3_pose/diablo3_pose.obj";
	// string diffuse_tex_path = RESOURCE_PATH + "diablo3_pose/diablo3_pose_diffuse.tga";
	// vector<string> shader_path = {
	// 	RESOURCE_PATH + "shader/vertex.shader", RESOURCE_PATH + "shader/fragment.shader"
	// };
	// CModel* test_model = new CModel(model_path, diffuse_tex_path, shader_path);
	//
	//
	// // render->AddModel(test_model, shader_path);
	vector<shared_ptr<CRenderObj>> render_objs;
	vector<shared_ptr<Light>> lights;
	CSceneLoader::LoadScene("scene/hello.json", render_objs, lights);
	render->InitLights(lights);
	
	// auto body_manager = new Tap::CBodyManager();
	auto render_window = Engine::GetRenderWindow();
	double current_time, new_time;
	current_time = new_time = glfwGetTime();
	while (!glfwWindowShouldClose(render_window))
	{
		if(glfwGetKey(render_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(render_window, true);
		}
		render->Update(new_time - current_time);
		for(auto& render_obj : render_objs)
		{
			render->RenderSceneObject(render_obj);			
		}
		render->PostUpdate();
		current_time = new_time;
		new_time = glfwGetTime();	
	}
	return 0;
}
