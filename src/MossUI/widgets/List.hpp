#pragma once

#include "MossUI/widgets/Widget.hpp"

#include <chrono>

namespace MossUI {

class ListWidget : public Container {
public:
    ListWidget() = default;

    void SetBackground(const Color& color, float alpha = 1.0f, bool outline = true) {
        background = true; bgColor = color; bgAlpha = alpha; bgOutline = outline;
    }
    void SetRowLine(const Color& color, float alpha = 1.0f) { rowLine = true; lineColor = color; lineAlpha = alpha; }
    void SetColumns(int n) { columns = n < 1 ? 1 : n; }
    void ResetScroll() { scroll.Reset(); cursor = -1; }
    float ScrollPosition() const { return static_cast<float>(scroll.pos); }
    void SetScrollPosition(float position) { scroll.pos = std::max(0, static_cast<int>(position)); }
    bool NavKey(int vk);
    int Cursor() const { return cursor; }
    void SetCursor(int i) { cursor = i; }
    float ContentHeight() const;

    void Render(UI& ui) override;
    bool Contains(float x, float y) const override;
    std::shared_ptr<Widget> HitTest(float x, float y) override;
    bool OnPress(UI& ui) override;
    void OnRelease(UI&, bool) override { scroll.drag = false; }
    bool OnScroll(UI& ui, float x, float y, int dir) override;

private:
    struct Line { std::vector<size_t> idx; float h = 0; bool full = false; };
    std::vector<Line> Lines() const;
    float LineOffset(const std::vector<Line>& lines, size_t index) const;
    void Place(const std::vector<Line>& lines);

    ScrollState scroll;
    int cursor = -1;
    int columns = 1;
    bool background = false, bgOutline = true, rowLine = false;
    Color bgColor{0, 0, 0, 255}, lineColor{0, 0, 0, 255};
    float bgAlpha = 1.0f, lineAlpha = 1.0f;
};

class RowList : public Widget {
public:
    float itemH = 36.0f;
    std::function<int()> count;
    std::function<bool(int)> isSelected;
    std::function<bool()> listFocused;
    std::function<void(UI&, int, const Rect&)> drawRow;
    std::function<void(int, float, bool)> onActivate;

    bool CanFocus() const override { return true; }
    void OnFocus(bool gained) override;
    int Fit() const { return std::max(1, static_cast<int>((bounds.y1 - bounds.y0 - 4.0f) / itemH)); }
    bool NeedsScroll() const { return count && count() > Fit(); }
    float RowLeft() const { return bounds.x0 + 6.0f; }
    float RowWidth() const { return (bounds.x1 - bounds.x0) - (NeedsScroll() ? 18.0f : 12.0f); }
    Rect Box(int p, float offset = 0.0f) const {
        const float y = bounds.y0 + 4.0f + static_cast<float>(p) * itemH - offset;
        return {RowLeft() - 2.0f, RowLeft() + RowWidth() + 2.0f, y, y + itemH};
    }
    void ResetScroll() { scroll.Reset(); }
    void Reveal(int idx) { scroll.Reveal(idx, Fit()); }

    void Render(UI& ui) override;
    bool OnPress(UI& ui) override;
    void OnRelease(UI& ui, bool inside) override;
    bool OnScroll(UI&, float x, float y, int dir) override;

private:
    ScrollState scroll;
    int pressIdx = -1, lastIdx = -1;
    float pressX = 0;
    std::chrono::steady_clock::time_point lastClick{};
};

struct DescLine {
    std::string text;
    float indent = 0;
    std::string url;
};

class DescView : public Widget {
public:
    std::vector<DescLine> lines;
    std::function<void(const std::string&)> onOpenLink;

    bool CanFocus() const override;
    void ResetScroll() { scroll.Reset(); }
    int Fit() const { return std::max(1, static_cast<int>((bounds.y1 - bounds.y0 - 4.0f) / 10.0f)); }
    float RowLeft() const { return bounds.x0 + 5.0f; }
    float RowWidth() const { return (bounds.x1 - bounds.x0) - 10.0f; }

    void Render(UI& ui) override;
    bool OnPress(UI& ui) override;
    void OnRelease(UI& ui, bool inside) override;
    bool OnScroll(UI&, float x, float y, int dir) override;
    bool OnKey(int key) override;

private:
    float LineX(const DescLine& l) const { return RowLeft() + l.indent; }
    float LineY(float row) const { return bounds.y0 + 4.0f + 10.0f * row; }
    Rect LinkRect(UI& ui, const DescLine& l, float row) const {
        const float x = LineX(l), y = LineY(row);
        return {x, x + ui.TextWidth(l.text), y, y + 10.0f};
    }
    ScrollState scroll;
    int pressLine = -1, pressedLink = -1, focusedLink = -1;
};

}
