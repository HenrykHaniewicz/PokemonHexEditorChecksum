#include "generation3_utils.h"
#include "data_utils.h"

namespace Generation3Utils {
    
    uint32_t getPID(const std::string& buffer, size_t pokemonBaseAddr) {
        return DataUtils::readU32LE(buffer, pokemonBaseAddr + 0x00);
    }
    
    uint32_t getOTID(const std::string& buffer, size_t pokemonBaseAddr) {
        return DataUtils::readU32LE(buffer, pokemonBaseAddr + 0x04);
    }
    
    uint32_t getDecryptionKey(const std::string& buffer, size_t pokemonBaseAddr) {
        return getPID(buffer, pokemonBaseAddr) ^ getOTID(buffer, pokemonBaseAddr);
    }
    
    uint16_t getStoredPokemonChecksum(const std::string& buffer, size_t pokemonBaseAddr) {
        return DataUtils::readU16LE(buffer, pokemonBaseAddr + 0x1C);
    }
    
    uint16_t calculatePokemonDataChecksum(const std::string& buffer, size_t pokemonBaseAddr, uint32_t decryptionKey) {
        uint32_t sum = 0;
        
        // Process 48 bytes (12 words) of encrypted data starting at offset 0x20
        for (int i = 0; i < 12; i++) {
            size_t offset = pokemonBaseAddr + 0x20 + (i * 4);
            uint32_t encryptedWord = DataUtils::readU32LE(buffer, offset);
            uint32_t decryptedWord = encryptedWord ^ decryptionKey;
            
            // Sum as two 16-bit values
            sum += (decryptedWord & 0xFFFF);
            sum += ((decryptedWord >> 16) & 0xFFFF);
        }
        
        return static_cast<uint16_t>(sum & 0xFFFF);
    }
    
    size_t findSectionOffset(const SectionInfo* sections, uint16_t sectionId) {
        // Assumes 14 sections as per Gen3 standard
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
}