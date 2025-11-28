# text_to_bytes_gba.py
# -*- coding: utf-8 -*-

import argparse
import re
import sys
import os

# ============================================================
#  WESTERN (ENGLISH) TABLES - Generation III (GBA)
# ============================================================

# Base mapping for characters whose codepoints do not differ
# between Ruby/Sapphire and FireRed/LeafGreen/Emerald.
BASE_EN_G3_ENCODING = {
    # Space
    " ": "00",

    # Digits & common punctuation (A- row)
    "0": "A1",
    "1": "A2",
    "2": "A3",
    "3": "A4",
    "4": "A5",
    "5": "A6",
    "6": "A7",
    "7": "A8",
    "8": "A9",
    "9": "AA",
    "!": "AB",
    "?": "AC",
    ".": "AD",
    "-": "AE",
    "･": "AF",

    # Quotes / symbols (B- row, except ellipsis which differs)
    "“": "B1",
    "”": "B2",
    "‘": "B3",
    "'": "B4",
    "♂": "B5",
    "♀": "B6",
    "$": "B7",
    ",": "B8",
    "×": "B9",
    "/": "BA",

    # Uppercase A–Z (B- / C- / D- rows)
    "A": "BB",
    "B": "BC",
    "C": "BD",
    "D": "BE",
    "E": "BF",
    "F": "C0",
    "G": "C1",
    "H": "C2",
    "I": "C3",
    "J": "C4",
    "K": "C5",
    "L": "C6",
    "M": "C7",
    "N": "C8",
    "O": "C9",
    "P": "CA",
    "Q": "CB",
    "R": "CC",
    "S": "CD",
    "T": "CE",
    "U": "CF",
    "V": "D0",
    "W": "D1",
    "X": "D2",
    "Y": "D3",
    "Z": "D4",

    # Lowercase a–z (D- / E- rows)
    "a": "D5",
    "b": "D6",
    "c": "D7",
    "d": "D8",
    "e": "D9",
    "f": "DA",
    "g": "DB",
    "h": "DC",
    "i": "DD",
    "j": "DE",
    "k": "DF",
    "l": "E0",
    "m": "E1",
    "n": "E2",
    "o": "E3",
    "p": "E4",
    "q": "E5",
    "r": "E6",
    "s": "E7",
    "t": "E8",
    "u": "E9",
    "v": "EA",
    "w": "EB",
    "x": "EC",
    "y": "ED",
    "z": "EE",

    # Pointer / menu arrow
    "►": "EF",

    # Umlauts & colon (F- row)
    ":": "F0",
    "Ä": "F1",
    "Ö": "F2",
    "Ü": "F3",
    "ä": "F4",
    "ö": "F5",
    "ü": "F6",

    # Parentheses and percent (5- row – codes from common tables)
    "%": "5B",
    "(": "5C",
    ")": "5D",

    # Arrows (documented as 0x79–0x7C range in Western versions)
    "↑": "79",
    "↓": "7A",
    "←": "7B",
    "→": "7C",

    # Ampersand (2- row)
    "&": "2D",
}

# Ruby / Sapphire: 0xB0 is a two-dot ellipsis (‥)
EN_G3_RSE_ENCODING = {
    **BASE_EN_G3_ENCODING,
    "‥": "B0",
}
# Also accept three-dot ellipsis as the same byte
EN_G3_RSE_ENCODING["…"] = "B0"

# FireRed / LeafGreen / Emerald: 0xB0 is a three-dot ellipsis (…)
EN_G3_FRLG_E_ENCODING = {
    **BASE_EN_G3_ENCODING,
    "…": "B0",
}
# Also accept the two-dot ellipsis as input
EN_G3_FRLG_E_ENCODING["‥"] = "B0"

EN_G3_RSE_DECODING = {v: k for k, v in EN_G3_RSE_ENCODING.items()}
EN_G3_FRLG_E_DECODING = {v: k for k, v in EN_G3_FRLG_E_ENCODING.items()}


# ============================================================
#  JAPANESE TABLES - Generation III (GBA)
# ============================================================

def build_jp_g3_maps(ellipsis_char="‥"):
    """
    Build Japanese encoding/decoding maps for Gen 3.
    The only difference between variants is the glyph used
    for the ellipsis at 0xB0 (‥ vs …).
    """
    rows = {
        0x0: ["あ", "い", "う", "え", "お", "か", "き", "く", "け", "こ", "さ", "し", "す", "せ", "そ"],
        0x1: ["た", "ち", "つ", "て", "と", "な", "に", "ぬ", "ね", "の", "は", "ひ", "ふ", "へ", "ほ", "ま"],
        0x2: ["み", "む", "め", "も", "や", "ゆ", "よ", "ら", "り", "る", "れ", "ろ", "わ", "を", "ん", "ぁ"],
        0x3: ["ぃ", "ぅ", "ぇ", "ぉ", "ゃ", "ゅ", "ょ", "が", "ぎ", "ぐ", "げ", "ご", "ざ", "じ", "ず", "ぜ"],
        0x4: ["ぞ", "だ", "ぢ", "づ", "で", "ど", "ば", "び", "ぶ", "べ", "ぼ", "ぱ", "ぴ", "ぷ", "ぺ", "ぽ"],
        0x5: ["っ", "ア", "イ", "ウ", "エ", "オ", "カ", "キ", "ク", "ケ", "コ", "サ", "シ", "ス", "セ", "ソ"],
        0x6: ["タ", "チ", "ツ", "テ", "ト", "ナ", "ニ", "ヌ", "ネ", "ノ", "ハ", "ヒ", "フ", "ヘ", "ホ", "マ"],
        0x7: ["ミ", "ム", "メ", "モ", "ヤ", "ユ", "ヨ", "ラ", "リ", "ル", "レ", "ロ", "ワ", "ヲ", "ン", "ァ"],
        0x8: ["ィ", "ゥ", "ェ", "ォ", "ャ", "ュ", "ョ", "ガ", "ギ", "グ", "ゲ", "ゴ", "ザ", "ジ", "ズ", "ゼ"],
        0x9: ["ゾ", "ダ", "ヂ", "ヅ", "デ", "ド", "バ", "ビ", "ブ", "ベ", "ボ", "パ", "ピ", "プ", "ペ", "ポ"],
        0xA: ["ッ", "０", "１", "２", "３", "４", "５", "６", "７", "８", "９", "！", "？", "。", "ー", "・"],
        0xB: [ellipsis_char, "『", "』", "「", "」", "♂", "♀", "円", "．", "×", "／", "Ａ", "Ｂ", "Ｃ", "Ｄ", "Ｅ"],
        0xC: ["Ｆ", "Ｇ", "Ｈ", "Ｉ", "Ｊ", "Ｋ", "Ｌ", "Ｍ", "Ｎ", "Ｏ", "Ｐ", "Ｑ", "Ｒ", "Ｓ", "Ｔ", "Ｕ"],
        0xD: ["Ｖ", "Ｗ", "Ｘ", "Ｙ", "Ｚ", "ａ", "ｂ", "ｃ", "ｄ", "ｅ", "ｆ", "ｇ", "ｈ", "ｉ", "ｊ", "ｋ"],
        0xE: ["ｌ", "ｍ", "ｎ", "ｏ", "ｐ", "ｑ", "ｒ", "ｓ", "ｔ", "ｕ", "ｖ", "ｗ", "ｘ", "ｙ", "ｚ", "►"],
        0xF: ["：", "Ä", "Ö", "Ü", "ä", "ö", "ü"],  # F7-FF are control characters
    }

    enc = {}
    dec = {}
    for hi, row in rows.items():
        # pad to 16 entries where needed
        if len(row) < 16:
            row = row + [None] * (16 - len(row))
        for lo, cell in enumerate(row):
            if cell is None:
                continue
            b = f"{hi:X}{lo:X}"
            dec[b] = cell
            enc.setdefault(cell, b)
    return enc, dec


# Japanese ellipsis rules:
# - Ruby/Sapphire/Emerald: two-dot ‥
# - FireRed/LeafGreen: three-dot …
JP_G3_RSE_ENCODING, JP_G3_RSE_DECODING = build_jp_g3_maps(ellipsis_char="‥")
JP_G3_FRLG_ENCODING, JP_G3_FRLG_DECODING = build_jp_g3_maps(ellipsis_char="…")


# ============================================================
#  TABLE SELECTION
# ============================================================

def get_tables(game: str, japan: bool):
    """
    Select appropriate encoding/decoding maps.

    game: one of 'ruby', 'sapphire', 'fr', 'lg', 'emerald'
    japan: True to use Japanese character set, False for Western.
    """
    game = game.lower()
    if japan:
        # In the Japanese set, Ruby/Sapphire/Emerald share the two-dot ellipsis;
        # FireRed/LeafGreen use the three-dot ellipsis.
        if game in ("ruby", "sapphire", "emerald"):
            return JP_G3_RSE_ENCODING, JP_G3_RSE_DECODING, False
        else:  # fr / lg (or anything else, conservatively)
            return JP_G3_FRLG_ENCODING, JP_G3_FRLG_DECODING, False
    else:
        # English set: Ruby/Sapphire vs FR/LG/Emerald
        if game in ("ruby", "sapphire"):
            return EN_G3_RSE_ENCODING, EN_G3_RSE_DECODING, True
        else:  # fr / lg / emerald (or default)
            return EN_G3_FRLG_E_ENCODING, EN_G3_FRLG_E_DECODING, True


# ============================================================
#  ENCODING & DECODING
# ============================================================

PLACEHOLDER_RE = re.compile(r"^<0x([0-9A-Fa-f]{2})>$")


def encode_text(s: str, mapping: dict, is_english: bool):
    """
    Encode a Unicode string to a list of byte hex strings (e.g. ['87','A4',...]).

    - If is_english is True, handle apostrophe contractions ('d, 'l, 's, 't, 'v, 'm, 'r)
      *if* they exist in the mapping (Gen 3 normally does not, but the logic is left in
      for consistency with the Gen 1/2 script).
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

        # 2) English contractions (only if the combined token exists)
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
        description="Pokémon Gen 3 (GBA) text encoder/decoder (Western & Japanese)"
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

    group = parser.add_mutually_exclusive_group()
    group.add_argument("--ruby", action="store_true", help="Use Pokémon Ruby encoding")
    group.add_argument("--sapphire", action="store_true", help="Use Pokémon Sapphire encoding")
    group.add_argument("--fr", action="store_true", help="Use Pokémon FireRed encoding")
    group.add_argument("--lg", action="store_true", help="Use Pokémon LeafGreen encoding")
    group.add_argument("--emerald", action="store_true", help="Use Pokémon Emerald encoding")

    parser.add_argument(
        "-j", "--japan", action="store_true",
        help="Use Japanese character set instead of Western"
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

    if args.ruby:
        game = "ruby"
    elif args.sapphire:
        game = "sapphire"
    elif args.fr:
        game = "fr"
    elif args.lg:
        game = "lg"
    elif args.emerald:
        game = "emerald"
    else:
        # Default to Emerald-style English encoding if nothing specified.
        game = "emerald"

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

    # Select mapping
    encode_map, decode_map, is_english = get_tables(game, args.japan)

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
    # Special case: when encoding FROM A FILE, treat line breaks as 0xFE
    # (Gen 3's line break control code), using the placeholder mechanism.
    if args.file:
        input_text = input_text.replace("\r\n", "\n").replace("\r", "\n")
        input_text = input_text.replace("\n", "<0xFE>")

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