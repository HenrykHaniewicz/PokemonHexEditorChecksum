#ifndef GENERATION1_UTILS_H
#define GENERATION1_UTILS_H

#include <cstdint>
#include <cstddef>
#include <string>

namespace Generation1Utils {

struct TrainerMemoryAddresses {
    uint32_t classNamesStart;   // Start of the trainer class name table
    uint32_t classNamesEnd;     // End of the trainer class name table (one past last byte)
    uint32_t pointerStart;      // Start of the 16 bit pointer table
    uint32_t pointerEnd;        // End of the pointer table (address of last pointer's second byte)
    uint32_t lastTrainerByte;   // Absolute offset of the final byte of the trainer data
};

static constexpr uint32_t TRAINER1_POINTER_BASE = 0x34000;

static constexpr TrainerMemoryAddresses TRAINER1_ADDRESSES_RB{
    0x399FF, // classNamesStart
    0x39B86, // classNamesEnd
    0x39D3B, // pointerStart
    0x39D98, // pointerEnd (last pointer begins at 0x39D97)
    0x3A52D  // lastTrainerByte
};

static constexpr TrainerMemoryAddresses TRAINER1_ADDRESSES_YELLOW{
    0x3997E, // classNamesStart
    0x39B05, // classNamesEnd
    0x39DD1, // pointerStart
    0x39E2E, // pointerEnd (last pointer begins at 0x39E2D)
    0x3A5B1  // lastTrainerByte
};


static constexpr TrainerMemoryAddresses TRAINER1_ADDRESSES_RG_J{
    0x39D1C, // classNamesStart
    0x39E5E, // classNamesEnd
    0x3A0AC, // pointerStart
    0x3A109, // pointerEnd (last pointer begins at 0x3A108)
    0x3A89E  // lastTrainerByte
};

static constexpr TrainerMemoryAddresses TRAINER1_ADDRESSES_Blue_J{
    0x39DB5, // classNamesStart
    0x39EF7, // classNamesEnd
    0x3A0AC, // pointerStart
    0x3A109, // pointerEnd (last pointer begins at 0x3A108)
    0x3A89E  // lastTrainerByte
};

static constexpr TrainerMemoryAddresses TRAINER1_ADDRESSES_YELLOW_J{
    0x39D34, // classNamesStart
    0x39E76, // classNamesEnd
    0x3A142, // pointerStart
    0x3A19F, // pointerEnd (last pointer begins at 0x3A19E)
    0x3A922  // lastTrainerByte
};

// Calculate the 8-bit checksum used in Gen 1 Pokemon games
// Returns the complement of the sum of bytes in the range
uint8_t calculate8BitChecksum(const std::string& buffer, size_t start, size_t end);

// Calculate checksum and also return the raw sum (for display purposes)
uint8_t calculate8BitChecksum(const std::string& buffer, size_t start, size_t end, uint32_t& outSum);

// Bank checksum data structure for Gen 1 box banks
struct BankChecksumData {
    // Main checksum
    uint32_t mainSum{0};
    uint8_t mainChecksum{0};
    uint8_t mainStoredChecksum{0};
    size_t mainChecksumLocation{0};
    bool mainMatches{false};
    
    // Sub-checksums (6 sub-regions per bank)
    uint32_t subSums[6]{0};
    uint8_t subChecksums[6]{0};
    uint8_t subStoredChecksums[6]{0};
    size_t subChecksumLocations[6]{0};
    bool subMatches[6]{false};
};

struct ChecksumConfig {
    size_t start;
    size_t end;
    size_t checksumLocation;
};

ChecksumConfig getRedBlueYellowConfig(bool isJapanese);

// Calculate all checksums for a Gen 1 box bank (Banks 2 and 3)
// baseAddr should be 0x4000 for Bank 2 or 0x6000 for Bank 3
void calculateBankChecksums(const std::string& buffer, size_t baseAddr, BankChecksumData& bankData);

// Check if a bank is unused (filled with 0xFF)
bool isBankUnused(const std::string& buffer, size_t baseAddr);

} // namespace Generation1Utils

#endif // GENERATION1_UTILS_H