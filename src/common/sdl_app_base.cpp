#include "sdl_app_base.h"

SDLAppBase::SDLAppBase(const std::string& title, int width, int height)
    : window(nullptr), renderer(nullptr), font(nullptr), largeFont(nullptr), regularFont(nullptr),
      windowTitle(title),
      windowWidth(width), windowHeight(height),
      charWidth(0), charHeight(0),
      running(false), needsRedraw(true) {
}

SDLAppBase::~SDLAppBase() {
    cleanup();
}

bool SDLAppBase::loadFonts(int normalSize, int largeSize) {
    const char* fontPaths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        "/System/Library/Fonts/Menlo.ttc",
        "/System/Library/Fonts/Monaco.ttf",
        "/Library/Fonts/Courier New.ttf",
        "C:\\Windows\\Fonts\\consola.ttf",
        "C:\\Windows\\Fonts\\cour.ttf",
        "C:\\Windows\\Fonts\\lucon.ttf",
        nullptr
    };
    
    for (int i = 0; fontPaths[i] != nullptr; i++) {
        font = TTF_OpenFont(fontPaths[i], normalSize);
        if (font) {
            regularFont = font;
            if (largeSize > 0) {
                largeFont = TTF_OpenFont(fontPaths[i], largeSize);
            }
            break;
        }
    }
    
    if (!font) {
        std::cerr << "Failed to load any monospace font!" << std::endl;
        return false;
    }
    
    TTF_GetStringSize(font, "W", 0, &charWidth, &charHeight);
    return true;
}

bool SDLAppBase::init() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    if (!TTF_Init()) {
        std::cerr << "TTF init failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    if (!loadFonts()) {
        return false;
    }

    window = SDL_CreateWindow(windowTitle.c_str(),
                              windowWidth,
                              windowHeight,
                              SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_StartTextInput(window);
    
    return true;
}

void SDLAppBase::run() {
    running = true;
    SDL_Event event;
    
    Uint32 lastTime = SDL_GetTicks();
    
    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
                continue;
            }

            if (event.type == SDL_EVENT_WINDOW_RESIZED ||
                event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                windowWidth = event.window.data1;
                windowHeight = event.window.data2;
                onResize(windowWidth, windowHeight);
                continue;
            }

            if (event.type == SDL_EVENT_WINDOW_EXPOSED) {
                needsRedraw = true;
                continue;
            }
            
            handleEvent(event);
        }
        
        update(deltaTime);
        
        if (needsRedraw) {
            render();
            needsRedraw = false;
        } else {
            SDL_Delay(10);
        }
    }
}

void SDLAppBase::cleanup() {
    SDL_StopTextInput(window);
    if (largeFont) {
        TTF_CloseFont(largeFont);
        largeFont = nullptr;
    }
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
        regularFont = nullptr;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    TTF_Quit();
    SDL_Quit();
}

void SDLAppBase::setWindowTitle(const std::string& title) {
    windowTitle = title;
    if (window) {
        SDL_SetWindowTitle(window, title.c_str());
    }
}

void SDLAppBase::onResize(int /*newWidth*/, int /*newHeight*/) {
    needsRedraw = true;
}

void SDLAppBase::update(float deltaTime) {
    updateMomentumScroll(deltaTime);
}

// ============================================================================
// Text Rendering
// ============================================================================

void SDLAppBase::renderText(const std::string& text, int x, int y, SDL_Color color, 
                           TTF_Font* f, SDL_Renderer* targetRenderer) {
    if (text.empty()) return;
    if (!f) f = font;
    if (!targetRenderer) targetRenderer = renderer;
    
    SDL_Surface* surface = TTF_RenderText_Blended(f, text.c_str(), text.size(), color);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(targetRenderer, surface);
    if (!texture) {
        SDL_DestroySurface(surface);
        return;
    }

    SDL_FRect dst{
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(surface->w),
        static_cast<float>(surface->h)
    };

    SDL_RenderTexture(targetRenderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
}

void SDLAppBase::renderTextScaled(const std::string& text, int x, int y, SDL_Color color,
                                  float scale, TTF_Font* f, SDL_Renderer* targetRenderer) {
    if (text.empty()) return;
    if (!f) f = font;
    if (!targetRenderer) targetRenderer = renderer;
    
    SDL_Surface* surface = TTF_RenderText_Blended(f, text.c_str(), text.size(), color);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(targetRenderer, surface);
    if (!texture) {
        SDL_DestroySurface(surface);
        return;
    }
    
    SDL_FRect destRect{
        static_cast<float>(x),
        static_cast<float>(y),
        surface->w * scale,
        surface->h * scale
    };

    SDL_RenderTexture(targetRenderer, texture, nullptr, &destRect);
    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
}

void SDLAppBase::renderCenteredText(const std::string& text, int y, SDL_Color color, 
                                   TTF_Font* f, SDL_Renderer* targetRenderer) {
    if (!f) f = font;
    int w, h;
    TTF_GetStringSize(f, text.c_str(), 0, &w, &h);
    renderText(text, (windowWidth - w) / 2, y, color, f, targetRenderer);
}

void SDLAppBase::renderCenteredTextAt(const std::string& text, int x, int y, SDL_Color color,
                                     TTF_Font* f, SDL_Renderer* targetRenderer) {
    if (!f) f = font;
    int w, h;
    TTF_GetStringSize(f, text.c_str(), 0, &w, &h);
    renderText(text, x - w / 2, y - h / 2, color, f, targetRenderer);
}

void SDLAppBase::getTextSize(const std::string& text, int& w, int& h, TTF_Font* f) {
    if (!f) f = font;
    TTF_GetStringSize(f, text.c_str(), 0, &w, &h);
}

// ============================================================================
// Drawing Utilities
// ============================================================================

void SDLAppBase::renderButton(const SDL_Rect& rect, const std::string& text, bool hovered,
                              SDL_Renderer* targetRenderer) {
    if (!targetRenderer) targetRenderer = renderer;
    
    SDL_Color btnColor = hovered ? colors.buttonHover : colors.buttonBg;
    renderFilledRect(rect, btnColor, targetRenderer);
    renderOutlineRect(rect, colors.dialogBorder, targetRenderer);
    
    int textW, textH;
    getTextSize(text, textW, textH);
    int textX = rect.x + (rect.w - textW) / 2;
    int textY = rect.y + (rect.h - textH) / 2;
    renderText(text, textX, textY, colors.text, nullptr, targetRenderer);
}

void SDLAppBase::renderFilledRect(const SDL_Rect& rect, SDL_Color color, 
                                 SDL_Renderer* targetRenderer) {
    if (!targetRenderer) targetRenderer = renderer;
    SDL_SetRenderDrawColor(targetRenderer, color.r, color.g, color.b, color.a);

    SDL_FRect r{
        static_cast<float>(rect.x),
        static_cast<float>(rect.y),
        static_cast<float>(rect.w),
        static_cast<float>(rect.h)
    };
    SDL_RenderFillRect(targetRenderer, &r);
}

void SDLAppBase::renderOutlineRect(const SDL_Rect& rect, SDL_Color color,
                                  SDL_Renderer* targetRenderer) {
    if (!targetRenderer) targetRenderer = renderer;
    SDL_SetRenderDrawColor(targetRenderer, color.r, color.g, color.b, color.a);

    SDL_FRect r{
        static_cast<float>(rect.x),
        static_cast<float>(rect.y),
        static_cast<float>(rect.w),
        static_cast<float>(rect.h)
    };
    SDL_RenderRect(targetRenderer, &r);
}

void SDLAppBase::renderLine(int x1, int y1, int x2, int y2, SDL_Color color,
                           SDL_Renderer* targetRenderer) {
    if (!targetRenderer) targetRenderer = renderer;
    SDL_SetRenderDrawColor(targetRenderer, color.r, color.g, color.b, color.a);
    SDL_RenderLine(targetRenderer,
                   static_cast<float>(x1),
                   static_cast<float>(y1),
                   static_cast<float>(x2),
                   static_cast<float>(y2));
}

bool SDLAppBase::isPointInRect(int x, int y, const SDL_Rect& rect) {
    return x >= rect.x && x < rect.x + rect.w &&
           y >= rect.y && y < rect.y + rect.h;
}

// ============================================================================
// Scrollbar Utilities
// ============================================================================

void SDLAppBase::getScrollbarGeometry(int& sbX, int& sbY, int& sbHeight, 
                                       int& thumbY, int& thumbHeight) const {
    sbX = windowWidth - scrollbar.width;
    sbY = scrollbar.headerOffset;
    sbHeight = windowHeight - scrollbar.headerOffset;
    
    if (scrollbar.canScroll()) {
        float thumbRatio = static_cast<float>(scrollbar.visibleItems) / 
                          static_cast<float>(scrollbar.totalItems);
        thumbHeight = std::max(30, static_cast<int>(sbHeight * thumbRatio));
        float scrollRatio = static_cast<float>(scrollbar.offset) / 
                           static_cast<float>(scrollbar.maxOffset());
        thumbY = sbY + static_cast<int>((sbHeight - thumbHeight) * scrollRatio);
    } else {
        thumbHeight = sbHeight;
        thumbY = sbY;
    }
}

void SDLAppBase::renderScrollbar(SDL_Renderer* targetRenderer) {
    if (!targetRenderer) targetRenderer = renderer;
    
    int sbX, sbY, sbHeight, thumbY, thumbHeight;
    getScrollbarGeometry(sbX, sbY, sbHeight, thumbY, thumbHeight);
    
    SDL_Rect scrollBgRect = {sbX, sbY, scrollbar.width, sbHeight};
    renderFilledRect(scrollBgRect, colors.scrollbarBg, targetRenderer);
    
    SDL_Rect thumbRect = {sbX + 2, thumbY, scrollbar.width - 4, thumbHeight};
    SDL_Color thumbColor = scrollbar.dragging ? colors.scrollbarHover : colors.scrollbarFg;
    renderFilledRect(thumbRect, thumbColor, targetRenderer);
}

bool SDLAppBase::handleScrollbarClick(int x, int y) {
    int sbX, sbY, sbHeight, thumbY, thumbHeight;
    getScrollbarGeometry(sbX, sbY, sbHeight, thumbY, thumbHeight);
    
    // Check if click is in scrollbar area
    if (x < sbX || x >= sbX + scrollbar.width ||
        y < sbY || y >= sbY + sbHeight) {
        return false;
    }
    
    if (!scrollbar.canScroll()) return true;
    
    // Check if clicking on thumb
    if (y >= thumbY && y < thumbY + thumbHeight) {
        scrollbar.dragging = true;
        scrollbar.dragStartY = y;
        scrollbar.dragStartRatio = static_cast<float>(scrollbar.offset) / 
                                   static_cast<float>(scrollbar.maxOffset());
    } else {
        // Click on track - jump to position
        float clickRatio = static_cast<float>(y - sbY) / static_cast<float>(sbHeight);
        scrollToRatio(clickRatio);
    }
    
    needsRedraw = true;
    return true;
}

void SDLAppBase::handleScrollbarDrag(int y) {
    if (!scrollbar.dragging || !scrollbar.canScroll()) return;
    
    int sbX, sbY, sbHeight, thumbY, thumbHeight;
    getScrollbarGeometry(sbX, sbY, sbHeight, thumbY, thumbHeight);
    
    int deltaY = y - scrollbar.dragStartY;
    float deltaRatio = static_cast<float>(deltaY) / static_cast<float>(sbHeight - thumbHeight);
    float newRatio = scrollbar.dragStartRatio + deltaRatio;
    
    scrollToRatio(newRatio);
}

void SDLAppBase::handleScrollbarRelease() {
    if (scrollbar.dragging) {
        scrollbar.dragging = false;
        needsRedraw = true;
    }
}

void SDLAppBase::scrollBy(int64_t items) {
    if (!scrollbar.canScroll()) return;
    
    int64_t newOffset = static_cast<int64_t>(scrollbar.offset) + items;
    newOffset = std::max(static_cast<int64_t>(0), newOffset);
    newOffset = std::min(static_cast<int64_t>(scrollbar.maxOffset()), newOffset);
    
    if (static_cast<size_t>(newOffset) != scrollbar.offset) {
        scrollbar.offset = static_cast<size_t>(newOffset);
        needsRedraw = true;
    }
}

void SDLAppBase::scrollBySmooth(float items) {
    if (!scrollbar.canScroll()) return;
    
    scrollbar.accumulatedScroll += items;
    
    while (scrollbar.accumulatedScroll >= 1.0f) {
        if (scrollbar.offset < scrollbar.maxOffset()) {
            scrollbar.offset++;
            needsRedraw = true;
        }
        scrollbar.accumulatedScroll -= 1.0f;
    }
    while (scrollbar.accumulatedScroll <= -1.0f) {
        if (scrollbar.offset > 0) {
            scrollbar.offset--;
            needsRedraw = true;
        }
        scrollbar.accumulatedScroll += 1.0f;
    }
    
    // Clamp accumulated scroll at boundaries
    if (scrollbar.offset == 0 && scrollbar.accumulatedScroll < 0) {
        scrollbar.accumulatedScroll = 0;
    }
    if (scrollbar.offset >= scrollbar.maxOffset() && scrollbar.accumulatedScroll > 0) {
        scrollbar.accumulatedScroll = 0;
    }
}

void SDLAppBase::scrollToRatio(float ratio) {
    if (!scrollbar.canScroll()) return;
    
    ratio = std::max(0.0f, std::min(1.0f, ratio));
    size_t newOffset = static_cast<size_t>(ratio * static_cast<float>(scrollbar.maxOffset()));
    
    if (newOffset != scrollbar.offset) {
        scrollbar.offset = newOffset;
        needsRedraw = true;
    }
}

void SDLAppBase::addScrollVelocity(float amount, float maxVelocity) {
    scrollbar.velocity += amount;
    
    if (scrollbar.velocity > maxVelocity) scrollbar.velocity = maxVelocity;
    if (scrollbar.velocity < -maxVelocity) scrollbar.velocity = -maxVelocity;
}

void SDLAppBase::updateMomentumScroll(float deltaTime) {
    if (std::abs(scrollbar.velocity) > ScrollbarState::STOP_THRESHOLD) {
        float scrollDelta = scrollbar.velocity * deltaTime * 60.0f;
        scrollBySmooth(scrollDelta);
        scrollbar.velocity *= ScrollbarState::FRICTION;
        needsRedraw = true;
    } else if (scrollbar.velocity != 0) {
        scrollbar.velocity = 0;
        scrollbar.accumulatedScroll = 0;
    }
}

// ============================================================================
// Confirmation Dialog
// ============================================================================

bool SDLAppBase::showConfirmDialog(const ConfirmDialogConfig& config) {
    // Create a modal dialog window
    SDL_Window* dialogWindow = SDL_CreateWindow(
        "Confirm",
        config.dialogWidth,
        config.dialogHeight,
        0
    );
    
    if (!dialogWindow) {
        std::cerr << "Failed to create dialog window: " << SDL_GetError() << std::endl;
        return false;
    }
    
    SDL_Renderer* dialogRenderer = SDL_CreateRenderer(dialogWindow, nullptr);
    if (!dialogRenderer) {
        std::cerr << "Failed to create dialog renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(dialogWindow);
        return false;
    }
    
    int centerX = config.dialogWidth / 2;
    
    SDL_Rect yesButton = {centerX - 140, config.dialogHeight - 80, 120, 50};
    SDL_Rect noButton = {centerX + 20, config.dialogHeight - 80, 120, 50};
    
    bool dialogRunning = true;
    bool result = false;
    bool yesHover = false;
    bool noHover = false;
    
    while (dialogRunning) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                dialogRunning = false;
                result = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_Y) {
                    dialogRunning = false;
                    result = true;
                } else if (event.key.key == SDLK_N || event.key.key == SDLK_ESCAPE) {
                    dialogRunning = false;
                    result = false;
                }
            } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
                int mx = static_cast<int>(event.motion.x);
                int my = static_cast<int>(event.motion.y);
                yesHover = isPointInRect(mx, my, yesButton);
                noHover = isPointInRect(mx, my, noButton);
            } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    int mx = static_cast<int>(event.button.x);
                    int my = static_cast<int>(event.button.y);
                    
                    if (isPointInRect(mx, my, yesButton)) {
                        dialogRunning = false;
                        result = true;
                    } else if (isPointInRect(mx, my, noButton)) {
                        dialogRunning = false;
                        result = false;
                    }
                }
            }
        }
        
        // Render dialog
        SDL_SetRenderDrawColor(dialogRenderer, colors.dialogBg.r, colors.dialogBg.g, 
                              colors.dialogBg.b, 255);
        SDL_RenderClear(dialogRenderer);
        
        // Title
        int messageY = 40;
        renderCenteredTextAt(config.title, centerX, messageY, colors.error, largeFont, dialogRenderer);
        messageY += 50;
        
        // Messages
        if (!config.message1.empty()) {
            renderCenteredTextAt(config.message1, centerX, messageY, colors.text, 
                               regularFont, dialogRenderer);
            messageY += 25;
        }
        
        if (!config.message2.empty()) {
            renderCenteredTextAt(config.message2, centerX, messageY, colors.warning, 
                               regularFont, dialogRenderer);
        }
        
        // Yes button
        SDL_Color yesColor = yesHover ? colors.buttonYesHover : colors.buttonYes;
        renderFilledRect(yesButton, yesColor, dialogRenderer);
        renderOutlineRect(yesButton, colors.buttonYesBorder, dialogRenderer);
        
        int textW, textH;
        getTextSize(config.yesText, textW, textH);
        renderText(config.yesText, 
                  yesButton.x + (yesButton.w - textW) / 2, 
                  yesButton.y + (yesButton.h - textH) / 2, 
                  colors.text, regularFont, dialogRenderer);
        
        // No button
        SDL_Color noColor = noHover ? colors.buttonNoHover : colors.buttonNo;
        renderFilledRect(noButton, noColor, dialogRenderer);
        renderOutlineRect(noButton, colors.buttonNoBorder, dialogRenderer);
        
        getTextSize(config.noText, textW, textH);
        renderText(config.noText, 
                  noButton.x + (noButton.w - textW) / 2, 
                  noButton.y + (noButton.h - textH) / 2, 
                  colors.text, regularFont, dialogRenderer);
        
        SDL_RenderPresent(dialogRenderer);
        SDL_Delay(16);
    }
    
    SDL_DestroyRenderer(dialogRenderer);
    SDL_DestroyWindow(dialogWindow);
    
    return result;
}