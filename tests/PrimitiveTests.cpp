// Tests for the shared drawing primitives, render backend, typed settings and Box layout.
#include "MossUI/MossUI.hpp"
#include "MossUI/core/State.hpp"
#include "MossUI/testing/TestBackend.hpp"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <string>

namespace {

using namespace MossUI;

bool Near(float a, float b) { return std::fabs(a - b) < 1e-4f; }

struct Env {
    testing::TestBackend backend;
    testing::TestHost host;
    Env() { SetBackend(&backend); SetHost(&host); ResetSkins(); SetStore(nullptr); }
};

void TestNineSliceQuads() {
    std::array<QuadSpec, 9> q;
    // 16x16 texture with 4px borders drawn into 40x30: corners keep 4x4, the middle stretches
    const int n = NineSliceQuads({10, 50, 20, 50}, {16, 4, 16, 4}, q);
    assert(n == 9);
    assert(Near(q[0].dst.x0, 10) && Near(q[0].dst.x1, 14) && Near(q[0].dst.y0, 20) && Near(q[0].dst.y1, 24));
    assert(Near(q[4].dst.x0, 14) && Near(q[4].dst.x1, 46) && Near(q[4].dst.y0, 24) && Near(q[4].dst.y1, 46));
    assert(Near(q[8].dst.x0, 46) && Near(q[8].dst.x1, 50) && Near(q[8].dst.y1, 50));
    assert(Near(q[0].uv.u0, 0) && Near(q[0].uv.u1, 0.25f));
    assert(Near(q[4].uv.u0, 0.25f) && Near(q[4].uv.u1, 0.75f));
    assert(Near(q[8].uv.u0, 0.75f) && Near(q[8].uv.u1, 1.0f));

    // too small for a middle: only the four corner cells that still have size are emitted
    const int tiny = NineSliceQuads({0, 8, 0, 8}, {16, 4, 16, 4}, q);
    assert(tiny == 4);
}

void TestImageAndTint() {
    Env env;
    env.backend.availableImages.insert("tex.png");
    UI ui;
    assert(!ui.Image({0, 10, 0, 10}, "missing.png"));
    assert(env.backend.quads.empty() && env.backend.flushes.empty());

    assert(ui.Image({0, 10, 0, 10}, "tex.png", Color{255, 0, 0, 255}));
    assert(env.backend.quads.size() == 1 && env.backend.flushes.size() == 1);
    assert(env.backend.flushes[0].r == 255 && env.backend.flushes[0].g == 0);
}

void TestTiled() {
    Env env;
    env.backend.availableImages.insert("tile.png");
    UI ui;
    assert(ui.ImageTiled({0, 70, 0, 40}, "tile.png", 32.0f));
    // 3 columns (32, 32, 6) x 2 rows (32, 8); one flush per row
    assert(env.backend.quads.size() == 6);
    assert(env.backend.flushes.size() == 2);
    const auto& last = env.backend.quads.back();
    assert(Near(last.rect.x1 - last.rect.x0, 6) && Near(last.rect.y1 - last.rect.y0, 8));
    assert(Near(last.uv.u1, 6.0f / 32.0f) && Near(last.uv.v1, 8.0f / 32.0f));
    assert(!ui.ImageTiled({0, 10, 0, 10}, "tile.png", 0.0f));
}

void TestSkinsAreData() {
    Env env;
    UI ui;
    assert(FindSkin(skin::PANEL) != nullptr);
    assert(!ui.Skinned({0, 40, 0, 40}, skin::PANEL));   // texture not available: caller falls back to fills

    env.backend.availableImages.insert("textures/ui/custom_panel");
    SetSkin(skin::PANEL, {"textures/ui/custom_panel", {8, 2, 8, 2}});
    assert(ui.Skinned({0, 40, 0, 40}, skin::PANEL));
    assert(env.backend.quads.size() == 9 && env.backend.quads[0].path == "textures/ui/custom_panel");
    assert(env.backend.flushes.size() == 1);

    // a Panel now draws through the skin instead of the fallback fill
    env.backend.fills.clear();
    ui.Panel({0, 40, 0, 40});
    assert(env.backend.fills.empty());

    assert(!ui.Skinned({0, 10, 0, 10}, "no_such_skin"));
    ResetSkins();
    assert(FindSkin(skin::PANEL)->path == "textures/ui/dialog_background_opaque");
}

// A RenderApi that records what the game would be asked to do.
struct FakeApi : RenderApi {
    std::set<std::string> loadable;
    std::map<std::string, int> loads;
    int quads = 0, flushes = 0, clipSets = 0, fills = 0;
    const Rect* lastClip = nullptr;
    Rect lastClipValue{};
    std::vector<std::string> missing;

    void FillRect(const Rect&, const Color&, float) override { ++fills; }
    Texture LoadTexture(const std::string& path, std::string& why) override {
        ++loads[path];
        if (!loadable.contains(path)) { why = "not loadable"; return nullptr; }
        return std::make_shared<int>(1);
    }
    void DrawQuad(const void* tex, const Rect&, const Uv&) override { assert(tex); ++quads; }
    void FlushQuads(const Color&) override { ++flushes; }
    void SetClip(const Rect* clip) override { ++clipSets; lastClip = clip; if (clip) lastClipValue = *clip; }
    void DrawText(const TextRun&) override {}
    void FlushText() override {}
    float TextWidth(const std::string& s) override { return static_cast<float>(s.size()) * 6.0f; }
    void OnTextureMissing(const std::string& path, const std::string&) override { missing.push_back(path); }
};

void TestRenderBackendTextureSettling() {
    FakeApi api;
    RenderBackend backend(api);

    backend.BeginFrame();
    // not loadable yet: unavailable, but not reported while settling
    assert(!backend.Quad({0, 1, 0, 1}, "late.png", {}));
    for (int i = 0; i < RenderBackend::SETTLE_FRAMES - 2; ++i) { backend.BeginFrame(); assert(!backend.Quad({0, 1, 0, 1}, "late.png", {})); }
    assert(api.missing.empty());

    // the game finishes loading it while we are still settling: picked up
    api.loadable.insert("late.png");
    backend.BeginFrame();
    assert(backend.Quad({0, 1, 0, 1}, "late.png", {}));
    assert(api.quads == 1);

    // a texture that never loads is reported once, after settling, and retried only occasionally
    backend.BeginFrame();
    for (int i = 0; i < RenderBackend::SETTLE_FRAMES + 2; ++i) { backend.BeginFrame(); assert(!backend.Quad({0, 1, 0, 1}, "gone.png", {})); }
    assert(api.missing.size() == 1 && api.missing[0] == "gone.png");
    const int loadsBefore = api.loads["gone.png"];
    for (int i = 0; i < 20; ++i) { backend.BeginFrame(); backend.Quad({0, 1, 0, 1}, "gone.png", {}); }
    assert(api.loads["gone.png"] == loadsBefore);                      // no per-frame retry once settled
    api.loadable.insert("gone.png");
    for (int i = 0; i < RenderBackend::RETRY_FRAMES; ++i) backend.BeginFrame();
    assert(backend.Quad({0, 1, 0, 1}, "gone.png", {}));                // picked up by the slow retry
    assert(api.missing.size() == 1);

    backend.ResetTextures();
    const int before = api.loads["late.png"];
    backend.BeginFrame();
    backend.Quad({0, 1, 0, 1}, "late.png", {});
    assert(api.loads["late.png"] == before + 1);                        // cache dropped: asked the game again
}

void TestRenderBackendClipStack() {
    FakeApi api;
    RenderBackend backend(api);
    backend.BeginFrame();
    assert(api.lastClip == nullptr);
    backend.PushClip({0, 100, 0, 100});
    assert(api.lastClip && api.lastClipValue.x1 == 100);
    backend.PushClip({10, 50, 10, 50});
    assert(api.lastClipValue.x1 == 50);
    backend.PopClip();
    assert(api.lastClip && api.lastClipValue.x1 == 100);
    backend.PopClip();
    assert(api.lastClip == nullptr);
    backend.PushClip({0, 5, 0, 5});
    backend.EndFrame();                                                 // a frame that forgot to pop is cleaned up
    assert(api.lastClip == nullptr);
}

void TestSettingRows() {
    bool flag = false;
    int number = 7;
    Setting t = Setting::Var("Flag", "k.flag", flag, true);
    Setting s = Setting::Var("Number", "k.num", number, 0, 100, 50, 5, "%");
    assert(t.As<row::Toggle>() && !t.As<row::Slider>());
    assert(s.As<row::Slider>() && s.As<row::Slider>()->unit == "%" && s.As<row::Slider>()->def == 50.0f);

    t.As<row::Toggle>()->set(true);
    assert(flag);
    s.As<row::Slider>()->set(12.4f);
    assert(number == 12);

    Setting cat = Setting::Category("Cat", {Setting::Var("Flag", "k.flag", flag, false), Setting::Var("Number", "k.num", number, 0, 100, 3)});
    cat.Reset();
    assert(!flag && number == 3);

    // Store receives the writes
    struct Mem : Store {
        std::map<std::string, int> ints;
        std::map<std::string, bool> bools;
        bool GetBool(const std::string& k, bool d) override { return bools.contains(k) ? bools[k] : d; }
        void SetBool(const std::string& k, bool v) override { bools[k] = v; }
        int GetInt(const std::string& k, int d) override { return ints.contains(k) ? ints[k] : d; }
        void SetInt(const std::string& k, int v) override { ints[k] = v; }
        std::string GetString(const std::string&, const std::string& d) override { return d; }
        void SetString(const std::string&, const std::string&) override {}
    } mem;
    SetStore(&mem);
    Setting stored = Setting::Var("Flag", "k.stored", flag, false);
    stored.As<row::Toggle>()->set(true);
    assert(mem.bools["k.stored"] == true);
    Setting storedNum = Setting::Var("Number", "k.storednum", number, 0, 100, 3);
    storedNum.As<row::Slider>()->set(41.0f);
    assert(mem.ints["k.storednum"] == 41);
    SetStore(nullptr);
}

void TestTabsAndVisibility() {
    bool on = false;
    bool dummy = false;
    std::vector<Setting> rows;
    rows.push_back(Setting::Var("Loose", "k.l", dummy, false));
    rows.push_back(Setting::Category("Advanced", {Setting::Section("Inner")}));
    rows.push_back(Setting::Category("General", {Setting::Section("Existing")}));
    auto tabs = settings::BuildTabs(rows);
    assert(tabs.size() == 2 && tabs[0].title == "Advanced" && tabs[1].title == "General");
    assert(tabs[1].rows.size() == 2);                                   // loose row joined the existing General tab

    std::vector<Setting> noGeneral;
    noGeneral.push_back(Setting::Var("Loose", "k.l", dummy, false));
    noGeneral.push_back(Setting::Category("Advanced", {}));
    tabs = settings::BuildTabs(std::move(noGeneral));
    assert(tabs.size() == 2 && tabs[0].title == "General");             // created in front

    std::vector<Setting> flat;
    flat.push_back(Setting::Var("A", "k.a", dummy, false));
    tabs = settings::BuildTabs(std::move(flat));
    assert(tabs.size() == 1 && tabs[0].title.empty());                  // no categories: one untitled page

    std::vector<Setting> gated;
    gated.push_back(Setting::Var("Gate", "k.g", on, false));
    gated.push_back(Setting::Var("Child", "k.c", dummy, false).ShownIf([&] { return on; }));
    std::vector<bool> before, after;
    settings::VisibilitySignature(gated, before);
    on = true;
    settings::VisibilitySignature(gated, after);
    assert(before != after);
}

void TestAddRows() {
    Env env;
    bool flag = false;
    int number = 1, choice = 0, color = 0xFF0000, key = 65;
    std::string text;
    int actions = 0;
    std::vector<Setting> rows;
    rows.push_back(Setting::Section("Sec"));
    rows.push_back(Setting::Var("Flag", "k.f", flag, false).Tip("tip"));
    rows.push_back(Setting::Var("Number", "k.n", number, 0, 10, 1));
    rows.push_back(Setting::Choice("Choice", {"a", "b"}, [&] { return choice; }, [&](int v) { choice = v; }));
    rows.push_back(Setting::Color("Color", [&] { return color; }, [&](int v) { color = v; }, 0));
    rows.push_back(Setting::Text("Text", [&] { return text; }, [&](const std::string& v) { text = v; }));
    rows.push_back(Setting::KeyBind("Key", [&] { return key; }, [&](int v) { key = v; }, 65));
    rows.push_back(Setting::Action("Go", [&] { ++actions; }));
    rows.push_back(Setting::Var("Hidden", "k.h", flag, false).ShownIf([] { return false; }));
    rows.push_back(Setting::Category("Cat", {Setting::Section("Inside")}));

    ListWidget list;
    list.bounds = {0, 300, 0, 400};
    settings::RowOptions options;
    std::string conflictLabel;
    options.keyConflict = [&](int code, const std::string& label) { conflictLabel = label; return code == 66 ? "Other: row" : ""; };
    settings::AddRows(list, rows, options);

    // Section, Flag, Number, Choice, Color, Text, Key, Action, (Hidden skipped), Cat heading, Inside = 10
    assert(list.children.size() == 10);
    assert(list.children[1]->tooltip == "tip");

    auto* keyWidget = dynamic_cast<KeyBindOptionWidget*>(list.children[6].get());
    assert(keyWidget && keyWidget->conflict);
    assert(keyWidget->conflict(66) == "Other: row" && conflictLabel == "Key");

    settings::RowOptions noTips;
    noTips.tooltips = false;
    ListWidget plain;
    settings::AddRows(plain, rows, noTips);
    assert(plain.children[1]->tooltip.empty());
}

void TestBoxLayout() {
    Box row(Axis::Horizontal, 8.0f);
    row.bounds = {100, 408, 200, 220};
    auto& left = row.Flex<ButtonWidget>(1, "L", [] {});
    auto& right = row.Flex<ButtonWidget>(1, "R", [] {});
    row.Arrange();
    // two flex children share 308 - 8 gap: 150 each (matches the hand-placed footer rows in the mods)
    assert(Near(left.bounds.x0, 100) && Near(left.bounds.x1, 250));
    assert(Near(right.bounds.x0, 258) && Near(right.bounds.x1, 408));
    assert(Near(left.bounds.y0, 200) && Near(left.bounds.y1, 220));

    Box col(Axis::Vertical, 2.0f, {5, 5, 5, 5});
    col.bounds = {0, 100, 0, 100};
    auto& top = col.Fixed<ButtonWidget>(20, "T", [] {});
    auto& fill = col.Flex<ButtonWidget>(1, "F", [] {});
    auto& hidden = col.Flex<ButtonWidget>(1, "H", [] {});
    hidden.visible = false;
    auto& weighted = col.Flex<ButtonWidget>(2, "W", [] {});
    col.Arrange();
    // inner height 90, minus 20 fixed and two 2px gaps (3 shown) = 66 free, split 1:2
    assert(Near(top.bounds.y0, 5) && Near(top.bounds.y1, 25) && Near(top.bounds.x0, 5) && Near(top.bounds.x1, 95));
    assert(Near(fill.bounds.y0, 27) && Near(fill.bounds.y1 - fill.bounds.y0, 22));
    assert(Near(weighted.bounds.y1 - weighted.bounds.y0, 44) && Near(weighted.bounds.y1, 95));

    // boxes nest, and a Spacer pushes siblings apart without being hit-testable
    Box outer(Axis::Horizontal);
    outer.bounds = {0, 200, 0, 20};
    outer.Fixed<ButtonWidget>(50, "A", [] {});
    auto& gap = outer.Flex<Spacer>(1);
    auto& end = outer.Fixed<ButtonWidget>(50, "B", [] {});
    outer.Arrange();
    assert(Near(gap.bounds.x0, 50) && Near(gap.bounds.x1, 150) && Near(end.bounds.x0, 150));
    assert(gap.HitTest(100, 10) == nullptr);

    Box nestedParent(Axis::Vertical);
    nestedParent.bounds = {0, 100, 0, 100};
    auto& inner = nestedParent.Flex<Box>(1, Axis::Horizontal, 0.0f);
    inner.Flex<ButtonWidget>(1, "x", [] {});
    nestedParent.Arrange();
    assert(Near(inner.bounds.x1, 100) && Near(inner.bounds.y1, 100));
    inner.Arrange();
    assert(Near(inner.children[0]->bounds.x1, 100));
}

void TestScreenBox() {
    // end to end: a Screen using Box re-lays itself out on resize with no pixel maths in Init
    struct S : Screen {
        void Init(float w, float h) override {
            auto& footer = root.Add<Box>(Axis::Horizontal, 8.0f);
            footer.bounds = {w / 2 - 154, w / 2 + 154, h - 28, h - 8};
            done = &footer.Flex<ButtonWidget>(1, "Done", [this] { ++closed; });
        }
        ButtonWidget* done = nullptr;
        int closed = 0;
    };
    Env env;
    auto screen = std::make_shared<S>();
    Open(screen);
    Render(800, 600);
    assert(Near(screen->done->bounds.x0, 400 - 154) && Near(screen->done->bounds.x1, 400 + 154));
    Render(1000, 700);
    assert(Near(screen->done->bounds.x0, 500 - 154) && Near(screen->done->bounds.y0, 672));
    OnMouse(0, 0, 500, 680);
    OnMouse(1, 1, 500, 680);
    OnMouse(1, 0, 500, 680);
    assert(screen->closed == 1);
    Close();
}

}

int main(int argc, char** argv) {
    struct Case { const char* name; void (*fn)(); };
    const Case cases[] = {
        {"NineSliceQuads", TestNineSliceQuads}, {"ImageAndTint", TestImageAndTint}, {"Tiled", TestTiled},
        {"SkinsAreData", TestSkinsAreData}, {"RenderBackendTextureSettling", TestRenderBackendTextureSettling},
        {"RenderBackendClipStack", TestRenderBackendClipStack}, {"SettingRows", TestSettingRows},
        {"TabsAndVisibility", TestTabsAndVisibility}, {"AddRows", TestAddRows}, {"BoxLayout", TestBoxLayout},
        {"ScreenBox", TestScreenBox},
    };
    for (const Case& c : cases) {
        if (argc > 1 && std::string(argv[1]) != c.name) continue;
        c.fn();
        std::printf("ok  %s\n", c.name);
    }
}
