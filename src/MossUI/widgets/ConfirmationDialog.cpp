#include "MossUI/widgets/ConfirmationDialog.hpp"

#include "MossUI/core/Layout.hpp"
#include "MossUI/core/State.hpp"
#include "MossUI/core/Keys.hpp"

#include <algorithm>
#include <utility>

namespace MossUI {

void ConfirmationDialog::Show(std::string dialogTitle, std::string dialogMessage,
                              std::vector<DialogAction> dialogActions, std::string dialogWarning,
                              DialogPresentation dialogPresentation) {
    title = std::move(dialogTitle);
    message = std::move(dialogMessage);
    actions = std::move(dialogActions);
    warning = std::move(dialogWarning);
    presentation = dialogPresentation;
    pressedButton = -1;
    focusedAction = 0;
    visible = true;
}

void ConfirmationDialog::Dismiss() {
    visible = false;
    pressedButton = -1;
    actions.clear();
}

Rect ConfirmationDialog::PanelBounds(UI& ui) const {
    const float width = std::max(160.0f, std::min(420.0f, bounds.x1 - bounds.x0 - 32.0f));
    const auto lines = ui.Wrap(message, width - 28.0f);
    const float warningHeight = warning.empty() ? 0.0f : 11.0f;
    const float height = 86.0f + static_cast<float>(lines.size()) * 11.0f + warningHeight;
    return Layout::Centered((bounds.x0 + bounds.x1) * 0.5f, (bounds.y0 + bounds.y1) * 0.5f, width, height);
}

std::vector<Rect> ConfirmationDialog::ActionBounds(UI& ui) const {
    if (actions.empty()) return {};

    const float screenWidth = bounds.x1 - bounds.x0;
    const float rowWidth = std::min(320.0f, std::max(0.0f, screenWidth - 32.0f));
    const float gap = CurrentWidgetMetrics().controlGap;
    const float buttonWidth = std::max(0.0f, (rowWidth - gap * static_cast<float>(actions.size() - 1)) /
                                                 static_cast<float>(actions.size()));
    float x = (bounds.x0 + bounds.x1 - rowWidth) * 0.5f;
    float y = 0.0f;
    const float buttonHeight = CurrentWidgetMetrics().buttonHeight;

    if (presentation == DialogPresentation::Overlay) {
        const auto lines = ui.Wrap(message, std::max(1.0f, screenWidth - 40.0f));
        y = (bounds.y0 + bounds.y1) * 0.5f + 1.0f + static_cast<float>(lines.size()) * 18.0f;
    } else {
        const Rect panel = PanelBounds(ui);
        const float panelWidth = panel.x1 - panel.x0 - 28.0f;
        const float panelGap = CurrentWidgetMetrics().controlGap;
        const float panelButtonWidth =
            std::max(0.0f, (panelWidth - panelGap * static_cast<float>(actions.size() - 1)) /
                               static_cast<float>(actions.size()));
        x = panel.x0 + 14.0f;
        y = panel.y1 - 31.0f;
        std::vector<Rect> result;
        result.reserve(actions.size());
        for (size_t i = 0; i < actions.size(); ++i) {
            const float left = x + static_cast<float>(i) * (panelButtonWidth + panelGap);
            result.push_back({left, left + panelButtonWidth, y, y + CurrentWidgetMetrics().buttonHeight});
        }
        return result;
    }

    std::vector<Rect> result;
    result.reserve(actions.size());
    for (size_t i = 0; i < actions.size(); ++i) {
        const float left = x + static_cast<float>(i) * (buttonWidth + gap);
        result.push_back({left, left + buttonWidth, y, y + buttonHeight});
    }
    return result;
}

bool ConfirmationDialog::IsModalContent(float x, float y) const {
    if (!visible) return false;
    if (presentation == DialogPresentation::Panel) {
        UI ui;
        const Rect p = PanelBounds(ui);
        return x >= p.x0 && x <= p.x1 && y >= p.y0 && y <= p.y1;
    }
    const float centerY = (bounds.y0 + bounds.y1) * 0.5f;
    const float left = bounds.x0 + (bounds.x1 - bounds.x0) * 0.08f;
    const float right = bounds.x1 - (bounds.x1 - bounds.x0) * 0.08f;
    return x >= left && x <= right && y >= centerY - 80.0f && y <= centerY + 100.0f;
}

void ConfirmationDialog::Render(UI& ui) {
    if (!visible) return;
    detail::FlushText();
    ui.Fill(bounds, col::BLACK, presentation == DialogPresentation::Overlay ? 0.70f : 0.58f);

    if (presentation == DialogPresentation::Overlay) {
        const float centerY = (bounds.y0 + bounds.y1) * 0.5f;
        const float textWidth = std::max(1.0f, bounds.x1 - bounds.x0 - 40.0f);
        const auto lines = ui.Wrap(message, std::min(620.0f, textWidth));
        float y = centerY - 70.0f;
        ui.Text(bounds.x0 + 10.0f, bounds.x1 - 10.0f, y, title, CurrentTheme().text, Align::Center, 1.0f);
        y += 22.0f;
        for (const auto& line : lines) {
            ui.Text(bounds.x0 + 12.0f, bounds.x1 - 12.0f, y, line, CurrentTheme().text, Align::Center, 1.0f);
            y += 18.0f;
        }
        if (!warning.empty()) {
            ui.Text(bounds.x0 + 10.0f, bounds.x1 - 10.0f, y + 1.0f, warning, CurrentTheme().warning,
                    Align::Center, 1.0f);
        }
        const auto buttons = ActionBounds(ui);
        for (size_t i = 0; i < buttons.size(); ++i)
            ui.DrawButton(buttons[i], actions[i].label, true, ui.Hovered(buttons[i]),
                          pressedButton == static_cast<int>(i) && ui.MouseDown());
        return;
    }

    const Rect panel = PanelBounds(ui);
    ui.Panel(panel);
    ui.Text(panel.x0 + 14.0f, panel.x1 - 14.0f, panel.y0 + 12.0f, title, CurrentTheme().text, Align::Left, 1.0f);
    const auto lines = ui.Wrap(message, panel.x1 - panel.x0 - 28.0f);
    float y = panel.y0 + 31.0f;
    for (const auto& line : lines) {
        ui.Text(panel.x0 + 14.0f, panel.x1 - 14.0f, y, line, CurrentTheme().muted, Align::Left, 1.0f);
        y += 11.0f;
    }
    if (!warning.empty())
        ui.Text(panel.x0 + 14.0f, panel.x1 - 14.0f, y, warning, CurrentTheme().warning, Align::Left, 1.0f);

    const auto buttons = ActionBounds(ui);
    for (size_t i = 0; i < buttons.size(); ++i)
        ui.DrawButton(buttons[i], actions[i].label, true, ui.Hovered(buttons[i]),
                      pressedButton == static_cast<int>(i) && ui.MouseDown());
}

bool ConfirmationDialog::OnPress(UI& ui) {
    pressedButton = -1;
    const auto buttons = ActionBounds(ui);
    for (size_t i = 0; i < buttons.size(); ++i)
        if (ui.Hovered(buttons[i])) {
            pressedButton = static_cast<int>(i);
            break;
        }
    return true;
}

void ConfirmationDialog::OnRelease(UI& ui, bool) {
    const int button = pressedButton;
    pressedButton = -1;
    if (button < 0) return;
    const auto buttons = ActionBounds(ui);
    if (static_cast<size_t>(button) >= buttons.size() || !ui.Hovered(buttons[static_cast<size_t>(button)])) return;
    ui.Click();
    Activate(static_cast<size_t>(button));
}

bool ConfirmationDialog::OnKey(int vk) {
    if (vk == Key(KeyCode::ESCAPE)) {
        Dismiss();
        return true;
    }
    if ((vk == Key(KeyCode::LEFT) || vk == Key(KeyCode::RIGHT)) && !actions.empty()) {
        focusedAction = std::clamp(focusedAction + (vk == Key(KeyCode::RIGHT) ? 1 : -1), 0,
                                   static_cast<int>(actions.size()) - 1);
        return true;
    }
    if ((vk == Key(KeyCode::ENTER) || vk == Key(KeyCode::SPACE)) && !actions.empty()) {
        Activate(static_cast<size_t>(std::clamp(focusedAction, 0, static_cast<int>(actions.size()) - 1)));
        return true;
    }
    return true;
}

void ConfirmationDialog::Activate(size_t index) {
    if (index >= actions.size()) return;
    auto action = std::move(actions[index].onSelect);
    Dismiss();
    if (action) action();
}

}
