#pragma once

#include <cassert>
#include <vector>
#include <GL/glew.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "Buffer.h"

struct PerFrameData
{
	glm::mat4 view;
	glm::mat4 projection;
	glm::vec4 color;
	glm::vec4 lightDirection;
};

/*
/  0  \
1     2
\  3  /

4     8
| 5 9 |
6 | |10
\ 7 11/
*/

const GLuint tileIndices[] = {
	0, 1, 2,
	1, 2, 3,
	4, 5, 6,
	6, 5, 7,
	8, 9, 10,
	10, 9, 11
};

const glm::vec3 tileVertices[] = {
	// tile top with upwards normals
	glm::vec3(-0.5f, -0.5f, 0.f),  glm::vec3(0.f, 0.f, 1.f),
	glm::vec3(0.5f,  -0.5f, 0.f),  glm::vec3(0.f, 0.f, 1.f),
	glm::vec3(-0.5f,  0.5f, 0.f),  glm::vec3(0.f, 0.f, 1.f),
	glm::vec3(0.5f,   0.5f, 0.f),  glm::vec3(0.f, 0.f, 1.f),

	// tile left side with sideways normals
	glm::vec3(0.5f,  -0.5f,  0.f), glm::vec3(1.f, 0.f, 0.f),
	glm::vec3(0.5f,   0.5f,  0.f), glm::vec3(1.f, 0.f, 0.f),
	glm::vec3(0.5f,  -0.5f, -1.f), glm::vec3(1.f, 0.f, 0.f),
	glm::vec3(0.5f,   0.5f, -1.f), glm::vec3(1.f, 0.f, 0.f),

	// tile right side with sideways normals
	glm::vec3(-0.5f,  0.5f,  0.f), glm::vec3(0.f, 1.f, 0.f),
	glm::vec3(0.5f,   0.5f,  0.f), glm::vec3(0.f, 1.f, 0.f),
	glm::vec3(-0.5f,  0.5f, -1.f), glm::vec3(0.f, 1.f, 0.f),
	glm::vec3(0.5f,   0.5f, -1.f), glm::vec3(0.f, 1.f, 0.f),
};

class TileMesh
{
public:
	static constexpr int MaxTiles = 1024 * 1024;

	TileMesh()
		: m_indicesBuffer(tileIndices, sizeof(tileIndices))
		, m_verticesBuffer(tileVertices, sizeof(tileVertices))
	{
		glCreateVertexArrays(1, &m_vao);
		glVertexArrayElementBuffer(m_vao, m_indicesBuffer.getHandle());
		glVertexArrayVertexBuffer(m_vao, 0, m_verticesBuffer.getHandle(), 0, sizeof(glm::vec3) + sizeof(glm::vec3));
		// position
		glEnableVertexArrayAttrib(m_vao, 0);
		glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(m_vao, 0, 0);
		// normal
		glEnableVertexArrayAttrib(m_vao, 1);
		glVertexArrayAttribFormat(m_vao, 1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3));
		glVertexArrayAttribBinding(m_vao, 1, 0);
	}

	~TileMesh()
	{
		glDeleteVertexArrays(1, &m_vao);
	}

	void setPerFrameData(const PerFrameData& perFrameData)
	{
		m_perFrameDataBuffer.update(perFrameData);
	}

	void addTile(const glm::vec3& tilePosition)
	{
		const GLuint baseInstance = static_cast<GLuint>(m_tilesBuffer.getObjectCount());
		m_tilesBuffer.addObject(glm::vec4(tilePosition, 0.f));
		m_indirectCommandsBuffer.addCommand(
			sizeof(tileIndices) / sizeof(GLuint), // number of vertices
			1, // number of instances to draw
			0, // index offset
			0, // vertex offset
			baseInstance
		);
	}

	void upload()
	{
		std::cout << "Uploading " << m_tilesBuffer.getObjectCount() << " tiles" << std::endl;
		m_tilesBuffer.upload();
		m_indirectCommandsBuffer.upload();
	}

	void draw()
	{
		glBindVertexArray(m_vao);
		m_perFrameDataBuffer.bind(GL_UNIFORM_BUFFER, PerFrameBufferIndex);
		m_tilesBuffer.bind(GL_SHADER_STORAGE_BUFFER, TilesBufferIndex);
		m_indirectCommandsBuffer.draw();
	}

protected:
	static constexpr GLuint PerFrameBufferIndex = 0;
	static constexpr GLuint TilesBufferIndex = 1;
	static constexpr GLuint TileTemplatesBufferIndex = 2;

	GLMutableBuffer<PerFrameData> m_perFrameDataBuffer;

	GLuint m_vao;
	GLBuffer m_indicesBuffer;
	GLBuffer m_verticesBuffer;

	GLArrayBuffer<glm::vec4, MaxTiles> m_tilesBuffer;

	GLIndirectCommandsBuffer<MaxTiles> m_indirectCommandsBuffer;
};
