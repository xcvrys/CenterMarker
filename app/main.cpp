#include "gui.h"
#include <thread>
#include <GLFW/glfw3.h>
#include "window.h"


int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
) {
    // Create gui
    gui::CreateHWindow("CenterMarker");
    gui::CreateDevice();
    gui::CreateImGui();

    gui::splashScreenStartTime = std::chrono::steady_clock::now();

    std::thread(window::mainWindow).detach();

    while (gui::isRunning) {
        gui::BeginRender();

        if (gui::splashScreen) {
            auto currentTime = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - gui::splashScreenStartTime).count() >= gui::SPLASH_SCREEN_DURATION) {
                gui::splashScreen = false;
            }

            gui::SplashScreen();
        }
        else {
            gui::Render();
        }
        gui::EndRender();
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }

    // Destroy gui
    gui::DestroyImGui();
    gui::DestroyDevice();
    gui::DestroyHWindow();

    return EXIT_SUCCESS;
}
