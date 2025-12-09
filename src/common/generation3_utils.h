#ifndef GENERATION3_UTILS_H
#define GENERATION3_UTILS_H

#include <cstdint>
#include <cstddef>
#include <string>

namespace Generation3Utils {
    
    // Gen 3 save structure constants
    static constexpr size_t GEN3_SAVE_SIZE = 0x20000;        // 128KB save file
    static constexpr size_t GEN3_BLOCK_SIZE = 0xE000;        // Each save block is 57344 bytes
    static constexpr size_t GEN3_SECTION_SIZE = 0x1000;      // Each section is 4096 bytes
    static constexpr size_t GEN3_NUM_SECTIONS = 14;
    
    // Section structure offsets
    static constexpr size_t GEN3_SECTION_ID_OFFSET = 0xFF4;
    static constexpr size_t GEN3_SECTION_CHECKSUM_OFFSET = 0xFF6;
    static constexpr size_t GEN3_SECTION_SAVE_INDEX_OFFSET = 0xFFC;
    
    // Security key offsets within Section 0
    static constexpr size_t GEN3_SECURITY_KEY_OFFSET_E = 0x00AC;
    static constexpr size_t GEN3_SECURITY_KEY_OFFSET_FRLG = 0x0AF8;
    
    // Game type constants for Gen 3
    enum Gen3Game {
        GEN3_GAME_RS = 0,
        GEN3_GAME_EMERALD = 1,
        GEN3_GAME_FRLG = 2
    };
    
    // Section data sizes for Pokemon Generation 3
    const size_t GEN3_SECTION_SIZES[14] = {
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
    
    // Common Gen3 section structure
    struct SectionInfo {
        uint16_t sectionId;
        uint32_t saveIndex;
        size_t dataSize;
        size_t sectionBaseAddress;
        
        // Additional fields that might be needed
        uint16_t calculatedChecksum;
        uint16_t storedChecksum;
        size_t checksumLocation;
        bool matches;
    };

    struct SaveBlock {
        SectionInfo sections[14];
        uint32_t saveIndex;
        bool valid;
    };
    
    // Pokemon data structure helpers
    uint32_t getPID(const std::string& buffer, size_t pokemonBaseAddr);
    uint32_t getOTID(const std::string& buffer, size_t pokemonBaseAddr);
    uint32_t getDecryptionKey(const std::string& buffer, size_t pokemonBaseAddr);
    uint16_t getStoredPokemonChecksum(const std::string& buffer, size_t pokemonBaseAddr);
    
    // Section helpers
    size_t findSectionOffset(const SectionInfo* sections, uint16_t sectionId);
    size_t findSectionOffset(const SectionInfo sections[], size_t numSections, uint16_t sectionId);
    
    // Security key and item encryption
    uint32_t getSecurityKey(const std::string& buffer, int game, size_t section0Offset);
    uint16_t decryptItemQuantity(uint16_t encryptedQty, int game, uint32_t securityKey);
    uint16_t encryptItemQuantity(uint16_t quantity, int game, uint32_t securityKey);
    
    // Checksum calculation
    uint16_t calculateSectionChecksum(const std::string& buffer, size_t baseAddr, size_t dataSize);
    uint16_t calculatePokemonDataChecksum(const std::string& buffer, size_t pokemonBaseAddr, uint32_t decryptionKey);
}

#endif // GENERATION3_UTILS_H