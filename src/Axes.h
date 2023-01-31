#pragma once

#include <glm/glm.hpp>

constexpr float tileSize = 20.f;
const glm::vec3 axes[] = {
	glm::vec3(-tileSize, -0.5f * tileSize, 0.001f /*-1.f * t*/),
	glm::vec3(tileSize, -0.5f * tileSize, 0.001f /*-1.f * t*/),
	glm::vec3(0.f, tileSize, 0.f)
};
