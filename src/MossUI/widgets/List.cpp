#include "MossUI/widgets/List.hpp"

#include "MossUI/core/Keys.hpp"

#include <algorithm>

namespace MossUI {

std::vector<ListWidget::Line> ListWidget::Lines() const {
    std::vector<Line> out;
    Line cur;
    auto flush = [&] { if (!cur.idx.empty()) { out.push_back(std::move(cur)); cur = Line{}; } };
    for (size_t i = 0; i < children.size(); ++i) {
        const Widget& c = *children[i];
        if (c.FullWidth()) {
            flush();
            Line l;
            l.idx = {i};
            l.h = c.PreferredHeight();
            l.full = true;
            out.push_back(std::move(l));
            continue;
        }
        cur.idx.push_back(i);
        cur.h = std::max(cur.h, c.PreferredHeight());
        if (static_cast<int>(cur.idx.size()) == columns) flush();
    }
    flush();
    return out;
}

float ListWidget::ContentHeight() const {
    float h = 0;
    for (const auto& l : Lines()) h += l.h;
    return h;
}

void ListWidget::Place(const std::vector<Line>& lines) {
    for (auto& c : children) c->visible = false;
    const float cw = (bounds.x1 - bounds.x0 - 2) / static_cast<float>(columns);
    const float position = scroll.UpdateVisualPosition();
    float y = bounds.y0 + 1.0f - position;
    for (const auto& line : lines) {
        if (y + line.h <= bounds.y0 + 1.0f) {
            y += line.h;
            continue;
        }
        if (y >= bounds.y1 - 1.0f) break;
        for (size_t c = 0; c < line.idx.size(); ++c) {
            Widget& w = *children[line.idx[c]];
            const float x0 = bounds.x0 + 1 + cw * static_cast<float>(c);
            w.bounds = {x0, line.full ? bounds.x1 - 1 : x0 + cw, y, y + w.PreferredHeight()};
            w.visible = true;
        }
        y += line.h;
    }
}

float ListWidget::LineOffset(const std::vector<Line>& lines, size_t index) const {
    float offset = 0.0f;
    for (size_t line = 0; line < index && line < lines.size(); ++line) offset += lines[line].h;
    return offset;
}

void ListWidget::Render(UI& ui) {
    if (background) {
        ui.Fill(bounds, bgColor, bgAlpha);
        if (bgOutline) ui.Outline(bounds, col::BLACK);
    }
    const auto lines = Lines();
    const int visiblePixels = std::max(0, static_cast<int>(std::floor(bounds.y1 - bounds.y0 - 2.0f)));
    float contentHeight = 0.0f;
    for (const auto& line : lines) contentHeight += line.h;
    const int contentPixels = std::max(0, static_cast<int>(std::ceil(contentHeight)));
    ui.DrawScrollbar(scroll, {bounds.x1 + 4, bounds.x1 + 9, bounds.y0, bounds.y1},
                     visiblePixels, contentPixels);
    Place(lines);
    Canvas canvas(ui, bounds);
    for (auto& c : children)
        if (c->visible) c->Render(ui);
    if (cursor >= 0 && cursor < static_cast<int>(children.size()) && children[static_cast<size_t>(cursor)]->visible)
        ui.Outline(children[static_cast<size_t>(cursor)]->bounds, col::WHITE, 0.8f);
    if (rowLine) {
        for (auto& c : children) {
            if (!c->visible || c->FullWidth()) continue;
            ui.Fill({c->bounds.x0, c->bounds.x1, c->bounds.y1 - 1, c->bounds.y1}, lineColor, lineAlpha);
        }
        if (columns == 2) {
            const float x = bounds.x0 + 1 + (bounds.x1 - bounds.x0 - 2) * 0.5f;
            ui.Fill({x, x + 1, bounds.y0 + 1, bounds.y1 - 1}, lineColor, lineAlpha);
        }
    }
}

bool ListWidget::NavKey(int vk) {
    const int n = static_cast<int>(children.size());
    if (vk == Key(KeyCode::UP) || vk == Key(KeyCode::DOWN)) {
        const int dir = vk == Key(KeyCode::DOWN) ? 1 : -1;
        int i = cursor < 0 ? (dir > 0 ? -1 : n) : cursor;
        do { i += dir; } while (i >= 0 && i < n && children[static_cast<size_t>(i)]->FullWidth());
        if (i < 0 || i >= n) return true;
        cursor = i;
        const auto lines = Lines();
        for (size_t l = 0; l < lines.size(); ++l) {
            if (std::find(lines[l].idx.begin(), lines[l].idx.end(), static_cast<size_t>(i)) == lines[l].idx.end()) continue;
            const float rowTop = LineOffset(lines, l);
            const float rowBottom = rowTop + lines[l].h;
            const float viewport = std::max(0.0f, bounds.y1 - bounds.y0 - 2.0f);
            if (rowTop < scroll.pos) scroll.pos = static_cast<int>(rowTop);
            else if (rowBottom > scroll.pos + viewport)
                scroll.pos = static_cast<int>(std::ceil(rowBottom - viewport));
            scroll.Step(0);
            break;
        }
        return true;
    }
    if (cursor < 0 || cursor >= n) return false;
    return children[static_cast<size_t>(cursor)]->OnNavKey(vk);
}

bool ListWidget::Contains(float x, float y) const {
    const float right = scroll.max > 0 ? bounds.x1 + 10 : bounds.x1;
    return x >= bounds.x0 && x <= right && y >= bounds.y0 && y <= bounds.y1;
}

std::shared_ptr<Widget> ListWidget::HitTest(float x, float y) {
    if (!visible || !Contains(x, y)) return nullptr;
    if (scroll.rail.x1 > scroll.rail.x0 && x >= scroll.rail.x0 - 2 && x <= scroll.rail.x1 + 2) return shared_from_this();
    Place(Lines());
    for (auto it = children.rbegin(); it != children.rend(); ++it)
        if (auto w = (*it)->HitTest(x, y)) return w;
    return shared_from_this();
}

bool ListWidget::OnPress(UI& ui) { return scroll.Grab(ui); }

bool ListWidget::OnScroll(UI&, float x, float y, int dir) {
    if (!visible || !Contains(x, y)) return false;
    scroll.Step(static_cast<int>(dir * CurrentWidgetMetrics().scrollStep));
    return true;
}

void RowList::OnFocus(bool gained) {
    Widget::OnFocus(gained);
    const int n = count ? count() : 0;
    if (!gained || n <= 0 || !onActivate) return;
    for (int i = 0; i < n; ++i)
        if (isSelected && isSelected(i)) return;
    onActivate(0, RowWidth(), false);
}

void RowList::Render(UI& ui) {
    const int n = count ? count() : 0, fit = Fit();
    ui.DrawScrollbar(scroll, {bounds.x1 - 6.0f, bounds.x1, bounds.y0, bounds.y1}, fit, n);
    const float position = scroll.visualPos;
    const int first = static_cast<int>(std::floor(position));
    const float offset = (position - static_cast<float>(first)) * itemH;
    const bool focused = !listFocused || listFocused();
    ui.PushClip(bounds);
    for (int p = 0; p <= fit; ++p) {
        const int idx = first + p;
        if (idx >= n) break;
        const Rect box = Box(p, offset);
        if (isSelected && isSelected(idx)) {
            ui.Fill(box, focused ? col::WHITE : col::SUMMARY);
            ui.Fill({box.x0 + 1, box.x1 - 1, box.y0 + 1, box.y1 - 1}, col::BLACK);
        }
        if (drawRow) drawRow(ui, idx, box);
    }
    ui.PopClip();
}

bool RowList::OnPress(UI& ui) {
    pressIdx = -1;
    if (scroll.Grab(ui)) return true;
    const int n = count ? count() : 0;
    const float position = scroll.At();
    const int first = static_cast<int>(std::floor(position));
    const float offset = (position - static_cast<float>(first)) * itemH;
    for (int p = 0; p <= Fit(); ++p) {
        const int idx = first + p;
        if (idx >= n) break;
        if (ui.Hovered(Box(p, offset))) { pressIdx = idx; pressX = ui.MouseX(); return true; }
    }
    return false;
}

void RowList::OnRelease(UI& ui, bool inside) {
    scroll.drag = false;
    const int idx = pressIdx;
    pressIdx = -1;
    const int n = count ? count() : 0;
    if (idx < 0 || !inside || idx >= n) return;
    ui.Click();
    const auto now = std::chrono::steady_clock::now();
    const bool dbl = lastIdx == idx && now - lastClick < std::chrono::milliseconds(250);
    lastIdx = idx;
    lastClick = now;
    if (onActivate) onActivate(idx, pressX - RowLeft(), dbl);
}

bool RowList::OnScroll(UI&, float x, float y, int dir) {
    if (!visible || !Contains(x, y)) return false;
    scroll.Step(dir);
    return true;
}

bool DescView::CanFocus() const {
    return std::any_of(lines.begin(), lines.end(), [](const DescLine& line) { return !line.url.empty(); });
}

void DescView::Render(UI& ui) {
    const int n = static_cast<int>(lines.size()), fit = Fit();
    ui.DrawScrollbar(scroll, {bounds.x1 - 6.0f, bounds.x1, bounds.y0, bounds.y1}, fit, n);
    const float position = scroll.visualPos;
    const int first = static_cast<int>(std::floor(position));
    ui.PushClip(bounds);
    for (int p = 0; p <= fit; ++p) {
        const int idx = first + p;
        if (idx >= n) break;
        const DescLine& l = lines[static_cast<size_t>(idx)];
        if (l.text.empty()) continue;
        const float row = static_cast<float>(idx) - position;
        if (l.url.empty()) {
            ui.Text(LineX(l), bounds.x1, LineY(row), l.text, col::TEXT_GRAY, Align::Left, 1.0f);
        } else {
            const bool hovered = ui.Hovered(LinkRect(ui, l, row));
            const bool pressed = pressedLink == idx && ui.MouseDown();
            ui.LinkText(LineX(l), bounds.x1, LineY(row), l.text, col::LINK, hovered, pressed);
        }
    }
    ui.PopClip();
}

bool DescView::OnPress(UI& ui) {
    pressLine = pressedLink = -1;
    if (scroll.Grab(ui)) return true;
    focusedLink = -1;
    const float position = scroll.At();
    const int first = static_cast<int>(std::floor(position));
    for (int p = 0; p <= Fit(); ++p) {
        const int idx = first + p;
        if (idx >= static_cast<int>(lines.size())) break;
        const DescLine& l = lines[static_cast<size_t>(idx)];
        if (l.url.empty()) continue;
        if (ui.Hovered(LinkRect(ui, l, static_cast<float>(idx) - position))) {
            pressLine = pressedLink = focusedLink = idx;
            return true;
        }
    }
    return false;
}

void DescView::OnRelease(UI& ui, bool inside) {
    scroll.drag = false;
    const int idx = pressLine;
    pressLine = pressedLink = -1;
    if (idx < 0 || !inside || idx >= static_cast<int>(lines.size())) return;
    const float visibleRow = static_cast<float>(idx) - scroll.At();
    if (visibleRow < 0.0f || visibleRow >= static_cast<float>(Fit())) return;
    const auto& line = lines[static_cast<size_t>(idx)];
    if (!ui.Hovered(LinkRect(ui, line, visibleRow))) return;
    ui.Click();
    if (onOpenLink) onOpenLink(line.url);
}

bool DescView::OnScroll(UI&, float x, float y, int dir) {
    if (!visible || !Contains(x, y)) return false;
    scroll.Step(dir * 2);
    return true;
}

bool DescView::OnKey(int key) {
    if (key == Key(KeyCode::UP) || key == Key(KeyCode::DOWN)) {
        const int direction = key == Key(KeyCode::DOWN) ? 1 : -1;
        int next = focusedLink;
        for (size_t attempts = 0; attempts < lines.size(); ++attempts) {
            next += direction;
            if (next < 0) next = static_cast<int>(lines.size()) - 1;
            if (next >= static_cast<int>(lines.size())) next = 0;
            if (!lines[static_cast<size_t>(next)].url.empty()) {
                focusedLink = next;
                scroll.Reveal(focusedLink, Fit());
                return true;
            }
        }
        return true;
    }
    if (key == Key(KeyCode::ENTER) && focusedLink >= 0 &&
        focusedLink < static_cast<int>(lines.size()) && onOpenLink) {
        onOpenLink(lines[static_cast<size_t>(focusedLink)].url);
        return true;
    }
    return false;
}

}
