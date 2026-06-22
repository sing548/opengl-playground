#include "renderer.h"
#include "stb/stb_image.h"

#include "../helpers/file-helper.h"

Renderer::Renderer(unsigned int width, unsigned int height)
{
	std::string screenVert = (std::filesystem::path(FileHelper::GetShaderDir()) / "screen.vert").string();
    std::string screenFrag = (std::filesystem::path(FileHelper::GetShaderDir()) / "screen.frag").string();
	std::string blurVert   = (std::filesystem::path(FileHelper::GetShaderDir()) / "blur.vert").string();
	std::string blurFrag   = (std::filesystem::path(FileHelper::GetShaderDir()) / "blur.frag").string();
	
	screenShader_ = std::make_unique<Shader>(screenVert.c_str(), screenFrag.c_str());
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
}

void Renderer::Draw(const RenderList& renderList, const FrameGlobals& globals, const std::unordered_map<std::string, bool>& settings)
{
	//ToDo: ECS (Entity Component System)
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glBindFramebuffer(GL_FRAMEBUFFER, frameFBO_);
	glClearBufferfv(GL_COLOR, 0, sceneClear_.data());
	glClearBufferfv(GL_COLOR, 1, black_.data());
	glClear(GL_DEPTH_BUFFER_BIT);

	std::vector<const DrawCommand*> sorted;
	sorted.reserve(renderList.commands.size());

	for (const auto& cmd : renderList.commands) sorted.push_back(&cmd);
	std::stable_sort(sorted.begin(), sorted.end(),
		[](const DrawCommand* a, const DrawCommand* b) {
			return a->renderPass < b->renderPass;
		});

	RenderPass lastPass = static_cast<RenderPass>(UINT16_MAX);
	Material* lastMat = nullptr;

	for (const DrawCommand* cmd : sorted) {
		if (cmd->renderPass != lastPass) {
			ApplyPassState(cmd->renderPass);
			lastPass = cmd->renderPass;
			lastMat = nullptr;
		}

		if (cmd->material != lastMat)
		{ 
			cmd->material->ApplyFrame(globals);
			lastMat = cmd->material;
		}

		cmd->material->ApplyInstance(cmd->transform, cmd->tint);
		cmd->mesh->Draw(cmd->material->GetShader());
	}

	for (auto* rendererable : sceneRenderables_) 
	{
		auto state = rendererable->GetRenderPass();
		if (state == RenderPass::Opaque && settings.at("debug_view"))
			state = RenderPass::Debug;
		ApplyPassState(state);
		rendererable->Render(globals, settings);
	}
	
	// ---------- Post-processing / Render to screen ----------
	
	if (settings.at("bloom"))
	{
		PostProcessing();
	}
	else
	{
		glDisable(GL_DEPTH_TEST);  
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDepthMask(GL_TRUE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		screenShader_->Use();
		screenShader_->SetInt("screenTexture", 0);
		screenShader_->SetInt("bloomTexture", 1);
		screenShader_->SetInt("bloom", settings.at("bloom"));
		screenShader_->SetFloat("exposure", 1.0f);
	
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBuffers_[0]);
	}

	glBindVertexArray(quadVAO_);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void Renderer::ResizeWindow(unsigned int width, unsigned int height)
{
	for (int i = 0; i < 2; i++) 
	{
		glBindTexture(GL_TEXTURE_2D, colorBuffers_[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	}
	
	glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	
	for (int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, pingPongColorbuffers_[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	}
}

void Renderer::ApplyPassState(RenderPass pass)
{
	switch (pass)
	{
		case RenderPass::Opaque:
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        	glEnable(GL_DEPTH_TEST);
        	glDepthMask(GL_TRUE);
			glDepthFunc(GL_LESS);  
			break;
		case RenderPass::Skybox:
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDepthFunc(GL_LEQUAL);
			glDepthMask(GL_FALSE);
			break;
		case RenderPass::Debug:
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        	glEnable(GL_DEPTH_TEST);
        	glDepthMask(GL_TRUE); 
			glDepthFunc(GL_LESS);  
			break;
	}
}

void Renderer::PostProcessing()
{
	glDisable(GL_DEPTH_TEST);  
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDepthMask(GL_TRUE);
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
	screenShader_->SetInt("bloom", true);
	screenShader_->SetFloat("exposure", 1.0f);

    glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorBuffers_[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pingPongColorbuffers_[1]);
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
