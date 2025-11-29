#ifndef HEX_EDITOR_H
#define HEX_EDITOR_H

#include "../common/sdl_app_base.h"
#include "../common/hex_utils.h"
#include "../encodings/encodings.h"
#include <string>
#include <set>
#include <vector>
#include <cmath>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

class HexEditor : public SDLAppBase {
private:
    // File data
    std::string fileBuffer;
    std::string fileName;
    std::string baseFileName;
    size_t fileSize;

    std::string savedFileBuffer;
    
    // Display settings
    static const int ROW_SIZE = 16;
    int headerHeight;
    int scrollbarWidth;
    int byteGrouping;
    
    // Text encoding
    TextEncoding textEncoding;
    
    // Base character dimensions (at zoom 1.0)
    int baseCharWidth;
    int baseCharHeight;
    
    // Effective (zoomed) character dimensions
    int effectiveCharWidth;
    int effectiveCharHeight;
    
    // Layout positions (recalculated on zoom)
    int addressX;
    int hexX;
    int asciiX;
    int contentEndX;
    
    // Scrolling
    size_t scrollOffset;
    size_t visibleRows;
    size_t totalRows;
    
    // Momentum scrolling
    float scrollVelocity;
    float accumulatedScroll;
    static constexpr float SCROLL_FRICTION = 0.92f;
    static constexpr float SCROLL_STOP_THRESHOLD = 0.1f;
    
    // Scrollbar dragging
    bool draggingScrollbar;
    int dragStartY;
    float dragStartRatio;
    
    // Zoom functionality
    float zoomLevel;
    float targetZoomLevel;
    static constexpr float MIN_ZOOM = 1.0f;
    static constexpr float MAX_ZOOM = 4.0f;
    static constexpr float ZOOM_STEP = 0.15f;
    static constexpr float ZOOM_SMOOTH_SPEED = 12.0f;
    
    // Input state
    bool gotoMode;
    std::string gotoAddressInput;
    
    // Editing state
    int64_t selectedByteIndex;
    std::string editBuffer;
    bool hasUnsavedChanges;
    std::set<size_t> modifiedBytes;

    // Per-byte undo stack
    struct EditAction {
        size_t index;
        char oldValue;
        char newValue;
    };
    std::vector<EditAction> undoStack;
    
    // Save dialog
    bool showConfirmDialog;
    bool confirmOverwrite;
    SDL_Rect yesButtonRect;
    SDL_Rect noButtonRect;
    SDL_Rect saveButtonRect;

    bool overwriteMode;

    // Selection state (for multi-byte selection)
    bool isSelecting;
    int64_t selectionStart;
    int64_t selectionEnd;
    
    // Internal methods
    void updateLayout();
    void updateWindowTitle();
    void recalculateLayoutForZoom();
    int getByteXPosition(int byteInRow);
    int getHexSectionWidth();
    void renderHeader();
    void renderConfirmationDialog();
    void getScrollbarGeometry(int& sbX, int& sbY, int& sbHeight, int& thumbY, int& thumbHeight);
    int getByteIndexFromPosition(int x, int y);
    
    void scrollBy(int64_t rows);
    void scrollBySmooth(float rows);
    void scrollToRatio(float ratio);
    void scrollToAddress(size_t address);
    
    void selectByte(int64_t index);
    void commitEdit();
    void handleEditInput(char c);

    void handleCopy();
    void clearSelection();
    bool hasSelection() const;
    void getSelectionRange(int64_t& start, int64_t& end) const;

    void undoLastEdit();
    void updateModifiedState(size_t index);
    
    bool fileExists(const std::string& path);
    std::string getOutputPath();
    bool saveFile();
    
    void appendHexInput(const std::string& text);
    void handlePaste();
    void handleGotoInput(SDL_Keycode key, Uint16 mod);
    void handleTextInput(const char* text);
    void handleMouseDown(int x, int y);
    void handleMouseUp();
    void handleMouseMotion(int x, int y);
    void handleMouseWheel(SDL_MouseWheelEvent& wheel);
    void handleKeyDown(SDL_Keycode key, Uint16 mod);
    
    // Zoom methods
    void setZoom(float zoom);
    void adjustZoom(float delta);
    float calculateMaxZoom();
    
protected:
    void render() override;
    void handleEvent(SDL_Event& event) override;
    void onResize(int newWidth, int newHeight) override;
    void update(float deltaTime) override;
    
public:
    HexEditor();
    bool loadFile(const char* filename);
    void setOverwriteMode(bool overwrite);
    void setByteGrouping(int grouping);
    void setTextEncoding(TextEncoding encoding);
    
    // Batch mode methods
    bool applyBatchEdits(const std::vector<std::pair<size_t, std::vector<unsigned char>>>& edits);
    void runBatchSaveMode();
};

#endif // HEX_EDITOR_H