#include "MossUI/widgets/ScrollContainer.hpp"

#include <algorithm>
#include <cmath>

namespace MossUI {

float ScrollContainer::ContentHeight() const {
    float height = 0.0f;
    size_t count = 0;
    for (const auto& child : children) {
        if (!child->visible) continue;
        if (count++) height += spacing;
        height += std::max(0.0f, child->PreferredHeight());
    }
    return height;
}

int ScrollContainer::VisiblePixels() const {
    return std::max(0, static_cast<int>(std::floor(bounds.y1 - bounds.y0)));
}

int ScrollContainer::ContentPixels() const {
    return std::max(0, static_cast<int>(std::ceil(ContentHeight())));
}

void ScrollContainer::Layout() {
    float y = bounds.y0 - scroll.UpdateVisualPosition();
    bool first = true;
    for (auto& child : children) {
        if (!child->visible) continue;
        if (!first) y += spacing;
        first = false;
        const float height = std::max(0.0f, child->PreferredHeight());
        child->bounds = {bounds.x0, bounds.x1, y, y + height};
        y += height;
    }
}

void ScrollContainer::Render(UI& ui) {
    if (!visible || bounds.x1 <= bounds.x0 || bounds.y1 <= bounds.y0) return;

    const int visible = VisiblePixels();
    const int total = ContentPixels();
    const Rect rail{bounds.x1 + 4.0f, bounds.x1 + 9.0f, bounds.y0, bounds.y1};
    ui.DrawScrollbar(scroll, rail, visible, total);
    Layout();

    ui.PushClip(bounds);
    for (auto& child : children) {
        if (child->visible && child->bounds.y1 > bounds.y0 && child->bounds.y0 < bounds.y1)
            child->Render(ui);
    }
    ui.PopClip();
}

bool ScrollContainer::Contains(float x, float y) const {
    const bool inViewport = x >= bounds.x0 && x <= bounds.x1 && y >= bounds.y0 && y <= bounds.y1;
    const bool inScrollbar = scroll.max > 0 && x >= scroll.rail.x0 - 2.0f && x <= scroll.rail.x1 + 2.0f &&
                            y >= scroll.rail.y0 && y <= scroll.rail.y1;
    return inViewport || inScrollbar;
}

std::shared_ptr<Widget> ScrollContainer::HitTest(float x, float y) {
    if (!visible || !Contains(x, y)) return nullptr;
    if (scroll.max > 0 && x >= scroll.rail.x0 - 2.0f && x <= scroll.rail.x1 + 2.0f) return shared_from_this();
    if (x < bounds.x0 || x > bounds.x1 || y < bounds.y0 || y > bounds.y1) return nullptr;

    Layout();
    for (auto it = children.rbegin(); it != children.rend(); ++it)
        if ((*it)->visible)
            if (auto child = (*it)->HitTest(x, y)) return child;
    return nullptr;
}

bool ScrollContainer::OnPress(UI& ui) {
    if (scroll.max <= 0 || ui.MouseX() < scroll.rail.x0 - 2.0f || ui.MouseX() > scroll.rail.x1 + 2.0f)
        return false;

    const float railHeight = scroll.rail.y1 - scroll.rail.y0;
    const float handleTop = scroll.rail.y0 +
                            (railHeight - scroll.handleH) * scroll.UpdateVisualPosition() /
                                static_cast<float>(scroll.max);
    const Rect handle{scroll.rail.x0 - 2.0f, scroll.rail.x1 + 2.0f, handleTop, handleTop + scroll.handleH};
    if (ui.Hovered(handle)) {
        scroll.drag = true;
    } else {
        const int page = std::max(1, VisiblePixels());
        scroll.Step(ui.MouseY() < handleTop ? -page : page);
    }
    return true;
}

bool ScrollContainer::OnScroll(UI&, float x, float y, int dir) {
    if (!visible || x < bounds.x0 || x > bounds.x1 || y < bounds.y0 || y > bounds.y1 || scroll.max <= 0) return false;
    scroll.Step(dir * 24);
    return true;
}

}
