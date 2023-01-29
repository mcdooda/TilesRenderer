#pragma once

#include <cassert>
#include <vector>
#include <GL/glew.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "Buffer.h"
#include "Program.h"

class DebugMesh
{
public:
	static constexpr int MaxVertices = 1024 * 1024;

	struct PerFrameData
	{
		glm::mat4 view;
		glm::mat4 projection;
	};

	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 color;
	};

	DebugMesh()
	{
		glCreateVertexArrays(1, &m_vao);
		glVertexArrayVertexBuffer(m_vao, 0, m_verticesBuffer.getHandle(), 0, sizeof(Vertex));
		// position
		glEnableVertexArrayAttrib(m_vao, 0);
		glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(m_vao, 0, 0);
		// color
		glEnableVertexArrayAttrib(m_vao, 1);
		glVertexArrayAttribFormat(m_vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));
		glVertexArrayAttribBinding(m_vao, 1, 0);

		m_debugProgram.load("shaders/debug.frag", "shaders/debug.vert");
	}

	~DebugMesh()
	{
		glDeleteVertexArrays(1, &m_vao);
	}

	void setPerFrameData(const PerFrameData& perFrameData)
	{
		m_perFrameDataBuffer.update(perFrameData);
	}

	void addLine(const glm::vec3& position1, const glm::vec3& position2, glm::vec3 color)
	{
		Vertex& vertex1 = m_verticesBuffer.addObject();
		vertex1.position = position1;
		vertex1.color = color;

		Vertex& vertex2 = m_verticesBuffer.addObject();
		vertex2.position = position2;
		vertex2.color = color;
	}

	void addAxes(const glm::vec3& position)
	{
		addLine(position + glm::vec3(-1.f, 0.f, 0.f), position + glm::vec3(1.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f));
		addLine(position + glm::vec3(0.f, -1.f, 0.f), position + glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
		addLine(position + glm::vec3(0.f, 0.f, -1.f), position + glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 1.f));
	}

	void draw()
	{
		m_verticesBuffer.upload();

		m_debugProgram.use();

		glBindVertexArray(m_vao);
		m_perFrameDataBuffer.bind(GL_UNIFORM_BUFFER, PerFrameBufferIndex);
		glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_verticesBuffer.getObjectCount()));
		glBindVertexArray(0);

		glUseProgram(0);

		m_verticesBuffer.clearObjects();
	}

protected:
	static constexpr GLuint PerFrameBufferIndex = 0;

	GLMutableBuffer<PerFrameData> m_perFrameDataBuffer;

	GLuint m_vao;
	GLArrayBuffer<Vertex, MaxVertices> m_verticesBuffer;

	GLProgram m_debugProgram;
};
