#include "generation3_utils.h"
#include "data_utils.h"
#include <algorithm>

namespace Generation3Utils {

// ============================================================================
// CrossSectionReader Implementation
// ============================================================================

CrossSectionReader::CrossSectionReader(const std::string& buffer, const std::vector<DataRange>& ranges)
    : buffer_(buffer), ranges_(ranges), totalSize_(0) {
    for (const auto& range : ranges_) {
        totalSize_ += range.second;
    }
}

std::vector<uint8_t> CrossSectionReader::readBytes(size_t logicalOffset, size_t length) const {
    std::vector<uint8_t> data(length, 0);
    size_t bytesRead = 0;
    size_t currentLogicalPos = 0;
    size_t currentLogicalStart = logicalOffset;
    
    for (const auto& range : ranges_) {
        if (bytesRead >= length) break;
        
        size_t rangeEnd = currentLogicalPos + range.second;
        
        // Check if the requested data starts within this range
        if (currentLogicalStart >= currentLogicalPos && currentLogicalStart < rangeEnd) {
            size_t offsetInRange = currentLogicalStart - currentLogicalPos;
            size_t bytesToRead = std::min(length - bytesRead, range.second - offsetInRange);
            
            for (size_t i = 0; i < bytesToRead; i++) {
                size_t physicalAddr = range.first + offsetInRange + i;
                if (physicalAddr < buffer_.size()) {
                    data[bytesRead + i] = DataUtils::readU8(buffer_, physicalAddr);
                }
            }
            bytesRead += bytesToRead;
            currentLogicalStart += bytesToRead;
        }
        // Check if we need to continue reading into this range
        else if (currentLogicalStart < rangeEnd && bytesRead > 0) {
            size_t offsetInRange = 0;
            if (currentLogicalStart > currentLogicalPos) {
                offsetInRange = currentLogicalStart - currentLogicalPos;
            }
            size_t bytesToRead = std::min(length - bytesRead, range.second - offsetInRange);
            
            for (size_t i = 0; i < bytesToRead; i++) {
                size_t physicalAddr = range.first + offsetInRange + i;
                if (physicalAddr < buffer_.size()) {
                    data[bytesRead + i] = DataUtils::readU8(buffer_, physicalAddr);
                }
            }
            bytesRead += bytesToRead;
            currentLogicalStart += bytesToRead;
        }
        
        currentLogicalPos = rangeEnd;
    }
    
    return data;
}

uint8_t CrossSectionReader::readU8(size_t logicalOffset) const {
    std::vector<uint8_t> bytes = readBytes(logicalOffset, 1);
    return bytes[0];
}

uint16_t CrossSectionReader::readU16LE(size_t logicalOffset) const {
    std::vector<uint8_t> bytes = readBytes(logicalOffset, 2);
    return static_cast<uint16_t>(bytes[0]) | 
           (static_cast<uint16_t>(bytes[1]) << 8);
}

uint32_t CrossSectionReader::readU32LE(size_t logicalOffset) const {
    std::vector<uint8_t> bytes = readBytes(logicalOffset, 4);
    return static_cast<uint32_t>(bytes[0]) | 
           (static_cast<uint32_t>(bytes[1]) << 8) | 
           (static_cast<uint32_t>(bytes[2]) << 16) | 
           (static_cast<uint32_t>(bytes[3]) << 24);
}

size_t CrossSectionReader::getPhysicalAddress(size_t logicalOffset) const {
    size_t currentLogicalPos = 0;
    
    for (const auto& range : ranges_) {
        size_t rangeEnd = currentLogicalPos + range.second;
        if (logicalOffset < rangeEnd) {
            return range.first + (logicalOffset - currentLogicalPos);
        }
        currentLogicalPos = rangeEnd;
    }
    
    return 0;
}

size_t CrossSectionReader::getTotalSize() const {
    return totalSize_;
}

// ============================================================================
// Pokemon Data Functions
// ============================================================================

uint32_t getPID(const std::string& buffer, size_t pokemonBaseAddr) {
    return DataUtils::readU32LE(buffer, pokemonBaseAddr + POKEMON_PID_OFFSET);
}

uint32_t getOTID(const std::string& buffer, size_t pokemonBaseAddr) {
    return DataUtils::readU32LE(buffer, pokemonBaseAddr + POKEMON_OTID_OFFSET);
}

uint32_t getDecryptionKey(const std::string& buffer, size_t pokemonBaseAddr) {
    return getPID(buffer, pokemonBaseAddr) ^ getOTID(buffer, pokemonBaseAddr);
}

uint16_t getStoredPokemonChecksum(const std::string& buffer, size_t pokemonBaseAddr) {
    return DataUtils::readU16LE(buffer, pokemonBaseAddr + POKEMON_CHECKSUM_OFFSET);
}

uint16_t calculatePokemonDataChecksum(const std::string& buffer, size_t pokemonBaseAddr, uint32_t decryptionKey) {
    uint32_t sum = 0;
    
    // Process 48 bytes (12 words) of encrypted data starting at offset 0x20
    for (int i = 0; i < 12; i++) {
        size_t offset = pokemonBaseAddr + POKEMON_DATA_OFFSET + (i * 4);
        uint32_t encryptedWord = DataUtils::readU32LE(buffer, offset);
        uint32_t decryptedWord = encryptedWord ^ decryptionKey;
        
        // Sum as two 16-bit values
        sum += (decryptedWord & 0xFFFF);
        sum += ((decryptedWord >> 16) & 0xFFFF);
    }
    
    return static_cast<uint16_t>(sum & 0xFFFF);
}

uint16_t calculateBoxPokemonChecksum(const CrossSectionReader& reader, size_t logicalOffset, uint32_t decryptionKey) {
    uint32_t sum = 0;
    
    // Process 48 bytes (12 words) of encrypted data starting at offset 0x20
    for (int i = 0; i < 12; i++) {
        uint32_t encryptedWord = reader.readU32LE(logicalOffset + POKEMON_DATA_OFFSET + (i * 4));
        uint32_t decryptedWord = encryptedWord ^ decryptionKey;
        
        // Sum as two 16-bit values
        sum += (decryptedWord & 0xFFFF);
        sum += ((decryptedWord >> 16) & 0xFFFF);
    }
    
    return static_cast<uint16_t>(sum & 0xFFFF);
}

// ============================================================================
// Section Functions
// ============================================================================

size_t findSectionOffset(const SectionInfo* sections, uint16_t sectionId) {
    return findSectionOffset(sections, 14, sectionId);
}

size_t findSectionOffset(const SectionInfo sections[], size_t numSections, uint16_t sectionId) {
    for (size_t i = 0; i < numSections; i++) {
        if (sections[i].sectionId == sectionId) {
            return sections[i].sectionBaseAddress;
        }
    }
    return static_cast<size_t>(-1);
}

uint16_t calculateSectionChecksum(const std::string& buffer, size_t baseAddr, size_t dataSize) {
    uint32_t sum = 0;
    
    for (size_t i = 0; i < dataSize; i += 4) {
        sum += DataUtils::readU32LE(buffer, baseAddr + i);
    }
    
    // Fold to 16-bit: upper + lower
    uint16_t upper = (sum >> 16) & 0xFFFF;
    uint16_t lower = sum & 0xFFFF;
    
    return upper + lower;
}

std::vector<CrossSectionReader::DataRange> buildBoxDataRanges(const SaveBlock& saveBlock) {
    std::vector<CrossSectionReader::DataRange> dataRanges;
    
    for (int sectionId = 5; sectionId <= 13; sectionId++) {
        size_t sectionOffset = findSectionOffset(saveBlock.sections, sectionId);
        if (sectionOffset == static_cast<size_t>(-1)) continue;
        
        // Section 5 starts at offset 0x04 (first 4 bytes are box metadata)
        size_t startOffset = (sectionId == 5) ? 0x04 : 0x00;
        size_t usableBytes = 0x0F80 - startOffset;  // 0x0F80 = 3968 bytes per section
        
        dataRanges.push_back({sectionOffset + startOffset, usableBytes});
    }
    
    return dataRanges;
}

// ============================================================================
// Security/Encryption Functions
// ============================================================================

uint32_t getSecurityKey(const std::string& buffer, int game, size_t section0Offset) {
    // Ruby/Sapphire: no encryption, return 0
    if (game == GEN3_GAME_RS) {
        return 0;
    }
    
    // Emerald: security key at offset 0x00AC in Section 0
    if (game == GEN3_GAME_EMERALD) {
        return DataUtils::readU32LE(buffer, section0Offset + GEN3_SECURITY_KEY_OFFSET_E);
    }
    
    // FireRed/LeafGreen: security key at offset 0x0AF8 in Section 0
    if (game == GEN3_GAME_FRLG) {
        return DataUtils::readU32LE(buffer, section0Offset + GEN3_SECURITY_KEY_OFFSET_FRLG);
    }
    
    return 0;
}

uint16_t decryptItemQuantity(uint16_t encryptedQty, int game, uint32_t securityKey) {
    // Ruby/Sapphire: no encryption
    if (game == GEN3_GAME_RS) {
        return encryptedQty;
    }
    
    // Emerald and FRLG: XOR with lower 16 bits of security key
    return encryptedQty ^ static_cast<uint16_t>(securityKey & 0xFFFF);
}

uint16_t encryptItemQuantity(uint16_t quantity, int game, uint32_t securityKey) {
    // Ruby/Sapphire: no encryption
    if (game == GEN3_GAME_RS) {
        return quantity;
    }
    
    // Emerald and FRLG: XOR with lower 16 bits of security key
    return quantity ^ static_cast<uint16_t>(securityKey & 0xFFFF);
}

} // namespace Generation3Utils