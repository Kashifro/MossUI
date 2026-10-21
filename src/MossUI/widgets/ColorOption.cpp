#include "MossUI/widgets/ColorOption.hpp"

#include "MossUI/core/Keys.hpp"
#include "MossUI/core/State.hpp"
#include "MossUI/screen/Screen.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>

namespace MossUI {

namespace {
void HsvToRgb(float h, float s, float v, int& r, int& g, int& b) {
    h = h - std::floor(h);
    const float k = h * 6.0f, f = k - std::floor(k);
    const float p = v * (1 - s), q = v * (1 - s * f), t = v * (1 - s * (1 - f));
    float R, G, B;
    switch (int(k) % 6) {
        case 0 : R = v; G = t; B = p; break;
        case 1 : R = q; G = v; B = p; break;
        case 2 : R = p; G = v; B = t; break;
        case 3 : R = p; G = q; B = v; break;
        case 4 : R = t; G = p; B = v; break;
        default : R = v; G = p; B = q; break;
    }
    r = int(std::lround(R * 255)); g = int(std::lround(G * 255)); b = int(std::lround(B * 255));
}
void RgbToHsv(int r, int g, int b, float& h, float& s, float& v) {
    const float R = r / 255.0f, G = g / 255.0f, B = b / 255.0f;
    const float mx = std::max({R, G, B}), mn = std::min({R, G, B}), d = mx - mn;
    v = mx; s = mx > 0 ? d / mx : 0;
    if (d <= 0) return;
    if (mx == R) h = std::fmod((G - B) / d, 6.0f) / 6.0f;
    else if (mx == G) h = ((B - R) / d + 2.0f) / 6.0f;
    else h = ((R - G) / d + 4.0f) / 6.0f;
    if (h < 0) h += 1.0f;
}
int ParseHexColor(const std::string& raw) {
    std::string hex;
    for (unsigned char c : raw) {
        if (std::isspace(c) || (hex.empty() && c == '#')) continue;
        if (!std::isxdigit(c)) return -1;
        hex.push_back(static_cast<char>(c));
    }
    if (hex.size() != 6) return -1;
    int value = 0;
    for (char c : hex) {
        const int digit = c >= '0' && c <= '9' ? c - '0' : (std::tolower(static_cast<unsigned char>(c)) - 'a' + 10);
        value = (value << 4) | digit;
    }
    return value;
}
}

ColorOptionWidget::ColorOptionWidget(std::string text, std::function<int()> get, std::function<void(int)> set)
    : get(std::move(get)), set(std::move(set)) {
    label = std::move(text);
}

void ColorOptionWidget::Sync() {
    const int cur = (get ? get() : 0) & 0xFFFFFF;
    if (drag != Drag::None || cur == lastRgb) return;
    lastRgb = cur;
    RgbToHsv((cur >> 16) & 255, (cur >> 8) & 255, cur & 255, h, s, v);
}

void ColorOptionWidget::Commit() {
    int r, g, b;
    HsvToRgb(h, s, v, r, g, b);
    lastRgb = (r << 16) | (g << 8) | b;
    if (set) set(lastRgb);
}

void ColorOptionWidget::Render(UI& ui) {
    Sync();
    const int cur = lastRgb < 0 ? 0 : lastRgb;
    DrawLabel(ui, 6, bounds.x1 - bounds.x0 - 100);
    const Rect sv = SvBox(), hue = HueBar();
    {
        int r, g, b;
        HsvToRgb(h, 1, 1, r, g, b);
        ui.Fill(sv, Color{r, g, b, 255});
        constexpr int N = 16;
        const float sw = (sv.x1 - sv.x0) / N, sh = (sv.y1 - sv.y0) / N;
        for (int i = 0; i < N; ++i)
            ui.Fill({sv.x0 + i * sw, sv.x0 + (i + 1) * sw, sv.y0, sv.y1}, col::WHITE, 1.0f - (i + 0.5f) / N);
        for (int j = 0; j < N; ++j)
            ui.Fill({sv.x0, sv.x1, sv.y0 + j * sh, sv.y0 + (j + 1) * sh}, col::BLACK, (j + 0.5f) / N);
    }
    constexpr int HN = 17;
    const float hh = (hue.y1 - hue.y0) / HN;
    for (int j = 0; j < HN; ++j) {
        int r, g, b;
        HsvToRgb((j + 0.5f) / HN, 1, 1, r, g, b);
        ui.Fill({hue.x0, hue.x1, hue.y0 + j * hh, hue.y0 + (j + 1) * hh}, Color{r, g, b, 255});
    }
    ui.Outline(sv, col::LINE); ui.Outline(hue, col::LINE);
    const float mx = sv.x0 + s * (sv.x1 - sv.x0), my = sv.y0 + (1 - v) * (sv.y1 - sv.y0);
    ui.Outline({mx - 3, mx + 3, my - 3, my + 3}, col::WHITE);
    const float hy = hue.y0 + h * (hue.y1 - hue.y0);
    ui.Fill({hue.x0 - 2, hue.x1 + 2, hy - 1, hy + 1}, col::WHITE);
    const Rect pv{hue.x1 + 14, hue.x1 + 14 + 44, sv.y0, sv.y0 + 30};
    ui.Fill(pv, Color{(cur >> 16) & 255, (cur >> 8) & 255, cur & 255, 255});
    ui.Outline(pv, col::LINE);
    char hex[16];
    std::snprintf(hex, sizeof(hex), "#%06X", cur);
    ui.Text(pv.x0 - 6, pv.x1 + 6, pv.y1 + 6, hex, col::WHITE, Align::Center, 1.0f);
    DrawTooltip(ui);
}

bool ColorOptionWidget::OnPress(UI& ui) {
    const Rect sv = SvBox(), hue = HueBar();
    const Rect hueHit{hue.x0 - 4, hue.x1 + 4, hue.y0, hue.y1};
    drag = ui.Hovered(sv) ? Drag::Sv : ui.Hovered(hueHit) ? Drag::Hue : Drag::None;
    if (drag != Drag::None) Apply(ui);
    return drag != Drag::None;
}

void ColorOptionWidget::Apply(UI& ui) {
    if (drag == Drag::Sv) {
        const Rect sv = SvBox();
        s = std::clamp((ui.MouseX() - sv.x0) / (sv.x1 - sv.x0), 0.0f, 1.0f);
        v = 1.0f - std::clamp((ui.MouseY() - sv.y0) / (sv.y1 - sv.y0), 0.0f, 1.0f);
    } else if (drag == Drag::Hue) {
        const Rect hue = HueBar();
        h = std::clamp((ui.MouseY() - hue.y0) / (hue.y1 - hue.y0), 0.0f, 0.999f);
    } else return;
    Commit();
}

bool ColorOptionWidget::OnNavKey(int vk) {
    if (CtrlHeld() && vk == Key(KeyCode::V) && detail::S().backend) {
        const int parsed = ParseHexColor(detail::S().backend->ClipboardGet());
        if (parsed >= 0) {
            lastRgb = -1;
            if (set) set(parsed);
            Sync();
        }
        return true;
    }
    if (CtrlHeld() && vk == Key(KeyCode::C) && detail::S().backend) {
        Sync();
        char hex[16];
        std::snprintf(hex, sizeof(hex), "#%06X", lastRgb < 0 ? 0 : lastRgb);
        detail::S().backend->ClipboardSet(hex);
        return true;
    }
    if (vk != Key(KeyCode::LEFT) && vk != Key(KeyCode::RIGHT)) return false;
    Sync();
    const float step = (ShiftHeld() ? 6.0f : 1.0f) / 36.0f;
    h += (vk == Key(KeyCode::RIGHT) ? step : -step);
    h -= std::floor(h);
    Commit();
    return true;
}

}
