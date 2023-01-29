#include <cassert>
#include <iostream>
#include <SDL2/SDL.h>
#include <GL/glew.h>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Camera.h"
#include "DebugMesh.h"
#include "TileMesh.h"

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

	glEnable(GL_DEPTH_TEST);

	// tiles
	TileMesh tileMesh;
	constexpr int mapHalfSize = 200;
	for (int x = -mapHalfSize; x <= mapHalfSize; ++x)
	{
		const float fx = static_cast<float>(x);
		for (int y = -mapHalfSize; y <= mapHalfSize; ++y)
		{
			const float fy = static_cast<float>(y);
			const float z = std::cos(std::sqrt(fx * fx + fy * fy) * 0.5f) * 0.3f - std::min(std::max(std::abs(fx), std::abs(fy)), 10.f) * 0.8f;
			tileMesh.addTile(glm::vec3(x, y, z));
		}
	}
	tileMesh.upload();

	// debug
	DebugMesh debugMesh;

	Camera camera;

	float dt = 0.01f;

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
				camera.zoom(zoom);
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
					move.y = 1.f;
				}
				else if (down)
				{
					move.y = -1.f;
				}
			}
			if (!(left && right))
			{
				if (left)
				{
					move.x = -1.f;
				}
				else if (right)
				{
					move.x = 1.f;
				}
			}
			camera.move(move * dt * 500.f);

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
			camera.rotate(rotation * dt);
		}

		glViewport(0, 0, windowWidth, windowHeight);

		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 projection = glm::ortho(
			static_cast<float>(windowWidth) * -0.5f, static_cast<float>(windowWidth) * 0.5f,
			static_cast<float>(windowHeight) * -0.5f, static_cast<float>(windowHeight) * 0.5f
		);

		{
			TileMesh::PerFrameData perFrameData;
			perFrameData.view = view;
			perFrameData.projection = projection;
			perFrameData.grassColor = glm::vec4(0.53f, 0.8f, 0.31f, 1.f);
			perFrameData.dirtColor = glm::vec4(0.51f, 0.43f, 0.3f, 1.f);

			const glm::vec3 initialLightDirection = glm::normalize(glm::vec3(-1.f, -1.f, -1.f));
			const glm::vec3 lightDirection = glm::rotateZ(initialLightDirection, t1);

			perFrameData.lightDirection = glm::vec4(lightDirection, 1.f);
			tileMesh.setPerFrameData(perFrameData);

			tileMesh.draw();
		}

		{
			DebugMesh::PerFrameData perFrameData;
			perFrameData.view = view;
			perFrameData.projection = projection;
			debugMesh.setPerFrameData(perFrameData);

			debugMesh.addAxes(camera.getCenter());

			debugMesh.draw();
		}

		glUseProgram(0);

        SDL_GL_SwapWindow(window);

		const float t2 = static_cast<float>(SDL_GetTicks()) * 0.001f;
		dt = t2 - t1;
		const float fps = 1.f / dt;

		std::stringstream title;
		title << fps;
		SDL_SetWindowTitle(window, title.str().c_str());
    }

	SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
