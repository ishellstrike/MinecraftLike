#include "WS.h"
#include <Settings.h>
#include "WindowPerfomance.h"
#include "WindowInventory.h"
#include "WindowDb.h"
#include "WindowTools.h"
#include "WindowEventlog.h"
#include "WindowCraftQueue.h"
#include "WindowRecipe.h"
#include "WindowRooms.h"
#include <WindowProfesions.h>

void WS::Draw(glm::vec2 wsize, float gt)
{

	WindowPerfomance &wp = WindowPerfomance::Get();
	WindowInventory &winv = WindowInventory::Get();
	WindowDb &wdb = WindowDb::Get();
	WindowRecipe &wr = WindowRecipe::Get();
	WindowEventlog &we = WindowEventlog::Get();
	WindowTools &wt = WindowTools::Get();
	WindowCraftQueue &wcq = WindowCraftQueue::Get();
	WindowRooms &wro = WindowRooms::Get();
	WindowProfessions &wpr = WindowProfessions::Get();

	if (ImGui::IsKeyPressed(GLFW_KEY_F3))
	{
		wp.Toggle();
	}

	if (ImGui::IsKeyPressed(GLFW_KEY_F4))
	{
		wpr.Toggle();
	}

	if (ImGui::IsKeyPressed(GLFW_KEY_F5))
	{
		we.Toggle();
	}

	if (ImGui::IsKeyPressed(GLFW_KEY_F11))
	{
		Settings::Get().debug = !Settings::Get().debug;
	}

	wp.Draw(wsize, gt);
	wdb.Draw(wsize, gt);
	winv.Draw(wsize, gt);
	wr.Draw(wsize, gt);
	we.Draw(wsize, gt);
	wt.Draw(wsize, gt);
	wcq.Draw(wsize, gt);
	wro.Draw(wsize, gt);
	wpr.Draw(wsize, gt);
}
