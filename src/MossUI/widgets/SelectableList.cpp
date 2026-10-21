#include "MossUI/widgets/SelectableList.hpp"

#include "MossUI/core/Keys.hpp"

#include <algorithm>

namespace MossUI {

int SelectableList::VisibleRows() const {
    return std::max(1, static_cast<int>((bounds.y1 - bounds.y0 - 2.0f) / CurrentWidgetMetrics().rowHeight));
}

int SelectableList::ItemAt(float y) const {
    const float rowHeight = CurrentWidgetMetrics().rowHeight;
    const float itemPosition = scroll.At() + (y - bounds.y0 - 1.0f) / rowHeight;
    const int index = static_cast<int>(std::floor(itemPosition));
    return y >= bounds.y0 + 1.0f && y < bounds.y1 - 1.0f &&
                   index >= 0 && index < static_cast<int>(items.size())
               ? index
               : -1;
}

void SelectableList::Render(UI& ui) {
    ui.Fill(bounds, CurrentTheme().panel, 0.9f);
    ui.Outline(bounds, CurrentTheme().border);
    const int visible = VisibleRows();
    ui.DrawScrollbar(scroll, {bounds.x1 + 3.0f, bounds.x1 + 8.0f, bounds.y0, bounds.y1}, visible,
                     static_cast<int>(items.size()));
    const float rowHeight = CurrentWidgetMetrics().rowHeight;
    const float position = scroll.visualPos;
    const int first = static_cast<int>(std::floor(position));
    const float offset = (position - static_cast<float>(first)) * rowHeight;
    ui.PushClip(bounds);
    for (int row = 0; row <= visible; ++row) {
        const int index = first + row;
        if (index >= static_cast<int>(items.size())) break;
        const float top = bounds.y0 + 1.0f + row * rowHeight - offset;
        const Rect area{bounds.x0 + 1.0f, bounds.x1 - 1.0f, top, top + rowHeight};
        if (index == selected) ui.Fill(area, CurrentTheme().pressed, 0.75f);
        else if (ui.Hovered(area)) ui.Fill(area, CurrentTheme().hover, 0.75f);
        ui.Text(area.x0 + 7.0f, area.x1 - 4.0f, area.y0 + (rowHeight - 10.0f) * 0.5f,
                ui.Trunc(items[static_cast<size_t>(index)], area.x1 - area.x0 - 12.0f),
                CurrentTheme().text, Align::Left, 1.0f);
    }
    ui.PopClip();
}

bool SelectableList::Contains(float x, float y) const {
    const bool body = x >= bounds.x0 && x <= bounds.x1 && y >= bounds.y0 && y <= bounds.y1;
    const bool rail = scroll.max > 0 && x >= bounds.x1 + 1.0f && x <= bounds.x1 + 10.0f &&
                      y >= bounds.y0 && y <= bounds.y1;
    return visible && (body || rail);
}

std::shared_ptr<Widget> SelectableList::HitTest(float x, float y) {
    return Contains(x, y) ? shared_from_this() : nullptr;
}

bool SelectableList::OnPress(UI& ui) {
    pressed = -1;
    if (scroll.Grab(ui)) return true;
    pressed = ItemAt(ui.MouseY());
    return pressed >= 0;
}

void SelectableList::OnDrag(UI& ui) {
    if (scroll.drag) {
        ui.DrawScrollbar(scroll, {bounds.x1 + 3.0f, bounds.x1 + 8.0f, bounds.y0, bounds.y1}, VisibleRows(),
                         static_cast<int>(items.size()));
    }
}

void SelectableList::OnRelease(UI& ui, bool inside) {
    scroll.drag = false;
    const int index = pressed;
    pressed = -1;
    if (!inside || index < 0 || index != ItemAt(ui.MouseY())) return;
    selected = index;
    ui.Click();
    if (onSelect) onSelect(selected);
}

bool SelectableList::OnScroll(UI&, float x, float y, int dir) {
    if (!Contains(x, y)) return false;
    scroll.Step(dir);
    return true;
}

bool SelectableList::OnKey(int vk) {
    if (vk != Key(KeyCode::UP) && vk != Key(KeyCode::DOWN) && vk != Key(KeyCode::ENTER)) return false;
    if (items.empty()) return true;
    if (vk == Key(KeyCode::ENTER)) {
        if (selected >= 0 && onSelect) onSelect(selected);
        return true;
    }
    const int direction = vk == Key(KeyCode::DOWN) ? 1 : -1;
    selected = std::clamp(selected < 0 ? (direction > 0 ? 0 : static_cast<int>(items.size()) - 1)
                                      : selected + direction,
                          0, static_cast<int>(items.size()) - 1);
    scroll.Reveal(selected, VisibleRows());
    if (onSelect) onSelect(selected);
    return true;
}

}
