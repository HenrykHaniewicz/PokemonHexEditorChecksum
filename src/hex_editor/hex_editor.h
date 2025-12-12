#ifndef HEX_EDITOR_H
#define HEX_EDITOR_H

#include "../common/sdl_app_base.h"
#include "../common/hex_utils.h"
#include "../encodings/text_encodings.h"
#include <string>
#include <set>
#include <vector>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

// ============================================================================
// Edit Action (for undo functionality)
// ============================================================================

struct EditAction {
    size_t index;
    char oldValue;
    char newValue;
};

// ============================================================================
// Hex Editor Class
// ============================================================================

class HexEditor : public SDLAppBase {
private:
    // ========================================================================
    // Constants
    // ========================================================================
    static const int ROW_SIZE = 16;
    
    static constexpr float MIN_ZOOM = 1.0f;
    static constexpr float MAX_ZOOM = 4.0f;
    static constexpr float ZOOM_STEP = 0.15f;
    static constexpr float ZOOM_SMOOTH_SPEED = 12.0f;
    static constexpr float AUTO_SCROLL_DELAY = 0.05f;

    // ========================================================================
    // File Data
    // ========================================================================
    std::string fileBuffer;
    std::string savedFileBuffer;
    std::string fileName;
    std::string baseFileName;
    size_t fileSize;
    
    // ========================================================================
    // Display Configuration
    // ========================================================================
    int headerHeight;
    int byteGrouping;
    TextEncoding textEncoding;
    
    // ========================================================================
    // Character Dimensions
    // ========================================================================
    // Base dimensions (at zoom 1.0)
    int baseCharWidth;
    int baseCharHeight;
    
    // Effective dimensions (scaled by zoom)
    int effectiveCharWidth;
    int effectiveCharHeight;
    int decodedCellWidth;

    // ========================================================================
    // Layout Positions (recalculated on zoom/resize)
    // ========================================================================
    int addressX;
    int hexX;
    int asciiX;
    int contentEndX;
    
    // ========================================================================
    // Zoom State
    // ========================================================================
    float zoomLevel;
    float targetZoomLevel;
    
    // ========================================================================
    // Input Mode State
    // ========================================================================
    bool gotoMode;
    std::string gotoAddressInput;
    
    bool searchMode;
    std::string searchInput;
    std::vector<size_t> searchMatches;
    size_t currentMatchIndex;
    
    // ========================================================================
    // Editing State
    // ========================================================================
    int64_t selectedByteIndex;
    std::string editBuffer;
    bool hasUnsavedChanges;
    std::set<size_t> modifiedBytes;
    std::vector<EditAction> undoStack;
    bool overwriteMode;

    // ========================================================================
    // Selection State
    // ========================================================================
    bool isSelecting;
    int64_t selectionStart;
    int64_t selectionEnd;
    
    // ========================================================================
    // UI State
    // ========================================================================
    SDL_Rect saveButtonRect;
    bool saveButtonHovered;
    
    // Auto-scroll during selection
    int autoScrollDirection;  // -1 = up, 0 = none, 1 = down
    float autoScrollTimer;

    // ========================================================================
    // Layout Methods
    // ========================================================================
    void recalculateLayoutForZoom();
    int getByteXPosition(int byteInRow) const;
    int getByteIndexFromPosition(int x, int y) const;
    bool isJapaneseEncoding() const;
    
    // ========================================================================
    // Zoom Methods
    // ========================================================================
    void setZoom(float zoom);
    void adjustZoom(float delta);
    float calculateMaxZoom() const;
    
    // ========================================================================
    // Navigation Methods
    // ========================================================================
    void scrollToAddress(size_t address);
    void selectByte(int64_t index);
    
    // ========================================================================
    // Selection Methods
    // ========================================================================
    void clearSelection();
    bool hasSelectionRange() const;
    void getSelectionRange(int64_t& start, int64_t& end) const;
    
    // ========================================================================
    // Editing Methods
    // ========================================================================
    void commitEdit();
    void handleEditInput(char c);
    void undoLastEdit();
    void updateModifiedState(size_t index);
    
    // ========================================================================
    // File Operations
    // ========================================================================
    bool fileExists(const std::string& path) const;
    std::string getOutputPath() const;
    bool saveFile();
    void updateWindowTitle();
    
    // ========================================================================
    // Clipboard Operations
    // ========================================================================
    void handleCopy();
    void handlePaste();
    void appendHexInput(const std::string& text);
    
    // ========================================================================
    // Search Methods
    // ========================================================================
    void updateSearchMatches();
    void gotoNextMatch();
    
    // ========================================================================
    // Text Analysis
    // ========================================================================
    bool containsJapaneseCharacters(const std::string& text) const;
    size_t getVisualCellCount(const std::string& text) const;
    
    // ========================================================================
    // Event Handlers
    // ========================================================================
    void handleTextInput(const char* text);
    void handleKeyDown(SDL_Keycode key, Uint16 mod);
    void handleGotoInput(SDL_Keycode key, Uint16 mod);
    void handleSearchInput(SDL_Keycode key, Uint16 mod);
    bool handleNavigationKey(SDL_Keycode key, Uint16 mod);
    void handleMouseDown(int x, int y);
    void handleMouseUp();
    void handleMouseMotion(int x, int y);
    void handleMouseWheel(SDL_MouseWheelEvent& wheel);
    
    // ========================================================================
    // Rendering Methods
    // ========================================================================
    void renderHeader();
    void renderDecodedContent(int y, size_t address, size_t bytesInRow);
    
protected:
    // ========================================================================
    // SDLAppBase Overrides
    // ========================================================================
    void render() override;
    void handleEvent(SDL_Event& event) override;
    void onResize(int newWidth, int newHeight) override;
    void update(float deltaTime) override;
    
public:
    // ========================================================================
    // Public Interface
    // ========================================================================
    HexEditor();
    
    // File operations
    bool loadFile(const char* filename);
    
    // Configuration
    void setOverwriteMode(bool overwrite);
    void setByteGrouping(int grouping);
    void setTextEncoding(TextEncoding encoding);
    
    // Batch operations
    bool applyBatchEdits(const std::vector<std::pair<size_t, std::vector<unsigned char>>>& edits);
    void runBatchSaveMode();
};

#endif // HEX_EDITOR_H