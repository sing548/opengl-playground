#include "window.h"

static void APIENTRY GLDebugCallback(GLenum source, GLenum type, GLuint id,
									 GLenum severity, GLsizei length,
									 const GLchar* message, const void* param)
{
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;

	const char* src = "Other";
	
	switch (source)
	{
		case GL_DEBUG_SOURCE_API:				src = "API"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:	src = "API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:		src = "API"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:		src = "API"; break;
		case GL_DEBUG_SOURCE_APPLICATION:		src = "API"; break;
	}

	const char* tpye = "Other";

	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR:				tpye = "Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:	tpye = "Error"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:	tpye = "Error"; break;
		case GL_DEBUG_TYPE_PORTABILITY:			tpye = "Error"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:			tpye = "Error"; break;
	}

	const char* sever = "INFO";

	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:		sever = "HIGH"; break;
		case GL_DEBUG_SEVERITY_MEDIUM:		sever = "HIGH"; break;
		case GL_DEBUG_SEVERITY_LOW:			sever = "HIGH"; break;
	}

	std::cerr << "GL - " << sever << "-" << src << "/" << tpye << "; id: " << id << "MSG: " << message << std::endl;
}

void Window::error_callback(int error, const char* description)
{
	std::cout << "Error " << error << " : " << description << std::endl;
}

void Window::mouse_callback(double xpos, double ypos)
{
	if (firstMouse_ != 0)
	{
		lastX_ = xpos;
		lastY_ = ypos;
		firstMouse_--;
	}

	float xoffset = xpos - lastX_;
	float yoffset = lastY_ - ypos;

	lastX_ = xpos;
	lastY_ = ypos;

	camera_->ProcessMouseMovement(xoffset, yoffset);
}

Window::Window(unsigned int width, unsigned int height, InputManager* inputManager, const char* title)
	: Window(width, height, std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f)), inputManager, title)
{
}

Window::Window(unsigned int width, unsigned int height, std::unique_ptr<Camera> camera, InputManager* inputManager, const char* title) 
	: camera_(std::move(camera)), inputManager_(inputManager)
{
	size_.width = width;
	size_.height = height;
	
	float lastX = width / 2.0f;
	float lastY = height / 2.0f;
	bool firstMouse = true;
	
    if (!glfwInit()) {
		throw std::runtime_error("Failed to initialize GLFW");
    }
    
    glfwSetErrorCallback(error_callback);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    
    window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
    
    if (!window_) {
		glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window_);
	
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		throw std::runtime_error("Failed to initialize GLAD");
    }

	GLint flags = 0;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

	if (false && flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(GLDebugCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}

	//glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

Camera& Window::GetCamera()
{
	return *camera_;
}

WindowSize Window::GetSize()
{
	return size_;
}

void Window::SwapBuffers()
{
	glfwSwapBuffers(window_);
	glfwPollEvents();
}

void Window::UpdateCameraPosition(glm::vec3 position)
{
	camera_->Position = position;
}

void Window::UpdateCameraOrientation(glm::vec3 orientation)
{
	camera_->Front = orientation;
}

void Window::UpdateCameraYaw(float yaw)
{
	camera_->Yaw = yaw;
}

void Window::UpdateCameraPitch(float pitch)
{
	camera_->Pitch = pitch;
}

void Window::ResizeWindow(unsigned int width, unsigned int height)
{
	if (width == 0 || height == 0)
		return;

	size_.width = width;
	size_.height = height;
	glViewport(0, 0, width, height);
	
	windowResized_ = true;
}

void Window::HandleInput(float deltaTime)
{
	if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window_, true);
    }

	if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		camera_->ProcessKeyboard(SPRINT, deltaTime);
	else if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera_->ProcessKeyboard(JOG, deltaTime);
	else if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		camera_->ProcessKeyboard(CRAWL, deltaTime);
	
	if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE && glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE)
		camera_->ProcessKeyboard(WALK, deltaTime);

	if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS)
		camera_->ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS)
		camera_->ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS)
		camera_->ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS)
		camera_->ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera_->ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window_, GLFW_KEY_C) == GLFW_PRESS)
		camera_->ProcessKeyboard(DOWN, deltaTime);
}

Window::~Window()
{
    glfwDestroyWindow(window_);
    glfwTerminate();
}

GLFWwindow* Window::Get()
{
    return window_;
}
