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
    
    // SDL3_ttf equivalent of TTF_SizeText, length 0 => null-terminated string
    TTF_GetStringSize(font, "W", 0, &charWidth, &charHeight);
    return true;
}


bool SDLAppBase::init() {
    // SDL_Init now returns bool: true on success, false on failure (SDL3)
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // TTF_Init also returns bool in SDL3_ttf
    if (!TTF_Init()) {
        std::cerr << "TTF init failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    if (!loadFonts()) {
        return false;
    }

    // SDL_CreateWindow no longer takes x/y positions, and SDL_WINDOW_SHOWN is gone
    window = SDL_CreateWindow(windowTitle.c_str(),
                              windowWidth,
                              windowHeight,
                              SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return false;
    }

    // SDL_CreateRenderer now takes (window, const char* name)
    // pass nullptr to let SDL choose a driver, flags parameter is gone
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

            // SDL_WINDOWEVENT* moved to top-level event types in SDL3
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
    // Base implementation just requests redraw
    // Derived classes can override to do layout calculations
    needsRedraw = true;
}

void SDLAppBase::update(float /*deltaTime*/) {
    // Derived classes can override
}

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