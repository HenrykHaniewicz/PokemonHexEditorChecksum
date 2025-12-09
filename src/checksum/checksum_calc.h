#ifndef CHECKSUM_CALC_H
#define CHECKSUM_CALC_H

#include "../common/sdl_app_base.h"
#include "../common/hex_utils.h"
#include "../common/data_utils.h"
#include "../common/generation3_utils.h"
#include <vector>
#include <cstdint>

enum GameMode {
    GAME_POKEMON_RED_BLUE,
    GAME_POKEMON_GOLD_SILVER,
    GAME_POKEMON_CRYSTAL,
    GAME_POKEMON_GENERATION3
};

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

struct PokemonChecksumResult {
    size_t location;
    uint16_t calculated;
    uint16_t stored;
    bool valid;
    std::string locationStr;
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
    Generation3Utils::SaveBlock gen3SaveA;
    Generation3Utils::SaveBlock gen3SaveB;
    bool gen3SaveAIsCurrent;

    // Pokemon checksum mode
    bool pokemonChecksumMode;

    std::vector<PokemonChecksumResult> pokemonResultsSaveA;
    std::vector<PokemonChecksumResult> pokemonResultsSaveB;

    // Low-level buffer read/write helpers
    uint8_t readU8(size_t offset) const;
    uint16_t readU16LE(size_t offset) const;
    uint32_t readU32LE(size_t offset) const;
    void writeU16LE(std::string& buffer, size_t offset, uint16_t value);
    
    // Pokemon data structure helpers
    uint16_t calculatePokemonDataChecksum(size_t pokemonBaseAddr, uint32_t decryptionKey) const;
    PokemonChecksumResult calculatePokemonChecksumResult(size_t pokemonBaseAddr, const std::string& locationStr) const;
    
    // Game-specific checksum calculations
    bool calculateChecksumPokemonRedBlue();
    bool calculateChecksumPokemonGoldSilver();
    bool calculateChecksumPokemonCrystal();
    bool calculateChecksumPokemonGeneration3();
    
    uint8_t calculateRedBlue8BitChecksum(size_t start, size_t end, uint32_t& outSum);
    void calculateRedBlueBankChecksums(size_t baseAddr, RedBlueBankData& bankData);
    
    uint16_t calculateGBC16BitChecksum(size_t start, size_t end, uint32_t& outSum);
    uint16_t calculateGBC16BitChecksumMultiRange(const std::vector<std::pair<size_t, size_t>>& ranges, uint32_t& outSum);
    
    uint16_t calculateGen3SectionChecksum(size_t baseAddr, size_t dataSize);
    void calculateGen3SaveBlock(size_t blockBaseAddr, Generation3Utils::SaveBlock& saveBlock, const char* blockName);

    bool calculatePokemonChecksum();

    size_t findSectionOffset(const Generation3Utils::SaveBlock& saveBlock, uint16_t sectionId);
    
    void calculateAllPokemonChecksums(const Generation3Utils::SaveBlock& saveBlock, 
                                     std::vector<PokemonChecksumResult>& results,
                                     const std::string& saveBlockName);
    
    void calculatePartyPokemonChecksums(const Generation3Utils::SaveBlock& saveBlock,
                                       std::vector<PokemonChecksumResult>& results,
                                       const std::string& saveBlockName);
    
    void calculateBoxPokemonChecksums(const Generation3Utils::SaveBlock& saveBlock,
                                     std::vector<PokemonChecksumResult>& results,
                                     const std::string& saveBlockName);
    
    // Writing and formatting
    bool writeChecksumsToFile();
    std::string formatReversedBytes16(uint16_t value);
    
protected:
    void render() override;
    void handleEvent(SDL_Event& event) override;
    void update(float deltaTime) override;
    
public:
    ChecksumCalculator();
    
    bool loadFile(const char* filename);
    bool setGame(const std::string& game);
    void setJapanese(bool japanese) { isJapanese = japanese; }
    void setWriteMode(bool write) { shouldWrite = write; }
    void setOverwriteMode(bool overwrite) { shouldOverwrite = overwrite; }
    void setPokemonMode(bool pokemon) { pokemonChecksumMode = pokemon; }
    bool calculateChecksum();
};

#endif // CHECKSUM_CALC_H