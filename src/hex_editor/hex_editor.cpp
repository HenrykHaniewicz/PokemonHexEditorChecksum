#include "hex_editor.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

HexEditor::HexEditor() 
    : SDLAppBase("GBA/GB Hex Editor", 800, 700),
      fileSize(0),
      headerHeight(50),
      byteGrouping(1),
      textEncoding(TextEncoding::ASCII),
      baseCharWidth(0), baseCharHeight(0),
      effectiveCharWidth(0), effectiveCharHeight(0),
      addressX(10), hexX(0), asciiX(0), contentEndX(0),
      zoomLevel(1.0f), targetZoomLevel(1.0f),
      gotoMode(false),
      searchMode(false),
      currentMatchIndex(0),
      selectedByteIndex(-1), hasUnsavedChanges(false),
      overwriteMode(false),
      isSelecting(false), selectionStart(-1), selectionEnd(-1),
      saveButtonHovered(false),
      autoScrollDirection(0), autoScrollTimer(0.0f) {
}

void HexEditor::setTextEncoding(TextEncoding encoding) {
    textEncoding = encoding;
    needsRedraw = true;
}

void HexEditor::setOverwriteMode(bool overwrite) {
    overwriteMode = overwrite;
}

void HexEditor::setByteGrouping(int grouping) {
    if (grouping == 1 || grouping == 2 || grouping == 4 || grouping == 8) {
        byteGrouping = grouping;
        if (fileSize > 0) {
            recalculateLayoutForZoom();
        }
    }
}

float HexEditor::calculateMaxZoom() {
    int availableWidth = windowWidth - scrollbar.width - 20;
    
    int baseHexX = addressX + baseCharWidth * 10;
    int numGroups = 16 / byteGrouping;
    int baseHexWidth = numGroups * (byteGrouping * 2 + 1) * baseCharWidth + baseCharWidth;
    
    int baseAsciiX = baseHexX + baseHexWidth;
    int baseContentWidth = baseAsciiX + baseCharWidth * 16 + 10;
    
    float maxZoom = static_cast<float>(availableWidth) / static_cast<float>(baseContentWidth);
    
    return std::max(MIN_ZOOM, std::min(maxZoom, MAX_ZOOM));
}

void HexEditor::setZoom(float zoom) {
    float maxZoom = calculateMaxZoom();
    float newTarget = std::max(MIN_ZOOM, std::min(zoom, maxZoom));
    
    if (std::abs(newTarget - targetZoomLevel) > 0.001f) {
        targetZoomLevel = newTarget;
        needsRedraw = true;
    }
}

void HexEditor::adjustZoom(float delta) {
    setZoom(targetZoomLevel + delta);
}

void HexEditor::recalculateLayoutForZoom() {
    if (baseCharWidth == 0) {
        baseCharWidth = charWidth;
        baseCharHeight = charHeight;
    }
    
    effectiveCharWidth = static_cast<int>(baseCharWidth * zoomLevel);
    effectiveCharHeight = static_cast<int>(baseCharHeight * zoomLevel);
    
    addressX = 10;
    hexX = addressX + effectiveCharWidth * 10;
    
    int numGroups = 16 / byteGrouping;
    int hexSectionWidth = numGroups * (byteGrouping * 2 + 1) * effectiveCharWidth;
    hexSectionWidth += effectiveCharWidth;
    
    asciiX = hexX + hexSectionWidth;
    contentEndX = asciiX + effectiveCharWidth * 16 + 10;
    
    // Update scrollbar configuration
    scrollbar.headerOffset = headerHeight;
    int availableHeight = windowHeight - headerHeight - effectiveCharHeight - 20;
    scrollbar.visibleItems = std::max(1, availableHeight / effectiveCharHeight);
    scrollbar.totalItems = (fileSize + ROW_SIZE - 1) / ROW_SIZE;
    
    needsRedraw = true;
}

int HexEditor::getByteXPosition(int byteInRow) {
    int groupIndex = byteInRow / byteGrouping;
    int posInGroup = byteInRow % byteGrouping;
    
    int x = hexX;
    x += groupIndex * (byteGrouping * 2 + 1) * effectiveCharWidth;
    x += posInGroup * 2 * effectiveCharWidth;
    
    if (byteInRow >= 8) {
        x += effectiveCharWidth;
    }
    
    return x;
}

int HexEditor::getHexSectionWidth() {
    return asciiX - hexX;
}

bool HexEditor::loadFile(const char* filename) {
    if (!HexUtils::loadFileToBuffer(filename, fileBuffer, fileSize)) {
        std::cerr << "Failed to open: " << filename << std::endl;
        return false;
    }
    
    fileName = filename;
    baseFileName = HexUtils::getBaseName(fileName);

    savedFileBuffer = fileBuffer;
    undoStack.clear();
    
    scrollbar.totalItems = (fileSize + ROW_SIZE - 1) / ROW_SIZE;
    scrollbar.offset = 0;
    hasUnsavedChanges = false;
    modifiedBytes.clear();
    selectedByteIndex = -1;
    zoomLevel = 1.0f;
    targetZoomLevel = 1.0f;
    searchMode = false;
    searchInput.clear();
    searchMatches.clear();
    currentMatchIndex = 0;
    needsRedraw = true;
    
    baseCharWidth = charWidth;
    baseCharHeight = charHeight;
    
    recalculateLayoutForZoom();
    updateWindowTitle();
    setConfirmOnQuit(false);
    
    return true;
}

void HexEditor::updateWindowTitle() {
    std::string title = "Hex Editor - " + fileName;
    if (hasUnsavedChanges) {
        title += " *";
    }
    setWindowTitle(title);
}

void HexEditor::updateLayout() {
    recalculateLayoutForZoom();
}

void HexEditor::onResize(int /*newWidth*/, int /*newHeight*/) {
    recalculateLayoutForZoom();
}

int HexEditor::getByteIndexFromPosition(int x, int y) {
    int contentY = headerHeight + 5 + effectiveCharHeight;
    if (y < contentY) return -1;
    
    int row = (y - contentY) / effectiveCharHeight;
    if (row < 0 || static_cast<size_t>(row) >= scrollbar.visibleItems) return -1;
    
    size_t actualRow = scrollbar.offset + row;
    if (actualRow >= scrollbar.totalItems) return -1;
    
    if (x < hexX || x >= asciiX) return -1;
    
    for (int i = 0; i < ROW_SIZE; i++) {
        int byteX = getByteXPosition(i);
        int byteEndX = byteX + effectiveCharWidth * 2;
        if (x >= byteX && x < byteEndX) {
            size_t byteIndex = actualRow * ROW_SIZE + i;
            if (byteIndex >= fileSize) return -1;
            return static_cast<int>(byteIndex);
        }
    }
    
    return -1;
}

void HexEditor::handleMouseWheel(SDL_MouseWheelEvent& wheel) {
    SDL_Keymod mod = SDL_GetModState();
    if (mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI)) {
        float zoomDelta = wheel.y * ZOOM_STEP;
        adjustZoom(zoomDelta);
        return;
    }

    float scrollAmount = -wheel.y * 0.2f;
    addScrollVelocity(scrollAmount);
}

void HexEditor::scrollToAddress(size_t address) {
    if (address >= fileSize) address = fileSize - 1;
    size_t row = address / ROW_SIZE;
    
    if (row > scrollbar.visibleItems / 2) {
        scrollbar.offset = row - scrollbar.visibleItems / 2;
    } else {
        scrollbar.offset = 0;
    }
    
    if (scrollbar.offset + scrollbar.visibleItems > scrollbar.totalItems) {
        scrollbar.offset = scrollbar.maxOffset();
    }
    needsRedraw = true;
}

void HexEditor::update(float deltaTime) {
    bool needsUpdate = false;
    
    // Handle smooth zoom transitions
    if (std::abs(targetZoomLevel - zoomLevel) > 0.001f) {
        float diff = targetZoomLevel - zoomLevel;
        float step = diff * ZOOM_SMOOTH_SPEED * deltaTime;
        
        if (std::abs(step) > std::abs(diff)) {
            zoomLevel = targetZoomLevel;
        } else {
            zoomLevel += step;
        }
        
        recalculateLayoutForZoom();
        needsUpdate = true;
    }
    
    // Handle auto-scrolling when selecting at edges
    if (isSelecting && autoScrollDirection != 0) {
        autoScrollTimer += deltaTime;
        if (autoScrollTimer >= AUTO_SCROLL_DELAY) {
            if (autoScrollDirection < 0 && scrollbar.offset > 0) {
                scrollBy(-1);
                
                // Extend selection to newly visible bytes at the top
                size_t firstVisibleByte = scrollbar.offset * ROW_SIZE;
                if (firstVisibleByte < static_cast<size_t>(selectionStart)) {
                    selectionEnd = static_cast<int64_t>(firstVisibleByte);
                }
                
                needsUpdate = true;
            } else if (autoScrollDirection > 0 && scrollbar.canScroll() && scrollbar.offset < scrollbar.maxOffset()) {
                scrollBy(1);
                
                // Extend selection to newly visible bytes at the bottom
                size_t lastVisibleByte = std::min(
                    (scrollbar.offset + scrollbar.visibleItems) * ROW_SIZE - 1,
                    fileSize - 1
                );
                if (lastVisibleByte > static_cast<size_t>(selectionStart)) {
                    selectionEnd = static_cast<int64_t>(lastVisibleByte);
                }
                
                needsUpdate = true;
            }
            autoScrollTimer = 0.0f;
        }
    }
    
    // Let base class handle momentum scrolling
    SDLAppBase::update(deltaTime);
    
    if (needsUpdate) {
        needsRedraw = true;
    }
}

void HexEditor::selectByte(int64_t index) {
    if (index >= 0 && static_cast<size_t>(index) < fileSize) {
        if (selectedByteIndex >= 0 && !editBuffer.empty()) {
            commitEdit();
        }
        
        selectedByteIndex = index;
        editBuffer.clear();
        
        size_t row = index / ROW_SIZE;
        if (row < scrollbar.offset || row >= scrollbar.offset + scrollbar.visibleItems) {
            scrollToAddress(index);
        }
        
        needsRedraw = true;
    }
}

void HexEditor::commitEdit() {
    if (selectedByteIndex >= 0 && editBuffer.length() == 2) {
        unsigned int value = std::stoul(editBuffer, nullptr, 16);
        if (fileBuffer[selectedByteIndex] != static_cast<char>(value)) {
            char oldValue = fileBuffer[selectedByteIndex];
            char newValue = static_cast<char>(value);

            undoStack.push_back(EditAction{
                static_cast<size_t>(selectedByteIndex),
                oldValue,
                newValue
            });

            fileBuffer[selectedByteIndex] = newValue;
            updateModifiedState(static_cast<size_t>(selectedByteIndex));
        }
    }
    editBuffer.clear();
    needsRedraw = true;
}

void HexEditor::undoLastEdit() {
    if (undoStack.empty()) {
        return;
    }

    if (!editBuffer.empty()) {
        editBuffer.clear();
    }

    EditAction action = undoStack.back();
    undoStack.pop_back();

    if (action.index < fileSize) {
        fileBuffer[action.index] = action.oldValue;
        updateModifiedState(action.index);
        clearSelection();
        selectByte(static_cast<int64_t>(action.index));
    }

    needsRedraw = true;
}

void HexEditor::updateModifiedState(size_t index) {
    if (index >= fileSize || savedFileBuffer.size() != fileSize) {
        return;
    }

    if (fileBuffer[index] != savedFileBuffer[index]) {
        modifiedBytes.insert(index);
    } else {
        modifiedBytes.erase(index);
    }

    hasUnsavedChanges = !modifiedBytes.empty();
    updateWindowTitle();
    setConfirmOnQuit(hasUnsavedChanges);
}

void HexEditor::handleEditInput(char c) {
    if (selectedByteIndex < 0) return;
    
    if (HexUtils::isHexDigit(c)) {
        editBuffer += HexUtils::toUpperHex(c);
        
        if (editBuffer.length() >= 2) {
            commitEdit();
            if (static_cast<size_t>(selectedByteIndex + 1) < fileSize) {
                selectByte(selectedByteIndex + 1);
            }
        }
        needsRedraw = true;
    }
}

bool HexEditor::fileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

std::string HexEditor::getOutputPath() {
    if (overwriteMode) {
        return fileName;
    }
    return "edited_files/" + baseFileName;
}

bool HexEditor::saveFile() {
    if (!overwriteMode) {
        MKDIR("edited_files");
    }
    
    std::string outputPath = getOutputPath();
    
    // Check if file exists and show confirmation dialog
    if (fileExists(outputPath)) {
        std::string displayName = HexUtils::getBaseName(outputPath);
        if (!showOverwriteConfirmDialog(displayName)) {
            std::cout << "Save cancelled." << std::endl;
            return false;
        }
    }
    
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Failed to save: " << outputPath << std::endl;
        return false;
    }
    
    outFile.write(fileBuffer.data(), static_cast<std::streamsize>(fileSize));
    outFile.close();
    
    savedFileBuffer = fileBuffer;
    modifiedBytes.clear();
    hasUnsavedChanges = false;
    updateWindowTitle();
    setConfirmOnQuit(false);
    needsRedraw = true;
    
    std::cout << "Saved to: " << outputPath << std::endl;
    return true;
}

void HexEditor::appendHexInput(const std::string& text) {
    for (char c : text) {
        if (HexUtils::isHexDigit(c) && gotoAddressInput.length() < 8) {
            gotoAddressInput += HexUtils::toUpperHex(c);
        }
    }
}

void HexEditor::clearSelection() {
    isSelecting = false;
    selectionStart = -1;
    selectionEnd = -1;
    needsRedraw = true;
}

bool HexEditor::hasSelection() const {
    return selectionStart >= 0 && selectionEnd >= 0 && selectionStart != selectionEnd;
}

void HexEditor::getSelectionRange(int64_t& start, int64_t& end) const {
    if (selectionStart <= selectionEnd) {
        start = selectionStart;
        end = selectionEnd;
    } else {
        start = selectionEnd;
        end = selectionStart;
    }
}

void HexEditor::handleCopy() {
    if (!hasSelection()) {
        if (selectedByteIndex >= 0) {
            unsigned char byte = fileBuffer[selectedByteIndex];
            std::string hexStr = HexUtils::toHexString(byte, 2);
            SDL_SetClipboardText(hexStr.c_str());
        }
        return;
    }
    
    int64_t start, end;
    getSelectionRange(start, end);
    
    std::stringstream ss;
    for (int64_t i = start; i <= end; i++) {
        if (i < static_cast<int64_t>(fileSize)) {
            unsigned char byte = fileBuffer[i];
            ss << HexUtils::toHexString(byte, 2);
        }
    }
    
    std::string hexStr = ss.str();
    SDL_SetClipboardText(hexStr.c_str());
}

void HexEditor::handlePaste() {
    if (SDL_HasClipboardText()) {
        char* clipboardText = SDL_GetClipboardText();
        if (clipboardText) {
            std::string text = clipboardText;
            SDL_free(clipboardText);
            
            if (text.length() >= 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
                text = text.substr(2);
            }
            
            if (gotoMode) {
                appendHexInput(text);
            } else if (searchMode) {
                for (char c : text) {
                    if (HexUtils::isHexDigit(c)) {
                        searchInput += HexUtils::toUpperHex(c);
                    }
                }
                updateSearchMatches();
            } else if (selectedByteIndex >= 0) {
                for (char c : text) {
                    if (HexUtils::isHexDigit(c)) {
                        handleEditInput(c);
                    }
                }
            }
            needsRedraw = true;
        }
    }
}

void HexEditor::updateSearchMatches() {
    searchMatches.clear();
    currentMatchIndex = 0;
    
    if (searchInput.empty() || searchInput.length() % 2 != 0) {
        needsRedraw = true;
        return;
    }
    
    // Convert search input to bytes
    std::vector<unsigned char> searchBytes;
    for (size_t i = 0; i < searchInput.length(); i += 2) {
        std::string byteStr = searchInput.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(std::stoul(byteStr, nullptr, 16));
        searchBytes.push_back(byte);
    }
    
    if (searchBytes.empty()) {
        needsRedraw = true;
        return;
    }
    
    // Search through the file buffer
    for (size_t i = 0; i <= fileSize - searchBytes.size(); i++) {
        bool match = true;
        for (size_t j = 0; j < searchBytes.size(); j++) {
            if (static_cast<unsigned char>(fileBuffer[i + j]) != searchBytes[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            searchMatches.push_back(i);
        }
    }
    
    needsRedraw = true;
}

void HexEditor::gotoNextMatch() {
    if (searchMatches.empty()) {
        return;
    }
    
    size_t matchAddr = searchMatches[currentMatchIndex];
    scrollToAddress(matchAddr);
    selectByte(static_cast<int64_t>(matchAddr));
    
    // Move to next match for next time
    currentMatchIndex = (currentMatchIndex + 1) % searchMatches.size();
    
    needsRedraw = true;
}

void HexEditor::handleSearchInput(SDL_Keycode key, Uint16 mod) {
    // Check for save command first
    if (key == SDLK_S && (mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI))) {
        saveFile();
        return;
    }
    
    if (key == SDLK_V && (mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI))) {
        handlePaste();
        return;
    }
    
    if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
        gotoNextMatch();
        return;
    } else if (key == SDLK_ESCAPE) {
        searchMode = false;
        searchInput.clear();
        searchMatches.clear();
        currentMatchIndex = 0;
    } else if (key == SDLK_BACKSPACE) {
        if (!searchInput.empty()) {
            searchInput.pop_back();
            updateSearchMatches();
        }
    } else if (key == SDLK_G && !(mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI))) {
        // Switch to goto mode only if not a command
        searchMode = false;
        gotoMode = true;
        gotoAddressInput.clear();
    }
    needsRedraw = true;
}

void HexEditor::handleGotoInput(SDL_Keycode key, Uint16 mod) {
    // Check for save command first
    if (key == SDLK_S && (mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI))) {
        saveFile();
        return;
    }
    
    if (key == SDLK_V && (mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI))) {
        handlePaste();
        return;
    }
    
    if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
        if (!gotoAddressInput.empty()) {
            size_t addr = HexUtils::parseHexAddress(gotoAddressInput);
            scrollToAddress(addr);
            selectByte(addr);
        }
        gotoMode = false;
        gotoAddressInput.clear();
    } else if (key == SDLK_ESCAPE) {
        gotoMode = false;
        gotoAddressInput.clear();
    } else if (key == SDLK_BACKSPACE) {
        if (!gotoAddressInput.empty()) {
            gotoAddressInput.pop_back();
        }
    } else if (key == SDLK_S && !(mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI))) {
        // Switch to search mode only if not a command
        gotoMode = false;
        searchMode = true;
        searchInput.clear();
        searchMatches.clear();
        currentMatchIndex = 0;
    }
    needsRedraw = true;
}

void HexEditor::handleTextInput(const char* text) {
    if (gotoMode) {
        appendHexInput(text);
        needsRedraw = true;
    } else if (searchMode) {
        for (const char* c = text; *c; c++) {
            if (HexUtils::isHexDigit(*c)) {
                searchInput += HexUtils::toUpperHex(*c);
            }
        }
        updateSearchMatches();
        needsRedraw = true;
    } else if (selectedByteIndex >= 0) {
        for (const char* c = text; *c; c++) {
            handleEditInput(*c);
        }
    }
}

void HexEditor::handleMouseDown(int x, int y) {
    if (isPointInRect(x, y, saveButtonRect)) {
        saveFile();
        return;
    }
    
    // Reset auto-scroll state when starting new selection
    autoScrollDirection = 0;
    autoScrollTimer = 0.0f;
    
    // Check scrollbar
    if (handleScrollbarClick(x, y)) {
        return;
    }
    
    int byteIndex = getByteIndexFromPosition(x, y);
    if (byteIndex >= 0) {
        if (selectedByteIndex >= 0 && !editBuffer.empty()) {
            commitEdit();
        }
        
        selectedByteIndex = byteIndex;
        selectionStart = byteIndex;
        selectionEnd = byteIndex;
        isSelecting = true;
        editBuffer.clear();
        needsRedraw = true;
    } else {
        if (selectedByteIndex >= 0) {
            commitEdit();
            selectedByteIndex = -1;
            editBuffer.clear();
        }
        clearSelection();
        needsRedraw = true;
    }
}

void HexEditor::handleMouseUp() {
    handleScrollbarRelease();
    
    if (isSelecting) {
        isSelecting = false;
        autoScrollDirection = 0;
        autoScrollTimer = 0.0f;
        needsRedraw = true;
    }
}

void HexEditor::handleMouseMotion(int x, int y) {
    if (scrollbar.dragging) {
        handleScrollbarDrag(y);
        return;
    }
    
    // Check if mouse is over save button
    bool wasHovered = saveButtonHovered;
    saveButtonHovered = isPointInRect(x, y, saveButtonRect);
    if (wasHovered != saveButtonHovered) {
        needsRedraw = true;
    }
    
    if (isSelecting) {
        int contentY = headerHeight + 5 + effectiveCharHeight;
        
        // Update auto-scroll direction based on mouse position
        if (y < contentY && scrollbar.offset > 0) {
            autoScrollDirection = -1; // Scroll up
        } else if (y > windowHeight - effectiveCharHeight && 
                   scrollbar.canScroll() && 
                   scrollbar.offset < scrollbar.maxOffset()) {
            autoScrollDirection = 1; // Scroll down
        } else {
            autoScrollDirection = 0; // No auto-scroll
        }
        
        int byteIndex = getByteIndexFromPosition(x, y);
        if (byteIndex >= 0 && byteIndex != selectionEnd) {
            selectionEnd = byteIndex;
            needsRedraw = true;
        }
    }
}

void HexEditor::handleKeyDown(SDL_Keycode key, Uint16 mod) {
    if (mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI)) {
        if (key == SDLK_Z) {
            undoLastEdit();
            return;
        }
        if (key == SDLK_EQUALS || key == SDLK_PLUS || key == SDLK_KP_PLUS) {
            adjustZoom(ZOOM_STEP);
            return;
        } else if (key == SDLK_MINUS || key == SDLK_KP_MINUS) {
            adjustZoom(-ZOOM_STEP);
            return;
        } else if (key == SDLK_0 || key == SDLK_KP_0) {
            setZoom(1.0f);
            return;
        }
    }
    
    if (key == SDLK_S && (mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI))) {
        saveFile();
        return;
    }
    
    if (key == SDLK_C && (mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI))) {
        handleCopy();
        return;
    }
    
    if (key == SDLK_V && (mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI))) {
        handlePaste();
        return;
    }
    
    switch (key) {
        case SDLK_UP:
            clearSelection();
            if (selectedByteIndex >= ROW_SIZE) {
                selectByte(selectedByteIndex - ROW_SIZE);
            } else {
                scrollBy(-1);
            }
            break;
        case SDLK_DOWN:
            clearSelection();
            if (selectedByteIndex >= 0 && static_cast<size_t>(selectedByteIndex + ROW_SIZE) < fileSize) {
                selectByte(selectedByteIndex + ROW_SIZE);
            } else {
                scrollBy(1);
            }
            break;
        case SDLK_LEFT:
            clearSelection();
            if (selectedByteIndex > 0) {
                selectByte(selectedByteIndex - 1);
            }
            break;
        case SDLK_RIGHT:
            clearSelection();
            if (selectedByteIndex >= 0 && static_cast<size_t>(selectedByteIndex + 1) < fileSize) {
                selectByte(selectedByteIndex + 1);
            }
            break;
        case SDLK_TAB:
            clearSelection();
            if (selectedByteIndex >= 0) {
                if (mod & SDL_KMOD_SHIFT) {
                    if (selectedByteIndex > 0) selectByte(selectedByteIndex - 1);
                } else {
                    if (static_cast<size_t>(selectedByteIndex + 1) < fileSize) selectByte(selectedByteIndex + 1);
                }
            }
            break;
        case SDLK_PAGEUP:
            clearSelection();
            scrollBy(-static_cast<int64_t>(scrollbar.visibleItems));
            break;
        case SDLK_PAGEDOWN:
            clearSelection();
            scrollBy(static_cast<int64_t>(scrollbar.visibleItems));
            break;
        case SDLK_HOME:
            clearSelection();
            if (mod & SDL_KMOD_CTRL) {
                scrollbar.offset = 0;
                selectByte(0);
            } else if (scrollbar.offset != 0) {
                scrollbar.offset = 0;
                needsRedraw = true;
            }
            break;
        case SDLK_END:
            clearSelection();
            if (mod & SDL_KMOD_CTRL) {
                selectByte(fileSize - 1);
            } else if (scrollbar.canScroll()) {
                size_t newOffset = scrollbar.maxOffset();
                if (scrollbar.offset != newOffset) {
                    scrollbar.offset = newOffset;
                    needsRedraw = true;
                }
            }
            break;
        case SDLK_G:
            gotoMode = true;
            gotoAddressInput.clear();
            needsRedraw = true;
            break;
        case SDLK_S:
            // Enter search mode (without Cmd/Ctrl modifier - that's handled above for save)
            searchMode = true;
            searchInput.clear();
            searchMatches.clear();
            currentMatchIndex = 0;
            needsRedraw = true;
            break;
        case SDLK_ESCAPE:
            if (hasSelection()) {
                clearSelection();
            } else if (selectedByteIndex >= 0) {
                commitEdit();
                selectedByteIndex = -1;
                editBuffer.clear();
                needsRedraw = true;
            } else {
                if (hasUnsavedChanges) {
                    if (showQuitConfirmDialog()) {
                        quit();
                    }
                } else {
                    quit();
                }
            }
            break;
        case SDLK_Q:
            if (selectedByteIndex < 0) {
                if (hasUnsavedChanges) {
                    if (showQuitConfirmDialog()) {
                        quit();
                    }
                } else {
                    quit();
                }
            }
            break;
        case SDLK_BACKSPACE:
            if (selectedByteIndex >= 0 && !editBuffer.empty()) {
                editBuffer.pop_back();
                needsRedraw = true;
            }
            break;
        default:
            break;
    }
}

void HexEditor::handleEvent(SDL_Event& event) {
    switch (event.type) {            
        case SDL_EVENT_TEXT_INPUT:
            handleTextInput(event.text.text);
            break;
            
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                handleMouseDown(static_cast<int>(event.button.x),
                                static_cast<int>(event.button.y));
            }
            break;
            
        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                handleMouseUp();
            }
            break;
            
        case SDL_EVENT_MOUSE_MOTION:
            handleMouseMotion(static_cast<int>(event.motion.x),
                              static_cast<int>(event.motion.y));
            break;
            
        case SDL_EVENT_MOUSE_WHEEL:
            handleMouseWheel(event.wheel);
            break;
            
        case SDL_EVENT_KEY_DOWN:
            if (gotoMode) {
                handleGotoInput(event.key.key, event.key.mod);
            } else if (searchMode) {
                handleSearchInput(event.key.key, event.key.mod);
            } else {
                handleKeyDown(event.key.key, event.key.mod);
            }
            break;
            
        default:
            break;
    }
}

void HexEditor::renderHeader() {
    SDL_Rect headerRect = {0, 0, windowWidth, headerHeight};
    renderFilledRect(headerRect, colors.headerBg);
    
    std::stringstream ss;
    ss << baseFileName << " | " << HexUtils::formatFileSize(fileSize);
    if (overwriteMode) {
        ss << " [OVERWRITE]";
    }
    if (hasUnsavedChanges) {
        ss << " [MODIFIED]";
    }
    
    SDL_Color headerColor = colors.text;
    if (overwriteMode) {
        headerColor = colors.warning;
    } else if (hasUnsavedChanges) {
        headerColor = colors.error;
    }
    
    renderText(ss.str(), 10, 5, headerColor);
    
    size_t currentAddr = scrollbar.offset * ROW_SIZE;
    size_t endAddr = std::min(currentAddr + scrollbar.visibleItems * ROW_SIZE, fileSize);
    ss.str("");
    
    if (selectedByteIndex >= 0) {
        ss << "Selected: 0x" << HexUtils::toHexString(selectedByteIndex, 8);
        if (!editBuffer.empty()) {
            ss << " [" << editBuffer << "_]";
        }
    } else {
        ss << "Offset: 0x" << HexUtils::toHexString(currentAddr, 8) 
           << " - 0x" << HexUtils::toHexString(endAddr, 8);
    }
    
    ss << " | Zoom: " << static_cast<int>(zoomLevel * 100 + 0.5f) << "%";
    
    renderText(ss.str(), 10, 5 + charHeight, colors.text);
    
    int rightX = windowWidth - scrollbar.width;
    
    saveButtonRect = {rightX - 180, 10, 50, charHeight + 6};
    
    // Render button with hover effect
    if (saveButtonHovered) {
        // Draw a slightly larger/brighter button when hovered
        SDL_Rect hoverRect = {saveButtonRect.x - 1, saveButtonRect.y - 1, 
                              saveButtonRect.w + 2, saveButtonRect.h + 2};
        renderFilledRect(hoverRect, {80, 80, 80, 255});
        renderButton(saveButtonRect, "Save");
        // Add a subtle glow effect
        renderOutlineRect(hoverRect, colors.accent);
    } else {
        renderButton(saveButtonRect, "Save");
    }
    
    if (gotoMode) {
        SDL_Rect inputRect = {rightX - 120, 8, 115, charHeight + 8};
        renderFilledRect(inputRect, colors.inputBg);
        
        std::string prompt = "0x" + gotoAddressInput + "_";
        renderText(prompt, rightX - 115, 10, colors.accent);
    } else if (searchMode) {
        // Search bar - same size as goto bar
        SDL_Rect inputRect = {rightX - 120, 8, 115, charHeight + 8};
        renderFilledRect(inputRect, colors.inputBg);
        
        // Calculate how many characters can fit in the search bar
        int availableWidth = 105; // 115 - 10 (5px padding on each side)
        int prefixWidth = charWidth * 2; // "S:"
        
        // Build match count string
        std::string matchStr;
        if (!searchInput.empty() && searchInput.length() % 2 == 0) {
            size_t numMatches = searchMatches.size();
            if (numMatches > 99) {
                matchStr = "(99+)";
            } else {
                matchStr = "(" + std::to_string(numMatches) + ")";
            }
        }
        int matchWidth = static_cast<int>(matchStr.length()) * charWidth;
        int cursorWidth = charWidth; // "_"
        
        int inputAvailableWidth = availableWidth - prefixWidth - matchWidth - cursorWidth;
        int maxVisibleChars = std::max(0, inputAvailableWidth / charWidth);
        
        // If input is longer than available space, show the end of it
        std::string visibleInput = searchInput;
        if (static_cast<int>(searchInput.length()) > maxVisibleChars && maxVisibleChars > 0) {
            visibleInput = searchInput.substr(searchInput.length() - maxVisibleChars);
        }
        
        std::string displayStr = "S:" + visibleInput + "_" + matchStr;
        renderText(displayStr, rightX - 115, 10, colors.accent);
    } else {
        renderText("G:Goto S:Search", rightX - 120, 18, colors.textDim);
    }
    
    renderLine(0, headerHeight - 1, windowWidth, headerHeight - 1, {60, 60, 60, 255});
}

void HexEditor::render() {
    SDL_SetRenderDrawColor(renderer, colors.background.r, colors.background.g, 
                          colors.background.b, 255);
    SDL_RenderClear(renderer);
    
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
    
    renderHeader();
    
    if (fileSize == 0) {
        renderText("No file loaded.", 10, headerHeight + 20, colors.text);
        SDL_RenderPresent(renderer);
        return;
    }
    
    int y = headerHeight + 5;
    
    renderTextScaled("Address", addressX, y, colors.textDim, zoomLevel);
    
    for (int i = 0; i < ROW_SIZE; ++i) {
        int headerByteX = getByteXPosition(i);
        renderTextScaled(HexUtils::toHexString(i, 2), headerByteX, y, colors.textDim, zoomLevel);
    }
    
    std::string decodedHeader = "Decoded";
    if (textEncoding != TextEncoding::ASCII) {
        decodedHeader = getEncodingName(textEncoding);
    }
    renderTextScaled(decodedHeader, asciiX, y, colors.textDim, zoomLevel);
    y += effectiveCharHeight;
    
    renderLine(addressX, y - 2, windowWidth - scrollbar.width - 5, y - 2, {50, 50, 50, 255});
    
    int64_t selStart = -1, selEnd = -1;
    if (hasSelection()) {
        getSelectionRange(selStart, selEnd);
    }
    
    // Build a set of search match positions for highlighting
    std::set<size_t> searchMatchPositions;
    if (searchMode && !searchMatches.empty() && searchInput.length() >= 2) {
        size_t matchLen = searchInput.length() / 2;
        for (size_t matchAddr : searchMatches) {
            for (size_t j = 0; j < matchLen; j++) {
                searchMatchPositions.insert(matchAddr + j);
            }
        }
    }
    
    for (size_t row = 0; row < scrollbar.visibleItems && (scrollbar.offset + row) < scrollbar.totalItems; row++) {
        size_t currentRow = scrollbar.offset + row;
        size_t address = currentRow * ROW_SIZE;
        size_t bytesInRow = std::min(static_cast<size_t>(ROW_SIZE), fileSize - address);
        
        if (row % 2 == 1) {
            SDL_Rect rowRect = {0, y, windowWidth - scrollbar.width, effectiveCharHeight};
            renderFilledRect(rowRect, {35, 35, 35, 255});
        }
        
        renderTextScaled(HexUtils::toHexString(address, 8), addressX, y, colors.accent, zoomLevel);
        
        for (size_t i = 0; i < ROW_SIZE; i++) {
            size_t byteIndex = address + i;
            int byteX = getByteXPosition(static_cast<int>(i));
            
            if (i < bytesInRow) {
                bool inSelection = (selStart >= 0 && selEnd >= 0 && 
                                   static_cast<int64_t>(byteIndex) >= selStart && 
                                   static_cast<int64_t>(byteIndex) <= selEnd);
                
                bool inSearchMatch = searchMatchPositions.count(byteIndex) > 0;
                
                if (static_cast<int64_t>(byteIndex) == selectedByteIndex) {
                    SDL_Rect selectRect = {byteX, y, effectiveCharWidth * 2, effectiveCharHeight};
                    renderFilledRect(selectRect, colors.selectedBg);
                } else if (inSelection) {
                    SDL_Rect selectRect = {byteX, y, effectiveCharWidth * 2, effectiveCharHeight};
                    renderFilledRect(selectRect, colors.selectedBg);
                } else if (inSearchMatch) {
                    SDL_Rect highlightRect = {byteX, y, effectiveCharWidth * 2, effectiveCharHeight};
                    renderFilledRect(highlightRect, {80, 80, 0, 255}); // Yellow-ish highlight for search matches
                }
                
                unsigned char byte = fileBuffer[byteIndex];
                std::string byteStr = HexUtils::toHexString(byte, 2);
                
                SDL_Color byteColor = modifiedBytes.count(byteIndex) ? colors.warning : colors.text;
                renderTextScaled(byteStr, byteX, y, byteColor, zoomLevel);
            }
        }
        
        std::string decodedStr;
        for (size_t i = 0; i < bytesInRow; i++) {
            unsigned char c = fileBuffer[address + i];
            std::string decoded = decodeByte(c, textEncoding);
            if (decoded.empty()) {
                decodedStr += '.';
            } else if (decoded.length() == 1 && static_cast<unsigned char>(decoded[0]) < 128) {
                decodedStr += decoded;
            } else {
                decodedStr += (decoded.length() > 0) ? decoded[0] : '?';
            }
        }
        renderTextScaled(decodedStr, asciiX, y, colors.success, zoomLevel);
        
        y += effectiveCharHeight;
    }
    
    renderScrollbar();
    
    SDL_RenderPresent(renderer);
}

bool HexEditor::applyBatchEdits(const std::vector<std::pair<size_t, std::vector<unsigned char>>>& edits) {
    for (const auto& edit : edits) {
        size_t addr = edit.first;
        const std::vector<unsigned char>& bytes = edit.second;
        
        for (size_t i = 0; i < bytes.size(); i++) {
            size_t targetAddr = addr + i;
            if (targetAddr < fileSize) {
                char oldValue = fileBuffer[targetAddr];
                char newValue = static_cast<char>(bytes[i]);

                if (oldValue != newValue) {
                    undoStack.push_back(EditAction{
                        targetAddr,
                        oldValue,
                        newValue
                    });

                    fileBuffer[targetAddr] = newValue;
                    updateModifiedState(targetAddr);
                }
            } else {
                std::cerr << "Warning: Address 0x" << HexUtils::toHexString(targetAddr, 8)
                          << " is beyond file size (" << fileSize << " bytes)" << std::endl;
            }
        }
    }
    
    needsRedraw = true;
    return true;
}

void HexEditor::runBatchSaveMode() {
    // If saveFile returned false, it means user cancelled or there was an error
    if (saveFile()) {
        return;
    }
}