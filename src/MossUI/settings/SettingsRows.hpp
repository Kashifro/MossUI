#pragma once

#include "MossUI/settings/Setting.hpp"
#include "MossUI/widgets/List.hpp"

#include <functional>
#include <string>
#include <vector>

namespace MossUI::settings {

struct Tab {
    std::string title;
    std::vector<Setting> rows;
};

struct RowOptions {
    bool tooltips = true;
    // "<Mod>: <row>" of another row already bound to `code`, or empty. `rowLabel` is the key row being built.
    std::function<std::string(int code, const std::string& rowLabel)> keyConflict;
};

// Top-level categories become tabs; loose rows join a tab called "General" (created if missing).
std::vector<Tab> BuildTabs(std::vector<Setting> rows);

// One bool per row (recursing into categories): changes when a shownIf flips, so a page knows to refill.
void VisibilitySignature(const std::vector<Setting>& rows, std::vector<bool>& signature, bool parentVisible = true);

// Turns rows into option widgets in `list`.
void AddRows(ListWidget& list, const std::vector<Setting>& rows, const RowOptions& options = {});

}
