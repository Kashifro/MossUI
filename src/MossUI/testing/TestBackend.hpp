#pragma once

#include "MossUI/core/Backend.hpp"

#include <set>
#include <vector>

namespace MossUI::testing {

class TestBackend final : public Backend {
public:
    struct FillCall {
        Rect rect;
        Color color;
        float alpha;
    };

    void Fill(const Rect& rect, const Color& color, float alpha) override { fills.push_back({rect, color, alpha}); }
    bool Image(const Rect&, const std::string& path) override { return availableImages.contains(path); }
    bool ImageTiled(const Rect&, const std::string& path, float) override { return availableImages.contains(path); }
    bool Skinned(const Rect&, Skin) override { return false; }
    void PushClip(const Rect& rect) override {
        ++clipDepth;
        clipRects.push_back(rect);
        activeClips.push_back(rect);
    }
    void PopClip() override { --clipDepth; activeClips.pop_back(); }
    void Text(const TextRun& run) override {
        texts.push_back(run);
        textClipDepths.push_back(clipDepth);
        textClipRects.push_back(activeClips.empty() ? Rect{0, 0, 0, 0} : activeClips.back());
    }
    void FlushText() override {}
    float TextWidth(const std::string& text) override { return static_cast<float>(text.size()) * 6.0f; }
    std::string ClipboardGet() override { return clipboard; }
    void ClipboardSet(const std::string& value) override { clipboard = value; }

    std::vector<FillCall> fills;
    std::vector<TextRun> texts;
    std::vector<Rect> clipRects;
    std::vector<int> textClipDepths;
    std::vector<Rect> textClipRects;
    std::vector<Rect> activeClips;
    std::set<std::string> availableImages;
    std::string clipboard;
    int clipDepth = 0;
};

class TestHost final : public Host {
public:
    bool ToGui(int rawX, int rawY, float& x, float& y) override {
        x = static_cast<float>(rawX);
        y = static_cast<float>(rawY);
        return true;
    }
    bool OverHud() override { return false; }
    void GrabMouse() override {}
    void ReleaseMouse() override {}
    bool MouseGrabbed() override { return false; }
    void Click() override { ++clicks; }

    int clicks = 0;
};

}
