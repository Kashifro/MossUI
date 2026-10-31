#include "MossUI/widgets/Box.hpp"

#include <algorithm>

namespace MossUI {

void Box::Arrange() {
    const Rect inner = Layout::Inset(bounds, padding);
    const bool horizontal = axis == Axis::Horizontal;
    const float total = horizontal ? inner.x1 - inner.x0 : inner.y1 - inner.y0;

    auto slotOf = [&](size_t i) {
        Slot s = i < slots.size() ? slots[i] : Slot{};
        if (s.fixed <= 0.0f && s.flex <= 0.0f) s.flex = 1.0f;
        return s;
    };

    size_t shown = 0;
    float fixedSum = 0.0f, flexSum = 0.0f;
    for (size_t i = 0; i < children.size(); ++i) {
        if (!children[i]->visible) continue;
        const Slot s = slotOf(i);
        ++shown;
        fixedSum += s.fixed;
        flexSum += s.flex;
    }
    if (shown == 0) return;

    const float gaps = gap * static_cast<float>(shown - 1);
    const float free = std::max(0.0f, total - fixedSum - gaps);

    float pos = horizontal ? inner.x0 : inner.y0;
    for (size_t i = 0; i < children.size(); ++i) {
        if (!children[i]->visible) continue;
        const Slot s = slotOf(i);
        const float size = s.fixed > 0.0f ? s.fixed : (flexSum > 0.0f ? free * s.flex / flexSum : 0.0f);
        children[i]->bounds = horizontal ? Rect{pos, pos + size, inner.y0, inner.y1}
                                         : Rect{inner.x0, inner.x1, pos, pos + size};
        pos += size + gap;
    }
}

}
