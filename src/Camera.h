#pragma once

#include <glm/glm.hpp>

constexpr float tileSize = 32.f;
const glm::vec3 axes[] = {
	glm::vec3(-tileSize, -0.5f * tileSize, 0.001f /*-1.f * t*/),
	glm::vec3(tileSize, -0.5f * tileSize, 0.001f /*-1.f * t*/),
	glm::vec3(0.f, tileSize, 0.f)
};

class Camera
{
public:
	Camera()
		: m_center(0.f)
	{
		m_view = glm::mat4(
			glm::vec4(axes[0], 0.f),
			glm::vec4(axes[1], 0.f),
			glm::vec4(axes[2], 0.f),
			glm::vec4(glm::vec3(0.f, 0.f, 0.f), 1.f)
		);
	}

	void zoom(float zoomFactor)
	{
		m_view = glm::inverse(glm::scale(glm::inverse(m_view), glm::vec3(1.f / zoomFactor, 1.f / zoomFactor, 1.f / zoomFactor)));
	}

	void move(const glm::vec3& move)
	{
		glm::mat4 viewTranslatedInverse = glm::translate(glm::inverse(m_view), move);
		m_center = glm::vec3(viewTranslatedInverse[3]);
		m_view = glm::inverse(viewTranslatedInverse);
	}

	void rotate(float angle)
	{
		m_view = glm::rotate(m_view, angle, glm::vec3(0.f, 0.f, 1.f));
	}

	const glm::mat4& getViewMatrix() const { return m_view; }
	const glm::vec3& getCenter() const { return m_center; }

protected:
	glm::mat4 m_view;
	glm::vec3 m_center;
};
