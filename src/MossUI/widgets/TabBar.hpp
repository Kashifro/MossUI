#pragma once

#include "MossUI/widgets/Widget.hpp"

namespace MossUI {

class TabBar : public Widget {
public:
    std::vector<std::string> tabs;
    int selected = 0;
    std::function<void(int)> onSelect;

    void Render(UI& ui) override;
    bool OnPress(UI& ui) override;
    void OnRelease(UI& ui, bool inside) override;
    bool CanFocus() const override { return true; }
    AccessibleRole Role() const override { return AccessibleRole::Tab; }
    bool OnKey(int vk) override;

private:
    int TabAt(float x) const;
    int pressed = -1;
};

class TabListView : public Widget {
public:
    static constexpr float ItemH = 21.0f;
    std::vector<std::string>* titles = nullptr;
    std::function<int()> current;
    std::function<void(int)> onSelect;
    bool CanFocus() const override { return titles && !titles->empty(); }
    AccessibleRole Role() const override { return AccessibleRole::Tab; }

    void Render(UI& ui) override;
    bool OnPress(UI& ui) override;
    void OnRelease(UI& ui, bool inside) override;
    bool OnKey(int key) override;

private:
    Rect Row(int i) const {
        const float y = bounds.y0 + 1.0f + static_cast<float>(i) * ItemH;
        return {bounds.x0 + 1.0f, bounds.x1 - 1.0f, y, y + ItemH};
    }
    int pressed = -1;
};

}
