#pragma once

#include <string>

namespace MossUI {

enum class KeyCode {
    UNKNOWN = 0,
    BACK = 4,
    BACKSPACE = 8,
    TAB = 9,
    ENTER = 13,
    LEFT_SHIFT = 16,
    RIGHT_SHIFT = 16 | 256,
    LEFT_CTRL = 17,
    RIGHT_CTRL = 17 | 256,
    PAUSE = 19,
    CAPS_LOCK = 20,
    ESCAPE = 27,
    SPACE = 32,
    PAGE_UP = 33,
    PAGE_DOWN = 34,
    END = 35,
    HOME = 36,
    LEFT = 37,
    UP = 38,
    RIGHT = 39,
    DOWN = 40,
    INSERT = 45,
    DELETE = 46,
    NUM_0 = 48,
    NUM_1,
    NUM_2,
    NUM_3,
    NUM_4,
    NUM_5,
    NUM_6,
    NUM_7,
    NUM_8,
    NUM_9 = 57,
    NUMPAD_0 = 0x60,
    NUMPAD_1,
    NUMPAD_2,
    NUMPAD_3,
    NUMPAD_4,
    NUMPAD_5,
    NUMPAD_6,
    NUMPAD_7,
    NUMPAD_8,
    NUMPAD_9 = 0x69,
    NUMPAD_MULTIPLY = 0x6a,
    NUMPAD_ADD,
    NUMPAD_SEPERATOR,
    NUMPAD_SUBTRACT,
    NUMPAD_DECIMAL,
    NUMPAD_DIVIDE = 0x6F,
    A = 65,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z = 90,
    FN1 = 112,
    FN2,
    FN3,
    FN4,
    FN5,
    FN6,
    FN7,
    FN8,
    FN9,
    FN10,
    FN11,
    FN12,
    NUM_LOCK = 144,
    SCROLL_LOCK = 145,
    SEMICOLON = 186,
    EQUAL = 187,
    COMMA = 188,
    MINUS = 189,
    PERIOD = 190,
    SLASH = 191,
    GRAVE = 192,
    LEFT_BRACKET = 219,
    BACKSLASH = 220,
    RIGHT_BRACKET = 221,
    APOSTROPHE = 222,
    MENU = 255,

    LEFT_SUPER = 1,
    RIGHT_SUPER = 1 | 256,
    LEFT_ALT = 0x12,
    RIGHT_ALT = 0x12 | 256
};

constexpr int Key(KeyCode k) { return static_cast<int>(k); }

constexpr int MOD_CTRL = 1 << 16;
constexpr int MOD_SHIFT = 1 << 17;
constexpr int MOD_ALT = 1 << 18;
constexpr int MOD_MASK = MOD_CTRL | MOD_SHIFT | MOD_ALT;
constexpr int BaseKey(int code) { return code & 0xFFFF; }
constexpr int KeyMods(int code) { return code & MOD_MASK; }
constexpr int ModifierBit(int code) {
    switch (code & 0xFF) {
        case 16 : return MOD_SHIFT;
        case 17 : return MOD_CTRL;
        case 18 : return MOD_ALT;
        default : return 0;
    }
}

inline std::string KeyName(int code) {
    if (const int mods = KeyMods(code)) {
        std::string out;
        if (mods & MOD_CTRL)  out += "Ctrl + ";
        if (mods & MOD_SHIFT) out += "Shift + ";
        if (mods & MOD_ALT)   out += "Alt + ";
        return out + KeyName(BaseKey(code));
    }
    if (code >= Key(KeyCode::A) && code <= Key(KeyCode::Z)) return std::string(1, static_cast<char>(code));
    if (code >= Key(KeyCode::NUM_0) && code <= Key(KeyCode::NUM_9)) return std::string(1, static_cast<char>(code));
    if (code >= Key(KeyCode::FN1) && code <= Key(KeyCode::FN12)) return "F" + std::to_string(code - Key(KeyCode::FN1) + 1);
    if (code >= Key(KeyCode::NUMPAD_0) && code <= Key(KeyCode::NUMPAD_9)) return "Numpad " + std::to_string(code - Key(KeyCode::NUMPAD_0));
    switch (static_cast<KeyCode>(code)) {
        case KeyCode::UNKNOWN : return "None";
        case KeyCode::BACKSPACE : return "Backspace";
        case KeyCode::TAB : return "Tab";
        case KeyCode::ENTER : return "Enter";
        case KeyCode::LEFT_SHIFT : return "Left Shift";
        case KeyCode::RIGHT_SHIFT : return "Right Shift";
        case KeyCode::LEFT_CTRL : return "Left Ctrl";
        case KeyCode::RIGHT_CTRL : return "Right Ctrl";
        case KeyCode::LEFT_ALT : return "Left Alt";
        case KeyCode::RIGHT_ALT : return "Right Alt";
        case KeyCode::LEFT_SUPER : return "Left Super";
        case KeyCode::RIGHT_SUPER : return "Right Super";
        case KeyCode::PAUSE : return "Pause";
        case KeyCode::CAPS_LOCK : return "Caps Lock";
        case KeyCode::ESCAPE : return "Escape";
        case KeyCode::SPACE : return "Space";
        case KeyCode::PAGE_UP : return "Page Up";
        case KeyCode::PAGE_DOWN : return "Page Down";
        case KeyCode::END : return "End";
        case KeyCode::HOME : return "Home";
        case KeyCode::LEFT : return "Left";
        case KeyCode::UP : return "Up";
        case KeyCode::RIGHT : return "Right";
        case KeyCode::DOWN : return "Down";
        case KeyCode::INSERT : return "Insert";
        case KeyCode::DELETE : return "Delete";
        case KeyCode::NUMPAD_MULTIPLY : return "Numpad *";
        case KeyCode::NUMPAD_ADD : return "Numpad +";
        case KeyCode::NUMPAD_SUBTRACT : return "Numpad -";
        case KeyCode::NUMPAD_DECIMAL : return "Numpad .";
        case KeyCode::NUMPAD_DIVIDE : return "Numpad /";
        case KeyCode::NUM_LOCK : return "Num Lock";
        case KeyCode::SCROLL_LOCK : return "Scroll Lock";
        case KeyCode::SEMICOLON : return ";";
        case KeyCode::EQUAL : return "=";
        case KeyCode::COMMA : return ",";
        case KeyCode::MINUS : return "-";
        case KeyCode::PERIOD : return ".";
        case KeyCode::SLASH : return "/";
        case KeyCode::GRAVE : return "`";
        case KeyCode::LEFT_BRACKET : return "[";
        case KeyCode::BACKSLASH : return "\\";
        case KeyCode::RIGHT_BRACKET : return "]";
        case KeyCode::APOSTROPHE : return "'";
        case KeyCode::MENU : return "Menu";
        default : return "Key " + std::to_string(code);
    }
}

}
