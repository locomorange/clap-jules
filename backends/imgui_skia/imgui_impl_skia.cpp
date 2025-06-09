// imgui_impl_skia.cpp
#include "imgui.h"
#include "imgui_impl_skia.h"

#include <core/SkCanvas.h>
#include <core/SkPaint.h>
#include <core/SkFont.h>
#include <core/SkImage.h>
#include <core/SkData.h>
#include <core/SkSurface.h>
#include <core/SkPixmap.h>
#include <core/SkColorSpace.h>
#include <core/SkBitmap.h> // For font atlas

// Data
static SkFont* g_Font = nullptr; // Example: Skia font object
static sk_sp<SkImage> g_FontTexture; // Skia representation of the font texture

bool ImGui_ImplSkia_Init() {
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendRendererUserData == NULL && "Already initialized a renderer backend!");
    io.BackendRendererUserData = (void*)nullptr; // Can store backend-specific data here
    io.BackendRendererName = "imgui_impl_skia";
    // io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset; // If backend supports VtxOffset

    // Create device objects (e.g., font texture)
    return ImGui_ImplSkia_CreateDeviceObjects();
}

void ImGui_ImplSkia_Shutdown() {
    ImGui_ImplSkia_DestroyDeviceObjects();
    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;
}

void ImGui_ImplSkia_NewFrame() {
    if (!g_FontTexture) {
        ImGui_ImplSkia_CreateFontsTexture();
    }
}

void ImGui_ImplSkia_RenderDrawData(ImDrawData* draw_data, SkCanvas* canvas) {
    if (!canvas) return;

    // Avoid rendering when minimized
    if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
        return;

    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
    // (Skia usually handles blending per paint)

    // Setup viewport, orthographic projection matrix
    // canvas->save();
    // canvas->scale(draw_data->FramebufferScale.x, draw_data->FramebufferScale.y);
    // canvas->translate(draw_data->DisplayPos.x, draw_data->DisplayPos.y); // Or handle offset manually

    // Will project scissor/clipping rectangles into framebuffer space
    // SkRect clip_rect;
    // clip_rect.setXYWH(draw_data->DisplayPos.x, draw_data->DisplayPos.y, draw_data->DisplaySize.x, draw_data->DisplaySize.y);
    // canvas->clipRect(clip_rect, SkClipOp::kIntersect, true);


    ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
        const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback) {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    ;// ImGui_ImplSkia_SetupRenderState(draw_data, canvas); // Or however Skia state is managed
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            } else {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec4 clip_rect_vec;
                clip_rect_vec.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
                clip_rect_vec.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
                clip_rect_vec.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
                clip_rect_vec.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

                if (clip_rect_vec.x < draw_data->DisplaySize.x * clip_scale.x && clip_rect_vec.y < draw_data->DisplaySize.y * clip_scale.y &&
                    clip_rect_vec.z >= 0.0f && clip_rect_vec.w >= 0.0f) {
                    canvas->save();
                    SkRect sk_clip_rect = SkRect::MakeLTRB(clip_rect_vec.x, clip_rect_vec.y, clip_rect_vec.z, clip_rect_vec.w);
                    canvas->clipRect(sk_clip_rect, SkClipOp::kIntersect, true);

                    // SkPaint paint;
                    // paint.setAntiAlias(true);
                    // Get SkImage from TextureID
                    // sk_sp<SkImage>* tex = (sk_sp<SkImage>*)pcmd->GetTexID();
                    // if (tex && (*tex)) {
                    //    paint.setShader( (*tex)->makeShader(SkSamplingOptions(SkFilterMode::kLinear)) );
                    // }


                    // Draw triangles
                    // This is a simplified example. Need to handle vertices, colors, UVs correctly.
                    // For Skia, skia_pathops::Op or drawVertices might be used.
                    // A common way is to build SkVertices object.
                    /*
                    std::vector<SkPoint> positions;
                    std::vector<SkColor> colors;
                    std::vector<SkPoint> texCoords; // if textured

                    for (unsigned int i = 0; i < pcmd->ElemCount; ++i) {
                        const ImDrawVert& v = vtx_buffer[idx_buffer[i]];
                        positions.push_back(SkPoint::Make(v.pos.x, v.pos.y));
                        colors.push_back(v.col); // ImU32 to SkColor
                        if (pcmd->TextureId != nullptr) {
                            texCoords.push_back(SkPoint::Make(v.uv.x, v.uv.y));
                        }
                    }

                    auto vertices = SkVertices::MakeCopy(
                        SkVertices::kTriangles_VertexMode,
                        positions.size(),
                        positions.data(),
                        texCoords.empty() ? nullptr : texCoords.data(),
                        colors.data()
                    );
                    canvas->drawVertices(vertices, SkBlendMode::kModulate, paint);
                    */
                    canvas->restore(); // clip
                }
            }
            // Advance idx_buffer (if not using ElemCount to index)
            // idx_buffer += pcmd->ElemCount; // This is wrong if ElemCount is not used as an offset
        }
    }
    // canvas->restore(); // Initial save
    // IMGUI_DEBUG_LOG("Rendered %d draw lists, %d cmds, %d vtx, %d idx
", draw_data->CmdListsCount, total_cmds, total_vtx, total_idx);
}


bool ImGui_ImplSkia_CreateFontsTexture() {
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    if (width == 0 || height == 0) return false;

    SkImageInfo image_info = SkImageInfo::Make(width, height, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkPixmap pixmap(image_info, pixels, image_info.minRowBytes());

    // We transfer ownership of the font data to Skia here.
    // If ImGui manages the font data elsewhere and it might be freed,
    // then we should use SkImage::MakeRasterData which copies.
    // Or, ensure ImGui keeps font data alive. For GetTexDataAsRGBA32, ImGui owns it.
    // So, a copy is safer.
    g_FontTexture = SkImage::MakeRasterData(image_info, SkData::MakeWithCopy(pixels, pixmap.computeByteSize()));
    if (!g_FontTexture) return false;

    io.Fonts->SetTexID((ImTextureID)&g_FontTexture);
    // IMGUI_DEBUG_LOG("Created font texture %dx%d
", width, height);
    return true;
}

void ImGui_ImplSkia_DestroyFontsTexture() {
    ImGuiIO& io = ImGui::GetIO();
    if (g_FontTexture) {
        // IMGUI_DEBUG_LOG("Destroying font texture
");
        g_FontTexture.reset();
        io.Fonts->SetTexID(0);
    }
}

bool ImGui_ImplSkia_CreateDeviceObjects() {
    // For Skia, most "device objects" are managed by Skia itself or are simple,
    // like the font texture. Shaders or complex paints could be pre-created here if needed.
    return ImGui_ImplSkia_CreateFontsTexture();
}

void ImGui_ImplSkia_DestroyDeviceObjects() {
    ImGui_ImplSkia_DestroyFontsTexture();
}
