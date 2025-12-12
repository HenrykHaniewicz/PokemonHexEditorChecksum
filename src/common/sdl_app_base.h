#ifndef SDL_APP_BASE_H
#define SDL_APP_BASE_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <iostream>
#include <functional>

// ============================================================================
// Color Scheme
// ============================================================================

struct ColorScheme {
    // Background colors
    SDL_Color background = {30, 30, 30, 255};
    SDL_Color headerBg = {45, 45, 45, 255};
    SDL_Color dialogBg = {50, 50, 60, 255};
    SDL_Color inputBg = {60, 60, 80, 255};
    SDL_Color selectedBg = {80, 80, 120, 255};
    
    // Text colors
    SDL_Color text = {200, 200, 200, 255};
    SDL_Color textDim = {140, 140, 140, 255};
    SDL_Color accent = {100, 149, 237, 255};
    SDL_Color success = {144, 238, 144, 255};
    SDL_Color warning = {255, 165, 0, 255};
    SDL_Color error = {255, 100, 100, 255};
    SDL_Color highlight = {255, 215, 0, 255};
    
    // Button colors
    SDL_Color buttonBg = {70, 70, 90, 255};
    SDL_Color buttonHover = {90, 90, 110, 255};
    SDL_Color buttonYes = {60, 150, 60, 255};
    SDL_Color buttonYesHover = {100, 200, 100, 255};
    SDL_Color buttonYesBorder = {150, 255, 150, 255};
    SDL_Color buttonNo = {150, 60, 60, 255};
    SDL_Color buttonNoHover = {200, 100, 100, 255};
    SDL_Color buttonNoBorder = {255, 150, 150, 255};
    
    // Border and scrollbar colors
    SDL_Color dialogBorder = {100, 100, 120, 255};
    SDL_Color scrollbarBg = {50, 50, 50, 255};
    SDL_Color scrollbarFg = {100, 100, 100, 255};
    SDL_Color scrollbarHover = {130, 130, 130, 255};
};

// ============================================================================
// Scrollbar State
// ============================================================================

struct ScrollbarState {
    // Configuration
    int width = 14;
    int headerOffset = 0;
    
    // Scroll position
    size_t offset = 0;
    size_t visibleItems = 0;
    size_t totalItems = 0;
    
    // Momentum scrolling
    float velocity = 0.0f;
    float accumulatedScroll = 0.0f;
    static constexpr float FRICTION = 0.92f;
    static constexpr float STOP_THRESHOLD = 0.1f;
    
    // Dragging state
    bool dragging = false;
    int dragStartY = 0;
    float dragStartRatio = 0.0f;
    
    // Utility methods
    bool canScroll() const { return totalItems > visibleItems; }
    size_t maxOffset() const { 
        return (totalItems > visibleItems) ? (totalItems - visibleItems) : 0; 
    }
};

// ============================================================================
// Confirmation Dialog Configuration
// ============================================================================

struct ConfirmDialogConfig {
    std::string title = "WARNING";
    std::string message1;
    std::string message2;
    std::string yesText = "YES (Y)";
    std::string noText = "NO (N)";
    int dialogWidth = 500;
    int dialogHeight = 250;
};

// ============================================================================
// UTF-8 Character Information
// ============================================================================

struct UTF8CharInfo {
    size_t byteLength;      // Number of bytes in this UTF-8 character
    bool isMultiByte;       // True if non-ASCII (likely Japanese/CJK)
    bool isCombiningMark;   // True if this is a combining dakuten/handakuten
    bool hasFollowingCombiningMark;  // True if next char is a combining mark
    size_t totalLength;     // Total bytes including any following combining mark
};

// ============================================================================
// SDL Application Base Class
// ============================================================================

class SDLAppBase {
protected:
    // ========================================================================
    // Core SDL Objects
    // ========================================================================
    SDL_Window* window;
    SDL_Renderer* renderer;
    
    // ========================================================================
    // Font Management
    // ========================================================================
    TTF_Font* font;
    TTF_Font* largeFont;
    TTF_Font* regularFont;
    TTF_Font* japaneseFont;
    
    std::string currentFontPath;
    std::string currentJapaneseFontPath;
    
    // ========================================================================
    // Window Properties
    // ========================================================================
    std::string windowTitle;
    int windowWidth;
    int windowHeight;
    
    // ========================================================================
    // Character Metrics
    // ========================================================================
    int charWidth;
    int charHeight;
    int japaneseCharWidth;
    
    // ========================================================================
    // Application State
    // ========================================================================
    bool running;
    bool needsRedraw;
    bool confirmOnQuit;
    
    // ========================================================================
    // Visual Configuration
    // ========================================================================
    ColorScheme colors;
    ScrollbarState scrollbar;
    
    // ========================================================================
    // Font Loading Methods
    // ========================================================================
    bool loadFonts(int normalSize = 14, int largeSize = 48);
    bool loadJapaneseFont(int size = 14);
    TTF_Font* loadScaledFont(int size);
    TTF_Font* loadScaledJapaneseFont(int size);
    
    // ========================================================================
    // UTF-8 Text Analysis
    // ========================================================================
    static size_t getUTF8CharLength(unsigned char firstByte);
    static bool isCombiningDakuten(const std::string& text, size_t pos);
    UTF8CharInfo analyzeUTF8Char(const std::string& text, size_t pos) const;
    
    // ========================================================================
    // Basic Text Rendering
    // ========================================================================
    void renderText(const std::string& text, int x, int y, SDL_Color color,
                   TTF_Font* f = nullptr, SDL_Renderer* targetRenderer = nullptr);
    void renderTextScaled(const std::string& text, int x, int y, SDL_Color color, 
                         float scale, TTF_Font* f = nullptr, 
                         SDL_Renderer* targetRenderer = nullptr);
    void renderCenteredText(const std::string& text, int y, SDL_Color color, 
                           TTF_Font* f = nullptr, SDL_Renderer* targetRenderer = nullptr);
    void renderCenteredTextAt(const std::string& text, int x, int y, SDL_Color color,
                             TTF_Font* f = nullptr, SDL_Renderer* targetRenderer = nullptr);
    void getTextSize(const std::string& text, int& w, int& h, TTF_Font* f = nullptr);
    
    // ========================================================================
    // Mixed Text Rendering (Latin + Japanese)
    // ========================================================================
    void renderMixedText(const std::string& text, int x, int y, SDL_Color color, 
                        TTF_Font* latinFont = nullptr, 
                        SDL_Renderer* targetRenderer = nullptr);
    void renderMixedTextScaled(const std::string& text, int x, int y, SDL_Color color, 
                              float scale, TTF_Font* latinFont = nullptr, 
                              SDL_Renderer* targetRenderer = nullptr);
    void renderMixedTextScaledViaTexture(const std::string& text, int x, int y, 
                                         SDL_Color color, float scale, 
                                         TTF_Font* latinFont, 
                                         SDL_Renderer* targetRenderer);
    
    // ========================================================================
    // Cell-Based Text Rendering (Fixed-Width Cells)
    // ========================================================================
    void renderMixedTextWithCellWidth(const std::string& text, int x, int y, 
                                      SDL_Color color, int cellWidth, 
                                      TTF_Font* latinFont = nullptr,
                                      SDL_Renderer* targetRenderer = nullptr);
    void renderMixedTextScaledWithCellWidth(const std::string& text, int x, int y, 
                                            SDL_Color color, float scale, 
                                            int baseCellWidth, 
                                            TTF_Font* latinFont = nullptr,
                                            SDL_Renderer* targetRenderer = nullptr);
    
    // ========================================================================
    // Character Metrics Access
    // ========================================================================
    int getJapaneseCharWidth() const { return japaneseCharWidth; }
    
    // ========================================================================
    // Drawing Primitives
    // ========================================================================
    void renderFilledRect(const SDL_Rect& rect, SDL_Color color, 
                         SDL_Renderer* targetRenderer = nullptr);
    void renderOutlineRect(const SDL_Rect& rect, SDL_Color color,
                          SDL_Renderer* targetRenderer = nullptr);
    void renderLine(int x1, int y1, int x2, int y2, SDL_Color color,
                   SDL_Renderer* targetRenderer = nullptr);
    void renderButton(const SDL_Rect& rect, const std::string& text, 
                     bool hovered = false, SDL_Renderer* targetRenderer = nullptr);
    
    // ========================================================================
    // Scrollbar Management
    // ========================================================================
    void getScrollbarGeometry(int& sbX, int& sbY, int& sbHeight, 
                              int& thumbY, int& thumbHeight) const;
    void renderScrollbar(SDL_Renderer* targetRenderer = nullptr);
    bool handleScrollbarClick(int x, int y);
    void handleScrollbarDrag(int y);
    void handleScrollbarRelease();
    void scrollBy(int64_t items);
    void scrollBySmooth(float items);
    void scrollToRatio(float ratio);
    void addScrollVelocity(float amount, float maxVelocity = 50.0f);
    void updateMomentumScroll(float deltaTime);
    
    // ========================================================================
    // Dialog Management
    // ========================================================================
    bool showConfirmDialog(const ConfirmDialogConfig& config);
    bool showOverwriteConfirmDialog(const std::string& filename);
    bool showQuitConfirmDialog();
    
    // ========================================================================
    // Utility Methods
    // ========================================================================
    bool isPointInRect(int x, int y, const SDL_Rect& rect);
    
    // ========================================================================
    // Virtual Methods for Derived Classes
    // ========================================================================
    virtual void render() = 0;
    virtual void handleEvent(SDL_Event& event) = 0;
    virtual void onResize(int newWidth, int newHeight);
    virtual void update(float deltaTime);
    
public:
    // ========================================================================
    // Public Interface
    // ========================================================================
    SDLAppBase(const std::string& title, int width, int height);
    virtual ~SDLAppBase();
    
    bool init();
    void run();
    void cleanup();
    
    void setWindowTitle(const std::string& title);
    void requestRedraw() { needsRedraw = true; }
    void setConfirmOnQuit(bool confirm) { confirmOnQuit = confirm; }
    void quit() { running = false; }
};

#endif // SDL_APP_BASE_H