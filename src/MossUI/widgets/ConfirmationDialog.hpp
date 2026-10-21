#pragma once

#include "MossUI/widgets/Widget.hpp"

#include <vector>

namespace MossUI {

enum class DialogPresentation { Panel, Overlay };

struct DialogAction {
    std::string label;
    std::function<void()> onSelect;
};

class ConfirmationDialog : public Widget {
public:
    ConfirmationDialog() { visible = false; }

    void Show(std::string title, std::string message, std::vector<DialogAction> actions,
              std::string warning = {}, DialogPresentation presentation = DialogPresentation::Panel);
    void Dismiss();
    void Render(UI& ui) override;
    bool IsModal() const override { return visible; }
    bool IsModalContent(float x, float y) const override;
    bool CanFocus() const override { return true; }
    AccessibleRole Role() const override { return AccessibleRole::Dialog; }
    void OnOutsideClick() override { Dismiss(); }
    bool OnPress(UI& ui) override;
    void OnRelease(UI& ui, bool inside) override;
    bool OnScroll(UI&, float x, float y, int) override { return visible && Contains(x, y); }
    bool OnKey(int vk) override;

private:
    Rect PanelBounds(UI& ui) const;
    std::vector<Rect> ActionBounds(UI& ui) const;
    void Activate(size_t index);

    std::string title;
    std::string message;
    std::string warning;
    std::vector<DialogAction> actions;
    DialogPresentation presentation = DialogPresentation::Panel;
    int pressedButton = -1;
    int focusedAction = 0;
};

}
