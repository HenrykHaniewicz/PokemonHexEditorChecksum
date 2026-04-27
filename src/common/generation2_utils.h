#ifndef GENERATION2_UTILS_H
#define GENERATION2_UTILS_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <utility>

namespace Generation2Utils {

// Calculate the 16-bit checksum used in Gen 2 Pokemon games
// Returns the sum of bytes in the range, masked to 16 bits
uint16_t calculate16BitChecksum(const std::string& buffer, size_t start, size_t end);

// Calculate checksum and also return the raw sum (for display purposes)
uint16_t calculate16BitChecksum(const std::string& buffer, size_t start, size_t end, uint32_t& outSum);

// Calculate 16-bit checksum over multiple non-contiguous ranges
uint16_t calculate16BitChecksumMultiRange(const std::string& buffer, 
                                          const std::vector<std::pair<size_t, size_t>>& ranges);

// Calculate checksum over multiple ranges and also return the raw sum
uint16_t calculate16BitChecksumMultiRange(const std::string& buffer,
                                          const std::vector<std::pair<size_t, size_t>>& ranges,
                                          uint32_t& outSum);

// Game-specific checksum configurations
struct ChecksumConfig {
    size_t start1;
    size_t end1;
    size_t checksumLocation1;
    
    // For checksum 2 - can be single range or multiple ranges
    std::vector<std::pair<size_t, size_t>> ranges2;
    size_t checksumLocation2;
};

// Get checksum configuration for Gold/Silver
ChecksumConfig getGoldSilverConfig(bool isJapanese);

// Get checksum configuration for Crystal
ChecksumConfig getCrystalConfig(bool isJapanese);

// Trainer memory addresses
struct TrainerMemoryAddresses {
    uint32_t classNamesStart;   // Start of the trainer class name table
    uint32_t classNamesEnd;     // End of the trainer class name table (one past last byte)
    uint32_t pointerStart;      // Start of the 16 bit pointer table
    uint32_t pointerEnd;        // End of the pointer table (address of last pointer's second byte)
    uint32_t lastTrainerByte;   // Absolute offset of the final byte of the trainer data
};

static constexpr TrainerMemoryAddresses TRAINER2_ADDRESSES_GS{
    0x1B0955, // classNamesStart
    0x1B0B73, // classNamesEnd
    0x03993E, // pointerStart
    0x0399C1, // pointerEnd (last pointer begins at 0x0399C0)
    0x03B684  // lastTrainerByte
};

static constexpr TrainerMemoryAddresses TRAINER2_ADDRESSES_CRYSTAL{
    0x02C1EF, // classNamesStart
    0x02C419, // classNamesEnd
    0x039999, // pointerStart
    0x039A1E, // pointerEnd (last pointer begins at 0x039A1D)
    0x03BA66  // lastTrainerByte
};

// JP GS is only 1 MB instead of 2.
static constexpr TrainerMemoryAddresses TRAINER2_ADDRESSES_GS_J{
    0x02D2D6, // classNamesStart
    0x02D4B4, // classNamesEnd
    0x03995C, // pointerStart
    0x0399DF, // pointerEnd (last pointer begins at 0x0399DE)
    0x03B35C  // lastTrainerByte
};

static constexpr TrainerMemoryAddresses TRAINER2_ADDRESSES_CRYSTAL_J{
    0x02D319, // classNamesStart
    0x02D4FF, // classNamesEnd
    0x0399BA, // pointerStart
    0x039A3F, // pointerEnd (last pointer begins at 0x039A3E)
    0x03B6FF  // lastTrainerByte (first byte = 0x39A40)
};

} // namespace Generation2Utils

#endif // GENERATION2_UTILS_H