#pragma once

#include "MossUI/widgets/Options.hpp"

namespace MossUI {

class ColorOptionWidget : public OptionWidget {
public:
    ColorOptionWidget(std::string label, std::function<int()> get, std::function<void(int)> set);
    void Render(UI& ui) override;
    float PreferredHeight() const override { return 104.0f; }
    bool OnPress(UI& ui) override;
    void OnDrag(UI& ui) override { Apply(ui); }
    void OnRelease(UI&, bool) override { drag = Drag::None; }
    bool OnNavKey(int vk) override;

private:
    enum class Drag { None, Sv, Hue };
    Rect SvBox() const { return {bounds.x0 + 10, bounds.x0 + 138, bounds.y0 + 28, bounds.y0 + 96}; }
    Rect HueBar() const { auto s = SvBox(); return {s.x1 + 10, s.x1 + 26, s.y0, s.y1}; }
    void Apply(UI& ui);
    void Commit();
    void Sync();
    std::function<int()> get;
    std::function<void(int)> set;
    float h = 0, s = 0, v = 1;
    int lastRgb = -1;
    Drag drag = Drag::None;
};

}
