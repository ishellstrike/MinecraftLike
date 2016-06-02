#include "WindowRooms.h"
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <Core\World.h>

WindowRooms::WindowRooms()
{
}

void WindowRooms::Draw(glm::vec2 wsize)
{
	ImGui::Begin("Room managment", &mOpen);
	if (selected)
	{
		char buf[100] = { 0 };
		strcpy_s(buf, &selected->name[0]);
		ImGui::InputText("Name", buf, 100);
		selected->name = buf;
		if (ImGui::Button("Remove"))
		{
			w->rooms.remove(selected);
			selected = nullptr;
		}
	}
	ImGui::End();
}
