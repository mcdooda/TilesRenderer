#pragma once

#include <cassert>
#include <string>
#include <GL/glew.h>
#include <SDL2/SDL_image.h>

class BindlessTexture
{
public:
	BindlessTexture() = delete;
	BindlessTexture(const BindlessTexture&) = delete;
	BindlessTexture(BindlessTexture&&) = delete;
	void operator=(const BindlessTexture&) = delete;
	void operator=(BindlessTexture&&) = delete;

	BindlessTexture(const std::string& filePath)
	{
		SDL_Surface* surface = IMG_Load(filePath.c_str());
		assert(surface != nullptr);

		m_size.x = surface->w;
		m_size.y = surface->h;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_handle);
		glTextureParameteri(m_handle, GL_TEXTURE_MAX_LEVEL, 0);
		glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureStorage2D(m_handle, 1, GL_RGBA8, surface->w, surface->h);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTextureSubImage2D(m_handle, 0, 0, 0, surface->w, surface->h, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
		glBindTextures(0, 1, &m_handle);

		m_handleBindless = glGetTextureHandleARB(m_handle);
		glMakeTextureHandleResidentARB(m_handleBindless);

		SDL_FreeSurface(surface);
	}

	~BindlessTexture()
	{
		glMakeTextureHandleNonResidentARB(m_handleBindless);
		glDeleteTextures(1, &m_handle);
	}

	GLuint64 getHandleBindless() const { return m_handleBindless; }
	const glm::ivec2& getSize() const { return m_size; }

protected:
	GLuint m_handle;
	GLuint64 m_handleBindless;
	glm::ivec2 m_size;
};
