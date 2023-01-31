#pragma once

#include <cassert>
#include <vector>
#include <GL/glew.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "Axes.h"
#include "BindlessTexture.h"
#include "Buffer.h"
#include "Program.h"
#include "TileTemplate.h"

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

struct TileVertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
};

struct alignas(16) TileData
{
	glm::vec4 position;
	unsigned int tileTemplateIndex;
	unsigned int tileVariantIndex;
};

class TileMesh
{
public:
	static constexpr int MaxTileTemplates = 256;
	static constexpr int MaxTiles = 1024 * 1024;

	struct PerFrameData
	{
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec4 grassColor;
		glm::vec4 dirtColor;
		glm::vec4 lightDirection;
	};

	TileMesh(const TileTemplate& tileTemplate)
		: m_indicesBuffer(tileIndices, sizeof(tileIndices))
	{
		const glm::ivec2& spriteSize = tileTemplate.getTexture().getSize();
		const float spriteWidth = static_cast<float>(spriteSize.x);
		const float spriteHeight = static_cast<float>(spriteSize.y);
		const float spriteTileHeight = spriteHeight / tileTemplate.getNumVariants();
		const float spriteTileWidth = spriteWidth / tileTemplate.getNumAnimationFrames();

		const float localMinU = 0.f;
		const float localMaxU = 1.f / tileTemplate.getNumAnimationFrames();
		const float localMinV = 0.f;
		const float localMaxV = 1.f / static_cast<float>(tileTemplate.getNumVariants());

		const float tileHeight3d = (spriteTileHeight + axes[0].y + axes[1].y) / axes[2].y;
		assert(tileHeight3d >= 0.f);
		const float bottomZ = -tileHeight3d;

		const glm::vec2 uv0(localMinU - axes[0].x / spriteWidth, localMinV);
		const glm::vec2 uv1(localMinU, localMinV - axes[0].y / spriteHeight);
		const glm::vec2 uv2(localMaxU, localMinV - axes[1].y / spriteHeight);
		const glm::vec2 uv3(localMinU - axes[0].x / spriteWidth, localMinV + (-axes[0].y - axes[1].y) / spriteHeight);
		const glm::vec2 uv4(uv1.x, localMaxV + axes[1].y / spriteHeight);
		const glm::vec2 uv5(uv2.x, localMaxV + axes[0].y / spriteHeight);
		const glm::vec2 uv6(uv3.x, localMaxV);

		const TileVertex tileVertices[] = {
			// tile top with upwards normals
			{ glm::vec3(-0.5f, -0.5f,    0.f),  glm::vec3(0.f, 0.f, 1.f), uv0 },
			{ glm::vec3(0.5f,  -0.5f,    0.f),  glm::vec3(0.f, 0.f, 1.f), uv1 },
			{ glm::vec3(-0.5f,  0.5f,    0.f),  glm::vec3(0.f, 0.f, 1.f), uv2 },
			{ glm::vec3(0.5f,   0.5f,    0.f),  glm::vec3(0.f, 0.f, 1.f), uv3 },

			// tile left side with sideways normals
			{ glm::vec3(0.5f,  -0.5f,     0.f), glm::vec3(1.f, 0.f, 0.f), uv1 },
			{ glm::vec3(0.5f,   0.5f,     0.f), glm::vec3(1.f, 0.f, 0.f), uv3 },
			{ glm::vec3(0.5f,  -0.5f, bottomZ), glm::vec3(1.f, 0.f, 0.f), uv4 },
			{ glm::vec3(0.5f,   0.5f, bottomZ), glm::vec3(1.f, 0.f, 0.f), uv6 },

			// tile right side with sideways normals
			{ glm::vec3(-0.5f,  0.5f,     0.f), glm::vec3(0.f, 1.f, 0.f), uv2 },
			{ glm::vec3(0.5f,   0.5f,     0.f), glm::vec3(0.f, 1.f, 0.f), uv3 },
			{ glm::vec3(-0.5f,  0.5f, bottomZ), glm::vec3(0.f, 1.f, 0.f), uv5 },
			{ glm::vec3(0.5f,   0.5f, bottomZ), glm::vec3(0.f, 1.f, 0.f), uv6 },
		};

		m_verticesBuffer.update(tileVertices, sizeof(tileVertices));

		glCreateVertexArrays(1, &m_vao);
		glVertexArrayElementBuffer(m_vao, m_indicesBuffer.getHandle());
		glVertexArrayVertexBuffer(m_vao, 0, m_verticesBuffer.getHandle(), 0, sizeof(TileVertex));
		// position
		glEnableVertexArrayAttrib(m_vao, 0);
		glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(TileVertex, position));
		glVertexArrayAttribBinding(m_vao, 0, 0);
		// normal
		glEnableVertexArrayAttrib(m_vao, 1);
		glVertexArrayAttribFormat(m_vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(TileVertex, normal));
		glVertexArrayAttribBinding(m_vao, 1, 0);
		// uv
		glEnableVertexArrayAttrib(m_vao, 2);
		glVertexArrayAttribFormat(m_vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(TileVertex, uv));
		glVertexArrayAttribBinding(m_vao, 2, 0);

		m_tileProgram.load("shaders/tile.frag", "shaders/tile.vert");
	}

	~TileMesh()
	{
		glDeleteVertexArrays(1, &m_vao);
	}

	void setPerFrameData(const PerFrameData& perFrameData)
	{
		m_perFrameDataBuffer.update(perFrameData);
	}

	int addTileTemplate(const TileTemplate& tileTemplate)
	{
		int index = static_cast<int>(m_tileTemplates.size());
		m_tileTemplates.push_back(tileTemplate);

		TileTemplateData tileTemplateData;
		tileTemplateData.albedoTexture = tileTemplate.getTexture().getHandleBindless();
		tileTemplateData.numVariants = tileTemplate.getNumVariants();
		tileTemplateData.numAnimationFrames = tileTemplate.getNumAnimationFrames();

		m_tileTemplatesBuffer.addObject(tileTemplateData);

		return index;
	}

	void addTile(const glm::vec3& tilePosition, int tileTemplateIndex)
	{
		const GLuint baseInstance = static_cast<GLuint>(m_tilesBuffer.getObjectCount());
		const TileTemplate& tileTemplate = m_tileTemplates[tileTemplateIndex];
		int tileVariantIndex = tileTemplate.getRandomTileVariantIndex();
		TileData tileData;
		tileData.position = glm::vec4(tilePosition, 1.f);
		tileData.tileTemplateIndex = tileTemplateIndex;
		tileData.tileVariantIndex = tileTemplate.getRandomTileVariantIndex();
		m_tilesBuffer.addObject(tileData);
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
		m_tileTemplatesBuffer.upload();
		m_indirectCommandsBuffer.upload();
	}

	void draw()
	{
		m_tileProgram.use();

		glBindVertexArray(m_vao);
		m_perFrameDataBuffer.bind(GL_UNIFORM_BUFFER, PerFrameBufferIndex);
		m_tilesBuffer.bind(GL_SHADER_STORAGE_BUFFER, TilesBufferIndex);
		m_tileTemplatesBuffer.bind(GL_SHADER_STORAGE_BUFFER, TileTemplatesBufferIndex);
		m_indirectCommandsBuffer.draw();
		glBindVertexArray(0);

		glUseProgram(0);
	}

protected:
	static constexpr GLuint PerFrameBufferIndex = 0;
	static constexpr GLuint TilesBufferIndex = 1;
	static constexpr GLuint TileTemplatesBufferIndex = 2;

	std::vector<TileTemplate> m_tileTemplates;

	GLMutableBuffer<PerFrameData> m_perFrameDataBuffer;

	GLuint m_vao;
	GLBuffer m_indicesBuffer;
	GLBuffer m_verticesBuffer;

	GLArrayBuffer<TileTemplateData, MaxTileTemplates> m_tileTemplatesBuffer;
	GLArrayBuffer<TileData, MaxTiles> m_tilesBuffer;

	GLIndirectCommandsBuffer<MaxTiles> m_indirectCommandsBuffer;

	GLProgram m_tileProgram;
};
