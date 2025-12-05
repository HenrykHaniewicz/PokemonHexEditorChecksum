#include "checksum_calc.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <sys/stat.h>

// ============================================================================
// Constructor
// ============================================================================

ChecksumCalculator::ChecksumCalculator()
    : SDLAppBase("Checksum Calculator", 600, 650),
      fileSize(0), 
      gameMode(GAME_POKEMON_RED_BLUE),
      isJapanese(false), 
      shouldWrite(false), shouldOverwrite(false),
      redBlueBank1Sum(0), redBlueBank1Checksum(0), redBlueBank1StoredChecksum(0),
      redBlueBank1ChecksumLocation(0), 
      redBlueBank1Start(0), redBlueBank1End(0), redBlueBank1Matches(false),
      goldSilverTotalSum1(0), goldSilverTotalSum2(0),
      goldSilverChecksum1(0), goldSilverChecksum2(0),
      goldSilverStoredChecksum1(0), goldSilverStoredChecksum2(0),
      goldSilverChecksum1Location(0), goldSilverChecksum2Location(0),
      goldSilverStart1(0), goldSilverEnd1(0),
      goldSilverChecksum1Matches(false), goldSilverChecksum2Matches(false),
      crystalTotalSum1(0), crystalTotalSum2(0),
      crystalChecksum1(0), crystalChecksum2(0),
      crystalStoredChecksum1(0), crystalStoredChecksum2(0),
      crystalChecksum1Location(0), crystalChecksum2Location(0),
      crystalStart1(0), crystalEnd1(0), 
      crystalStart2(0), crystalEnd2(0),
      crystalChecksum1Matches(false), crystalChecksum2Matches(false),
      gen3SaveAIsCurrent(false),
      pokemonChecksumMode(false) {
    
    memset(&redBlueBank2, 0, sizeof(redBlueBank2));
    memset(&redBlueBank3, 0, sizeof(redBlueBank3));
    memset(&gen3SaveA, 0, sizeof(gen3SaveA));
    memset(&gen3SaveB, 0, sizeof(gen3SaveB));
}

// ============================================================================
// Low-level Buffer Read/Write Helpers
// ============================================================================

uint8_t ChecksumCalculator::readU8(size_t offset) const {
    return DataUtils::readU8(fileBuffer, offset);
}

uint16_t ChecksumCalculator::readU16LE(size_t offset) const {
    return DataUtils::readU16LE(fileBuffer, offset);
}

uint32_t ChecksumCalculator::readU32LE(size_t offset) const {
    return DataUtils::readU32LE(fileBuffer, offset);
}

void ChecksumCalculator::writeU16LE(std::string& buffer, size_t offset, uint16_t value) {
    DataUtils::writeU16LE(buffer, offset, value);
}

// ============================================================================
// Pokemon Data Structure Helpers (for Gen 3)
// ============================================================================

uint16_t ChecksumCalculator::calculatePokemonDataChecksum(size_t pokemonBaseAddr, uint32_t decryptionKey) const {
    return Generation3Utils::calculatePokemonDataChecksum(fileBuffer, pokemonBaseAddr, decryptionKey);
}

PokemonChecksumResult ChecksumCalculator::calculatePokemonChecksumResult(
    size_t pokemonBaseAddr, const std::string& locationStr) const {
    
    PokemonChecksumResult result;
    result.location = pokemonBaseAddr + 0x1C;
    result.locationStr = locationStr;
    
    uint32_t key = Generation3Utils::getDecryptionKey(fileBuffer, pokemonBaseAddr);
    result.calculated = Generation3Utils::calculatePokemonDataChecksum(fileBuffer, pokemonBaseAddr, key);
    result.stored = Generation3Utils::getStoredPokemonChecksum(fileBuffer, pokemonBaseAddr);
    result.valid = (result.calculated == result.stored);
    
    return result;
}

// ============================================================================
// Public Interface Functions
// ============================================================================

bool ChecksumCalculator::loadFile(const char* filename) {
    if (!HexUtils::loadFileToBuffer(filename, fileBuffer, fileSize)) {
        std::cerr << "Failed to open: " << filename << std::endl;
        return false;
    }
    fileName = filename;
    return true;
}

bool ChecksumCalculator::setGame(const std::string& game) {
    std::string g = game;
    std::transform(g.begin(), g.end(), g.begin(), ::tolower);
    
    if (g == "red" || g == "blue" || g == "yellow" || g == "green" || 
        g == "pokemon_red" || g == "pokemon_blue" || g == "pokemon_yellow" ||
        g == "pokemon_red_blue" || g == "redblue") {
        gameMode = GAME_POKEMON_RED_BLUE;
        if (g == "yellow" || g == "pokemon_yellow") {
            gameName = "Pokemon Yellow";
        } else {
            gameName = "Pokemon Red/Blue/Green";
        }
    } else if (g == "gold" || g == "silver" || g == "pokemon_gold" || g == "pokemon_silver" ||
               g == "pokemon_gold_silver" || g == "goldsilver") {
        gameMode = GAME_POKEMON_GOLD_SILVER;
        gameName = "Pokemon Gold/Silver";
    } else if (g == "crystal" || g == "pokemon_crystal") {
        gameMode = GAME_POKEMON_CRYSTAL;
        gameName = "Pokemon Crystal";
    } else if (g == "ruby" || g == "pokemon_ruby") {
        gameMode = GAME_POKEMON_GENERATION3;
        gameName = "Pokemon Ruby";
    } else if (g == "sapphire" || g == "pokemon_sapphire") {
        gameMode = GAME_POKEMON_GENERATION3;
        gameName = "Pokemon Sapphire";
    } else if (g == "emerald" || g == "pokemon_emerald") {
        gameMode = GAME_POKEMON_GENERATION3;
        gameName = "Pokemon Emerald";
    } else if (g == "firered" || g == "fire_red" || g == "pokemon_firered" || g == "pokemon_fire_red") {
        gameMode = GAME_POKEMON_GENERATION3;
        gameName = "Pokemon FireRed";
    } else if (g == "leafgreen" || g == "leaf_green" || g == "pokemon_leafgreen" || g == "pokemon_leaf_green") {
        gameMode = GAME_POKEMON_GENERATION3;
        gameName = "Pokemon LeafGreen";
    } else if (g == "gen3" || g == "generation3" || g == "generation_3") {
        gameMode = GAME_POKEMON_GENERATION3;
        gameName = "Pokemon Generation 3";
    } else {
        std::cerr << "Unknown game: " << game << std::endl;
        std::cerr << "Supported games: red, blue, yellow, green, gold, silver, crystal, ruby, sapphire, emerald, firered, leafgreen" << std::endl;
        return false;
    }
    
    if (isJapanese) {
        if (gameMode == GAME_POKEMON_CRYSTAL || gameMode == GAME_POKEMON_RED_BLUE || gameMode == GAME_POKEMON_GOLD_SILVER) {
            gameName += " (Japanese)";
        } else {
            std::cout << "Note: Japanese version has no known checksum difference for " << gameName << std::endl;
            std::cout << "Proceeding with regular checksum calculation." << std::endl;
        }
    }
    
    return true;
}

bool ChecksumCalculator::calculateChecksum() {
    setWindowTitle("Checksum Calculator - " + gameName);

    if (pokemonChecksumMode) {
        return calculatePokemonChecksum();
    }
    
    bool result = false;
    switch (gameMode) {
        case GAME_POKEMON_RED_BLUE:
            result = calculateChecksumPokemonRedBlue();
            break;
        case GAME_POKEMON_GOLD_SILVER:
            result = calculateChecksumPokemonGoldSilver();
            break;
        case GAME_POKEMON_CRYSTAL:
            result = calculateChecksumPokemonCrystal();
            break;
        case GAME_POKEMON_GENERATION3:
            result = calculateChecksumPokemonGeneration3();
            break;
        default:
            std::cerr << "Unknown game mode" << std::endl;
            return false;
    }
    
    if (result && shouldWrite) {
        return writeChecksumsToFile();
    }
    
    return result;
}

// ============================================================================
// Game-Specific Checksum Calculation Functions
// ============================================================================

bool ChecksumCalculator::calculateChecksumPokemonRedBlue() {
    redBlueBank1Start = 0x2598;
    
    if (isJapanese) {
        // Japanese Red/Green has different Checksum range and location (yet to test Japanese Blue)
        redBlueBank1End = 0x3593;
        redBlueBank1ChecksumLocation = 0x3594;
    } else {
        // English Red/Blue
        redBlueBank1End = 0x3522;
        redBlueBank1ChecksumLocation = 0x3523;
    }
    
    if (0x7A52 >= fileSize) {
        std::cerr << "Error: Address out of range (file size: 0x" 
                  << std::hex << fileSize << ", need at least 0x7A53)" << std::endl;
        return false;
    }
    
    std::cout << "\n=== Pokemon Red/Blue Checksum Calculation ===" << std::endl;
    std::cout << "File: " << fileName << " (" << std::dec << fileSize << " bytes)" << std::endl;
    
    // Bank 1
    std::cout << "\n--- Bank 1 ---" << std::endl;
    std::cout << "Range: 0x" << HexUtils::toHexString(redBlueBank1Start, 4) 
              << " - 0x" << HexUtils::toHexString(redBlueBank1End, 4) << std::endl;
    
    redBlueBank1Checksum = calculateRedBlue8BitChecksum(redBlueBank1Start, redBlueBank1End, redBlueBank1Sum);
    redBlueBank1StoredChecksum = readU8(redBlueBank1ChecksumLocation);
    redBlueBank1Matches = (redBlueBank1Checksum == redBlueBank1StoredChecksum);
    
    std::cout << "Sum: 0x" << std::hex << redBlueBank1Sum << std::endl;
    std::cout << "Checksum: calc=0x" << HexUtils::toHexString(redBlueBank1Checksum, 2)
              << " stored=0x" << HexUtils::toHexString(redBlueBank1StoredChecksum, 2)
              << " @ 0x" << HexUtils::toHexString(redBlueBank1ChecksumLocation, 4)
              << (redBlueBank1Matches ? " OK" : " MISMATCH") << std::endl;
    
    // Bank 2 (base address 0x4000)
    std::cout << "\n--- Bank 2 (base 0x4000) ---" << std::endl;
    calculateRedBlueBankChecksums(0x4000, redBlueBank2);
    
    // Bank 3 (base address 0x6000)
    std::cout << "\n--- Bank 3 (base 0x6000) ---" << std::endl;
    calculateRedBlueBankChecksums(0x6000, redBlueBank3);
    
    std::cout << "\n=============================================\n" << std::endl;
    
    return true;
}

bool ChecksumCalculator::calculateChecksumPokemonGoldSilver() {
    if (isJapanese) {
        // Japanese Gold/Silver addresses
        goldSilverStart1 = 0x2009;
        goldSilverEnd1 = 0x2C8B;  // 0x2009 + 0x0C83 - 1
        goldSilverChecksum1Location = 0x2D0D;
        
        // Checksum 2 is a single range in Japanese version
        goldSilverRanges2 = {
            {0x7209, 0x7E8B}  // 0x7209 + 0x0C83 - 1
        };
        goldSilverChecksum2Location = 0x7F0D;
    } else {
        // English Gold/Silver addresses
        goldSilverStart1 = 0x2009;
        goldSilverEnd1 = 0x2D68;
        goldSilverChecksum1Location = 0x2D69;
        
        goldSilverRanges2 = {
            {0x0C6B, 0x17EC},
            {0x3D96, 0x3F3F},
            {0x7E39, 0x7E6C}
        };
        goldSilverChecksum2Location = 0x7E6D;
    }
    
    // Check file size
    if (goldSilverChecksum2Location >= fileSize) {
        std::cerr << "Error: Address out of range (file size: 0x" 
                  << std::hex << fileSize << ", need at least 0x" << (goldSilverChecksum2Location + 2) << ")" << std::endl;
        return false;
    }
    
    std::cout << "\n=== Pokemon Gold/Silver Checksum Calculation";
    if (isJapanese) std::cout << " (Japanese)";
    std::cout << " ===" << std::endl;
    std::cout << "File: " << fileName << " (" << std::dec << fileSize << " bytes)" << std::endl;
    
    // First checksum
    std::cout << "\n--- Checksum 1 ---" << std::endl;
    std::cout << "Range: 0x" << HexUtils::toHexString(goldSilverStart1, 4) 
              << " - 0x" << HexUtils::toHexString(goldSilverEnd1, 4) << std::endl;
    std::cout << "Checksum location: 0x" << HexUtils::toHexString(goldSilverChecksum1Location, 4) << std::endl;
    
    goldSilverChecksum1 = calculateGBC16BitChecksum(goldSilverStart1, goldSilverEnd1, goldSilverTotalSum1);
    goldSilverStoredChecksum1 = readU16LE(goldSilverChecksum1Location);
    goldSilverChecksum1Matches = (goldSilverChecksum1 == goldSilverStoredChecksum1);
    
    std::cout << "Sum of range: 0x" << std::hex << goldSilverTotalSum1 << std::endl;
    std::cout << "*** CHECKSUM 1: calc=0x" << HexUtils::toHexString(goldSilverChecksum1, 4)
              << " stored=0x" << HexUtils::toHexString(goldSilverStoredChecksum1, 4)
              << " (bytes: 0x" << formatReversedBytes16(goldSilverChecksum1) << ")"
              << (goldSilverChecksum1Matches ? " OK" : " MISMATCH") << " ***" << std::endl;
    
    // Second checksum
    if (isJapanese) {
        std::cout << "\n--- Checksum 2 ---" << std::endl;
    } else {
        std::cout << "\n--- Checksum 2 (non-contiguous) ---" << std::endl;
    }
    
    for (const auto& range : goldSilverRanges2) {
        std::cout << "Range: 0x" << HexUtils::toHexString(range.first, 4) 
                  << " - 0x" << HexUtils::toHexString(range.second, 4) << std::endl;
    }
    std::cout << "Checksum location: 0x" << HexUtils::toHexString(goldSilverChecksum2Location, 4) << std::endl;
    
    goldSilverChecksum2 = calculateGBC16BitChecksumMultiRange(goldSilverRanges2, goldSilverTotalSum2);
    goldSilverStoredChecksum2 = readU16LE(goldSilverChecksum2Location);
    goldSilverChecksum2Matches = (goldSilverChecksum2 == goldSilverStoredChecksum2);
    
    std::cout << "Sum of ranges: 0x" << std::hex << goldSilverTotalSum2 << std::endl;
    std::cout << "*** CHECKSUM 2: calc=0x" << HexUtils::toHexString(goldSilverChecksum2, 4)
              << " stored=0x" << HexUtils::toHexString(goldSilverStoredChecksum2, 4)
              << " (bytes: 0x" << formatReversedBytes16(goldSilverChecksum2) << ")"
              << (goldSilverChecksum2Matches ? " OK" : " MISMATCH") << " ***" << std::endl;
    std::cout << "=============================================\n" << std::endl;
    
    return true;
}

bool ChecksumCalculator::calculateChecksumPokemonCrystal() {
    crystalStart1 = 0x2009;
    crystalChecksum1Location = 0x2D0D;
    
    if (isJapanese) {
        // Japanese Crystal has different Checksum 2 range
        crystalEnd1 = 0x2AE2;
        crystalStart2 = 0x7209;
        crystalEnd2 = 0x7CE2;
        crystalChecksum2Location = 0x7F0D;
    } else {
        // Regular Crystal
        crystalEnd1 = 0x2B82;
        crystalStart2 = 0x1209;
        crystalEnd2 = 0x1D82;
        crystalChecksum2Location = 0x1F0D;
    }
    
    if (crystalEnd1 >= fileSize || crystalEnd2 >= fileSize) {
        std::cerr << "Error: Address out of range (file size: 0x" 
                  << std::hex << fileSize << ")" << std::endl;
        return false;
    }
    
    std::cout << "\n=== Pokemon Crystal Checksum Calculation";
    if (isJapanese) std::cout << " (Japanese)";
    std::cout << " ===" << std::endl;
    std::cout << "File: " << fileName << " (" << std::dec << fileSize << " bytes)" << std::endl;
    
    // First checksum
    std::cout << "\n--- Checksum 1 ---" << std::endl;
    std::cout << "Range: 0x" << HexUtils::toHexString(crystalStart1, 4) 
              << " - 0x" << HexUtils::toHexString(crystalEnd1, 4) << std::endl;
    std::cout << "Checksum location: 0x" << HexUtils::toHexString(crystalChecksum1Location, 4) << std::endl;
    
    crystalChecksum1 = calculateGBC16BitChecksum(crystalStart1, crystalEnd1, crystalTotalSum1);
    crystalStoredChecksum1 = readU16LE(crystalChecksum1Location);
    crystalChecksum1Matches = (crystalChecksum1 == crystalStoredChecksum1);
    
    std::cout << "Sum of range: 0x" << std::hex << crystalTotalSum1 << std::endl;
    std::cout << "*** CHECKSUM 1: calc=0x" << HexUtils::toHexString(crystalChecksum1, 4)
              << " stored=0x" << HexUtils::toHexString(crystalStoredChecksum1, 4)
              << " (bytes: 0x" << formatReversedBytes16(crystalChecksum1) << ")"
              << (crystalChecksum1Matches ? " OK" : " MISMATCH") << " ***" << std::endl;
    
    // Second checksum
    std::cout << "\n--- Checksum 2 ---" << std::endl;
    std::cout << "Range: 0x" << HexUtils::toHexString(crystalStart2, 4) 
              << " - 0x" << HexUtils::toHexString(crystalEnd2, 4) << std::endl;
    std::cout << "Checksum location: 0x" << HexUtils::toHexString(crystalChecksum2Location, 4) << std::endl;
    
    crystalChecksum2 = calculateGBC16BitChecksum(crystalStart2, crystalEnd2, crystalTotalSum2);
    crystalStoredChecksum2 = readU16LE(crystalChecksum2Location);
    crystalChecksum2Matches = (crystalChecksum2 == crystalStoredChecksum2);
    
    std::cout << "Sum of range: 0x" << std::hex << crystalTotalSum2 << std::endl;
    std::cout << "*** CHECKSUM 2: calc=0x" << HexUtils::toHexString(crystalChecksum2, 4)
              << " stored=0x" << HexUtils::toHexString(crystalStoredChecksum2, 4)
              << " (bytes: 0x" << formatReversedBytes16(crystalChecksum2) << ")"
              << (crystalChecksum2Matches ? " OK" : " MISMATCH") << " ***" << std::endl;
    std::cout << "=============================================\n" << std::endl;
    
    return true;
}

bool ChecksumCalculator::calculateChecksumPokemonGeneration3() {
    // Check file size (need at least 128KB for full save structure)
    const size_t requiredSize = 0x20000;
    
    if (fileSize < requiredSize) {
        std::cerr << "Error: File too small (size: 0x" << std::hex << fileSize 
                  << ", need at least 0x" << requiredSize << ")" << std::endl;
        return false;
    }

    // No Japanese check as Gen 3 checksums are same as English (as far as I know)
    
    std::cout << "\n=== " << gameName << " Checksum Calculation ===" << std::endl;
    std::cout << "File: " << fileName << " (" << std::dec << fileSize << " bytes)" << std::endl;
    
    // Calculate checksums for Save A (base 0x000000)
    calculateGen3SaveBlock(0x000000, gen3SaveA, "Save A");
    
    // Calculate checksums for Save B (base 0x00E000)
    calculateGen3SaveBlock(0x00E000, gen3SaveB, "Save B");
    
    // Determine which save is current
    gen3SaveAIsCurrent = (gen3SaveA.saveIndex > gen3SaveB.saveIndex);
    
    std::cout << "\n--- Summary ---" << std::endl;
    std::cout << "Save A index: " << std::dec << gen3SaveA.saveIndex << std::endl;
    std::cout << "Save B index: " << std::dec << gen3SaveB.saveIndex << std::endl;
    std::cout << "Current save: " << (gen3SaveAIsCurrent ? "A" : "B") << std::endl;
    
    // Count mismatches
    int mismatchesA = 0, mismatchesB = 0;
    for (int i = 0; i < 14; i++) {
        if (!gen3SaveA.sections[i].matches) mismatchesA++;
        if (!gen3SaveB.sections[i].matches) mismatchesB++;
    }
    
    if (mismatchesA > 0) {
        std::cout << "Save A: " << mismatchesA << " checksum mismatch(es)" << std::endl;
    } else {
        std::cout << "Save A: All checksums valid" << std::endl;
    }
    
    if (mismatchesB > 0) {
        std::cout << "Save B: " << mismatchesB << " checksum mismatch(es)" << std::endl;
    } else {
        std::cout << "Save B: All checksums valid" << std::endl;
    }
    
    std::cout << "=============================================\n" << std::endl;
    
    return true;
}

// ============================================================================
// Helper Calculation Functions
// ============================================================================

uint8_t ChecksumCalculator::calculateRedBlue8BitChecksum(size_t start, size_t end, uint32_t& outSum) {
    outSum = 0;
    for (size_t i = start; i <= end; i++) {
        outSum += readU8(i);
    }
    uint8_t sumMod = outSum & 0xFF;
    return ~sumMod;
}

uint16_t ChecksumCalculator::calculateGBC16BitChecksum(size_t start, size_t end, uint32_t& outSum) {
    outSum = 0;
    for (size_t i = start; i <= end; i++) {
        outSum += readU8(i);
    }
    return outSum & 0xFFFF;
}

uint16_t ChecksumCalculator::calculateGBC16BitChecksumMultiRange(const std::vector<std::pair<size_t, size_t>>& ranges, uint32_t& outSum) {
    outSum = 0;
    for (const auto& range : ranges) {
        for (size_t i = range.first; i <= range.second; i++) {
            outSum += readU8(i);
        }
    }
    return outSum & 0xFFFF;
}

uint16_t ChecksumCalculator::calculateGen3SectionChecksum(size_t baseAddr, size_t dataSize) {
    return Generation3Utils::calculateSectionChecksum(fileBuffer, baseAddr, dataSize);
}

void ChecksumCalculator::calculateRedBlueBankChecksums(size_t baseAddr, RedBlueBankData& bankData) {
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
    
    // Check if entire bank is filled with 0xFF (unused box)
    bool isAllFF = true;
    for (size_t i = baseAddr + mainStart; i <= baseAddr + mainEnd; i++) {
        if (readU8(i) != 0xFF) {
            isAllFF = false;
            break;
        }
    }
    
    bankData.mainChecksum = calculateRedBlue8BitChecksum(
        baseAddr + mainStart, 
        baseAddr + mainEnd, 
        bankData.mainSum
    );
    bankData.mainChecksumLocation = baseAddr + mainChecksumOffset;
    bankData.mainStoredChecksum = readU8(bankData.mainChecksumLocation);
    
    if (isAllFF) {
        bankData.mainMatches = true;
        std::cout << "  [Bank filled with 0xFF - unused, treating as valid]" << std::endl;
    } else {
        bankData.mainMatches = (bankData.mainChecksum == bankData.mainStoredChecksum);
    }
    
    std::cout << "  Main range: 0x" << HexUtils::toHexString(baseAddr + mainStart, 4) 
              << " - 0x" << HexUtils::toHexString(baseAddr + mainEnd, 4) << std::endl;
    std::cout << "  Main sum: 0x" << std::hex << bankData.mainSum 
              << ", Checksum: calc=0x" << HexUtils::toHexString(bankData.mainChecksum, 2)
              << " stored=0x" << HexUtils::toHexString(bankData.mainStoredChecksum, 2)
              << " @ 0x" << HexUtils::toHexString(bankData.mainChecksumLocation, 4)
              << (bankData.mainMatches ? " OK" : " MISMATCH") << std::endl;
    
    std::cout << "  Sub-checksums:" << std::endl;
    for (int i = 0; i < 6; i++) {
        bankData.subChecksums[i] = calculateRedBlue8BitChecksum(
            baseAddr + subRanges[i][0],
            baseAddr + subRanges[i][1],
            bankData.subSums[i]
        );
        bankData.subChecksumLocations[i] = baseAddr + subChecksumOffsets[i];
        bankData.subStoredChecksums[i] = readU8(bankData.subChecksumLocations[i]);
        
        if (isAllFF) {
            bankData.subMatches[i] = true;
        } else {
            bankData.subMatches[i] = (bankData.subChecksums[i] == bankData.subStoredChecksums[i]);
        }
        
        std::cout << "    [" << i << "] 0x" << HexUtils::toHexString(baseAddr + subRanges[i][0], 4)
                  << " - 0x" << HexUtils::toHexString(baseAddr + subRanges[i][1], 4)
                  << " : sum=0x" << std::hex << bankData.subSums[i]
                  << ", calc=0x" << HexUtils::toHexString(bankData.subChecksums[i], 2)
                  << " stored=0x" << HexUtils::toHexString(bankData.subStoredChecksums[i], 2)
                  << " @ 0x" << HexUtils::toHexString(bankData.subChecksumLocations[i], 4)
                  << (bankData.subMatches[i] ? " OK" : " MISMATCH") << std::endl;
    }
}

void ChecksumCalculator::calculateGen3SaveBlock(size_t blockBaseAddr, Gen3SaveBlock& saveBlock, const char* blockName) {
    std::cout << "\n--- " << blockName << " (base 0x" << HexUtils::toHexString(blockBaseAddr, 5) << ") ---" << std::endl;
    
    saveBlock.valid = true;
    
    for (int i = 0; i < 14; i++) {
        size_t sectionBase = blockBaseAddr + (i * 0x1000);
        
        uint16_t sectionId = readU16LE(sectionBase + 0x0FF4);
        uint16_t storedChecksum = readU16LE(sectionBase + 0x0FF6);
        uint32_t saveIndex = readU32LE(sectionBase + 0x0FFC);
        
        if (sectionId > 13) {
            std::cerr << "Warning: Invalid section ID " << sectionId << " at section " << i << std::endl;
            saveBlock.valid = false;
            continue;
        }
        
        size_t dataSize = Generation3Utils::GEN3_SECTION_SIZES[sectionId];
        uint16_t calculatedChecksum = Generation3Utils::calculateSectionChecksum(fileBuffer, sectionBase, dataSize);
        
        // Populate the common structure
        saveBlock.sections[i].sectionId = sectionId;
        saveBlock.sections[i].saveIndex = saveIndex;
        saveBlock.sections[i].dataSize = dataSize;
        saveBlock.sections[i].sectionBaseAddress = sectionBase;
        saveBlock.sections[i].calculatedChecksum = calculatedChecksum;
        saveBlock.sections[i].storedChecksum = storedChecksum;
        saveBlock.sections[i].checksumLocation = sectionBase + 0x0FF6;
        saveBlock.sections[i].matches = (calculatedChecksum == storedChecksum);
        
        std::cout << "  Section " << std::dec << std::setw(2) << i 
                  << " [ID " << std::setw(2) << sectionId << "]: "
                  << "calc=0x" << HexUtils::toHexString(calculatedChecksum, 4)
                  << " stored=0x" << HexUtils::toHexString(storedChecksum, 4)
                  << " @ 0x" << HexUtils::toHexString(sectionBase + 0x0FF6, 5)
                  << (saveBlock.sections[i].matches ? " OK" : " MISMATCH")
                  << std::endl;
    }
    
    saveBlock.saveIndex = saveBlock.sections[13].saveIndex;
    std::cout << "  Save Index: " << std::dec << saveBlock.saveIndex << std::endl;
}

size_t ChecksumCalculator::findSectionOffset(const Gen3SaveBlock& saveBlock, uint16_t sectionId) {
    return Generation3Utils::findSectionOffset(saveBlock.sections, sectionId);
}

void ChecksumCalculator::calculatePartyPokemonChecksums(const Gen3SaveBlock& saveBlock,
                                                       std::vector<PokemonChecksumResult>& results,
                                                       const std::string& saveBlockName) {
    size_t sectionOffset = findSectionOffset(saveBlock, 1);
    if (sectionOffset == static_cast<size_t>(-1)) {
        std::cerr << "Warning: Could not find Section 1 in " << saveBlockName << std::endl;
        return;
    }
    
    size_t teamSizeOffset;
    size_t teamPokemonOffset;
    
    if (gameName == "Pokemon FireRed" || gameName == "Pokemon LeafGreen") {
        teamSizeOffset = 0x0034;
        teamPokemonOffset = 0x0038;
    } else {
        teamSizeOffset = 0x0234;
        teamPokemonOffset = 0x0238;
    }
    
    uint32_t teamSize = readU32LE(sectionOffset + teamSizeOffset);
    
    if (teamSize > 6) {
        std::cerr << "Warning: Invalid team size " << teamSize << " in " << saveBlockName << std::endl;
        teamSize = 0;
    }
    
    for (uint32_t i = 0; i < teamSize; i++) {
        size_t pokemonOffset = sectionOffset + teamPokemonOffset + (i * 100);
        std::string locationStr = saveBlockName + " Party " + std::to_string(i + 1);
        
        results.push_back(calculatePokemonChecksumResult(pokemonOffset, locationStr));
    }
}

void ChecksumCalculator::calculateBoxPokemonChecksums(const Gen3SaveBlock& saveBlock,
                                                     std::vector<PokemonChecksumResult>& results,
                                                     const std::string& saveBlockName) {
    // Build data ranges for each section (5-13)
    std::vector<std::pair<size_t, size_t>> dataRanges;
    
    for (int sectionId = 5; sectionId <= 13; sectionId++) {
        size_t sectionOffset = findSectionOffset(saveBlock, sectionId);
        if (sectionOffset == static_cast<size_t>(-1)) continue;
        
        size_t startOffset = (sectionId == 5) ? 0x04 : 0x00;
        size_t usableBytes = 0x0F80 - startOffset;
        
        dataRanges.push_back({sectionOffset + startOffset, usableBytes});
    }
    
    // Helper to read bytes across section boundaries
    auto readBytesAcrossSections = [&](size_t logicalStart, size_t length) -> std::vector<uint8_t> {
        std::vector<uint8_t> data(length);
        size_t bytesRead = 0;
        size_t currentLogicalPos = 0;
        
        for (const auto& range : dataRanges) {
            if (bytesRead >= length) break;
            
            size_t rangeEnd = currentLogicalPos + range.second;
            
            if (logicalStart >= currentLogicalPos && logicalStart < rangeEnd) {
                size_t offsetInRange = logicalStart - currentLogicalPos;
                size_t bytesToRead = std::min(length - bytesRead, range.second - offsetInRange);
                
                for (size_t i = 0; i < bytesToRead; i++) {
                    data[bytesRead + i] = readU8(range.first + offsetInRange + i);
                }
                bytesRead += bytesToRead;
                logicalStart += bytesToRead;
            } else if (logicalStart < currentLogicalPos && rangeEnd > logicalStart) {
                size_t bytesToRead = std::min(length - bytesRead, range.second);
                
                for (size_t i = 0; i < bytesToRead; i++) {
                    data[bytesRead + i] = readU8(range.first + i);
                }
                bytesRead += bytesToRead;
                logicalStart += bytesToRead;
            }
            
            currentLogicalPos = rangeEnd;
        }
        
        return data;
    };
    
    // Helper to get physical address from logical offset
    auto getPhysicalAddress = [&](size_t logicalPos) -> size_t {
        size_t currentLogicalPos = 0;
        
        for (const auto& range : dataRanges) {
            if (logicalPos < currentLogicalPos + range.second) {
                return range.first + (logicalPos - currentLogicalPos);
            }
            currentLogicalPos += range.second;
        }
        return 0;
    };
    
    // Helper to read 32-bit value across sections
    auto readU32Across = [&](size_t logicalOffset) -> uint32_t {
        std::vector<uint8_t> bytes = readBytesAcrossSections(logicalOffset, 4);
        return static_cast<uint32_t>(bytes[0]) | 
               (static_cast<uint32_t>(bytes[1]) << 8) | 
               (static_cast<uint32_t>(bytes[2]) << 16) | 
               (static_cast<uint32_t>(bytes[3]) << 24);
    };
    
    // Helper to read 16-bit value across sections
    auto readU16Across = [&](size_t logicalOffset) -> uint16_t {
        std::vector<uint8_t> bytes = readBytesAcrossSections(logicalOffset, 2);
        return static_cast<uint16_t>(bytes[0]) | (static_cast<uint16_t>(bytes[1]) << 8);
    };
    
    size_t totalPokemonProcessed = 0;
    size_t logicalOffset = 0;
    
    while (totalPokemonProcessed < 420) {
        uint32_t personality = readU32Across(logicalOffset);
        
        if (personality == 0) {
            logicalOffset += 80;
            totalPokemonProcessed++;
            continue;
        }
        
        uint32_t otid = readU32Across(logicalOffset + 4);
        uint32_t key = personality ^ otid;
        
        // Decrypt and sum the 48 bytes of data
        uint32_t sum = 0;
        for (int i = 0; i < 12; i++) {
            uint32_t encryptedWord = readU32Across(logicalOffset + 0x20 + (i * 4));
            uint32_t decryptedWord = encryptedWord ^ key;
            
            sum += (decryptedWord & 0xFFFF);
            sum += ((decryptedWord >> 16) & 0xFFFF);
        }
        
        uint16_t calculatedChecksum = sum & 0xFFFF;
        uint16_t storedChecksum = readU16Across(logicalOffset + 0x1C);
        size_t checksumPhysicalAddress = getPhysicalAddress(logicalOffset + 0x1C);
        
        int boxNumber = static_cast<int>(totalPokemonProcessed / 30);
        int slotInBox = static_cast<int>(totalPokemonProcessed % 30);
        
        PokemonChecksumResult result;
        result.location = checksumPhysicalAddress;
        result.calculated = calculatedChecksum;
        result.stored = storedChecksum;
        result.valid = (calculatedChecksum == storedChecksum);
        result.locationStr = saveBlockName + " Box " + std::to_string(boxNumber + 1) + 
                           " Slot " + std::to_string(slotInBox + 1);
        
        results.push_back(result);
        
        logicalOffset += 80;
        totalPokemonProcessed++;
    }
}

void ChecksumCalculator::calculateAllPokemonChecksums(const Gen3SaveBlock& saveBlock,
                                                     std::vector<PokemonChecksumResult>& results,
                                                     const std::string& saveBlockName) {
    results.clear();
    calculatePartyPokemonChecksums(saveBlock, results, saveBlockName);
    calculateBoxPokemonChecksums(saveBlock, results, saveBlockName);
}

bool ChecksumCalculator::calculatePokemonChecksum() {
    if (gameMode != GAME_POKEMON_GENERATION3) {
        std::cerr << "Error: -p flag is only for Generation 3 games" << std::endl;
        std::cerr << "Current game: " << gameName << std::endl;
        return false;
    }
    
    if (!calculateChecksumPokemonGeneration3()) {
        return false;
    }
    
    std::cout << "\n=== Pokemon Checksum Verification ===" << std::endl;
    std::cout << "Checking all Pokemon in party and boxes..." << std::endl;
    
    calculateAllPokemonChecksums(gen3SaveA, pokemonResultsSaveA, "Save A");
    calculateAllPokemonChecksums(gen3SaveB, pokemonResultsSaveB, "Save B");
    
    int invalidCountA = 0, invalidCountB = 0;
    for (const auto& result : pokemonResultsSaveA) {
        if (!result.valid) invalidCountA++;
    }
    for (const auto& result : pokemonResultsSaveB) {
        if (!result.valid) invalidCountB++;
    }
    
    std::cout << "\nSave A: Found " << pokemonResultsSaveA.size() << " Pokemon, " 
              << invalidCountA << " invalid checksums" << std::endl;
    std::cout << "Save B: Found " << pokemonResultsSaveB.size() << " Pokemon, " 
              << invalidCountB << " invalid checksums" << std::endl;
    
    if (invalidCountA > 0) {
        std::cout << "\nInvalid checksums in Save A:" << std::endl;
        for (const auto& result : pokemonResultsSaveA) {
            if (!result.valid) {
                std::cout << "  " << result.locationStr << " @ 0x" 
                          << HexUtils::toHexString(result.location, 5)
                          << " - calc: 0x" << HexUtils::toHexString(result.calculated, 4)
                          << " stored: 0x" << HexUtils::toHexString(result.stored, 4)
                          << std::endl;
            }
        }
    }
    
    if (invalidCountB > 0) {
        std::cout << "\nInvalid checksums in Save B:" << std::endl;
        for (const auto& result : pokemonResultsSaveB) {
            if (!result.valid) {
                std::cout << "  " << result.locationStr << " @ 0x" 
                          << HexUtils::toHexString(result.location, 5)
                          << " - calc: 0x" << HexUtils::toHexString(result.calculated, 4)
                          << " stored: 0x" << HexUtils::toHexString(result.stored, 4)
                          << std::endl;
            }
        }
    }
    
    if (invalidCountA == 0 && invalidCountB == 0) {
        std::cout << "\nAll Pokemon checksums are valid!" << std::endl;
    }
    
    return true;
}

// ============================================================================
// File Writing
// ============================================================================

bool ChecksumCalculator::writeChecksumsToFile() {
    std::string outputFile;
    
    if (shouldOverwrite) {
        if (!showOverwriteConfirmDialog(HexUtils::getBaseName(fileName))) {
            std::cout << "\nOverwrite cancelled. Exiting program." << std::endl;
            return false;
        }
        
        outputFile = fileName;
    } else {
        mkdir("edited_files", 0755);
        
        std::string baseName = HexUtils::getBaseName(fileName);
        size_t dotPos = baseName.rfind('.');
        std::string nameWithoutExt = baseName.substr(0, dotPos);
        std::string extension = (dotPos != std::string::npos) ? baseName.substr(dotPos) : "";
        outputFile = "edited_files/" + nameWithoutExt + "_checksum" + extension;
    }
    
    std::string outputBuffer = fileBuffer;
    
    switch (gameMode) {
        case GAME_POKEMON_RED_BLUE:
            outputBuffer[redBlueBank1ChecksumLocation] = static_cast<char>(redBlueBank1Checksum);
            
            outputBuffer[redBlueBank2.mainChecksumLocation] = static_cast<char>(redBlueBank2.mainChecksum);
            for (int i = 0; i < 6; i++) {
                outputBuffer[redBlueBank2.subChecksumLocations[i]] = static_cast<char>(redBlueBank2.subChecksums[i]);
            }
            
            outputBuffer[redBlueBank3.mainChecksumLocation] = static_cast<char>(redBlueBank3.mainChecksum);
            for (int i = 0; i < 6; i++) {
                outputBuffer[redBlueBank3.subChecksumLocations[i]] = static_cast<char>(redBlueBank3.subChecksums[i]);
            }
            break;
            
        case GAME_POKEMON_GOLD_SILVER:
            writeU16LE(outputBuffer, goldSilverChecksum1Location, goldSilverChecksum1);
            writeU16LE(outputBuffer, goldSilverChecksum2Location, goldSilverChecksum2);
            break;
            
        case GAME_POKEMON_CRYSTAL:
            writeU16LE(outputBuffer, crystalChecksum1Location, crystalChecksum1);
            writeU16LE(outputBuffer, crystalChecksum2Location, crystalChecksum2);
            break;
            
        case GAME_POKEMON_GENERATION3:
            for (int i = 0; i < 14; i++) {
                writeU16LE(outputBuffer, gen3SaveA.sections[i].checksumLocation, 
                          gen3SaveA.sections[i].calculatedChecksum);
                writeU16LE(outputBuffer, gen3SaveB.sections[i].checksumLocation, 
                          gen3SaveB.sections[i].calculatedChecksum);
            }
            break;
    }
    
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile) {
        std::cerr << "Failed to create output file: " << outputFile << std::endl;
        return false;
    }
    
    outFile.write(outputBuffer.c_str(), static_cast<std::streamsize>(fileSize));
    outFile.close();
    
    if (shouldOverwrite) {
        std::cout << "\nChecksums written (file overwritten): " << outputFile << std::endl;
    } else {
        std::cout << "\nChecksums written to: " << outputFile << std::endl;
    }
    
    return false;
}

// ============================================================================
// Formatting Helpers
// ============================================================================

std::string ChecksumCalculator::formatReversedBytes16(uint16_t value) {
    uint8_t low = value & 0xFF;
    uint8_t high = (value >> 8) & 0xFF;
    std::stringstream ss;
    ss << HexUtils::toHexString(low, 2) << HexUtils::toHexString(high, 2);
    return ss.str();
}

// ============================================================================
// Event Handling
// ============================================================================

void ChecksumCalculator::handleEvent(SDL_Event& event) {
    if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_ESCAPE || event.key.key == SDLK_Q) {
            quit();
        }
    } else if (pokemonChecksumMode) {
        if (event.type == SDL_EVENT_MOUSE_WHEEL) {
            addScrollVelocity(-event.wheel.y * 0.5f);
        } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
            handleScrollbarClick(static_cast<int>(event.button.x), static_cast<int>(event.button.y));
        } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            handleScrollbarRelease();
        } else if (event.type == SDL_EVENT_MOUSE_MOTION && scrollbar.dragging) {
            handleScrollbarDrag(static_cast<int>(event.motion.y));
        }
    }
}

void ChecksumCalculator::update(float deltaTime) {
    SDLAppBase::update(deltaTime);
}

// ============================================================================
// Rendering
// ============================================================================

void ChecksumCalculator::render() {
    SDL_SetRenderDrawColor(renderer, colors.background.r, colors.background.g, 
                          colors.background.b, 255);
    SDL_RenderClear(renderer);
    
    int y = 15;
    std::stringstream ss;
    
    renderCenteredText("CHECKSUM CALCULATOR", y, colors.accent);
    y += charHeight + 5;
    renderCenteredText(gameName, y, colors.highlight);
    y += charHeight + 10;
    
    std::string baseName = HexUtils::getBaseName(fileName);
    renderCenteredText("File: " + baseName, y, colors.text);
    y += charHeight + 10;
    
    renderLine(30, y, windowWidth - 30, y, {80, 80, 80, 255});
    y += 10;

    // Individual pokemon checksum mode
    if (pokemonChecksumMode) {
        renderCenteredText("=== POKEMON CHECKSUM VERIFICATION ===", y, colors.accent);
        y += charHeight + 10;
        
        // Count invalid checksums
        int invalidA = 0, invalidB = 0;
        for (const auto& r : pokemonResultsSaveA) if (!r.valid) invalidA++;
        for (const auto& r : pokemonResultsSaveB) if (!r.valid) invalidB++;
        
        // Current save indicator
        std::string currentSave = gen3SaveAIsCurrent ? "A" : "B";
        renderCenteredText("Current Save: " + currentSave, y, colors.highlight);
        y += charHeight + 10;
        
        // Summary
        ss.str("");
        ss << "Save A: " << pokemonResultsSaveA.size() << " Pokemon, " 
           << invalidA << " invalid";
        SDL_Color colorA = (invalidA == 0) ? colors.success : colors.error;
        renderCenteredText(ss.str(), y, colorA);
        y += charHeight + 3;
        
        ss.str("");
        ss << "Save B: " << pokemonResultsSaveB.size() << " Pokemon, " 
           << invalidB << " invalid";
        SDL_Color colorB = (invalidB == 0) ? colors.success : colors.error;
        renderCenteredText(ss.str(), y, colorB);
        y += charHeight + 15;
        
        if (invalidA == 0 && invalidB == 0) {
            renderCenteredText("ALL POKEMON", y, colors.success, largeFont);
            y += 55;
            renderCenteredText("CHECKSUMS VALID", y, colors.success, largeFont);
        } else {
            renderCenteredText("INVALID CHECKSUMS FOUND", y, colors.error);
            y += charHeight + 10;
            
            int contentStartY = y;
            int contentEndY = windowHeight - charHeight - 30;
            int contentHeight = contentEndY - contentStartY;
            int maxVisibleLines = contentHeight / charHeight;
            
            // Count total invalid entries
            std::vector<std::pair<std::string, const PokemonChecksumResult*>> allInvalid;
            
            for (const auto& result : pokemonResultsSaveA) {
                if (!result.valid) {
                    allInvalid.push_back({"Save A", &result});
                }
            }
            for (const auto& result : pokemonResultsSaveB) {
                if (!result.valid) {
                    allInvalid.push_back({"Save B", &result});
                }
            }
            
            int totalInvalidLines = static_cast<int>(allInvalid.size()) + 2;
            
            // Configure scrollbar
            scrollbar.headerOffset = contentStartY;
            scrollbar.visibleItems = static_cast<size_t>(maxVisibleLines);
            scrollbar.totalItems = static_cast<size_t>(totalInvalidLines);
            
            bool needsScrollbar = scrollbar.canScroll();
            
            // Clamp scroll offset
            if (scrollbar.offset > scrollbar.maxOffset()) {
                scrollbar.offset = scrollbar.maxOffset();
            }
            
            // Create clipping region for content
            SDL_Rect clipRect = {0, contentStartY, windowWidth - (needsScrollbar ? scrollbar.width : 0), contentHeight};
            SDL_SetRenderClipRect(renderer, &clipRect);
            
            // Render content with scroll offset
            int lineY = contentStartY - static_cast<int>(scrollbar.offset * static_cast<size_t>(charHeight));
            int currentLine = 0;
            
            // Group invalid entries by save
            auto renderSaveGroup = [&](const std::string& saveName) {
                bool headerShown = false;
                for (const auto& [save, result] : allInvalid) {
                    if (save == saveName) {
                        if (!headerShown) {
                            if (currentLine >= static_cast<int>(scrollbar.offset) && lineY < contentEndY) {
                                renderText(saveName + ":", 30, lineY, colors.warning);
                            }
                            lineY += charHeight + 3;
                            currentLine++;
                            headerShown = true;
                        }
                        
                        if (currentLine >= static_cast<int>(scrollbar.offset) && lineY < contentEndY) {
                            ss.str("");
                            ss << "  " << result->locationStr << " @ 0x" 
                               << HexUtils::toHexString(result->location, 5);
                            renderText(ss.str(), 40, lineY, colors.error);
                        }
                        lineY += charHeight;
                        currentLine++;
                    }
                }
                if (headerShown) {
                    lineY += 5;
                }
            };
            
            renderSaveGroup("Save A");
            renderSaveGroup("Save B");
            
            SDL_SetRenderClipRect(renderer, nullptr);
            
            // Draw scrollbar only if needed
            if (needsScrollbar) {
                renderScrollbar();
            }
        }
        
        renderCenteredText("Press ESC or Q to quit", windowHeight - charHeight - 15, colors.textDim);
        
        SDL_RenderPresent(renderer);
        return;
    }
    
    // Game-specific display
    switch (gameMode) {
        case GAME_POKEMON_RED_BLUE: {
            // Bank 1
            renderCenteredText("=== Bank 1 ===", y, colors.accent);
            y += charHeight + 3;
            
            ss.str("");
            ss << "Range: 0x" << HexUtils::toHexString(redBlueBank1Start, 4)
               << " - 0x" << HexUtils::toHexString(redBlueBank1End, 4)
               << "  |  Location: 0x" << HexUtils::toHexString(redBlueBank1ChecksumLocation, 4);
            renderCenteredText(ss.str(), y, colors.warning);
            y += charHeight + 5;
            
            ss.str("");
            ss << "0x" << HexUtils::toHexString(redBlueBank1Checksum, 2);
            SDL_Color bank1Color = redBlueBank1Matches ? colors.highlight : colors.error;
            renderCenteredText(ss.str(), y, bank1Color, largeFont);
            y += 55;
            
            // Bank 2
            renderCenteredText("=== Bank 2 ===", y, colors.accent);
            y += charHeight + 3;
            
            ss.str("");
            ss << "Main: 0x4000 - 0x5A4B  |  Location: 0x5A4C";
            renderCenteredText(ss.str(), y, colors.warning);
            y += charHeight + 5;
            
            ss.str("");
            ss << "0x" << HexUtils::toHexString(redBlueBank2.mainChecksum, 2);
            SDL_Color bank2Color = redBlueBank2.mainMatches ? colors.highlight : colors.error;
            renderCenteredText(ss.str(), y, bank2Color, largeFont);
            y += 55;
            
            // Sub-checksums for Bank 2
            ss.str("");
            ss << "Sub: ";
            for (int i = 0; i < 6; i++) {
                ss << HexUtils::toHexString(redBlueBank2.subChecksums[i], 2);
                if (i < 5) ss << " ";
            }
            ss << "  @ 0x5A4D-0x5A52";
            renderCenteredText(ss.str(), y, colors.text);
            y += charHeight + 15;
            
            // Bank 3
            renderCenteredText("=== Bank 3 ===", y, colors.accent);
            y += charHeight + 3;
            
            ss.str("");
            ss << "Main: 0x6000 - 0x7A4B  |  Location: 0x7A4C";
            renderCenteredText(ss.str(), y, colors.warning);
            y += charHeight + 5;
            
            ss.str("");
            ss << "0x" << HexUtils::toHexString(redBlueBank3.mainChecksum, 2);
            SDL_Color bank3Color = redBlueBank3.mainMatches ? colors.highlight : colors.error;
            renderCenteredText(ss.str(), y, bank3Color, largeFont);
            y += 55;
            
            // Sub-checksums for Bank 3
            ss.str("");
            ss << "Sub: ";
            for (int i = 0; i < 6; i++) {
                ss << HexUtils::toHexString(redBlueBank3.subChecksums[i], 2);
                if (i < 5) ss << " ";
            }
            ss << "  @ 0x7A4D-0x7A52";
            renderCenteredText(ss.str(), y, colors.text);
            break;
        }
        
        case GAME_POKEMON_GOLD_SILVER: {
            // First checksum
            renderCenteredText("=== Checksum 1 ===", y, colors.accent);
            y += charHeight + 5;
            
            ss.str("");
            ss << "Range: 0x" << HexUtils::toHexString(goldSilverStart1, 4)
               << " - 0x" << HexUtils::toHexString(goldSilverEnd1, 4)
               << "  |  Location: 0x" << HexUtils::toHexString(goldSilverChecksum1Location, 4);
            renderCenteredText(ss.str(), y, colors.warning);
            y += charHeight + 10;
            
            ss.str("");
            ss << "0x" << formatReversedBytes16(goldSilverChecksum1);
            SDL_Color gs1Color = goldSilverChecksum1Matches ? colors.highlight : colors.error;
            renderCenteredText(ss.str(), y, gs1Color, largeFont);
            y += 65;
            
            // Second checksum
            if (isJapanese) {
                // Japanese version has single range
                renderCenteredText("=== Checksum 2 ===", y, colors.accent);
                y += charHeight + 5;
                
                ss.str("");
                ss << "Range: 0x" << HexUtils::toHexString(goldSilverRanges2[0].first, 4)
                   << " - 0x" << HexUtils::toHexString(goldSilverRanges2[0].second, 4)
                   << "  |  Location: 0x" << HexUtils::toHexString(goldSilverChecksum2Location, 4);
                renderCenteredText(ss.str(), y, colors.warning);
                y += charHeight + 10;
            } else {
                // English version has multiple ranges
                renderCenteredText("=== Checksum 2 (non-contiguous) ===", y, colors.accent);
                y += charHeight + 5;
                
                ss.str("");
                ss << "Ranges: ";
                for (size_t i = 0; i < goldSilverRanges2.size(); i++) {
                    ss << "0x" << HexUtils::toHexString(goldSilverRanges2[i].first, 4)
                       << "-0x" << HexUtils::toHexString(goldSilverRanges2[i].second, 4);
                    if (i < goldSilverRanges2.size() - 1) ss << ", ";
                }
                renderCenteredText(ss.str(), y, colors.success);
                y += charHeight + 3;
                
                ss.str("");
                ss << "Location: 0x" << HexUtils::toHexString(goldSilverChecksum2Location, 4);
                renderCenteredText(ss.str(), y, colors.warning);
                y += charHeight + 10;
            }
            
            ss.str("");
            ss << "0x" << formatReversedBytes16(goldSilverChecksum2);
            SDL_Color gs2Color = goldSilverChecksum2Matches ? colors.highlight : colors.error;
            renderCenteredText(ss.str(), y, gs2Color, largeFont);
            break;
        }
        
        case GAME_POKEMON_CRYSTAL: {
            // First checksum
            renderCenteredText("=== Checksum 1 ===", y, colors.accent);
            y += charHeight + 5;
            
            ss.str("");
            ss << "Range: 0x" << HexUtils::toHexString(crystalStart1, 4)
               << " - 0x" << HexUtils::toHexString(crystalEnd1, 4)
               << "  |  Location: 0x" << HexUtils::toHexString(crystalChecksum1Location, 4);
            renderCenteredText(ss.str(), y, colors.warning);
            y += charHeight + 10;
            
            ss.str("");
            ss << "0x" << formatReversedBytes16(crystalChecksum1);
            SDL_Color crys1Color = crystalChecksum1Matches ? colors.highlight : colors.error;
            renderCenteredText(ss.str(), y, crys1Color, largeFont);
            y += 65;
            
            // Second checksum
            renderCenteredText("=== Checksum 2 ===", y, colors.accent);
            y += charHeight + 5;
            
            ss.str("");
            ss << "Range: 0x" << HexUtils::toHexString(crystalStart2, 4)
               << " - 0x" << HexUtils::toHexString(crystalEnd2, 4)
               << "  |  Location: 0x" << HexUtils::toHexString(crystalChecksum2Location, 4);
            renderCenteredText(ss.str(), y, colors.warning);
            y += charHeight + 10;
            
            ss.str("");
            ss << "0x" << formatReversedBytes16(crystalChecksum2);
            SDL_Color crys2Color = crystalChecksum2Matches ? colors.highlight : colors.error;
            renderCenteredText(ss.str(), y, crys2Color, largeFont);
            break;
        }
        
        case GAME_POKEMON_GENERATION3: {
            // Current save indicator
            ss.str("");
            ss << "Current Save: " << (gen3SaveAIsCurrent ? "A" : "B")
               << " (A:" << gen3SaveA.saveIndex << " B:" << gen3SaveB.saveIndex << ")";
            renderCenteredText(ss.str(), y, gen3SaveAIsCurrent ? colors.success : colors.warning);
            y += charHeight + 15;
            
            // Save A
            renderCenteredText("=== Save A ===", y, colors.accent);
            y += charHeight + 5;
            
            int startX = 30;
            int colWidth = 76;
            
            for (int i = 0; i < 14; i++) {
                int col = i % 7;
                int row = i / 7;
                int x = startX + col * colWidth;
                int yPos = y + row * (charHeight + 3);
                
                ss.str("");
                ss << std::dec << std::setw(2) << gen3SaveA.sections[i].sectionId << ":"
                   << HexUtils::toHexString(gen3SaveA.sections[i].calculatedChecksum, 4);
                
                SDL_Color textColor = gen3SaveA.sections[i].matches ? colors.success : colors.error;
                renderText(ss.str(), x, yPos, textColor);
            }
            y += (charHeight + 3) * 2 + 15;
            
            // Save B
            renderCenteredText("=== Save B ===", y, colors.accent);
            y += charHeight + 5;
            
            for (int i = 0; i < 14; i++) {
                int col = i % 7;
                int row = i / 7;
                int x = startX + col * colWidth;
                int yPos = y + row * (charHeight + 3);
                
                ss.str("");
                ss << std::dec << std::setw(2) << gen3SaveB.sections[i].sectionId << ":"
                   << HexUtils::toHexString(gen3SaveB.sections[i].calculatedChecksum, 4);
                
                SDL_Color textColor = gen3SaveB.sections[i].matches ? colors.success : colors.error;
                renderText(ss.str(), x, yPos, textColor);
            }
            y += (charHeight + 3) * 2 + 15;
            
            int mismatchesA = 0, mismatchesB = 0;
            for (int i = 0; i < 14; i++) {
                if (!gen3SaveA.sections[i].matches) mismatchesA++;
                if (!gen3SaveB.sections[i].matches) mismatchesB++;
            }
            
            ss.str("");
            if (mismatchesA == 0 && mismatchesB == 0) {
                ss << "All checksums valid";
                renderCenteredText(ss.str(), y, colors.success);
            } else {
                ss << "Mismatches - A: " << mismatchesA << "  B: " << mismatchesB;
                renderCenteredText(ss.str(), y, colors.error);
            }
            y += charHeight + 10;
            
            renderCenteredText("Format: SectionID:Checksum (green=valid, red=mismatch)", y, colors.textDim);
            break;
        }
    }
    
    renderCenteredText("Press ESC or Q to quit", windowHeight - charHeight - 15, colors.textDim);
    
    SDL_RenderPresent(renderer);
}