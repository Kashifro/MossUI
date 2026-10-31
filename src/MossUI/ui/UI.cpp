#include "MossUI/ui/UI.hpp"

#include "MossUI/core/State.hpp"

#include <algorithm>
#include <array>
#include <map>
#include <system_error>

namespace MossUI {

namespace detail {
State& S() {
    static State s;
    return s;
}

void FlushText() {
    auto& s = S();
    for (const auto& run : s.texts) s.backend->Text(run);
    s.texts.clear();
    s.backend->FlushText();
}

void DrawTooltipBox() {
    auto& s = S();
    UI ui;
    constexpr float horizontalPadding = 4.0f;
    const auto& metrics = CurrentWidgetMetrics();
    const auto lines = ui.Wrap(s.tip, metrics.tooltipMaxWidth - horizontalPadding * 2.0f);
    if (lines.empty()) return;
    float textWidth = 0.0f;
    for (const auto& line : lines) textWidth = std::max(textWidth, ui.TextWidth(line));
    const float w = std::min(metrics.tooltipMaxWidth, textWidth + horizontalPadding * 2.0f);
    const float h = static_cast<float>(lines.size()) * 11.0f + 8.0f;
    const float x = std::clamp(s.mx + 10.0f, 2.0f, std::max(2.0f, s.w - w - 2.0f));
    const float y = std::clamp(s.my + 12.0f, 2.0f, std::max(2.0f, s.h - h - 2.0f));
    const Rect box{x, x + w, y, y + h};
    ui.Fill(box, col::BLACK);
    ui.Outline(box, CurrentTheme().border);
    float ty = y + 4;
    for (const auto& l : lines) {
        ui.Text(x + horizontalPadding, x + w - horizontalPadding, ty, l, col::WHITE, Align::Left, 1.0f);
        ty += 11;
    }
}
}

namespace {
using detail::S;
constexpr float CHAR_W = 6.0f;

Rect Intersect(const Rect& a, const Rect& b) {
    const float x0 = std::max(a.x0, b.x0);
    const float y0 = std::max(a.y0, b.y0);
    return {x0, std::max(x0, std::min(a.x1, b.x1)),
            y0, std::max(y0, std::min(a.y1, b.y1))};
}

bool HasArea(const Rect& rect) { return rect.x1 > rect.x0 && rect.y1 > rect.y0; }
}

void SetClipboardText(const std::string& text) { if (S().host) S().host->ClipboardSet(text); }

namespace {
std::map<std::string, SkinDef>& Skins() {
    static std::map<std::string, SkinDef> skins = {
        {skin::PANEL, {"textures/ui/dialog_background_opaque", {16, 4, 16, 4}}},
        {skin::SCROLL_HANDLE, {"textures/ui/ScrollHandle", {3, 1, 5, 2}}},
        {skin::SLIDER_BUTTON, {"textures/ui/slider_button_default", {6, 2, 6, 2}}},
        {skin::SLIDER_BUTTON_HOVER, {"textures/ui/slider_button_hover", {6, 2, 6, 2}}},
    };
    return skins;
}
}

void SetSkin(const std::string& name, SkinDef def) { Skins()[name] = std::move(def); }

const SkinDef* FindSkin(const std::string& name) {
    const auto it = Skins().find(name);
    return it == Skins().end() ? nullptr : &it->second;
}

void ResetSkins() {
    auto& skins = Skins();
    skins.clear();
    skins[skin::PANEL] = {"textures/ui/dialog_background_opaque", {16, 4, 16, 4}};
    skins[skin::SCROLL_HANDLE] = {"textures/ui/ScrollHandle", {3, 1, 5, 2}};
    skins[skin::SLIDER_BUTTON] = {"textures/ui/slider_button_default", {6, 2, 6, 2}};
    skins[skin::SLIDER_BUTTON_HOVER] = {"textures/ui/slider_button_hover", {6, 2, 6, 2}};
}

void SetAssetDir(std::filesystem::path dir) {
    S().assetDir = std::move(dir);
    S().assetCache.clear();
}

std::string AssetPath(const std::string& name) {
    auto& s = S();
    if (s.assetDir.empty()) return {};
    const auto it = s.assetCache.find(name);
    if (it != s.assetCache.end()) return it->second;
    std::error_code ec;
    const auto file = s.assetDir / (name + ".png");
    if (!std::filesystem::is_regular_file(file, ec) || ec) return {};
    return s.assetCache[name] = file.generic_string();
}

float UI::Width() const { return S().w; }
float UI::Height() const { return S().h; }
float UI::MouseX() const { return S().mx; }
float UI::MouseY() const { return S().my; }
bool UI::InGame() const { return S().inGame; }
bool UI::HasMouse() const { return S().hasMouse; }
bool UI::MouseDown() const { return S().down; }
void UI::BlockHover(bool blocked) { S().hoverBlocked = blocked; }
bool UI::HoverBlocked() const { return S().hoverBlocked; }

bool UI::Hovered(const Rect& r) const {
    const auto& s = S();
    if (!s.hasMouse || s.hoverBlocked) return false;
    if (!s.clips.empty()) {
        const Rect& k = s.clips.back();
        if (s.mx < k.x0 || s.mx > k.x1 || s.my < k.y0 || s.my > k.y1) return false;
    }
    return s.mx >= r.x0 && s.mx <= r.x1 && s.my >= r.y0 && s.my <= r.y1;
}

void UI::Fill(const Rect& r, const Color& c, float alpha) {
    const auto& clips = S().clips;
    const Rect clipped = clips.empty() ? r : Intersect(r, clips.back());
    if (HasArea(clipped)) S().backend->Fill(clipped, c, alpha);
}

void UI::Outline(const Rect& r, const Color& c, float alpha) {
    Fill({r.x0, r.x1, r.y0, r.y0 + 1}, c, alpha);
    Fill({r.x0, r.x1, r.y1 - 1, r.y1}, c, alpha);
    Fill({r.x0, r.x0 + 1, r.y0 + 1, r.y1 - 1}, c, alpha);
    Fill({r.x1 - 1, r.x1, r.y0 + 1, r.y1 - 1}, c, alpha);
}

void UI::Backdrop(float alpha) { Fill({0, S().w, 0, S().h}, col::DIM, alpha); }

void UI::Panel(const Rect& r) {
    if (Skinned(r, skin::PANEL)) return;
    Fill(r, CurrentTheme().panel, 0.95f);
    Outline(r, CurrentTheme().border);
}

void UI::PushClip(const Rect& r) {
    detail::FlushText();
    auto& state = S();
    const Rect clipped = state.clips.empty() ? r : Intersect(r, state.clips.back());
    state.clips.push_back(clipped);
    state.backend->PushClip(clipped);
}

void UI::PopClip() {
    detail::FlushText();
    auto& state = S();
    if (state.clips.empty()) return;
    state.backend->PopClip();
    state.clips.pop_back();
}

bool UI::Image(const Rect& r, const std::string& path, const Color& tint) {
    Backend* b = S().backend;
    if (!b->Quad(r, path, Uv{})) return false;
    b->FlushQuads(tint);
    return true;
}

bool UI::NineSlice(const Rect& r, const std::string& path, const Slice& slice, const Color& tint) {
    Backend* b = S().backend;
    std::array<QuadSpec, 9> quads;
    const int n = NineSliceQuads(r, slice, quads);
    for (int i = 0; i < n; ++i)
        if (!b->Quad(quads[static_cast<size_t>(i)].dst, path, quads[static_cast<size_t>(i)].uv)) return false;   // all nine share one texture
    b->FlushQuads(tint);
    return true;
}

bool UI::Skinned(const Rect& r, const std::string& skinName, const Color& tint) {
    const SkinDef* def = FindSkin(skinName);
    return def && NineSlice(r, def->path, def->slice, tint);
}

bool UI::DrawGlyph(Rect r, const std::string& name, bool enabled, bool hov, bool down) {
    const char* state = !enabled ? "" : down ? "_pressed" : hov ? "_hover" : "";
    std::string path = AssetPath(name + state);
    if (down && enabled) { r.x0 += 1; r.x1 += 1; r.y0 += 1; r.y1 += 1; }
    if (!path.empty() && Image(r, path)) return true;
    if (state[0] == '\0') return false;
    path = AssetPath(name);
    return !path.empty() && Image(r, path);
}

void UI::Preload(const std::string& path) { S().backend->Preload(path); }

bool UI::ImageTiled(const Rect& r, const std::string& path, float tile, const Color& tint) {
    if (tile <= 0.0f) return false;
    Backend* b = S().backend;
    for (float y = r.y0; y < r.y1; y += tile) {
        const float h = std::min(tile, r.y1 - y);
        for (float x = r.x0; x < r.x1; x += tile) {
            const float w = std::min(tile, r.x1 - x);
            if (!b->Quad({x, x + w, y, y + h}, path, Uv{0, 0, w / tile, h / tile})) return false;
        }
        b->FlushQuads(tint);   // one flush per row keeps the batch small
    }
    return true;
}

void UI::Text(float x0, float x1, float y, const std::string& s, const Color& c, Align align, float alpha, bool shadow) {
    const float scale = CurrentAccessibilityOptions().textScale;
    S().texts.push_back({{x0, x1, y, y + 10.0f * scale}, s, c, std::min(alpha, 0.95f), align, shadow, scale});
}

void UI::LinkText(float x, float maxX, float y, const std::string& text, const Color& color, bool hovered,
                  bool pressed) {
    const auto& theme = CurrentTheme();
    const Color linkColor = pressed ? theme.linkPressed
                                    : hovered ? theme.linkHover
                                              : (color.r == 85 && color.g == 85 ? theme.link : color);
    Text(x, maxX, y, text, linkColor, Align::Left, 1.0f);
    Fill({x, std::min(maxX, x + TextWidth(text)), y + 9.0f, y + 10.0f}, linkColor);
}

float UI::TextWidth(const std::string& s) const {
    if (s.empty()) return 0.0f;
    const float w = S().backend->TextWidth(s);
    const float width = w > 0 ? w : static_cast<float>(s.size()) * CHAR_W;
    return width * CurrentAccessibilityOptions().textScale;
}

std::string UI::Trunc(const std::string& s, float width) const {
    if (TextWidth(s) <= width) return s;
    if (width <= 0.0f) return {};

    std::vector<size_t> boundaries{0};
    for (size_t i = 0; i < s.size();) {
        const auto lead = static_cast<unsigned char>(s[i]);
        size_t length = lead < 0x80 ? 1 : (lead & 0xE0) == 0xC0 ? 2 : (lead & 0xF0) == 0xE0 ? 3 :
                                              (lead & 0xF8) == 0xF0 ? 4 : 1;
        if (i + length > s.size()) length = 1;
        i += length;
        boundaries.push_back(i);
    }

    auto longestFit = [&](const std::string& suffix) {
        size_t low = 0, high = boundaries.size() - 1;
        while (low < high) {
            const size_t mid = low + (high - low + 1) / 2;
            const std::string candidate = s.substr(0, boundaries[mid]) + suffix;
            if (TextWidth(candidate) <= width) low = mid;
            else high = mid - 1;
        }
        return low;
    };

    const size_t withEllipsis = longestFit("...");
    if (withEllipsis > 0 || TextWidth("...") <= width)
        return s.substr(0, boundaries[withEllipsis]) + "...";
    return s.substr(0, boundaries[longestFit("")]);
}

std::vector<std::string> UI::Wrap(const std::string& s, float width) const {
    std::vector<std::string> lines;
    std::string line, word;
    auto pushLine = [&] { lines.push_back(std::move(line)); line.clear(); };
    auto flushWord = [&] {
        if (word.empty()) return;
        while (word.size() > 1 && TextWidth(word) > width) {
            size_t n = 1;
            while (n < word.size() && TextWidth(word.substr(0, n + 1)) <= width) ++n;
            if (!line.empty()) pushLine();
            lines.push_back(word.substr(0, n));
            word.erase(0, n);
        }
        const std::string cand = line.empty() ? word : line + " " + word;
        if (!line.empty() && TextWidth(cand) > width) { pushLine(); line = std::move(word); }
        else line = cand;
        word.clear();
    };
    for (char ch : s) {
        if (ch == '\n') { flushWord(); pushLine(); }
        else if (ch == ' ' || ch == '\t') flushWord();
        else word += ch;
    }
    flushWord();
    if (!line.empty()) pushLine();
    return lines;
}

void UI::Click() { if (S().host) S().host->Click(); }

void UI::Tooltip(const std::string& text) {
    if (!S().hoverBlocked) S().tip = text;
}

Canvas::Canvas(UI& ui, const Rect& bounds) : ui(&ui) { ui.PushClip(bounds); }

Canvas::~Canvas() {
    if (ui) ui->PopClip();
}

}
