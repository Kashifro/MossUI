#pragma once

#include "MossUI/widgets/Widget.hpp"

namespace MossUI {

class SelectableList : public Widget {
public:
    std::vector<std::string> items;
    int selected = -1;
    std::function<void(int)> onSelect;
    void ResetScroll() { scroll.Reset(); }

    void Render(UI& ui) override;
    bool Contains(float x, float y) const override;
    std::shared_ptr<Widget> HitTest(float x, float y) override;
    bool OnPress(UI& ui) override;
    void OnDrag(UI& ui) override;
    void OnRelease(UI& ui, bool inside) override;
    bool OnScroll(UI&, float x, float y, int dir) override;
    bool CanFocus() const override { return true; }
    bool OnKey(int vk) override;

private:
    int VisibleRows() const;
    int ItemAt(float y) const;
    ScrollState scroll;
    int pressed = -1;
};

}
