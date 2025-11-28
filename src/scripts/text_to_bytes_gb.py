# -*- coding: utf-8 -*-

import argparse
import re
import sys
import os

# ============================================================
#  ENGLISH TABLES (Gen 1 & Gen 2)
# ============================================================

# -------- Gen 1 English --------
EN_G1_ENCODING = {
    # Uppercase A-P: 0x80-0x8F
    **{chr(ord('A') + i): f"8{i:X}" for i in range(16)},
    # Uppercase Q-Z: 0x90-0x99
    **{chr(ord('Q') + i): f"9{i:X}" for i in range(10)},
    "(": "9A",
    ")": "9B",
    ":": "9C",
    ";": "9D",
    "[": "9E",
    "]": "9F",

    # Lowercase a-p: 0xA0-0xAF
    **{chr(ord('a') + i): f"A{i:X}" for i in range(16)},
    # Lowercase q-z: 0xB0-0xB9
    **{chr(ord('q') + i): f"B{i:X}" for i in range(10)},
    "é": "BA",

    # Contractions
    "'d": "BB",
    "'l": "BC",
    "'s": "BD",
    "'t": "BE",
    "'v": "BF",

    # Space
    " ": "7F",

    # Row 7- (0x70-0x7F)
    "‘": "70",
    "’": "71",
    "“": "72",
    "”": "73",
    "・": "74",
    "…": "75",
    "ぁ": "76",
    "ぇ": "77",
    "ぉ": "78",
    "╔": "79",
    "═": "7A",
    "╗": "7B",
    "║": "7C",
    "╚": "7D",
    "╝": "7E",

    # Row 6- bits you gave
    "：": "6D",   # full-width colon
    "ぃ": "6E",
    "ぅ": "6F",

    # Row E-
    "'": "E0",
    # E1/E2 PK/MN are left to placeholders if needed
    "-": "E3",
    "'r": "E4",
    "'m": "E5",
    "?": "E6",
    "!": "E7",
    ".": "E8",
    "ァ": "E9",
    "ゥ": "EA",
    "ェ": "EB",
    "▷": "EC",
    "▶": "ED",
    "▼": "EE",
    "♂": "EF",

    # Row F-
    "$": "F0",      # Poké Dollar
    "×": "F1",
    ".(alt)": "F2",  # explicit token if you need the alt dot
    "/": "F3",
    ",": "F4",
    "♀": "F5",
    **{str(i): f"F{6+i:X}" for i in range(10)},  # 0-9
}

EN_G1_DECODING = {v: k for k, v in EN_G1_ENCODING.items()}
EN_G1_DECODING["F2"] = "."   # alt dot decodes as '.'


# -------- Gen 2 English --------
EN_G2_ENCODING = {
    # Uppercase A-P: 0x80-0x8F
    **{chr(ord('A') + i): f"8{i:X}" for i in range(16)},
    # Uppercase Q-Z: 0x90-0x99
    **{chr(ord('Q') + i): f"9{i:X}" for i in range(10)},
    "(": "9A",
    ")": "9B",
    ":": "9C",
    ";": "9D",
    "[": "9E",
    "]": "9F",

    # Lowercase a-p: 0xA0-0xAF
    **{chr(ord('a') + i): f"A{i:X}" for i in range(16)},
    # Lowercase q-z: 0xB0-0xB9
    **{chr(ord('q') + i): f"B{i:X}" for i in range(10)},

    # Umlauts
    "Ä": "C0",
    "Ö": "C1",
    "Ü": "C2",
    "ä": "C3",
    "ö": "C4",
    "ü": "C5",

    # Contractions / ligatures
    "'d": "D0",
    "'l": "D1",
    "'m": "D2",
    "'r": "D3",
    "'s": "D4",
    "'t": "D5",
    "'v": "D6",

    # Space
    " ": "7F",

    # Punctuation & symbols
    "'": "E0",
    "?": "E6",
    "!": "E7",
    ".": "E8",
    "&": "E9",
    "é": "EA",

    "$": "F0",
    "×": "F1",
    ".(alt)": "F2",
    "/": "F3",
    ",": "F4",
    "♀": "F5",
    **{str(i): f"F{6+i:X}" for i in range(10)},  # 0-9
}

EN_G2_DECODING = {v: k for k, v in EN_G2_ENCODING.items()}
EN_G2_DECODING["F2"] = "."


# ============================================================
#  JAPANESE TABLES (Gen 1 & Gen 2)
# ============================================================

def build_jp_gen1_maps():
    rows = {
        0x0: ["NULL", "イ゙", "ヴ", "エ゙", "オ゙", "ガ", "ギ", "グ", "ゲ", "ゴ", "ザ", "ジ", "ズ", "ゼ", "ゾ", "ダ"],
        0x1: ["ヂ", "ヅ", "デ", "ド", "ナ゙", "ニ゙", "ヌ゙", "ネ゙", "ノ゙", "バ", "ビ", "ブ", "ボ", "マ゙", "ミ゙", "ム゙"],
        0x2: ["ィ゙", "あ゙", "い゙", "ゔ", "え゙", "お゙", "が", "ぎ", "ぐ", "げ", "ご", "ざ", "じ", "ず", "ぜ", "ぞ"],
        0x3: ["だ", "ぢ", "づ", "で", "ど", "な゙", "に゙", "ぬ゙", "ね゙", "の゙", "ば", "び", "ぶ", "べ", "ぼ", "ま゙"],
        0x4: ["パ", "ピ", "プ", "ポ", "ぱ", "ぴ", "ぷ", "ぺ", "ぽ", "ま゚", "Control", "も゚", "Control", None, None, None],
        0x5: ["Control characters"] * 16,
        0x6: ["A", "B", "C", "D", "E", "F", "G", "H", "I", "V", "S", "L", "M", "：", "ぃ", "ぅ"],
        0x7: ["「", "」", "『", "』", "・", "…", "ぁ", "ぇ", "ぉ",
              "Text box borders", None, None, None, None, None, None],
        0x8: ["ア", "イ", "ウ", "エ", "オ", "カ", "キ", "ク", "ケ", "コ", "サ", "シ", "ス", "セ", "ソ", "タ"],
        0x9: ["チ", "ツ", "テ", "ト", "ナ", "ニ", "ヌ", "ネ", "ノ", "ハ", "ヒ", "フ", "ホ", "マ", "ミ", "ム"],
        0xA: ["メ", "モ", "ヤ", "ユ", "ヨ", "ラ", "ル", "レ", "ロ", "ワ", "ヲ", "ン", "ッ", "ャ", "ュ", "ョ"],
        0xB: ["ィ", "あ", "い", "う", "え", "お", "か", "き", "く", "け", "こ", "さ", "し", "す", "せ", "そ"],
        0xC: ["た", "ち", "つ", "て", "と", "な", "に", "ぬ", "ね", "の", "は", "ひ", "ふ", "へ", "ほ", "ま"],
        0xD: ["み", "む", "め", "も", "や", "ゆ", "よ", "ら", "リ", "る", "れ", "ろ", "わ", "を", "ん", "っ"],
        0xE: ["ゃ", "ゅ", "ょ", "ー", "゜", "゛", "?", "!", "。", "ァ", "ゥ", "ェ", "▷", "▶", "▼", "♂"],
        0xF: ["円", "×", ".", "/", "ォ", "♀", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"],
    }

    enc = {}
    dec = {}
    skip_tokens = {"NULL", "Control", "Control characters", "Text box borders", "*"}
    for hi, row in rows.items():
        if len(row) < 16:
            row = row + [None] * (16 - len(row))
        for lo, cell in enumerate(row):
            byte = f"{hi:X}{lo:X}"
            if not cell or cell in skip_tokens:
                continue
            dec[byte] = cell
            enc.setdefault(cell, byte)
    return enc, dec


def build_jp_gen2_maps():
    rows = {
        0x0: ["?", "イ゙", "ヴ", "エ゙", "オ゙", "ガ", "ギ", "グ", "ゲ", "ゴ", "ザ", "ジ", "ズ", "ゼ", "ゾ", "ダ"],
        0x1: ["ヂ", "ヅ", "デ", "ド", "*", "*", "*", "ネ゙", "ノ゙", "バ", "ビ", "ブ", "ボ", "*", "*", "*"],
        0x2: ["ィ゙", "あ゙", "*", "*", "*", "*", "が", "ぎ", "ぐ", "げ", "ご", "ざ", "じ", "ず", "ぜ", "ぞ"],
        0x3: ["だ", "ぢ", "づ", "で", "ど", "*", "*", "*", "*", "*", "ば", "び", "ぶ", "べ", "ぼ", "*"],
        0x4: ["パ", "ピ", "プ", "ポ", "ぱ", "ぴ", "ぷ", "ぺ", "ぽ", "*", "*", "*", "*", "も゚", "*", "*"],
        0x5: ["*"] * 16,
        0x6: ["", "▲", "", "D", "E", "F", "G", "H", "I", "V", "S", "L", "M", ":", "ぃ", "ぅ"],
        0x7: ["「", "」", "『", "』", "・", "…", "ぁ", "ぇ", "ぉ",
              "text box borders", None, None, None, None, None, None],
        0x8: ["ア", "イ", "ウ", "エ", "オ", "カ", "キ", "ク", "ケ", "コ", "サ", "シ", "ス", "セ", "ソ", "タ"],
        0x9: ["チ", "ツ", "テ", "ト", "ナ", "ニ", "ヌ", "ネ", "ノ", "ハ", "ヒ", "フ", "ホ", "マ", "ミ", "ム"],
        0xA: ["メ", "モ", "ヤ", "ユ", "ヨ", "ラ", "ル", "レ", "ロ", "ワ", "ヲ", "ン", "ッ", "ャ", "ュ", "ョ"],
        0xB: ["ィ", "あ", "い", "う", "え", "お", "か", "き", "く", "け", "こ", "さ", "し", "す", "せ", "そ"],
        0xC: ["た", "ち", "つ", "て", "と", "な", "に", "ぬ", "ね", "の", "は", "ひ", "ふ", "へ", "ほ", "ま"],
        0xD: ["み", "む", "め", "も", "や", "ゆ", "よ", "ら", "リ", "る", "れ", "ろ", "わ", "を", "ん", "っ"],
        0xE: ["ゃ", "ゅ", "ょ", "ー", "゜", "゛", "?", "!", "。", "ァ", "ゥ", "ェ", "▷", "▶", "▼", "♂"],
        0xF: ["円", "×", ".", "/", "ォ", "♀", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"],
    }

    enc = {}
    dec = {}
    skip_tokens = {"NULL", "Control", "Control characters", "text box borders", "*", ""}
    for hi, row in rows.items():
        if len(row) < 16:
            row = row + [None] * (16 - len(row))
        for lo, cell in enumerate(row):
            byte = f"{hi:X}{lo:X}"
            if not cell or cell in skip_tokens:
                continue
            dec[byte] = cell
            enc.setdefault(cell, byte)
    return enc, dec


JP_G1_ENCODING, JP_G1_DECODING = build_jp_gen1_maps()
JP_G2_ENCODING, JP_G2_DECODING = build_jp_gen2_maps()


# ============================================================
#  TABLE SELECTION
# ============================================================

def get_tables(gen: int, japan: bool):
    if japan:
        if gen == 1:
            return JP_G1_ENCODING, JP_G1_DECODING, False
        else:
            return JP_G2_ENCODING, JP_G2_DECODING, False
    else:
        if gen == 1:
            return EN_G1_ENCODING, EN_G1_DECODING, True
        else:
            return EN_G2_ENCODING, EN_G2_DECODING, True


# ============================================================
#  ENCODING & DECODING
# ============================================================

PLACEHOLDER_RE = re.compile(r"^<0x([0-9A-Fa-f]{2})>$")


def encode_text(s: str, mapping: dict, is_english: bool):
    """
    Encode a Unicode string to a list of byte hex strings (e.g. ['87','A4',...])

    - If is_english is True, handle apostrophe contractions ('d, 'l, 's, 't, 'v, 'm, 'r).
    - Support placeholder tokens <0xHH> -> direct byte insertion.
    """
    result = []
    i = 0
    length = len(s)

    while i < length:
        ch = s[i]

        # 1) Placeholder token: <0xHH>
        if ch == "<":
            end = s.find(">", i + 1)
            if end != -1:
                token = s[i:end+1]
                m = PLACEHOLDER_RE.match(token)
                if m:
                    byte = m.group(1).upper()
                    result.append(byte)
                    i = end + 1
                    continue
            # If not valid placeholder, fall through and treat '<' as normal

        # 2) English contractions
        if is_english and ch == "'" and i + 1 < length:
            pair = s[i:i+2]
            if pair in mapping:
                result.append(mapping[pair])
                i += 2
                continue

        # 3) Normal single-character lookup
        if ch not in mapping:
            raise ValueError(f"Character not encodable in this table: {ch!r}")
        result.append(mapping[ch])
        i += 1

    return result


def decode_bytes(byte_list, reverse_map: dict):
    """
    Decode list of hex byte strings to text.

    - If byte not found in reverse_map, produce placeholder token <0xHH>
      so it can be round-tripped.
    """
    out = []
    for b in byte_list:
        b = b.upper()
        if b in reverse_map:
            out.append(reverse_map[b])
        else:
            out.append(f"<0x{b}>")
    return "".join(out)


# ============================================================
#  CLI WITH FILE SUPPORT + NEWLINE SPECIAL CASES
# ============================================================

def main():
    parser = argparse.ArgumentParser(
        description="Pokémon Gen 1/2 text encoder/decoder (English & Japanese)"
    )
    parser.add_argument(
        "text",
        nargs="?",
        help="Input text (encode) or hex bytes (decode). Optional when using -f."
    )
    parser.add_argument(
        "-f", "--file",
        help="Read input text or bytes from a file. Overrides command-line input."
    )
    parser.add_argument(
        "--gen", type=int, choices=[1, 2], default=1,
        help="Generation: 1 or 2 (default: 1)"
    )
    parser.add_argument(
        "-j", "--japan", action="store_true",
        help="Use Japanese tables instead of English"
    )
    parser.add_argument(
        "-r", "--reverse", action="store_true",
        help="Decode bytes → text instead of encode"
    )
    parser.add_argument(
        "-n", action="store_true",
        help="No spaces between output byte values"
    )

    args = parser.parse_args()

    # ------------------------------
    # Load input
    # ------------------------------
    if args.file:
        if not os.path.isfile(args.file):
            print(f"Error: file not found: {args.file}", file=sys.stderr)
            sys.exit(1)
        try:
            with open(args.file, "r", encoding="utf-8") as f:
                input_text = f.read()
        except Exception as e:
            print(f"Error reading file: {e}", file=sys.stderr)
            sys.exit(1)
    else:
        if args.text is None:
            print("Error: No input provided. Use either TEXT or -f FILE.", file=sys.stderr)
            sys.exit(1)
        input_text = args.text

    encode_map, decode_map, is_english = get_tables(args.gen, args.japan)

    # ------------------------------
    # DECODE MODE
    # ------------------------------
    if args.reverse:
        # Treat all whitespace (spaces, newlines, tabs, etc.) as separators.
        cleaned = re.sub(r"\s+", "", input_text)
        if len(cleaned) % 2 != 0:
            print("Error: byte string must have an even number of hex digits.",
                  file=sys.stderr)
            sys.exit(1)
        bytes_list = [cleaned[i:i+2] for i in range(0, len(cleaned), 2)]
        decoded = decode_bytes(bytes_list, decode_map)
        print(decoded)
        return

    # ------------------------------
    # ENCODE MODE
    # ------------------------------
    # Special case: when encoding FROM A FILE, treat line breaks as <0x55>
    # so they become the raw byte 0x55 via placeholder mechanism.
    if args.file:
        # Normalise line endings then turn newlines into placeholder.
        input_text = input_text.replace("\r\n", "\n").replace("\r", "\n")
        input_text = input_text.replace("\n", "<0x55>")

    try:
        encoded = encode_text(input_text, encode_map, is_english=is_english)
    except ValueError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

    if args.n:
        print("".join(encoded))
    else:
        print(" ".join(encoded))


if __name__ == "__main__":
    main()
