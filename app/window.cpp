#define GLFW_EXPOSE_NATIVE_WIN32
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <Windows.h>
#include "window.h"
#include "cursor.h"

#include <chrono>

HWND findWindow(const char* windowTitle) {
    return FindWindow(0, windowTitle);
}

void setWindowExStyles(HWND hwnd, DWORD exStyles) {
    DWORD currentStyles = GetWindowLong(hwnd, GWL_EXSTYLE);
    SetWindowLong(hwnd, GWL_EXSTYLE, currentStyles | exStyles);
}

void renderDot() {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0f, 0.0f);
    for (int x = 0; x <= 360; x++) {
        float DotdegInRad = x * 3.14159 / 180;
        glVertex2f(cos(DotdegInRad) * cursor::DotRadius, sin(DotdegInRad) * cursor::DotRadius);
    }
    glEnd();
}

void renderCircle() {
    glLineWidth(cursor::CircleThickness);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; i++) {
        float CircledegInRad = i * 3.14159 / 180;
        float x = cos(CircledegInRad) * cursor::CircleRadius;
        float y = sin(CircledegInRad) * cursor::CircleRadius;
        glVertex2f(x, y);
    }
    glEnd();
}

void renderCross() {
    glLineWidth(cursor::CrossThickness);
    glBegin(GL_LINES);

    glVertex2f(-cursor::CrossSize, 0.0f);
    glVertex2f(-cursor::CrossSpacing, 0.0f);

    glVertex2f(cursor::CrossSpacing, 0.0f);
    glVertex2f(cursor::CrossSize, 0.0f);

    glVertex2f(0.0f, -cursor::CrossSize);
    glVertex2f(0.0f, -cursor::CrossSpacing);

    glVertex2f(0.0f, cursor::CrossSpacing);
    glVertex2f(0.0f, cursor::CrossSize);

    glEnd();

    glLineWidth(1.0f);
}

int window::mainWindow() noexcept {
    if (!glfwInit()) {
        return -1;
    }

    window::configWindowHints();

    window = glfwCreateWindow(window::WIDTH, window::HEIGHT, window::WINDOW_NAME, nullptr, nullptr);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!window::clickable) {
        HWND hwnd = glfwGetWin32Window(window);
        SetLayeredWindowAttributes(hwnd, 0, 1, LWA_ALPHA);

        SetWindowLong(hwnd, GWL_EXSTYLE, (GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW) & ~WS_EX_APPWINDOW);
        SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER);

    }

    centerWindow(window);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        glColor4f(cursor::Color.x, cursor::Color.y, cursor::Color.z, cursor::Alpha);

        if (cursor::ShowDot && cursor::DotRadius > 0) {
            renderDot();
        }
        if (cursor::ShowCircle && cursor::CircleRadius > 0) {
            renderCircle();
        }

        if (cursor::Showcross) {
            renderCross();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(35));
    }
    glfwTerminate();
    return 0;
}



void window::configWindowHints() noexcept {
    if (!window::clickable) {
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    }
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
}


void window::centerWindow(GLFWwindow* window) noexcept {
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwSetWindowPos(window, (mode->width - window::WIDTH) / 2, (mode->height - window::HEIGHT) / 2);
}

void window::start() noexcept {
    windowThread = std::thread(mainWindow);
    windowThread.detach();
}

