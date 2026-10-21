#pragma once

#include "MossUI/widgets/Widget.hpp"

namespace MossUI {

class ScrollContainer : public Container {
public:
    void ResetScroll() { scroll.Reset(); }
    void SetSpacing(float value) { spacing = value < 0.0f ? 0.0f : value; }
    float ContentHeight() const;

    void Render(UI& ui) override;
    bool Contains(float x, float y) const override;
    std::shared_ptr<Widget> HitTest(float x, float y) override;
    bool OnPress(UI& ui) override;
    void OnRelease(UI&, bool) override { scroll.drag = false; }
    bool OnScroll(UI&, float x, float y, int dir) override;

private:
    void Layout();
    int VisiblePixels() const;
    int ContentPixels() const;

    ScrollState scroll;
    float spacing = 0.0f;
};

}
