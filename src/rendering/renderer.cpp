#include "renderer.h"

Renderer::Renderer(unsigned int width, unsigned int height)
{
    screenShader_ = std::make_unique<Shader>("shaders/screen.vert", "shaders/screen.frag");
	modelShader_ = std::make_unique<Shader>("shaders/model.vert", "shaders/model.frag");

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

	glGenFramebuffers(1, &framebuffer_);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

	glGenTextures(1, &textureColorbuffer_);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer_);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer_, 0);

	glGenRenderbuffers(1, &rbo_);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	glViewport(0, 0, width, height);	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	backgroundRGBA_ = glm::vec4(.201f, 0.301f, 0.401f, 1.00f);
}

void Renderer::Draw(const Scene& scene, const Camera& camera, unsigned int width, unsigned int height)
{
	//screenShader_->setInt("screenTexture", 0);
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
	glClearColor(backgroundRGBA_.x, backgroundRGBA_.y, backgroundRGBA_.z, backgroundRGBA_.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    modelShader_->Use();
	
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 500.0f);
	glm::mat4 view = camera.GetViewMatrix();
	modelShader_->SetMat4("projection", projection);
	modelShader_->SetMat4("view", view);

	auto models = scene.GetModels();
	for (unsigned int i = 0; i < models.size(); i++)
	{
		glm::mat4 modelProjection = glm::mat4(1.0f);
		modelProjection = glm::translate(modelProjection, glm::vec3(0.0f, 0.0f, 0.0f));
		modelProjection = glm::scale(modelProjection, glm::vec3(0.5f, 0.5f, 0.5f));
		modelShader_->SetMat4("model", modelProjection);
		models[i].model.Draw(*modelShader_);
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	screenShader_->Use();

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(quadVAO_);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer_);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
