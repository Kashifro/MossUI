#pragma once

#include "MossUI/core/Types.hpp"

#include <string>

namespace MossUI {

enum class Skin { Panel, ScrollHandle, SliderButton, SliderButtonHover };

struct TextRun {
    Rect rect;
    std::string text;
    Color color;
    float alpha;
    Align align;
    bool shadow;
    float scale = 1.0f;
};

class Backend {
public:
    virtual ~Backend() = default;

    virtual void Fill(const Rect& r, const Color& c, float alpha) = 0;
    virtual bool Image(const Rect& r, const std::string& path) = 0;
    virtual bool ImageTiled(const Rect& r, const std::string& path, float tile) = 0;
    virtual void Preload(const std::string& ) {}
    virtual bool Skinned(const Rect& r, Skin skin) = 0;
    virtual void PushClip(const Rect& ) {}
    virtual void PopClip() {}
    virtual void Text(const TextRun& run) = 0;
    virtual void FlushText() = 0;
    virtual float TextWidth(const std::string& s) = 0;
    virtual std::string ClipboardGet() = 0;
    virtual void ClipboardSet(const std::string& text) = 0;
    virtual void Accessibility(const AccessibilityInfo&) {}
};

class Host {
public:
    virtual ~Host() = default;

    virtual bool ToGui(int rawX, int rawY, float& x, float& y) = 0;
    virtual bool OverHud() = 0;
    virtual void GrabMouse() = 0;
    virtual void ReleaseMouse() = 0;
    virtual bool MouseGrabbed() = 0;
    virtual void ScreenOpened() {}
    virtual void Click() {}
};

}
