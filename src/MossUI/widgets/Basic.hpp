#pragma once

#include "MossUI/widgets/Widget.hpp"

#include <chrono>

namespace MossUI {

class LabelWidget : public Widget {
public:
    LabelWidget(std::string text, Align align = Align::Left, const Color& color = col::WHITE, float alpha = 1.0f)
        : text(std::move(text)), align(align), color(color), alpha(alpha) {}
    void Render(UI& ui) override;

    std::string text;
    Align align;
    Color color;
    float alpha;
};

class ButtonWidget : public Widget {
public:
    enum class Style { Light, Dark };
    ButtonWidget(std::string label, std::function<void()> onPress, Style style = Style::Light)
        : label(std::move(label)), onPress(std::move(onPress)), style(style) {}
    void Render(UI& ui) override;
    bool OnPress(UI&) override { pressed = enabled; return enabled; }
    void OnRelease(UI& ui, bool inside) override;
    bool CanFocus() const override { return enabled; }
    AccessibleRole Role() const override { return AccessibleRole::Button; }
    std::string AccessibleValue() const override { return label; }
    bool Disabled() const override { return !enabled; }
    bool OnKey(int vk) override;

    std::string label;
    std::function<void()> onPress;
    Style style;
    bool enabled = true;

protected:
    bool pressed = false;
};

class ConfirmButton : public ButtonWidget {
public:
    ConfirmButton(std::string text, std::function<void()> action);
    void Render(UI& ui) override;
    void SetBase(const std::string& text) { base = text; if (!armed) label = text; }

    bool confirm = true;
    std::string glyph;

private:
    void Disarm() { armed = false; label = base; }
    std::string base;
    std::function<void()> run;
    bool armed = false;
    std::chrono::steady_clock::time_point armedAt{};
};

class IconButton : public ButtonWidget {
public:
    IconButton(std::string asset, std::function<void()> onPress)
        : ButtonWidget("", std::move(onPress)), asset(std::move(asset)) {}
    void Render(UI& ui) override;

    std::string asset;
};

class TextFieldWidget : public Widget {
public:
    explicit TextFieldWidget(std::string placeholder) : placeholder(std::move(placeholder)) {}
    void Render(UI& ui) override;
    bool OnPress(UI& ui) override;
    void OnRelease(UI&, bool) override { state.drag = false; }
    bool CanFocus() const override { return true; }
    bool WantsFocus() const override { return state.focused; }
    void OnFocus(bool gained) override { Widget::OnFocus(gained); state.focused = gained; state.drag = false; }
    bool OnKey(int vk) override;

    TextFieldState state;
    std::string placeholder;
};

class SearchField : public TextFieldWidget {
public:
    using TextFieldWidget::TextFieldWidget;
    bool OnKey(int vk) override;
};

}
