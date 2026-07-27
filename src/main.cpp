//=============================================================================
// src/main.cpp - Unified Pokemon Save Tool Suite
// Compile with: make single   (or wrap in a .app with: make app)
//=============================================================================

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <atomic>

#include "hex_editor/hex_editor.h"
#include "checksum/checksum_calc.h"
#include "mirage_island/mirage_island.h"
#include "pokemon_bag/pokemon_bag.h"
#include "pokemon_party/pokemon_party.h"
#include "pokemon_trainer/pokemon_trainer.h"

// ============================================================================
// Constants
// ============================================================================
namespace {

constexpr int   WIN_W    = 820;
constexpr int   WIN_H    = 640;
constexpr float PAD      = 20.0f;
constexpr float FONT_SZ  = 16.0f;
constexpr float TITLE_SZ = 26.0f;
constexpr float SMALL_SZ = 13.0f;
constexpr float BTN_H    = 32.0f;
constexpr float CB_SZ    = 18.0f;
constexpr float INPUT_H  = 30.0f;

// Colour palette
const SDL_Color C_BG         = {45,  45,  45,  255};
const SDL_Color C_BTN        = {74,  144, 217, 255};
const SDL_Color C_BTN_HOV    = {90,  160, 233, 255};
const SDL_Color C_SEL        = {76,  175, 80,  255};
const SDL_Color C_SEL_HOV    = {102, 187, 106, 255};
const SDL_Color C_PANEL      = {58,  58,  58,  255};
const SDL_Color C_TEXT       = {255, 255, 255, 255};
const SDL_Color C_DIMTEXT    = {170, 170, 170, 255};
const SDL_Color C_INPUT_BG   = {26,  26,  26,  255};
const SDL_Color C_INPUT_BDR  = {100, 100, 100, 255};
const SDL_Color C_INPUT_ACT  = {74,  144, 217, 255};
const SDL_Color C_ERR        = {244, 67,  54,  255};
const SDL_Color C_LAUNCH     = {76,  175, 80,  255};
const SDL_Color C_LAUNCH_HOV = {102, 187, 106, 255};
const SDL_Color C_SEP        = {80,  80,  80,  255};

// Tool identifiers
enum class TID { NONE = 0, HEX, CHECKSUM, MIRAGE, BAG, PARTY, TRAINER };

struct ToolDef {
    TID  id;
    const char* name;
    const char* desc;
    bool needsGame, hasJpn, hasWrite, hasPokemon, hasGrouping, hasEncoding;
    std::vector<std::string> games;
};

const ToolDef TOOLS[] = {
    { TID::HEX, "Hex Editor", "View and edit save / ROM file bytes",
      false, false, false, false, true, true, {} },
    { TID::CHECKSUM, "Checksum", "Calculate and fix save-file checksums",
      true, true, true, true, false, false,
      {"red","blue","yellow","green","gold","silver","crystal",
       "ruby","sapphire","emerald","firered","leafgreen"} },
    { TID::MIRAGE, "Mirage Island", "Set Mirage Island value from party PID",
      true, false, false, false, false, false,
      {"ruby","sapphire","emerald"} },
    { TID::BAG, "Pokemon Bag", "Edit bag items and quantities",
      true, true, false, false, false, false,
      {"red","blue","yellow","green","gold","silver","crystal",
       "ruby","sapphire","emerald","firered","leafgreen"} },
    { TID::PARTY, "Pokemon Party", "Edit party Pokemon data and stats",
      true, true, false, false, false, false,
      {"red","blue","yellow","green","gold","silver","crystal",
       "ruby","sapphire","emerald","firered","leafgreen"} },
    { TID::TRAINER, "Pokemon Trainer", "Edit trainer data in ROMs",
      true, true, false, false, false, false,
      {"red","blue","yellow","gold","silver","crystal",
       "ruby","sapphire","emerald","firered","leafgreen"} },
};
constexpr int N_TOOLS = sizeof(TOOLS) / sizeof(TOOLS[0]);

const char* ENC_NAMES[] = {"ASCII", "E1", "E2", "E3", "J1", "J2", "J3"};
constexpr int N_ENC = 7;
const int GRP_VALS[] = {1, 2, 4, 8};
constexpr int N_GRP = 4;

// File dialog filters (used by the Browse button)
const SDL_DialogFileFilter kFileFilters[] = {
    { "Save & ROM files", "sav;srm;gb;gbc;gba;dat;bin" },
    { "All files",        "*" },
};
constexpr int kNumFilters = sizeof(kFileFilters) / sizeof(kFileFilters[0]);

// Hit-testable float rect
struct FR {
    float x = 0, y = 0, w = 0, h = 0;
    bool hit(float mx, float my) const {
        return mx >= x && mx < x + w && my >= y && my < y + h;
    }
};

} // anonymous namespace

// ============================================================================
// Launcher class
// ============================================================================
class Launcher {
public:
    bool init()    { return initSDL(); }
    void run();
    void cleanup() { cleanupSDL(); }

private:
    // ---- SDL resources (created / destroyed by initSDL / cleanupSDL) ----
    SDL_Window*   win_    = nullptr;
    SDL_Renderer* ren_    = nullptr;
    TTF_Font*     fNorm_  = nullptr;
    TTF_Font*     fTitle_ = nullptr;
    TTF_Font*     fSmall_ = nullptr;

    bool initSDL();
    void cleanupSDL();
    TTF_Font* findFont(float sz);

    // ---- Persistent state (survives tool launches) ----
    int         selTool_ = -1;       // index into TOOLS[]
    std::string file_;               // file path
    std::string game_;               // selected game name
    bool jpn_ = false, ow_ = false, wr_ = false, pkm_ = false;
    int  grp_ = 1, enc_ = 0;        // grouping value, encoding index

    // ---- UI transient state ----
    bool running_   = true;
    bool fileAct_   = false;         // file text-input active
    int  fileCur_   = 0;             // cursor position in file_
    int  fileScrX_  = 0;             // horizontal pixel scroll
    std::string err_;                // error string shown below Launch
    float mx_ = 0, my_ = 0;         // last known mouse position

    // ---- Native file-dialog hand-off (callback may run off-thread) ----
    std::atomic<bool> pendingReady_{false};
    std::string       pendingPath_;

    // ---- Cached layout (populated every render, consumed by onEvent) ----
    struct Layout {
        FR toolBtn[6];
        FR fileIn;
        FR browse;
        FR gameBtns[16];   int nGame = 0;
        FR cbOW, cbJpn, cbWr, cbPkm;
        bool showJpn = false, showWr = false, showPkm = false;
        FR grpBtn[4];      bool showGrp = false;
        FR encBtn[7];      bool showEnc = false;
        FR launch;
    } L_;

    // ---- Drawing primitives ----
    void dText(const std::string& s, float x, float y,
               SDL_Color c, TTF_Font* f = nullptr);
    void tSize(const std::string& s, int& w, int& h,
               TTF_Font* f = nullptr);
    void dRect(float x, float y, float w, float h,
               SDL_Color c, bool fill = true);
    void dButton(const std::string& lbl, FR& out,
                 float x, float y, float w, float h,
                 bool sel, bool hov);
    void dCheckbox(const std::string& lbl, FR& out,
                   float x, float y, bool chk);

    // ---- High-level ----
    void render();
    void onEvent(SDL_Event& ev);
    bool launchTool();

    // ---- File dialog ----
    static void SDLCALL onFileDialog(void* userdata,
                                     const char* const* filelist, int filter);
    void openFileDialog();
};

// ============================================================================
// Font loading - tries common system monospace font paths
// ============================================================================
TTF_Font* Launcher::findFont(float sz) {
    static const char* paths[] = {
#ifdef __APPLE__
        "/System/Library/Fonts/SFMono-Regular.otf",
        "/System/Library/Fonts/Menlo.ttc",
        "/System/Library/Fonts/Monaco.ttf",
        "/Library/Fonts/Courier New.ttf",
#elif defined(_WIN32)
        "C:\\Windows\\Fonts\\consola.ttf",
        "C:\\Windows\\Fonts\\cour.ttf",
        "C:\\Windows\\Fonts\\lucon.ttf",
#else
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        "/usr/share/fonts/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/freefont/FreeMono.ttf",
#endif
        nullptr
    };
    for (int i = 0; paths[i]; ++i) {
        TTF_Font* f = TTF_OpenFont(paths[i], sz);
        if (f) return f;
    }
    return nullptr;
}

// ============================================================================
// SDL init / cleanup
// ============================================================================
bool Launcher::initSDL() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init: " << SDL_GetError() << "\n";
        return false;
    }
    if (!TTF_Init()) {
        std::cerr << "TTF_Init: " << SDL_GetError() << "\n";
        return false;
    }
    win_ = SDL_CreateWindow("Pokemon Save Tools", WIN_W, WIN_H, 0);
    if (!win_) {
        std::cerr << "SDL_CreateWindow failed\n";
        return false;
    }
    ren_ = SDL_CreateRenderer(win_, nullptr);
    if (!ren_) {
        std::cerr << "SDL_CreateRenderer failed\n";
        return false;
    }
    fNorm_  = findFont(FONT_SZ);
    fTitle_ = findFont(TITLE_SZ);
    fSmall_ = findFont(SMALL_SZ);
    if (!fNorm_ || !fTitle_) {
        std::cerr << "Could not find a system monospace font.\n";
        return false;
    }
    if (!fSmall_) fSmall_ = fNorm_;
    return true;
}

void Launcher::cleanupSDL() {
    if (fSmall_ && fSmall_ != fNorm_) TTF_CloseFont(fSmall_);
    if (fTitle_) TTF_CloseFont(fTitle_);
    if (fNorm_)  TTF_CloseFont(fNorm_);
    fSmall_ = fTitle_ = fNorm_ = nullptr;
    if (ren_) SDL_DestroyRenderer(ren_);
    ren_ = nullptr;
    if (win_) SDL_DestroyWindow(win_);
    win_ = nullptr;
    TTF_Quit();
    SDL_Quit();
}

// ============================================================================
// Drawing primitives
// ============================================================================
void Launcher::dText(const std::string& s, float x, float y,
                     SDL_Color c, TTF_Font* f) {
    if (s.empty()) return;
    if (!f) f = fNorm_;
    SDL_Surface* sf = TTF_RenderText_Blended(f, s.c_str(), 0, c);
    if (!sf) return;
    SDL_Texture* tx = SDL_CreateTextureFromSurface(ren_, sf);
    if (tx) {
        SDL_FRect d{x, y, (float)sf->w, (float)sf->h};
        SDL_RenderTexture(ren_, tx, nullptr, &d);
        SDL_DestroyTexture(tx);
    }
    SDL_DestroySurface(sf);
}

void Launcher::tSize(const std::string& s, int& w, int& h, TTF_Font* f) {
    if (!f) f = fNorm_;
    if (s.empty()) { w = 0; h = (int)FONT_SZ; return; }
    TTF_GetStringSize(f, s.c_str(), 0, &w, &h);
}

void Launcher::dRect(float x, float y, float w, float h,
                     SDL_Color c, bool fill) {
    SDL_SetRenderDrawColor(ren_, c.r, c.g, c.b, c.a);
    SDL_FRect r{x, y, w, h};
    if (fill) SDL_RenderFillRect(ren_, &r);
    else      SDL_RenderRect(ren_, &r);
}

void Launcher::dButton(const std::string& lbl, FR& out,
                       float x, float y, float w, float h,
                       bool sel, bool hov) {
    out = {x, y, w, h};
    SDL_Color bg = sel ? (hov ? C_SEL_HOV : C_SEL)
                       : (hov ? C_BTN_HOV : C_BTN);
    dRect(x, y, w, h, bg);
    int tw, th;
    tSize(lbl, tw, th);
    dText(lbl, x + (w - tw) / 2, y + (h - th) / 2, C_TEXT);
}

void Launcher::dCheckbox(const std::string& lbl, FR& out,
                         float x, float y, bool chk) {
    out = {x, y, 320, CB_SZ + 4};
    bool hov = out.hit(mx_, my_);
    dRect(x, y + 1, CB_SZ, CB_SZ, chk ? C_SEL : C_PANEL);
    dRect(x, y + 1, CB_SZ, CB_SZ, hov ? C_INPUT_ACT : C_INPUT_BDR, false);
    if (chk) dText("X", x + 4, y + 1, C_TEXT);
    dText(lbl, x + CB_SZ + 8, y, C_TEXT);
}

// ============================================================================
// Render - draws every frame and caches element positions in L_
// ============================================================================
void Launcher::render() {
    SDL_SetRenderDrawColor(ren_, C_BG.r, C_BG.g, C_BG.b, 255);
    SDL_RenderClear(ren_);

    float cy = PAD;

    // ---- Title ----
    {
        int tw, th;
        tSize("Pokemon Save Tools", tw, th, fTitle_);
        dText("Pokemon Save Tools", (WIN_W - tw) / 2.0f, cy, C_TEXT, fTitle_);
        cy += th + 10;
    }

    // ---- Tool selector (2 rows x 3 cols) ----
    {
        const float bw = 230, bh = BTN_H, gx = 12, gy = 8;
        float sx = (WIN_W - (3 * bw + 2 * gx)) / 2.0f;
        for (int i = 0; i < N_TOOLS; ++i) {
            int c = i % 3, r = i / 3;
            float bx = sx + c * (bw + gx);
            float by = cy + r * (bh + gy);
            bool sel = (selTool_ == i);
            bool hov = FR{bx, by, bw, bh}.hit(mx_, my_);
            dButton(TOOLS[i].name, L_.toolBtn[i], bx, by, bw, bh, sel, hov);
        }
        cy += 2 * (bh + gy) + 4;
    }

    // ---- Tool description ----
    if (selTool_ >= 0)
        dText(TOOLS[selTool_].desc, PAD + 4, cy, C_DIMTEXT, fSmall_);
    else
        dText("Select a tool above to begin.", PAD + 4, cy, C_DIMTEXT, fSmall_);
    cy += 20;

    // ---- Separator ----
    dRect(PAD, cy, WIN_W - 2 * PAD, 1, C_SEP);
    cy += 12;

    if (selTool_ < 0) {
        SDL_RenderPresent(ren_);
        return;
    }
    const ToolDef& T = TOOLS[selTool_];

    // ---- File path input + Browse button ----
    {
        dText("File:", PAD, cy + 5, C_TEXT);
        const float browseW = 92, gap = 8;
        float ix = PAD + 60;
        float iw = WIN_W - ix - PAD - browseW - gap;
        L_.fileIn = {ix, cy, iw, INPUT_H};

        SDL_Color bdr = fileAct_ ? C_INPUT_ACT : C_INPUT_BDR;
        dRect(ix, cy, iw, INPUT_H, C_INPUT_BG);
        dRect(ix, cy, iw, INPUT_H, bdr, false);

        // Clip so text doesn't overflow the field
        SDL_Rect clip{(int)(ix + 4), (int)cy, (int)(iw - 8), (int)INPUT_H};
        SDL_SetRenderClipRect(ren_, &clip);

        float tx = ix + 6, ty = cy + (INPUT_H - FONT_SZ) / 2;
        int cwPx = 0, ch;
        if (!file_.empty()) {
            std::string bc = file_.substr(0, fileCur_);
            tSize(bc, cwPx, ch);
        }

        float maxVis = iw - 16;
        if (cwPx - fileScrX_ > (int)maxVis)
            fileScrX_ = cwPx - (int)maxVis + 20;
        if (cwPx - fileScrX_ < 0)
            fileScrX_ = cwPx;
        if (fileScrX_ < 0)
            fileScrX_ = 0;

        if (!file_.empty())
            dText(file_, tx - fileScrX_, ty, C_TEXT);
        else if (!fileAct_)
            dText("Enter file path or click Browse...", tx, ty, C_DIMTEXT);

        if (fileAct_ && (SDL_GetTicks() / 500) % 2 == 0) {
            float cx = tx + cwPx - fileScrX_;
            dRect(cx, cy + 4, 2, INPUT_H - 8, C_TEXT);
        }
        SDL_SetRenderClipRect(ren_, nullptr);

        // Browse button (to the right of the field)
        float bx = ix + iw + gap;
        bool bhov = FR{bx, cy, browseW, INPUT_H}.hit(mx_, my_);
        dButton("Browse", L_.browse, bx, cy, browseW, INPUT_H, false, bhov);
    }
    cy += INPUT_H + 12;

    // ---- Game selector ----
    L_.nGame = 0;
    if (T.needsGame && !T.games.empty()) {
        dText("Game:", PAD, cy + 4, C_TEXT);
        float gx = PAD + 60;
        const float bw = 105, bh = 28, gap = 6;
        int cols = 4;
        for (int i = 0; i < (int)T.games.size() && i < 16; ++i) {
            int c = i % cols, r = i / cols;
            float bxx = gx + c * (bw + gap);
            float byy = cy + r * (bh + gap);
            bool sel = (game_ == T.games[i]);
            bool hov = FR{bxx, byy, bw, bh}.hit(mx_, my_);
            dButton(T.games[i], L_.gameBtns[i], bxx, byy, bw, bh, sel, hov);
            L_.nGame = i + 1;
        }
        int rows = ((int)T.games.size() + cols - 1) / cols;
        cy += rows * (bh + gap) + 10;
    }

    // ---- Options header ----
    dText("Options:", PAD, cy, C_DIMTEXT, fSmall_);
    cy += 22;

    // Overwrite (-o) - always shown for selected tool
    dCheckbox("Overwrite original file (-o)", L_.cbOW, PAD + 10, cy, ow_);
    cy += 28;

    // Japanese (-j)
    L_.showJpn = T.hasJpn;
    if (T.hasJpn) {
        dCheckbox("Japanese offsets (-j)", L_.cbJpn, PAD + 10, cy, jpn_);
        cy += 28;
    }

    // Write mode (-w) - checksum only
    L_.showWr = T.hasWrite;
    if (T.hasWrite) {
        dCheckbox("Write checksums to file (-w)", L_.cbWr, PAD + 10, cy, wr_);
        cy += 28;
    }

    // Pokemon checksum mode (-p) - checksum only
    L_.showPkm = T.hasPokemon;
    if (T.hasPokemon) {
        dCheckbox("Pokemon checksum mode (-p)", L_.cbPkm, PAD + 10, cy, pkm_);
        cy += 28;
    }

    // Byte grouping - hex editor only
    L_.showGrp = T.hasGrouping;
    if (T.hasGrouping) {
        dText("Grouping:", PAD + 10, cy + 4, C_TEXT);
        float gx = PAD + 120;
        for (int i = 0; i < N_GRP; ++i) {
            float bx = gx + i * 52.0f;
            bool sel = (grp_ == GRP_VALS[i]);
            bool hov = FR{bx, cy, 42, 26}.hit(mx_, my_);
            dButton(std::to_string(GRP_VALS[i]), L_.grpBtn[i],
                    bx, cy, 42, 26, sel, hov);
        }
        cy += 34;
    }

    // Text encoding - hex editor only
    L_.showEnc = T.hasEncoding;
    if (T.hasEncoding) {
        dText("Encoding:", PAD + 10, cy + 4, C_TEXT);
        float gx = PAD + 120;
        for (int i = 0; i < N_ENC; ++i) {
            float bx = gx + i * 64.0f;
            bool sel = (enc_ == i);
            bool hov = FR{bx, cy, 54, 26}.hit(mx_, my_);
            dButton(ENC_NAMES[i], L_.encBtn[i],
                    bx, cy, 54, 26, sel, hov);
        }
        cy += 34;
    }

    cy += 10;

    // ---- Launch button ----
    {
        float bw = 220, bh = 42;
        float bx = (WIN_W - bw) / 2;
        L_.launch = {bx, cy, bw, bh};
        bool hov = L_.launch.hit(mx_, my_);
        dRect(bx, cy, bw, bh, hov ? C_LAUNCH_HOV : C_LAUNCH);
        int lw, lh;
        tSize("Launch", lw, lh);
        dText("Launch", bx + (bw - lw) / 2, cy + (bh - lh) / 2, C_TEXT);
        cy += bh + 12;
    }

    // ---- Error message ----
    if (!err_.empty()) {
        dText(err_, PAD, cy, C_ERR, fSmall_);
    }

    SDL_RenderPresent(ren_);
}

// ============================================================================
// Event handling - uses cached layout positions from last render()
// ============================================================================
void Launcher::onEvent(SDL_Event& ev) {
    if (ev.type == SDL_EVENT_QUIT) {
        running_ = false;
        return;
    }

    if (ev.type == SDL_EVENT_MOUSE_MOTION) {
        mx_ = ev.motion.x;
        my_ = ev.motion.y;
        return;
    }

    // ---- Mouse click ----
    if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN
        && ev.button.button == SDL_BUTTON_LEFT) {
        float mx = ev.button.x, my = ev.button.y;
        err_.clear();

        // Tool buttons
        for (int i = 0; i < N_TOOLS; ++i) {
            if (L_.toolBtn[i].hit(mx, my)) {
                if (selTool_ != i) {
                    selTool_ = i;
                    const auto& g = TOOLS[i].games;
                    if (!TOOLS[i].needsGame)
                        game_.clear();
                    else if (std::find(g.begin(), g.end(), game_) == g.end())
                        game_.clear();
                    if (!TOOLS[i].hasJpn)      jpn_ = false;
                    if (!TOOLS[i].hasWrite)     wr_  = false;
                    if (!TOOLS[i].hasPokemon)   pkm_ = false;
                    if (!TOOLS[i].hasGrouping)  grp_ = 1;
                    if (!TOOLS[i].hasEncoding)  enc_ = 0;
                }
                return;
            }
        }

        // Browse button -> native file dialog
        if (L_.browse.hit(mx, my)) {
            openFileDialog();
            return;
        }

        // File input field
        if (L_.fileIn.hit(mx, my)) {
            if (!fileAct_) {
                fileAct_ = true;
                SDL_StartTextInput(win_);
            }
            fileCur_ = (int)file_.size();
            return;
        } else if (fileAct_) {
            fileAct_ = false;
            SDL_StopTextInput(win_);
        }

        if (selTool_ < 0) return;

        // Game buttons
        {
            const auto& g = TOOLS[selTool_].games;
            for (int i = 0; i < L_.nGame; ++i) {
                if (L_.gameBtns[i].hit(mx, my)) {
                    game_ = g[i];
                    return;
                }
            }
        }

        // Checkboxes
        if (L_.cbOW.hit(mx, my))                          { ow_  = !ow_;  return; }
        if (L_.showJpn && L_.cbJpn.hit(mx, my))           { jpn_ = !jpn_; return; }
        if (L_.showWr  && L_.cbWr.hit(mx, my))            { wr_  = !wr_;  return; }
        if (L_.showPkm && L_.cbPkm.hit(mx, my))           { pkm_ = !pkm_; return; }

        // Grouping buttons
        if (L_.showGrp) {
            for (int i = 0; i < N_GRP; ++i)
                if (L_.grpBtn[i].hit(mx, my)) { grp_ = GRP_VALS[i]; return; }
        }

        // Encoding buttons
        if (L_.showEnc) {
            for (int i = 0; i < N_ENC; ++i)
                if (L_.encBtn[i].hit(mx, my)) { enc_ = i; return; }
        }

        // Launch button
        if (L_.launch.hit(mx, my)) {
            launchTool();
            return;
        }
    }

    // ---- Text input (file path field) ----
    if (fileAct_) {
        if (ev.type == SDL_EVENT_TEXT_INPUT) {
            file_.insert(fileCur_, ev.text.text);
            fileCur_ += (int)std::strlen(ev.text.text);
        }
        if (ev.type == SDL_EVENT_KEY_DOWN) {
            auto k = ev.key.key;
            if (k == SDLK_BACKSPACE && fileCur_ > 0) {
                file_.erase(--fileCur_, 1);
            } else if (k == SDLK_DELETE && fileCur_ < (int)file_.size()) {
                file_.erase(fileCur_, 1);
            } else if (k == SDLK_LEFT  && fileCur_ > 0) {
                --fileCur_;
            } else if (k == SDLK_RIGHT && fileCur_ < (int)file_.size()) {
                ++fileCur_;
            } else if (k == SDLK_HOME) {
                fileCur_ = 0;
            } else if (k == SDLK_END) {
                fileCur_ = (int)file_.size();
            } else if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
                fileAct_ = false;
                SDL_StopTextInput(win_);
            } else if (k == SDLK_ESCAPE) {
                fileAct_ = false;
                SDL_StopTextInput(win_);
            } else if (k == 'v'
                       && (ev.key.mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI))) {
                char* clip = SDL_GetClipboardText();
                if (clip && *clip) {
                    file_.insert(fileCur_, clip);
                    fileCur_ += (int)std::strlen(clip);
                }
                SDL_free(clip);
            }
        }
        return;   // swallow all events while typing
    }

    // ---- Global keys (when not editing the file field) ----
    if (ev.type == SDL_EVENT_KEY_DOWN) {
        auto k = ev.key.key;
        if (k == SDLK_ESCAPE || k == 'q')
            running_ = false;
        else if ((k == SDLK_RETURN || k == SDLK_KP_ENTER) && selTool_ >= 0)
            launchTool();
    }
}

// ============================================================================
// Native file dialog
// ============================================================================
void SDLCALL Launcher::onFileDialog(void* userdata,
                                    const char* const* filelist, int /*filter*/) {
    auto* self = static_cast<Launcher*>(userdata);
    if (!self) return;
    if (!filelist) return;       // error occurred (see SDL_GetError())
    if (!filelist[0]) return;    // user cancelled
    // The callback can run on a separate thread; hand the result back to the
    // main loop via an atomic flag rather than touching shared state directly.
    self->pendingPath_ = filelist[0];
    self->pendingReady_.store(true, std::memory_order_release);
}

void Launcher::openFileDialog() {
    if (fileAct_) { fileAct_ = false; SDL_StopTextInput(win_); }
    const char* loc = file_.empty() ? nullptr : file_.c_str();
    SDL_ShowOpenFileDialog(onFileDialog, this, win_,
                           kFileFilters, kNumFilters, loc, false);
}

// ============================================================================
// Tool launch
// Tears down the launcher window, runs the selected tool, then re-inits.
// Each tool creates (and destroys) its own SDL window inside init()/run().
// ============================================================================
bool Launcher::launchTool() {
    if (selTool_ < 0) {
        err_ = "Select a tool first.";
        return false;
    }
    if (file_.empty()) {
        err_ = "Enter a file path.";
        return false;
    }
    const ToolDef& T = TOOLS[selTool_];
    if (T.needsGame && game_.empty()) {
        err_ = "Select a game.";
        return false;
    }
    if (T.id == TID::CHECKSUM && ow_ && !wr_) {
        err_ = "Overwrite (-o) requires Write (-w) for the checksum tool.";
        return false;
    }

    if (fileAct_) {
        fileAct_ = false;
        SDL_StopTextInput(win_);
    }

    // Destroy launcher SDL context so the tool gets a clean slate
    cleanupSDL();

    bool ok = false;

    switch (T.id) {

    case TID::HEX: {
        HexEditor ed;
        if (!ed.init()) { err_ = "Hex Editor init failed."; break; }
        ed.setByteGrouping(grp_);
        if (enc_ > 0)
            ed.setTextEncoding(parseEncodingArg(ENC_NAMES[enc_]));
        ed.setOverwriteMode(ow_);
        if (!ed.loadFile(file_.c_str())) {
            err_ = "Failed to load: " + file_;
            break;
        }
        ed.run();
        ok = true;
    } break;

    case TID::CHECKSUM: {
        ChecksumCalculator calc;
        if (!calc.init()) { err_ = "Checksum init failed."; break; }
        if (!calc.loadFile(file_.c_str())) {
            err_ = "Failed to load: " + file_;
            break;
        }
        calc.setJapanese(jpn_);
        calc.setWriteMode(wr_);
        calc.setOverwriteMode(ow_);
        calc.setPokemonMode(pkm_);
        if (!calc.setGame(game_)) { err_ = "Invalid game: " + game_; break; }
        if (!calc.calculateChecksum()) { err_ = "Checksum calculation failed."; break; }
        calc.run();
        ok = true;
    } break;

    case TID::MIRAGE: {
        MirageIslandEditor ed;
        if (!ed.init()) { err_ = "Mirage Island init failed."; break; }
        if (!ed.loadFile(file_.c_str())) {
            err_ = "Failed to load: " + file_;
            break;
        }
        if (!ed.setGame(game_)) { err_ = "Invalid game: " + game_; break; }
        ed.setOverwriteMode(ow_);
        ed.execute();   // may fail; run() still shows the result
        ed.run();
        ok = true;
    } break;

    case TID::BAG: {
        PokemonBagEditor ed;
        ed.setJapanese(jpn_);
        ed.setOverwriteMode(ow_);
        if (!ed.loadFile(file_.c_str())) {
            err_ = "Failed to load: " + file_;
            break;
        }
        if (!ed.setGame(game_)) { err_ = "Invalid game: " + game_; break; }
        if (!ed.init()) { err_ = "Bag Editor init failed."; break; }
        ed.run();
        ed.cleanup();
        ok = true;
    } break;

    case TID::PARTY: {
        PokemonPartyEditor ed;
        ed.setJapanese(jpn_);
        ed.setOverwriteMode(ow_);
        if (!ed.loadFile(file_.c_str())) {
            err_ = "Failed to load: " + file_;
            break;
        }
        if (!ed.setGame(game_)) { err_ = "Invalid game: " + game_; break; }
        if (!ed.init()) { err_ = "Party Editor init failed."; break; }
        ed.run();
        ed.cleanup();
        ok = true;
    } break;

    case TID::TRAINER: {
        PokemonTrainerEditor ed;
        ed.setJapanese(jpn_);
        ed.setOverwriteMode(ow_);
        if (!ed.loadFile(file_.c_str())) {
            err_ = "Failed to load: " + file_;
            break;
        }
        if (!ed.setGame(game_)) { err_ = "Invalid game: " + game_; break; }
        if (!ed.init()) { err_ = "Trainer Editor init failed."; break; }
        ed.run();
        ed.cleanup();
        ok = true;
    } break;

    default:
        break;
    }

    if (!initSDL()) {
        running_ = false;
        return false;
    }
    return ok;
}

// ============================================================================
// Main loop
// ============================================================================
void Launcher::run() {
    while (running_) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev))
            onEvent(ev);

        // Apply a path chosen via the native Browse dialog.
        if (pendingReady_.load(std::memory_order_acquire)) {
            file_     = pendingPath_;
            fileCur_  = (int)file_.size();
            fileScrX_ = 0;
            err_.clear();
            pendingReady_.store(false, std::memory_order_release);
        }

        render();
        SDL_Delay(16);   // ~60 fps
    }
}

// ============================================================================
// Entry point
// ============================================================================
int main(int /*argc*/, char** /*argv*/) {
    Launcher app;
    if (!app.init()) return 1;
    app.run();
    app.cleanup();
    return 0;
}