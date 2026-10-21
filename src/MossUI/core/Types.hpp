#pragma once

#include <algorithm>
#include <cmath>
#include <string>

namespace MossUI {

struct Rect { float x0, x1, y0, y1; };
struct Color { int r, g, b, a; };
enum class Align { Left, Center, Right };

enum class AccessibleRole { Generic, Button, Tab, List, TextField, Slider, Toggle, Dialog };
struct AccessibilityInfo {
    AccessibleRole role = AccessibleRole::Generic;
    std::string name, description, value;
    bool focused = false, disabled = false;
};

struct Theme {
    Color background{0, 0, 0, 170};
    Color panel{20, 22, 28, 230};
    Color border{90, 92, 100, 255};
    Color text{255, 255, 255, 255};
    Color button{198, 198, 198, 255};
    Color buttonHover{168, 169, 172, 255};
    Color buttonPressed{140, 141, 144, 255};
    Color buttonText{64, 64, 64, 255};
    Color buttonHighlight{240, 240, 240, 255};
    Color buttonShadow{110, 111, 114, 255};
    Color disabled{70, 71, 73, 255};
    Color muted{170, 170, 180, 255};
    Color accent{80, 170, 200, 255};
    Color hover{62, 63, 65, 255};
    Color pressed{92, 93, 96, 255};
    Color link{85, 85, 255, 255};
    Color linkHover{130, 130, 255, 255};
    Color linkPressed{170, 170, 255, 255};
    Color warning{210, 60, 60, 255};
    Color switchOn{47, 122, 31, 255};
    Color switchOnHover{70, 152, 46, 255};
    Color switchOff{66, 67, 69, 255};
    Color switchOffHover{88, 89, 92, 255};
    Color switchOnMark{150, 215, 125, 255};
    Color switchOffMark{160, 161, 164, 255};
    Color switchThumb{192, 192, 194, 255};
    Color switchThumbHover{214, 214, 216, 255};
    Color switchThumbHighlight{244, 244, 246, 255};
    Color switchThumbShadow{112, 113, 116, 255};
    Color sliderTrackBorder{34, 35, 36, 255};
    Color sliderTrack{116, 117, 120, 255};
    Color sliderTrackHover{132, 133, 136, 255};
    Color sliderFill{56, 133, 40, 255};
    Color sliderFillHover{74, 160, 52, 255};
    Color sliderThumb{198, 198, 198, 255};
    Color selection{40, 100, 190, 255};
};

struct AccessibilityOptions {
    float textScale = 1.0f;
};

struct WidgetMetrics {
    float controlHeight = 20.0f;
    float buttonHeight = 20.0f;
    float controlGap = 5.0f;
    float panelPadding = 12.0f;
    float listPadding = 5.0f;
    float focusOutlineWidth = 1.0f;
    float scrollSmoothness = 18.0f;
    float scrollStep = 24.0f;
    float rowHeight = 22.0f;
    float comboPopupMaxHeight = 176.0f;
    float scrollbarMinHandleHeight = 16.0f;
    float tooltipMaxWidth = 190.0f;
    float toastMinWidth = 160.0f;
    float toastMaxWidth = 300.0f;
    float toastMargin = 12.0f;
    float toastHeight = 28.0f;
    float toastSpacing = 34.0f;
    float toastPadding = 10.0f;
    int toastMaxVisible = 3;
    int toastQueueLimit = 8;
};

inline Theme& CurrentTheme() {
    static Theme theme;
    return theme;
}

inline void SetTheme(const Theme& theme) { CurrentTheme() = theme; }
inline void ResetTheme() { CurrentTheme() = Theme{}; }
inline AccessibilityOptions& CurrentAccessibilityOptions() {
    static AccessibilityOptions options;
    return options;
}
inline void SetAccessibilityOptions(const AccessibilityOptions& options) {
    CurrentAccessibilityOptions() = options;
    auto& scale = CurrentAccessibilityOptions().textScale;
    if (!std::isfinite(scale)) scale = 1.0f;
    else scale = std::clamp(scale, 0.75f, 2.0f);
}
inline void ResetAccessibilityOptions() { CurrentAccessibilityOptions() = AccessibilityOptions{}; }
inline WidgetMetrics& CurrentWidgetMetrics() {
    static WidgetMetrics metrics;
    return metrics;
}
inline void SetWidgetMetrics(const WidgetMetrics& metrics) {
    auto clamp = [](float value, float fallback, float min, float max) {
        return std::isfinite(value) ? std::clamp(value, min, max) : fallback;
    };
    auto& current = CurrentWidgetMetrics();
    current = metrics;
    current.controlHeight = clamp(metrics.controlHeight, 20.0f, 8.0f, 128.0f);
    current.buttonHeight = clamp(metrics.buttonHeight, 20.0f, 8.0f, 128.0f);
    current.controlGap = clamp(metrics.controlGap, 5.0f, 0.0f, 64.0f);
    current.panelPadding = clamp(metrics.panelPadding, 12.0f, 0.0f, 128.0f);
    current.listPadding = clamp(metrics.listPadding, 5.0f, 0.0f, 64.0f);
    current.focusOutlineWidth = clamp(metrics.focusOutlineWidth, 1.0f, 0.5f, 8.0f);
    current.scrollSmoothness = clamp(metrics.scrollSmoothness, 18.0f, 1.0f, 40.0f);
    current.scrollStep = clamp(metrics.scrollStep, 24.0f, 1.0f, 256.0f);
    current.rowHeight = clamp(metrics.rowHeight, 22.0f, 8.0f, 128.0f);
    current.comboPopupMaxHeight = clamp(metrics.comboPopupMaxHeight, 176.0f, 32.0f, 1024.0f);
    current.scrollbarMinHandleHeight = clamp(metrics.scrollbarMinHandleHeight, 16.0f, 4.0f, 64.0f);
    current.tooltipMaxWidth = clamp(metrics.tooltipMaxWidth, 190.0f, 80.0f, 800.0f);
    current.toastMinWidth = clamp(metrics.toastMinWidth, 160.0f, 80.0f, 800.0f);
    current.toastMaxWidth = std::max(current.toastMinWidth,
                                     clamp(metrics.toastMaxWidth, 300.0f, 80.0f, 800.0f));
    current.toastMargin = clamp(metrics.toastMargin, 12.0f, 0.0f, 128.0f);
    current.toastHeight = clamp(metrics.toastHeight, 28.0f, 8.0f, 128.0f);
    current.toastSpacing = clamp(metrics.toastSpacing, 34.0f, current.toastHeight, 256.0f);
    current.toastPadding = clamp(metrics.toastPadding, 10.0f, 0.0f, 64.0f);
    current.toastMaxVisible = std::clamp(metrics.toastMaxVisible, 1, 10);
    current.toastQueueLimit = std::clamp(metrics.toastQueueLimit, current.toastMaxVisible, 100);
}
inline void ResetWidgetMetrics() { CurrentWidgetMetrics() = WidgetMetrics{}; }

namespace col {
inline constexpr Color BLACK{0, 0, 0, 255};
inline constexpr Color WHITE{255, 255, 255, 255};
inline constexpr Color GRAY{170, 170, 180, 255};
inline constexpr Color TEXT_GRAY{170, 170, 170, 255};
inline constexpr Color SUMMARY{128, 128, 128, 255};
inline constexpr Color LINK{85, 85, 255, 255};
inline constexpr Color BADGE_TEXT{202, 202, 202, 255};
inline constexpr Color INK{64, 64, 64, 255};
inline constexpr Color DIM{0, 0, 0, 170};
inline constexpr Color NAV{49, 50, 51, 255};
inline constexpr Color PAGE{72, 73, 74, 255};
inline constexpr Color ROW_SEL{92, 93, 96, 255};
inline constexpr Color ROW_HOVER{62, 63, 65, 255};
inline constexpr Color LINE{30, 31, 32, 255};
inline constexpr Color DARK{30, 31, 32, 255};
inline constexpr Color DARK_HOVER{52, 53, 55, 255};
inline constexpr Color HEADER_BG{30, 31, 32, 255};
inline constexpr Color WARN{210, 60, 60, 255};
inline constexpr Color CHANGED{200, 160, 60, 255};
inline constexpr Color ACCENT[4] = {{80, 170, 200, 255}, {150, 90, 190, 255}, {200, 160, 60, 255}, {90, 170, 100, 255}};
}

}
