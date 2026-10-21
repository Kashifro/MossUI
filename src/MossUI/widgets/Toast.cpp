#include "MossUI/widgets/Toast.hpp"

#include "MossUI/core/Types.hpp"
#include "MossUI/core/State.hpp"
#include "MossUI/ui/UI.hpp"

#include <algorithm>
#include <deque>

namespace MossUI {
namespace {

struct Entry {
    std::string message;
    std::chrono::steady_clock::time_point expires;
};

std::deque<Entry>& Entries() {
    static std::deque<Entry> entries;
    return entries;
}

}

void ShowToast(std::string message, std::chrono::milliseconds duration) {
    if (message.empty()) return;
    auto& entries = Entries();
    const auto limit = static_cast<size_t>(CurrentWidgetMetrics().toastQueueLimit);
    while (entries.size() >= limit) entries.pop_front();
    entries.push_back({std::move(message), std::chrono::steady_clock::now() + std::max(duration, std::chrono::milliseconds(1))});
}

namespace detail {

void RenderToasts(UI& ui) {
    auto& entries = Entries();
    const auto now = std::chrono::steady_clock::now();
    for (auto it = entries.begin(); it != entries.end();) {
        if (it->expires <= now) it = entries.erase(it);
        else ++it;
    }
    const auto& metrics = CurrentWidgetMetrics();
    const size_t count = std::min(entries.size(), static_cast<size_t>(metrics.toastMaxVisible));
    float y = ui.Height() - metrics.toastMargin;
    for (size_t i = 0; i < count; ++i) {
        const auto& entry = entries[entries.size() - count + i];
        const float width = std::min(metrics.toastMaxWidth,
                                     std::max(metrics.toastMinWidth,
                                              ui.TextWidth(entry.message) + metrics.toastPadding * 2.0f));
        const Rect box{ui.Width() - width - metrics.toastMargin, ui.Width() - metrics.toastMargin,
                       y - metrics.toastHeight, y};
        ui.Fill(box, CurrentTheme().panel, 0.96f);
        ui.Outline(box, CurrentTheme().accent);
        ui.Text(box.x0 + metrics.toastPadding, box.x1 - metrics.toastPadding,
                box.y0 + (metrics.toastHeight - 10.0f) * 0.5f,
                ui.Trunc(entry.message, width - metrics.toastPadding * 2.0f),
                CurrentTheme().text, Align::Left, 1.0f);
        y -= metrics.toastSpacing;
    }
}

}
}
