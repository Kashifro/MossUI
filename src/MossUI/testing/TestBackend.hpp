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
    bool Quad(const Rect& rect, const std::string& path, const Uv& uv) override {
        if (!availableImages.contains(path)) return false;
        quads.push_back({rect, path, uv});
        return true;
    }
    void FlushQuads(const Color& tint) override { flushes.push_back(tint); }
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

    struct QuadCall {
        Rect rect;
        std::string path;
        Uv uv;
    };

    std::vector<FillCall> fills;
    std::vector<QuadCall> quads;
    std::vector<Color> flushes;
    std::vector<TextRun> texts;
    std::vector<Rect> clipRects;
    std::vector<int> textClipDepths;
    std::vector<Rect> textClipRects;
    std::vector<Rect> activeClips;
    std::set<std::string> availableImages;
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
    std::string ClipboardGet() override { return clipboard; }
    void ClipboardSet(const std::string& value) override { clipboard = value; }
    void OpenUri(const std::string& uri) override { opened.push_back(uri); }

    int clicks = 0;
    std::string clipboard;
    std::vector<std::string> opened;
};

}
