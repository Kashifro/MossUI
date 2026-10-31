#pragma once

#include "MossUI/core/Backend.hpp"
#include "MossUI/core/NineSlice.hpp"
#include "MossUI/core/Types.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

namespace MossUI {

void SetClipboardText(const std::string& text);

// A named nine-slice texture. Widgets look skins up by name, so a mod (or a resource pack's loader) can restyle the UI
// with SetSkin(skin::PANEL, {...}) without touching a backend.
struct SkinDef { std::string path; Slice slice; };
namespace skin {
inline constexpr const char* PANEL = "panel";
inline constexpr const char* SCROLL_HANDLE = "scroll_handle";
inline constexpr const char* SLIDER_BUTTON = "slider_button";
inline constexpr const char* SLIDER_BUTTON_HOVER = "slider_button_hover";
}
void SetSkin(const std::string& name, SkinDef def);
const SkinDef* FindSkin(const std::string& name);
void ResetSkins();
void SetAssetDir(std::filesystem::path dir);
std::string AssetPath(const std::string& name);

struct ScrollState {
    int pos = 0, max = 0;
    Rect rail{0, 0, 0, 0};
    float handleH = 0;
    bool drag = false;
    float visualPos = 0.0f;
    std::chrono::steady_clock::time_point lastUpdate{};
    bool visualInitialized = false;

    float At() const { return visualInitialized ? visualPos : static_cast<float>(pos); }

    bool Grab(class UI& ui);

    void Reveal(int idx, int fit) {
        if (idx < pos) pos = idx;
        else if (idx >= pos + fit) pos = idx - fit + 1;
        pos = std::clamp(pos, 0, std::max(0, max));
    }

    void Step(int delta) { pos = std::clamp(pos + delta, 0, std::max(0, max)); }

    float UpdateVisualPosition() {
        pos = std::clamp(pos, 0, max);
        const auto now = std::chrono::steady_clock::now();
        if (!visualInitialized) {
            visualPos = static_cast<float>(pos);
            visualInitialized = true;
        } else if (drag) {
            visualPos = static_cast<float>(pos);
        } else {
            const float elapsed = std::chrono::duration<float>(now - lastUpdate).count();
            const float blend = 1.0f - std::exp(-CurrentWidgetMetrics().scrollSmoothness * elapsed);
            visualPos += (static_cast<float>(pos) - visualPos) * blend;
            if (std::abs(visualPos - static_cast<float>(pos)) < 0.01f)
                visualPos = static_cast<float>(pos);
        }
        lastUpdate = now;
        visualPos = std::clamp(visualPos, 0.0f, static_cast<float>(max));
        return visualPos;
    }

    void Reset() {
        pos = 0;
        visualPos = 0.0f;
        visualInitialized = true;
        lastUpdate = std::chrono::steady_clock::now();
    }
};

struct TextFieldState {
    std::string text;
    size_t maxLen = 64;
    bool focused = false;
    int caret = 0, anchor = 0;
    int viewStart = 0;
    bool drag = false;
    Rect rect{0, 0, 0, 0};
    float textX0 = 0;
    std::vector<float> bounds;
};

class UI {
public:
    float Width() const;
    float Height() const;
    float MouseX() const;
    float MouseY() const;
    bool HasMouse() const;
    bool MouseDown() const;
    bool Hovered(const Rect& r) const;
    void BlockHover(bool blocked = true);
    bool HoverBlocked() const;
    bool InGame() const;
    void Click();

    void Fill(const Rect& r, const Color& c, float alpha = 1.0f);
    void Outline(const Rect& r, const Color& c, float alpha = 1.0f);
    void Backdrop(float alpha = 0.9f);
    void Panel(const Rect& r);
    void PushClip(const Rect& r);
    void PopClip();
    bool Image(const Rect& r, const std::string& path, const Color& tint = col::WHITE);
    bool DrawGlyph(Rect r, const std::string& name, bool enabled, bool hov, bool down);
    void Preload(const std::string& path);
    bool ImageTiled(const Rect& r, const std::string& path, float tile = 32.0f, const Color& tint = col::WHITE);
    bool NineSlice(const Rect& r, const std::string& path, const Slice& slice, const Color& tint = col::WHITE);
    bool Skinned(const Rect& r, const std::string& skinName, const Color& tint = col::WHITE);
    void Text(float x0, float x1, float y, const std::string& s, const Color& c, Align align, float alpha = 0.9f,
              bool shadow = true);
    void LinkText(float x, float maxX, float y, const std::string& text, const Color& color, bool hovered, bool pressed);
    float TextWidth(const std::string& s) const;
    std::string Trunc(const std::string& s, float width) const;
    std::vector<std::string> Wrap(const std::string& s, float width) const;
    void Tooltip(const std::string& text);

    void DrawButton(const Rect& r, const std::string& label, bool enabled, bool hovered, bool pressed);
    void DrawDarkButton(const Rect& r, const std::string& label, bool hovered, const Color* outline = nullptr);
    static Rect SwitchHit(const Rect& row);
    void DrawSwitch(const Rect& row, bool on);
    static Rect SliderHit(const Rect& area);
    void DrawSlider(const Rect& area, float value, float lo, float hi, float step, bool hovered);
    void DrawScrollbar(ScrollState& st, const Rect& rail, int visible, int total);
    void DrawTextField(TextFieldState& st, const Rect& r, const std::string& placeholder);
};

inline bool ScrollState::Grab(UI& ui) {
    drag = max > 0 && rail.x1 > rail.x0 && ui.Hovered({rail.x0 - 2.0f, rail.x1 + 2.0f, rail.y0, rail.y1});
    return drag;
}

class Canvas {
public:
    Canvas(UI& ui, const Rect& bounds);
    ~Canvas();

    Canvas(const Canvas&) = delete;
    Canvas& operator=(const Canvas&) = delete;

private:
    UI* ui;
};

}
