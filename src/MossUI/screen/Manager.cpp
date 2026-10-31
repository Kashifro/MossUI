#include "MossUI/screen/Screen.hpp"

#include "MossUI/core/Keys.hpp"
#include "MossUI/core/State.hpp"

#include <algorithm>
#include <functional>
#include <vector>

namespace MossUI {

namespace {
using detail::S;

struct Manager {
    std::vector<std::shared_ptr<Screen>> stack;
    std::shared_ptr<Widget> pressed;
    std::shared_ptr<Widget> focus;
    AccessibilityInfo lastAccessibility{};
    bool hasAccessibility = false;
    bool overHud = false;

    std::shared_ptr<Screen> Top() const { return stack.empty() ? nullptr : stack.back(); }
};

Manager M;

void SetFocus(std::shared_ptr<Widget> w) {
    if (w == M.focus) return;
    auto old = std::move(M.focus);
    M.focus = std::move(w);
    if (old) old->OnFocus(false);
    if (M.focus) M.focus->OnFocus(true);
}

void CollectFocusable(const std::shared_ptr<Widget>& widget, std::vector<std::shared_ptr<Widget>>& result) {
    if (!widget || !widget->visible) return;
    if (widget->CanFocus()) result.push_back(widget);
    if (auto* container = widget->AsContainer())
        for (const auto& child : container->children) CollectFocusable(child, result);
}

// The innermost focus scope containing `target`, or null when it is in no scope.
Container* FocusScopeContaining(const std::shared_ptr<Widget>& widget, const Widget* target, Container* current) {
    if (!widget || !widget->visible) return nullptr;
    Container* container = widget->AsContainer();
    if (container && container->focusScope) current = container;
    if (widget.get() == target) return current;
    if (container)
        for (const auto& child : container->children)
            if (auto* scope = FocusScopeContaining(child, target, current)) return scope;
    return nullptr;
}

Container* FocusScopeIn(Container& root, const Widget* target) {
    Container* current = root.focusScope ? &root : nullptr;
    for (const auto& child : root.children)
        if (auto* scope = FocusScopeContaining(child, target, current)) return scope;
    return nullptr;
}

void CycleFocus(bool backwards) {
    auto screen = M.Top();
    if (!screen) return;
    if (auto modal = screen->root.ActiveModal()) {
        SetFocus(modal->CanFocus() ? modal : nullptr);
        return;
    }
    std::vector<std::shared_ptr<Widget>> focusable;
    Container* scope = FocusScopeIn(screen->root, M.focus.get());
    if (!scope) scope = &screen->root;
    if (scope != &screen->root && scope->CanFocus()) focusable.push_back(scope->shared_from_this());
    for (const auto& child : scope->children) CollectFocusable(child, focusable);
    if (focusable.empty()) {
        SetFocus(nullptr);
        return;
    }
    const auto current = std::find(focusable.begin(), focusable.end(), M.focus);
    size_t index;
    if (current == focusable.end()) index = backwards ? focusable.size() - 1 : 0;
    else {
        const auto currentIndex = static_cast<size_t>(current - focusable.begin());
        index = backwards ? (currentIndex + focusable.size() - 1) % focusable.size()
                          : (currentIndex + 1) % focusable.size();
    }
    SetFocus(focusable[index]);
}

void ResetInput() {
    auto& s = S();
    s.down = false;
    s.shift = s.ctrl = s.alt = false;
    if (M.focus) M.focus->OnFocus(false);
    M.focus.reset();
    M.pressed.reset();
}

void Measure(float w, float h) {
    auto& s = S();
    s.w = w;
    s.h = h;
}
}

void SetBackend(Backend* backend) { S().backend = backend; M.hasAccessibility = false; }
void SetHost(Host* host) { S().host = host; }

bool IsOpen() { return !M.stack.empty(); }
bool SuppressMouseGrab() { return !M.stack.empty() && M.overHud; }
Screen* Current() { return M.stack.empty() ? nullptr : M.stack.back().get(); }
bool ShiftHeld() { return S().shift; }
bool CtrlHeld() { return S().ctrl; }
bool AltHeld() { return S().alt; }

void Open(std::shared_ptr<Screen> screen) {
    if (!screen) return;
    while (!M.stack.empty()) Close();
    M.stack.push_back(std::move(screen));
    M.stack.back()->dirty = true;
    ResetInput();
    auto& s = S();
    s.hasMouse = false;
    s.host->ScreenOpened();
    M.overHud = s.host->OverHud();
    s.inGame = M.overHud;
    if (M.overHud) s.host->ReleaseMouse();
    M.stack.back()->OnOpen();
}

void PushScreen(std::shared_ptr<Screen> screen) {
    if (!screen) return;
    if (M.stack.empty()) {
        Open(std::move(screen));
        return;
    }
    ResetInput();
    S().host->ScreenOpened();
    screen->dirty = true;
    M.stack.push_back(std::move(screen));
    M.stack.back()->OnOpen();
}

void Close() {
    if (M.stack.empty()) return;
    auto old = std::move(M.stack.back());
    M.stack.pop_back();
    ResetInput();
    if (M.stack.empty()) {
        const bool regrab = M.overHud;
        M.overHud = false;
        if (regrab) S().host->GrabMouse();
    } else {
        M.stack.back()->dirty = true;
    }
    old->OnClose();
}

void FocusWidget(std::shared_ptr<Widget> w) { SetFocus(std::move(w)); }

void OnKey(int key, bool down, bool shift, bool ctrl, bool alt) {
    auto keep = M.Top();
    if (!keep) return;
    auto& s = S();
    s.shift = shift;
    s.ctrl = ctrl;
    s.alt = alt;
    if (auto modal = keep->root.ActiveModal()) {
        SetFocus(modal->CanFocus() ? modal : nullptr);
        if (down) modal->OnKey(key);
        else modal->OnKeyUp(key);
        return;
    }
    if (!down) {
        if (M.focus) {
            auto focus = M.focus;
            focus->OnKeyUp(key);
            if (M.focus == focus && !focus->WantsFocus()) SetFocus(nullptr);
        }
        return;
    }

    if (key == Key(KeyCode::TAB)) {
        CycleFocus(shift);
        return;
    }
    if (M.focus) {
        auto focus = M.focus;
        if (focus->OnKey(key)) {
            if (M.focus == focus && !focus->WantsFocus()) SetFocus(nullptr);
            return;
        }
    }
    if (keep->OnKey(key)) return;
    if (key == Key(KeyCode::UP) || key == Key(KeyCode::DOWN) ||
        key == Key(KeyCode::LEFT) || key == Key(KeyCode::RIGHT)) {
        CycleFocus(key == Key(KeyCode::UP) || key == Key(KeyCode::LEFT));
        return;
    }
    if (key == Key(KeyCode::ESCAPE)) {
        if (M.focus) SetFocus(nullptr);
        else Close();
    }
}

void OnNavigate(NavigationAction action, bool down) {
    if (!down) return;
    if (action == NavigationAction::Previous || action == NavigationAction::Next) {
        CycleFocus(action == NavigationAction::Previous);
        return;
    }
    int key = 0;
    switch (action) {
        case NavigationAction::Previous :
        case NavigationAction::Next : return;
        case NavigationAction::Up : key = Key(KeyCode::UP); break;
        case NavigationAction::Down : key = Key(KeyCode::DOWN); break;
        case NavigationAction::Left : key = Key(KeyCode::LEFT); break;
        case NavigationAction::Right : key = Key(KeyCode::RIGHT); break;
        case NavigationAction::Accept : key = Key(KeyCode::ENTER); break;
        case NavigationAction::Back : key = Key(KeyCode::ESCAPE); break;
    }
    OnKey(key, true, ShiftHeld(), CtrlHeld(), AltHeld());
}

bool OnMouse(int button, int data, int rawX, int rawY) {
    auto keep = M.Top();
    if (!keep) return false;
    auto& s = S();

    float gx, gy;
    if ((button == 0 || !s.hasMouse) && s.host->ToGui(rawX, rawY, gx, gy)) {
        s.mx = gx;
        s.my = gy;
        s.hasMouse = true;
    }
    UI ui;

    if (button == 4) {
        if (auto modal = keep->root.ActiveModal()) modal->OnScroll(ui, s.mx, s.my, data > 0 ? -1 : 1);
        else keep->root.OnScroll(ui, s.mx, s.my, data > 0 ? -1 : 1);
    } else if (button == 1) {
        if (data == 1) {
            s.down = true;
            auto modal = keep->root.ActiveModal();
            if (modal && !modal->IsModalContent(s.mx, s.my)) {
                modal->OnOutsideClick();
                s.down = false;
                M.pressed.reset();
                SetFocus(nullptr);
                return true;
            }
            auto target = modal ? modal->HitTest(s.mx, s.my) : keep->root.HitTest(s.mx, s.my);
            if (M.focus && M.focus != target) M.focus->OnOutsideClick();
            const bool capture = target && target->OnPress(ui);
            M.pressed = capture ? target : nullptr;
            SetFocus(target && target->CanFocus() ? target : nullptr);
        } else if (data == 0 && s.down) {
            s.down = false;
            auto released = std::move(M.pressed);
            M.pressed.reset();
            if (released) released->OnRelease(ui, released->Contains(s.mx, s.my));
        }
    }
    return true;
}

void RenderOverlay(float w, float h, const std::function<void(UI&)>& draw) {
    if (w <= 0 || h <= 0 || !draw) return;
    auto& s = S();
    const bool hadMouse = s.hasMouse;
    Measure(w, h);
    s.hasMouse = false;   // an overlay is not interactive: nothing hovers
    s.hoverBlocked = false;
    s.texts.clear();
    s.tip.clear();
    UI ui;
    draw(ui);
    detail::FlushText();
    s.hasMouse = hadMouse;
}

void Render(float w, float h) {
    if (w <= 0 || h <= 0) return;
    auto& s = S();
    Measure(w, h);
    s.hoverBlocked = false;

    auto keep = M.Top();
    if (!keep) {
        s.texts.clear();
        s.tip.clear();
        UI ui;
        detail::RenderToasts(ui);
        detail::FlushText();
        return;
    }

    if (M.overHud && s.host->MouseGrabbed()) s.host->ReleaseMouse();
    s.texts.clear();
    s.tip.clear();

    if (keep->dirty || keep->layoutW != w || keep->layoutH != h) {
        if (M.pressed) { M.pressed.reset(); s.down = false; }
        SetFocus(nullptr);
        keep->root.Clear();
        keep->Init(w, h);
        keep->layoutW = w;
        keep->layoutH = h;
        keep->dirty = false;
    }

    UI ui;
    if (M.pressed && s.down && s.hasMouse) M.pressed->OnDrag(ui);
    keep->Render(ui);
    if (s.backend) {
        const AccessibilityInfo current = M.focus ? M.focus->Accessibility() : AccessibilityInfo{};
        const bool changed = !M.hasAccessibility || current.role != M.lastAccessibility.role ||
            current.name != M.lastAccessibility.name || current.description != M.lastAccessibility.description ||
            current.value != M.lastAccessibility.value || current.focused != M.lastAccessibility.focused ||
            current.disabled != M.lastAccessibility.disabled;
        if (changed) {
            s.backend->Accessibility(current);
            M.lastAccessibility = current;
            M.hasAccessibility = true;
        }
    }
    if (auto modal = keep->root.ActiveModal()) {
        if (M.focus != modal && modal->CanFocus()) SetFocus(modal);
    }
    detail::RenderToasts(ui);
    detail::FlushText();
    if (!s.tip.empty() && s.hasMouse) {
        detail::DrawTooltipBox();
        detail::FlushText();
    }
}

}
