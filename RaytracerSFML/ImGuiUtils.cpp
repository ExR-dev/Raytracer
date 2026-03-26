#include "ImGuiUtils.h"
#include <functional>
#include <string>

static void WrapMousePosEx(int axises_mask, const ImRect& wrap_rect)
{
	ImGuiContext& g = *GImGui;
	IM_ASSERT(axises_mask == 1 || axises_mask == 2 || axises_mask == (1 | 2));
	ImVec2 p_mouse = g.IO.MousePos;
	for (int axis = 0; axis < 2; axis++)
	{
		if ((axises_mask & (1 << axis)) == 0)
			continue;
		if (p_mouse[axis] >= wrap_rect.Max[axis])
			p_mouse[axis] = wrap_rect.Min[axis] + 1.0f;
		else if (p_mouse[axis] <= wrap_rect.Min[axis])
			p_mouse[axis] = wrap_rect.Max[axis] - 1.0f;
	}
	if (p_mouse.x != g.IO.MousePos.x || p_mouse.y != g.IO.MousePos.y)
		ImGui::TeleportMousePos(p_mouse);
}

static void WrapMousePos(int axises_mask)
{
	ImGuiContext& g = *GImGui;
#ifdef IMGUI_HAS_DOCK
	if (g.IO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		const ImGuiPlatformMonitor* monitor = ImGui::GetViewportPlatformMonitor(g.MouseViewport);
		WrapMousePosEx(axises_mask, ImRect(monitor->MainPos, monitor->MainPos + monitor->MainSize - ImVec2(1.0f, 1.0f)));
	}
	else
#endif
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		WrapMousePosEx(axises_mask, ImRect(viewport->Pos, viewport->Pos + viewport->Size - ImVec2(1.0f, 1.0f)));
	}
}

void ImGuiUtils::LockMouseOnActive()
{
	return; // TEMP: Disabled

	static ImGuiID target_item = 0;
	static sf::Vector2i  prev_mous_pos;

	ImGuiContext& g = *GImGui;
	ImGuiID id = ImGui::GetItemID();

	if (ImGui::IsItemActive() && (!ImGui::GetInputTextState(id) || g.InputTextDeactivatedState.ID == id))
	{
		if (target_item == 0)
		{
			target_item = id;
			prev_mous_pos = sf::Mouse::getPosition();
		}
		WrapMousePos(1 << ImGuiAxis_X);
		ImGui::SetMouseCursor(ImGuiMouseCursor_None);
	}
	else if (target_item > 0 && target_item == id && (ImGui::IsItemDeactivated() || g.InputTextDeactivatedState.ID != id))
	{
		sf::Mouse::setPosition(prev_mous_pos);
		target_item = 0;
	}
}