#pragma once
#include "Camera.h"
#include "common.h"
#include "skybox.h"

namespace tinyGL
{
	class Light;
	class CRenderObj;
	class CModel;
	class CCamera;

	class CRender
	{
	public:
		static CRender* GetRender();
		
		GLFWwindow* render_window;
		CRender(){ }

		int Init();
		int Update(double delta);
		void PostUpdate();

		void RenderSceneObject();
		// todo:整合一下吧这两个
		void RenderShadowMap();
		
		// load image file and create texture 
		static GLuint LoadTexture(const std::string& texture_path);
		
	private:
		int InitRender();
		int InitCamera();
		void RenderSkyBox();
		void RenderScene() const;
		
		//void UpdateLightDir(float delta);
		
	private:
		CSkyBox m_SkyBox;
		//shadow map
		GLuint m_ShadowMapFBO		= GL_NONE;
		GLuint m_ShadowMapProgramID = GL_NONE;	
		GLuint m_DepthTexture		= GL_NONE;			
		GLuint m_DepthMatrixID		= GL_NONE;
		GLuint null_tex_id			= GL_NONE;
		int shadowmap_width = 2048;
		int shadowmap_height = 2048;
		// debug
		GLuint m_ShadowMapDebugShaderId = GL_NONE;
		GLuint m_QuadVAO = GL_NONE;
		GLuint m_QuadVBO = GL_NONE;
		
		glm::mat4 m_DepthMVP;
		glm::mat4 light_space_mat;

		CCamera* mainCamera = nullptr;

		double light_yaw = 0.0;
		double light_pitch = 45.0;
	};
}
