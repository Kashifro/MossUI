#pragma once

#include "MossUI/core/Backend.hpp"
#include "MossUI/ui/UI.hpp"

#include <filesystem>
#include <map>
#include <vector>

namespace MossUI::detail {

struct State {
    Backend* backend = nullptr;
    Host* host = nullptr;
    float w = 0, h = 0;
    float mx = -1, my = -1;
    bool hasMouse = false, down = false, inGame = false, hoverBlocked = false;
    bool shift = false, ctrl = false, alt = false;
    std::vector<TextRun> texts;
    std::vector<Rect> clips;
    std::string tip;
    std::filesystem::path assetDir;
    std::map<std::string, std::string> assetCache;
};

State& S();

void FlushText();
void DrawTooltipBox();
void RenderToasts(UI& ui);

void TextKey(TextFieldState& f, int key);
void SelectAll(TextFieldState& f);
void PlaceCaret(TextFieldState& f, float mouseX, bool extend);

}
