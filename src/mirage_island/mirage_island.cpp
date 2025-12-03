#include "mirage_island.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <sys/stat.h>

// Section data sizes for Pokemon Generation 3
static const size_t GEN3_SECTION_SIZES[14] = {
    3884,  // ID 0: Trainer info
    3968,  // ID 1: Team / items
    3968,  // ID 2: Game State
    3968,  // ID 3: Misc Data
    3848,  // ID 4: Rival info
    3968,  // ID 5: PC buffer A
    3968,  // ID 6: PC buffer B
    3968,  // ID 7: PC buffer C
    3968,  // ID 8: PC buffer D
    3968,  // ID 9: PC buffer E
    3968,  // ID 10: PC buffer F
    3968,  // ID 11: PC buffer G
    3968,  // ID 12: PC buffer H
    2000   // ID 13: PC buffer I
};

// Mirage Island random number offsets within Section 2
static const size_t MIRAGE_OFFSET_EMERALD = 0x464;
static const size_t MIRAGE_OFFSET_RUBY_SAPPHIRE = 0x408;

// PID offset within Section 1 (first Pokemon in party)
static const size_t PID_OFFSET_SECTION1 = 0x238;

// ============================================================================
// Constructor
// ============================================================================

MirageIslandEditor::MirageIslandEditor()
    : SDLAppBase("Mirage Island Editor", 550, 680),
      fileSize(0),
      gameMode(MIRAGE_GAME_INVALID),
      shouldOverwrite(false),
      operationComplete(false),
      operationSuccess(false),
      saveAIndex(0),
      saveBIndex(0),
      saveAIsCurrent(false),
      pidByte1(0),
      pidByte2(0),
      originalMirageValue(0),
      newMirageValue(0),
      originalChecksum(0),
      newChecksum(0),
      pidOffset(0),
      mirageIslandOffset(0),
      checksumOffset(0),
      section1Offset(0),
      section2Offset(0) {
    
    memset(saveASections, 0, sizeof(saveASections));
    memset(saveBSections, 0, sizeof(saveBSections));
}

// ============================================================================
// Low-level Buffer Read/Write Helpers
// ============================================================================

uint8_t MirageIslandEditor::readU8(size_t offset) const {
    return DataUtils::readU8(fileBuffer, offset);
}

uint16_t MirageIslandEditor::readU16LE(size_t offset) const {
    return DataUtils::readU16LE(fileBuffer, offset);
}

uint32_t MirageIslandEditor::readU32LE(size_t offset) const {
    return DataUtils::readU32LE(fileBuffer, offset);
}

void MirageIslandEditor::writeU8(std::string& buffer, size_t offset, uint8_t value) {
    DataUtils::writeU8(buffer, offset, value);
}

void MirageIslandEditor::writeU16LE(std::string& buffer, size_t offset, uint16_t value) {
    DataUtils::writeU16LE(buffer, offset, value);
}

// ============================================================================
// Section Handling
// ============================================================================

void MirageIslandEditor::parseSaveBlock(size_t blockBaseAddr, Gen3SectionInfo* sections, uint32_t& saveIndex) {
    for (int i = 0; i < 14; i++) {
        size_t sectionBase = blockBaseAddr + (i * 0x1000);
        
        sections[i].sectionId = readU16LE(sectionBase + 0x0FF4);
        sections[i].sectionBaseAddress = sectionBase;
        sections[i].saveIndex = readU32LE(sectionBase + 0x0FFC);
    }
    
    saveIndex = sections[13].saveIndex;
}

size_t MirageIslandEditor::findSectionOffset(const Gen3SectionInfo* sections, uint16_t sectionId) {
    for (int i = 0; i < 14; i++) {
        if (sections[i].sectionId == sectionId) {
            return sections[i].sectionBaseAddress;
        }
    }
    return static_cast<size_t>(-1);
}

// ============================================================================
// Checksum Calculation
// ============================================================================

uint16_t MirageIslandEditor::calculateGen3SectionChecksum(size_t baseAddr, size_t dataSize) {
    uint32_t sum = 0;
    
    for (size_t i = 0; i < dataSize; i += 4) {
        sum += readU32LE(baseAddr + i);
    }
    
    uint16_t upper = (sum >> 16) & 0xFFFF;
    uint16_t lower = sum & 0xFFFF;
    
    return upper + lower;
}

// ============================================================================
// Public Interface Functions
// ============================================================================

bool MirageIslandEditor::loadFile(const char* filename) {
    if (!HexUtils::loadFileToBuffer(filename, fileBuffer, fileSize)) {
        errorMessage = "Failed to open file: " + std::string(filename);
        std::cerr << errorMessage << std::endl;
        return false;
    }
    fileName = filename;
    return true;
}

bool MirageIslandEditor::setGame(const std::string& game) {
    std::string g = game;
    std::transform(g.begin(), g.end(), g.begin(), ::tolower);
    
    if (g == "ruby" || g == "pokemon_ruby") {
        gameMode = MIRAGE_GAME_RUBY_SAPPHIRE;
        gameName = "Pokemon Ruby";
    } else if (g == "sapphire" || g == "pokemon_sapphire") {
        gameMode = MIRAGE_GAME_RUBY_SAPPHIRE;
        gameName = "Pokemon Sapphire";
    } else if (g == "emerald" || g == "pokemon_emerald") {
        gameMode = MIRAGE_GAME_EMERALD;
        gameName = "Pokemon Emerald";
    } else {
        gameMode = MIRAGE_GAME_INVALID;
        errorMessage = "Mirage Island only exists in Ruby, Sapphire, and Emerald";
        std::cerr << "Error: " << errorMessage << std::endl;
        std::cerr << "Provided game: " << game << std::endl;
        return false;
    }
    
    return true;
}

bool MirageIslandEditor::determineCurrentSave() {
    parseSaveBlock(0x000000, saveASections, saveAIndex);
    parseSaveBlock(0x00E000, saveBSections, saveBIndex);
    
    saveAIsCurrent = (saveAIndex >= saveBIndex);
    
    std::cout << "Save A index: " << std::dec << saveAIndex << std::endl;
    std::cout << "Save B index: " << std::dec << saveBIndex << std::endl;
    std::cout << "Current save: " << (saveAIsCurrent ? "A" : "B") << std::endl;
    
    return true;
}

bool MirageIslandEditor::performMirageIslandEdit() {
    const Gen3SectionInfo* currentSections = saveAIsCurrent ? saveASections : saveBSections;
    
    section1Offset = findSectionOffset(currentSections, 1);
    section2Offset = findSectionOffset(currentSections, 2);
    
    if (section1Offset == static_cast<size_t>(-1)) {
        errorMessage = "Could not find Section 1 in current save";
        std::cerr << "Error: " << errorMessage << std::endl;
        return false;
    }
    
    if (section2Offset == static_cast<size_t>(-1)) {
        errorMessage = "Could not find Section 2 in current save";
        std::cerr << "Error: " << errorMessage << std::endl;
        return false;
    }
    
    std::cout << "\nSection 1 base address: 0x" << HexUtils::toHexString(section1Offset, 5) << std::endl;
    std::cout << "Section 2 base address: 0x" << HexUtils::toHexString(section2Offset, 5) << std::endl;
    
    pidOffset = section1Offset + PID_OFFSET_SECTION1;
    pidByte1 = readU8(pidOffset);
    pidByte2 = readU8(pidOffset + 1);
    
    std::cout << "\nPID location: 0x" << HexUtils::toHexString(pidOffset, 5) << std::endl;
    std::cout << "PID bytes (first 2): " << HexUtils::toHexString(pidByte1, 2) 
              << " " << HexUtils::toHexString(pidByte2, 2) << std::endl;
    
    size_t mirageOffsetInSection;
    if (gameMode == MIRAGE_GAME_EMERALD) {
        mirageOffsetInSection = MIRAGE_OFFSET_EMERALD;
    } else {
        mirageOffsetInSection = MIRAGE_OFFSET_RUBY_SAPPHIRE;
    }
    
    mirageIslandOffset = section2Offset + mirageOffsetInSection;
    
    originalMirageValue = readU16LE(mirageIslandOffset);
    
    std::cout << "\nMirage Island location: 0x" << HexUtils::toHexString(mirageIslandOffset, 5) << std::endl;
    std::cout << "Original Mirage Island value: 0x" << HexUtils::toHexString(originalMirageValue, 4)
              << " (bytes: " << HexUtils::toHexString(originalMirageValue & 0xFF, 2) 
              << " " << HexUtils::toHexString((originalMirageValue >> 8) & 0xFF, 2) << ")" << std::endl;
    
    newMirageValue = static_cast<uint16_t>(pidByte1) | (static_cast<uint16_t>(pidByte2) << 8);
    writeU8(fileBuffer, mirageIslandOffset, pidByte1);
    writeU8(fileBuffer, mirageIslandOffset + 1, pidByte2);
    
    std::cout << "New Mirage Island value: 0x" << HexUtils::toHexString(newMirageValue, 4)
              << " (bytes: " << HexUtils::toHexString(pidByte1, 2) 
              << " " << HexUtils::toHexString(pidByte2, 2) << ")" << std::endl;
    
    checksumOffset = section2Offset + 0x0FF6;
    originalChecksum = readU16LE(checksumOffset);
    
    newChecksum = calculateGen3SectionChecksum(section2Offset, GEN3_SECTION_SIZES[2]);
    
    std::cout << "\nSection 2 checksum location: 0x" << HexUtils::toHexString(checksumOffset, 5) << std::endl;
    std::cout << "Original checksum: 0x" << HexUtils::toHexString(originalChecksum, 4)
              << " (bytes: " << formatReversedBytes16(originalChecksum) << ")" << std::endl;
    std::cout << "New checksum: 0x" << HexUtils::toHexString(newChecksum, 4)
              << " (bytes: " << formatReversedBytes16(newChecksum) << ")" << std::endl;
    
    writeU16LE(fileBuffer, checksumOffset, newChecksum);
    
    return true;
}

bool MirageIslandEditor::writeToFile() {
    if (shouldOverwrite) {
        ConfirmDialogConfig config;
        config.title = "WARNING";
        config.message1 = "Overwrite this file?";
        config.message2 = HexUtils::getBaseName(fileName);
        
        if (!showConfirmDialog(config)) {
            std::cout << "\nOverwrite cancelled. Exiting program." << std::endl;
            operationSuccess = false;
            errorMessage = "Overwrite cancelled by user";
            return false;
        }
        
        outputFilePath = fileName;
    } else {
        mkdir("edited_files", 0755);
        
        std::string baseName = HexUtils::getBaseName(fileName);
        size_t dotPos = baseName.rfind('.');
        std::string nameWithoutExt = baseName.substr(0, dotPos);
        std::string extension = (dotPos != std::string::npos) ? baseName.substr(dotPos) : "";
        outputFilePath = "edited_files/" + nameWithoutExt + "_mirage" + extension;
    }
    
    std::ofstream outFile(outputFilePath, std::ios::binary);
    if (!outFile) {
        errorMessage = "Failed to create output file: " + outputFilePath;
        std::cerr << errorMessage << std::endl;
        return false;
    }
    
    outFile.write(fileBuffer.c_str(), static_cast<std::streamsize>(fileSize));
    outFile.close();
    
    if (shouldOverwrite) {
        std::cout << "\nMirage Island edit complete (file overwritten): " << outputFilePath << std::endl;
    } else {
        std::cout << "\nMirage Island edit saved to: " << outputFilePath << std::endl;
    }
    
    return true;
}

bool MirageIslandEditor::execute() {
    if (gameMode == MIRAGE_GAME_INVALID) {
        return false;
    }
    
    const size_t requiredSize = 0x20000;
    
    if (fileSize < requiredSize) {
        std::stringstream ss;
        ss << "File too small (size: 0x" << std::hex << fileSize 
           << ", need at least 0x" << requiredSize << ")";
        errorMessage = ss.str();
        std::cerr << "Error: " << errorMessage << std::endl;
        return false;
    }
    
    std::cout << "\n=== Mirage Island Editor - " << gameName << " ===" << std::endl;
    std::cout << "File: " << fileName << " (" << std::dec << fileSize << " bytes)" << std::endl;
    
    if (!determineCurrentSave()) {
        return false;
    }
    
    if (!performMirageIslandEdit()) {
        return false;
    }
    
    operationSuccess = writeToFile();
    operationComplete = true;
    
    std::cout << "\n=== Operation Complete ===" << std::endl;
    
    return operationSuccess;
}

// ============================================================================
// Formatting Helpers
// ============================================================================

std::string MirageIslandEditor::formatReversedBytes16(uint16_t value) {
    uint8_t low = value & 0xFF;
    uint8_t high = (value >> 8) & 0xFF;
    std::stringstream ss;
    ss << HexUtils::toHexString(low, 2) << " " << HexUtils::toHexString(high, 2);
    return ss.str();
}

// ============================================================================
// Event Handling
// ============================================================================

void MirageIslandEditor::handleEvent(SDL_Event& event) {
    if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_ESCAPE || event.key.key == SDLK_Q) {
            quit();
        }
    }
}

void MirageIslandEditor::update(float deltaTime) {
    SDLAppBase::update(deltaTime);
}

// ============================================================================
// Rendering
// ============================================================================

void MirageIslandEditor::render() {
    SDL_SetRenderDrawColor(renderer, colors.background.r, colors.background.g, 
                          colors.background.b, 255);
    SDL_RenderClear(renderer);
    
    int y = 15;
    std::stringstream ss;
    
    // Title
    renderCenteredText("MIRAGE ISLAND EDITOR", y, colors.accent);
    y += charHeight + 5;
    renderCenteredText(gameName, y, colors.highlight);
    y += charHeight + 10;
    
    // File info
    std::string baseName = HexUtils::getBaseName(fileName);
    renderCenteredText("File: " + baseName, y, colors.text);
    y += charHeight + 10;
    
    renderLine(30, y, windowWidth - 30, y, {80, 80, 80, 255});
    y += 15;
    
    // If operation failed early, show error
    if (!operationComplete && !errorMessage.empty()) {
        renderCenteredText("ERROR", y, colors.error, largeFont);
        y += 55;
        renderCenteredText(errorMessage, y, colors.error);
        renderCenteredText("Press ESC or Q to quit", windowHeight - charHeight - 15, colors.textDim);
        SDL_RenderPresent(renderer);
        return;
    }
    
    // Save block information
    renderCenteredText("=== Save Block Info ===", y, colors.accent);
    y += charHeight + 8;
    
    ss.str("");
    ss << "Save A Index: " << std::dec << saveAIndex << "    Save B Index: " << saveBIndex;
    renderCenteredText(ss.str(), y, colors.text);
    y += charHeight + 5;
    
    ss.str("");
    ss << "Current Save: " << (saveAIsCurrent ? "A" : "B");
    renderCenteredText(ss.str(), y, colors.warning);
    y += charHeight + 15;
    
    // Section locations
    renderCenteredText("=== Section Locations ===", y, colors.accent);
    y += charHeight + 8;
    
    ss.str("");
    ss << "Section 1 (Team/Items): 0x" << HexUtils::toHexString(section1Offset, 5);
    renderCenteredText(ss.str(), y, colors.text);
    y += charHeight + 3;
    
    ss.str("");
    ss << "Section 2 (Game State): 0x" << HexUtils::toHexString(section2Offset, 5);
    renderCenteredText(ss.str(), y, colors.text);
    y += charHeight + 15;
    
    // PID information
    renderCenteredText("=== Party Pokemon PID ===", y, colors.accent);
    y += charHeight + 8;
    
    ss.str("");
    ss << "PID Location: 0x" << HexUtils::toHexString(pidOffset, 5);
    renderCenteredText(ss.str(), y, colors.text);
    y += charHeight + 3;
    
    ss.str("");
    ss << "PID Low Bytes: " << HexUtils::toHexString(pidByte1, 2) 
       << " " << HexUtils::toHexString(pidByte2, 2);
    renderCenteredText(ss.str(), y, colors.highlight);
    y += charHeight + 15;
    
    // Mirage Island value change
    renderCenteredText("=== Mirage Island Value ===", y, colors.accent);
    y += charHeight + 8;
    
    ss.str("");
    ss << "Location: 0x" << HexUtils::toHexString(mirageIslandOffset, 5);
    renderCenteredText(ss.str(), y, colors.text);
    y += charHeight + 5;
    
    ss.str("");
    ss << "0x" << HexUtils::toHexString(originalMirageValue, 4);
    renderText(ss.str(), windowWidth / 2 - 80, y, colors.textDim);
    renderText("->", windowWidth / 2 - 15, y, colors.text);
    ss.str("");
    ss << "0x" << HexUtils::toHexString(newMirageValue, 4);
    renderText(ss.str(), windowWidth / 2 + 30, y, colors.success);
    y += charHeight + 15;
    
    // Checksum update
    renderCenteredText("=== Section 2 Checksum ===", y, colors.accent);
    y += charHeight + 8;
    
    ss.str("");
    ss << "Location: 0x" << HexUtils::toHexString(checksumOffset, 5);
    renderCenteredText(ss.str(), y, colors.text);
    y += charHeight + 5;
    
    ss.str("");
    ss << "0x" << HexUtils::toHexString(originalChecksum, 4);
    renderText(ss.str(), windowWidth / 2 - 80, y, colors.textDim);
    renderText("->", windowWidth / 2 - 15, y, colors.text);
    ss.str("");
    ss << "0x" << HexUtils::toHexString(newChecksum, 4);
    renderText(ss.str(), windowWidth / 2 + 30, y, colors.success);
    y += charHeight + 20;
    
    // Status
    renderLine(30, y, windowWidth - 30, y, {80, 80, 80, 255});
    y += 15;
    
    if (operationComplete) {
        if (operationSuccess) {
            renderCenteredText("SUCCESS", y, colors.success, largeFont);
            y += 65;
            
            if (shouldOverwrite) {
                renderCenteredText("File overwritten", y, colors.text);
            } else {
                renderCenteredText("Saved to: " + outputFilePath, y, colors.text);
            }
        } else {
            renderCenteredText("FAILED", y, colors.error, largeFont);
            y += 65;
            if (!errorMessage.empty()) {
                renderCenteredText(errorMessage, y, colors.error);
            }
        }
    }
    
    renderCenteredText("Press ESC or Q to quit", windowHeight - charHeight - 15, colors.textDim);
    
    SDL_RenderPresent(renderer);
}