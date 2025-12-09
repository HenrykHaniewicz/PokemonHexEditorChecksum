# text_to_bytes_gb.py
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

    # Color themes for Gen 1 (Red/Blue mix) and Gen 2 (Gold/Silver mix)
    THEMES = {
        # Gen 1: Red and Blue gradient theme
        0: {
            "name": "Gen 1",
            "window_bg": "#2D1B4E",           # Deep purple (red+blue mix)
            "panel_bg": "#3D2B5E",            # Lighter purple
            "text_bg": "#1A1025",             # Dark purple-black
            "text_fg": "#E8E0F0",             # Light lavender
            "button_bg": "#8B2252",           # Deep rose
            "button_hover": "#A03262",        # Lighter rose
            "button_pressed": "#6B1242",      # Darker rose
            "button_fg": "#FFFFFF",           # White
            "accent": "#6A5ACD",              # Slate blue
            "label_fg": "#C8B8E8",            # Light purple
            "combo_bg": "#4D3B6E",            # Medium purple
            "combo_fg": "#E8E0F0",            # Light lavender
            "checkbox_fg": "#C8B8E8",         # Light purple
            "status_fg": "#A898D8",           # Muted purple
        },
        # Gen 2: Gold and Silver theme
        1: {
            "name": "Gen 2",
            "window_bg": "#2A2520",           # Dark bronze
            "panel_bg": "#3A3530",            # Medium bronze
            "text_bg": "#1A1815",             # Very dark brown
            "text_fg": "#F0E6D0",             # Cream/ivory
            "button_bg": "#B8860B",           # Dark goldenrod
            "button_hover": "#D4A017",        # Brighter gold
            "button_pressed": "#8B6914",      # Darker gold
            "button_fg": "#1A1510",           # Near black
            "accent": "#C0C0C0",              # Silver
            "label_fg": "#D4C4A8",            # Light tan
            "combo_bg": "#4A4540",            # Gray-brown
            "combo_fg": "#F0E6D0",            # Cream
            "checkbox_fg": "#D4C4A8",         # Light tan
            "status_fg": "#A89878",           # Muted gold
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
            self.setWindowTitle("Pokémon Gen 1/2 Text Encoder/Decoder")
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
            self.game_combo.addItems(["Gen 1 (Red/Blue/Yellow)", "Gen 2 (Gold/Silver/Crystal)"])
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
            self.bytes_field.setPlaceholderText("Enter hex bytes to decode here (e.g., 87 A4 B2 or 87A4B2)...")
            layout.addWidget(self.bytes_field)

            # Bottom bar: Options and status
            bottom_layout = QHBoxLayout()

            # No spaces checkbox
            self.no_spaces_checkbox = QCheckBox("No spaces between bytes")
            self.no_spaces_checkbox.setToolTip("Output bytes without spaces (e.g., 87A4B2 instead of 87 A4 B2)")
            bottom_layout.addWidget(self.no_spaces_checkbox)

            bottom_layout.addStretch()

            # Status label
            self.status_label = QLabel("")
            self.status_label.setObjectName("statusLabel")
            bottom_layout.addWidget(self.status_label)

            layout.addLayout(bottom_layout)

            # Apply initial theme
            self.update_theme()

        def update_theme(self):
            """Update the color theme based on selected game."""
            game_index = self.game_combo.currentIndex()
            theme = THEMES.get(game_index, THEMES[0])
            self.setStyleSheet(get_stylesheet(theme))

        def get_current_tables(self):
            """Get the encoding/decoding tables based on current selection."""
            is_japanese = self.region_combo.currentIndex() == 1
            gen = 1 if self.game_combo.currentIndex() == 0 else 2
            return get_tables(gen, is_japanese)

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
                content_with_placeholders = content.replace("\n", "<0x55>")

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
    return parser


def main():
    # Check if any arguments were passed (besides the script name)
    if len(sys.argv) == 1:
        # No arguments - launch GUI
        run_gui()
        return

    parser = create_argument_parser()
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