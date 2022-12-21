#pragma once

#include "raylib.h"
#include "imgui.h"

namespace rlImGuiColors
{
    inline ImVec4 Convert(::Color color)
    {
        return ImVec4(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
    }
}