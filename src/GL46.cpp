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

	GLProgram program;
	program.load("shaders/tile.frag", "shaders/tile.vert");

    SDL_Event event;
    bool loop = true;
    while (loop)
    {
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
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					windowWidth = event.window.data1;
					windowWidth = event.window.data2;
					break;
				}
            }
        }

		const float currentTime = static_cast<float>(SDL_GetTicks()) * 0.001f;

		glClear(GL_COLOR_BUFFER_BIT);

		glViewport(0, 0, windowWidth, windowHeight);

		PerFrameData perFrameData;
		const float t = 100.f;
		const glm::vec3 axes[] = {
			glm::vec3(-t, -0.5f * t, 0.f /*-1.f * t*/),
			glm::vec3(t, -0.5f * t, 0.f /*-1.f * t*/),
			glm::vec3(0.f, t, 0.f)
		};
		perFrameData.view = glm::mat4(
			glm::vec4(axes[0], 0.f),
			glm::vec4(axes[1], 0.f),
			glm::vec4(axes[2], 0.f),
			glm::vec4(glm::vec3(0.f, 0.f, 0.f), 1.f)
		);
		perFrameData.projection = glm::ortho(
			static_cast<float>(windowWidth) * -0.5f, static_cast<float>(windowWidth) * 0.5f,
			static_cast<float>(windowHeight) * -0.5f, static_cast<float>(windowHeight) * 0.5f
		);

		perFrameData.color = glm::vec4(1.f, 1.f, 0.f, 1.f);

		const glm::vec3 initialLightDirection = glm::normalize(glm::vec3(-1.f, -1.f, -1.f));
		const glm::vec3 lightDirection = glm::rotateZ(initialLightDirection, currentTime);
		//const glm::vec3 lightDirection = initialLightDirection;

		perFrameData.lightDirection = glm::vec4(lightDirection, 1.f);
		tileMesh.setPerFrameData(perFrameData);

		GLIndirectCommandsBuffer<1024>& commandsBuffer = tileMesh.getIndirectCommandsBuffer();
		commandsBuffer.clearCommands();
		commandsBuffer.addCommand(
			sizeof(tileIndices) / sizeof(GLuint), // number of vertices
			1, // number of instances to draw
			0, // index offset
			0, // vertex offset
			0 // base instance
		);
		commandsBuffer.addCommand(
			sizeof(tileIndices) / sizeof(GLuint), // number of vertices
			1, // number of instances to draw
			0, // index offset
			0, // vertex offset
			1 // base instance
		);
		commandsBuffer.upload();

		program.use();

		tileMesh.draw();

		glUseProgram(0);

        SDL_GL_SwapWindow(window);
    }

	SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
