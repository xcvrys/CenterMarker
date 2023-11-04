#include "gui.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"

#include <d3d9.h>
#include <chrono>
#include <thread>


#include "window.h"
#include "cursor.h"

// RENDER VARS
bool splash_screen = true;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter
);

long __stdcall WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter)
{
	if (ImGui_ImplWin32_WndProcHandler(window, message, wideParameter, longParameter))
		return true;

	switch (message)
	{
	case WM_SIZE: {
		if (gui::device && wideParameter != SIZE_MINIMIZED)
		{
			gui::presentParameters.BackBufferWidth = LOWORD(longParameter);
			gui::presentParameters.BackBufferHeight = HIWORD(longParameter);
			gui::ResetDevice();
		}
	}return 0;

	case WM_SYSCOMMAND: {
		if ((wideParameter & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
	}break;

	case WM_DESTROY: {
		PostQuitMessage(0);
	}return 0;

	case WM_LBUTTONDOWN: {
		gui::position = MAKEPOINTS(longParameter); // set click points
	}return 0;

	case WM_MOUSEMOVE: {
		if (wideParameter == MK_LBUTTON)
		{
			const auto points = MAKEPOINTS(longParameter);
			auto rect = ::RECT{ };

			GetWindowRect(gui::window, &rect);

			rect.left += points.x - gui::position.x;
			rect.top += points.y - gui::position.y;

			if (gui::position.x >= 0 &&
				gui::position.x <= gui::WIDTH &&
				gui::position.y >= 0 && gui::position.y <= 19)
				SetWindowPos(
					gui::window,
					HWND_TOPMOST,
					rect.left,
					rect.top,
					0, 0,
					SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER
				);
		}

	}return 0;

	}

	return DefWindowProc(window, message, wideParameter, longParameter);
}

void gui::CreateHWindow(const char* windowName) noexcept
{
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_CLASSDC;
	windowClass.lpfnWndProc =
		[](HWND window, UINT message, WPARAM wideParameter, LPARAM longParameter) -> LRESULT {
		return WindowProcess(window, message, wideParameter, longParameter);
		};
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandleA(0);
	windowClass.hIcon = 0;
	windowClass.hCursor = 0;
	windowClass.hbrBackground = 0;
	windowClass.lpszMenuName = 0;
	windowClass.lpszClassName = "class001";
	windowClass.hIconSm = 0;
	RegisterClassEx(&windowClass);

	window = CreateWindowEx(
		0,
		"class001",
		windowName,
		WS_POPUP,
		100,
		100,
		WIDTH,
		HEIGHT,
		0,
		0,
		windowClass.hInstance,
		0
	);

	HICON hIcon = (HICON)LoadImage(GetModuleHandle(NULL), "icon.ico", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);

	if (hIcon) {
		SendMessage(window, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		SendMessage(window, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	}

	ShowWindow(window, SW_SHOWDEFAULT);
	UpdateWindow(window);
}


void gui::DestroyHWindow() noexcept
{
	DestroyWindow(window);
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

bool gui::CreateDevice() noexcept
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d)
		return false;

	ZeroMemory(&presentParameters, sizeof(presentParameters));

	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameters.EnableAutoDepthStencil = TRUE;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&presentParameters,
		&device) < 0)
		return false;

	return true;
}

void gui::ResetDevice() noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = device->Reset(&presentParameters);

	if (result == D3DERR_INVALIDCALL)
		IM_ASSERT(0);

	ImGui_ImplDX9_CreateDeviceObjects();
}

void gui::DestroyDevice() noexcept
{
	if (device)
	{
		device->Release();
		device = nullptr;
	}

	if (d3d)
	{
		d3d->Release();
		d3d = nullptr;
	}
}

void gui::CreateImGui() noexcept
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ::ImGui::GetIO();

	io.IniFilename = NULL;

	// Set the background color in the ImGui style
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] = ImVec4(14.0f / 255.0f, 17.0f / 255.0f, 22.0f / 255.0f, 1.0f);

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);
}


void gui::DestroyImGui() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void gui::BeginRender() noexcept
{
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);

		if (message.message == WM_QUIT)
		{
			isRunning = !isRunning;
			return;
		}
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void gui::EndRender() noexcept
{
	ImGui::EndFrame();

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(14, 17, 22, 255), 1.0f, 0);

	if (device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}

	const auto result = device->Present(0, 0, 0, 0);

	// Handle loss of D3D9 device
	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
}


void RenderCenterMarkerText()
{
	ImVec2 windowSize = ImGui::GetWindowSize();

	const char* centerMarkerText =
		"  ___         _           __  __          _           \n"
		" / __|___ _ _| |_ ___ _ _|  \\/  |__ _ _ _| |_____ _ _ \n"
		"| (__/ -_) ' \\  _/ -_) '_| |\\/| / _` | '_| / / -_) '_|\n"
		" \\___\\___|_||_\\__\\___|_| |_|  |\\__,_|_| |_|\\_\\___|_|\n";

	ImVec2 textSize = ImGui::CalcTextSize(centerMarkerText);
	ImVec2 centerPosition = ImVec2((windowSize.x - textSize.x) * 0.5f, (windowSize.y - textSize.y) * 0.5f);

	ImGui::SetCursorScreenPos(centerPosition);
	ImGui::TextUnformatted(centerMarkerText);
}

void gui::SplashScreen() noexcept
{
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ WIDTH, HEIGHT });
	ImGui::Begin(
		"CenterMarkerSplashScreen",
		&isRunning,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBackground
	);

	RenderCenterMarkerText();

	ImGui::End();
}

float radius = 0.0f;


void gui::Render() noexcept
{
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ WIDTH, HEIGHT });
	ImGui::Begin(
		"CenterMarker",
		&isRunning,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse
	);

	ImGui::BeginTabBar("#Additional Parameters");
	if (ImGui::BeginTabItem("Center Marker")) {
	/*	ImGui::EndTabItem();
	}*/

	//if (ImGui::BeginTabItem("Color Picker")) {

		static bool showColorMessage = false;
		static float colorMessageTimer = 0.0f;

		ImGui::Text("Select Center Marker Color");
		if (ImGui::ColorEdit3("##CenterMarkerColor", (float*)&cursor::Color)) {
			showColorMessage = true;
			colorMessageTimer = 3.0f;
		}

		if (ImGui::ColorButton("##ColoButtonWhile", ImVec4(1.0f, 1.0f, 1.0f, 1.f), ImGuiColorEditFlags_NoPicker)) {
			cursor::Color = ImVec4(1.0f, 1.0f, 1.0f, 1.f);
			showColorMessage = true;
			colorMessageTimer = 3.0f;
		}
		ImGui::SameLine();

		if (ImGui::ColorButton("##ColoButtonRed", ImVec4(0.8f, 0.0f, 0.0f, 1.f), ImGuiColorEditFlags_NoPicker)) {
			cursor::Color = ImVec4(0.8f, 0.0f, 0.0f, 1.f);
			showColorMessage = true;
			colorMessageTimer = 3.0f;
		}
		ImGui::SameLine();

		if (ImGui::ColorButton("##ColoButtonGreen", ImVec4(0.0f, 0.8f, 0.0f, 1.f), ImGuiColorEditFlags_NoPicker)) {
			cursor::Color = ImVec4(0.0f, 0.8f, 0.0f, 1.f);
			showColorMessage = true;
			colorMessageTimer = 3.0f;
		}
		ImGui::SameLine();

		if (ImGui::ColorButton("##ColoButtonBlue", ImVec4(0.0f, 0.0f, 0.8f, 1.f), ImGuiColorEditFlags_NoPicker)) {
			cursor::Color = ImVec4(0.0f, 0.0f, 0.8f, 1.f);
			showColorMessage = true;
			colorMessageTimer = 3.0f;
		}

		if (0) {
			if (colorMessageTimer > 0) {
				colorMessageTimer -= ImGui::GetIO().DeltaTime;
				int r = static_cast<int>(cursor::Color.x * 255.0f);
				int g = static_cast<int>(cursor::Color.y * 255.0f);
				int b = static_cast<int>(cursor::Color.z * 255.0f);

				ImGui::TextColored(ImVec4(cursor::Color.x, cursor::Color.y, cursor::Color.z, 1.f), "Selected Color (RGB): %d, %d, %d", r, g, b);
				ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize("0.00").x);
				ImGui::TextColored(ImVec4(cursor::Color.x, cursor::Color.y, cursor::Color.z, 1.f), "%.2f", colorMessageTimer);
			}
			else {
				showColorMessage = false;
			}
		}

		ImGui::Text("Main");

		ImGui::Checkbox("Show Dot", &cursor::ShowDot);
		if (cursor::ShowDot) {
			ImGui::SetCursorPosX(15);
			ImGui::SliderFloat("##CenterMarkerDotRadius", (float*)&cursor::DotRadius, 0.01f, 0.25f, "Radius: %.2f");
		}

		ImGui::Checkbox("Show Circle", &cursor::ShowCircle);
		if (cursor::ShowCircle) {
			ImGui::SetCursorPosX(15);
			ImGui::SliderFloat("##CenterMarkerCirclePosition", (float*)&cursor::CircleRadius, 0.01f, .95f, "Radius: %.2f");
			ImGui::SetCursorPosX(15);
			ImGui::SliderFloat("##CenterMarkerCircleThickness", (float*)&cursor::CircleThickness, 0.01f, 3.f, "Thickness: %.2f");
		}

		ImGui::Checkbox("Show Crosshair", &cursor::Showcross);
		if (cursor::Showcross) {
			ImGui::SetCursorPosX(15);
			ImGui::SliderFloat("##CenterMarkerCrosshairPosition", (float*)&cursor::CrossSize, 0.01f, 1.f, "CrossSize: %.2f");
			ImGui::SetCursorPosX(15);
			ImGui::SliderFloat("##CenterMarkerCrosshairThickness", (float*)&cursor::CrossThickness, 0.01f, 10.f, "CrossThickness: %.2f");
			ImGui::SetCursorPosX(15);
			ImGui::SliderFloat("##CenterMarkerCrosshairSpacing", (float*)&cursor::CrossSpacing, 0.01f, 1.f, "CrossSpacing: %.2f");
		}

		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("Info")) {
		ImGui::Text("This software is provided 'as-is' without any warranty.");
		ImGui::Text("");
		ImGui::Text("Created by:");
		ImGui::SameLine();

		if (ImGui::Button("xcvrys")) {
			const char* url = "https://github.com/xcvrys";
			#ifdef _WIN32
			ShellExecuteA(nullptr, "open", url, nullptr, nullptr, SW_SHOWNORMAL);
			#endif
		}
		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();
	ImGui::End();
}


