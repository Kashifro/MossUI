#pragma once

#include "MossUI/widgets/Basic.hpp"
#include "MossUI/widgets/ComboBox.hpp"
#include "MossUI/widgets/Widget.hpp"

namespace MossUI {

class SeparatorOptionWidget : public Widget {
public:
    explicit SeparatorOptionWidget(std::string label) : label(std::move(label)) {}
    void Render(UI& ui) override;
    float PreferredHeight() const override { return 24.0f; }
    bool FullWidth() const override { return true; }

    std::string label;
};

class OptionWidget : public Widget {
public:
    std::string label;
    bool CanFocus() const override { return true; }
    AccessibleRole Role() const override { return AccessibleRole::Generic; }
    bool OnKey(int vk) override { return OnNavKey(vk); }
    std::string AccessibleValue() const override { return label; }

protected:
    Rect Button() const { return {bounds.x1 - 98, bounds.x1 - 8, bounds.y0 + 5, bounds.y0 + 25}; }
    void DrawLabel(UI& ui, float y, float width) const;
    void DrawTooltip(UI& ui) const;
};

class ToggleOptionWidget : public OptionWidget {
public:
    AccessibleRole Role() const override { return AccessibleRole::Toggle; }
    std::string AccessibleValue() const override { return get && get() ? "On" : "Off"; }
    ToggleOptionWidget(std::string label, std::function<bool()> get, std::function<void(bool)> set);
    void Render(UI& ui) override;
    bool OnPress(UI& ui) override;
    void OnRelease(UI& ui, bool inside) override;
    bool OnNavKey(int vk) override;

private:
    std::function<bool()> get;
    std::function<void(bool)> set;
    bool armed = false;
};

class SliderOptionWidget : public OptionWidget {
public:
    AccessibleRole Role() const override { return AccessibleRole::Slider; }
    std::string AccessibleValue() const override { return get ? std::to_string(get()) : std::string(); }
    SliderOptionWidget(std::string label, std::function<float()> get, std::function<void(float)> set, float min, float max,
                       float step = 0, int decimals = 0, std::string suffix = {});
    void Render(UI& ui) override;
    float PreferredHeight() const override { return 46.0f; }
    bool OnPress(UI& ui) override;
    void OnDrag(UI& ui) override { Apply(ui); }
    void OnRelease(UI&, bool) override { dragging = false; }
    bool OnScroll(UI&, float x, float y, int dir) override;
    bool OnNavKey(int vk) override;

private:
    void Apply(UI& ui);
    void StepBy(int dir);
    Rect Track() const;
    std::function<float()> get;
    std::function<void(float)> set;
    float min, max, step;
    int decimals;
    std::string suffix;
    bool dragging = false;
};

class ChoiceOptionWidget : public OptionWidget {
public:
    AccessibleRole Role() const override { return AccessibleRole::Button; }
    std::string AccessibleValue() const override {
        const int index = get ? get() : -1;
        return index >= 0 && index < static_cast<int>(combo.options.size()) ? combo.options[static_cast<size_t>(index)] : std::string();
    }
    ChoiceOptionWidget(std::string label, std::vector<std::string> options,
                       std::function<int()> get, std::function<void(int)> set);
    void Render(UI& ui) override;
    float PreferredHeight() const override;
    bool OnPress(UI& ui) override;
    void OnDrag(UI& ui) override { combo.OnDrag(ui); }
    void OnRelease(UI& ui, bool inside) override { combo.OnRelease(ui, inside); }
    bool OnScroll(UI& ui, float x, float y, int dir) override { return combo.OnScroll(ui, x, y, dir); }
    void OnOutsideClick() override { combo.OnOutsideClick(); }
    bool OnNavKey(int vk) override;

private:
    void Sync();
    ComboBox combo;
    std::function<int()> get;
    std::function<void(int)> set;
};

class TextOptionWidget : public OptionWidget {
public:
    AccessibleRole Role() const override { return AccessibleRole::TextField; }
    std::string AccessibleValue() const override { return get ? get() : std::string(); }
    TextOptionWidget(std::string label, std::function<std::string()> get,
                     std::function<void(const std::string&)> set, std::string placeholder = {});
    void Render(UI& ui) override;
    bool OnPress(UI& ui) override;
    void OnRelease(UI& ui, bool inside) override { field.OnRelease(ui, inside); }
    bool WantsFocus() const override { return focused || field.WantsFocus(); }
    void OnFocus(bool gained) override;
    void OnOutsideClick() override { field.OnFocus(false); }
    bool OnKey(int vk) override;
    bool OnNavKey(int vk) override;

private:
    Rect FieldRect() const { return {bounds.x1 - 128, bounds.x1 - 8, bounds.y0 + 5, bounds.y0 + 25}; }
    TextFieldWidget field;
    std::function<std::string()> get;
    std::function<void(const std::string&)> set;
};

class KeyBindOptionWidget : public OptionWidget {
public:
    KeyBindOptionWidget(std::string label, std::function<int()> get, std::function<void(int)> set);
    void Render(UI& ui) override;
    bool OnPress(UI& ui) override;
    bool WantsFocus() const override { return focused || capturing; }
    void OnFocus(bool gained) override { Widget::OnFocus(gained); if (!gained) capturing = false; }
    bool OnKey(int vk) override;
    void OnKeyUp(int vk) override;
    bool OnNavKey(int vk) override;

    std::function<std::string(int)> conflict;

private:
    std::function<int()> get;
    std::function<void(int)> set;
    bool capturing = false;
    int pendingMod = 0;
};

}
