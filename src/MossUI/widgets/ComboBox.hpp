#pragma once

#include "MossUI/widgets/Widget.hpp"

namespace MossUI {

class ComboBox : public Widget {
public:
    std::vector<std::string> options;
    int selected = 0;
    std::function<void(int)> onChange;
    bool Expanded() const { return expanded; }

    void Render(UI& ui) override;
    bool Contains(float x, float y) const override;
    std::shared_ptr<Widget> HitTest(float x, float y) override;
    bool OnPress(UI& ui) override;
    void OnDrag(UI& ui) override;
    void OnRelease(UI&, bool) override { scroll.drag = false; }
    bool OnScroll(UI&, float x, float y, int direction) override;
    bool CanFocus() const override { return true; }
    void OnOutsideClick() override { expanded = false; }
    bool OnKey(int vk) override;

private:
    Rect PopupBounds() const;
    bool expanded = false;
    float screenHeight = 0.0f;
    ScrollState scroll;
};

}
