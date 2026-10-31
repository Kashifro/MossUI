#pragma once

#include "MossUI/settings/Store.hpp"

#include <cmath>
#include <functional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace MossUI {

// One row type per kind of setting. A Setting holds exactly one of these, so a row can only carry what its kind needs.
namespace row {
struct Toggle   { std::function<bool()> get; std::function<void(bool)> set; bool def = false; };
struct Key      { std::function<int()> get; std::function<void(int)> set; int def = 0; };   // raw key code
struct Slider   { std::function<float()> get; std::function<void(float)> set;
                  float min = 0, max = 1, step = 0, def = 0; int decimals = 0; std::string unit; };
struct Color    { std::function<int()> get; std::function<void(int)> set; int def = 0; };   // 0xRRGGBB
struct Choice   { std::vector<std::string> options; std::function<int()> get; std::function<void(int)> set; int def = 0; };
struct Text     { std::function<std::string()> get; std::function<void(const std::string&)> set;
                  std::string def, placeholder; };
struct Action   { std::function<void()> run; bool confirm = false; };   // confirm: first click arms it
struct Section  {};                                                      // heading row
struct Category {};                                                      // a tab; its rows are Setting::children
}

struct Setting {
    using Row = std::variant<row::Toggle, row::Key, row::Slider, row::Color, row::Choice, row::Text, row::Action,
                             row::Section, row::Category>;

    std::string label;
    std::string tooltip;
    Row data;
    std::vector<Setting> children;       // Category only
    std::function<bool()> shownIf;       // optional: the row is only listed while this returns true

    template <class T> const T* As() const { return std::get_if<T>(&data); }
    bool IsCategory() const { return std::holds_alternative<row::Category>(data); }

    Setting Tip(std::string text) && { tooltip = std::move(text); return std::move(*this); }
    Setting ShownIf(std::function<bool()> fn) && { shownIf = std::move(fn); return std::move(*this); }

    // Puts this row (and a category's children) back to its default.
    void Reset() const;

    static Setting Toggle(std::string label, std::function<bool()> get, std::function<void(bool)> set, bool def) {
        return Make(std::move(label), row::Toggle{std::move(get), std::move(set), def});
    }
    static Setting KeyBind(std::string label, std::function<int()> get, std::function<void(int)> set, int def) {
        return Make(std::move(label), row::Key{std::move(get), std::move(set), def});
    }
    static Setting Slider(std::string label, std::function<float()> get, std::function<void(float)> set, float min,
                          float max, float def, float step = 0, int decimals = 0, std::string unit = {}) {
        return Make(std::move(label), row::Slider{std::move(get), std::move(set), min, max, step, def, decimals, std::move(unit)});
    }
    static Setting Color(std::string label, std::function<int()> get, std::function<void(int)> set, int def) {
        return Make(std::move(label), row::Color{std::move(get), std::move(set), def});
    }
    static Setting Choice(std::string label, std::vector<std::string> options, std::function<int()> get,
                          std::function<void(int)> set, int def = 0) {
        return Make(std::move(label), row::Choice{std::move(options), std::move(get), std::move(set), def});
    }
    static Setting Text(std::string label, std::function<std::string()> get,
                        std::function<void(const std::string&)> set, std::string def = {}, std::string placeholder = {}) {
        return Make(std::move(label), row::Text{std::move(get), std::move(set), std::move(def), std::move(placeholder)});
    }
    static Setting Action(std::string label, std::function<void()> run, bool confirm = false) {
        return Make(std::move(label), row::Action{std::move(run), confirm});
    }
    static Setting Section(std::string label) { return Make(std::move(label), row::Section{}); }
    static Setting Category(std::string label, std::vector<Setting> children) {
        Setting s = Make(std::move(label), row::Category{});
        s.children = std::move(children);
        return s;
    }

    // Rows bound to a variable and a persisted key: the setter stores the value in `var` and in the Store.
    // (Loading the saved value into `var` at startup stays with the owner: var = store.GetBool(key, var).)
    static Setting Var(std::string label, std::string key, bool& var, bool def) {
        return Toggle(std::move(label), [&var] { return var; },
                      [&var, key = std::move(key)](bool v) { var = v; if (auto* s = CurrentStore()) s->SetBool(key, v); }, def);
    }
    static Setting Var(std::string label, std::string key, int& var, int min, int max, int def, int step = 1,
                       std::string unit = {}) {
        return Slider(std::move(label), [&var] { return static_cast<float>(var); },
                      [&var, key = std::move(key)](float v) {
                          var = static_cast<int>(std::lround(v));
                          if (auto* s = CurrentStore()) s->SetInt(key, var);
                      },
                      static_cast<float>(min), static_cast<float>(max), static_cast<float>(def),
                      static_cast<float>(step), 0, std::move(unit));
    }
    static Setting ColorVar(std::string label, std::string key, int& var, int def) {
        return Color(std::move(label), [&var] { return var; },
                     [&var, key = std::move(key)](int v) { var = v; if (auto* s = CurrentStore()) s->SetInt(key, v); }, def);
    }

    // Older names, kept so existing modules keep compiling.
    static Setting BoolVar(std::string label, std::string key, bool& var, bool def) { return Var(std::move(label), std::move(key), var, def); }
    static Setting IntVar(std::string label, std::string key, int& var, int min, int max, int def, int step = 1, std::string unit = {}) {
        return Var(std::move(label), std::move(key), var, min, max, def, step, std::move(unit));
    }

private:
    static Setting Make(std::string label, Row r) {
        Setting s;
        s.label = std::move(label);
        s.data = std::move(r);
        return s;
    }
};

}
