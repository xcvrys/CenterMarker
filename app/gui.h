#pragma once
#include <d3d9.h>
#include <chrono>

namespace gui
{
	// constant window size
	constexpr int WIDTH = 500;
	constexpr int HEIGHT = 700;

	// when this changes, exit threads
	// and close menu :)
	inline bool isRunning = true;

	// winapi window vars
	inline HWND window = nullptr;
	inline WNDCLASSEX windowClass = { };

	// points for window movement
	inline POINTS position = { };

	// direct x state vars
	inline PDIRECT3D9 d3d = nullptr;
	inline LPDIRECT3DDEVICE9 device = nullptr;
	inline D3DPRESENT_PARAMETERS presentParameters = { };

	// handle window creation & destruction
	void CreateHWindow(const char* windowName) noexcept;
	void DestroyHWindow() noexcept;

	// handle device creation & destruction
	bool CreateDevice() noexcept;
	void ResetDevice() noexcept;
	void DestroyDevice() noexcept;

	// handle ImGui creation & destruction
	void CreateImGui() noexcept;
	void DestroyImGui() noexcept;

	void BeginRender() noexcept;
	void EndRender() noexcept;
	void Render() noexcept;

	// handle splash screen
	void SplashScreen() noexcept;
	inline bool splashScreen = true;
	inline std::chrono::time_point<std::chrono::steady_clock> splashScreenStartTime;
	//constexpr int SPLASH_SCREEN_DURATION = 2500; // Normal duration
	constexpr int SPLASH_SCREEN_DURATION = 500; // Dev duration
}
