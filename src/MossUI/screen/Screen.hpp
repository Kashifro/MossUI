#pragma once

#include "MossUI/core/Backend.hpp"
#include "MossUI/ui/UI.hpp"
#include "MossUI/widgets/Widget.hpp"

#include <functional>
#include <memory>

namespace MossUI {

class Screen {
public:
    virtual ~Screen() = default;

    virtual void Init(float width, float height) = 0;
    virtual void Render(UI& ui) { ui.Backdrop(); root.Render(ui); }
    virtual bool OnKey(int ) { return false; }
    virtual void OnOpen() {}
    virtual void OnClose() {}

    void Rebuild() { dirty = true; }
    Container root;

    float layoutW = 0, layoutH = 0;
    bool dirty = true;
};

void SetBackend(Backend* backend);
void SetHost(Host* host);

void Open(std::shared_ptr<Screen> screen);
void PushScreen(std::shared_ptr<Screen> screen);
void Close();
bool IsOpen();
Screen* Current();
bool SuppressMouseGrab();

bool OnMouse(int button, int data, int rawX, int rawY);
void OnKey(int key, bool down, bool shift, bool ctrl, bool alt);
enum class NavigationAction { Previous, Next, Up, Down, Left, Right, Accept, Back };
void OnNavigate(NavigationAction action, bool down = true);

void Render(float width, float height);
// One-shot, non-interactive drawing on top of the game (HUD overlays): sets up size and text batching, runs `draw`,
// flushes. Needs the backend to have started its frame, like Render.
void RenderOverlay(float width, float height, const std::function<void(UI&)>& draw);

void FocusWidget(std::shared_ptr<Widget> w);
bool ShiftHeld();
bool CtrlHeld();
bool AltHeld();

}
