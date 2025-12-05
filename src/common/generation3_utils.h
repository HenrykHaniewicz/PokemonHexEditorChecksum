#ifndef GENERATION3_UTILS_H
#define GENERATION3_UTILS_H

#include <cstdint>
#include <cstddef>
#include <string>

namespace Generation3Utils {
    
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
    
    // Pokemon data structure helpers
    uint32_t getPID(const std::string& buffer, size_t pokemonBaseAddr);
    uint32_t getOTID(const std::string& buffer, size_t pokemonBaseAddr);
    uint32_t getDecryptionKey(const std::string& buffer, size_t pokemonBaseAddr);
    uint16_t getStoredPokemonChecksum(const std::string& buffer, size_t pokemonBaseAddr);
    
    // Section helpers
    size_t findSectionOffset(const SectionInfo* sections, uint16_t sectionId);
    size_t findSectionOffset(const SectionInfo sections[], size_t numSections, uint16_t sectionId);
    
    // Checksum calculation
    uint16_t calculateSectionChecksum(const std::string& buffer, size_t baseAddr, size_t dataSize);
    uint16_t calculatePokemonDataChecksum(const std::string& buffer, size_t pokemonBaseAddr, uint32_t decryptionKey);
}

#endif // GENERATION3_UTILS_H