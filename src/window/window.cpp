#include "window.h"

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
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
    
    if (!window_) {
		glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window_);
	
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		throw std::runtime_error("Failed to initialize GLAD");
    }

	glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

Camera Window::GetCamera()
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

void Window::setCallbacks()
{
	glfwSetWindowUserPointer(window_, this);

	glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);

		Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
		if (self) {
			self->ResizeWindow(width, height);
		}
    });

	glfwSetKeyCallback(window_, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
            auto window = static_cast<Window*>(glfwGetWindowUserPointer(w));
            window->inputManager_->UpdateKey(key, action);
    });
}

void Window::setMouseLooking()
{
	glfwSetCursorPosCallback(window_, [](GLFWwindow* window, double xPos, double yPos) {
		Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));

		if (self) {
			self->mouse_callback(xPos, yPos);
		}
	});
}

void Window::ResizeWindow(unsigned int width, unsigned int height)
{
	size_.width = width;
	size_.height = height;
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
