#include "renderer.h"

Renderer::Renderer(unsigned int width, unsigned int height)
	: Renderer(width, height, false, true)
{
    
}

Renderer::Renderer(unsigned int width, unsigned int height, bool showHitboxes, bool showSkyBox)
{
	std::string screenVert = (std::filesystem::path(FileHelper::GetShaderDir()) / "screen.vert").string();
    std::string screenFrag = (std::filesystem::path(FileHelper::GetShaderDir()) / "screen.frag").string();
    std::string modelVert  = (std::filesystem::path(FileHelper::GetShaderDir()) / "model.vert").string();
    std::string modelFrag  = (std::filesystem::path(FileHelper::GetShaderDir()) / "model.frag").string();
	std::string hitboxVert = (std::filesystem::path(FileHelper::GetShaderDir()) / "hitbox.vert").string();
	std::string hitboxFrag = (std::filesystem::path(FileHelper::GetShaderDir()) / "hitbox.frag").string();
	std::string skyboxVert = (std::filesystem::path(FileHelper::GetShaderDir()) / "skybox.vert").string();
	std::string skyboxFrag = (std::filesystem::path(FileHelper::GetShaderDir()) / "skybox.frag").string();
	std::string blurVert   = (std::filesystem::path(FileHelper::GetShaderDir()) / "blur.vert").string();
	std::string blurFrag   = (std::filesystem::path(FileHelper::GetShaderDir()) / "blur.frag").string();
	
    screenShader_ = std::make_unique<Shader>(screenVert.c_str(), screenFrag.c_str());
    modelShader_  = std::make_unique<Shader>(modelVert.c_str(), modelFrag.c_str());
	hitboxShader_ = std::make_unique<Shader>(hitboxVert.c_str(), modelFrag.c_str());
	skyboxShader_ = std::make_unique<Shader>(skyboxVert.c_str(), skyboxFrag.c_str());
	blurShader_	  = std::make_unique<Shader>(blurVert.c_str(), blurFrag.c_str());

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

	glGenFramebuffers(1, &frameFBO_);
	glBindFramebuffer(GL_FRAMEBUFFER, frameFBO_);

	// multiple buffers
	glGenTextures(2, colorBuffers_);

	for (unsigned int i = 0; i < 2; i++)
	{
	    glBindTexture(GL_TEXTURE_2D, colorBuffers_[i]);
	    glTexImage2D(
	        GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL
	    );
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	    // attach texture to framebuffer
	    glFramebufferTexture2D(
	        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers_[i], 0
	    );
	}

	glGenRenderbuffers(1, &rbo_);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_);

	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	glViewport(0, 0, width, height);	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenFramebuffers(2, pingpongFBO_);
	glGenTextures(2, pingPongColorbuffers_);

	for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO_[i]);
        glBindTexture(GL_TEXTURE_2D, pingPongColorbuffers_[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingPongColorbuffers_[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

	backgroundRGBA_ = glm::vec4(.201f, 0.301f, 0.401f, 1.00f);
	sceneClear_ = {
    	backgroundRGBA_.x,
    	backgroundRGBA_.y,
    	backgroundRGBA_.z,
    	
		backgroundRGBA_.w
	};
	black_ = { 0.0f, 0.0f, 0.0f, 1.0f };


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

	std::string base = (std::filesystem::path(FileHelper::GetAssetsDir()) / "skybox" / "NASA2").string();

	std::vector<std::string> faces
	{
	    (std::filesystem::path(base) / "posx.png").string(),
	    (std::filesystem::path(base) / "negx.png").string(),
	    (std::filesystem::path(base) / "posy.png").string(),
	    (std::filesystem::path(base) / "negy.png").string(),
	    (std::filesystem::path(base) / "posz.png").string(),
	    (std::filesystem::path(base) / "negz.png").string()
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

void Renderer::Draw(const Scene& scene, const Camera& camera, unsigned int width, unsigned int height, const std::map<std::string, bool>& settings)
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glBindFramebuffer(GL_FRAMEBUFFER, frameFBO_);
	glClearBufferfv(GL_COLOR, 0, sceneClear_.data());
	glClearBufferfv(GL_COLOR, 1, black_.data());
	glClear(GL_DEPTH_BUFFER_BIT);

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
	for (auto& [id, model] : models)
	{
		if (i > 1) {
			std::string pos = std::to_string(i - 2);

			modelShader_->SetVec3("pointLights[" + pos + "].position", model.GetPosition());

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

	for (auto& [id, model] : models)
	{
		glm::mat4 modelProjection = glm::mat4(1.0f);
		modelProjection = glm::translate(modelProjection, model.GetPosition());
		modelProjection *=  glm::mat4_cast(glm::quat(model.GetRotation()));
		modelProjection = glm::scale(modelProjection, model.GetScale());

		modelShader_->SetMat4("model", modelProjection);

		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelProjection)));
		modelShader_->SetMat3("normalMatrix", normalMatrix);
		model.Draw(*modelShader_);
	}

	hitboxShader_->Use();
	hitboxShader_->SetMat4("projection", projection);
	hitboxShader_->SetMat4("view", view);

	if (this->showHitboxes_)
	for (auto& [id, model] : models)
	{
		// Construct model matrix (same as your object transform)
    	glm::mat4 modelMat = glm::mat4(1.0f);
    	modelMat = glm::translate(modelMat, model.GetPosition());
    	modelMat = glm::scale(modelMat, glm::vec3(model.GetRadius()));

    	// No hitboxShader.Use() here — renderer already sets it
    	hitboxShader_->SetMat4("projection", projection);
    	hitboxShader_->SetMat4("view", view);
    	hitboxShader_->SetMat4("model", modelMat);

    	// Draw unit sphere as wireframe
    	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		model.DrawHitbox(*hitboxShader_);
    	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	if (this->showSkyBox_)
	{
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


	bool horizontal = true, firstIteration = true;
	unsigned int amount = 10;
	blurShader_->Use();
	blurShader_->SetInt("image", 0);

	for (unsigned int i = 0; i < amount; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO_[horizontal]);
		blurShader_->SetInt("horizontal", horizontal);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, firstIteration ?  colorBuffers_[1] : pingPongColorbuffers_[!horizontal]);

		glBindVertexArray(quadVAO_);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		horizontal = !horizontal;

		if (firstIteration) firstIteration = false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    screenShader_->Use();
	screenShader_->SetInt("screenTexture", 0);
	screenShader_->SetInt("bloomTexture", 1);
	screenShader_->SetInt("bloom", settings.at("bloom"));
	screenShader_->SetFloat("exposure", 1.0f);

    glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorBuffers_[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pingPongColorbuffers_[1]);

	glBindVertexArray(quadVAO_);
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
