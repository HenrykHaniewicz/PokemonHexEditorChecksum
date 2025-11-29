#include "hex_editor.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

HexEditor::HexEditor() 
    : SDLAppBase("GBA/GB Hex Editor", 800, 700),
      fileSize(0),
      headerHeight(50), scrollbarWidth(14),
      byteGrouping(1),
      textEncoding(TextEncoding::ASCII),
      baseCharWidth(0), baseCharHeight(0),
      effectiveCharWidth(0), effectiveCharHeight(0),
      addressX(10), hexX(0), asciiX(0), contentEndX(0),
      scrollOffset(0), visibleRows(0), totalRows(0),
      scrollVelocity(0.0f), accumulatedScroll(0.0f),
      draggingScrollbar(false), dragStartY(0), dragStartRatio(0.0f),
      zoomLevel(1.0f), targetZoomLevel(1.0f),
      gotoMode(false),
      selectedByteIndex(-1), hasUnsavedChanges(false),
      showConfirmDialog(false), confirmOverwrite(false),
      overwriteMode(false),
      isSelecting(false), selectionStart(-1), selectionEnd(-1) {
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
    int availableWidth = windowWidth - scrollbarWidth - 20;  // 20 for margins
    
    // Calculate base content width (at zoom 1.0)
    int baseHexX = addressX + baseCharWidth * 10;
    
    // Hex section: 16 bytes, with grouping spaces and center gap
    int numGroups = 16 / byteGrouping;
    int baseHexWidth = numGroups * (byteGrouping * 2 + 1) * baseCharWidth + baseCharWidth;  // +1 char for center gap
    
    int baseAsciiX = baseHexX + baseHexWidth;
    int baseContentWidth = baseAsciiX + baseCharWidth * 16 + 10;
    
    float maxZoom = (float)availableWidth / (float)baseContentWidth;
    
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
    
    effectiveCharWidth = (int)(baseCharWidth * zoomLevel);
    effectiveCharHeight = (int)(baseCharHeight * zoomLevel);
    
    addressX = 10;
    hexX = addressX + effectiveCharWidth * 10;
    
    // Calculate hex section width with proper spacing
    int numGroups = 16 / byteGrouping;
    int hexSectionWidth = numGroups * (byteGrouping * 2 + 1) * effectiveCharWidth;
    hexSectionWidth += effectiveCharWidth;  // Extra gap in middle (between byte 7 and 8)
    
    asciiX = hexX + hexSectionWidth;
    contentEndX = asciiX + effectiveCharWidth * 16 + 10;
    
    // Calculate visible rows
    int availableHeight = windowHeight - headerHeight - effectiveCharHeight - 20;
    visibleRows = std::max(1, availableHeight / effectiveCharHeight);
    
    needsRedraw = true;
}

int HexEditor::getByteXPosition(int byteInRow) {
    int groupIndex = byteInRow / byteGrouping;
    int posInGroup = byteInRow % byteGrouping;
    
    int x = hexX;
    // Add space for each complete group before this one
    x += groupIndex * (byteGrouping * 2 + 1) * effectiveCharWidth;
    // Add position within current group
    x += posInGroup * 2 * effectiveCharWidth;
    
    // Add extra gap after byte 7 (between first and second half)
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
    
    totalRows = (fileSize + ROW_SIZE - 1) / ROW_SIZE;
    scrollOffset = 0;
    hasUnsavedChanges = false;
    modifiedBytes.clear();
    selectedByteIndex = -1;
    zoomLevel = 1.0f;
    targetZoomLevel = 1.0f;
    needsRedraw = true;
    
    baseCharWidth = charWidth;
    baseCharHeight = charHeight;
    
    recalculateLayoutForZoom();
    updateWindowTitle();
    
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

void HexEditor::getScrollbarGeometry(int& sbX, int& sbY, int& sbHeight, int& thumbY, int& thumbHeight) {
    sbX = windowWidth - scrollbarWidth;
    sbY = headerHeight;
    sbHeight = windowHeight - headerHeight;
    
    if (totalRows > visibleRows) {
        float thumbRatio = (float)visibleRows / totalRows;
        thumbHeight = std::max(30, (int)(sbHeight * thumbRatio));
        float scrollRatio = (float)scrollOffset / (totalRows - visibleRows);
        thumbY = sbY + (int)((sbHeight - thumbHeight) * scrollRatio);
    } else {
        thumbHeight = sbHeight;
        thumbY = sbY;
    }
}

int HexEditor::getByteIndexFromPosition(int x, int y) {
    int contentY = headerHeight + 5 + effectiveCharHeight;
    if (y < contentY) return -1;
    
    int row = (y - contentY) / effectiveCharHeight;
    if (row < 0 || (size_t)row >= visibleRows) return -1;
    
    size_t actualRow = scrollOffset + row;
    if (actualRow >= totalRows) return -1;
    
    // Check if click is in hex area
    if (x < hexX || x >= asciiX) return -1;
    
    // Find which byte was clicked
    for (int i = 0; i < ROW_SIZE; i++) {
        int byteX = getByteXPosition(i);
        int byteEndX = byteX + effectiveCharWidth * 2;
        if (x >= byteX && x < byteEndX) {
            size_t byteIndex = actualRow * ROW_SIZE + i;
            if (byteIndex >= fileSize) return -1;
            return (int)byteIndex;
        }
    }
    
    return -1;
}

void HexEditor::scrollBy(int64_t rows) {
    if (totalRows <= visibleRows) return;
    
    int64_t newOffset = (int64_t)scrollOffset + rows;
    newOffset = std::max((int64_t)0, newOffset);
    newOffset = std::min((int64_t)(totalRows - visibleRows), newOffset);
    
    if ((size_t)newOffset != scrollOffset) {
        scrollOffset = newOffset;
        needsRedraw = true;
    }
}

void HexEditor::scrollBySmooth(float rows) {
    if (totalRows <= visibleRows) return;
    
    accumulatedScroll += rows;
    
    while (accumulatedScroll >= 1.0f) {
        if (scrollOffset < totalRows - visibleRows) {
            scrollOffset++;
            needsRedraw = true;
        }
        accumulatedScroll -= 1.0f;
    }
    while (accumulatedScroll <= -1.0f) {
        if (scrollOffset > 0) {
            scrollOffset--;
            needsRedraw = true;
        }
        accumulatedScroll += 1.0f;
    }
    
    if (scrollOffset == 0 && accumulatedScroll < 0) {
        accumulatedScroll = 0;
    }
    if (scrollOffset >= totalRows - visibleRows && accumulatedScroll > 0) {
        accumulatedScroll = 0;
    }
}

void HexEditor::handleMouseWheel(SDL_MouseWheelEvent& wheel) {
    if (showConfirmDialog) return;
    
    SDL_Keymod mod = SDL_GetModState();
    if (mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI)) {
        // Zoom with scroll wheel when Ctrl/Cmd is held
        float zoomDelta = wheel.y * ZOOM_STEP;
        adjustZoom(zoomDelta);
        return;
    }

    float scrollAmount = -wheel.y;

    // Treat all wheel motion as "precise" and use momentum
    scrollVelocity += scrollAmount * 0.2f;

    float maxVelocity = 50.0f;
    if (scrollVelocity > maxVelocity) scrollVelocity = maxVelocity;
    if (scrollVelocity < -maxVelocity) scrollVelocity = -maxVelocity;
}


void HexEditor::scrollToRatio(float ratio) {
    if (totalRows <= visibleRows) return;
    
    ratio = std::max(0.0f, std::min(1.0f, ratio));
    size_t newOffset = (size_t)(ratio * (totalRows - visibleRows));
    
    if (newOffset != scrollOffset) {
        scrollOffset = newOffset;
        needsRedraw = true;
    }
}

void HexEditor::scrollToAddress(size_t address) {
    if (address >= fileSize) address = fileSize - 1;
    size_t row = address / ROW_SIZE;
    
    if (row > visibleRows / 2) {
        scrollOffset = row - visibleRows / 2;
    } else {
        scrollOffset = 0;
    }
    
    if (scrollOffset + visibleRows > totalRows) {
        scrollOffset = (totalRows > visibleRows) ? totalRows - visibleRows : 0;
    }
    needsRedraw = true;
}

void HexEditor::update(float deltaTime) {
    bool needsUpdate = false;
    
    // Handle smooth zoom transitions
    if (std::abs(targetZoomLevel - zoomLevel) > 0.001f) {
        float diff = targetZoomLevel - zoomLevel;
        float step = diff * ZOOM_SMOOTH_SPEED * deltaTime;
        
        // Prevent overshooting
        if (std::abs(step) > std::abs(diff)) {
            zoomLevel = targetZoomLevel;
        } else {
            zoomLevel += step;
        }
        
        recalculateLayoutForZoom();
        needsUpdate = true;
    }
    
    // Handle momentum scrolling
    if (std::abs(scrollVelocity) > SCROLL_STOP_THRESHOLD) {
        float scrollDelta = scrollVelocity * deltaTime * 60.0f;
        scrollBySmooth(scrollDelta);
        scrollVelocity *= SCROLL_FRICTION;
        needsUpdate = true;
    } else if (scrollVelocity != 0) {
        scrollVelocity = 0;
        accumulatedScroll = 0;
    }
    
    if (needsUpdate) {
        needsRedraw = true;
    }
}

void HexEditor::selectByte(int64_t index) {
    if (index >= 0 && (size_t)index < fileSize) {
        if (selectedByteIndex >= 0 && !editBuffer.empty()) {
            commitEdit();
        }
        
        selectedByteIndex = index;
        editBuffer.clear();
        
        size_t row = index / ROW_SIZE;
        if (row < scrollOffset || row >= scrollOffset + visibleRows) {
            scrollToAddress(index);
        }
        
        needsRedraw = true;
    }
}

void HexEditor::commitEdit() {
    if (selectedByteIndex >= 0 && editBuffer.length() == 2) {
        unsigned int value = std::stoul(editBuffer, nullptr, 16);
        if (fileBuffer[selectedByteIndex] != (char)value) {
            char oldValue = fileBuffer[selectedByteIndex];
            char newValue = (char)value;

            // Record this change for undo
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

    // For half-typed bytes
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
}

void HexEditor::handleEditInput(char c) {
    if (selectedByteIndex < 0) return;
    
    if (HexUtils::isHexDigit(c)) {
        editBuffer += HexUtils::toUpperHex(c);
        
        if (editBuffer.length() >= 2) {
            commitEdit();
            if ((size_t)(selectedByteIndex + 1) < fileSize) {
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
    
    if (fileExists(outputPath) && !confirmOverwrite) {
        showConfirmDialog = true;
        needsRedraw = true;
        return false;
    }
    
    showConfirmDialog = false;
    confirmOverwrite = false;
    
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Failed to save: " << outputPath << std::endl;
        return false;
    }
    
    outFile.write(fileBuffer.data(), fileSize);
    outFile.close();
    
    savedFileBuffer = fileBuffer;
    modifiedBytes.clear();
    hasUnsavedChanges = false;
    updateWindowTitle();
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
        // If no selection, copy single selected byte
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
        if (i < (int64_t)fileSize) {
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

void HexEditor::handleGotoInput(SDL_Keycode key, Uint16 mod) {
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
    }
    needsRedraw = true;
}

void HexEditor::handleTextInput(const char* text) {
    if (gotoMode) {
        appendHexInput(text);
        needsRedraw = true;
    } else if (selectedByteIndex >= 0) {
        for (const char* c = text; *c; c++) {
            handleEditInput(*c);
        }
    }
}

void HexEditor::handleMouseDown(int x, int y) {
    if (showConfirmDialog) {
        if (isPointInRect(x, y, yesButtonRect)) {
            confirmOverwrite = true;
            saveFile();
        } else if (isPointInRect(x, y, noButtonRect)) {
            showConfirmDialog = false;
            confirmOverwrite = false;
            needsRedraw = true;
        }
        return;
    }
    
    if (isPointInRect(x, y, saveButtonRect)) {
        confirmOverwrite = false;
        saveFile();
        return;
    }
    
    int sbX, sbY, sbHeight, thumbY, thumbHeight;
    getScrollbarGeometry(sbX, sbY, sbHeight, thumbY, thumbHeight);
    
    if (x >= sbX && x < sbX + scrollbarWidth &&
        y >= sbY && y < sbY + sbHeight) {
        
        if (totalRows <= visibleRows) return;
        
        if (y >= thumbY && y < thumbY + thumbHeight) {
            draggingScrollbar = true;
            dragStartY = y;
            dragStartRatio = (float)scrollOffset / (totalRows - visibleRows);
        } else {
            float clickRatio = (float)(y - sbY) / sbHeight;
            scrollToRatio(clickRatio);
        }
        needsRedraw = true;
        return;
    }
    
    int byteIndex = getByteIndexFromPosition(x, y);
    if (byteIndex >= 0) {
        // Start selection
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
    if (draggingScrollbar) {
        draggingScrollbar = false;
        needsRedraw = true;
    }
    
    if (isSelecting) {
        isSelecting = false;
        // Keep the selection for copying
        needsRedraw = true;
    }
}


void HexEditor::handleMouseMotion(int x, int y) {
    if (draggingScrollbar && totalRows > visibleRows) {
        int sbX, sbY, sbHeight, thumbY, thumbHeight;
        getScrollbarGeometry(sbX, sbY, sbHeight, thumbY, thumbHeight);
        
        int deltaY = y - dragStartY;
        float deltaRatio = (float)deltaY / (sbHeight - thumbHeight);
        float newRatio = dragStartRatio + deltaRatio;
        
        scrollToRatio(newRatio);
        return;
    }
    
    if (isSelecting) {
        int byteIndex = getByteIndexFromPosition(x, y);
        if (byteIndex >= 0 && byteIndex != selectionEnd) {
            selectionEnd = byteIndex;
            needsRedraw = true;
        }
    }
}

void HexEditor::handleKeyDown(SDL_Keycode key, Uint16 mod) {
    // Handle zoom shortcuts (Cmd/Ctrl + Plus/Minus)
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
            if (selectedByteIndex >= 0 && (size_t)(selectedByteIndex + ROW_SIZE) < fileSize) {
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
            if (selectedByteIndex >= 0 && (size_t)(selectedByteIndex + 1) < fileSize) {
                selectByte(selectedByteIndex + 1);
            }
            break;
        case SDLK_TAB:
            clearSelection();
            if (selectedByteIndex >= 0) {
                if (mod & SDL_KMOD_SHIFT) {
                    if (selectedByteIndex > 0) selectByte(selectedByteIndex - 1);
                } else {
                    if ((size_t)(selectedByteIndex + 1) < fileSize) selectByte(selectedByteIndex + 1);
                }
            }
            break;
        case SDLK_PAGEUP:
            clearSelection();
            scrollBy(-(int64_t)visibleRows);
            break;
        case SDLK_PAGEDOWN:
            clearSelection();
            scrollBy(visibleRows);
            break;
        case SDLK_HOME:
            clearSelection();
            if (mod & SDL_KMOD_CTRL) {
                scrollOffset = 0;
                selectByte(0);
            } else if (scrollOffset != 0) {
                scrollOffset = 0;
                needsRedraw = true;
            }
            break;
        case SDLK_END:
            clearSelection();
            if (mod & SDL_KMOD_CTRL) {
                selectByte(fileSize - 1);
            } else if (totalRows > visibleRows) {
                size_t newOffset = totalRows - visibleRows;
                if (scrollOffset != newOffset) {
                    scrollOffset = newOffset;
                    needsRedraw = true;
                }
            }
            break;
        case SDLK_G:
            gotoMode = true;
            gotoAddressInput.clear();
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
                quit();
            }
            break;
        case SDLK_Q:
            if (selectedByteIndex < 0) {
                quit();
            }
            break;
        case SDLK_BACKSPACE:
            if (selectedByteIndex >= 0 && !editBuffer.empty()) {
                editBuffer.pop_back();
                needsRedraw = true;
            }
            break;
    }
}

void HexEditor::handleEvent(SDL_Event& event) {
    switch (event.type) {            
        case SDL_EVENT_TEXT_INPUT:
            if (!showConfirmDialog) {
                handleTextInput(event.text.text);
            }
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
            if (showConfirmDialog) {
                if (event.key.key == SDLK_Y) {
                    confirmOverwrite = true;
                    saveFile();
                } else if (event.key.key == SDLK_N || 
                           event.key.key == SDLK_ESCAPE) {
                    showConfirmDialog = false;
                    confirmOverwrite = false;
                    needsRedraw = true;
                }
            } else if (gotoMode) {
                handleGotoInput(event.key.key, event.key.mod);
            } else {
                handleKeyDown(event.key.key, event.key.mod);
            }
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
        headerColor = colors.warning;  // Use warning color to make it more visible
    } else if (hasUnsavedChanges) {
        headerColor = colors.error;
    }
    
    renderText(ss.str(), 10, 5, headerColor);
    
    size_t currentAddr = scrollOffset * ROW_SIZE;
    size_t endAddr = std::min(currentAddr + visibleRows * ROW_SIZE, fileSize);
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
    
    // Show zoom level
    ss << " | Zoom: " << (int)(zoomLevel * 100 + 0.5f) << "%";
    
    renderText(ss.str(), 10, 5 + charHeight, colors.text);
    
    int rightX = windowWidth - scrollbarWidth;
    
    saveButtonRect = {rightX - 180, 10, 50, charHeight + 6};
    renderButton(saveButtonRect, "Save");
    
    if (gotoMode) {
        SDL_Rect inputRect = {rightX - 120, 8, 115, charHeight + 8};
        renderFilledRect(inputRect, colors.inputBg);
        
        std::string prompt = "0x" + gotoAddressInput + "_";
        renderText(prompt, rightX - 115, 10, colors.accent);
    } else {
        renderText("G:Goto", rightX - 60, 18, colors.textDim);
    }
    
    renderLine(0, headerHeight - 1, windowWidth, headerHeight - 1, {60, 60, 60, 255});
}

void HexEditor::renderConfirmationDialog() {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect fullScreen = {0, 0, windowWidth, windowHeight};
    renderFilledRect(fullScreen, {0, 0, 0, 150}, renderer);
    
    int dialogW = 350;
    int dialogH = 120;
    int dialogX = (windowWidth - dialogW) / 2;
    int dialogY = (windowHeight - dialogH) / 2;
    
    SDL_Rect dialogRect = {dialogX, dialogY, dialogW, dialogH};
    renderFilledRect(dialogRect, colors.dialogBg);
    renderOutlineRect(dialogRect, colors.dialogBorder);
    
    std::string msg1 = overwriteMode ? "File already exists." : "File already exists in 'edited_files'.";
    renderText(msg1, dialogX + 20, dialogY + 15, colors.text);
    renderText("Do you want to overwrite it?", dialogX + 20, dialogY + 35, colors.text);
    
    int buttonW = 80;
    int buttonH = 30;
    int buttonY = dialogY + dialogH - buttonH - 15;
    
    yesButtonRect = {dialogX + dialogW / 2 - buttonW - 20, buttonY, buttonW, buttonH};
    noButtonRect = {dialogX + dialogW / 2 + 20, buttonY, buttonW, buttonH};
    
    renderButton(yesButtonRect, "Yes");
    renderButton(noButtonRect, "No");
}


void HexEditor::render() {
    SDL_SetRenderDrawColor(renderer, colors.background.r, colors.background.g, 
                          colors.background.b, 255);
    SDL_RenderClear(renderer);
    
    // Reset any scaling
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
    
    renderHeader();
    
    if (fileSize == 0) {
        renderText("No file loaded.", 10, headerHeight + 20, colors.text);
        SDL_RenderPresent(renderer);
        return;
    }
    
    int y = headerHeight + 5;
    
    // Render column headers with zoomed font
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
    
    renderLine(addressX, y - 2, windowWidth - scrollbarWidth - 5, y - 2, {50, 50, 50, 255});
    
    // Get selection range if there is one
    int64_t selStart = -1, selEnd = -1;
    if (hasSelection()) {
        getSelectionRange(selStart, selEnd);
    }
    
    for (size_t row = 0; row < visibleRows && (scrollOffset + row) < totalRows; row++) {
        size_t currentRow = scrollOffset + row;
        size_t address = currentRow * ROW_SIZE;
        size_t bytesInRow = std::min((size_t)ROW_SIZE, fileSize - address);
        
        // Alternating row background
        if (row % 2 == 1) {
            SDL_Rect rowRect = {0, y, windowWidth - scrollbarWidth, effectiveCharHeight};
            renderFilledRect(rowRect, {35, 35, 35, 255});
        }
        
        // Address
        renderTextScaled(HexUtils::toHexString(address, 8), addressX, y, colors.accent, zoomLevel);
        
        // Hex bytes
        for (size_t i = 0; i < ROW_SIZE; i++) {
            size_t byteIndex = address + i;
            int byteX = getByteXPosition((int)i);
            
            if (i < bytesInRow) {
                // Check if this byte is in selection range
                bool inSelection = (selStart >= 0 && selEnd >= 0 && 
                                   (int64_t)byteIndex >= selStart && 
                                   (int64_t)byteIndex <= selEnd);
                
                // Selection or single byte highlight
                if (inSelection || (int64_t)byteIndex == selectedByteIndex) {
                    SDL_Rect selectRect = {byteX, y, effectiveCharWidth * 2, effectiveCharHeight};
                    renderFilledRect(selectRect, colors.selectedBg);
                }
                
                unsigned char byte = fileBuffer[byteIndex];
                std::string byteStr = HexUtils::toHexString(byte, 2);
                
                SDL_Color byteColor = modifiedBytes.count(byteIndex) ? colors.warning : colors.text;
                renderTextScaled(byteStr, byteX, y, byteColor, zoomLevel);
            }
        }
        
        //  Decoded text
        std::string decodedStr;
        for (size_t i = 0; i < bytesInRow; i++) {
            unsigned char c = fileBuffer[address + i];
            std::string decoded = decodeByte(c, textEncoding);
            if (decoded.empty()) {
                decodedStr += '.';
            } else if (decoded.length() == 1 && (unsigned char)decoded[0] < 128) {
                decodedStr += decoded;
            } else {
                // Multi-byte character - just use first char or placeholder
                decodedStr += (decoded.length() > 0) ? decoded[0] : '?';
            }
        }
        renderTextScaled(decodedStr, asciiX, y, colors.success, zoomLevel);
        
        y += effectiveCharHeight;
    }
    
    // Scrollbar
    int sbX, sbY, sbHeight, thumbY, thumbHeight;
    getScrollbarGeometry(sbX, sbY, sbHeight, thumbY, thumbHeight);
    
    SDL_Rect scrollBgRect = {sbX, sbY, scrollbarWidth, sbHeight};
    renderFilledRect(scrollBgRect, colors.scrollbarBg);
    
    SDL_Rect thumbRect = {sbX + 2, thumbY, scrollbarWidth - 4, thumbHeight};
    SDL_Color thumbColor = draggingScrollbar ? colors.scrollbarHover : colors.scrollbarFg;
    renderFilledRect(thumbRect, thumbColor);
    
    if (showConfirmDialog) {
        renderConfirmationDialog();
    }
    
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
    confirmOverwrite = false;
    if (saveFile()) {
        return;
    }
    
    running = true;
    SDL_Event event;
    
    while (running && showConfirmDialog) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
                showConfirmDialog = false;
                std::cout << "Save cancelled." << std::endl;
                break;
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
            
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                event.button.button == SDL_BUTTON_LEFT) {
                int mx = static_cast<int>(event.button.x);
                int my = static_cast<int>(event.button.y);
                if (isPointInRect(mx, my, yesButtonRect)) {
                    confirmOverwrite = true;
                    saveFile();
                    running = false;
                } else if (isPointInRect(mx, my, noButtonRect)) {
                    showConfirmDialog = false;
                    running = false;
                    std::cout << "Save cancelled." << std::endl;
                }
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_Y) {
                    confirmOverwrite = true;
                    saveFile();
                    running = false;
                } else if (event.key.key == SDLK_N || 
                           event.key.key == SDLK_ESCAPE) {
                    showConfirmDialog = false;
                    running = false;
                    std::cout << "Save cancelled." << std::endl;
                }
            }
        }
        
        if (needsRedraw) {
            render();
            needsRedraw = false;
        } else {
            SDL_Delay(1);
        }
    }
}
