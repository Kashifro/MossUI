#include "MossUI/ui/UI.hpp"

#include "MossUI/core/State.hpp"

#include <algorithm>

namespace MossUI {

namespace {
using detail::S;
}

void UI::DrawButton(const Rect& r, const std::string& label, bool enabled, bool hovered, bool pressed) {
    const auto& theme = CurrentTheme();
    const bool hov = enabled && hovered;
    const bool down = hov && pressed;
    const Color face = !enabled ? theme.disabled : down ? theme.buttonPressed : hov ? theme.buttonHover : theme.button;
    const Color lit = down ? theme.buttonShadow : theme.buttonHighlight;
    const Color shade = down ? theme.buttonHighlight : theme.buttonShadow;
    Fill(r, col::BLACK);
    const Rect in{r.x0 + 1, r.x1 - 1, r.y0 + 1, r.y1 - 1};
    Fill(in, face);
    if (enabled) {
        Fill({in.x0, in.x1, in.y0, in.y0 + 1}, lit);
        Fill({in.x0, in.x0 + 1, in.y0, in.y1}, lit);
        Fill({in.x0 + 1, in.x1, in.y1 - 1, in.y1}, shade);
        Fill({in.x1 - 1, in.x1, in.y0 + 1, in.y1}, shade);
    }
    const std::string text = Trunc(label, std::max(0.0f, r.x1 - r.x0 - 8.0f));
    Text(r.x0 + 4.0f, r.x1 - 4.0f, r.y0 + ((r.y1 - r.y0) - 8.0f) * 0.5f + (down ? 1.0f : 0.0f), text,
         enabled ? theme.buttonText : theme.muted,
         Align::Center, 0.9f, false);
}

void UI::DrawDarkButton(const Rect& r, const std::string& label, bool hovered, const Color* outline) {
    const auto& theme = CurrentTheme();
    Fill(r, theme.panel);
    Outline(r, hovered ? theme.text : outline ? *outline : theme.border);
    if (hovered) Fill({r.x0 + 1, r.x1 - 1, r.y0 + 1, r.y1 - 1}, theme.hover);
    const std::string text = Trunc(label, std::max(0.0f, r.x1 - r.x0 - 8.0f));
    Text(r.x0 + 4.0f, r.x1 - 4.0f, r.y0 + ((r.y1 - r.y0) - 8.0f) * 0.5f, text, theme.text, Align::Center);
}

Rect UI::SwitchHit(const Rect& row) { return {row.x1 - 46, row.x1 - 4, row.y0, row.y1}; }

void UI::DrawSwitch(const Rect& row, bool on) {
    const auto& theme = CurrentTheme();
    const float x0 = row.x1 - 40, y0 = (row.y0 + row.y1) * 0.5f - 8;
    const bool hov = Hovered(SwitchHit(row));
    const Color blk = col::BLACK;
    const Rect whole{x0, x0 + 30, y0, y0 + 16};
    const Rect body = on ? Rect{x0, x0 + 17, y0, y0 + 16} : Rect{x0 + 13, x0 + 30, y0, y0 + 16};
    const Rect thumb = on ? Rect{x0 + 16, x0 + 30, y0, y0 + 16} : Rect{x0, x0 + 14, y0, y0 + 16};
    Fill(body, blk);
    const Rect bi{body.x0 + 1, body.x1 - 1, body.y0 + 1, body.y1 - 1};
    Fill(bi, on ? (hov ? theme.switchOnHover : theme.switchOn)
                : (hov ? theme.switchOffHover : theme.switchOff));
    const float bcx = (bi.x0 + bi.x1) * 0.5f, bcy = (bi.y0 + bi.y1) * 0.5f;
    if (on) {
        Fill({bcx - 1, bcx + 1, bcy - 4, bcy + 4}, theme.switchOnMark);
    } else {
        const Color g = theme.switchOffMark;
        Fill({bcx - 3, bcx + 3, bcy - 3, bcy - 2}, g);
        Fill({bcx - 3, bcx + 3, bcy + 2, bcy + 3}, g);
        Fill({bcx - 3, bcx - 2, bcy - 3, bcy + 3}, g);
        Fill({bcx + 2, bcx + 3, bcy - 3, bcy + 3}, g);
    }
    Fill(thumb, blk);
    const Rect ti{thumb.x0 + 1, thumb.x1 - 1, thumb.y0 + 1, thumb.y1 - 1};
    Fill(ti, hov ? theme.switchThumbHover : theme.switchThumb);
    Fill({ti.x0, ti.x1, ti.y0, ti.y0 + 1}, theme.switchThumbHighlight);
    Fill({ti.x0, ti.x0 + 1, ti.y0, ti.y1}, theme.switchThumbHighlight);
    Fill({ti.x0 + 1, ti.x1, ti.y1 - 1, ti.y1}, theme.switchThumbShadow);
    Fill({ti.x1 - 1, ti.x1, ti.y0 + 1, ti.y1}, theme.switchThumbShadow);
    if (hov) Outline(whole, col::WHITE);
}

Rect UI::SliderHit(const Rect& area) { return {area.x0 - 6, area.x1 + 6, area.y0, area.y1}; }

void UI::DrawSlider(const Rect& area, float value, float lo, float hi, float step, bool hov) {
    const auto& theme = CurrentTheme();
    const float tx0 = area.x0, tx1 = area.x1, cy = (area.y0 + area.y1) * 0.5f;
    const float v = std::clamp(value, lo, hi);
    const float t = hi > lo ? (v - lo) / (hi - lo) : 0.0f;
    const float hw = 10.0f, hx = tx0 + hw * 0.5f + (tx1 - tx0 - hw) * t;
    const Color frame = theme.sliderTrackBorder;
    const Rect track{tx0, tx1, cy - 4, cy + 4};
    Fill(track, frame);
    Fill({track.x0 + 1, track.x1 - 1, track.y0 + 1, track.y1 - 1},
         hov ? theme.sliderTrackHover : theme.sliderTrack);
    Fill({track.x0 + 1, hx, track.y0 + 1, track.y1 - 1}, hov ? theme.sliderFillHover : theme.sliderFill);
    const int steps = step > 0 ? static_cast<int>((hi - lo) / step + 0.5f) : 0;
    if (steps > 1 && steps <= 12)
        for (int k = 1; k < steps; ++k) {
            const float nx = tx0 + hw * 0.5f + (tx1 - tx0 - hw) * static_cast<float>(k) / static_cast<float>(steps);
            Fill({nx, nx + 1, track.y0 + 1, track.y1 - 1}, frame);
        }
    const Rect hr{hx - hw * 0.5f, hx + hw * 0.5f, cy - 8, cy + 8};
    if (!S().backend->Skinned(hr, hov ? Skin::SliderButtonHover : Skin::SliderButton)) {
        Fill(hr, frame);
        Fill({hr.x0 + 1, hr.x1 - 1, hr.y0 + 1, hr.y1 - 1}, theme.sliderThumb);
    }
}

void UI::DrawScrollbar(ScrollState& sc, const Rect& rail, int visible, int total) {
    auto& s = S();
    sc.max = std::max(0, total - visible);
    sc.pos = std::clamp(sc.pos, 0, sc.max);
    sc.rail = {0, 0, 0, 0};
    if (sc.max <= 0) return;
    sc.rail = rail;
    const float railH = sc.rail.y1 - sc.rail.y0;
    sc.handleH = std::max(CurrentWidgetMetrics().scrollbarMinHandleHeight,
                          railH * static_cast<float>(visible) / static_cast<float>(total));
    if (sc.drag && s.down && s.hasMouse) {
        const float frac = std::clamp((s.my - sc.rail.y0 - sc.handleH * 0.5f) / std::max(1.0f, railH - sc.handleH), 0.0f, 1.0f);
        sc.pos = static_cast<int>(frac * static_cast<float>(sc.max) + 0.5f);
        sc.visualPos = static_cast<float>(sc.pos);
        sc.visualInitialized = true;
    }
    const float visualPos = sc.UpdateVisualPosition();
    const float hy = sc.rail.y0 + (railH - sc.handleH) * visualPos / static_cast<float>(sc.max);
    Fill(sc.rail, col::LINE);
    const Rect hr{sc.rail.x0, sc.rail.x1, hy, hy + sc.handleH};
    if (!S().backend->Skinned(hr, Skin::ScrollHandle)) Fill(hr, col::WHITE, 0.9f);
}

}
