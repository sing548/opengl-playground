#include "renderer.h"

#ifndef SHADER_DIR
#define SHADER_DIR "./shaders"
#endif

#ifndef ASSETS_DIR
#define ASSETS_DIR "./assets"
#endif


Renderer::Renderer(unsigned int width, unsigned int height)
	: Renderer(width, height, false, true)
{
    
}

Renderer::Renderer(unsigned int width, unsigned int height, bool showHitboxes, bool showSkyBox)
{
	std::string screenVert = std::string(SHADER_DIR) + "/screen.vert";
    std::string screenFrag = std::string(SHADER_DIR) + "/screen.frag";
    std::string modelVert  = std::string(SHADER_DIR) + "/model.vert";
    std::string modelFrag  = std::string(SHADER_DIR) + "/model.frag";
	std::string hitboxVert = std::string(SHADER_DIR) + "/hitbox.vert";
	std::string hitboxFrag = std::string(SHADER_DIR) + "/hitbox.frag";
	std::string skyboxVert = std::string(SHADER_DIR) + "/skybox.vert";
	std::string skyboxFrag = std::string(SHADER_DIR) + "/skybox.frag";
	
    screenShader_ = std::make_unique<Shader>(screenVert.c_str(), screenFrag.c_str());
    modelShader_  = std::make_unique<Shader>(modelVert.c_str(), modelFrag.c_str());
	hitboxShader_ = std::make_unique<Shader>(hitboxVert.c_str(), modelFrag.c_str());
	skyboxShader_ = std::make_unique<Shader>(skyboxVert.c_str(), skyboxFrag.c_str());

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
	showHitboxes_ = false;

	/// SKYBOX

	float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
	};

    glGenVertexArrays(1, &skyboxVAO_);
    glGenBuffers(1, &skyboxVBO_);
    glBindVertexArray(skyboxVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	std::vector<std::string> faces
	{
	    ASSETS_DIR "/skybox/NASA2/posx.png",
	    ASSETS_DIR "/skybox/NASA2/negx.png",
	    ASSETS_DIR "/skybox/NASA2/posy.png",
	    ASSETS_DIR "/skybox/NASA2/negy.png",
	    ASSETS_DIR "/skybox/NASA2/posz.png",
	    ASSETS_DIR "/skybox/NASA2/negz.png"
	};
	cubemapTexture_ = LoadCubemap(faces);

	this->showHitboxes_ = showHitboxes;
	this->showSkyBox_ = showSkyBox;
}

void Renderer::ToggleHitboxes()
{
	this->showHitboxes_ = !this->showHitboxes_;
}

void Renderer::ToggleSkyBox()
{
	this->showSkyBox_ = !this->showSkyBox_;
}

void Renderer::Draw(const Scene& scene, const Camera& camera, unsigned int width, unsigned int height)
{
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
	glClearColor(backgroundRGBA_.x, backgroundRGBA_.y, backgroundRGBA_.z, backgroundRGBA_.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    modelShader_->Use();
	
	glm::vec3 dirLight(-1.0f, 0.0f, 0.0f);

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 500.0f);
	glm::mat4 view = camera.GetViewMatrix();
	modelShader_->SetVec3("viewPos", camera.Position);
	modelShader_->SetMat4("projection", projection);
	modelShader_->SetMat4("view", view);

	modelShader_->SetVec3("dirLight.direction", dirLight);
	modelShader_->SetVec3("dirLight.ambient", glm::vec3(0.2f, 0.22f, 0.25f));
	modelShader_->SetVec3("dirLight.diffuse", glm::vec3(0.4f, 0.42f, 0.45f));

	auto models = scene.GetModels();

	int i = 0;
	for (auto& it : models)
	{
		if (i > 1) {
			std::string pos = std::to_string(i - 2);

			modelShader_->SetVec3("pointLights[" + pos + "].position", it.model.GetPosition());

			modelShader_->SetVec3("pointLights[" + pos + "].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
			modelShader_->SetVec3("pointLights[" + pos + "].diffuse", glm::vec3(0.2f, 0.0f, 0.0f));
			modelShader_->SetVec3("pointLights[" + pos + "].specular", glm::vec3(1.0f, 0.0f, 0.0f));

			modelShader_->SetFloat("pointLights[" + pos + "].constant", 1.0f);
			modelShader_->SetFloat("pointLights[" + pos + "].linear", 0.09f);
			modelShader_->SetFloat("pointLights[" + pos + "].quadratic", 0.032f);
		}
		i++;
	}
	
	if (i > 256) i = 256;
	modelShader_->SetInt("numPointLights", i - 2);

	for (auto& it : models)
	{
		glm::mat4 modelProjection = glm::mat4(1.0f);
		modelProjection = glm::translate(modelProjection, it.model.GetPosition());
		modelProjection *=  glm::mat4_cast(glm::quat(it.model.GetRotation()));
		modelProjection = glm::scale(modelProjection, it.model.GetScale());

		modelShader_->SetMat4("model", modelProjection);

		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelProjection)));
		modelShader_->SetMat3("normalMatrix", normalMatrix);
		it.model.Draw(*modelShader_);
	}

	hitboxShader_->Use();
	hitboxShader_->SetMat4("projection", projection);
	hitboxShader_->SetMat4("view", view);

	if (this->showHitboxes_)
	for (auto& it : models)
	{
		// Construct model matrix (same as your object transform)
    	glm::mat4 modelMat = glm::mat4(1.0f);
    	modelMat = glm::translate(modelMat, it.model.GetPosition());
    	modelMat = glm::scale(modelMat, glm::vec3(it.model.radius));

    	// No hitboxShader.Use() here — renderer already sets it
    	hitboxShader_->SetMat4("projection", projection);
    	hitboxShader_->SetMat4("view", view);
    	hitboxShader_->SetMat4("model", modelMat);

    	// Draw unit sphere as wireframe
    	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		it.model.DrawHitbox(*hitboxShader_);
    	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	if (this->showSkyBox_) {
		glDepthFunc(GL_LEQUAL);          // allow skybox depth = 1.0 to pass
    	glDepthMask(GL_FALSE);           // disable writing to depth buffer

    	skyboxShader_->Use();
    	glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation
    	skyboxShader_->SetMat4("view", viewNoTranslation);
    	skyboxShader_->SetMat4("projection", projection);

    	glBindVertexArray(skyboxVAO_);
    	glActiveTexture(GL_TEXTURE0);
    	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture_);
    	glDrawArrays(GL_TRIANGLES, 0, 36);
    	glBindVertexArray(0);

    	glDepthMask(GL_TRUE);            // restore depth writing
    	glDepthFunc(GL_LESS);            // restore default depth test
	}	
	
	// ---------- Post-processing / Render to screen ----------
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  // back to default framebuffer
    glDisable(GL_DEPTH_TEST);              // no depth for screen quad
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    screenShader_->Use();
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(quadVAO_);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

unsigned int Renderer::LoadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;

	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

		if (data)
		{
			int format = nrChannels == 4 ? GL_RGBA : GL_RGB;
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else 
		{
			std::cout << "Cubemap text failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
