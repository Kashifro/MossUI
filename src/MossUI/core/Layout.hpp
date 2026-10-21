#pragma once

#include "MossUI/core/Types.hpp"

#include <algorithm>
#include <array>
#include <vector>

namespace MossUI::Layout {

struct Insets {
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
};

inline float ScaleFor(float width, float height, float referenceWidth = 854.0f, float referenceHeight = 480.0f) {
    const float xScale = width > 0.0f ? width / referenceWidth : 1.0f;
    const float yScale = height > 0.0f ? height / referenceHeight : 1.0f;
    return std::clamp(std::min(xScale, yScale), 0.5f, 2.0f);
}

inline float Unit(float value, float scale) { return value * scale; }

inline Rect Inset(const Rect& rect, Insets insets) {
    return {rect.x0 + insets.left, rect.x1 - insets.right, rect.y0 + insets.top, rect.y1 - insets.bottom};
}

inline Rect Centered(float centerX, float centerY, float width, float height) {
    return {centerX - width * 0.5f, centerX + width * 0.5f, centerY - height * 0.5f, centerY + height * 0.5f};
}

inline std::array<Rect, 2> SplitHorizontal(const Rect& rect, float gap = 0.0f, float firstFraction = 0.5f) {
    const float available = std::max(0.0f, rect.x1 - rect.x0 - gap);
    const float split = rect.x0 + available * std::clamp(firstFraction, 0.0f, 1.0f);
    return {{{rect.x0, split, rect.y0, rect.y1}, {split + gap, rect.x1, rect.y0, rect.y1}}};
}

inline Rect AlignCenter(const Rect& within, float width, float height) {
    return Centered((within.x0 + within.x1) * 0.5f, (within.y0 + within.y1) * 0.5f, width, height);
}

inline Rect TakeTop(Rect& remaining, float height, float gap = 0.0f) {
    const float cut = std::clamp(height, 0.0f, std::max(0.0f, remaining.y1 - remaining.y0));
    const Rect result{remaining.x0, remaining.x1, remaining.y0, remaining.y0 + cut};
    remaining.y0 = std::min(remaining.y1, result.y1 + std::max(0.0f, gap));
    return result;
}

inline Rect TakeBottom(Rect& remaining, float height, float gap = 0.0f) {
    const float cut = std::clamp(height, 0.0f, std::max(0.0f, remaining.y1 - remaining.y0));
    const Rect result{remaining.x0, remaining.x1, remaining.y1 - cut, remaining.y1};
    remaining.y1 = std::max(remaining.y0, result.y0 - std::max(0.0f, gap));
    return result;
}

inline Rect TakeLeft(Rect& remaining, float width, float gap = 0.0f) {
    const float cut = std::clamp(width, 0.0f, std::max(0.0f, remaining.x1 - remaining.x0));
    const Rect result{remaining.x0, remaining.x0 + cut, remaining.y0, remaining.y1};
    remaining.x0 = std::min(remaining.x1, result.x1 + std::max(0.0f, gap));
    return result;
}

inline Rect TakeRight(Rect& remaining, float width, float gap = 0.0f) {
    const float cut = std::clamp(width, 0.0f, std::max(0.0f, remaining.x1 - remaining.x0));
    const Rect result{remaining.x1 - cut, remaining.x1, remaining.y0, remaining.y1};
    remaining.x1 = std::max(remaining.x0, result.x0 - std::max(0.0f, gap));
    return result;
}

inline std::vector<Rect> Columns(const Rect& rect, int count, float gap = 0.0f) {
    if (count <= 0) return {};
    const float width = std::max(0.0f, rect.x1 - rect.x0);
    const float actualGap = count > 1
                                ? std::min(std::max(0.0f, gap), width / static_cast<float>(count - 1))
                                : 0.0f;
    const float columnWidth = std::max(0.0f, (width - actualGap * static_cast<float>(count - 1)) /
                                                  static_cast<float>(count));
    std::vector<Rect> result;
    result.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        const float x0 = rect.x0 + static_cast<float>(i) * (columnWidth + actualGap);
        result.push_back({x0, x0 + columnWidth, rect.y0, rect.y1});
    }
    return result;
}

}
