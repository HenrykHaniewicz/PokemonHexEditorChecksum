#ifndef SDL_APP_BASE_H
#define SDL_APP_BASE_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <iostream>
#include <functional>

struct ColorScheme {
    SDL_Color background = {30, 30, 30, 255};
    SDL_Color headerBg = {45, 45, 45, 255};
    SDL_Color text = {200, 200, 200, 255};
    SDL_Color textDim = {100, 100, 100, 255};
    SDL_Color accent = {100, 149, 237, 255};
    SDL_Color success = {144, 238, 144, 255};
    SDL_Color warning = {255, 165, 0, 255};
    SDL_Color error = {255, 100, 100, 255};
    SDL_Color highlight = {255, 215, 0, 255};
    SDL_Color inputBg = {60, 60, 80, 255};
    SDL_Color selectedBg = {80, 80, 120, 255};
    SDL_Color buttonBg = {70, 70, 90, 255};
    SDL_Color buttonHover = {90, 90, 110, 255};
    SDL_Color buttonYes = {60, 150, 60, 255};
    SDL_Color buttonYesHover = {100, 200, 100, 255};
    SDL_Color buttonYesBorder = {150, 255, 150, 255};
    SDL_Color buttonNo = {150, 60, 60, 255};
    SDL_Color buttonNoHover = {200, 100, 100, 255};
    SDL_Color buttonNoBorder = {255, 150, 150, 255};
    SDL_Color dialogBg = {50, 50, 60, 255};
    SDL_Color dialogBorder = {100, 100, 120, 255};
    SDL_Color scrollbarBg = {50, 50, 50, 255};
    SDL_Color scrollbarFg = {100, 100, 100, 255};
    SDL_Color scrollbarHover = {130, 130, 130, 255};
};

// Scrollbar configuration and state
struct ScrollbarState {
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
    
    // Check if scrolling is needed
    bool canScroll() const { return totalItems > visibleItems; }
    
    // Get maximum scroll offset
    size_t maxOffset() const { 
        return (totalItems > visibleItems) ? (totalItems - visibleItems) : 0; 
    }
};

// Confirmation dialog configuration
struct ConfirmDialogConfig {
    std::string title = "WARNING";
    std::string message1;
    std::string message2;
    std::string yesText = "YES (Y)";
    std::string noText = "NO (N)";
    int dialogWidth = 500;
    int dialogHeight = 250;
};

class SDLAppBase {
protected:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    TTF_Font* largeFont;
    TTF_Font* regularFont;
    
    std::string windowTitle;
    int windowWidth;
    int windowHeight;
    int charWidth;
    int charHeight;
    
    bool running;
    bool needsRedraw;
    
    ColorScheme colors;

    // Quit confirmation
    bool confirmOnQuit;
    
    // Scrollbar state
    ScrollbarState scrollbar;
    
    // Font loading
    bool loadFonts(int normalSize = 14, int largeSize = 48);
    
    // Text rendering utilities
    void renderText(const std::string& text, int x, int y, SDL_Color color,
                   TTF_Font* f = nullptr, SDL_Renderer* targetRenderer = nullptr);
    void renderTextScaled(const std::string& text, int x, int y, SDL_Color color, float scale,
                   TTF_Font* f = nullptr, SDL_Renderer* targetRenderer = nullptr);
    void renderCenteredText(const std::string& text, int y, SDL_Color color, 
                           TTF_Font* f = nullptr, SDL_Renderer* targetRenderer = nullptr);
    void getTextSize(const std::string& text, int& w, int& h, TTF_Font* f = nullptr);
    void renderCenteredTextAt(const std::string& text, int x, int y, SDL_Color color,
                             TTF_Font* f = nullptr, SDL_Renderer* targetRenderer = nullptr);
    
    // Drawing utilities
    void renderButton(const SDL_Rect& rect, const std::string& text, bool hovered = false,
                     SDL_Renderer* targetRenderer = nullptr);
    void renderFilledRect(const SDL_Rect& rect, SDL_Color color, 
                         SDL_Renderer* targetRenderer = nullptr);
    void renderOutlineRect(const SDL_Rect& rect, SDL_Color color,
                          SDL_Renderer* targetRenderer = nullptr);
    void renderLine(int x1, int y1, int x2, int y2, SDL_Color color,
                   SDL_Renderer* targetRenderer = nullptr);
    
    // Scrollbar utilities
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
    
    // Confirmation dialog utilities
    bool showConfirmDialog(const ConfirmDialogConfig& config);

    // Dialog boxes
    bool showOverwriteConfirmDialog(const std::string& filename);
    bool showQuitConfirmDialog();
    
    // Utility functions
    bool isPointInRect(int x, int y, const SDL_Rect& rect);
    
    // For derived classes
    virtual void render() = 0;
    virtual void handleEvent(SDL_Event& event) = 0;
    virtual void onResize(int newWidth, int newHeight);
    virtual void update(float deltaTime);
    
public:
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