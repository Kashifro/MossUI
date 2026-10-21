#include "MossUI/widgets/Options.hpp"

#include "MossUI/core/State.hpp"
#include "MossUI/core/Keys.hpp"
#include "MossUI/screen/Screen.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace MossUI {

namespace {
bool IsActivate(int vk) { return vk == Key(KeyCode::ENTER) || vk == Key(KeyCode::SPACE); }
}

void SeparatorOptionWidget::Render(UI& ui) {
    ui.Text(bounds.x0 + 10, bounds.x1 - 10, bounds.y0 + 9, label, col::GRAY, Align::Left, 1.0f);
    ui.Fill({bounds.x0 + 10, bounds.x1 - 10, bounds.y1 - 3, bounds.y1 - 2}, col::GRAY, 0.35f);
}

void OptionWidget::DrawLabel(UI& ui, float y, float width) const {
    ui.Text(bounds.x0 + 10, bounds.x0 + 10 + width, bounds.y0 + y, ui.Trunc(label, width), col::WHITE, Align::Left, 1.0f);
}

void OptionWidget::DrawTooltip(UI& ui) const {
    if (ui.Hovered(bounds) && !tooltip.empty()) ui.Tooltip(tooltip);
}

ToggleOptionWidget::ToggleOptionWidget(std::string text, std::function<bool()> get, std::function<void(bool)> set)
    : get(std::move(get)), set(std::move(set)) {
    label = std::move(text);
}

void ToggleOptionWidget::Render(UI& ui) {
    DrawLabel(ui, 11, bounds.x1 - bounds.x0 - 120);
    ui.DrawSwitch(bounds, get ? get() : false);
    DrawTooltip(ui);
}

bool ToggleOptionWidget::OnPress(UI& ui) {
    const Rect hit = UI::SwitchHit(bounds);
    armed = ui.MouseX() >= hit.x0 && ui.MouseX() <= hit.x1;
    return armed;
}

void ToggleOptionWidget::OnRelease(UI& ui, bool inside) {
    const Rect hit = UI::SwitchHit(bounds);
    const bool fire = armed && inside && ui.MouseX() >= hit.x0 && ui.MouseX() <= hit.x1;
    armed = false;
    if (!fire) return;
    ui.Click();
    if (get && set) set(!get());
}

SliderOptionWidget::SliderOptionWidget(std::string text, std::function<float()> get, std::function<void(float)> set, float min,
                                       float max, float step, int decimals, std::string suffix)
    : get(std::move(get)), set(std::move(set)), min(min), max(max), step(step), decimals(decimals), suffix(std::move(suffix)) {
    label = std::move(text);
}

Rect SliderOptionWidget::Track() const {
    const float cy = bounds.y0 + 34;
    return {bounds.x0 + 10, bounds.x1 - 10, cy - 9, cy + 9};
}

void SliderOptionWidget::Render(UI& ui) {
    const float v = std::clamp(get ? get() : min, min, max);
    DrawLabel(ui, 6, bounds.x1 - bounds.x0 - 90);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.*f%s", decimals, static_cast<double>(v), suffix.c_str());
    ui.Text(bounds.x1 - 90, bounds.x1 - 10, bounds.y0 + 6, buf, col::WHITE, Align::Right, 1.0f);
    const Rect track = Track();
    ui.DrawSlider(track, v, min, max, step, dragging || ui.Hovered(UI::SliderHit(track)));
    DrawTooltip(ui);
}

bool SliderOptionWidget::OnPress(UI& ui) {
    const Rect hit = UI::SliderHit(Track());
    dragging = ui.Hovered(hit);
    if (dragging) Apply(ui);
    return dragging;
}

void SliderOptionWidget::Apply(UI& ui) {
    const Rect t = Track();
    const float x0 = t.x0 + 5.0f, x1 = t.x1 - 5.0f;
    const float f = std::clamp((ui.MouseX() - x0) / std::max(1.0f, x1 - x0), 0.0f, 1.0f);
    float nv = min + f * (max - min);
    if (step > 0) nv = std::clamp(min + std::round((nv - min) / step) * step, min, max);
    if (set && !(get && std::fabs(get() - nv) < 1e-4f)) set(nv);
}

void SliderOptionWidget::StepBy(int dir) {
    const float base = step > 0 ? step : (max - min) / 50.0f;
    const float inc = base * (ShiftHeld() ? 10.0f : 1.0f);
    const float cur = get ? get() : min;
    float nv = std::clamp(cur + static_cast<float>(dir) * inc, min, max);
    if (step > 0) nv = std::clamp(min + std::round((nv - min) / step) * step, min, max);
    if (set && std::fabs(nv - cur) > 1e-6f) set(nv);
}

bool SliderOptionWidget::OnScroll(UI&, float x, float y, int dir) {
    if (!ShiftHeld() || !Contains(x, y)) return false;
    StepBy(-dir);
    return true;
}

bool ToggleOptionWidget::OnNavKey(int vk) {
    if (!get || !set) return false;
    if (IsActivate(vk)) set(!get());
    else if (vk == Key(KeyCode::LEFT)) set(false);
    else if (vk == Key(KeyCode::RIGHT)) set(true);
    else return false;
    return true;
}

bool SliderOptionWidget::OnNavKey(int vk) {
    if (vk != Key(KeyCode::LEFT) && vk != Key(KeyCode::RIGHT)) return false;
    StepBy(vk == Key(KeyCode::RIGHT) ? 1 : -1);
    return true;
}

ChoiceOptionWidget::ChoiceOptionWidget(std::string text, std::vector<std::string> options,
                                       std::function<int()> get, std::function<void(int)> set)
    : get(std::move(get)), set(std::move(set)) {
    label = std::move(text);
    combo.options = std::move(options);
    combo.onChange = [this](int idx) { if (this->set) this->set(idx); };
}

void ChoiceOptionWidget::Sync() {
    combo.bounds = Button();
    if (!combo.options.empty() && get)
        combo.selected = std::clamp(get(), 0, static_cast<int>(combo.options.size()) - 1);
}

float ChoiceOptionWidget::PreferredHeight() const {
    if (!combo.Expanded() || combo.options.empty()) return 30.0f;
    const auto& m = CurrentWidgetMetrics();
    return 30.0f + std::min(m.comboPopupMaxHeight, static_cast<float>(combo.options.size()) * m.rowHeight);
}

void ChoiceOptionWidget::Render(UI& ui) {
    Sync();
    DrawLabel(ui, 11, bounds.x1 - bounds.x0 - 120);
    combo.Render(ui);
    if (!combo.Expanded()) DrawTooltip(ui);
}

bool ChoiceOptionWidget::OnPress(UI& ui) {
    Sync();
    if (!combo.Contains(ui.MouseX(), ui.MouseY())) {
        combo.OnOutsideClick();
        return false;
    }
    return combo.OnPress(ui);
}

bool ChoiceOptionWidget::OnNavKey(int vk) {
    Sync();
    if ((vk == Key(KeyCode::LEFT) || vk == Key(KeyCode::RIGHT)) && !combo.options.empty()) {
        const int next = std::clamp(combo.selected + (vk == Key(KeyCode::RIGHT) ? 1 : -1),
                                    0, static_cast<int>(combo.options.size()) - 1);
        combo.selected = next;
        if (set) set(next);
        return true;
    }
    return combo.OnKey(vk);
}

TextOptionWidget::TextOptionWidget(std::string text, std::function<std::string()> get,
                                   std::function<void(const std::string&)> set, std::string placeholder)
    : field(std::move(placeholder)), get(std::move(get)), set(std::move(set)) {
    label = std::move(text);
}

void TextOptionWidget::Render(UI& ui) {
    field.bounds = FieldRect();
    if (!field.state.focused && get) field.state.text = get();
    DrawLabel(ui, 11, bounds.x1 - bounds.x0 - 148);
    field.Render(ui);
    DrawTooltip(ui);
}

bool TextOptionWidget::OnPress(UI& ui) {
    field.bounds = FieldRect();
    if (!ui.Hovered(field.bounds)) {
        field.OnFocus(false);
        return false;
    }
    if (!field.state.focused && get) field.state.text = get();
    return field.OnPress(ui);
}

void TextOptionWidget::OnFocus(bool gained) {
    OptionWidget::OnFocus(gained);
    if (!gained) field.OnFocus(false);
}

bool TextOptionWidget::OnKey(int vk) {
    if (!field.state.focused) return OnNavKey(vk);
    if (vk == Key(KeyCode::ESCAPE) || vk == Key(KeyCode::ENTER)) {
        field.OnFocus(false);
        return true;
    }
    const std::string before = field.state.text;
    field.OnKey(vk);
    if (set && field.state.text != before) set(field.state.text);
    return true;
}

bool TextOptionWidget::OnNavKey(int vk) {
    if (!IsActivate(vk)) return false;
    if (get) field.state.text = get();
    field.OnFocus(true);
    field.state.caret = field.state.anchor = static_cast<int>(field.state.text.size());
    FocusWidget(shared_from_this());
    return true;
}

bool KeyBindOptionWidget::OnNavKey(int vk) {
    if (!IsActivate(vk)) return false;
    pendingMod = 0;
    capturing = true;
    FocusWidget(shared_from_this());
    return true;
}

KeyBindOptionWidget::KeyBindOptionWidget(std::string text, std::function<int()> get, std::function<void(int)> set)
    : get(std::move(get)), set(std::move(set)) {
    label = std::move(text);
}

void KeyBindOptionWidget::Render(UI& ui) {
    DrawLabel(ui, 11, bounds.x1 - bounds.x0 - 120);
    const int code = get ? get() : 0;
    const std::string clash = capturing || !conflict ? std::string() : conflict(code);
    ui.DrawDarkButton(Button(), capturing ? "Press a key..." : KeyName(code), ui.Hovered(Button()),
                      clash.empty() ? nullptr : &col::WARN);
    DrawTooltip(ui);
    if (!clash.empty() && ui.Hovered(bounds)) ui.Tooltip("Same key as " + clash);
}

bool KeyBindOptionWidget::OnPress(UI& ui) {
    const Rect b = Button();
    pendingMod = 0;
    capturing = ui.Hovered(b);
    if (capturing) ui.Click();
    return false;
}

bool KeyBindOptionWidget::OnKey(int vk) {
    if (!capturing) return false;
    if (vk == Key(KeyCode::ESCAPE)) { capturing = false; pendingMod = 0; return true; }
    if (ModifierBit(vk)) { pendingMod = vk; return true; }
    const auto& st = detail::S();
    const int mods = (st.ctrl ? MOD_CTRL : 0) | (st.shift ? MOD_SHIFT : 0) | (st.alt ? MOD_ALT : 0);
    if (set) set(vk | mods);
    capturing = false;
    pendingMod = 0;
    return true;
}

void KeyBindOptionWidget::OnKeyUp(int vk) {
    if (!capturing || !pendingMod || vk != pendingMod) return;
    if (set) set(vk);
    capturing = false;
    pendingMod = 0;
}

}
