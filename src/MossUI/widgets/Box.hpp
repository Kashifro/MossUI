#pragma once

#include "MossUI/core/Layout.hpp"
#include "MossUI/widgets/Widget.hpp"

namespace MossUI {

enum class Axis { Horizontal, Vertical };

// A row or column that lays its children out itself. Give it `bounds` once; every child gets its rectangle from the
// box on each render, so a resize needs no pixel maths in the screen:
//
//   auto& footer = root.Add<Box>(Axis::Horizontal, 8.0f);
//   footer.bounds = {W / 2 - 154, W / 2 + 154, H - 28, H - 8};
//   footer.Flex<ButtonWidget>(1, "Open Mods Folder", openFolder);
//   footer.Flex<ButtonWidget>(1, "Done", close);
//
// Fixed(px) children take exactly px along the axis, Flex(weight) children share what is left in proportion to
// their weights. Hidden children take no space. Boxes nest.
class Box : public Container {
public:
    explicit Box(Axis axis = Axis::Vertical, float gap = 0.0f, Layout::Insets padding = {})
        : axis(axis), gap(gap), padding(padding) {}

    template <class T, class... Args>
    T& Fixed(float px, Args&&... args) {
        T& child = Add<T>(std::forward<Args>(args)...);
        slots.resize(children.size() - 1);
        slots.push_back({px, 0.0f});
        return child;
    }
    template <class T, class... Args>
    T& Flex(float weight, Args&&... args) {
        T& child = Add<T>(std::forward<Args>(args)...);
        slots.resize(children.size() - 1);
        slots.push_back({0.0f, weight});
        return child;
    }
    void Fixed(float px, std::shared_ptr<Widget> child) {
        children.push_back(std::move(child));
        slots.resize(children.size() - 1);
        slots.push_back({px, 0.0f});
    }
    void Flex(float weight, std::shared_ptr<Widget> child) {
        children.push_back(std::move(child));
        slots.resize(children.size() - 1);
        slots.push_back({0.0f, weight});
    }
    void Clear() { Container::Clear(); slots.clear(); }

    // Assigns the children their rectangles. Called by Render; call it yourself to read child bounds earlier.
    void Arrange();

    void Render(UI& ui) override { Arrange(); Container::Render(ui); }

private:
    struct Slot { float fixed = 0.0f, flex = 0.0f; };   // both 0 = Flex(1)
    Axis axis;
    float gap;
    Layout::Insets padding;
    std::vector<Slot> slots;
};

// Takes space in a Box and draws nothing.
class Spacer : public Widget {
public:
    void Render(UI&) override {}
    std::shared_ptr<Widget> HitTest(float, float) override { return nullptr; }
};

}
