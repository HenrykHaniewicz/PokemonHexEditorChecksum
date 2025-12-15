#include "generation2_utils.h"
#include "data_utils.h"

namespace Generation2Utils {

uint16_t calculate16BitChecksum(const std::string& buffer, size_t start, size_t end) {
    uint32_t sum = 0;
    for (size_t i = start; i <= end && i < buffer.size(); i++) {
        sum += DataUtils::readU8(buffer, i);
    }
    return sum & 0xFFFF;
}

uint16_t calculate16BitChecksum(const std::string& buffer, size_t start, size_t end, uint32_t& outSum) {
    outSum = 0;
    for (size_t i = start; i <= end && i < buffer.size(); i++) {
        outSum += DataUtils::readU8(buffer, i);
    }
    return outSum & 0xFFFF;
}

uint16_t calculate16BitChecksumMultiRange(const std::string& buffer,
                                          const std::vector<std::pair<size_t, size_t>>& ranges) {
    uint32_t sum = 0;
    for (const auto& range : ranges) {
        for (size_t i = range.first; i <= range.second && i < buffer.size(); i++) {
            sum += DataUtils::readU8(buffer, i);
        }
    }
    return sum & 0xFFFF;
}

uint16_t calculate16BitChecksumMultiRange(const std::string& buffer,
                                          const std::vector<std::pair<size_t, size_t>>& ranges,
                                          uint32_t& outSum) {
    outSum = 0;
    for (const auto& range : ranges) {
        for (size_t i = range.first; i <= range.second && i < buffer.size(); i++) {
            outSum += DataUtils::readU8(buffer, i);
        }
    }
    return outSum & 0xFFFF;
}

ChecksumConfig getGoldSilverConfig(bool isJapanese) {
    ChecksumConfig config;
    
    if (isJapanese) {
        config.start1 = 0x2009;
        config.end1 = 0x2C8B;
        config.checksumLocation1 = 0x2D0D;
        
        config.ranges2 = {{0x7209, 0x7E8B}};
        config.checksumLocation2 = 0x7F0D;
    } else {
        config.start1 = 0x2009;
        config.end1 = 0x2D68;
        config.checksumLocation1 = 0x2D69;
        
        config.ranges2 = {
            {0x0C6B, 0x17EC},
            {0x3D96, 0x3F3F},
            {0x7E39, 0x7E6C}
        };
        config.checksumLocation2 = 0x7E6D;
    }
    
    return config;
}

ChecksumConfig getCrystalConfig(bool isJapanese) {
    ChecksumConfig config;
    
    config.start1 = 0x2009;
    config.checksumLocation1 = 0x2D0D;
    
    if (isJapanese) {
        config.end1 = 0x2AE2;
        config.ranges2 = {{0x7209, 0x7CE2}};
        config.checksumLocation2 = 0x7F0D;
    } else {
        config.end1 = 0x2B82;
        config.ranges2 = {{0x1209, 0x1D82}};
        config.checksumLocation2 = 0x1F0D;
    }
    
    return config;
}

} // namespace Generation2Utils