#include "MossUI/widgets/Basic.hpp"

#include "MossUI/core/Keys.hpp"
#include "MossUI/core/State.hpp"

#include <algorithm>
#include <chrono>

namespace MossUI {

namespace {
void Fire(const std::function<void()>& f) {
    if (!f) return;
    auto fn = f;
    fn();
}
}

void LabelWidget::Render(UI& ui) { ui.Text(bounds.x0, bounds.x1, bounds.y0 + 11, text, color, align, alpha); }

void ButtonWidget::Render(UI& ui) {
    const bool hov = ui.Hovered(bounds);
    if (style == Style::Light) ui.DrawButton(bounds, label, enabled, hov, pressed && ui.MouseDown());
    else                       ui.DrawDarkButton(bounds, label, hov);
    if (hov && !tooltip.empty()) ui.Tooltip(tooltip);
}

void ButtonWidget::OnRelease(UI& ui, bool inside) {
    const bool fire = pressed && inside && enabled;
    pressed = false;
    if (!fire) return;
    ui.Click();
    Fire(onPress);
}

bool ButtonWidget::OnKey(int vk) {
    if (!enabled || (vk != Key(KeyCode::ENTER) && vk != Key(KeyCode::SPACE))) return false;
    UI ui;
    ui.Click();
    Fire(onPress);
    return true;
}

ConfirmButton::ConfirmButton(std::string text, std::function<void()> action)
    : ButtonWidget(text, nullptr), base(std::move(text)), run(std::move(action)) {
    onPress = [this] {
        const auto now = std::chrono::steady_clock::now();
        if (!confirm || (armed && now - armedAt < std::chrono::seconds(3))) {
            Disarm();
            Fire(run);
            return;
        }
        armed = true;
        armedAt = now;
        label = "Click again to confirm";
    };
}

void ConfirmButton::Render(UI& ui) {
    if (armed && std::chrono::steady_clock::now() - armedAt >= std::chrono::seconds(3)) Disarm();
    ButtonWidget::Render(ui);
    if (glyph.empty()) return;
    const float size = std::min(20.0f, bounds.y1 - bounds.y0);
    const float centerY = (bounds.y0 + bounds.y1) * 0.5f;
    const bool hov = ui.Hovered(bounds);
    ui.DrawGlyph({bounds.x0 + 2.0f, bounds.x0 + 2.0f + size, centerY - size * 0.5f, centerY + size * 0.5f},
                 glyph, enabled, hov, hov && pressed && ui.MouseDown());
}

void IconButton::Render(UI& ui) {
    const bool hov = ui.Hovered(bounds);
    const bool down = pressed && ui.MouseDown();
    ui.DrawButton(bounds, "", enabled, hov, down);
    const float cx = (bounds.x0 + bounds.x1) * 0.5f, cy = (bounds.y0 + bounds.y1) * 0.5f;
    ui.DrawGlyph({cx - 10.0f, cx + 10.0f, cy - 10.0f, cy + 10.0f}, asset, enabled, hov, down);
    if (hov && !tooltip.empty()) ui.Tooltip(tooltip);
}

void TextFieldWidget::Render(UI& ui) { ui.DrawTextField(state, bounds, placeholder); }

bool TextFieldWidget::OnPress(UI& ui) {
    static auto lastClick = std::chrono::steady_clock::time_point{};
    const auto now = std::chrono::steady_clock::now();
    state.focused = true;
    if (now - lastClick < std::chrono::milliseconds(350)) detail::SelectAll(state);
    else detail::PlaceCaret(state, ui.MouseX(), detail::S().shift);
    lastClick = now;
    state.drag = true;
    return true;
}

bool TextFieldWidget::OnKey(int vk) {
    if (vk == Key(KeyCode::ESCAPE)) return false;
    detail::TextKey(state, vk);
    return true;
}

bool SearchField::OnKey(int vk) {
    if (vk == Key(KeyCode::UP) || vk == Key(KeyCode::DOWN) || vk == Key(KeyCode::ENTER) ||
        vk == Key(KeyCode::PAGE_UP) || vk == Key(KeyCode::PAGE_DOWN))
        return false;
    return TextFieldWidget::OnKey(vk);
}

}
