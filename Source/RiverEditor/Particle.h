#pragma once

#include <Core\Engine.h>

struct Particle
{
	glm::vec4 position;
	glm::vec4 speed;
	glm::vec4 initialPos;
	glm::vec4 initialSpeed;

	Particle() {};

	Particle(const glm::vec4 &pos, const glm::vec4 &speed)
	{
		SetInitial(pos, speed);
	}

	void SetInitial(const glm::vec4 &pos, const glm::vec4 &speed)
	{
		position = pos;
		initialPos = pos;

		this->speed = speed;
		initialSpeed = speed;
	}
};
