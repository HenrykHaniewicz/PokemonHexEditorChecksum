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
#  GUI
# ============================================================

def run_gui():
    """Launch the PyQt6 GUI for encoding/decoding."""
    try:
        from PyQt6.QtWidgets import (
            QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
            QLabel, QTextEdit, QPushButton, QComboBox, QMessageBox,
            QCheckBox, QFileDialog
        )
        from PyQt6.QtCore import Qt
    except ImportError:
        print("Error: PyQt6 is not installed.", file=sys.stderr)
        print("Please install it with: pip install PyQt6", file=sys.stderr)
        print()
        # Show usage
        parser = create_argument_parser()
        parser.print_help()
        sys.exit(1)

    # Color themes for each Gen 3 game
    THEMES = {
        # Ruby - Deep red theme
        0: {
            "name": "Ruby",
            "window_bg": "#2D1518",           # Dark red-brown
            "panel_bg": "#3D2528",            # Slightly lighter
            "text_bg": "#1A0D0F",             # Very dark red
            "text_fg": "#F0D0D0",             # Light pink-white
            "button_bg": "#A01020",           # Ruby red
            "button_hover": "#C01830",        # Brighter red
            "button_pressed": "#801018",      # Darker red
            "button_fg": "#FFFFFF",           # White
            "accent": "#E84040",              # Bright red accent
            "label_fg": "#E0B0B0",            # Pink-ish
            "combo_bg": "#4D2528",            # Medium dark red
            "combo_fg": "#F0D0D0",            # Light pink
            "checkbox_fg": "#E0B0B0",         # Pink-ish
            "status_fg": "#C09090",           # Muted pink
        },
        # Sapphire - Deep blue theme
        1: {
            "name": "Sapphire",
            "window_bg": "#151828",           # Dark blue
            "panel_bg": "#252838",            # Slightly lighter blue
            "text_bg": "#0D1018",             # Very dark blue
            "text_fg": "#D0D8F0",             # Light blue-white
            "button_bg": "#1040A0",           # Sapphire blue
            "button_hover": "#1850C0",        # Brighter blue
            "button_pressed": "#103080",      # Darker blue
            "button_fg": "#FFFFFF",           # White
            "accent": "#4080E8",              # Bright blue accent
            "label_fg": "#B0B8E0",            # Light blue
            "combo_bg": "#25284D",            # Medium dark blue
            "combo_fg": "#D0D8F0",            # Light blue
            "checkbox_fg": "#B0B8E0",         # Light blue
            "status_fg": "#9098C0",           # Muted blue
        },
        # FireRed - Orange-red fire theme
        2: {
            "name": "FireRed",
            "window_bg": "#2D1A10",           # Dark brown-orange
            "panel_bg": "#3D2A18",            # Slightly lighter
            "text_bg": "#1A0F08",             # Very dark brown
            "text_fg": "#F0E0D0",             # Warm white
            "button_bg": "#D04010",           # Fire orange
            "button_hover": "#E85020",        # Brighter orange
            "button_pressed": "#A03010",      # Darker orange
            "button_fg": "#FFFFFF",           # White
            "accent": "#FF6030",              # Bright orange accent
            "label_fg": "#E0C8B0",            # Warm tan
            "combo_bg": "#4D3020",            # Medium brown
            "combo_fg": "#F0E0D0",            # Warm white
            "checkbox_fg": "#E0C8B0",         # Warm tan
            "status_fg": "#C0A080",           # Muted tan
        },
        # LeafGreen - Fresh green theme
        3: {
            "name": "LeafGreen",
            "window_bg": "#152D18",           # Dark green
            "panel_bg": "#253D28",            # Slightly lighter green
            "text_bg": "#0D1A0F",             # Very dark green
            "text_fg": "#D0F0D8",             # Light green-white
            "button_bg": "#208030",           # Leaf green
            "button_hover": "#30A040",        # Brighter green
            "button_pressed": "#186028",      # Darker green
            "button_fg": "#FFFFFF",           # White
            "accent": "#40E060",              # Bright green accent
            "label_fg": "#B0E0B8",            # Light green
            "combo_bg": "#254D28",            # Medium dark green
            "combo_fg": "#D0F0D8",            # Light green
            "checkbox_fg": "#B0E0B8",         # Light green
            "status_fg": "#90C098",           # Muted green
        },
        # Emerald - Rich emerald green theme
        4: {
            "name": "Emerald",
            "window_bg": "#102D28",           # Dark teal-green
            "panel_bg": "#183D35",            # Slightly lighter
            "text_bg": "#081A18",             # Very dark teal
            "text_fg": "#D0F0E8",             # Light mint
            "button_bg": "#109068",           # Emerald green
            "button_hover": "#18B080",        # Brighter emerald
            "button_pressed": "#107050",      # Darker emerald
            "button_fg": "#FFFFFF",           # White
            "accent": "#30E0A0",              # Bright emerald accent
            "label_fg": "#A8E0D0",            # Light teal
            "combo_bg": "#204D45",            # Medium dark teal
            "combo_fg": "#D0F0E8",            # Light mint
            "checkbox_fg": "#A8E0D0",         # Light teal
            "status_fg": "#80C0B0",           # Muted teal
        },
    }

    def get_stylesheet(theme):
        return f"""
            QMainWindow, QWidget {{
                background-color: {theme["window_bg"]};
            }}
            QLabel {{
                color: {theme["label_fg"]};
                font-weight: bold;
            }}
            QTextEdit {{
                background-color: {theme["text_bg"]};
                color: {theme["text_fg"]};
                border: 2px solid {theme["accent"]};
                border-radius: 5px;
                padding: 5px;
                font-family: Consolas, Monaco, monospace;
                font-size: 11pt;
            }}
            QTextEdit:focus {{
                border: 2px solid {theme["button_bg"]};
            }}
            QPushButton {{
                background-color: {theme["button_bg"]};
                color: {theme["button_fg"]};
                border: none;
                border-radius: 5px;
                padding: 8px 16px;
                font-weight: bold;
                font-size: 10pt;
            }}
            QPushButton:hover {{
                background-color: {theme["button_hover"]};
            }}
            QPushButton:pressed {{
                background-color: {theme["button_pressed"]};
            }}
            QComboBox {{
                background-color: {theme["combo_bg"]};
                color: {theme["combo_fg"]};
                border: 2px solid {theme["accent"]};
                border-radius: 5px;
                padding: 5px 10px;
                font-weight: bold;
            }}
            QComboBox:hover {{
                border-color: {theme["button_bg"]};
            }}
            QComboBox::drop-down {{
                border: none;
                width: 20px;
            }}
            QComboBox::down-arrow {{
                image: none;
                border-left: 5px solid transparent;
                border-right: 5px solid transparent;
                border-top: 5px solid {theme["combo_fg"]};
                margin-right: 5px;
            }}
            QComboBox QAbstractItemView {{
                background-color: {theme["combo_bg"]};
                color: {theme["combo_fg"]};
                selection-background-color: {theme["button_bg"]};
                selection-color: {theme["button_fg"]};
            }}
            QCheckBox {{
                color: {theme["checkbox_fg"]};
                font-weight: bold;
            }}
            QCheckBox::indicator {{
                width: 18px;
                height: 18px;
                border: 2px solid {theme["accent"]};
                border-radius: 3px;
                background-color: {theme["text_bg"]};
            }}
            QCheckBox::indicator:checked {{
                background-color: {theme["button_bg"]};
                border-color: {theme["button_bg"]};
            }}
            QCheckBox::indicator:hover {{
                border-color: {theme["button_hover"]};
            }}
            #statusLabel {{
                color: {theme["status_fg"]};
                font-style: italic;
            }}
        """

    class MainWindow(QMainWindow):
        def __init__(self):
            super().__init__()
            self.setWindowTitle("Pokémon Gen 3 (GBA) Text Encoder/Decoder")
            self.setMinimumSize(600, 450)

            central_widget = QWidget()
            self.setCentralWidget(central_widget)
            layout = QVBoxLayout(central_widget)

            # Top bar: Region, Game selection, and Load button
            top_bar_layout = QHBoxLayout()

            # Region dropdown
            top_bar_layout.addWidget(QLabel("Region:"))
            self.region_combo = QComboBox()
            self.region_combo.addItems(["English/Western", "Japanese"])
            top_bar_layout.addWidget(self.region_combo)

            top_bar_layout.addSpacing(20)

            # Game dropdown
            top_bar_layout.addWidget(QLabel("Game:"))
            self.game_combo = QComboBox()
            self.game_combo.addItems([
                "Ruby",
                "Sapphire",
                "FireRed",
                "LeafGreen",
                "Emerald"
            ])
            self.game_combo.setCurrentIndex(4)  # Default to Emerald
            self.game_combo.currentIndexChanged.connect(self.update_theme)
            top_bar_layout.addWidget(self.game_combo)

            top_bar_layout.addStretch()

            # Load file button
            self.load_button = QPushButton("Load File...")
            self.load_button.setToolTip("Load text from a file")
            self.load_button.clicked.connect(self.load_file)
            top_bar_layout.addWidget(self.load_button)

            layout.addLayout(top_bar_layout)

            # Text field
            layout.addWidget(QLabel("Text:"))
            self.text_field = QTextEdit()
            self.text_field.setPlaceholderText("Enter text to encode here...")
            layout.addWidget(self.text_field)

            # Middle section: Encode/Decode buttons
            middle_layout = QHBoxLayout()
            middle_layout.addStretch()

            self.encode_button = QPushButton("▼ Encode ▼")
            self.encode_button.setToolTip("Convert text to bytes")
            self.encode_button.setMinimumWidth(120)
            self.encode_button.clicked.connect(self.encode)
            middle_layout.addWidget(self.encode_button)

            middle_layout.addSpacing(20)

            self.decode_button = QPushButton("▲ Decode ▲")
            self.decode_button.setToolTip("Convert bytes to text")
            self.decode_button.setMinimumWidth(120)
            self.decode_button.clicked.connect(self.decode)
            middle_layout.addWidget(self.decode_button)

            middle_layout.addStretch()
            layout.addLayout(middle_layout)

            # Bytes field
            layout.addWidget(QLabel("Bytes (hex):"))
            self.bytes_field = QTextEdit()
            self.bytes_field.setPlaceholderText("Enter hex bytes to decode here (e.g., BB BD or BBBD)...")
            layout.addWidget(self.bytes_field)

            # Bottom bar: Options and status
            bottom_layout = QHBoxLayout()

            # No spaces checkbox
            self.no_spaces_checkbox = QCheckBox("No spaces between bytes")
            self.no_spaces_checkbox.setToolTip("Output bytes without spaces (e.g., BBBD instead of BB BD)")
            bottom_layout.addWidget(self.no_spaces_checkbox)

            bottom_layout.addStretch()

            # Status label
            self.status_label = QLabel("")
            self.status_label.setObjectName("statusLabel")
            bottom_layout.addWidget(self.status_label)

            layout.addLayout(bottom_layout)

            # Apply initial theme (Emerald)
            self.update_theme()

        def update_theme(self):
            """Update the color theme based on selected game."""
            game_index = self.game_combo.currentIndex()
            theme = THEMES.get(game_index, THEMES[4])  # Default to Emerald
            self.setStyleSheet(get_stylesheet(theme))

        def get_game_name(self):
            """Get the game name from the dropdown."""
            game_map = {
                0: "ruby",
                1: "sapphire",
                2: "fr",
                3: "lg",
                4: "emerald"
            }
            return game_map.get(self.game_combo.currentIndex(), "emerald")

        def get_current_tables(self):
            """Get the encoding/decoding tables based on current selection."""
            is_japanese = self.region_combo.currentIndex() == 1
            game = self.get_game_name()
            return get_tables(game, is_japanese)

        def load_file(self):
            """Load text from a file and put it in both fields."""
            file_path, _ = QFileDialog.getOpenFileName(
                self,
                "Open Text File",
                "",
                "Text Files (*.txt);;All Files (*)"
            )

            if not file_path:
                return  # User cancelled

            try:
                with open(file_path, "r", encoding="utf-8") as f:
                    content = f.read()

                # Normalize line endings and convert to placeholder
                content = content.replace("\r\n", "\n").replace("\r", "\n")
                content_with_placeholders = content.replace("\n", "<0xFE>")

                # Put the content in both fields
                self.text_field.setPlainText(content_with_placeholders)
                self.bytes_field.setPlainText(content_with_placeholders)

                self.status_label.setText(f"Loaded: {os.path.basename(file_path)}")

            except Exception as e:
                QMessageBox.warning(self, "File Error", f"Could not read file:\n{e}")
                self.status_label.setText("File load failed.")

        def encode(self):
            """Encode text to bytes."""
            text = self.text_field.toPlainText()
            if not text:
                self.status_label.setText("No text to encode.")
                return

            encode_map, decode_map, is_english = self.get_current_tables()

            try:
                encoded = encode_text(text, encode_map, is_english=is_english)
                if self.no_spaces_checkbox.isChecked():
                    self.bytes_field.setPlainText("".join(encoded))
                else:
                    self.bytes_field.setPlainText(" ".join(encoded))
                self.status_label.setText(f"Encoded {len(text)} characters to {len(encoded)} bytes.")
            except ValueError as e:
                QMessageBox.warning(self, "Encoding Error", str(e))
                self.status_label.setText("Encoding failed.")

        def decode(self):
            """Decode bytes to text."""
            bytes_text = self.bytes_field.toPlainText()
            if not bytes_text:
                self.status_label.setText("No bytes to decode.")
                return

            encode_map, decode_map, is_english = self.get_current_tables()

            # Clean up the input - remove all whitespace
            cleaned = re.sub(r"\s+", "", bytes_text)

            if len(cleaned) % 2 != 0:
                QMessageBox.warning(
                    self, "Decoding Error",
                    "Byte string must have an even number of hex digits."
                )
                self.status_label.setText("Decoding failed.")
                return

            # Validate hex characters
            if not all(c in '0123456789ABCDEFabcdef' for c in cleaned):
                QMessageBox.warning(
                    self, "Decoding Error",
                    "Byte string contains invalid characters. Use only hex digits (0-9, A-F)."
                )
                self.status_label.setText("Decoding failed.")
                return

            bytes_list = [cleaned[i:i+2] for i in range(0, len(cleaned), 2)]
            decoded = decode_bytes(bytes_list, decode_map)
            self.text_field.setPlainText(decoded)
            self.status_label.setText(f"Decoded {len(bytes_list)} bytes to text.")

    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())


# ============================================================
#  CLI WITH FILE SUPPORT + NEWLINE SPECIAL CASES
# ============================================================

def create_argument_parser():
    """Create and return the argument parser."""
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
    return parser


def main():
    # Check if any arguments were passed (besides the script name)
    if len(sys.argv) == 1:
        # No arguments - launch GUI
        run_gui()
        return

    parser = create_argument_parser()
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