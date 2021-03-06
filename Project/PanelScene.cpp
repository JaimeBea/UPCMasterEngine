#include "PanelScene.h"

#include "Globals.h"
#include "Application.h"
#include "ModuleEditor.h"
#include "ModuleCamera.h"
#include "ModuleRender.h"

#include "imgui.h"
#include "Math/float3x3.h"
#include "SDL_mouse.h"
#include "SDL_scancode.h"

#include "Leaks.h"

PanelScene::PanelScene() : Panel("Scene", true) {}

void PanelScene::Update()
{
	ImGui::SetNextWindowDockID(App->editor->dock_main_id, ImGuiCond_FirstUseEver);
	if (ImGui::Begin(name, &enabled))
	{
		// Update viewport size
		ImVec2 size = ImGui::GetContentRegionAvail();
		if (App->renderer->viewport_width != size.x || App->renderer->viewport_height != size.y)
		{
			App->camera->ViewportResized((int)size.x, (int)size.y);
			App->renderer->ViewportResized((int)size.x, (int)size.y);
		}

		// Draw
		ImGui::Image((void*)App->renderer->render_texture, size, ImVec2(0, 1), ImVec2(1, 0));

		// Capture input
		if (ImGui::IsWindowFocused())
		{
			ImGui::CaptureKeyboardFromApp(false);
			if (ImGui::IsAnyMouseDown() || ImGui::IsItemHovered())
			{
				ImGui::CaptureMouseFromApp(false);
			}
		}
	}
	ImGui::End();
}
