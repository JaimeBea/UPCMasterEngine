#include "Globals.h"
#include "Application.h"
#include "Logging.h"

#include "SDL.h"
#include <stdlib.h>

enum class MainState
{
	CREATION,
	START,
	UPDATE,
	FINISH,
	EXIT
};

Application* App = nullptr;

int main(int argc, char ** argv)
{
	int main_return = EXIT_FAILURE;
	MainState state = MainState::CREATION;

	while (state != MainState::EXIT)
	{
		switch (state)
		{
		case MainState::CREATION:
			LOG("Application Creation --------------");
			App = new Application();
			state = MainState::START;
			break;

		case MainState::START:
			LOG("Application Init --------------");
			if (App->Init() == false)
			{
				LOG("Application Init exits with error -----");
				state = MainState::EXIT;
			}
			else
			{
				state = MainState::UPDATE;
				LOG("Application Update --------------");
			}
			break;

		case MainState::UPDATE:
		{
			UpdateStatus update_return = App->Update();

			if (update_return == UpdateStatus::ERROR)
			{
				LOG("Application Update exits with error -----");
				state = MainState::EXIT;
			}

			if (update_return == UpdateStatus::STOP)
			{
				state = MainState::FINISH;
			}
			break;
		}

		case MainState::FINISH:
			LOG("Application CleanUp --------------");
			if (App->CleanUp() == false)
			{
				LOG("Application CleanUp exits with error -----");
			}
			else
			{
				main_return = EXIT_SUCCESS;
			}
			delete App;
			state = MainState::EXIT;
			break;
		}

	}

	LOG("Bye :)\n");

	// Clear log at the end
	for (char* log_line : log_lines)
	{
		delete[] log_line;
	}

	return main_return;
}
