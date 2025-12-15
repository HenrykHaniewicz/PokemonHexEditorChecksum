#ifndef GENERATION3_UTILS_H
#define GENERATION3_UTILS_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <utility>

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

// Section sizes for each section ID (0-13)
constexpr size_t GEN3_SECTION_SIZES[14] = {
    3884,  // Section 0: Trainer Info
    3968,  // Section 1: Team / Items
    3968,  // Section 2: Game State
    3968,  // Section 3: Misc Data
    3848,  // Section 4: Rival Info
    3968,  // Section 5: PC Buffer A
    3968,  // Section 6: PC Buffer B
    3968,  // Section 7: PC Buffer C
    3968,  // Section 8: PC Buffer D
    3968,  // Section 9: PC Buffer E
    3968,  // Section 10: PC Buffer F
    3968,  // Section 11: PC Buffer G
    3968,  // Section 12: PC Buffer H
    2000   // Section 13: PC Buffer I
};

// Game type constants for security key handling
constexpr int GEN3_GAME_RS = 0;      // Ruby/Sapphire
constexpr int GEN3_GAME_EMERALD = 1;
constexpr int GEN3_GAME_FRLG = 2;    // FireRed/LeafGreen

// Security key offsets within Section 0
constexpr size_t GEN3_SECURITY_KEY_OFFSET_E = 0x00AC;     // Emerald
constexpr size_t GEN3_SECURITY_KEY_OFFSET_FRLG = 0x0AF8;  // FireRed/LeafGreen

// Pokemon structure offsets
constexpr size_t POKEMON_PID_OFFSET = 0x00;
constexpr size_t POKEMON_OTID_OFFSET = 0x04;
constexpr size_t POKEMON_CHECKSUM_OFFSET = 0x1C;
constexpr size_t POKEMON_DATA_OFFSET = 0x20;
constexpr size_t POKEMON_DATA_SIZE = 48;  // 12 words * 4 bytes

// Box Pokemon size (unencrypted structure in PC)
constexpr size_t BOX_POKEMON_SIZE = 80;
constexpr size_t MAX_BOX_POKEMON = 420;  // 14 boxes * 30 slots

// Section info structure
struct SectionInfo {
    uint16_t sectionId{0};
    uint32_t saveIndex{0};
    size_t dataSize{0};
    size_t sectionBaseAddress{0};
    uint16_t calculatedChecksum{0};
    uint16_t storedChecksum{0};
    size_t checksumLocation{0};
    bool matches{false};
};

// Save block structure (contains 14 sections)
struct SaveBlock {
    SectionInfo sections[14];
    uint32_t saveIndex{0};
    bool valid{false};
};

// Pokemon checksum result for individual Pokemon verification
struct PokemonChecksumResult {
    size_t location{0};
    uint16_t calculated{0};
    uint16_t stored{0};
    bool valid{false};
    std::string locationStr;
};

// ============================================================================
// Cross-Section Reader
// ============================================================================

// Class for reading data that spans across multiple save sections
// Used primarily for reading box Pokemon data which is spread across sections 5-13
class CrossSectionReader {
public:
    // Data range: pair of (physical start address, size in bytes)
    using DataRange = std::pair<size_t, size_t>;
    
    CrossSectionReader(const std::string& buffer, const std::vector<DataRange>& ranges);
    
    // Read raw bytes from logical offset
    std::vector<uint8_t> readBytes(size_t logicalOffset, size_t length) const;
    
    // Read typed values from logical offset (little-endian)
    uint8_t readU8(size_t logicalOffset) const;
    uint16_t readU16LE(size_t logicalOffset) const;
    uint32_t readU32LE(size_t logicalOffset) const;
    
    // Convert logical offset to physical address in the buffer
    size_t getPhysicalAddress(size_t logicalOffset) const;
    
    // Get total logical size across all ranges
    size_t getTotalSize() const;
    
private:
    const std::string& buffer_;
    std::vector<DataRange> ranges_;
    size_t totalSize_{0};
};

// ============================================================================
// Pokemon Data Functions
// ============================================================================

// Get Pokemon Personality ID
uint32_t getPID(const std::string& buffer, size_t pokemonBaseAddr);

// Get Pokemon Original Trainer ID
uint32_t getOTID(const std::string& buffer, size_t pokemonBaseAddr);

// Get decryption key (PID XOR OTID)
uint32_t getDecryptionKey(const std::string& buffer, size_t pokemonBaseAddr);

// Get stored checksum from Pokemon structure
uint16_t getStoredPokemonChecksum(const std::string& buffer, size_t pokemonBaseAddr);

// Calculate checksum for Pokemon data (requires decryption key)
uint16_t calculatePokemonDataChecksum(const std::string& buffer, size_t pokemonBaseAddr, uint32_t decryptionKey);

// Calculate checksum for box Pokemon using CrossSectionReader
uint16_t calculateBoxPokemonChecksum(const CrossSectionReader& reader, size_t logicalOffset, uint32_t decryptionKey);

// ============================================================================
// Section Functions
// ============================================================================

// Find section offset by ID (returns -1 if not found)
size_t findSectionOffset(const SectionInfo* sections, uint16_t sectionId);
size_t findSectionOffset(const SectionInfo sections[], size_t numSections, uint16_t sectionId);

// Calculate section checksum (32-bit sum folded to 16-bit)
uint16_t calculateSectionChecksum(const std::string& buffer, size_t baseAddr, size_t dataSize);

// Build data ranges for box Pokemon sections (sections 5-13)
std::vector<CrossSectionReader::DataRange> buildBoxDataRanges(const SaveBlock& saveBlock);

// ============================================================================
// Security/Encryption Functions
// ============================================================================

// Get security key for item quantity encryption (game-specific)
uint32_t getSecurityKey(const std::string& buffer, int game, size_t section0Offset);

// Decrypt item quantity (for Emerald and FRLG)
uint16_t decryptItemQuantity(uint16_t encryptedQty, int game, uint32_t securityKey);

// Encrypt item quantity (for Emerald and FRLG)
uint16_t encryptItemQuantity(uint16_t quantity, int game, uint32_t securityKey);

} // namespace Generation3Utils

#endif // GENERATION3_UTILS_H