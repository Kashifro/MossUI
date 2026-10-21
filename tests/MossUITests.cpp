#include "MossUI/MossUI.hpp"
#include "MossUI/testing/TestBackend.hpp"

#include <cassert>
#include <cmath>
#include <limits>
#include <memory>
#include <thread>

namespace {

using namespace MossUI;

class TestScreen : public Screen {
public:
    void Init(float, float) override {
        first = &root.Add<ButtonWidget>("First", [this] { ++firstActivations; });
        first->bounds = {20, 120, 20, 50};
        second = &root.Add<ButtonWidget>("Second", [this] { ++secondActivations; });
        second->bounds = {20, 120, 60, 90};
        auto& label = root.Add<LabelWidget>("Scaled label");
        label.bounds = {20, 120, 100, 120};
    }

    ButtonWidget* first = nullptr;
    ButtonWidget* second = nullptr;
    int firstActivations = 0;
    int secondActivations = 0;
};

class TestModal : public Widget {
public:
    void Render(UI&) override {}
    bool IsModal() const override { return visible; }
    bool IsModalContent(float x, float y) const override {
        return x >= 200 && x <= 400 && y >= 200 && y <= 300;
    }
    void OnOutsideClick() override { visible = false; }
    bool OnPress(UI&) override { return true; }
    bool OnKey(int key) override {
        if (key == Key(KeyCode::ENTER)) ++confirmations;
        if (key == Key(KeyCode::ESCAPE)) visible = false;
        return true;
    }
    int confirmations = 0;
};

class ModalScreen : public Screen {
public:
    void Init(float, float) override {
        auto& button = root.Add<ButtonWidget>("Behind modal", [this] { ++backgroundActivations; });
        button.bounds = {20, 120, 20, 50};
        button.tooltip = "Background tip";
        auto& overlay = root.Add<TestModal>();
        overlay.bounds = {0, 800, 0, 600};
        modal = &overlay;
    }
    TestModal* modal = nullptr;
    int backgroundActivations = 0;
};

class DialogScreen : public Screen {
public:
    void Init(float, float) override {
        dialog = &root.Add<ConfirmationDialog>();
        dialog->bounds = {0, 800, 0, 600};
        dialog->Show("Open link?", "https://example.test",
                     {{"Open", [this] { selectedAction = 1; }},
                      {"Copy", [this] { selectedAction = 2; }},
                      {"Cancel", {}}},
                     "Only open links you trust.", DialogPresentation::Overlay);
    }
    ConfirmationDialog* dialog = nullptr;
    int selectedAction = 0;
};

class VariableHeightRow : public Widget {
public:
    explicit VariableHeightRow(float height) : height(height) {}
    float PreferredHeight() const override { return height; }
    void Render(UI& ui) override {
        ui.Fill(bounds, col::WHITE);
        ui.Text(bounds.x0, bounds.x1, bounds.y0, "row", col::WHITE, Align::Left);
        ui.Text(bounds.x0, bounds.x1, bounds.y1 - 10.0f, "row", col::WHITE, Align::Left);
    }

private:
    float height;
};

class VariableListScreen : public Screen {
public:
    void Init(float, float) override {
        list = &root.Add<ListWidget>();
        list->bounds = {100, 300, 60, 220};
        for (int i = 0; i < 14; ++i) list->Add<VariableHeightRow>(i % 3 == 0 ? 46.0f : 31.0f);
    }
    ListWidget* list = nullptr;
};

void AssertWhiteFillsClipped(const testing::TestBackend& backend, const Rect& clip) {
    for (const auto& fill : backend.fills) {
        if (fill.color.r != 255 || fill.color.g != 255 || fill.color.b != 255) continue;
        assert(fill.rect.x0 >= clip.x0 && fill.rect.x1 <= clip.x1);
        assert(fill.rect.y0 >= clip.y0 && fill.rect.y1 <= clip.y1);
    }
    for (size_t i = 0; i < backend.textClipDepths.size(); ++i) {
        assert(backend.textClipDepths[i] > 0);
        const Rect& textClip = backend.textClipRects[i];
        assert(textClip.x0 == clip.x0 && textClip.x1 == clip.x1);
        assert(textClip.y0 == clip.y0 && textClip.y1 == clip.y1);
    }
}

bool HasPartiallyVisibleRow(const VariableListScreen& screen) {
    for (const auto& child : screen.list->children) {
        if (!child->visible) continue;
        if (child->bounds.y0 < screen.list->bounds.y0 + 1.0f ||
            child->bounds.y1 > screen.list->bounds.y1 - 1.0f)
            return true;
    }
    return false;
}

class ComponentsScreen : public Screen {
public:
    void Init(float, float) override {
        combo = &root.Add<ComboBox>();
        combo->bounds = {100, 220, 20, 42};
        combo->options = {"One", "Two", "Three"};
        tabs = &root.Add<TabBar>();
        tabs->bounds = {100, 220, 100, 124};
        tabs->tabs = {"First", "Second"};
        list = &root.Add<SelectableList>();
        list->bounds = {100, 220, 160, 240};
        list->items = {"Alpha", "Beta"};
    }
    ComboBox* combo = nullptr;
    TabBar* tabs = nullptr;
    SelectableList* list = nullptr;
};

void Click(int x, int y) {
    OnMouse(1, 1, x, y);
    OnMouse(1, 0, x, y);
}

void TestLayoutAndAccessibility() {
    const Rect centered = Layout::Centered(100, 80, 40, 20);
    assert(centered.x0 == 80 && centered.x1 == 120 && centered.y0 == 70 && centered.y1 == 90);
    const auto halves = Layout::SplitHorizontal({0, 100, 0, 20}, 10);
    assert(halves[0].x1 == 45 && halves[1].x0 == 55);

    testing::TestBackend backend;
    testing::TestHost host;
    SetBackend(&backend);
    SetHost(&host);
    SetAccessibilityOptions({1.5f});
    auto screen = std::make_shared<TestScreen>();
    Open(screen);
    Render(800, 600);
    bool foundScaledRun = false;
    for (const auto& run : backend.texts) {
        if (run.text == "Scaled label") {
            foundScaledRun = std::fabs(run.scale - 1.5f) < 0.001f;
            break;
        }
    }
    assert(foundScaledRun);

    WidgetMetrics metrics;
    metrics.rowHeight = 40.0f;
    metrics.toastMaxVisible = 20;
    SetWidgetMetrics(metrics);
    assert(CurrentWidgetMetrics().rowHeight == 40.0f);
    assert(CurrentWidgetMetrics().toastMaxVisible == 10);
    metrics.rowHeight = std::numeric_limits<float>::quiet_NaN();
    SetWidgetMetrics(metrics);
    assert(CurrentWidgetMetrics().rowHeight == 22.0f);
    ResetWidgetMetrics();

    ScrollState scroll;
    scroll.max = 20;
    scroll.pos = 10;
    assert(scroll.UpdateVisualPosition() == 10.0f);
    scroll.pos = 18;
    scroll.lastUpdate = std::chrono::steady_clock::now() - std::chrono::milliseconds(50);
    const float interpolated = scroll.UpdateVisualPosition();
    assert(interpolated > 10.0f && interpolated < 18.0f);
    scroll.drag = true;
    scroll.pos = 5;
    assert(scroll.UpdateVisualPosition() == 5.0f);

    UI ui;
    assert(!ui.Image({0, 10, 0, 10}, "missing.png"));
    Close();
    ResetAccessibilityOptions();
}

void TestButtonTextFitsBounds() {
    testing::TestBackend backend;
    testing::TestHost host;
    SetBackend(&backend);
    SetHost(&host);
    UI ui;
    const Rect button{20, 90, 30, 52};
    ui.DrawButton(button, "A very long button label that must stay inside its bounds", true, false, false);
    assert(backend.texts.size() == 1);
    const TextRun& text = backend.texts.front();
    assert(ui.TextWidth(text.text) <= button.x1 - button.x0 - 8.0f);
    assert(text.rect.x0 >= button.x0 && text.rect.x1 <= button.x1);
    assert(text.rect.y0 >= button.y0 && text.rect.y1 <= button.y1);
}

void TestCanvasPreservesTextPosition() {
    testing::TestBackend backend;
    testing::TestHost host;
    SetBackend(&backend);
    SetHost(&host);
    UI ui;
    const Rect canvasBounds{20, 120, 30, 80};
    {
        Canvas canvas(ui, canvasBounds);
        ui.Text(10, 140, 75, "partially clipped", col::WHITE, Align::Left);
        ui.Text(10, 140, 10, "outside", col::WHITE, Align::Left);
    }
    assert(backend.texts.size() == 2);
    const Rect& text = backend.texts.front().rect;
    assert(text.x0 == 10 && text.x1 == 140);
    assert(text.y0 == 75.0f && text.y1 == 85.0f);
    assert(backend.textClipDepths.front() == 1);
    const Rect& activeClip = backend.textClipRects.front();
    assert(activeClip.x0 == canvasBounds.x0 && activeClip.x1 == canvasBounds.x1);
    assert(activeClip.y0 == canvasBounds.y0 && activeClip.y1 == canvasBounds.y1);
    assert(backend.clipDepth == 0);
}

void TestConfigurableConfirmationDialog() {
    testing::TestBackend backend;
    testing::TestHost host;
    SetBackend(&backend);
    SetHost(&host);
    auto screen = std::make_shared<DialogScreen>();
    Open(screen);
    Render(800, 600);

    Click(290, 328);
    assert(screen->selectedAction == 1);
    assert(!screen->dialog->visible);
    Close();
}

void TestVariableHeightListScrollClampsAtViewport() {
    testing::TestBackend backend;
    testing::TestHost host;
    SetBackend(&backend);
    SetHost(&host);
    WidgetMetrics metrics = CurrentWidgetMetrics();
    metrics.scrollSmoothness = 40.0f;
    metrics.scrollStep = 64.0f;
    SetWidgetMetrics(metrics);
    auto screen = std::make_shared<VariableListScreen>();
    Open(screen);
    Render(800, 600);

    UI ui;
    for (int i = 0; i < 100; ++i) screen->list->OnScroll(ui, 150.0f, 120.0f, 1);
    bool sawPartialAtBottom = false;
    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        backend.fills.clear();
        backend.texts.clear();
        backend.textClipDepths.clear();
        backend.textClipRects.clear();
        Render(800, 600);
        AssertWhiteFillsClipped(backend, screen->list->bounds);
        if (HasPartiallyVisibleRow(*screen)) {
            sawPartialAtBottom = true;
            bool rowTextCrossesViewport = false;
            for (size_t t = 0; t < backend.texts.size(); ++t) {
                const auto& text = backend.texts[t];
                const auto& clip = backend.textClipRects[t];
                if (text.text != "row" || text.rect.y1 <= clip.y1) continue;
                assert(backend.textClipDepths[t] > 0);
                assert(clip.x0 == screen->list->bounds.x0 && clip.x1 == screen->list->bounds.x1);
                assert(clip.y0 == screen->list->bounds.y0 && clip.y1 == screen->list->bounds.y1);
                rowTextCrossesViewport = true;
            }
            assert(rowTextCrossesViewport);
        }
    }
    assert(sawPartialAtBottom);
    const auto& last = screen->list->children.back();
    assert(last->visible);
    assert(last->bounds.y1 <= screen->list->bounds.y1 - 1.0f);
    assert(backend.clipDepth == 0);
    for (int i = 0; i < 100; ++i) screen->list->OnScroll(ui, 150.0f, 120.0f, -1);
    bool sawPartialAtTop = false;
    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        backend.fills.clear();
        backend.texts.clear();
        backend.textClipDepths.clear();
        backend.textClipRects.clear();
        Render(800, 600);
        AssertWhiteFillsClipped(backend, screen->list->bounds);
        if (HasPartiallyVisibleRow(*screen)) {
            sawPartialAtTop = true;
            bool rowTextCrossesViewport = false;
            for (size_t t = 0; t < backend.texts.size(); ++t) {
                const auto& text = backend.texts[t];
                const auto& clip = backend.textClipRects[t];
                if (text.text != "row" || text.rect.y0 >= clip.y0) continue;
                assert(backend.textClipDepths[t] > 0);
                assert(clip.x0 == screen->list->bounds.x0 && clip.x1 == screen->list->bounds.x1);
                assert(clip.y0 == screen->list->bounds.y0 && clip.y1 == screen->list->bounds.y1);
                rowTextCrossesViewport = true;
            }
            assert(rowTextCrossesViewport);
        }
    }
    assert(sawPartialAtTop);
    const auto& first = screen->list->children.front();
    assert(first->visible);
    assert(first->bounds.y0 >= screen->list->bounds.y0 + 1.0f);
    assert(backend.clipDepth == 0);
    Close();
    ResetWidgetMetrics();
}

void TestKeyboardFocus() {
    testing::TestBackend backend;
    testing::TestHost host;
    SetBackend(&backend);
    SetHost(&host);
    auto screen = std::make_shared<TestScreen>();
    Open(screen);
    Render(800, 600);

    OnKey(Key(KeyCode::TAB), true, false, false, false);
    OnKey(Key(KeyCode::TAB), false, false, false, false);
    OnKey(Key(KeyCode::ENTER), true, false, false, false);
    assert(screen->firstActivations == 1);
    OnKey(Key(KeyCode::ENTER), false, false, false, false);

    OnKey(Key(KeyCode::TAB), true, false, false, false);
    OnKey(Key(KeyCode::TAB), false, false, false, false);
    OnKey(Key(KeyCode::ENTER), true, false, false, false);
    assert(screen->secondActivations == 1);
    Close();
}

void TestModalBlocksHover() {
    testing::TestBackend backend;
    testing::TestHost host;
    SetBackend(&backend);
    SetHost(&host);
    auto screen = std::make_shared<ModalScreen>();
    Open(screen);
    OnMouse(0, 0, 50, 35);
    backend.texts.clear();
    Render(800, 600);
    for (const auto& run : backend.texts) assert(run.text != "Background tip");

    screen->modal->visible = false;
    backend.texts.clear();
    Render(800, 600);
    bool sawTip = false;
    for (const auto& run : backend.texts)
        if (run.text == "Background tip") sawTip = true;
    assert(sawTip);
    Close();
}

void TestModalInputCapture() {
    testing::TestBackend backend;
    testing::TestHost host;
    SetBackend(&backend);
    SetHost(&host);
    auto screen = std::make_shared<ModalScreen>();
    Open(screen);
    Render(800, 600);

    Click(250, 250);
    assert(screen->backgroundActivations == 0);
    assert(screen->modal->visible);

    OnKey(Key(KeyCode::ENTER), true, false, false, false);
    assert(screen->modal->confirmations == 1);
    OnKey(Key(KeyCode::ESCAPE), true, false, false, false);
    assert(!screen->modal->visible);
    assert(Current() == screen.get());

    screen->modal->visible = true;
    Click(40, 35);
    assert(!screen->modal->visible);
    assert(screen->backgroundActivations == 0);
    Click(40, 35);
    assert(screen->backgroundActivations == 1);
    Close();
}

void TestPopupAndWidgetKeyboardInput() {
    testing::TestBackend backend;
    testing::TestHost host;
    SetBackend(&backend);
    SetHost(&host);
    auto screen = std::make_shared<ComponentsScreen>();
    Open(screen);
    Render(800, 600);

    Click(120, 30);
    assert(screen->combo->Expanded());
    Click(120, 66);
    assert(screen->combo->selected == 1);
    Click(120, 30);
    Click(300, 300);
    assert(!screen->combo->Expanded());
    Click(120, 66);
    assert(screen->combo->selected == 1);

    OnKey(Key(KeyCode::TAB), true, false, false, false);
    OnKey(Key(KeyCode::TAB), false, false, false, false);
    OnKey(Key(KeyCode::TAB), true, false, false, false);
    OnKey(Key(KeyCode::TAB), false, false, false, false);
    OnKey(Key(KeyCode::RIGHT), true, false, false, false);
    assert(screen->tabs->selected == 1);
    OnKey(Key(KeyCode::TAB), true, false, false, false);
    OnKey(Key(KeyCode::TAB), false, false, false, false);
    OnKey(Key(KeyCode::DOWN), true, false, false, false);
    assert(screen->list->selected == 0);
    Close();
}

void TestScreenStackAndNewOptions() {
    testing::TestBackend backend;
    testing::TestHost host;
    SetBackend(&backend);
    SetHost(&host);

    auto parent = std::make_shared<TestScreen>();
    auto child = std::make_shared<TestScreen>();
    Open(parent);
    assert(Current() == parent.get());
    PushScreen(child);
    assert(Current() == child.get());
    OnKey(Key(KeyCode::ESCAPE), true, false, false, false);
    assert(Current() == parent.get());
    Close();
    assert(!IsOpen());

    int choiceVal = 0;
    ChoiceOptionWidget choice("Mode", {"Fast", "Fancy", "Ultra"}, [&] { return choiceVal; }, [&](int v) { choiceVal = v; });
    choice.bounds = {0, 300, 0, 30};
    choice.OnNavKey(Key(KeyCode::RIGHT));
    assert(choiceVal == 1);

    std::string textVal = "hi";
    auto textOpt = std::make_shared<TextOptionWidget>("Prefix", [&] { return textVal; }, [&](const std::string& s) { textVal = s; }, "hint");
    textOpt->bounds = {0, 300, 0, 30};
    textOpt->OnNavKey(Key(KeyCode::ENTER));
    textOpt->OnKey(Key(KeyCode::A));
    assert(textVal == "hia");

    float sliderVal = 10.0f;
    SliderOptionWidget slider("Scale", [&] { return sliderVal; }, [&](float v) { sliderVal = v; }, 0.0f, 100.0f, 1.0f);
    slider.bounds = {0, 300, 0, 46};
    auto dummyScreen = std::make_shared<TestScreen>();
    Open(dummyScreen);
    OnKey(Key(KeyCode::LEFT_SHIFT), true, true, false, false);
    slider.OnNavKey(Key(KeyCode::RIGHT));
    assert(std::fabs(sliderVal - 20.0f) < 0.01f);

    int colorVal = 0x112233;
    ColorOptionWidget colorOpt("Tint", [&] { return colorVal; }, [&](int v) { colorVal = v; });
    colorOpt.bounds = {0, 300, 0, 104};
    backend.clipboard = "#FF8800";
    OnKey(Key(KeyCode::V), true, false, true, false);
    colorOpt.OnNavKey(Key(KeyCode::V));
    assert(colorVal == 0xFF8800);
    Close();
}

}

int main() {
    TestLayoutAndAccessibility();
    TestModalBlocksHover();
    TestModalInputCapture();
    TestPopupAndWidgetKeyboardInput();
    TestScreenStackAndNewOptions();
    TestButtonTextFitsBounds();
    TestCanvasPreservesTextPosition();
    TestConfigurableConfirmationDialog();
    TestVariableHeightListScrollClampsAtViewport();
    TestKeyboardFocus();
}
