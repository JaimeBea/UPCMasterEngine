#include "ModuleInput.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRender.h"
#include "ModuleCamera.h"
#include "imgui_impl_sdl.h"
#include "SDL.h"

ModuleInput::ModuleInput()
{
	memset(keyboard, KEY_IDLE, sizeof(KeyState) * SDL_NUM_SCANCODES);
	memset(mouse_buttons, KEY_IDLE, sizeof(KeyState) * NUM_MOUSE_BUTTONS);
}

ModuleInput::~ModuleInput() {}

bool ModuleInput::Init()
{
	LOG("Init SDL input event system");
	bool ret = true;
	SDL_Init(0);

	if(SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		LOG("SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	return ret;
}

UpdateStatus ModuleInput::PreUpdate()
{
	ImGuiIO& io = ImGui::GetIO();

	mouse_motion = { 0, 0 };
	mouse_wheel_motion = 0;

	int mouse_x;
	int mouse_y;
	SDL_GetMouseState(&mouse_x, &mouse_y);
	mouse.x = (float) mouse_x;
	mouse.y = (float) mouse_y;
	mouse_motion.x = 0;
	mouse_motion.y = 0;

	int window_id = SDL_GetWindowID(App->window->window);

	for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
	{
		if (keyboard[i] == KEY_DOWN)
			keyboard[i] = KEY_REPEAT;

		if (keyboard[i] == KEY_UP)
			keyboard[i] = KEY_IDLE;

		if (io.WantCaptureKeyboard)
			keyboard[i] = KEY_IDLE;
	}

	for (int i = 0; i < NUM_MOUSE_BUTTONS; ++i)
	{
		if (mouse_buttons[i] == KEY_DOWN)
			mouse_buttons[i] = KEY_REPEAT;

		if (mouse_buttons[i] == KEY_UP)
			mouse_buttons[i] = KEY_IDLE;

		if (io.WantCaptureMouse)
			keyboard[i] = KEY_IDLE;
	}

	SDL_Event event;
	while (SDL_PollEvent(&event) != 0)
	{
		ImGui_ImplSDL2_ProcessEvent(&event);
		
		switch (event.type)
		{
		case SDL_QUIT:
			return UpdateStatus::STOP;

		case SDL_WINDOWEVENT:
			if (event.window.windowID == window_id)
			{
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_CLOSE:
					return UpdateStatus::STOP;

				case SDL_WINDOWEVENT_RESIZED:
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					App->renderer->WindowResized(event.window.data1, event.window.data2);
					App->camera->WindowResized(event.window.data1, event.window.data2);
					break;
				}
			}
			break;

		case SDL_MOUSEMOTION:
			if (!io.WantCaptureMouse)
			{
				mouse_motion.x += event.motion.xrel;
				mouse_motion.y += event.motion.yrel;
			}
			break;

		case SDL_MOUSEWHEEL:
			if (!io.WantCaptureMouse)
			{
				if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
				{
					mouse_wheel_motion = (float) event.wheel.x;
				}
				else
				{
					mouse_wheel_motion = (float) event.wheel.y;
				}
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
			if (!io.WantCaptureMouse)
			{
				mouse_buttons[event.button.button - 1] = KEY_DOWN;
			}
			break;

		case SDL_MOUSEBUTTONUP:
			if (!io.WantCaptureMouse)
			{
				mouse_buttons[event.button.button - 1] = KEY_UP;
			}
			break;

		case SDL_KEYDOWN:
			if (!io.WantCaptureKeyboard)
			{
				keyboard[event.key.keysym.scancode] = KEY_DOWN;
			}
			break;

		case SDL_KEYUP:
			if (!io.WantCaptureKeyboard)
			{
				keyboard[event.key.keysym.scancode] = KEY_UP;
			}
			break;
		}
	}

	if (GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
	{
		return UpdateStatus::STOP;
	}

    return UpdateStatus::CONTINUE;
}

// Called before quitting
bool ModuleInput::CleanUp()
{
	LOG("Quitting SDL input event subsystem");
	SDL_QuitSubSystem(SDL_INIT_EVENTS);
	return true;
}
