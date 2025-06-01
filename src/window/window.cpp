#include "window.h"

void Window::error_callback(int error, const char* description)
{
	std::cout << "Error " << error << " : " << description << std::endl;
}

void Window::mouse_callback(double xpos, double ypos)
{
	if (firstMouse_)
	{
		lastX_ = xpos;
		lastY_ = ypos;
		firstMouse_ = false;
	}

	float xoffset = xpos - lastX_;
	float yoffset = lastY_ - ypos;

	lastX_ = xpos;
	lastY_ = ypos;

	camera_->ProcessMouseMovement(xoffset, yoffset);
}

Window::Window(unsigned int width, unsigned int height, const char* title)
{
	height_ = height;
    width_ = width;

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

	screenShader_ = std::make_unique<Shader>("shaders/screen.vert", "shaders/screen.frag");
	modelShader_ = std::make_unique<Shader>("shaders/model.vert", "shaders/model.frag");
	camera_ = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));

	model_ = std::make_unique<Model>("../assets/models/stars/generic_star/star.obj");
    
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    backgroundRGBA_ = glm::vec4(.201f, 0.301f, 0.401f, 1.00f);

    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	glGenVertexArrays(1, &quadVAO_);
	glGenBuffers(1, &quadVBO_);
	glBindVertexArray(quadVAO_);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	screenShader_->use();
	screenShader_->setInt("screenTexture", 0);

	glGenFramebuffers(1, &framebuffer_);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

	glGenTextures(1, &textureColorbuffer_);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer_, 0);

	glGenRenderbuffers(1, &rbo_);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_); // now actually attach it
	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	setCallbacks();
	glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::draw()
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
	glEnable(GL_DEPTH_TEST);

	glClearColor(backgroundRGBA_.x, backgroundRGBA_.y, backgroundRGBA_.z, backgroundRGBA_.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	modelShader_->use();
	
	glm::mat4 projection = glm::perspective(glm::radians(camera_->Zoom), (float)width_ / (float)height_, 0.1f, 500.0f);
	glm::mat4 view = camera_->GetViewMatrix();
	modelShader_->setMat4("projection", projection);
	modelShader_->setMat4("view", view);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	//model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
	modelShader_->setMat4("model", model);
	model_->Draw(*modelShader_);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	screenShader_->use();
	
	//screenShader_->setInt("screenTexture", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(quadVAO_);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer_);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glfwSwapBuffers(window_);
	glfwPollEvents();
}

void Window::processInput(float deltaTime)
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

void Window::setCallbacks()
{
	glfwSetWindowUserPointer(window_, this);

	glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    });

	glfwSetCursorPosCallback(window_, [](GLFWwindow* window, double xPos, double yPos) {
		Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));

		if (self) {
			self->mouse_callback(xPos, yPos);
		}
	});
}

Window::~Window()
{
    glfwDestroyWindow(window_);
    glfwTerminate();
}

GLFWwindow* Window::get()
{
    return window_;
}