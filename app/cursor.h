#pragma once
#include "../imgui/imgui.h"

namespace cursor {
    inline bool ShowDot = false;
    inline float DotRadius = 0.1f;

    inline bool ShowCircle = false;
    inline float CircleRadius = 1.f;
    inline float CircleThickness = 1.5f;

    inline float Alpha = 1.f;

    inline bool Showcross = false;
    inline float CrossSize = .12f;
    inline float CrossSpacing = .06f;
    inline float CrossThickness = 1.5f;


    inline ImVec4 Color = ImVec4(0.08f, 0.946f, 0.189f, .5f);
}
