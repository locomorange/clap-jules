// imgui_impl_skia.h
#pragma once
#include "imgui.h" // IMGUI_API

// Forward declarations
class SkCanvas;
struct SkFont; // Or whatever Skia font representation is best
class SkSurface; // For font atlas

IMGUI_IMPL_API bool     ImGui_ImplSkia_Init();
IMGUI_IMPL_API void     ImGui_ImplSkia_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplSkia_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplSkia_RenderDrawData(ImDrawData* draw_data, SkCanvas* canvas);

// Called by Init/NewFrame/Shutdown
IMGUI_IMPL_API bool     ImGui_ImplSkia_CreateFontsTexture();
IMGUI_IMPL_API void     ImGui_ImplSkia_DestroyFontsTexture();
IMGUI_IMPL_API bool     ImGui_ImplSkia_CreateDeviceObjects();
IMGUI_IMPL_API void     ImGui_ImplSkia_DestroyDeviceObjects();
