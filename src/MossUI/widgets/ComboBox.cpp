#include "MossUI/widgets/ComboBox.hpp"

#include "MossUI/core/Keys.hpp"

#include <algorithm>

namespace MossUI {

Rect ComboBox::PopupBounds() const {
    const auto& metrics = CurrentWidgetMetrics();
    const float height = std::min(metrics.comboPopupMaxHeight,
                                  static_cast<float>(options.size()) * metrics.rowHeight);
    const float top = bounds.y1 + height <= screenHeight ? bounds.y1 : bounds.y0 - height;
    return {bounds.x0, bounds.x1, top, top + height};
}

void ComboBox::Render(UI& ui) {
    screenHeight = ui.Height();
    const bool hovered = ui.Hovered(bounds);
    ui.DrawButton(bounds, selected >= 0 && selected < static_cast<int>(options.size())
                              ? options[static_cast<size_t>(selected)] : std::string(),
                  true, hovered, false);
    if (!expanded || options.empty()) return;
    const Rect popup = PopupBounds();
    ui.Fill(popup, CurrentTheme().panel, 1.0f);
    ui.Outline(popup, CurrentTheme().border);
    const float rowHeight = CurrentWidgetMetrics().rowHeight;
    const int visible = std::max(1, static_cast<int>((popup.y1 - popup.y0) / rowHeight));
    ui.DrawScrollbar(scroll, {popup.x1 + 2.0f, popup.x1 + 7.0f, popup.y0, popup.y1}, visible,
                     static_cast<int>(options.size()));
    const float position = scroll.visualPos;
    const int first = static_cast<int>(std::floor(position));
    const float offset = (position - static_cast<float>(first)) * rowHeight;
    ui.PushClip(popup);
    for (int row = 0; row <= visible && first + row < static_cast<int>(options.size()); ++row) {
        const int index = first + row;
        const float top = popup.y0 + row * rowHeight - offset;
        const Rect item{popup.x0 + 1.0f, popup.x1 - 1.0f, top, top + rowHeight};
        if (index == selected) ui.Fill(item, CurrentTheme().pressed, 0.8f);
        else if (ui.Hovered(item)) ui.Fill(item, CurrentTheme().hover, 0.8f);
        ui.Text(item.x0 + 6.0f, item.x1 - 4.0f, item.y0 + (rowHeight - 10.0f) * 0.5f,
                ui.Trunc(options[static_cast<size_t>(index)], item.x1 - item.x0 - 10.0f),
                CurrentTheme().text, Align::Left, 1.0f);
    }
    ui.PopClip();
}

bool ComboBox::Contains(float x, float y) const {
    const Rect popup = PopupBounds();
    return visible && (Widget::Contains(x, y) ||
                       (expanded && y >= popup.y0 && y <= popup.y1 && x >= bounds.x0 && x <= popup.x1 + 9.0f));
}

std::shared_ptr<Widget> ComboBox::HitTest(float x, float y) {
    return Contains(x, y) ? shared_from_this() : nullptr;
}

bool ComboBox::OnPress(UI& ui) {
    const Rect popup = PopupBounds();
    if (options.empty()) {
        expanded = false;
        return true;
    }
    if (expanded && ui.MouseY() >= popup.y0 && ui.MouseY() <= popup.y1 && ui.MouseX() > popup.x1 && scroll.max > 0) {
        scroll.drag = true;
        return true;
    }
    if (!expanded || ui.MouseY() < popup.y0 || ui.MouseY() > popup.y1 || ui.MouseX() > popup.x1) {
        expanded = !expanded;
        if (expanded) {
            const int visible = std::max(1, static_cast<int>((popup.y1 - popup.y0) / CurrentWidgetMetrics().rowHeight));
            scroll.pos = std::clamp(selected, 0, std::max(0, static_cast<int>(options.size()) - visible));
        }
        return true;
    }
    const float rowHeight = CurrentWidgetMetrics().rowHeight;
    const int index = static_cast<int>(std::floor(scroll.At() + (ui.MouseY() - popup.y0) / rowHeight));
    if (ui.MouseY() >= popup.y0 && ui.MouseY() < popup.y1 && index >= 0 &&
        index < static_cast<int>(options.size())) {
        selected = index;
        expanded = false;
        ui.Click();
        if (onChange) onChange(selected);
    } else {
        expanded = false;
    }
    return true;
}

void ComboBox::OnDrag(UI& ui) {
    if (expanded && scroll.drag) {
        const Rect popup = PopupBounds();
        const int visible = std::max(1, static_cast<int>((popup.y1 - popup.y0) / CurrentWidgetMetrics().rowHeight));
        ui.DrawScrollbar(scroll, {popup.x1 + 2.0f, popup.x1 + 7.0f, popup.y0, popup.y1}, visible,
                         static_cast<int>(options.size()));
    }
}

bool ComboBox::OnScroll(UI&, float x, float y, int direction) {
    if (!expanded || !Contains(x, y) || options.empty()) return false;
    const Rect popup = PopupBounds();
    const int visible = std::max(1, static_cast<int>((popup.y1 - popup.y0) / CurrentWidgetMetrics().rowHeight));
    scroll.pos = std::clamp(scroll.pos + direction, 0, std::max(0, static_cast<int>(options.size()) - visible));
    return true;
}

bool ComboBox::OnKey(int vk) {
    if (vk == Key(KeyCode::ESCAPE)) {
        expanded = false;
        return true;
    }
    if (vk == Key(KeyCode::ENTER) || vk == Key(KeyCode::SPACE)) {
        if (options.empty()) return true;
        expanded = !expanded;
        if (expanded) {
            const Rect popup = PopupBounds();
            const int visible = std::max(1, static_cast<int>((popup.y1 - popup.y0) / CurrentWidgetMetrics().rowHeight));
            scroll.pos = std::clamp(selected, 0, std::max(0, static_cast<int>(options.size()) - visible));
        }
        return true;
    }
    if ((vk == Key(KeyCode::UP) || vk == Key(KeyCode::DOWN)) && !options.empty()) {
        selected = std::clamp(selected + (vk == Key(KeyCode::DOWN) ? 1 : -1), 0,
                              static_cast<int>(options.size()) - 1);
        if (expanded) {
            const Rect popup = PopupBounds();
            const int visible = std::max(1, static_cast<int>((popup.y1 - popup.y0) / CurrentWidgetMetrics().rowHeight));
            scroll.Reveal(selected, visible);
        }
        if (onChange) onChange(selected);
        return true;
    }
    return false;
}

}
