#ifndef TEXT_ENCODINGS_H
#define TEXT_ENCODINGS_H

#include <string>
#include <unordered_map>
#include <vector>

enum class TextEncoding {
    ASCII,
    EN_G1,  // English Gen 1
    EN_G2,  // English Gen 2
    JP_G1,  // Japanese Gen 1
    JP_G2,  // Japanese Gen 2
    EN_G3,  // English Gen 3
    JP_G3   // Japanese Gen 3
};

inline TextEncoding parseEncodingArg(const std::string& arg) {
    if (arg == "E1") return TextEncoding::EN_G1;
    if (arg == "E2") return TextEncoding::EN_G2;
    if (arg == "E3") return TextEncoding::EN_G3;
    if (arg == "J1") return TextEncoding::JP_G1;
    if (arg == "J2") return TextEncoding::JP_G2;
    if (arg == "J3") return TextEncoding::JP_G3;
    return TextEncoding::ASCII;
}

inline std::string getEncodingName(TextEncoding enc) {
    switch (enc) {
        case TextEncoding::EN_G1: return "English Gen 1";
        case TextEncoding::EN_G2: return "English Gen 2";
        case TextEncoding::EN_G3: return "English Gen 3";
        case TextEncoding::JP_G1: return "Japanese Gen 1";
        case TextEncoding::JP_G2: return "Japanese Gen 2";
        case TextEncoding::JP_G3: return "Japanese Gen 3";
        default: return "ASCII";
    }
}

// ============================================================================
// Common encoding tables
// ============================================================================

namespace EncodingTables {

// English Generation 1
static const std::unordered_map<unsigned char, std::string> EN_G1_TABLE = {
    // Space
    {0x7F, " "},
    
    // Row 7- punctuation
    {0x70, "'"}, {0x71, "'"}, {0x72, "\""}, {0x73, "\""}, 
    {0x74, "・"}, {0x75, "…"}, {0x76, "ぁ"}, {0x77, "ぇ"}, 
    {0x78, "ぉ"}, {0x79, "╔"}, {0x7A, "═"}, {0x7B, "╗"},
    {0x7C, "║"}, {0x7D, "╚"}, {0x7E, "╝"},
    
    // Row 6-
    {0x6D, ":"}, {0x6E, "ぃ"}, {0x6F, "ぅ"},
    
    // Uppercase A-Z (0x80-0x99)
    {0x80, "A"}, {0x81, "B"}, {0x82, "C"}, {0x83, "D"}, {0x84, "E"},
    {0x85, "F"}, {0x86, "G"}, {0x87, "H"}, {0x88, "I"}, {0x89, "J"},
    {0x8A, "K"}, {0x8B, "L"}, {0x8C, "M"}, {0x8D, "N"}, {0x8E, "O"},
    {0x8F, "P"}, {0x90, "Q"}, {0x91, "R"}, {0x92, "S"}, {0x93, "T"},
    {0x94, "U"}, {0x95, "V"}, {0x96, "W"}, {0x97, "X"}, {0x98, "Y"},
    {0x99, "Z"},
    
    // Punctuation after Z
    {0x9A, "("}, {0x9B, ")"}, {0x9C, ":"}, {0x9D, ";"}, {0x9E, "["}, {0x9F, "]"},
    
    // Lowercase a-z (0xA0-0xB9)
    {0xA0, "a"}, {0xA1, "b"}, {0xA2, "c"}, {0xA3, "d"}, {0xA4, "e"},
    {0xA5, "f"}, {0xA6, "g"}, {0xA7, "h"}, {0xA8, "i"}, {0xA9, "j"},
    {0xAA, "k"}, {0xAB, "l"}, {0xAC, "m"}, {0xAD, "n"}, {0xAE, "o"},
    {0xAF, "p"}, {0xB0, "q"}, {0xB1, "r"}, {0xB2, "s"}, {0xB3, "t"},
    {0xB4, "u"}, {0xB5, "v"}, {0xB6, "w"}, {0xB7, "x"}, {0xB8, "y"},
    {0xB9, "z"},
    
    // Special characters after lowercase
    {0xBA, "é"},
    {0xBB, "'d"}, {0xBC, "'l"}, {0xBD, "'s"}, {0xBE, "'t"}, {0xBF, "'v"},
    
    // Row E-
    {0xE0, "'"}, {0xE3, "-"}, {0xE4, "'r"}, {0xE5, "'m"},
    {0xE6, "?"}, {0xE7, "!"}, {0xE8, "."},
    {0xE9, "ァ"}, {0xEA, "ゥ"}, {0xEB, "ェ"},
    {0xEC, "▷"}, {0xED, "▶"}, {0xEE, "▼"}, {0xEF, "♂"},
    
    // Row F-
    {0xF0, "$"}, {0xF1, "×"}, {0xF2, "."}, {0xF3, "/"}, {0xF4, ","},
    {0xF5, "♀"},
    {0xF6, "0"}, {0xF7, "1"}, {0xF8, "2"}, {0xF9, "3"}, {0xFA, "4"},
    {0xFB, "5"}, {0xFC, "6"}, {0xFD, "7"}, {0xFE, "8"}, {0xFF, "9"},
};

// English Generation 2
static const std::unordered_map<unsigned char, std::string> EN_G2_TABLE = {
    // Space
    {0x7F, " "},
    
    // Uppercase A-Z (0x80-0x99)
    {0x80, "A"}, {0x81, "B"}, {0x82, "C"}, {0x83, "D"}, {0x84, "E"},
    {0x85, "F"}, {0x86, "G"}, {0x87, "H"}, {0x88, "I"}, {0x89, "J"},
    {0x8A, "K"}, {0x8B, "L"}, {0x8C, "M"}, {0x8D, "N"}, {0x8E, "O"},
    {0x8F, "P"}, {0x90, "Q"}, {0x91, "R"}, {0x92, "S"}, {0x93, "T"},
    {0x94, "U"}, {0x95, "V"}, {0x96, "W"}, {0x97, "X"}, {0x98, "Y"},
    {0x99, "Z"},
    
    // Punctuation after Z
    {0x9A, "("}, {0x9B, ")"}, {0x9C, ":"}, {0x9D, ";"}, {0x9E, "["}, {0x9F, "]"},
    
    // Lowercase a-z (0xA0-0xB9)
    {0xA0, "a"}, {0xA1, "b"}, {0xA2, "c"}, {0xA3, "d"}, {0xA4, "e"},
    {0xA5, "f"}, {0xA6, "g"}, {0xA7, "h"}, {0xA8, "i"}, {0xA9, "j"},
    {0xAA, "k"}, {0xAB, "l"}, {0xAC, "m"}, {0xAD, "n"}, {0xAE, "o"},
    {0xAF, "p"}, {0xB0, "q"}, {0xB1, "r"}, {0xB2, "s"}, {0xB3, "t"},
    {0xB4, "u"}, {0xB5, "v"}, {0xB6, "w"}, {0xB7, "x"}, {0xB8, "y"},
    {0xB9, "z"},
    
    // Umlauts
    {0xC0, "Ä"}, {0xC1, "Ö"}, {0xC2, "Ü"}, {0xC3, "ä"}, {0xC4, "ö"}, {0xC5, "ü"},
    
    // Contractions
    {0xD0, "'d"}, {0xD1, "'l"}, {0xD2, "'m"}, {0xD3, "'r"},
    {0xD4, "'s"}, {0xD5, "'t"}, {0xD6, "'v"},
    
    // Row E-
    {0xE0, "'"}, {0xE6, "?"}, {0xE7, "!"}, {0xE8, "."}, {0xE9, "&"}, {0xEA, "é"},
    
    // Row F-
    {0xF0, "$"}, {0xF1, "×"}, {0xF2, "."}, {0xF3, "/"}, {0xF4, ","},
    {0xF5, "♀"},
    {0xF6, "0"}, {0xF7, "1"}, {0xF8, "2"}, {0xF9, "3"}, {0xFA, "4"},
    {0xFB, "5"}, {0xFC, "6"}, {0xFD, "7"}, {0xFE, "8"}, {0xFF, "9"},
};

// English Generation 3
static const std::unordered_map<unsigned char, std::string> EN_G3_TABLE = {
    // Space
    {0x00, " "},

    // Misc punctuation / symbols
    {0x2D, "&"},

    // Percent & parentheses
    {0x5B, "%"}, {0x5C, "("}, {0x5D, ")"},

    // Arrows
    {0x79, "↑"}, {0x7A, "↓"}, {0x7B, "←"}, {0x7C, "→"},

    // Digits & punctuation (A- row)
    {0xA1, "0"}, {0xA2, "1"}, {0xA3, "2"}, {0xA4, "3"}, {0xA5, "4"},
    {0xA6, "5"}, {0xA7, "6"}, {0xA8, "7"}, {0xA9, "8"}, {0xAA, "9"},
    {0xAB, "!"}, {0xAC, "?"}, {0xAD, "."}, {0xAE, "-"}, {0xAF, "･"},

    // Ellipsis byte – normalized to "."
    {0xB0, "."},

    // Quotes / symbols (B- row)
    {0xB1, """}, {0xB2, """}, {0xB3, "'"}, {0xB4, "'"},
    {0xB5, "♂"}, {0xB6, "♀"}, {0xB7, "$"}, {0xB8, ","},
    {0xB9, "×"}, {0xBA, "/"},

    // Uppercase A–Z
    {0xBB, "A"}, {0xBC, "B"}, {0xBD, "C"}, {0xBE, "D"}, {0xBF, "E"},
    {0xC0, "F"}, {0xC1, "G"}, {0xC2, "H"}, {0xC3, "I"}, {0xC4, "J"},
    {0xC5, "K"}, {0xC6, "L"}, {0xC7, "M"}, {0xC8, "N"}, {0xC9, "O"},
    {0xCA, "P"}, {0xCB, "Q"}, {0xCC, "R"}, {0xCD, "S"}, {0xCE, "T"},
    {0xCF, "U"}, {0xD0, "V"}, {0xD1, "W"}, {0xD2, "X"}, {0xD3, "Y"},
    {0xD4, "Z"},

    // Lowercase a–z
    {0xD5, "a"}, {0xD6, "b"}, {0xD7, "c"}, {0xD8, "d"}, {0xD9, "e"},
    {0xDA, "f"}, {0xDB, "g"}, {0xDC, "h"}, {0xDD, "i"}, {0xDE, "j"},
    {0xDF, "k"}, {0xE0, "l"}, {0xE1, "m"}, {0xE2, "n"}, {0xE3, "o"},
    {0xE4, "p"}, {0xE5, "q"}, {0xE6, "r"}, {0xE7, "s"}, {0xE8, "t"},
    {0xE9, "u"}, {0xEA, "v"}, {0xEB, "w"}, {0xEC, "x"}, {0xED, "y"},
    {0xEE, "z"},

    // Pointer / arrow
    {0xEF, "►"},

    // Umlauts & colon (F- row)
    {0xF0, ":"}, {0xF1, "Ä"}, {0xF2, "Ö"}, {0xF3, "Ü"},
    {0xF4, "ä"}, {0xF5, "ö"}, {0xF6, "ü"},
};

// Japanese Generation 1
static const std::unordered_map<unsigned char, std::string> JP_G1_TABLE = {
    // Row 0- (dakuten variants)
    {0x01, "イ゙"}, {0x02, "ヴ"}, {0x03, "エ゙"}, {0x04, "オ゙"},
    {0x05, "ガ"}, {0x06, "ギ"}, {0x07, "グ"}, {0x08, "ゲ"}, {0x09, "ゴ"},
    {0x0A, "ザ"}, {0x0B, "ジ"}, {0x0C, "ズ"}, {0x0D, "ゼ"}, {0x0E, "ゾ"}, {0x0F, "ダ"},
    
    // Row 1-
    {0x10, "ヂ"}, {0x11, "ヅ"}, {0x12, "デ"}, {0x13, "ド"},
    {0x14, "ナ゙"}, {0x15, "ニ゙"}, {0x16, "ヌ゙"}, {0x17, "ネ゙"}, {0x18, "ノ゙"},
    {0x19, "バ"}, {0x1A, "ビ"}, {0x1B, "ブ"}, {0x1C, "ボ"},
    {0x1D, "マ゙"}, {0x1E, "ミ゙"}, {0x1F, "ム゙"},
    
    // Row 2- (hiragana dakuten)
    {0x20, "ィ゙"}, {0x21, "あ゙"}, {0x22, "い゙"}, {0x23, "ゔ"}, {0x24, "え゙"}, {0x25, "お゙"},
    {0x26, "が"}, {0x27, "ぎ"}, {0x28, "ぐ"}, {0x29, "げ"}, {0x2A, "ご"},
    {0x2B, "ざ"}, {0x2C, "じ"}, {0x2D, "ず"}, {0x2E, "ぜ"}, {0x2F, "ぞ"},
    
    // Row 3-
    {0x30, "だ"}, {0x31, "ぢ"}, {0x32, "づ"}, {0x33, "で"}, {0x34, "ど"},
    {0x35, "な゙"}, {0x36, "に゙"}, {0x37, "ぬ゙"}, {0x38, "ね゙"}, {0x39, "の゙"},
    {0x3A, "ば"}, {0x3B, "び"}, {0x3C, "ぶ"}, {0x3D, "べ"}, {0x3E, "ぼ"}, {0x3F, "ま゙"},
    
    // Row 4- (handakuten)
    {0x40, "パ"}, {0x41, "ピ"}, {0x42, "プ"}, {0x43, "ポ"},
    {0x44, "ぱ"}, {0x45, "ぴ"}, {0x46, "ぷ"}, {0x47, "ぺ"}, {0x48, "ぽ"},
    {0x49, "ま゚"}, {0x4B, "も゚"},
    
    // Row 6- (some letters and symbols)
    {0x60, "A"}, {0x61, "B"}, {0x62, "C"}, {0x63, "D"}, {0x64, "E"},
    {0x65, "F"}, {0x66, "G"}, {0x67, "H"}, {0x68, "I"}, {0x69, "V"},
    {0x6A, "S"}, {0x6B, "L"}, {0x6C, "M"}, {0x6D, ":"}, {0x6E, "ぃ"}, {0x6F, "ぅ"},
    
    // Row 7- (brackets and symbols)
    {0x70, "「"}, {0x71, "」"}, {0x72, "『"}, {0x73, "』"},
    {0x74, "・"}, {0x75, "…"}, {0x76, "ぁ"}, {0x77, "ぇ"}, {0x78, "ぉ"},
    
    // Row 8- (katakana)
    {0x80, "ア"}, {0x81, "イ"}, {0x82, "ウ"}, {0x83, "エ"}, {0x84, "オ"},
    {0x85, "カ"}, {0x86, "キ"}, {0x87, "ク"}, {0x88, "ケ"}, {0x89, "コ"},
    {0x8A, "サ"}, {0x8B, "シ"}, {0x8C, "ス"}, {0x8D, "セ"}, {0x8E, "ソ"}, {0x8F, "タ"},
    
    // Row 9-
    {0x90, "チ"}, {0x91, "ツ"}, {0x92, "テ"}, {0x93, "ト"},
    {0x94, "ナ"}, {0x95, "ニ"}, {0x96, "ヌ"}, {0x97, "ネ"}, {0x98, "ノ"},
    {0x99, "ハ"}, {0x9A, "ヒ"}, {0x9B, "フ"}, {0x9C, "ホ"},
    {0x9D, "マ"}, {0x9E, "ミ"}, {0x9F, "ム"},
    
    // Row A-
    {0xA0, "メ"}, {0xA1, "モ"}, {0xA2, "ヤ"}, {0xA3, "ユ"}, {0xA4, "ヨ"},
    {0xA5, "ラ"}, {0xA6, "ル"}, {0xA7, "レ"}, {0xA8, "ロ"},
    {0xA9, "ワ"}, {0xAA, "ヲ"}, {0xAB, "ン"}, {0xAC, "ッ"},
    {0xAD, "ャ"}, {0xAE, "ュ"}, {0xAF, "ョ"},
    
    // Row B- (hiragana)
    {0xB0, "ィ"}, {0xB1, "あ"}, {0xB2, "い"}, {0xB3, "う"}, {0xB4, "え"}, {0xB5, "お"},
    {0xB6, "か"}, {0xB7, "き"}, {0xB8, "く"}, {0xB9, "け"}, {0xBA, "こ"},
    {0xBB, "さ"}, {0xBC, "し"}, {0xBD, "す"}, {0xBE, "せ"}, {0xBF, "そ"},
    
    // Row C-
    {0xC0, "た"}, {0xC1, "ち"}, {0xC2, "つ"}, {0xC3, "て"}, {0xC4, "と"},
    {0xC5, "な"}, {0xC6, "に"}, {0xC7, "ぬ"}, {0xC8, "ね"}, {0xC9, "の"},
    {0xCA, "は"}, {0xCB, "ひ"}, {0xCC, "ふ"}, {0xCD, "へ"}, {0xCE, "ほ"}, {0xCF, "ま"},
    
    // Row D-
    {0xD0, "み"}, {0xD1, "む"}, {0xD2, "め"}, {0xD3, "も"},
    {0xD4, "や"}, {0xD5, "ゆ"}, {0xD6, "よ"},
    {0xD7, "ら"}, {0xD8, "リ"}, {0xD9, "る"}, {0xDA, "れ"}, {0xDB, "ろ"},
    {0xDC, "わ"}, {0xDD, "を"}, {0xDE, "ん"}, {0xDF, "っ"},
    
    // Row E-
    {0xE0, "ゃ"}, {0xE1, "ゅ"}, {0xE2, "ょ"}, {0xE3, "ー"},
    {0xE4, "゜"}, {0xE5, "゛"}, {0xE6, "?"}, {0xE7, "!"}, {0xE8, "。"},
    {0xE9, "ァ"}, {0xEA, "ゥ"}, {0xEB, "ェ"},
    {0xEC, "▷"}, {0xED, "▶"}, {0xEE, "▼"}, {0xEF, "♂"},
    
    // Row F-
    {0xF0, "円"}, {0xF1, "×"}, {0xF2, "."}, {0xF3, "/"}, {0xF4, "ォ"}, {0xF5, "♀"},
    {0xF6, "0"}, {0xF7, "1"}, {0xF8, "2"}, {0xF9, "3"}, {0xFA, "4"},
    {0xFB, "5"}, {0xFC, "6"}, {0xFD, "7"}, {0xFE, "8"}, {0xFF, "9"},
};

// Japanese Generation 2
static const std::unordered_map<unsigned char, std::string> JP_G2_TABLE = {
    // Row 0-
    {0x00, "?"}, {0x01, "イ゙"}, {0x02, "ヴ"}, {0x03, "エ゙"}, {0x04, "オ゙"},
    {0x05, "ガ"}, {0x06, "ギ"}, {0x07, "グ"}, {0x08, "ゲ"}, {0x09, "ゴ"},
    {0x0A, "ザ"}, {0x0B, "ジ"}, {0x0C, "ズ"}, {0x0D, "ゼ"}, {0x0E, "ゾ"}, {0x0F, "ダ"},
    
    // Row 1-
    {0x10, "ヂ"}, {0x11, "ヅ"}, {0x12, "デ"}, {0x13, "ド"},
    {0x17, "ネ゙"}, {0x18, "ノ゙"},
    {0x19, "バ"}, {0x1A, "ビ"}, {0x1B, "ブ"}, {0x1C, "ボ"},
    
    // Row 2-
    {0x20, "ィ゙"}, {0x21, "あ゙"},
    {0x26, "が"}, {0x27, "ぎ"}, {0x28, "ぐ"}, {0x29, "げ"}, {0x2A, "ご"},
    {0x2B, "ざ"}, {0x2C, "じ"}, {0x2D, "ず"}, {0x2E, "ぜ"}, {0x2F, "ぞ"},
    
    // Row 3-
    {0x30, "だ"}, {0x31, "ぢ"}, {0x32, "づ"}, {0x33, "で"}, {0x34, "ど"},
    {0x3A, "ば"}, {0x3B, "び"}, {0x3C, "ぶ"}, {0x3D, "べ"}, {0x3E, "ぼ"},
    
    // Row 4-
    {0x40, "パ"}, {0x41, "ピ"}, {0x42, "プ"}, {0x43, "ポ"},
    {0x44, "ぱ"}, {0x45, "ぴ"}, {0x46, "ぷ"}, {0x47, "ぺ"}, {0x48, "ぽ"},
    {0x4D, "も゚"},
    
    // Row 6-
    {0x61, "▲"}, {0x63, "D"}, {0x64, "E"},
    {0x65, "F"}, {0x66, "G"}, {0x67, "H"}, {0x68, "I"}, {0x69, "V"},
    {0x6A, "S"}, {0x6B, "L"}, {0x6C, "M"}, {0x6D, ":"}, {0x6E, "ぃ"}, {0x6F, "ぅ"},
    
    // Row 7-
    {0x70, "「"}, {0x71, "」"}, {0x72, "『"}, {0x73, "』"},
    {0x74, "・"}, {0x75, "…"}, {0x76, "ぁ"}, {0x77, "ぇ"}, {0x78, "ぉ"},
    
    // Row 8- (katakana)
    {0x80, "ア"}, {0x81, "イ"}, {0x82, "ウ"}, {0x83, "エ"}, {0x84, "オ"},
    {0x85, "カ"}, {0x86, "キ"}, {0x87, "ク"}, {0x88, "ケ"}, {0x89, "コ"},
    {0x8A, "サ"}, {0x8B, "シ"}, {0x8C, "ス"}, {0x8D, "セ"}, {0x8E, "ソ"}, {0x8F, "タ"},
    
    // Row 9-
    {0x90, "チ"}, {0x91, "ツ"}, {0x92, "テ"}, {0x93, "ト"},
    {0x94, "ナ"}, {0x95, "ニ"}, {0x96, "ヌ"}, {0x97, "ネ"}, {0x98, "ノ"},
    {0x99, "ハ"}, {0x9A, "ヒ"}, {0x9B, "フ"}, {0x9C, "ホ"},
    {0x9D, "マ"}, {0x9E, "ミ"}, {0x9F, "ム"},
    
    // Row A-
    {0xA0, "メ"}, {0xA1, "モ"}, {0xA2, "ヤ"}, {0xA3, "ユ"}, {0xA4, "ヨ"},
    {0xA5, "ラ"}, {0xA6, "ル"}, {0xA7, "レ"}, {0xA8, "ロ"},
    {0xA9, "ワ"}, {0xAA, "ヲ"}, {0xAB, "ン"}, {0xAC, "ッ"},
    {0xAD, "ャ"}, {0xAE, "ュ"}, {0xAF, "ョ"},
    
    // Row B- (hiragana)
    {0xB0, "ィ"}, {0xB1, "あ"}, {0xB2, "い"}, {0xB3, "う"}, {0xB4, "え"}, {0xB5, "お"},
    {0xB6, "か"}, {0xB7, "き"}, {0xB8, "く"}, {0xB9, "け"}, {0xBA, "こ"},
    {0xBB, "さ"}, {0xBC, "し"}, {0xBD, "す"}, {0xBE, "せ"}, {0xBF, "そ"},
    
    // Row C-
    {0xC0, "た"}, {0xC1, "ち"}, {0xC2, "つ"}, {0xC3, "て"}, {0xC4, "と"},
    {0xC5, "な"}, {0xC6, "に"}, {0xC7, "ぬ"}, {0xC8, "ね"}, {0xC9, "の"},
    {0xCA, "は"}, {0xCB, "ひ"}, {0xCC, "ふ"}, {0xCD, "へ"}, {0xCE, "ほ"}, {0xCF, "ま"},
    
    // Row D-
    {0xD0, "み"}, {0xD1, "む"}, {0xD2, "め"}, {0xD3, "も"},
    {0xD4, "や"}, {0xD5, "ゆ"}, {0xD6, "よ"},
    {0xD7, "ら"}, {0xD8, "リ"}, {0xD9, "る"}, {0xDA, "れ"}, {0xDB, "ろ"},
    {0xDC, "わ"}, {0xDD, "を"}, {0xDE, "ん"}, {0xDF, "っ"},
    
    // Row E-
    {0xE0, "ゃ"}, {0xE1, "ゅ"}, {0xE2, "ょ"}, {0xE3, "ー"},
    {0xE4, "゜"}, {0xE5, "゛"}, {0xE6, "?"}, {0xE7, "!"}, {0xE8, "。"},
    {0xE9, "ァ"}, {0xEA, "ゥ"}, {0xEB, "ェ"},
    {0xEC, "▷"}, {0xED, "▶"}, {0xEE, "▼"}, {0xEF, "♂"},
    
    // Row F-
    {0xF0, "円"}, {0xF1, "×"}, {0xF2, "."}, {0xF3, "/"}, {0xF4, "ォ"}, {0xF5, "♀"},
    {0xF6, "0"}, {0xF7, "1"}, {0xF8, "2"}, {0xF9, "3"}, {0xFA, "4"},
    {0xFB, "5"}, {0xFC, "6"}, {0xFD, "7"}, {0xFE, "8"}, {0xFF, "9"},
};

// Japanese Generation 3
static const std::unordered_map<unsigned char, std::string> JP_G3_TABLE = {
    // 0x0- row (hiragana)
    {0x00, "あ"}, {0x01, "い"}, {0x02, "う"}, {0x03, "え"}, {0x04, "お"},
    {0x05, "か"}, {0x06, "き"}, {0x07, "く"}, {0x08, "け"}, {0x09, "こ"},
    {0x0A, "さ"}, {0x0B, "し"}, {0x0C, "す"}, {0x0D, "せ"}, {0x0E, "そ"},

    // 0x1- row
    {0x10, "た"}, {0x11, "ち"}, {0x12, "つ"}, {0x13, "て"}, {0x14, "と"},
    {0x15, "な"}, {0x16, "に"}, {0x17, "ぬ"}, {0x18, "ね"}, {0x19, "の"},
    {0x1A, "は"}, {0x1B, "ひ"}, {0x1C, "ふ"}, {0x1D, "へ"}, {0x1E, "ほ"},
    {0x1F, "ま"},

    // 0x2- row
    {0x20, "み"}, {0x21, "む"}, {0x22, "め"}, {0x23, "も"}, {0x24, "や"},
    {0x25, "ゆ"}, {0x26, "よ"}, {0x27, "ら"}, {0x28, "り"}, {0x29, "る"},
    {0x2A, "れ"}, {0x2B, "ろ"}, {0x2C, "わ"}, {0x2D, "を"}, {0x2E, "ん"},
    {0x2F, "ぁ"},

    // 0x3- row
    {0x30, "ぃ"}, {0x31, "ぅ"}, {0x32, "ぇ"}, {0x33, "ぉ"}, {0x34, "ゃ"},
    {0x35, "ゅ"}, {0x36, "ょ"}, {0x37, "が"}, {0x38, "ぎ"}, {0x39, "ぐ"},
    {0x3A, "げ"}, {0x3B, "ご"}, {0x3C, "ざ"}, {0x3D, "じ"}, {0x3E, "ず"},
    {0x3F, "ぜ"},

    // 0x4- row
    {0x40, "ぞ"}, {0x41, "だ"}, {0x42, "ぢ"}, {0x43, "づ"}, {0x44, "で"},
    {0x45, "ど"}, {0x46, "ば"}, {0x47, "び"}, {0x48, "ぶ"}, {0x49, "べ"},
    {0x4A, "ぼ"}, {0x4B, "ぱ"}, {0x4C, "ぴ"}, {0x4D, "ぷ"}, {0x4E, "ぺ"},
    {0x4F, "ぽ"},

    // 0x5- row (katakana / small tsu)
    {0x50, "っ"}, {0x51, "ア"}, {0x52, "イ"}, {0x53, "ウ"}, {0x54, "エ"},
    {0x55, "オ"}, {0x56, "カ"}, {0x57, "キ"}, {0x58, "ク"}, {0x59, "ケ"},
    {0x5A, "コ"}, {0x5B, "サ"}, {0x5C, "シ"}, {0x5D, "ス"}, {0x5E, "セ"},
    {0x5F, "ソ"},

    // 0x6- row
    {0x60, "タ"}, {0x61, "チ"}, {0x62, "ツ"}, {0x63, "テ"}, {0x64, "ト"},
    {0x65, "ナ"}, {0x66, "ニ"}, {0x67, "ヌ"}, {0x68, "ネ"}, {0x69, "ノ"},
    {0x6A, "ハ"}, {0x6B, "ヒ"}, {0x6C, "フ"}, {0x6D, "ヘ"}, {0x6E, "ホ"},
    {0x6F, "マ"},

    // 0x7- row
    {0x70, "ミ"}, {0x71, "ム"}, {0x72, "メ"}, {0x73, "モ"}, {0x74, "ヤ"},
    {0x75, "ユ"}, {0x76, "ヨ"}, {0x77, "ラ"}, {0x78, "リ"}, {0x79, "ル"},
    {0x7A, "レ"}, {0x7B, "ロ"}, {0x7C, "ワ"}, {0x7D, "ヲ"}, {0x7E, "ン"},
    {0x7F, "ァ"},

    // 0x8- row
    {0x80, "ィ"}, {0x81, "ゥ"}, {0x82, "ェ"}, {0x83, "ォ"}, {0x84, "ャ"},
    {0x85, "ュ"}, {0x86, "ョ"}, {0x87, "ガ"}, {0x88, "ギ"}, {0x89, "グ"},
    {0x8A, "ゲ"}, {0x8B, "ゴ"}, {0x8C, "ザ"}, {0x8D, "ジ"}, {0x8E, "ズ"},
    {0x8F, "ゼ"},

    // 0x9- row
    {0x90, "ゾ"}, {0x91, "ダ"}, {0x92, "ヂ"}, {0x93, "ヅ"}, {0x94, "デ"},
    {0x95, "ド"}, {0x96, "バ"}, {0x97, "ビ"}, {0x98, "ブ"}, {0x99, "ベ"},
    {0x9A, "ボ"}, {0x9B, "パ"}, {0x9C, "ピ"}, {0x9D, "プ"}, {0x9E, "ペ"},
    {0x9F, "ポ"},

    // 0xA- row (numbers, punctuation)
    {0xA0, "ッ"}, {0xA1, "０"}, {0xA2, "１"}, {0xA3, "２"}, {0xA4, "３"},
    {0xA5, "４"}, {0xA6, "５"}, {0xA7, "６"}, {0xA8, "７"}, {0xA9, "８"},
    {0xAA, "９"}, {0xAB, "！"}, {0xAC, "？"}, {0xAD, "。"}, {0xAE, "ー"},
    {0xAF, "・"},

    // 0xB- row – 0xB0 is ellipsis; normalize to "."
    {0xB0, "."}, {0xB1, "『"}, {0xB2, "』"}, {0xB3, "「"}, {0xB4, "」"},
    {0xB5, "♂"}, {0xB6, "♀"}, {0xB7, "円"}, {0xB8, "．"}, {0xB9, "×"},
    {0xBA, "／"}, {0xBB, "Ａ"}, {0xBC, "Ｂ"}, {0xBD, "Ｃ"}, {0xBE, "Ｄ"},
    {0xBF, "Ｅ"},

    // 0xC- row (fullwidth Latin uppercase)
    {0xC0, "Ｆ"}, {0xC1, "Ｇ"}, {0xC2, "Ｈ"}, {0xC3, "Ｉ"}, {0xC4, "Ｊ"},
    {0xC5, "Ｋ"}, {0xC6, "Ｌ"}, {0xC7, "Ｍ"}, {0xC8, "Ｎ"}, {0xC9, "Ｏ"},
    {0xCA, "Ｐ"}, {0xCB, "Ｑ"}, {0xCC, "Ｒ"}, {0xCD, "Ｓ"}, {0xCE, "Ｔ"},
    {0xCF, "Ｕ"},

    // 0xD- row (fullwidth Latin lowercase)
    {0xD0, "Ｖ"}, {0xD1, "Ｗ"}, {0xD2, "Ｘ"}, {0xD3, "Ｙ"}, {0xD4, "Ｚ"},
    {0xD5, "ａ"}, {0xD6, "ｂ"}, {0xD7, "ｃ"}, {0xD8, "ｄ"}, {0xD9, "ｅ"},
    {0xDA, "ｆ"}, {0xDB, "ｇ"}, {0xDC, "ｈ"}, {0xDD, "ｉ"}, {0xDE, "ｊ"},
    {0xDF, "ｋ"},

    // 0xE- row
    {0xE0, "ｌ"}, {0xE1, "ｍ"}, {0xE2, "ｎ"}, {0xE3, "ｏ"}, {0xE4, "ｐ"},
    {0xE5, "ｑ"}, {0xE6, "ｒ"}, {0xE7, "ｓ"}, {0xE8, "ｔ"}, {0xE9, "ｕ"},
    {0xEA, "ｖ"}, {0xEB, "ｗ"}, {0xEC, "ｘ"}, {0xED, "ｙ"}, {0xEE, "ｚ"},
    {0xEF, "►"},

    // 0xF- row (punctuation + umlauts)
    {0xF0, "："}, {0xF1, "Ä"}, {0xF2, "Ö"}, {0xF3, "Ü"},
    {0xF4, "ä"}, {0xF5, "ö"}, {0xF6, "ü"},
};

// Get the appropriate table for an encoding
inline const std::unordered_map<unsigned char, std::string>& getTable(TextEncoding encoding) {
    switch (encoding) {
        case TextEncoding::EN_G1: return EN_G1_TABLE;
        case TextEncoding::EN_G2: return EN_G2_TABLE;
        case TextEncoding::EN_G3: return EN_G3_TABLE;
        case TextEncoding::JP_G1: return JP_G1_TABLE;
        case TextEncoding::JP_G2: return JP_G2_TABLE;
        case TextEncoding::JP_G3: return JP_G3_TABLE;
        default: {
            static const std::unordered_map<unsigned char, std::string> empty;
            return empty;
        }
    }
}

} // namespace EncodingTables

// ============================================================================
// Decoding functions
// ============================================================================

// Decode a single byte
inline std::string decodeByte(unsigned char byte, TextEncoding encoding) {
    if (encoding == TextEncoding::ASCII) {
        // Standard ASCII
        if (byte >= 32 && byte < 127) {
            return std::string(1, (char)byte);
        }
        return "";
    }
    
    const auto& table = EncodingTables::getTable(encoding);
    auto it = table.find(byte);
    if (it != table.end()) {
        return it->second;
    }
    return "";  // Control character
}

// Individual decoding functions
inline std::string decodeByteEN_G1(unsigned char byte) {
    return decodeByte(byte, TextEncoding::EN_G1);
}

inline std::string decodeByteEN_G2(unsigned char byte) {
    return decodeByte(byte, TextEncoding::EN_G2);
}

inline std::string decodeByteEN_G3(unsigned char byte) {
    return decodeByte(byte, TextEncoding::EN_G3);
}

inline std::string decodeByteJP_G1(unsigned char byte) {
    return decodeByte(byte, TextEncoding::JP_G1);
}

inline std::string decodeByteJP_G2(unsigned char byte) {
    return decodeByte(byte, TextEncoding::JP_G2);
}

inline std::string decodeByteJP_G3(unsigned char byte) {
    return decodeByte(byte, TextEncoding::JP_G3);
}

// Decode text (vector of bytes)
inline std::string decodeText(const std::vector<unsigned char>& bytes, TextEncoding encoding, 
                              unsigned char terminator = 0x50) {
    std::string result;
    for (unsigned char byte : bytes) {
        if (byte == terminator) break;
        result += decodeByte(byte, encoding);
    }
    return result;
}

// ============================================================================
// Encoding functions
// ============================================================================

// Build reverse mapping for encoding (cached)
inline std::unordered_map<std::string, unsigned char> buildReverseTable(TextEncoding encoding) {
    std::unordered_map<std::string, unsigned char> reverseTable;
    const auto& table = EncodingTables::getTable(encoding);
    
    for (const auto& pair : table) {
        // Only add if not already present (prefer first occurrence)
        if (reverseTable.find(pair.second) == reverseTable.end()) {
            reverseTable[pair.second] = pair.first;
        }
    }
    
    return reverseTable;
}

// Encode a single character/string to a byte
inline unsigned char encodeByte(const std::string& ch, TextEncoding encoding) {
    if (encoding == TextEncoding::ASCII) {
        if (ch.size() == 1) {
            return static_cast<unsigned char>(ch[0]);
        }
        return 0;
    }
    
    // Get cached reverse table
    static std::unordered_map<TextEncoding, std::unordered_map<std::string, unsigned char>> reverseTables;
    
    if (reverseTables.find(encoding) == reverseTables.end()) {
        reverseTables[encoding] = buildReverseTable(encoding);
    }
    
    const auto& reverseTable = reverseTables[encoding];
    auto it = reverseTable.find(ch);
    if (it != reverseTable.end()) {
        return it->second;
    }
    
    // Fall back to ASCII for single characters (Gen1/Gen2 only)
    if (ch.size() == 1 && (encoding == TextEncoding::EN_G1 || encoding == TextEncoding::EN_G2)) {
        unsigned char c = static_cast<unsigned char>(ch[0]);
        if (c >= 32 && c < 127) {
            return c;
        }
    }
    
    return 0;
}

// Individual encoding functions
inline unsigned char encodeByteEN_G1(const std::string& ch) {
    return encodeByte(ch, TextEncoding::EN_G1);
}

inline unsigned char encodeByteEN_G2(const std::string& ch) {
    return encodeByte(ch, TextEncoding::EN_G2);
}

inline unsigned char encodeByteEN_G3(const std::string& ch) {
    return encodeByte(ch, TextEncoding::EN_G3);
}

inline unsigned char encodeByteJP_G1(const std::string& ch) {
    return encodeByte(ch, TextEncoding::JP_G1);
}

inline unsigned char encodeByteJP_G2(const std::string& ch) {
    return encodeByte(ch, TextEncoding::JP_G2);
}

inline unsigned char encodeByteJP_G3(const std::string& ch) {
    return encodeByte(ch, TextEncoding::JP_G3);
}

// Encode text string to bytes
inline std::vector<unsigned char> encodeText(const std::string& text, TextEncoding encoding, 
                                              size_t maxLength = 0, unsigned char terminator = 0x50) {
    std::vector<unsigned char> result;
    
    // For multi-byte UTF-8, we need to properly iterate through the string
    size_t i = 0;
    while (i < text.length() && (maxLength == 0 || result.size() < maxLength)) {
        // Determine the length of the current UTF-8 character
        unsigned char firstByte = static_cast<unsigned char>(text[i]);
        size_t charLen = 1;
        
        if ((firstByte & 0x80) == 0) {
            charLen = 1;
        } else if ((firstByte & 0xE0) == 0xC0) {
            charLen = 2;
        } else if ((firstByte & 0xF0) == 0xE0) {
            charLen = 3;
        } else if ((firstByte & 0xF8) == 0xF0) {
            charLen = 4;
        }
        
        // Extract the character
        std::string ch = text.substr(i, charLen);
        unsigned char encoded = encodeByte(ch, encoding);
        
        if (encoded != 0) {
            result.push_back(encoded);
        }
        
        i += charLen;
    }
    
    // Add terminator if there's room
    if (maxLength == 0 || result.size() < maxLength) {
        result.push_back(terminator);
    }
    
    // Pad with terminators if needed
    while (maxLength > 0 && result.size() < maxLength) {
        result.push_back(terminator);
    }
    
    return result;
}

#endif // TEXT_ENCODINGS_H