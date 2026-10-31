#pragma once

#include "MossUI/core/Types.hpp"
#include "MossUI/ui/UI.hpp"

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace MossUI {

class Container;   // declared here, not via "class Container*" below: that would bind to a game's global ::Container

class Widget : public std::enable_shared_from_this<Widget> {
public:
    virtual ~Widget() = default;

    Rect bounds{0, 0, 0, 0};
    bool visible = true;
    bool focused = false;
    std::string tooltip;
    std::string accessibleName, accessibleDescription;

    Widget& At(const Rect& rect) { bounds = rect; return *this; }
    Widget& Tip(std::string text) { tooltip = std::move(text); return *this; }
    Widget& Accessible(std::string name, std::string description = {}) {
        accessibleName = std::move(name); accessibleDescription = std::move(description); return *this;
    }
    // Lets the screen manager walk containers without RTTI (dynamic_cast), so mods can build with -fno-rtti.
    virtual Container* AsContainer() { return nullptr; }
    virtual AccessibleRole Role() const { return AccessibleRole::Generic; }
    virtual std::string AccessibleValue() const { return {}; }
    virtual bool Disabled() const { return false; }
    AccessibilityInfo Accessibility() const {
        return {Role(), accessibleName.empty() ? AccessibleValue() : accessibleName,
                accessibleDescription, AccessibleValue(), focused, Disabled()};
    }

    virtual void Render(UI& ui) = 0;
    virtual float PreferredHeight() const { return 30.0f; }
    virtual bool FullWidth() const { return false; }
    virtual bool CanFocus() const { return false; }
    virtual bool IsModal() const { return false; }
    virtual bool IsModalContent(float x, float y) const { return Contains(x, y); }
    virtual void OnOutsideClick() {}

    virtual bool Contains(float x, float y) const { return x >= bounds.x0 && x <= bounds.x1 && y >= bounds.y0 && y <= bounds.y1; }
    virtual std::shared_ptr<Widget> HitTest(float x, float y) {
        return visible && Contains(x, y) ? shared_from_this() : nullptr;
    }

    virtual bool OnPress(UI&) { return false; }
    virtual void OnDrag(UI&) {}
    virtual void OnRelease(UI&, bool ) {}
    virtual void OnFocus(bool gained) { focused = gained; }
    virtual bool WantsFocus() const { return focused; }
    virtual bool OnKey(int ) { return false; }
    virtual void OnKeyUp(int ) {}
    virtual bool OnNavKey(int ) { return false; }
    virtual bool OnScroll(UI&, float , float , int ) { return false; }
};

class Container : public Widget {
public:
    bool focusScope = false;
    Container* AsContainer() override { return this; }
    template <typename T, typename... Args>
    T& Add(Args&&... args) {
        auto p = std::make_shared<T>(std::forward<Args>(args)...);
        T& ref = *p;
        children.push_back(std::move(p));
        return ref;
    }
    void Add(std::shared_ptr<Widget> w) { children.push_back(std::move(w)); }
    void Clear() { children.clear(); }
    std::shared_ptr<Widget> ActiveModal() const {
        for (auto it = children.rbegin(); it != children.rend(); ++it)
            if ((*it)->visible && (*it)->IsModal()) return *it;
        return nullptr;
    }

    void Render(UI& ui) override {
        const auto modal = ActiveModal();
        const bool prev = ui.HoverBlocked();
        for (auto& c : children) {
            if (!c->visible) continue;
            ui.BlockHover(prev || (modal && c != modal));
            c->Render(ui);
        }
        ui.BlockHover(prev);
    }
    std::shared_ptr<Widget> HitTest(float x, float y) override {
        for (auto it = children.rbegin(); it != children.rend(); ++it)
            if (auto w = (*it)->HitTest(x, y)) return w;
        return nullptr;
    }
    bool OnScroll(UI& ui, float x, float y, int dir) override {
        for (auto it = children.rbegin(); it != children.rend(); ++it)
            if ((*it)->visible && (*it)->OnScroll(ui, x, y, dir)) return true;
        return false;
    }

    std::vector<std::shared_ptr<Widget>> children;
};

}
