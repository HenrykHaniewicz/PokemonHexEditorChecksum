#ifndef CHECKSUM_CALC_H
#define CHECKSUM_CALC_H

#include "../common/sdl_app_base.h"
#include "../common/hex_utils.h"
#include <vector>
#include <cstdint>

enum GameMode {
    GAME_POKEMON_RED_BLUE,
    GAME_POKEMON_GOLD_SILVER,
    GAME_POKEMON_CRYSTAL,
    GAME_POKEMON_GENERATION3
};

// Structure for Pokemon Red/Blue bank data
struct RedBlueBankData {
    uint32_t mainSum;
    uint8_t mainChecksum;
    uint8_t mainStoredChecksum;
    size_t mainChecksumLocation;
    bool mainMatches;
    
    uint32_t subSums[6];
    uint8_t subChecksums[6];
    uint8_t subStoredChecksums[6];
    size_t subChecksumLocations[6];
    bool subMatches[6];
};

// Structure for Pokemon Generation 3 section data
struct Gen3SectionData {
    uint16_t sectionId;
    uint32_t saveIndex;
    size_t dataSize;
    uint16_t calculatedChecksum;
    uint16_t storedChecksum;
    size_t checksumLocation;
    size_t sectionBaseAddress;
    bool matches;
};

// Structure for Pokemon Generation 3 save block
struct Gen3SaveBlock {
    Gen3SectionData sections[14];
    uint32_t saveIndex;
    bool valid;
};

class ChecksumCalculator : public SDLAppBase {
private:
    // File data
    std::string fileBuffer;
    std::string fileName;
    size_t fileSize;
    
    // Game mode and flags
    GameMode gameMode;
    std::string gameName;
    bool isJapanese;
    bool shouldWrite;
    bool shouldOverwrite;
    
    // Pokemon Red/Blue results
    uint32_t redBlueBank1Sum;
    uint8_t redBlueBank1Checksum;
    uint8_t redBlueBank1StoredChecksum;
    size_t redBlueBank1ChecksumLocation;
    size_t redBlueBank1Start;
    size_t redBlueBank1End;
    bool redBlueBank1Matches;
    RedBlueBankData redBlueBank2;
    RedBlueBankData redBlueBank3;
    
    // Pokemon Gold/Silver results
    uint32_t goldSilverTotalSum1;
    uint32_t goldSilverTotalSum2;
    uint16_t goldSilverChecksum1;
    uint16_t goldSilverChecksum2;
    uint16_t goldSilverStoredChecksum1;
    uint16_t goldSilverStoredChecksum2;
    size_t goldSilverChecksum1Location;
    size_t goldSilverChecksum2Location;
    size_t goldSilverStart1, goldSilverEnd1;
    std::vector<std::pair<size_t, size_t>> goldSilverRanges2;
    bool goldSilverChecksum1Matches;
    bool goldSilverChecksum2Matches;
    
    // Pokemon Crystal results
    uint32_t crystalTotalSum1;
    uint32_t crystalTotalSum2;
    uint16_t crystalChecksum1;
    uint16_t crystalChecksum2;
    uint16_t crystalStoredChecksum1;
    uint16_t crystalStoredChecksum2;
    size_t crystalChecksum1Location;
    size_t crystalChecksum2Location;
    size_t crystalStart1, crystalEnd1;
    size_t crystalStart2, crystalEnd2;
    bool crystalChecksum1Matches;
    bool crystalChecksum2Matches;
    
    // Pokemon Generation 3 results
    Gen3SaveBlock gen3SaveA;
    Gen3SaveBlock gen3SaveB;
    bool gen3SaveAIsCurrent;
    
protected:
    void render() override;
    void handleEvent(SDL_Event& event) override;
    
public:
    ChecksumCalculator();
    
    bool loadFile(const char* filename);
    bool setGame(const std::string& game);
    void setJapanese(bool japanese) { isJapanese = japanese; }
    void setWriteMode(bool write) { shouldWrite = write; }
    void setOverwriteMode(bool overwrite) { shouldOverwrite = overwrite; }
    bool calculateChecksum();
    
private:
    bool calculateChecksumPokemonRedBlue();
    bool calculateChecksumPokemonGoldSilver();
    bool calculateChecksumPokemonCrystal();
    bool calculateChecksumPokemonGeneration3();
    
    // Helper for Red/Blue bank calculations
    uint8_t calculateRedBlue8BitChecksum(size_t start, size_t end, uint32_t& outSum);
    void calculateRedBlueBankChecksums(size_t baseAddr, RedBlueBankData& bankData);
    
    // Helper for Gold/Silver/Crystal 16-bit calculations
    uint16_t calculateGBC16BitChecksum(size_t start, size_t end, uint32_t& outSum);
    uint16_t calculateGBC16BitChecksumMultiRange(const std::vector<std::pair<size_t, size_t>>& ranges, uint32_t& outSum);
    
    // Helper for Generation 3 calculations
    uint16_t calculateGen3SectionChecksum(size_t baseAddr, size_t dataSize);
    void calculateGen3SaveBlock(size_t blockBaseAddr, Gen3SaveBlock& saveBlock, const char* blockName);
    
    // Helper to write checksums to file
    bool writeChecksumsToFile();
    
    // Confirmation dialog for overwrite
    bool showOverwriteConfirmation();
    
    // Helper to display 16-bit value with reversed bytes
    std::string formatReversedBytes16(uint16_t value);
    
};

#endif // CHECKSUM_CALC_H