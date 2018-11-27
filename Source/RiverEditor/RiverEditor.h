#pragma once

#include <unordered_map>
#include <memory>

#include "Particle.h"

#include <Core/Engine.h>
#include <Component\Camera\Camera.h>
#include <Core\GPU\ParticleEffect.h>

class Mesh;
class Shader;
class Texture2D;

class RiverEditor : public World
{
public:
	RiverEditor() {}
	virtual ~RiverEditor() {}

	virtual void Init() override;

private:
	virtual void FrameStart() override;
	virtual void Update(float deltaTimeSeconds) override;
	virtual void FrameEnd() override;

	// Input controls
	void OnInputUpdate(float deltaTime, int mods);
	void OnKeyPress(int key, int mods) override;
	void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
	void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;

	void ClearScreen();

	// Sets the scene parameters to their default values
	void DefaultParameters();

	// Converts the screen space position to a point on the XoY plane
	glm::vec3 ScreenToWorldSpace(int x, int y);

	// Returns a point on the bezier curve described by the control points
	glm::vec3 GetBezierPoint(float t);

	// Updates the particle effect based on the river parameters
	void UpdateVFX();

	// Basic rendering of objects
	void RenderMesh(std::shared_ptr<Mesh> &mesh, std::shared_ptr<Shader> &shader, const glm::vec3 &position, const glm::vec3 &scale);

	// Specific rendering of the curve
	void RenderRiver(Texture2D *texture);

	// Render water VFX
	void RenderVFX(std::unique_ptr< ParticleEffect<Particle> > &effect, std::shared_ptr<Shader> &shader, const glm::vec3 &position, const glm::vec3 &scale);

private:
	// Resource managers
	std::unordered_map< std::string, std::shared_ptr<Mesh> > meshes;
	std::unordered_map< std::string, std::shared_ptr<Shader> > shaders;
	std::unordered_map< std::string, std::shared_ptr<Texture2D> > textures;

	// Control points
	int controlPointsCount;
	std::vector< glm::vec3 > controlPoints;

	// River animation
	float animationSpeed;
	float tilingFactor;

	// Particle Effect
	std::unique_ptr< ParticleEffect<Particle> > splashEffect;

	// Editing
	float smoothness;

	// Camera
	float viewDistance;
	glm::vec2 aspectRatio;
	std::unique_ptr<EngineComponents::Camera> camera;

	// Current mouse selection
	int selection;
	float clickDistanceThreshold;

	// Curve generatiom parameters
	int instanceCount;
	int generatedPoints;
	float riverWidth;
};