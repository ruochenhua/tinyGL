#include "Engine.h"

using namespace tinyGL;
static Engine g_engine;
void framebuffer_resize_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    g_engine.SetWidthHeight(width, height);
    //glfwSetWindowSize(window, width, height);
    //glfwSetWindowAspectRatio(window, width, height);
}

Engine::Engine()
{
    // 构建窗口
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        assert(0);
    }
    // 初始化opengl
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL
	
    // Open a window and create its OpenGL context
    // (In the accompanying source code, this variable is global for simplicity)
    render_window = glfwCreateWindow(window_width, window_height, "tinyGL", nullptr, nullptr);
    window_aspect_ratio = (float)window_width/window_height;
    if (render_window == nullptr) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible.\n");
        glfwTerminate();
        assert(0);
    }

    glfwMakeContextCurrent(render_window); // Initialize GLEW
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        assert(0);
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(render_window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetFramebufferSizeCallback(render_window, framebuffer_resize_callback);
}


GLFWwindow* Engine::GetRenderWindow()
{
    return g_engine.render_window;
}

string Engine::ReadFile(const string& file_path)
{
    std::string file_content;
    std::ifstream file_stream(file_path, std::ios::in);
    if (file_stream.is_open()) {
        std::stringstream sstr;
        sstr << file_stream.rdbuf();
        file_content = sstr.str();
        file_stream.close();
    }
    else
    {
        printf("Impossible to open %s. Are you in the right directory ? \n", file_path.c_str());
    }

    return file_content;
}

void Engine::SetWidthHeight(int width, int height)
{
    window_width = width;
    window_height = height;
    window_aspect_ratio = (float)width / height;
}

Engine Engine::GetEngine()
{
    return g_engine;
}



