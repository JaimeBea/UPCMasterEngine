#include "ModuleRender.h"

#include "Globals.h"
#include "Application.h"
#include "Logging.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleCamera.h"
#include "ModuleDebugDraw.h"
#include "ModuleTextures.h"
#include "ModuleModels.h"

#include "GL/glew.h"
#include "SDL.h"

#include "Leaks.h"

static void __stdcall OurOpenGLErrorFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	const char* tmp_source = "", * tmp_type = "", * tmp_severity = "";
	switch (source) {
	case GL_DEBUG_SOURCE_API: tmp_source = "API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: tmp_source = "Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: tmp_source = "Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY: tmp_source = "Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION: tmp_source = "Application"; break;
	case GL_DEBUG_SOURCE_OTHER: tmp_source = "Other"; break;
	};
	switch (type) {
	case GL_DEBUG_TYPE_ERROR: tmp_type = "Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: tmp_type = "Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: tmp_type = "Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY: tmp_type = "Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE: tmp_type = "Performance"; break;
	case GL_DEBUG_TYPE_MARKER: tmp_type = "Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP: tmp_type = "Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP: tmp_type = "Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER: tmp_type = "Other"; break;
	};
	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH: tmp_severity = "high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM: tmp_severity = "medium"; break;
	case GL_DEBUG_SEVERITY_LOW: tmp_severity = "low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: tmp_severity = "notification"; break;
	};

	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
	{
		return;
	}

	LOG("<Source:%s> <Type:%s> <Severity:%s> <ID:%d> <Message:%s>", tmp_source, tmp_type, tmp_severity, id, message);
}

bool ModuleRender::Init()
{
	LOG("Creating Renderer context");

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4); // desired version
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	context = SDL_GL_CreateContext(App->window->window);

	GLenum err = glewInit();
	LOG("Using Glew %s", glewGetString(GLEW_VERSION));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(&OurOpenGLErrorFunction, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
#endif

	glGenFramebuffers(1, &framebuffer);
	glGenRenderbuffers(1, &depth_renderbuffer);
	glGenTextures(1, &render_texture);

	ViewportResized(200, 200);

	return true;
}

bool ModuleRender::Start()
{
	current_model = App->models->LoadModel("Assets/BakerHouse.fbx");

	return true;
}

UpdateStatus ModuleRender::PreUpdate()
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glViewport(0, 0, viewport_width, viewport_height);

	glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return UpdateStatus::CONTINUE;
}

UpdateStatus ModuleRender::Update()
{
	App->debug_draw->Draw(App->camera->GetViewMatrix(), App->camera->GetProjectionMatrix(), viewport_width, viewport_height);

	// Load model or texture if one gets dropped
	const char* dropped_file_name = App->input->GetDroppedFileName();
	if (dropped_file_name != nullptr)
	{
		Model* loaded_model = App->models->LoadModel(dropped_file_name);
		if (loaded_model)
		{
			App->models->ReleaseModel(current_model);
			current_model = loaded_model;
			App->camera->Focus(current_model);
		}
		else
		{
			unsigned loaded_texture = App->textures->LoadTexture(dropped_file_name);
			if (loaded_texture)
			{
				for (unsigned material : current_model->materials)
				{
					App->textures->ReleaseTexture(material);
				}
				current_model->materials.clear();
				current_model->materials.push_back(loaded_texture);
				for (Mesh& mesh : current_model->meshes)
				{
					mesh.material_index = 0;
				}
			}
		}
		App->input->ReleaseDroppedFileName();
	}

	// Draw the model
	current_model->Draw(current_model_model_matrix);

	return UpdateStatus::CONTINUE;
}

UpdateStatus ModuleRender::PostUpdate()
{
	SDL_GL_SwapWindow(App->window->window);

	return UpdateStatus::CONTINUE;
}

bool ModuleRender::CleanUp()
{
	glDeleteTextures(1, &render_texture);
	glDeleteRenderbuffers(1, &depth_renderbuffer);
	glDeleteFramebuffers(1, &framebuffer);

	App->models->ReleaseModel(current_model);

	return true;
}

void ModuleRender::ViewportResized(int width, int height)
{
	viewport_width = width;
	viewport_height = height;

	// Framebuffer calculations
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glBindTexture(GL_TEXTURE_2D, render_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_texture, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_renderbuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG("ERROR: Framebuffer is not complete!");
	}
}

void ModuleRender::SetVSync(bool vsync)
{
	SDL_GL_SetSwapInterval(vsync);
}
