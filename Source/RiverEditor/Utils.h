#pragma once

#include <Core/Engine.h>

namespace Utils
{
	float RandFloat(float min, float max)
	{
		return min + static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (max - min));
	}

	glm::vec2 RandInsideUnitCircle()
	{
		float seed = Utils::RandFloat(0.0f, 1.0f);
		float radius_seed = Utils::RandFloat(0.0f, 1.0f);
		float x = sqrt(radius_seed) * cos(2 * M_PI * seed);
		float y = sqrt(radius_seed) * sin(2 * M_PI * seed);

		return glm::vec2(x, y);
	}
}
