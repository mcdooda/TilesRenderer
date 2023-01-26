#include <cassert>
#include <iostream>
#include <SDL2/SDL.h>
#include <GL/glew.h>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Program.h"
#include "Renderer.h"

#define DEFAULT_WINDOW_WIDTH 500
#define DEFAULT_WINDOW_HEIGHT 500

static void handleGLDebugMessage(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	GLchar const* message,
	void const* user_param
)
{
	const char* sourceString = [source]()
	{
		switch (source)
		{
		case GL_DEBUG_SOURCE_API: return "API";
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
		case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
		case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
		case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
		case GL_DEBUG_SOURCE_OTHER: return "OTHER";
		}
		return "";
	}();

	const char* typeString = [type]()
	{
		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR: return "ERROR";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
		case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
		case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
		case GL_DEBUG_TYPE_MARKER: return "MARKER";
		case GL_DEBUG_TYPE_OTHER: return "OTHER";
		}
		return "";
	}();

	const char* severityString = [severity]()
	{
		switch (severity) {
		case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
		case GL_DEBUG_SEVERITY_LOW: return "LOW";
		case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
		case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
		}
		return "";
	}();

	std::cerr << sourceString << ", " << typeString << ", " << severityString << ", " << id << ": " << message << std::endl << std::endl;

	assert(type != GL_DEBUG_TYPE_ERROR);
}

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);

	int windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    //windowFlags |= SDL_WINDOW_FULLSCREEN;

	int windowWidth = DEFAULT_WINDOW_WIDTH;
	int windowHeight = DEFAULT_WINDOW_HEIGHT;

	SDL_Window* window = SDL_CreateWindow(
        "GL46",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		windowWidth, windowHeight,
        windowFlags
    );

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    assert(glContext != nullptr);

    SDL_GL_MakeCurrent(window, glContext);

	glewExperimental = GL_TRUE;
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	int err = glewInit();
    assert(err == GLEW_OK);

	glDebugMessageCallback(handleGLDebugMessage, nullptr);
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);

	glClearColor(0.5f, 0.3f, 0.2f, 1.f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	TileMesh tileMesh;
	constexpr int mapHalfSize = 200;
	for (int x = -mapHalfSize; x <= mapHalfSize; ++x)
	{
		const float fx = static_cast<float>(x);
		for (int y = -mapHalfSize; y <= mapHalfSize; ++y)
		{
			const float fy = static_cast<float>(y);
			tileMesh.addTile(glm::vec3(x, y, std::cos(x * 0.3f + y * 0.1f)));
		}
	}
	tileMesh.upload();

	const float tileSize = 32.f;
	const glm::vec3 axes[] = {
		glm::vec3(-tileSize, -0.5f * tileSize, 0.f /*-1.f * t*/),
		glm::vec3(tileSize, -0.5f * tileSize, 0.f /*-1.f * t*/),
		glm::vec3(0.f, tileSize, 0.f)
	};
	glm::mat4 view = glm::mat4(
		glm::vec4(axes[0], 0.f),
		glm::vec4(axes[1], 0.f),
		glm::vec4(axes[2], 0.f),
		glm::vec4(glm::vec3(0.f, 0.f, 0.f), 1.f)
	);

	float dt = 0.01f;

	GLProgram program;
	program.load("shaders/tile.frag", "shaders/tile.vert");

    SDL_Event event;
    bool loop = true;
    while (loop)
    {
		const float t1 = static_cast<float>(SDL_GetTicks()) * 0.001f;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                loop = false;
                break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					loop = false;
					break;
				}
				break;
			case SDL_MOUSEWHEEL:
			{
				const float wheelY = static_cast<float>(event.wheel.y);
				const float zoom = 1.f + wheelY * 0.1f;
				view = glm::scale(view, glm::vec3(zoom, zoom, zoom));
			}
				break;
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					windowWidth = event.window.data1;
					windowHeight = event.window.data2;
					break;
				}
			}
		}

		if (const Uint8* keyboardState = SDL_GetKeyboardState(nullptr))
		{
			const bool up = keyboardState[SDL_SCANCODE_UP];
			const bool down = keyboardState[SDL_SCANCODE_DOWN];
			const bool left = keyboardState[SDL_SCANCODE_LEFT];
			const bool right = keyboardState[SDL_SCANCODE_RIGHT];
			glm::vec3 move(0.f);
			if (!(up && down))
			{
				if (up)
				{
					move.y = -1.f;
				}
				else if (down)
				{
					move.y = 1.f;
				}
			}
			if (!(left && right))
			{
				if (left)
				{
					move.x = 1.f;
				}
				else if (right)
				{
					move.x = -1.f;
				}
			}
			view = glm::translate(view, move * dt * 100.f);

			const bool rotateLeft = keyboardState[SDL_SCANCODE_J];
			const bool rotateRight = keyboardState[SDL_SCANCODE_K];
			float rotation = 0.f;
			if (!(rotateLeft && rotateRight))
			{
				if (rotateLeft)
				{
					rotation = -1.f;
				}
				else if (rotateRight)
				{
					rotation = 1.f;
				}
			}
			view = glm::rotate(view, rotation * dt, glm::vec3(0.f, 0.f, 1.f));
		}

		glClear(GL_COLOR_BUFFER_BIT);

		glViewport(0, 0, windowWidth, windowHeight);

		PerFrameData perFrameData;
		perFrameData.view = view;
		perFrameData.projection = glm::ortho(
			static_cast<float>(windowWidth) * -0.5f, static_cast<float>(windowWidth) * 0.5f,
			static_cast<float>(windowHeight) * -0.5f, static_cast<float>(windowHeight) * 0.5f
		);

		perFrameData.color = glm::vec4(1.f, 1.f, 0.f, 1.f);

		const glm::vec3 initialLightDirection = glm::normalize(glm::vec3(-1.f, -1.f, -1.f));
		const glm::vec3 lightDirection = glm::rotateZ(initialLightDirection, t1);
		//const glm::vec3 lightDirection = initialLightDirection;

		perFrameData.lightDirection = glm::vec4(lightDirection, 1.f);
		tileMesh.setPerFrameData(perFrameData);

		program.use();

		tileMesh.draw();

		glUseProgram(0);

        SDL_GL_SwapWindow(window);

		const float t2 = static_cast<float>(SDL_GetTicks()) * 0.001f;
		dt = t2 - t1;
		const float fps = 1.f / dt;

		char title[20];
		sprintf(title, "%.1f", fps);
		SDL_SetWindowTitle(window, title);
    }

	SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
