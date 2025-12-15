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

} // namespace Generation2Utils

#endif // GENERATION2_UTILS_H