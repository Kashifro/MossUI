#include "MossUI/widgets/TabBar.hpp"

#include "MossUI/core/Keys.hpp"

#include <algorithm>

namespace MossUI {

int TabBar::TabAt(float x) const {
    if (tabs.empty() || x < bounds.x0 || x > bounds.x1) return -1;
    const float width = (bounds.x1 - bounds.x0) / static_cast<float>(tabs.size());
    return std::clamp(static_cast<int>((x - bounds.x0) / std::max(1.0f, width)), 0,
                      static_cast<int>(tabs.size()) - 1);
}

void TabBar::Render(UI& ui) {
    if (tabs.empty()) return;
    const float width = (bounds.x1 - bounds.x0) / static_cast<float>(tabs.size());
    for (size_t i = 0; i < tabs.size(); ++i) {
        const Rect tab{bounds.x0 + width * static_cast<float>(i), bounds.x0 + width * static_cast<float>(i + 1),
                       bounds.y0, bounds.y1};
        const bool active = static_cast<int>(i) == selected;
        const bool hovered = ui.Hovered(tab);
        ui.Fill(tab, active ? CurrentTheme().pressed : hovered ? CurrentTheme().hover : CurrentTheme().panel, 0.9f);
        if (active) ui.Fill({tab.x0, tab.x1, tab.y1 - 2.0f, tab.y1}, CurrentTheme().accent);
        ui.Outline(tab, CurrentTheme().border, 0.7f);
        ui.Text(tab.x0 + 3.0f, tab.x1 - 3.0f, tab.y0 + ((tab.y1 - tab.y0) - 8.0f) * 0.5f,
                ui.Trunc(tabs[i], tab.x1 - tab.x0 - 8.0f), CurrentTheme().text, Align::Center, 1.0f);
    }
}

bool TabBar::OnPress(UI& ui) {
    pressed = TabAt(ui.MouseX());
    return pressed >= 0;
}

void TabBar::OnRelease(UI& ui, bool inside) {
    const int index = pressed;
    pressed = -1;
    if (!inside || index < 0 || index != TabAt(ui.MouseX())) return;
    selected = index;
    ui.Click();
    if (onSelect) onSelect(selected);
}

bool TabBar::OnKey(int vk) {
    if (tabs.empty() || (vk != Key(KeyCode::LEFT) && vk != Key(KeyCode::RIGHT))) return false;
    selected = std::clamp(selected + (vk == Key(KeyCode::RIGHT) ? 1 : -1), 0, static_cast<int>(tabs.size()) - 1);
    if (onSelect) onSelect(selected);
    return true;
}

void TabListView::Render(UI& ui) {
    if (!titles) return;
    ui.Fill(bounds, col::BLACK, 0.45f);
    ui.Outline(bounds, col::BLACK);
    for (size_t i = 0; i < titles->size(); ++i) {
        const Rect row = Row(static_cast<int>(i));
        if (row.y1 > bounds.y1) break;
        if (static_cast<int>(i) == (current ? current() : 0)) ui.Fill(row, col::WHITE, 0.18f);
        else if (ui.Hovered(row)) ui.Fill(row, col::WHITE, 0.08f);
        ui.Text(row.x0 + 5.0f, row.x1 - 3.0f, row.y0 + 6.0f,
                ui.Trunc((*titles)[i], row.x1 - row.x0 - 10.0f), col::WHITE, Align::Left, 1.0f);
    }
}

bool TabListView::OnPress(UI& ui) {
    pressed = -1;
    for (size_t i = 0; titles && i < titles->size(); ++i)
        if (ui.Hovered(Row(static_cast<int>(i)))) pressed = static_cast<int>(i);
    return pressed >= 0;
}

void TabListView::OnRelease(UI& ui, bool inside) {
    const int i = pressed;
    pressed = -1;
    if (i < 0 || !inside || !ui.Hovered(Row(i)) || !onSelect) return;
    ui.Click();
    onSelect(i);
}

bool TabListView::OnKey(int key) {
    if (!titles || titles->empty() || (key != Key(KeyCode::UP) && key != Key(KeyCode::DOWN))) return false;
    const int currentIndex = current ? current() : 0;
    const int next = std::clamp(currentIndex + (key == Key(KeyCode::DOWN) ? 1 : -1),
                                0, static_cast<int>(titles->size()) - 1);
    if (onSelect) onSelect(next);
    return true;
}

}
