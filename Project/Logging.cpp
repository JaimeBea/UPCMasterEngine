#include "Logging.h"

#include <sstream>
#include <windows.h>
#include <stdio.h>

#include "Leaks.h"

void log(const char file[], int line, const char* format, ...)
{
	static char tmp_string[4096];
	static char tmp_string2[4096];
	static va_list  ap;

	// Construct the string from variable arguments
	va_start(ap, format);
	vsprintf_s(tmp_string, 4096, format, ap);
	va_end(ap);
	sprintf_s(tmp_string2, 4096, "%s(%d) : %s\n", file, line, tmp_string);
	OutputDebugString(tmp_string2);

	log_string.append(tmp_string2);
}

void log_delta_ms(float delta_ms)
{
	float fps = 1000.0f / delta_ms;
	fps_log_index -= 1;
	if (fps_log_index < 0)
	{
		fps_log_index = FPS_LOG_SIZE - 1;
	}
	fps_log[fps_log_index] = fps;
	ms_log[fps_log_index] = delta_ms;
}

std::string log_string;
int fps_log_index = FPS_LOG_SIZE - 1;
float fps_log[FPS_LOG_SIZE] = { 0 };
float ms_log[FPS_LOG_SIZE] = { 0 };
