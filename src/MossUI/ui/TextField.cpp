#include "MossUI/ui/UI.hpp"

#include "MossUI/core/Keys.hpp"
#include "MossUI/core/State.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>

namespace MossUI {

namespace {
using detail::S;
char CharFor(int key, bool shift) {
    if (key >= 65 && key <= 90) return static_cast<char>(shift ? key : key + 32);
    if (key >= 48 && key <= 57) return shift ? ")!@#$%^&*("[key - 48] : static_cast<char>(key);
    if (key >= 96 && key <= 105) return static_cast<char>('0' + (key - 96));
    if (key == 32) return ' ';
    struct Oem { int vk; char plain, shifted; };
    static constexpr Oem oem[] = {{186, ';', ':'}, {187, '=', '+'}, {188, ',', '<'}, {189, '-', '_'}, {190, '.', '>'},
                                  {191, '/', '?'}, {192, '`', '~'}, {219, '[', '{'}, {220, '\\', '|'}, {221, ']', '}'},
                                  {222, '\'', '"'}};
    for (const auto& o : oem)
        if (o.vk == key) return shift ? o.shifted : o.plain;
    return 0;
}
bool HasSel(const TextFieldState& f) { return f.caret != f.anchor; }
void DeleteSel(TextFieldState& f) {
    const int a = std::min(f.caret, f.anchor), b = std::max(f.caret, f.anchor);
    f.text.erase(static_cast<size_t>(a), static_cast<size_t>(b - a));
    f.caret = f.anchor = a;
}
void InsertText(TextFieldState& f, const std::string& text) {
    std::string clean;
    for (char c : text) if (c >= 32 && c < 127) clean += c;
    if (HasSel(f)) DeleteSel(f);
    clean.resize(std::min(clean.size(), f.maxLen - std::min(f.text.size(), f.maxLen)));
    f.text.insert(static_cast<size_t>(f.caret), clean);
    f.caret = f.anchor = f.caret + static_cast<int>(clean.size());
}
int WordLeft(const TextFieldState& f, int i) {
    while (i > 0 && f.text[static_cast<size_t>(i - 1)] == ' ') --i;
    while (i > 0 && f.text[static_cast<size_t>(i - 1)] != ' ') --i;
    return i;
}
int WordRight(const TextFieldState& f, int i) {
    const int n = static_cast<int>(f.text.size());
    while (i < n && f.text[static_cast<size_t>(i)] != ' ') ++i;
    while (i < n && f.text[static_cast<size_t>(i)] == ' ') ++i;
    return i;
}
void MoveCaret(TextFieldState& f, int to) {
    f.caret = std::clamp(to, 0, static_cast<int>(f.text.size()));
    if (!S().shift) f.anchor = f.caret;
}
}

namespace detail {
void SelectAll(TextFieldState& f) { f.anchor = 0; f.caret = static_cast<int>(f.text.size()); }

void PlaceCaret(TextFieldState& f, float mouseX, bool extend) {
    int k = 0;
    const float rel = mouseX - f.textX0;
    for (size_t i = 0; i < f.bounds.size(); ++i)
        if (std::fabs(f.bounds[i] - rel) < std::fabs(f.bounds[static_cast<size_t>(k)] - rel)) k = static_cast<int>(i);
    f.caret = f.viewStart + k;
    if (!extend) f.anchor = f.caret;
}

void TextKey(TextFieldState& f, int key) {
    const bool ctrl = S().ctrl;
    const int n = static_cast<int>(f.text.size());
    if (ctrl) {
        const int a = std::min(f.caret, f.anchor), b = std::max(f.caret, f.anchor);
        switch (key) {
            case 'A' : SelectAll(f); return;
            case 'C' : if (HasSel(f)) S().backend->ClipboardSet(f.text.substr(static_cast<size_t>(a), static_cast<size_t>(b - a))); return;
            case 'X' : if (HasSel(f)) { S().backend->ClipboardSet(f.text.substr(static_cast<size_t>(a), static_cast<size_t>(b - a))); DeleteSel(f); } return;
            case 'V' : InsertText(f, S().backend->ClipboardGet()); return;
            default : break;
        }
    }
    switch (key) {
        case 8 :
            if (HasSel(f)) DeleteSel(f);
            else if (f.caret > 0) {
                const int from = ctrl ? WordLeft(f, f.caret) : f.caret - 1;
                f.text.erase(static_cast<size_t>(from), static_cast<size_t>(f.caret - from));
                f.caret = f.anchor = from;
            }
            return;
        case 46 :
            if (HasSel(f)) DeleteSel(f);
            else if (f.caret < n) {
                const int to = ctrl ? WordRight(f, f.caret) : f.caret + 1;
                f.text.erase(static_cast<size_t>(f.caret), static_cast<size_t>(to - f.caret));
            }
            return;
        case 37 :
            if (HasSel(f) && !S().shift) f.caret = f.anchor = std::min(f.caret, f.anchor);
            else MoveCaret(f, ctrl ? WordLeft(f, f.caret) : f.caret - 1);
            return;
        case 39 :
            if (HasSel(f) && !S().shift) f.caret = f.anchor = std::max(f.caret, f.anchor);
            else MoveCaret(f, ctrl ? WordRight(f, f.caret) : f.caret + 1);
            return;
        case 36 : MoveCaret(f, 0); return;
        case 35 : MoveCaret(f, n); return;
        case 13 : f.focused = false; return;
        default : break;
    }
    if (char c = CharFor(key, S().shift)) InsertText(f, std::string(1, c));
}
}

void UI::DrawTextField(TextFieldState& f, const Rect& sb, const std::string& placeholder) {
    auto& s = S();
    f.rect = sb;
    Fill(sb, col::DARK);
    Outline(sb, f.focused ? col::WHITE : col::BLACK);

    const int len = static_cast<int>(f.text.size());
    f.caret = std::clamp(f.caret, 0, len);
    f.anchor = std::clamp(f.anchor, 0, len);
    const float tx0 = sb.x0 + 6, avail = (sb.x1 - sb.x0) - 14;
    f.textX0 = tx0;
    const float ty = sb.y0 + std::max(5.0f, ((sb.y1 - sb.y0) - 8.0f) * 0.5f);
    const float shift = ty - (sb.y0 + 5.0f);
    f.viewStart = std::clamp(f.viewStart, 0, len);
    if (f.caret < f.viewStart) f.viewStart = f.caret;
    while (f.viewStart < f.caret &&
           TextWidth(f.text.substr(static_cast<size_t>(f.viewStart), static_cast<size_t>(f.caret - f.viewStart))) > avail - 2)
        ++f.viewStart;
    int n = 0;
    while (f.viewStart + n < len &&
           TextWidth(f.text.substr(static_cast<size_t>(f.viewStart), static_cast<size_t>(n + 1))) <= avail) ++n;
    f.bounds.assign(static_cast<size_t>(n) + 1, 0.0f);
    for (int k = 1; k <= n; ++k)
        f.bounds[static_cast<size_t>(k)] = TextWidth(f.text.substr(static_cast<size_t>(f.viewStart), static_cast<size_t>(k)));

    if (f.text.empty() && !f.focused) {
        Text(tx0, sb.x1, ty, placeholder, col::GRAY, Align::Left);
    } else {
        if (f.focused && f.caret != f.anchor) {
            const int s0 = std::clamp(std::min(f.caret, f.anchor) - f.viewStart, 0, n);
            const int s1 = std::clamp(std::max(f.caret, f.anchor) - f.viewStart, 0, n);
            if (s0 < s1)
                Fill({tx0 + f.bounds[static_cast<size_t>(s0)], tx0 + f.bounds[static_cast<size_t>(s1)], sb.y0 + 3 + shift, sb.y1 - 3 - shift},
                     CurrentTheme().selection);
        }
        Text(tx0, sb.x1, ty, f.text.substr(static_cast<size_t>(f.viewStart), static_cast<size_t>(n)), col::WHITE, Align::Left);
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        if (f.focused && (ms / 530) % 2 == 0) {
            const int c = std::clamp(f.caret - f.viewStart, 0, n);
            const float cx = tx0 + f.bounds[static_cast<size_t>(c)];
            Fill({cx, cx + 1, sb.y0 + 3 + shift, sb.y1 - 3 - shift}, col::WHITE);
        }
    }
    if (f.drag && s.down && s.hasMouse) {
        const float rel = s.mx - tx0;
        int k = 0;
        for (int i = 0; i <= n; ++i)
            if (std::fabs(f.bounds[static_cast<size_t>(i)] - rel) < std::fabs(f.bounds[static_cast<size_t>(k)] - rel)) k = i;
        int pos = f.viewStart + k;
        if (rel < -2 && f.viewStart > 0) pos = f.viewStart - 1;
        else if (rel > f.bounds[static_cast<size_t>(n)] + 2 && f.viewStart + n < len) pos = f.viewStart + n + 1;
        f.caret = pos;
    }
}

}
