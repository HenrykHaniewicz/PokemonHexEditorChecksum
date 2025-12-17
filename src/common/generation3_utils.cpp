#include "generation3_utils.h"
#include "data_utils.h"
#include <algorithm>
#include <sstream>

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
// Save Block Functions
// ============================================================================

bool parseSaveBlocks(const std::string& buffer, SaveBlock& blockA, SaveBlock& blockB) {
    if (buffer.size() < GEN3_SAVE_SIZE) {
        return false;
    }
    
    // Parse Block A (starts at 0x0000)
    blockA.valid = true;
    for (size_t i = 0; i < GEN3_NUM_SECTIONS; i++) {
        size_t sectionBase = i * GEN3_SECTION_SIZE;
        
        blockA.sections[i].sectionBaseAddress = sectionBase;
        blockA.sections[i].sectionId = DataUtils::readU16LE(buffer, sectionBase + GEN3_SECTION_ID_OFFSET);
        blockA.sections[i].saveIndex = DataUtils::readU32LE(buffer, sectionBase + GEN3_SECTION_SAVE_INDEX_OFFSET);
        blockA.sections[i].storedChecksum = DataUtils::readU16LE(buffer, sectionBase + GEN3_SECTION_CHECKSUM_OFFSET);
        blockA.sections[i].checksumLocation = sectionBase + GEN3_SECTION_CHECKSUM_OFFSET;
        
        if (blockA.sections[i].sectionId < 14) {
            blockA.sections[i].dataSize = GEN3_SECTION_SIZES[blockA.sections[i].sectionId];
        }
        
        blockA.sections[i].calculatedChecksum = calculateSectionChecksum(buffer, sectionBase, blockA.sections[i].dataSize);
        blockA.sections[i].matches = (blockA.sections[i].calculatedChecksum == blockA.sections[i].storedChecksum);
    }
    blockA.saveIndex = blockA.sections[0].saveIndex;
    
    // Parse Block B (starts at 0xE000)
    blockB.valid = true;
    for (size_t i = 0; i < GEN3_NUM_SECTIONS; i++) {
        size_t sectionBase = GEN3_BLOCK_SIZE + (i * GEN3_SECTION_SIZE);
        
        blockB.sections[i].sectionBaseAddress = sectionBase;
        blockB.sections[i].sectionId = DataUtils::readU16LE(buffer, sectionBase + GEN3_SECTION_ID_OFFSET);
        blockB.sections[i].saveIndex = DataUtils::readU32LE(buffer, sectionBase + GEN3_SECTION_SAVE_INDEX_OFFSET);
        blockB.sections[i].storedChecksum = DataUtils::readU16LE(buffer, sectionBase + GEN3_SECTION_CHECKSUM_OFFSET);
        blockB.sections[i].checksumLocation = sectionBase + GEN3_SECTION_CHECKSUM_OFFSET;
        
        if (blockB.sections[i].sectionId < 14) {
            blockB.sections[i].dataSize = GEN3_SECTION_SIZES[blockB.sections[i].sectionId];
        }
        
        blockB.sections[i].calculatedChecksum = calculateSectionChecksum(buffer, sectionBase, blockB.sections[i].dataSize);
        blockB.sections[i].matches = (blockB.sections[i].calculatedChecksum == blockB.sections[i].storedChecksum);
    }
    blockB.saveIndex = blockB.sections[0].saveIndex;
    
    return true;
}

const SaveBlock* getActiveSaveBlock(const SaveBlock& blockA, const SaveBlock& blockB) {
    // The block with the higher save index is the active one
    // Handle wraparound: if one is 0xFFFFFFFF and other is 0, the 0 is newer
    if (blockA.saveIndex == 0xFFFFFFFF && blockB.saveIndex == 0) {
        return &blockB;
    }
    if (blockB.saveIndex == 0xFFFFFFFF && blockA.saveIndex == 0) {
        return &blockA;
    }
    return (blockA.saveIndex >= blockB.saveIndex) ? &blockA : &blockB;
}

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
    
    uint16_t upper = (sum >> 16) & 0xFFFF;
    uint16_t lower = sum & 0xFFFF;
    
    return upper + lower;
}

void updateSectionChecksum(std::string& buffer, size_t sectionBaseAddr, size_t dataSize) {
    uint16_t checksum = calculateSectionChecksum(buffer, sectionBaseAddr, dataSize);
    DataUtils::writeU16LE(buffer, sectionBaseAddr + GEN3_SECTION_CHECKSUM_OFFSET, checksum);
}

std::vector<CrossSectionReader::DataRange> buildBoxDataRanges(const SaveBlock& saveBlock) {
    std::vector<CrossSectionReader::DataRange> dataRanges;
    
    for (int sectionId = 5; sectionId <= 13; sectionId++) {
        size_t sectionOffset = findSectionOffset(saveBlock.sections, sectionId);
        if (sectionOffset == static_cast<size_t>(-1)) continue;
        
        size_t startOffset = (sectionId == 5) ? 0x04 : 0x00;
        size_t usableBytes = 0x0F80 - startOffset;
        
        dataRanges.push_back({sectionOffset + startOffset, usableBytes});
    }
    
    return dataRanges;
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

uint32_t getDecryptionKey(uint32_t pid, uint32_t otid) {
    return pid ^ otid;
}

uint16_t getStoredPokemonChecksum(const std::string& buffer, size_t pokemonBaseAddr) {
    return DataUtils::readU16LE(buffer, pokemonBaseAddr + POKEMON_CHECKSUM_OFFSET);
}

// ============================================================================
// Nature Functions
// ============================================================================

uint8_t getNatureFromPID(uint32_t pid) {
    return static_cast<uint8_t>(pid % 25);
}

const char* getNatureName(uint32_t pid) {
    return NATURE_NAMES[pid % 25];
}

const char* getNatureNameByIndex(uint8_t natureIndex) {
    if (natureIndex >= 25) return "Unknown";
    return NATURE_NAMES[natureIndex];
}

uint8_t lookupNatureByName(const std::string& name) {
    std::string query = name;
    std::transform(query.begin(), query.end(), query.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    
    for (uint8_t i = 0; i < 25; i++) {
        std::string natureName(NATURE_NAMES[i]);
        std::transform(natureName.begin(), natureName.end(), natureName.begin(), [](unsigned char c) {
            return static_cast<char>(std::toupper(c));
        });
        
        if (natureName == query) {
            return i;
        }
    }
    
    return 0xFF; // Not found
}

uint32_t calculatePIDForNature(uint32_t currentPID, uint8_t desiredNature) {
    if (desiredNature >= 25) return currentPID;
    
    uint8_t currentNature = getNatureFromPID(currentPID);
    if (currentNature == desiredNature) return currentPID;
    
    int32_t diff = static_cast<int32_t>(desiredNature) - static_cast<int32_t>(currentNature);
    if (diff < 0) diff += 25;
    
    return currentPID + static_cast<uint32_t>(diff);
}

// ============================================================================
// Substructure Functions
// ============================================================================

int getSubstructureOrderIndex(uint32_t pid) {
    return static_cast<int>(pid % 24);
}

const char* getSubstructureOrderString(uint32_t pid) {
    return SUBSTRUCTURE_ORDERS[pid % 24];
}

int getSubstructureOffset(uint32_t pid, Substructure sub) {
    const char* order = SUBSTRUCTURE_ORDERS[pid % 24];
    char target = 'G';
    
    switch (sub) {
        case Substructure::GROWTH:  target = 'G'; break;
        case Substructure::ATTACKS: target = 'A'; break;
        case Substructure::EVS:     target = 'E'; break;
        case Substructure::MISC:    target = 'M'; break;
    }
    
    for (int i = 0; i < 4; i++) {
        if (order[i] == target) {
            return i * 12;
        }
    }
    
    return 0;
}

// ============================================================================
// Encryption/Decryption Functions
// ============================================================================

std::array<uint8_t, 48> decryptPokemonData(const std::string& buffer, size_t pokemonBaseAddr) {
    uint32_t key = getDecryptionKey(buffer, pokemonBaseAddr);
    return decryptPokemonData(buffer, pokemonBaseAddr, key);
}

std::array<uint8_t, 48> decryptPokemonData(const std::string& buffer, size_t pokemonBaseAddr, uint32_t decryptionKey) {
    std::array<uint8_t, 48> decrypted;
    
    for (size_t i = 0; i < 12; i++) {
        size_t offset = pokemonBaseAddr + POKEMON_DATA_OFFSET + (i * 4);
        uint32_t encrypted = DataUtils::readU32LE(buffer, offset);
        uint32_t decryptedWord = encrypted ^ decryptionKey;
        
        decrypted[i * 4 + 0] = decryptedWord & 0xFF;
        decrypted[i * 4 + 1] = (decryptedWord >> 8) & 0xFF;
        decrypted[i * 4 + 2] = (decryptedWord >> 16) & 0xFF;
        decrypted[i * 4 + 3] = (decryptedWord >> 24) & 0xFF;
    }
    
    return decrypted;
}

void encryptPokemonData(std::string& buffer, size_t pokemonBaseAddr, const std::array<uint8_t, 48>& data) {
    uint32_t key = getDecryptionKey(buffer, pokemonBaseAddr);
    encryptPokemonData(buffer, pokemonBaseAddr, data, key);
}

void encryptPokemonData(std::string& buffer, size_t pokemonBaseAddr, const std::array<uint8_t, 48>& data, uint32_t decryptionKey) {
    for (size_t i = 0; i < 12; i++) {
        uint32_t plainWord = static_cast<uint32_t>(data[i * 4 + 0]) |
                            (static_cast<uint32_t>(data[i * 4 + 1]) << 8) |
                            (static_cast<uint32_t>(data[i * 4 + 2]) << 16) |
                            (static_cast<uint32_t>(data[i * 4 + 3]) << 24);
        
        uint32_t encrypted = plainWord ^ decryptionKey;
        
        size_t offset = pokemonBaseAddr + POKEMON_DATA_OFFSET + (i * 4);
        DataUtils::writeU32LE(buffer, offset, encrypted);
    }
}

Gen3SubstructureData parseSubstructureData(const std::array<uint8_t, 48>& decryptedData, uint32_t pid) {
    Gen3SubstructureData data;
    
    // Get offsets for each substructure
    int growthOffset = getSubstructureOffset(pid, Substructure::GROWTH);
    int attacksOffset = getSubstructureOffset(pid, Substructure::ATTACKS);
    int evsOffset = getSubstructureOffset(pid, Substructure::EVS);
    int miscOffset = getSubstructureOffset(pid, Substructure::MISC);
    
    // Parse Growth substructure
    data.species = static_cast<uint16_t>(decryptedData[growthOffset]) |
                   (static_cast<uint16_t>(decryptedData[growthOffset + 1]) << 8);
    data.heldItem = static_cast<uint16_t>(decryptedData[growthOffset + 2]) |
                    (static_cast<uint16_t>(decryptedData[growthOffset + 3]) << 8);
    data.experience = static_cast<uint32_t>(decryptedData[growthOffset + 4]) |
                      (static_cast<uint32_t>(decryptedData[growthOffset + 5]) << 8) |
                      (static_cast<uint32_t>(decryptedData[growthOffset + 6]) << 16) |
                      (static_cast<uint32_t>(decryptedData[growthOffset + 7]) << 24);
    data.ppBonuses = decryptedData[growthOffset + 8];
    data.friendship = decryptedData[growthOffset + 9];
    
    // Parse Attacks substructure
    for (int i = 0; i < 4; i++) {
        data.moves[i] = static_cast<uint16_t>(decryptedData[attacksOffset + i * 2]) |
                        (static_cast<uint16_t>(decryptedData[attacksOffset + i * 2 + 1]) << 8);
    }
    for (int i = 0; i < 4; i++) {
        data.pp[i] = decryptedData[attacksOffset + 8 + i];
    }
    
    // Parse EVs & Condition substructure
    data.hpEV = decryptedData[evsOffset];
    data.attackEV = decryptedData[evsOffset + 1];
    data.defenseEV = decryptedData[evsOffset + 2];
    data.speedEV = decryptedData[evsOffset + 3];
    data.spAtkEV = decryptedData[evsOffset + 4];
    data.spDefEV = decryptedData[evsOffset + 5];
    data.coolness = decryptedData[evsOffset + 6];
    data.beauty = decryptedData[evsOffset + 7];
    data.cuteness = decryptedData[evsOffset + 8];
    data.smartness = decryptedData[evsOffset + 9];
    data.toughness = decryptedData[evsOffset + 10];
    data.feel = decryptedData[evsOffset + 11];
    
    // Parse Miscellaneous substructure
    data.pokerus = decryptedData[miscOffset];
    data.metLocation = decryptedData[miscOffset + 1];
    data.originsInfo = static_cast<uint16_t>(decryptedData[miscOffset + 2]) |
                       (static_cast<uint16_t>(decryptedData[miscOffset + 3]) << 8);
    data.ivsEggAbility = static_cast<uint32_t>(decryptedData[miscOffset + 4]) |
                          (static_cast<uint32_t>(decryptedData[miscOffset + 5]) << 8) |
                          (static_cast<uint32_t>(decryptedData[miscOffset + 6]) << 16) |
                          (static_cast<uint32_t>(decryptedData[miscOffset + 7]) << 24);
    data.ribbonsObedience = static_cast<uint32_t>(decryptedData[miscOffset + 8]) |
                             (static_cast<uint32_t>(decryptedData[miscOffset + 9]) << 8) |
                             (static_cast<uint32_t>(decryptedData[miscOffset + 10]) << 16) |
                             (static_cast<uint32_t>(decryptedData[miscOffset + 11]) << 24);
    
    return data;
}

std::array<uint8_t, 48> buildSubstructureData(const Gen3SubstructureData& data, uint32_t pid) {
    std::array<uint8_t, 48> result{};
    
    int growthOffset = getSubstructureOffset(pid, Substructure::GROWTH);
    int attacksOffset = getSubstructureOffset(pid, Substructure::ATTACKS);
    int evsOffset = getSubstructureOffset(pid, Substructure::EVS);
    int miscOffset = getSubstructureOffset(pid, Substructure::MISC);
    
    // Build Growth substructure
    result[growthOffset] = data.species & 0xFF;
    result[growthOffset + 1] = (data.species >> 8) & 0xFF;
    result[growthOffset + 2] = data.heldItem & 0xFF;
    result[growthOffset + 3] = (data.heldItem >> 8) & 0xFF;
    result[growthOffset + 4] = data.experience & 0xFF;
    result[growthOffset + 5] = (data.experience >> 8) & 0xFF;
    result[growthOffset + 6] = (data.experience >> 16) & 0xFF;
    result[growthOffset + 7] = (data.experience >> 24) & 0xFF;
    result[growthOffset + 8] = data.ppBonuses;
    result[growthOffset + 9] = data.friendship;
    result[growthOffset + 10] = 0;
    result[growthOffset + 11] = 0;
    
    // Build Attacks substructure
    for (int i = 0; i < 4; i++) {
        result[attacksOffset + i * 2] = data.moves[i] & 0xFF;
        result[attacksOffset + i * 2 + 1] = (data.moves[i] >> 8) & 0xFF;
    }
    for (int i = 0; i < 4; i++) {
        result[attacksOffset + 8 + i] = data.pp[i];
    }
    
    // Build EVs & Condition substructure
    result[evsOffset] = data.hpEV;
    result[evsOffset + 1] = data.attackEV;
    result[evsOffset + 2] = data.defenseEV;
    result[evsOffset + 3] = data.speedEV;
    result[evsOffset + 4] = data.spAtkEV;
    result[evsOffset + 5] = data.spDefEV;
    result[evsOffset + 6] = data.coolness;
    result[evsOffset + 7] = data.beauty;
    result[evsOffset + 8] = data.cuteness;
    result[evsOffset + 9] = data.smartness;
    result[evsOffset + 10] = data.toughness;
    result[evsOffset + 11] = data.feel;
    
    // Build Miscellaneous substructure
    result[miscOffset] = data.pokerus;
    result[miscOffset + 1] = data.metLocation;
    result[miscOffset + 2] = data.originsInfo & 0xFF;
    result[miscOffset + 3] = (data.originsInfo >> 8) & 0xFF;
    result[miscOffset + 4] = data.ivsEggAbility & 0xFF;
    result[miscOffset + 5] = (data.ivsEggAbility >> 8) & 0xFF;
    result[miscOffset + 6] = (data.ivsEggAbility >> 16) & 0xFF;
    result[miscOffset + 7] = (data.ivsEggAbility >> 24) & 0xFF;
    result[miscOffset + 8] = data.ribbonsObedience & 0xFF;
    result[miscOffset + 9] = (data.ribbonsObedience >> 8) & 0xFF;
    result[miscOffset + 10] = (data.ribbonsObedience >> 16) & 0xFF;
    result[miscOffset + 11] = (data.ribbonsObedience >> 24) & 0xFF;
    
    return result;
}

// ============================================================================
// Pokemon Checksum Functions
// ============================================================================

uint16_t calculatePokemonDataChecksum(const std::array<uint8_t, 48>& decryptedData) {
    uint32_t sum = 0;
    
    for (size_t i = 0; i < 48; i += 2) {
        uint16_t word = static_cast<uint16_t>(decryptedData[i]) |
                        (static_cast<uint16_t>(decryptedData[i + 1]) << 8);
        sum += word;
    }
    
    return static_cast<uint16_t>(sum & 0xFFFF);
}

uint16_t calculatePokemonDataChecksum(const std::string& buffer, size_t pokemonBaseAddr, uint32_t decryptionKey) {
    uint32_t sum = 0;
    
    for (int i = 0; i < 12; i++) {
        size_t offset = pokemonBaseAddr + POKEMON_DATA_OFFSET + (i * 4);
        uint32_t encryptedWord = DataUtils::readU32LE(buffer, offset);
        uint32_t decryptedWord = encryptedWord ^ decryptionKey;
        
        sum += (decryptedWord & 0xFFFF);
        sum += ((decryptedWord >> 16) & 0xFFFF);
    }
    
    return static_cast<uint16_t>(sum & 0xFFFF);
}

uint16_t calculateBoxPokemonChecksum(const CrossSectionReader& reader, size_t logicalOffset, uint32_t decryptionKey) {
    uint32_t sum = 0;
    
    for (int i = 0; i < 12; i++) {
        uint32_t encryptedWord = reader.readU32LE(logicalOffset + POKEMON_DATA_OFFSET + (i * 4));
        uint32_t decryptedWord = encryptedWord ^ decryptionKey;
        
        sum += (decryptedWord & 0xFFFF);
        sum += ((decryptedWord >> 16) & 0xFFFF);
    }
    
    return static_cast<uint16_t>(sum & 0xFFFF);
}

void updatePokemonChecksum(std::string& buffer, size_t pokemonBaseAddr) {
    uint32_t key = getDecryptionKey(buffer, pokemonBaseAddr);
    uint16_t checksum = calculatePokemonDataChecksum(buffer, pokemonBaseAddr, key);
    DataUtils::writeU16LE(buffer, pokemonBaseAddr + POKEMON_CHECKSUM_OFFSET, checksum);
}

bool verifyPokemonChecksum(const std::string& buffer, size_t pokemonBaseAddr) {
    uint16_t stored = getStoredPokemonChecksum(buffer, pokemonBaseAddr);
    uint32_t key = getDecryptionKey(buffer, pokemonBaseAddr);
    uint16_t calculated = calculatePokemonDataChecksum(buffer, pokemonBaseAddr, key);
    return stored == calculated;
}

// ============================================================================
// IV Extraction/Setting Functions
// ============================================================================

uint8_t getIVHP(uint32_t ivsEggAbility) {
    return static_cast<uint8_t>(ivsEggAbility & IV_HP_MASK);
}

uint8_t getIVAttack(uint32_t ivsEggAbility) {
    return static_cast<uint8_t>((ivsEggAbility & IV_ATTACK_MASK) >> 5);
}

uint8_t getIVDefense(uint32_t ivsEggAbility) {
    return static_cast<uint8_t>((ivsEggAbility & IV_DEFENSE_MASK) >> 10);
}

uint8_t getIVSpeed(uint32_t ivsEggAbility) {
    return static_cast<uint8_t>((ivsEggAbility & IV_SPEED_MASK) >> 15);
}

uint8_t getIVSpAtk(uint32_t ivsEggAbility) {
    return static_cast<uint8_t>((ivsEggAbility & IV_SPATK_MASK) >> 20);
}

uint8_t getIVSpDef(uint32_t ivsEggAbility) {
    return static_cast<uint8_t>((ivsEggAbility & IV_SPDEF_MASK) >> 25);
}

bool isEgg(uint32_t ivsEggAbility) {
    return (ivsEggAbility & EGG_FLAG) != 0;
}

bool hasSecondAbility(uint32_t ivsEggAbility) {
    return (ivsEggAbility & ABILITY_FLAG) != 0;
}

uint32_t setIVHP(uint32_t ivsEggAbility, uint8_t value) {
    return (ivsEggAbility & ~IV_HP_MASK) | (value & 0x1F);
}

uint32_t setIVAttack(uint32_t ivsEggAbility, uint8_t value) {
    return (ivsEggAbility & ~IV_ATTACK_MASK) | ((static_cast<uint32_t>(value) & 0x1F) << 5);
}

uint32_t setIVDefense(uint32_t ivsEggAbility, uint8_t value) {
    return (ivsEggAbility & ~IV_DEFENSE_MASK) | ((static_cast<uint32_t>(value) & 0x1F) << 10);
}

uint32_t setIVSpeed(uint32_t ivsEggAbility, uint8_t value) {
    return (ivsEggAbility & ~IV_SPEED_MASK) | ((static_cast<uint32_t>(value) & 0x1F) << 15);
}

uint32_t setIVSpAtk(uint32_t ivsEggAbility, uint8_t value) {
    return (ivsEggAbility & ~IV_SPATK_MASK) | ((static_cast<uint32_t>(value) & 0x1F) << 20);
}

uint32_t setIVSpDef(uint32_t ivsEggAbility, uint8_t value) {
    return (ivsEggAbility & ~IV_SPDEF_MASK) | ((static_cast<uint32_t>(value) & 0x1F) << 25);
}

uint32_t setEggFlag(uint32_t ivsEggAbility, bool isEggFlag) {
    if (isEggFlag) {
        return ivsEggAbility | EGG_FLAG;
    } else {
        return ivsEggAbility & ~EGG_FLAG;
    }
}

uint32_t setAbilityFlag(uint32_t ivsEggAbility, bool secondAbility) {
    if (secondAbility) {
        return ivsEggAbility | ABILITY_FLAG;
    } else {
        return ivsEggAbility & ~ABILITY_FLAG;
    }
}

// ============================================================================
// Origins Info Functions
// ============================================================================

uint8_t getLevelMet(uint16_t originsInfo) {
    return static_cast<uint8_t>(originsInfo & 0x7F);
}

uint8_t getGameOfOrigin(uint16_t originsInfo) {
    return static_cast<uint8_t>((originsInfo >> 7) & 0x0F);
}

uint8_t getPokeBallCaughtIn(uint16_t originsInfo) {
    return static_cast<uint8_t>((originsInfo >> 11) & 0x0F);
}

bool getOTGender(uint16_t originsInfo) {
    return (originsInfo & 0x8000) != 0;
}

uint16_t setLevelMet(uint16_t originsInfo, uint8_t level) {
    return (originsInfo & ~0x007F) | (level & 0x7F);
}

uint16_t setGameOfOrigin(uint16_t originsInfo, uint8_t game) {
    return (originsInfo & ~0x0780) | ((static_cast<uint16_t>(game) & 0x0F) << 7);
}

uint16_t setPokeBallCaughtIn(uint16_t originsInfo, uint8_t ball) {
    return (originsInfo & ~0x7800) | ((static_cast<uint16_t>(ball) & 0x0F) << 11);
}

uint16_t setOTGender(uint16_t originsInfo, bool female) {
    if (female) {
        return originsInfo | 0x8000;
    } else {
        return originsInfo & ~0x8000;
    }
}

// ============================================================================
// Security/Encryption Functions
// ============================================================================

uint32_t getSecurityKey(const std::string& buffer, int game, size_t section0Offset) {
    if (game == GEN3_GAME_RS) {
        return 0;
    }
    
    if (game == GEN3_GAME_EMERALD) {
        return DataUtils::readU32LE(buffer, section0Offset + GEN3_SECURITY_KEY_OFFSET_E);
    }
    
    if (game == GEN3_GAME_FRLG) {
        return DataUtils::readU32LE(buffer, section0Offset + GEN3_SECURITY_KEY_OFFSET_FRLG);
    }
    
    return 0;
}

uint16_t decryptItemQuantity(uint16_t encryptedQty, int game, uint32_t securityKey) {
    if (game == GEN3_GAME_RS) {
        return encryptedQty;
    }
    return encryptedQty ^ static_cast<uint16_t>(securityKey & 0xFFFF);
}

uint16_t encryptItemQuantity(uint16_t quantity, int game, uint32_t securityKey) {
    if (game == GEN3_GAME_RS) {
        return quantity;
    }
    return quantity ^ static_cast<uint16_t>(securityKey & 0xFFFF);
}

// ============================================================================
// Helper Functions
// ============================================================================

const char* getLanguageName(uint8_t languageCode) {
    switch (languageCode) {
        case 1: return "Japanese";
        case 2: return "English";
        case 3: return "French";
        case 4: return "Italian";
        case 5: return "German";
        case 6: return "Unused";
        case 7: return "Spanish";
        default: return "Unknown";
    }
}

const char* getPokeBallName(uint8_t ballIndex) {
    switch (ballIndex) {
        case 1: return "Master Ball";
        case 2: return "Ultra Ball";
        case 3: return "Great Ball";
        case 4: return "Poke Ball";
        case 5: return "Safari Ball";
        case 6: return "Net Ball";
        case 7: return "Dive Ball";
        case 8: return "Nest Ball";
        case 9: return "Repeat Ball";
        case 10: return "Timer Ball";
        case 11: return "Luxury Ball";
        case 12: return "Premier Ball";
        default: return "Unknown";
    }
}

const char* getGameOfOriginName(uint8_t gameIndex) {
    switch (gameIndex) {
        case 1: return "Sapphire";
        case 2: return "Ruby";
        case 3: return "Emerald";
        case 4: return "FireRed";
        case 5: return "LeafGreen";
        case 15: return "Colosseum/XD";
        default: return "Unknown";
    }
}

std::string getStatusConditionString(uint32_t status) {
    if (status == 0) return "Healthy";
    
    std::stringstream ss;
    
    if (status & STATUS_SLEEP_MASK) {
        int sleepTurns = status & STATUS_SLEEP_MASK;
        ss << "Sleep(" << sleepTurns << ")";
    }
    if (status & STATUS_POISON) {
        if (!ss.str().empty()) ss << ", ";
        ss << "Poison";
    }
    if (status & STATUS_BURN) {
        if (!ss.str().empty()) ss << ", ";
        ss << "Burn";
    }
    if (status & STATUS_FREEZE) {
        if (!ss.str().empty()) ss << ", ";
        ss << "Freeze";
    }
    if (status & STATUS_PARALYSIS) {
        if (!ss.str().empty()) ss << ", ";
        ss << "Paralysis";
    }
    if (status & STATUS_BAD_POISON) {
        if (!ss.str().empty()) ss << ", ";
        ss << "Bad Poison";
    }
    
    return ss.str().empty() ? "Healthy" : ss.str();
}

} // namespace Generation3Utils