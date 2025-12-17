#ifndef GENERATION1_UTILS_H
#define GENERATION1_UTILS_H

#include <cstdint>
#include <cstddef>
#include <string>

namespace Generation1Utils {

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