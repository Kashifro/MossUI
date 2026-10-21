#pragma once

#include "MossUI/ui/UI.hpp"

namespace MossUI {

void DrawMenuBackground(UI& ui, const Rect& r, const std::string& texture, float dim = 0.55f);
float DrawBadge(UI& ui, float x, float y, const std::string& text);
void HeaderSeparator(UI& ui, float x0, float x1, float y);
void FooterSeparator(UI& ui, float x0, float x1, float y);
void ListFrame(UI& ui, const Rect& r);

}
