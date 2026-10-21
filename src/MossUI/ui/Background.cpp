#include "MossUI/ui/Background.hpp"

#include <algorithm>

namespace MossUI {

namespace {
struct BadgeLook { Color outline, fill; };

BadgeLook BadgeColors(const std::string& name) {
    if (name == "Library")    return {{16, 116, 84, 255},  {9, 57, 41, 255}};
    if (name == "Deprecated") return {{132, 20, 38, 255},  {83, 12, 23, 255}};
    if (name == "Modpack")    return {{122, 43, 124, 255}, {81, 13, 84, 255}};
    if (name == "Minecraft")  return {{111, 108, 106, 255}, {49, 48, 47, 255}};
    return {{43, 75, 124, 255}, {14, 42, 85, 255}};
}
}

void DrawMenuBackground(UI& ui, const Rect& r, const std::string& texture, float dim) {
    if (!texture.empty() && ui.ImageTiled(r, texture)) {
        ui.Fill(r, col::BLACK, std::clamp(dim, 0.0f, 1.0f));
        return;
    }
    ui.Fill(r, col::DIM, dim);
}

float DrawBadge(UI& ui, float x, float y, const std::string& text) {
    const float w = ui.TextWidth(text) + 6.0f;
    const auto look = BadgeColors(text);
    ui.Fill({x, x + w + 1, y - 1, y + 10}, look.outline);
    ui.Fill({x + 1, x + w, y, y + 9}, look.fill);
    ui.Text(x + 1, x + w, y + 0.5f, text, col::BADGE_TEXT, Align::Center, 1.0f, false);
    return w + 3.0f;
}

void HeaderSeparator(UI& ui, float x0, float x1, float y) {
    ui.Fill({x0, x1, y - 2, y - 1}, col::BLACK);
    ui.Fill({x0, x1, y - 1, y}, Color{55, 55, 55, 255});
}

void FooterSeparator(UI& ui, float x0, float x1, float y) {
    ui.Fill({x0, x1, y, y + 1}, Color{55, 55, 55, 255});
    ui.Fill({x0, x1, y + 1, y + 2}, col::BLACK);
}

void ListFrame(UI& ui, const Rect& r) {
    ui.Fill(r, col::BLACK, 0.45f);
    HeaderSeparator(ui, r.x0, r.x1, r.y0);
    FooterSeparator(ui, r.x0, r.x1, r.y1);
}

}
