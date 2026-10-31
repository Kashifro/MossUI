#include "MossUI/settings/SettingsRows.hpp"

#include "MossUI/widgets/Basic.hpp"
#include "MossUI/widgets/ColorOption.hpp"
#include "MossUI/widgets/Options.hpp"

#include <algorithm>
#include <iterator>

namespace MossUI::settings {

std::vector<Tab> BuildTabs(std::vector<Setting> rows) {
    std::vector<Tab> tabs;
    std::vector<Setting> loose;
    for (auto& r : rows) {
        if (r.IsCategory()) tabs.push_back({r.label, std::move(r.children)});
        else loose.push_back(std::move(r));
    }
    if (tabs.empty()) return {{{}, std::move(loose)}};
    if (loose.empty()) return tabs;

    const auto general = std::find_if(tabs.begin(), tabs.end(), [](const Tab& t) { return t.title == "General"; });
    if (general == tabs.end()) tabs.insert(tabs.begin(), Tab{"General", std::move(loose)});
    else general->rows.insert(general->rows.end(), std::make_move_iterator(loose.begin()),
                              std::make_move_iterator(loose.end()));
    return tabs;
}

void VisibilitySignature(const std::vector<Setting>& rows, std::vector<bool>& signature, bool parentVisible) {
    for (const auto& r : rows) {
        const bool visible = parentVisible && (!r.shownIf || r.shownIf());
        signature.push_back(visible);
        if (r.IsCategory()) VisibilitySignature(r.children, signature, visible);
    }
}

namespace {

// Builds the widget for one value row; null for rows without one (action, section, category).
struct WidgetFor {
    const Setting& setting;
    const RowOptions& options;

    std::shared_ptr<OptionWidget> operator()(const row::Toggle& r) const {
        return std::make_shared<ToggleOptionWidget>(setting.label, r.get, r.set);
    }
    std::shared_ptr<OptionWidget> operator()(const row::Slider& r) const {
        return std::make_shared<SliderOptionWidget>(setting.label, r.get, r.set, r.min, r.max, r.step, r.decimals, r.unit);
    }
    std::shared_ptr<OptionWidget> operator()(const row::Color& r) const {
        return std::make_shared<ColorOptionWidget>(setting.label, r.get, r.set);
    }
    std::shared_ptr<OptionWidget> operator()(const row::Choice& r) const {
        return std::make_shared<ChoiceOptionWidget>(setting.label, r.options, r.get, r.set);
    }
    std::shared_ptr<OptionWidget> operator()(const row::Text& r) const {
        return std::make_shared<TextOptionWidget>(setting.label, r.get, r.set, r.placeholder);
    }
    std::shared_ptr<OptionWidget> operator()(const row::Key& r) const {
        auto key = std::make_shared<KeyBindOptionWidget>(setting.label, r.get, r.set);
        if (options.keyConflict)
            key->conflict = [fn = options.keyConflict, label = setting.label](int code) { return fn(code, label); };
        return key;
    }
    std::shared_ptr<OptionWidget> operator()(const row::Action&) const { return nullptr; }
    std::shared_ptr<OptionWidget> operator()(const row::Section&) const { return nullptr; }
    std::shared_ptr<OptionWidget> operator()(const row::Category&) const { return nullptr; }
};

}

void AddRows(ListWidget& list, const std::vector<Setting>& rows, const RowOptions& options) {
    for (const auto& r : rows) {
        if (r.shownIf && !r.shownIf()) continue;

        if (r.IsCategory()) {
            list.Add<SeparatorOptionWidget>(r.label);
            AddRows(list, r.children, options);
            continue;
        }
        if (std::holds_alternative<row::Section>(r.data)) {
            list.Add<SeparatorOptionWidget>(r.label);
            continue;
        }
        if (const auto* action = r.As<row::Action>()) {
            auto button = std::make_shared<ConfirmButton>(r.label, action->run);
            button->confirm = action->confirm;
            if (options.tooltips) button->tooltip = r.tooltip;
            button->Accessible(r.label, r.tooltip);
            list.Add(std::move(button));
            continue;
        }

        auto widget = std::visit(WidgetFor{r, options}, r.data);
        if (options.tooltips) widget->tooltip = r.tooltip;
        widget->Accessible(r.label, r.tooltip);
        list.Add(std::move(widget));
    }
}

}
