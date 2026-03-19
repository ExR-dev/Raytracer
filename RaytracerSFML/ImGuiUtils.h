#pragma once
#include "imgui.h"
#include <imgui_internal.h>
#include "misc/cpp/imgui_stdlib.h"
#include "imgui-SFML.h"
#include <SFML/Window/Mouse.hpp>

namespace ImGuiUtils
{
	static int windowPosX, windowPosY;

	void LockMouseOnActive();
}
