#include "sdl_app_base.h"
#include <cmath>
#include <algorithm>

// ============================================================================
// Constructor / Destructor
// ============================================================================

SDLAppBase::SDLAppBase(const std::string& title, int width, int height)
    : window(nullptr)
    , renderer(nullptr)
    , font(nullptr)
    , largeFont(nullptr)
    , regularFont(nullptr)
    , japaneseFont(nullptr)
    , windowTitle(title)
    , windowWidth(width)
    , windowHeight(height)
    , charWidth(0)
    , charHeight(0)
    , japaneseCharWidth(0)
    , running(false)
    , needsRedraw(true)
    , confirmOnQuit(false) {
}

SDLAppBase::~SDLAppBase() {
    cleanup();
}

// ============================================================================
// Initialization and Lifecycle
// ============================================================================

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

    window = SDL_CreateWindow(windowTitle.c_str(), windowWidth, windowHeight,
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
                if (confirmOnQuit) {
                    if (showQuitConfirmDialog()) {
                        running = false;
                    }
                } else {
                    running = false;
                }
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
    
    if (japaneseFont) {
        TTF_CloseFont(japaneseFont);
        japaneseFont = nullptr;
    }
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
// Font Loading
// ============================================================================

bool SDLAppBase::loadFonts(int normalSize, int largeSize) {
    const char* fontPaths[] = {
        // Linux
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        // macOS
        "/System/Library/Fonts/Menlo.ttc",
        "/System/Library/Fonts/Monaco.ttf",
        "/Library/Fonts/Courier New.ttf",
        // Windows
        "C:\\Windows\\Fonts\\consola.ttf",
        "C:\\Windows\\Fonts\\cour.ttf",
        "C:\\Windows\\Fonts\\lucon.ttf",
        nullptr
    };
    
    for (int i = 0; fontPaths[i] != nullptr; i++) {
        font = TTF_OpenFont(fontPaths[i], normalSize);
        if (font) {
            currentFontPath = fontPaths[i];
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
    
    // Also load Japanese font (not critical if it fails)
    loadJapaneseFont(normalSize);
    
    return true;
}

bool SDLAppBase::loadJapaneseFont(int size) {
    const char* japaneseFontPaths[] = {
        // macOS
        "/System/Library/Fonts/ヒラギノ丸ゴ ProN W4.ttc",
        "/System/Library/Fonts/Hiragino Sans GB.ttc",
        // Linux
        "/usr/share/fonts/truetype/fonts-japanese-gothic.ttf",
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        nullptr
    };
    
    for (int i = 0; japaneseFontPaths[i] != nullptr; i++) {
        japaneseFont = TTF_OpenFont(japaneseFontPaths[i], size);
        if (japaneseFont) {
            currentJapaneseFontPath = japaneseFontPaths[i];
            break;
        }
    }
    
    if (japaneseFont) {
        // Measure a typical Japanese character to get the width
        int w, h;
        TTF_GetStringSize(japaneseFont, "あ", 0, &w, &h);
        japaneseCharWidth = w;
    }
    
    return japaneseFont != nullptr;
}

TTF_Font* SDLAppBase::loadScaledFont(int size) {
    if (currentFontPath.empty()) return nullptr;
    return TTF_OpenFont(currentFontPath.c_str(), size);
}

TTF_Font* SDLAppBase::loadScaledJapaneseFont(int size) {
    if (currentJapaneseFontPath.empty()) return nullptr;
    return TTF_OpenFont(currentJapaneseFontPath.c_str(), size);
}

// ============================================================================
// UTF-8 Text Analysis
// ============================================================================

size_t SDLAppBase::getUTF8CharLength(unsigned char firstByte) {
    if ((firstByte & 0x80) == 0) return 1;        // ASCII
    if ((firstByte & 0xE0) == 0xC0) return 2;     // 2-byte UTF-8
    if ((firstByte & 0xF0) == 0xE0) return 3;     // 3-byte UTF-8
    if ((firstByte & 0xF8) == 0xF0) return 4;     // 4-byte UTF-8
    return 1;  // Invalid, treat as single byte
}

bool SDLAppBase::isCombiningDakuten(const std::string& text, size_t pos) {
    // Check if we have enough bytes for a 3-byte UTF-8 character
    if (pos + 2 >= text.length()) return false;
    
    unsigned char b1 = static_cast<unsigned char>(text[pos]);
    unsigned char b2 = static_cast<unsigned char>(text[pos + 1]);
    unsigned char b3 = static_cast<unsigned char>(text[pos + 2]);
    
    // Check for combining dakuten (U+3099) or combining handakuten (U+309A)
    // UTF-8: E3 82 99 (゙) and E3 82 9A (゚)
    // Note: U+309B (゛) and U+309C (゜) are STANDALONE, not combining
    if (b1 == 0xE3 && b2 == 0x82 && (b3 == 0x99 || b3 == 0x9A)) {
        return true;
    }
    
    return false;
}

UTF8CharInfo SDLAppBase::analyzeUTF8Char(const std::string& text, size_t pos) const {
    UTF8CharInfo info = {1, false, false, false, 1};
    
    if (pos >= text.length()) return info;
    
    unsigned char firstByte = static_cast<unsigned char>(text[pos]);
    info.byteLength = getUTF8CharLength(firstByte);
    info.isMultiByte = (info.byteLength > 1);
    info.totalLength = info.byteLength;
    
    // Check if this character itself is a combining mark
    // Only check if it's a 3-byte character and not at the start
    if (info.byteLength == 3 && pos > 0) {
        if (isCombiningDakuten(text, pos)) {
            // Verify there's a valid base character before this
            size_t prevPos = pos - 1;
            while (prevPos > 0 && (text[prevPos] & 0xC0) == 0x80) {
                prevPos--;
            }
            char prevChar = text[prevPos];
            if (prevChar != ' ' && prevChar != '.') {
                info.isCombiningMark = true;
            }
        }
    }
    
    // Check if the next character is a combining mark that should attach to this one
    if (!info.isCombiningMark) {
        size_t nextPos = pos + info.byteLength;
        if (nextPos < text.length() && isCombiningDakuten(text, nextPos)) {
            info.hasFollowingCombiningMark = true;
            info.totalLength += 3;  // Combining marks are 3 bytes
        }
    }
    
    return info;
}

// ============================================================================
// Basic Text Rendering
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
// Mixed Text Rendering (Latin + Japanese)
// ============================================================================

void SDLAppBase::renderMixedText(const std::string& text, int x, int y, SDL_Color color, 
                                 TTF_Font* latinFont, SDL_Renderer* targetRenderer) {
    if (text.empty()) return;
    if (!latinFont) latinFont = font;
    if (!targetRenderer) targetRenderer = renderer;
    
    // If no Japanese font, fall back to regular rendering
    if (!japaneseFont) {
        renderText(text, x, y, color, latinFont, targetRenderer);
        return;
    }
    
    int currentX = x;
    size_t i = 0;
    
    while (i < text.length()) {
        UTF8CharInfo charInfo = analyzeUTF8Char(text, i);
        std::string ch = text.substr(i, charInfo.byteLength);
        
        TTF_Font* fontToUse = charInfo.isMultiByte ? japaneseFont : latinFont;
        
        SDL_Surface* surface = TTF_RenderText_Blended(fontToUse, ch.c_str(), ch.size(), color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(targetRenderer, surface);
            if (texture) {
                SDL_FRect dst{
                    static_cast<float>(currentX),
                    static_cast<float>(y),
                    static_cast<float>(surface->w),
                    static_cast<float>(surface->h)
                };
                SDL_RenderTexture(targetRenderer, texture, nullptr, &dst);
                currentX += surface->w;
                SDL_DestroyTexture(texture);
            }
            SDL_DestroySurface(surface);
        }
        
        i += charInfo.byteLength;
    }
}

void SDLAppBase::renderMixedTextScaled(const std::string& text, int x, int y, SDL_Color color, 
                                       float scale, TTF_Font* latinFont, 
                                       SDL_Renderer* targetRenderer) {
    if (text.empty()) return;
    if (!latinFont) latinFont = font;
    if (!targetRenderer) targetRenderer = renderer;
    
    // If no Japanese font or scale is 1.0, fall back to regular mixed text rendering
    if (!japaneseFont || std::abs(scale - 1.0f) < 0.001f) {
        renderMixedText(text, x, y, color, latinFont, targetRenderer);
        return;
    }
    
    int baseFontSize = TTF_GetFontSize(latinFont);
    int scaledFontSize = static_cast<int>(baseFontSize * scale);
    
    TTF_Font* scaledLatinFont = loadScaledFont(scaledFontSize);
    TTF_Font* scaledJapaneseFont = loadScaledJapaneseFont(scaledFontSize);
    
    // If we couldn't load scaled fonts, fall back to texture scaling
    if (!scaledLatinFont || !scaledJapaneseFont) {
        if (scaledLatinFont) TTF_CloseFont(scaledLatinFont);
        if (scaledJapaneseFont) TTF_CloseFont(scaledJapaneseFont);
        renderMixedTextScaledViaTexture(text, x, y, color, scale, latinFont, targetRenderer);
        return;
    }
    
    int currentX = x;
    size_t i = 0;
    
    while (i < text.length()) {
        UTF8CharInfo charInfo = analyzeUTF8Char(text, i);
        std::string ch = text.substr(i, charInfo.byteLength);
        
        TTF_Font* fontToUse = charInfo.isMultiByte ? scaledJapaneseFont : scaledLatinFont;
        
        SDL_Surface* surface = TTF_RenderText_Blended(fontToUse, ch.c_str(), ch.size(), color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(targetRenderer, surface);
            if (texture) {
                SDL_FRect dst{
                    static_cast<float>(currentX),
                    static_cast<float>(y),
                    static_cast<float>(surface->w),
                    static_cast<float>(surface->h)
                };
                SDL_RenderTexture(targetRenderer, texture, nullptr, &dst);
                currentX += surface->w;
                SDL_DestroyTexture(texture);
            }
            SDL_DestroySurface(surface);
        }
        
        i += charInfo.byteLength;
    }
    
    TTF_CloseFont(scaledLatinFont);
    TTF_CloseFont(scaledJapaneseFont);
}

void SDLAppBase::renderMixedTextScaledViaTexture(const std::string& text, int x, int y, 
                                                  SDL_Color color, float scale, 
                                                  TTF_Font* latinFont, 
                                                  SDL_Renderer* targetRenderer) {
    if (!latinFont) latinFont = font;
    if (!targetRenderer) targetRenderer = renderer;
    
    // Calculate total size needed
    int totalWidth = 0;
    int maxHeight = 0;
    
    size_t i = 0;
    while (i < text.length()) {
        UTF8CharInfo charInfo = analyzeUTF8Char(text, i);
        std::string ch = text.substr(i, charInfo.byteLength);
        TTF_Font* fontToUse = (charInfo.isMultiByte && japaneseFont) ? japaneseFont : latinFont;
        
        int w, h;
        TTF_GetStringSize(fontToUse, ch.c_str(), ch.size(), &w, &h);
        totalWidth += w;
        maxHeight = std::max(maxHeight, h);
        
        i += charInfo.byteLength;
    }
    
    // Create a texture to render the text at normal size
    SDL_Texture* textTexture = SDL_CreateTexture(targetRenderer, 
                                                  SDL_PIXELFORMAT_RGBA32, 
                                                  SDL_TEXTUREACCESS_TARGET, 
                                                  totalWidth, maxHeight);
    if (!textTexture) {
        renderMixedText(text, x, y, color, latinFont, targetRenderer);
        return;
    }
    
    SDL_Texture* oldTarget = SDL_GetRenderTarget(targetRenderer);
    SDL_SetRenderTarget(targetRenderer, textTexture);
    SDL_SetTextureBlendMode(textTexture, SDL_BLENDMODE_BLEND);
    
    SDL_SetRenderDrawColor(targetRenderer, 0, 0, 0, 0);
    SDL_RenderClear(targetRenderer);
    
    // Render text to texture
    int currentX = 0;
    i = 0;
    
    while (i < text.length()) {
        UTF8CharInfo charInfo = analyzeUTF8Char(text, i);
        std::string ch = text.substr(i, charInfo.byteLength);
        TTF_Font* fontToUse = (charInfo.isMultiByte && japaneseFont) ? japaneseFont : latinFont;
        
        SDL_Surface* surface = TTF_RenderText_Blended(fontToUse, ch.c_str(), ch.size(), color);
        if (surface) {
            SDL_Texture* charTexture = SDL_CreateTextureFromSurface(targetRenderer, surface);
            if (charTexture) {
                SDL_FRect dst{
                    static_cast<float>(currentX),
                    0,
                    static_cast<float>(surface->w),
                    static_cast<float>(surface->h)
                };
                SDL_RenderTexture(targetRenderer, charTexture, nullptr, &dst);
                currentX += surface->w;
                SDL_DestroyTexture(charTexture);
            }
            SDL_DestroySurface(surface);
        }
        
        i += charInfo.byteLength;
    }
    
    SDL_SetRenderTarget(targetRenderer, oldTarget);
    
    SDL_FRect scaledDst{
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(totalWidth) * scale,
        static_cast<float>(maxHeight) * scale
    };
    
    SDL_RenderTexture(targetRenderer, textTexture, nullptr, &scaledDst);
    SDL_DestroyTexture(textTexture);
}

// ============================================================================
// Cell-Based Text Rendering (Fixed-Width Cells)
// ============================================================================

void SDLAppBase::renderMixedTextWithCellWidth(const std::string& text, int x, int y, 
                                               SDL_Color color, int cellWidth, 
                                               TTF_Font* latinFont,
                                               SDL_Renderer* targetRenderer) {
    if (text.empty()) return;
    if (!latinFont) latinFont = font;
    if (!targetRenderer) targetRenderer = renderer;
    
    int currentX = x;
    size_t i = 0;
    
    while (i < text.length()) {
        UTF8CharInfo charInfo = analyzeUTF8Char(text, i);
        
        // If this is a combining mark, move back one cell to overlay
        if (charInfo.isCombiningMark && currentX > x) {
            currentX -= cellWidth;
        }
        
        // Get the string to render (include following combining mark if present)
        std::string charToRender = text.substr(i, charInfo.totalLength);
        
        TTF_Font* fontToUse = (charInfo.isMultiByte && japaneseFont) ? japaneseFont : latinFont;
        
        int charW, charH;
        TTF_GetStringSize(fontToUse, charToRender.c_str(), charToRender.size(), &charW, &charH);
        
        // Center the character in the cell
        int offsetX = std::max(0, (cellWidth - charW) / 2);
        
        SDL_Surface* surface = TTF_RenderText_Blended(fontToUse, charToRender.c_str(), 
                                                      charToRender.size(), color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(targetRenderer, surface);
            if (texture) {
                SDL_FRect dst{
                    static_cast<float>(currentX + offsetX),
                    static_cast<float>(y),
                    static_cast<float>(surface->w),
                    static_cast<float>(surface->h)
                };
                SDL_RenderTexture(targetRenderer, texture, nullptr, &dst);
                SDL_DestroyTexture(texture);
            }
            SDL_DestroySurface(surface);
        }
        
        // Only advance the cell position if this wasn't a combining mark
        if (!charInfo.isCombiningMark) {
            currentX += cellWidth;
        }
        
        i += charInfo.totalLength;
    }
}

void SDLAppBase::renderMixedTextScaledWithCellWidth(const std::string& text, int x, int y, 
                                                     SDL_Color color, float scale, 
                                                     int baseCellWidth,
                                                     TTF_Font* latinFont, 
                                                     SDL_Renderer* targetRenderer) {
    if (text.empty()) return;
    if (!latinFont) latinFont = font;
    if (!targetRenderer) targetRenderer = renderer;
    
    // For scale ~1.0, use non-scaled version
    if (std::abs(scale - 1.0f) < 0.001f) {
        renderMixedTextWithCellWidth(text, x, y, color, baseCellWidth, latinFont, targetRenderer);
        return;
    }
    
    int scaledCellWidth = static_cast<int>(baseCellWidth * scale);
    int baseFontSize = TTF_GetFontSize(latinFont);
    int scaledFontSize = static_cast<int>(baseFontSize * scale);
    
    TTF_Font* scaledLatinFont = loadScaledFont(scaledFontSize);
    TTF_Font* scaledJapaneseFont = japaneseFont ? loadScaledJapaneseFont(scaledFontSize) : nullptr;
    
    // Fall back to non-scaled rendering if font loading fails
    if (!scaledLatinFont) {
        renderMixedTextWithCellWidth(text, x, y, color, scaledCellWidth, latinFont, targetRenderer);
        if (scaledJapaneseFont) TTF_CloseFont(scaledJapaneseFont);
        return;
    }
    
    int currentX = x;
    size_t i = 0;
    
    while (i < text.length()) {
        UTF8CharInfo charInfo = analyzeUTF8Char(text, i);
        
        // If this is a combining mark, move back one cell to overlay
        if (charInfo.isCombiningMark && currentX > x) {
            currentX -= scaledCellWidth;
        }
        
        std::string charToRender = text.substr(i, charInfo.totalLength);
        
        TTF_Font* fontToUse = (charInfo.isMultiByte && scaledJapaneseFont) 
                             ? scaledJapaneseFont : scaledLatinFont;
        
        int charW, charH;
        TTF_GetStringSize(fontToUse, charToRender.c_str(), charToRender.size(), &charW, &charH);
        
        int offsetX = std::max(0, (scaledCellWidth - charW) / 2);
        
        SDL_Surface* surface = TTF_RenderText_Blended(fontToUse, charToRender.c_str(), 
                                                      charToRender.size(), color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(targetRenderer, surface);
            if (texture) {
                SDL_FRect dst{
                    static_cast<float>(currentX + offsetX),
                    static_cast<float>(y),
                    static_cast<float>(surface->w),
                    static_cast<float>(surface->h)
                };
                SDL_RenderTexture(targetRenderer, texture, nullptr, &dst);
                SDL_DestroyTexture(texture);
            }
            SDL_DestroySurface(surface);
        }
        
        if (!charInfo.isCombiningMark) {
            currentX += scaledCellWidth;
        }
        
        i += charInfo.totalLength;
    }
    
    TTF_CloseFont(scaledLatinFont);
    if (scaledJapaneseFont) TTF_CloseFont(scaledJapaneseFont);
}

// ============================================================================
// Drawing Primitives
// ============================================================================

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

bool SDLAppBase::isPointInRect(int x, int y, const SDL_Rect& rect) {
    return x >= rect.x && x < rect.x + rect.w &&
           y >= rect.y && y < rect.y + rect.h;
}

// ============================================================================
// Scrollbar Management
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
    
    if (x < sbX || x >= sbX + scrollbar.width ||
        y < sbY || y >= sbY + sbHeight) {
        return false;
    }
    
    if (!scrollbar.canScroll()) return true;
    
    if (y >= thumbY && y < thumbY + thumbHeight) {
        scrollbar.dragging = true;
        scrollbar.dragStartY = y;
        scrollbar.dragStartRatio = static_cast<float>(scrollbar.offset) / 
                                   static_cast<float>(scrollbar.maxOffset());
    } else {
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
    scrollbar.velocity = std::max(-maxVelocity, std::min(maxVelocity, scrollbar.velocity));
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
// Dialog Management
// ============================================================================

bool SDLAppBase::showConfirmDialog(const ConfirmDialogConfig& config) {
    SDL_Window* dialogWindow = SDL_CreateWindow("Confirm",
                                                 config.dialogWidth,
                                                 config.dialogHeight,
                                                 0);
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
    
    SDL_RaiseWindow(dialogWindow);
    
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
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    dialogRunning = false;
                    result = false;
                    break;
                    
                case SDL_EVENT_KEY_DOWN:
                    if (event.key.key == SDLK_Y) {
                        dialogRunning = false;
                        result = true;
                    } else if (event.key.key == SDLK_N || event.key.key == SDLK_ESCAPE) {
                        dialogRunning = false;
                        result = false;
                    }
                    break;
                    
                case SDL_EVENT_MOUSE_MOTION: {
                    int mx = static_cast<int>(event.motion.x);
                    int my = static_cast<int>(event.motion.y);
                    yesHover = isPointInRect(mx, my, yesButton);
                    noHover = isPointInRect(mx, my, noButton);
                    break;
                }
                    
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
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
                    break;
                    
                case SDL_EVENT_WINDOW_FOCUS_LOST:
                    if (event.window.windowID == SDL_GetWindowID(dialogWindow)) {
                        SDL_RaiseWindow(dialogWindow);
                    }
                    break;
            }
        }
        
        // Render dialog
        SDL_SetRenderDrawColor(dialogRenderer, colors.dialogBg.r, colors.dialogBg.g, 
                              colors.dialogBg.b, 255);
        SDL_RenderClear(dialogRenderer);
        
        // Title
        int messageY = 40;
        renderCenteredTextAt(config.title, centerX, messageY, colors.error, 
                            largeFont, dialogRenderer);
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
    
    SDL_RaiseWindow(window);
    needsRedraw = true;
    
    return result;
}

bool SDLAppBase::showOverwriteConfirmDialog(const std::string& filename) {
    ConfirmDialogConfig config;
    config.message1 = "Overwrite this file?";
    config.message2 = filename;
    config.yesText = "YES (Y)";
    config.noText = "NO (N)";
    
    return showConfirmDialog(config);
}

bool SDLAppBase::showQuitConfirmDialog() {
    ConfirmDialogConfig config;
    config.message1 = "Are you sure you want to quit?";
    config.message2 = "Any unsaved changes will be lost.";
    config.yesText = "QUIT (Y)";
    config.noText = "CANCEL (N)";
    
    return showConfirmDialog(config);
}