#include "generation1_utils.h"
#include "data_utils.h"

namespace Generation1Utils {

uint8_t calculate8BitChecksum(const std::string& buffer, size_t start, size_t end) {
    uint32_t sum = 0;
    for (size_t i = start; i <= end && i < buffer.size(); i++) {
        sum += DataUtils::readU8(buffer, i);
    }
    uint8_t sumMod = sum & 0xFF;
    return ~sumMod;
}

uint8_t calculate8BitChecksum(const std::string& buffer, size_t start, size_t end, uint32_t& outSum) {
    outSum = 0;
    for (size_t i = start; i <= end && i < buffer.size(); i++) {
        outSum += DataUtils::readU8(buffer, i);
    }
    uint8_t sumMod = outSum & 0xFF;
    return ~sumMod;
}

bool isBankUnused(const std::string& buffer, size_t baseAddr) {
    const size_t mainStart = 0x0000;
    const size_t mainEnd = 0x1A4B;
    
    for (size_t i = baseAddr + mainStart; i <= baseAddr + mainEnd && i < buffer.size(); i++) {
        if (DataUtils::readU8(buffer, i) != 0xFF) {
            return false;
        }
    }
    return true;
}

void calculateBankChecksums(const std::string& buffer, size_t baseAddr, BankChecksumData& bankData) {
    const size_t mainStart = 0x0000;
    const size_t mainEnd = 0x1A4B;
    const size_t mainChecksumOffset = 0x1A4C;
    
    const size_t subRanges[6][2] = {
        {0x0000, 0x0461},
        {0x0462, 0x08C3},
        {0x08C4, 0x0D25},
        {0x0D26, 0x1187},
        {0x1188, 0x15E9},
        {0x15EA, 0x1A4B}
    };
    const size_t subChecksumOffsets[6] = {0x1A4D, 0x1A4E, 0x1A4F, 0x1A50, 0x1A51, 0x1A52};
    
    bool isAllFF = isBankUnused(buffer, baseAddr);
    
    // Main checksum
    bankData.mainChecksum = calculate8BitChecksum(
        buffer,
        baseAddr + mainStart,
        baseAddr + mainEnd,
        bankData.mainSum
    );
    bankData.mainChecksumLocation = baseAddr + mainChecksumOffset;
    bankData.mainStoredChecksum = DataUtils::readU8(buffer, bankData.mainChecksumLocation);
    
    if (isAllFF) {
        bankData.mainMatches = true;
    } else {
        bankData.mainMatches = (bankData.mainChecksum == bankData.mainStoredChecksum);
    }
    
    // Sub-checksums
    for (int i = 0; i < 6; i++) {
        bankData.subChecksums[i] = calculate8BitChecksum(
            buffer,
            baseAddr + subRanges[i][0],
            baseAddr + subRanges[i][1],
            bankData.subSums[i]
        );
        bankData.subChecksumLocations[i] = baseAddr + subChecksumOffsets[i];
        bankData.subStoredChecksums[i] = DataUtils::readU8(buffer, bankData.subChecksumLocations[i]);
        
        if (isAllFF) {
            bankData.subMatches[i] = true;
        } else {
            bankData.subMatches[i] = (bankData.subChecksums[i] == bankData.subStoredChecksums[i]);
        }
    }
}

} // namespace Generation1Utils