#ifndef MIRAGE_ISLAND_H
#define MIRAGE_ISLAND_H

#include "../common/sdl_app_base.h"
#include "../common/hex_utils.h"
#include "../common/data_utils.h"
#include "../common/generation3_utils.h"
#include <vector>
#include <cstdint>
#include <string>

enum MirageIslandGame {
    MIRAGE_GAME_RUBY_SAPPHIRE,
    MIRAGE_GAME_EMERALD,
    MIRAGE_GAME_INVALID
};

class MirageIslandEditor : public SDLAppBase {
private:
    // File data
    std::string fileBuffer;
    std::string fileName;
    size_t fileSize;
    
    // Game mode
    MirageIslandGame gameMode;
    std::string gameName;
    
    // Flags
    bool shouldOverwrite;
    bool operationComplete;
    bool operationSuccess;
    std::string errorMessage;
    
    // Save block info
    Generation3Utils::SectionInfo saveASections[14];
    Generation3Utils::SectionInfo saveBSections[14];
    uint32_t saveAIndex;
    uint32_t saveBIndex;
    bool saveAIsCurrent;
    
    // Operation results
    uint8_t pidByte1;
    uint8_t pidByte2;
    uint16_t originalMirageValue;
    uint16_t newMirageValue;
    uint16_t originalChecksum;
    uint16_t newChecksum;
    size_t pidOffset;
    size_t mirageIslandOffset;
    size_t checksumOffset;
    size_t section1Offset;
    size_t section2Offset;
    
    // Output file path
    std::string outputFilePath;
    
    // Low-level buffer read/write helpers
    uint8_t readU8(size_t offset) const;
    uint16_t readU16LE(size_t offset) const;
    uint32_t readU32LE(size_t offset) const;
    void writeU8(std::string& buffer, size_t offset, uint8_t value);
    void writeU16LE(std::string& buffer, size_t offset, uint16_t value);
    
    // Section handling
    void parseSaveBlock(size_t blockBaseAddr, Generation3Utils::SectionInfo* sections, uint32_t& saveIndex);
    size_t findSectionOffset(const Generation3Utils::SectionInfo* sections, uint16_t sectionId);
    
    // Core operations
    bool determineCurrentSave();
    bool performMirageIslandEdit();
    bool writeToFile();
    
    // Formatting helpers
    std::string formatReversedBytes16(uint16_t value);
    
protected:
    void render() override;
    void handleEvent(SDL_Event& event) override;
    void update(float deltaTime) override;
    
public:
    MirageIslandEditor();
    
    bool loadFile(const char* filename);
    bool setGame(const std::string& game);
    void setOverwriteMode(bool overwrite) { shouldOverwrite = overwrite; }
    bool execute();
};

#endif // MIRAGE_ISLAND_H