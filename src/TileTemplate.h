#pragma once

#include <cassert>
#include <cstdint>
#include <memory>

#include "BindlessTexture.h"

static constexpr GLuint64 InvalidTexture = 0xFFFFFFFFFFFFFFFF;

struct TileTemplateData
{
	GLuint64 albedoTexture = InvalidTexture;
	GLuint numVariants;
	GLuint numAnimationFrames;
};

class TileTemplate
{
public:
	TileTemplate(const std::string& filePath, float* tileVariantProbabilities, int numTileVariants, float frameDuration, GLuint numAnimationFrames)
		: m_texture(std::make_shared<BindlessTexture>(filePath))
		, m_tileVariantProbabilities(tileVariantProbabilities, tileVariantProbabilities + numTileVariants)
		, m_frameDuration(frameDuration)
		, m_numAnimationFrames(numAnimationFrames)
	{
		m_tileVariantProbabilitiesSum = 0.f;
		for (float probability : m_tileVariantProbabilities)
		{
			m_tileVariantProbabilitiesSum += probability;
		}
	}

	int getRandomTileVariantIndex() const
	{
		assert(!m_tileVariantProbabilities.empty() && m_tileVariantProbabilitiesSum > 0.f);
		float random = static_cast<float>(rand()) / RAND_MAX * m_tileVariantProbabilitiesSum;
		int randomIndex = 0;
		for (float probability : m_tileVariantProbabilities)
		{
			if (random <= probability)
			{
				break;
			}
			random -= probability;
			++randomIndex;
		}
		assert(0 <= randomIndex && randomIndex < m_tileVariantProbabilities.size());
		return randomIndex;
	}

	const BindlessTexture& getTexture() const { return *m_texture; }
	GLuint getNumVariants() const { return static_cast<GLuint>(m_tileVariantProbabilities.size()); }
	GLuint getNumAnimationFrames() const { return m_numAnimationFrames; }

protected:
	std::shared_ptr<BindlessTexture> m_texture;
	std::vector<float> m_tileVariantProbabilities;
	float m_tileVariantProbabilitiesSum;
	float m_frameDuration;
	GLuint m_numAnimationFrames;
};
