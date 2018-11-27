#include "RiverEditor.h"
#include "Utils.h"

#include <vector>
#include <iostream>

void RiverEditor::DefaultParameters()
{
	// River control
	controlPointsCount = 4;
	smoothness = 0.5f;
	animationSpeed = 0.1f;
	tilingFactor = 5.0f; 
	maxAnimationSpeed = 4.0f;

	// VFX
	particleFallSpeed = glm::vec3(0.0f, -0.9f, 0.0f);
	
	// Camera
	aspectRatio = glm::vec2(16.0f, 9.0f);
	viewDistance = 100.0f;

	// Mouse picking
	clickDistanceThreshold = 0.4f;
	selection = -1;

	// Curve generation
	instanceCount = 1;
	generatedPoints = 30;
	riverWidth = 0.75f;

	// Post processing
	postProcessOn = true;
	waveEffectFrequency = 16.0f;
}

void RiverEditor::Init()
{
	DefaultParameters();

	// Default rendering mode will use depth buffer
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	// Init frame buffer
	glm::vec2 resolution = window->GetResolution();
	frameBuffer = std::unique_ptr<FrameBuffer>(new FrameBuffer());
	frameBuffer->Generate(resolution.x, resolution.y, 2);

	// Camera setup --------------------------------------------------------------
	camera = std::unique_ptr<EngineComponents::Camera>(new EngineComponents::Camera());

	// Set an orthogonal camera on OZ => a 2D scene
	camera->SetOrthographic(aspectRatio.x, aspectRatio.y, 0.01f, viewDistance);
	camera->SetPositionAndRotation(glm::vec3(0, 0.0f, 5.0f), glm::quat(glm::vec3(0)));
	camera->Update();

	// Control points ------------------------------------------------------------
	controlPoints.push_back(glm::vec3(-aspectRatio.x / 2.0f + 1.0f, 0.0f, 0.0f));
	controlPoints.push_back(glm::vec3(0.0f, aspectRatio.y / 2.0f - 1.0f, 0.0f));
	controlPoints.push_back(glm::vec3(0.0f, -aspectRatio.y / 2.0f + 1.0f, 0.0f));
	controlPoints.push_back(glm::vec3(aspectRatio.x / 2.0f - 1.0f, 0.0f, 0.0f));

	// Quad mesh -----------------------------------------------------------------
	{
		std::vector<VertexFormat> vertices =
		{
			VertexFormat(glm::vec3(-0.5, 0.5, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(0.0f, 1.0f)),
			VertexFormat(glm::vec3(0.5, 0.5, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 1.0f)),
			VertexFormat(glm::vec3(0.5, -0.5, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(1.0f, 0.0f)),
			VertexFormat(glm::vec3(-0.5, -0.5, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec2(0.0f, 0.0f))
		};

		std::vector<unsigned short> indices =
		{
			0, 1, 2,
			0, 2, 3
		};

		meshes["quad"] = std::shared_ptr<Mesh>(new Mesh("quad"));
		meshes["quad"]->InitFromData(vertices, indices);
	}

	// River mesh ----------------------------------------------------------------
	{
		std::vector<VertexFormat> vertices =
		{
			VertexFormat(controlPoints[0], glm::vec3(0, 1, 0)),
			VertexFormat(controlPoints[controlPointsCount - 1], glm::vec3(0, 1, 0))
		};

		std::vector<unsigned short> indices =
		{
			0, 1
		};

		meshes["river"] = std::shared_ptr<Mesh>(new Mesh("generated initial surface points"));
		meshes["river"]->InitFromData(vertices, indices);
		meshes["river"]->SetDrawMode(GL_LINES);
	}

	// Default Shader ------------------------------------------------------------
	{
		Shader *shader = new Shader("Simple");
		shader->AddShader(RESOURCE_PATH::SHADERS + "RiverEditor/Simple.VS.glsl", GL_VERTEX_SHADER);
		shader->AddShader(RESOURCE_PATH::SHADERS + "RiverEditor/Simple.FS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = std::shared_ptr<Shader>(shader);
	}

	// Curve Shader --------------------------------------------------------------
	{
		Shader *shader = new Shader("BezierCurve");
		shader->AddShader(RESOURCE_PATH::SHADERS + "RiverEditor/Pass.VS.glsl", GL_VERTEX_SHADER);
		shader->AddShader(RESOURCE_PATH::SHADERS + "RiverEditor/Bezier.GS.glsl", GL_GEOMETRY_SHADER);
		shader->AddShader(RESOURCE_PATH::SHADERS + "RiverEditor/Simple.FS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = std::shared_ptr<Shader>(shader);
	}	

	// Particle Shader -----------------------------------------------------------
	{
		Shader *shader = new Shader("Particle");
		shader->AddShader(RESOURCE_PATH::SHADERS + "RiverEditor/Particle.VS.glsl", GL_VERTEX_SHADER);
		shader->AddShader(RESOURCE_PATH::SHADERS + "RiverEditor/Particle.GS.glsl", GL_GEOMETRY_SHADER);
		shader->AddShader(RESOURCE_PATH::SHADERS + "RiverEditor/Simple.FS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = std::shared_ptr<Shader>(shader);
	}

	// Bloom Shader --------------------------------------------------------------
	{
		Shader *shader = new Shader("Bloom");
		shader->AddShader(RESOURCE_PATH::SHADERS + "RiverEditor/Simple.VS.glsl", GL_VERTEX_SHADER);
		shader->AddShader(RESOURCE_PATH::SHADERS + "RiverEditor/Bloom.FS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = std::shared_ptr<Shader>(shader);
		postProcessFX.push_back(shader->GetName());
	}

	// Blur Shader ---------------------------------------------------------------
	{
		Shader *shader = new Shader("Blur");
		shader->AddShader(RESOURCE_PATH::SHADERS + "RiverEditor/Simple.VS.glsl", GL_VERTEX_SHADER);
		shader->AddShader(RESOURCE_PATH::SHADERS + "RiverEditor/Blur.FS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = std::shared_ptr<Shader>(shader);
		postProcessFX.push_back(shader->GetName());
	}

	// Wave Shader ---------------------------------------------------------------
	{
		Shader *shader = new Shader("Wave");
		shader->AddShader(RESOURCE_PATH::SHADERS + "RiverEditor/Simple.VS.glsl", GL_VERTEX_SHADER);
		shader->AddShader(RESOURCE_PATH::SHADERS + "RiverEditor/Wave.FS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = std::shared_ptr<Shader>(shader);
		postProcessFX.push_back(shader->GetName());
	}

	// Water Texture -------------------------------------------------------------
	TextureManager::LoadTexture(RESOURCE_PATH::TEXTURES + "RiverEditor", "water.png", "water");

	// Particle Texture ----------------------------------------------------------
	TextureManager::LoadTexture(RESOURCE_PATH::TEXTURES, "particle.png", "water_splash");

	// Control Point Texture -----------------------------------------------------
	TextureManager::LoadTexture(RESOURCE_PATH::TEXTURES + "RiverEditor", "test.png", "button");

	// Background Texture ----------------------------------------------------------
	TextureManager::LoadTexture(RESOURCE_PATH::TEXTURES + "RiverEditor", "RockCliff.png", "background");

	// Particle Effect -----------------------------------------------------------
	splashEffect = std::unique_ptr< ParticleEffect<Particle> >(new ParticleEffect<Particle>());
	splashEffect->particleTexture = TextureManager::GetTexture("water_splash");
	splashEffect->fallSpeed = particleFallSpeed;
	splashEffect->decayRadius = riverWidth / 2;
	splashEffect->particleSize = riverWidth / 10;
	UpdateVFX();

	// PostProcessing stuff ------------------------------------------------------
	currentEffect = 0;
}

void RiverEditor::FrameStart()
{
	if (postProcessOn)
	{
		frameBuffer->Bind();
	}
	ClearScreen();
}

void RiverEditor::Update(float deltaTimeSeconds)
{
	glm::vec3 planeOffset = glm::vec3(0.0f, 0.0f, 0.1f);

	// Render control points gizmos
	glm::vec3 controlPointScale = glm::vec3(clickDistanceThreshold * 2.0 / sqrt(2.0f));
	Texture2D *texture = TextureManager::GetTexture("button");
	for (auto &point : controlPoints)
	{
		RenderMesh(meshes["quad"], shaders["Simple"], texture, point + planeOffset, controlPointScale);
	}

	// Render background
	texture = TextureManager::GetTexture("background");
	RenderMesh(meshes["quad"], shaders["Simple"], texture, -planeOffset, glm::vec3(aspectRatio.x, aspectRatio.y, 0.0f));

	// Render river curve
	RenderRiver(TextureManager::GetTexture("water"));

	// Render river vfx depending on the speed
	if (animationSpeed > 0.0f)
	{
		// Small offset so we keep vfx bounded in the river
		float offset = 0.025f;

		for (float t = offset; t <= 1; t += 1.0 / animationSpeed)
			RenderVFX(splashEffect, shaders["Particle"], GetBezierPoint(t), deltaTimeSeconds);
	}
}

void RiverEditor::FrameEnd()
{
	if (postProcessOn)
	{
		FrameBuffer::BindDefault();
		ClearScreen();

		ApplyPostProcessing(shaders[postProcessFX[currentEffect]]);
	}
}

void RiverEditor::RenderMesh(std::shared_ptr<Mesh> &mesh, std::shared_ptr<Shader> &shader, Texture2D *texture,
									const glm::vec3 &position, const glm::vec3 &scale)
{
	if (!mesh || !shader || !shader->program)
		return;

	shader->Use();

	// Build Model
	glm::mat4 model(1);
	model = glm::translate(model, position);
	model = glm::scale(model, scale);

	// Send model to shader
	glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(model));

	// Send View & Projection to shader
	glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
	glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));

	// Apply textures if needed
	if (texture)
	{
		mesh->UseMaterials(false);
		texture->BindToTextureUnit(GL_TEXTURE0);
		glUniform1i(shader->loc_textures[0], 0);
	}
	else
	{
		mesh->UseMaterials(true);
	}

	// Render mesh with textures
	mesh->Render();

	if (texture)
		texture->UnBind();
}

void RiverEditor::RenderRiver(Texture2D *texture)
{
	auto mesh = meshes["river"];
	auto shader = shaders["BezierCurve"];
	if (!mesh || !shader || !shader->GetProgramID() || !texture)
		return;

	shader->Use();

	// Send model to shader
	glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));

	// Send View & Projection to shader
	glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
	glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));

	// Send control points
	for (int i = 0; i < controlPointsCount; i++)
	{
		std::string name = "control_points[" + std::to_string(i) + "]";
		int loc = glGetUniformLocation(shader->program, name.c_str());
		glUniform3fv(loc, 1, glm::value_ptr(controlPoints[i]));
	}

	// Send other parameters
	int loc = glGetUniformLocation(shader->program, "generated_points_count");
	glUniform1i(loc, generatedPoints);
	loc = glGetUniformLocation(shader->program, "surface_width");
	glUniform1f(loc, riverWidth);
	loc = glGetUniformLocation(shader->program, "no_of_instances");
	glUniform1i(loc, instanceCount);

	// River flow
	loc = glGetUniformLocation(shader->program, "time");
	glUniform1f(loc, Engine::GetElapsedTime());
	loc = glGetUniformLocation(shader->program, "speed");
	glUniform1f(loc, animationSpeed);
	loc = glGetUniformLocation(shader->program, "tilingFactor");
	glUniform1f(loc, tilingFactor);

	// Texture
	texture->BindToTextureUnit(GL_TEXTURE0);
	glUniform1i(shader->loc_textures[0], 0);

	// Draw the object instanced
	glBindVertexArray(mesh->GetBuffers()->VAO);
	glDrawElementsInstanced(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, (void*)0, instanceCount);
	glBindVertexArray(0);

	texture->UnBind();
}

void RiverEditor::RenderVFX(std::unique_ptr< ParticleEffect<Particle> > &effect, std::shared_ptr<Shader> &shader,
								const glm::vec3 &position, float deltaTime)
{
	if (!effect || !shader || !shader->program)
		return;

	// Magic shit
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);

	// Modify the effect's transform and then Render it 
	effect->source->SetWorldPosition(position);
	effect->Render(camera.get(), shader.get(), deltaTime);

	// Stop da magic
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void RiverEditor::ApplyPostProcessing(std::shared_ptr<Shader> &shader)
{
	if (!shader || !shader->program)
		return;

	shader->Use();

	// Send screen resolution to shader
	int loc = glGetUniformLocation(shader->program, "screen_size");
	glUniform2iv(loc, 1, glm::value_ptr(window->GetResolution()));

	// Send time to shader
	loc = glGetUniformLocation(shader->program, "time");
	glUniform1f(loc, Engine::GetElapsedTime());

	// Other params
	loc = glGetUniformLocation(shader->program, "frequency");
	glUniform1f(loc, waveEffectFrequency);

	// Send the secondary textures to GPU
	Texture2D *texture = frameBuffer->GetTexture(1);
	texture->BindToTextureUnit(GL_TEXTURE1);
	glUniform1i(shader->loc_textures[1], 1);

	// Render the quad
	texture = frameBuffer->GetTexture(0);
	RenderMesh(meshes["quad"], shader, texture, glm::vec3(0), glm::vec3(aspectRatio.x, aspectRatio.y, 0.0f));

	// Unbind secondary textures
	frameBuffer->GetTexture(1)->UnBind();
}

void RiverEditor::UpdateVFX()
{
	unsigned int nrParticles = 100 * riverWidth;
	splashEffect->Generate(nrParticles, true);
	splashEffect->particleSize = riverWidth / 10;
	splashEffect->decayRadius = riverWidth / 2;

	auto particleSSBO = splashEffect->GetParticleBuffer();
	Particle* data = const_cast<Particle*>(particleSSBO->GetBuffer());

	// Reset particle values
	for (unsigned int i = 0; i < nrParticles; i++)
	{
		glm::vec4 pos(1);
		glm::vec2 randPos = Utils::RandInsideUnitCircle();
		pos.x = riverWidth / 2 * randPos.x;
		pos.y = riverWidth / 2 * randPos.y;
		pos.z = 0.0f;

		glm::vec4 speed(0);
		speed.x = Utils::RandFloat(-riverWidth / 2, riverWidth / 2);
		speed.y = Utils::RandFloat(0.0f, animationSpeed);
		speed.z = 0.0f;

		data[i].SetInitial(pos, speed);
	}
	particleSSBO->SetBufferData(data);
}

void RiverEditor::ClearScreen()
{
	// Clears the color buffer (using the previously set color) and depth buffer
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Sets the screen area where to draw
	// Changes with the window resolution
	glm::ivec2 resolution = window->GetResolution();
	glViewport(0, 0, resolution.x, resolution.y);
}

glm::vec3 RiverEditor::ScreenToWorldSpace(int x, int y)
{
	// Get current viewport
	glm::ivec2 resolution = window->GetResolution();
	glm::vec4 viewport = glm::vec4(0.0f, 0.0f, resolution.x, resolution.y);

	// Get mouse position
	glm::vec3 nearPlaneCoords = glm::vec3(x, resolution.y - y, 0.0f);
	glm::vec3 farPlaneCoords = glm::vec3(x, resolution.y - y, 1.0f);

	// Convert back to world space
	glm::vec3 nearPlaneMousePos = glm::unProject(nearPlaneCoords, camera->GetViewMatrix(), 
									camera->GetProjectionMatrix(), viewport);
	glm::vec3 farPlaneMousePos = glm::unProject(farPlaneCoords, camera->GetViewMatrix(), 
									camera->GetProjectionMatrix(), viewport);

	// Make ray
	glm::vec3 pickRay = farPlaneMousePos - nearPlaneMousePos;

	// Intersect ray with z = 0 plane
	float t = -nearPlaneMousePos.z / pickRay.z;
	return nearPlaneMousePos + t * pickRay;
}

glm::vec3 RiverEditor::GetBezierPoint(float t)
{
	glm::vec3 result = glm::vec3(0);
	float c = 1;
	for (int i = 0; i < controlPointsCount; i++)
	{
		if (i > 0)
			c *= (controlPointsCount - i) / float(i);
		result += c * pow((1 - t), controlPointsCount - 1 - i) * pow(t, i) * controlPoints[i];
	}
	return result;
}

void RiverEditor::OnInputUpdate(float deltaTime, int mods)
{
	// THICCness
	if (window->KeyHold(GLFW_KEY_T))
	{
		riverWidth += smoothness * deltaTime;
		UpdateVFX();
	}
	if (window->KeyHold(GLFW_KEY_R))
	{
		riverWidth -= smoothness * deltaTime;
		UpdateVFX();
	}

	// Tiling Factor
	if (window->KeyHold(GLFW_KEY_P))
	{
		tilingFactor += smoothness * deltaTime;
	}
	if (window->KeyHold(GLFW_KEY_O))
	{
		tilingFactor -= smoothness * deltaTime;
	}
}

void RiverEditor::OnKeyPress(int key, int mods)
{
	// Animation Speed
	if (key == GLFW_KEY_KP_ADD)
	{
		animationSpeed += smoothness;
		animationSpeed = animationSpeed > maxAnimationSpeed ? maxAnimationSpeed : animationSpeed;
		UpdateVFX();
	}
	if (key == GLFW_KEY_KP_SUBTRACT)
	{
		animationSpeed -= smoothness;
		animationSpeed = animationSpeed < 0.0f ? 0.0f : animationSpeed;
		UpdateVFX();
	}

	// Post Processing
	if (key == GLFW_KEY_SPACE)
	{
		if (postProcessOn)
		{
			if (postProcessFX.size() - 1 == currentEffect)
			{
				postProcessOn = !postProcessOn;
				currentEffect = 0;
			}
			else
			{
				currentEffect++;
			}
		}
		else
		{
			postProcessOn = !postProcessOn;
		}
	}
}

void RiverEditor::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// If there's anything selected
	if (selection != -1)
	{		
		// Move the selected point at the new position
		controlPoints[selection] = ScreenToWorldSpace(mouseX, mouseY);
	}
}

void RiverEditor::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT))
	{
		glm::vec3 mousePos = ScreenToWorldSpace(mouseX, mouseY);

		// Select the nearest control point
		for (int i = 0; i < controlPointsCount; i++)
		{
			if (glm::length(mousePos - controlPoints[i]) < clickDistanceThreshold)
			{
				selection = i;
			}
		}
	}
}

void RiverEditor::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// Reset selection
	if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT))
	{
		selection = -1;
	}
}
