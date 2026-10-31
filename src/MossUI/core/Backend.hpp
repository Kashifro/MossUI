#pragma once

#include "MossUI/core/NineSlice.hpp"
#include "MossUI/core/Types.hpp"

#include <filesystem>
#include <string>

namespace MossUI {

struct TextRun {
    Rect rect;
    std::string text;
    Color color;
    float alpha;
    Align align;
    bool shadow;
    float scale = 1.0f;
};

// What MossUI draws with. Textured drawing is ONE primitive (Quad); Image, tiling, nine-slice and skins are built on it
// in MossUI::UI, so a backend never reimplements them. Most mods should not implement this by hand: derive from
// RenderApi (MossUI/render/RenderBackend.hpp) instead and get texture caching, clipping and the skin logic for free.
class Backend {
public:
    virtual ~Backend() = default;

    virtual void Fill(const Rect& r, const Color& c, float alpha) = 0;

    // Queues one textured quad. False = the texture is not available (missing, or still settling): nothing is queued.
    virtual bool Quad(const Rect& r, const std::string& path, const Uv& uv) = 0;
    // Draws the queued quads multiplied by `tint`.
    virtual void FlushQuads(const Color& tint) = 0;
    virtual void Preload(const std::string&) {}

    virtual void PushClip(const Rect&) {}
    virtual void PopClip() {}
    virtual void Text(const TextRun& run) = 0;
    virtual void FlushText() = 0;
    virtual float TextWidth(const std::string& s) = 0;
    virtual void Accessibility(const AccessibilityInfo&) {}
};

// What MossUI needs from the platform around the renderer: pointer mapping, mouse grab, clipboard, launching.
class Host {
public:
    virtual ~Host() = default;

    virtual bool ToGui(int rawX, int rawY, float& x, float& y) = 0;
    virtual bool OverHud() = 0;
    virtual void GrabMouse() = 0;
    virtual void ReleaseMouse() = 0;
    virtual bool MouseGrabbed() = 0;
    virtual std::string ClipboardGet() = 0;
    virtual void ClipboardSet(const std::string& text) = 0;

    virtual void ScreenOpened() {}
    virtual void Click() {}
    virtual void OpenUri(const std::string&) {}                 // http(s) link
    virtual void OpenFolder(const std::filesystem::path&) {}    // a directory
};

}
