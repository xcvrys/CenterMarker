#pragma once
#include <chrono>
#include <GLFW/glfw3.h>
#include "../imgui/imgui.h"

namespace window
{
	inline constexpr int WIDTH = 200;
	inline constexpr int HEIGHT = 200;
	inline constexpr const char* WINDOW_NAME = "CenterMarkerCursor";

	inline GLFWwindow* window = nullptr;

	inline std::thread windowThread;
	
	void start() noexcept;

	inline bool clickable = false;

	int mainWindow() noexcept;
	void configWindowHints() noexcept;
	void centerWindow(GLFWwindow* window) noexcept;
}
