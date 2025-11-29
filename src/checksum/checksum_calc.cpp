#include "checksum_calc.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <sys/stat.h>

// Section data sizes for Pokemon Generation 3
static const size_t GEN3_SECTION_SIZES[14] = {
    3884,  // ID 0: Trainer info
    3968,  // ID 1: Team / items
    3968,  // ID 2: Game State
    3968,  // ID 3: Misc Data
    3848,  // ID 4: Rival info
    3968,  // ID 5: PC buffer A
    3968,  // ID 6: PC buffer B
    3968,  // ID 7: PC buffer C
    3968,  // ID 8: PC buffer D
    3968,  // ID 9: PC buffer E
    3968,  // ID 10: PC buffer F
    3968,  // ID 11: PC buffer G
    3968,  // ID 12: PC buffer H
    2000   // ID 13: PC buffer I
};

// ============================================================================
// Constructor
// ============================================================================

ChecksumCalculator::ChecksumCalculator()
    : SDLAppBase("Checksum Calculator", 600, 650),
      fileSize(0), gameMode(GAME_POKEMON_RED_BLUE),
      isJapanese(false), shouldWrite(false), shouldOverwrite(false),
      redBlueBank1Sum(0), redBlueBank1Checksum(0), redBlueBank1StoredChecksum(0),
      redBlueBank1ChecksumLocation(0), redBlueBank1Start(0), redBlueBank1End(0),
      redBlueBank1Matches(false),
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
      crystalStart1(0), crystalEnd1(0), crystalStart2(0), crystalEnd2(0),
      crystalChecksum1Matches(false), crystalChecksum2Matches(false),
      gen3SaveAIsCurrent(false) {
    
    memset(&redBlueBank2, 0, sizeof(redBlueBank2));
    memset(&redBlueBank3, 0, sizeof(redBlueBank3));
    memset(&gen3SaveA, 0, sizeof(gen3SaveA));
    memset(&gen3SaveB, 0, sizeof(gen3SaveB));
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
        if (gameMode == GAME_POKEMON_CRYSTAL || gameMode == GAME_POKEMON_RED_BLUE || 
            gameMode == GAME_POKEMON_GOLD_SILVER) {
            gameName += " (Japanese)";
        } else {
            std::cout << "Note: Japanese version has no known checksum difference for " << gameName << std::endl;
            std::cout << "Proceeding with regular checksum calculation." << std::endl;
        }
    }
    
    return true;
}

bool ChecksumCalculator::calculateChecksum() {
    std::string baseName = HexUtils::getBaseName(fileName);
    setWindowTitle("Checksum Calculator - " + gameName);
    
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
    
    // Read stored checksum
    redBlueBank1StoredChecksum = (unsigned char)fileBuffer[redBlueBank1ChecksumLocation];
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
    
    // Read stored checksum
    goldSilverStoredChecksum1 = (unsigned char)fileBuffer[goldSilverChecksum1Location];
    goldSilverStoredChecksum1 |= ((unsigned char)fileBuffer[goldSilverChecksum1Location + 1]) << 8;
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
    
    // Read stored checksum
    goldSilverStoredChecksum2 = (unsigned char)fileBuffer[goldSilverChecksum2Location];
    goldSilverStoredChecksum2 |= ((unsigned char)fileBuffer[goldSilverChecksum2Location + 1]) << 8;
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
    
    // Read stored checksum
    crystalStoredChecksum1 = (unsigned char)fileBuffer[crystalChecksum1Location];
    crystalStoredChecksum1 |= ((unsigned char)fileBuffer[crystalChecksum1Location + 1]) << 8;
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
    
    // Read stored checksum
    crystalStoredChecksum2 = (unsigned char)fileBuffer[crystalChecksum2Location];
    crystalStoredChecksum2 |= ((unsigned char)fileBuffer[crystalChecksum2Location + 1]) << 8;
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
        outSum += (unsigned char)fileBuffer[i];
    }
    uint8_t sumMod = outSum & 0xFF;
    return ~sumMod;
}

uint16_t ChecksumCalculator::calculateGBC16BitChecksum(size_t start, size_t end, uint32_t& outSum) {
    outSum = 0;
    for (size_t i = start; i <= end; i++) {
        outSum += (unsigned char)fileBuffer[i];
    }
    return outSum & 0xFFFF;
}

uint16_t ChecksumCalculator::calculateGBC16BitChecksumMultiRange(const std::vector<std::pair<size_t, size_t>>& ranges, uint32_t& outSum) {
    outSum = 0;
    for (const auto& range : ranges) {
        for (size_t i = range.first; i <= range.second; i++) {
            outSum += (unsigned char)fileBuffer[i];
        }
    }
    return outSum & 0xFFFF;
}

uint16_t ChecksumCalculator::calculateGen3SectionChecksum(size_t baseAddr, size_t dataSize) {
    uint32_t sum = 0;
    
    // Read 32-bit words and sum them
    for (size_t i = 0; i < dataSize; i += 4) {
        uint32_t word = 0;
        word |= (unsigned char)fileBuffer[baseAddr + i];
        word |= ((unsigned char)fileBuffer[baseAddr + i + 1]) << 8;
        word |= ((unsigned char)fileBuffer[baseAddr + i + 2]) << 16;
        word |= ((unsigned char)fileBuffer[baseAddr + i + 3]) << 24;
        sum += word;
    }
    
    // Fold to 16-bit: upper + lower
    uint16_t upper = (sum >> 16) & 0xFFFF;
    uint16_t lower = sum & 0xFFFF;
    
    return upper + lower;
}

void ChecksumCalculator::calculateRedBlueBankChecksums(size_t baseAddr, RedBlueBankData& bankData) {
    // Offsets relative to base address
    const size_t mainStart = 0x0000;
    const size_t mainEnd = 0x1A4B;
    const size_t mainChecksumOffset = 0x1A4C;
    
    // Sub-region offsets (relative to base)
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
        if ((unsigned char)fileBuffer[i] != 0xFF) {
            isAllFF = false;
            break;
        }
    }
    
    // Calculate main checksum for this bank
    bankData.mainChecksum = calculateRedBlue8BitChecksum(
        baseAddr + mainStart, 
        baseAddr + mainEnd, 
        bankData.mainSum
    );
    bankData.mainChecksumLocation = baseAddr + mainChecksumOffset;
    
    // Read stored checksum
    bankData.mainStoredChecksum = (unsigned char)fileBuffer[bankData.mainChecksumLocation];
    
    // If bank is all FF, treat as valid (unused box system)
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
    
    // Calculate 6 sub-checksums
    std::cout << "  Sub-checksums:" << std::endl;
    for (int i = 0; i < 6; i++) {
        bankData.subChecksums[i] = calculateRedBlue8BitChecksum(
            baseAddr + subRanges[i][0],
            baseAddr + subRanges[i][1],
            bankData.subSums[i]
        );
        bankData.subChecksumLocations[i] = baseAddr + subChecksumOffsets[i];
        
        // Read stored checksum
        bankData.subStoredChecksums[i] = (unsigned char)fileBuffer[bankData.subChecksumLocations[i]];
        
        // If bank is all FF, treat sub-checksums as valid too
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
        
        // Read section ID (16-bit at offset 0x0FF4)
        uint16_t sectionId = (unsigned char)fileBuffer[sectionBase + 0x0FF4];
        sectionId |= ((unsigned char)fileBuffer[sectionBase + 0x0FF5]) << 8;
        
        // Read stored checksum (16-bit at offset 0x0FF6)
        uint16_t storedChecksum = (unsigned char)fileBuffer[sectionBase + 0x0FF6];
        storedChecksum |= ((unsigned char)fileBuffer[sectionBase + 0x0FF7]) << 8;
        
        // Read save index (32-bit at offset 0x0FFC)
        uint32_t saveIndex = (unsigned char)fileBuffer[sectionBase + 0x0FFC];
        saveIndex |= ((unsigned char)fileBuffer[sectionBase + 0x0FFD]) << 8;
        saveIndex |= ((unsigned char)fileBuffer[sectionBase + 0x0FFE]) << 16;
        saveIndex |= ((unsigned char)fileBuffer[sectionBase + 0x0FFF]) << 24;
        
        // Validate section ID
        if (sectionId > 13) {
            std::cerr << "Warning: Invalid section ID " << sectionId << " at section " << i << std::endl;
            saveBlock.valid = false;
            continue;
        }
        
        // Get data size for this section
        size_t dataSize = GEN3_SECTION_SIZES[sectionId];
        
        // Calculate checksum
        uint16_t calculatedChecksum = calculateGen3SectionChecksum(sectionBase, dataSize);
        
        // Store results
        saveBlock.sections[i].sectionId = sectionId;
        saveBlock.sections[i].saveIndex = saveIndex;
        saveBlock.sections[i].dataSize = dataSize;
        saveBlock.sections[i].calculatedChecksum = calculatedChecksum;
        saveBlock.sections[i].storedChecksum = storedChecksum;
        saveBlock.sections[i].checksumLocation = sectionBase + 0x0FF6;
        saveBlock.sections[i].sectionBaseAddress = sectionBase;
        saveBlock.sections[i].matches = (calculatedChecksum == storedChecksum);
        
        std::cout << "  Section " << std::dec << std::setw(2) << i 
                  << " [ID " << std::setw(2) << sectionId << "]: "
                  << "calc=0x" << HexUtils::toHexString(calculatedChecksum, 4)
                  << " stored=0x" << HexUtils::toHexString(storedChecksum, 4)
                  << " @ 0x" << HexUtils::toHexString(sectionBase + 0x0FF6, 5)
                  << (saveBlock.sections[i].matches ? " OK" : " MISMATCH")
                  << std::endl;
    }
    
    // Get save index from last section (section 13)
    saveBlock.saveIndex = saveBlock.sections[13].saveIndex;
    std::cout << "  Save Index: " << std::dec << saveBlock.saveIndex << std::endl;
}

// ============================================================================
// Confirmation Dialog
// ============================================================================

bool ChecksumCalculator::showOverwriteConfirmation() {
    // SDL3: SDL_CreateWindow(title, w, h, flags)
    SDL_Window* confirmWindow = SDL_CreateWindow(
        "Confirm Overwrite",
        500,
        250,
        0
    );
    
    if (!confirmWindow) {
        std::cerr << "Failed to create confirmation window: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // SDL3: SDL_CreateRenderer(window, const char* name)
    SDL_Renderer* confirmRenderer = SDL_CreateRenderer(confirmWindow, nullptr);
    
    if (!confirmRenderer) {
        std::cerr << "Failed to create confirmation renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(confirmWindow);
        return false;
    }
    
    SDL_Rect yesButton = {100, 170, 120, 50};
    SDL_Rect noButton  = {280, 170, 120, 50};
    
    bool running = true;
    bool result = false;
    bool yesHover = false;
    bool noHover = false;
    
    std::string baseName = HexUtils::getBaseName(fileName);
    
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
                result = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_Y) {
                    running = false;
                    result = true;
                } else if (event.key.key == SDLK_N || event.key.key == SDLK_ESCAPE) {
                    running = false;
                    result = false;
                }
            } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
                int mx = static_cast<int>(event.motion.x);
                int my = static_cast<int>(event.motion.y);
                yesHover = isPointInRect(mx, my, yesButton);
                noHover  = isPointInRect(mx, my, noButton);
            } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    int mx = static_cast<int>(event.button.x);
                    int my = static_cast<int>(event.button.y);
                    
                    if (isPointInRect(mx, my, yesButton)) {
                        running = false;
                        result = true;
                    } else if (isPointInRect(mx, my, noButton)) {
                        running = false;
                        result = false;
                    }
                }
            }
        }
        
        SDL_SetRenderDrawColor(confirmRenderer, colors.dialogBg.r, colors.dialogBg.g, 
                              colors.dialogBg.b, 255);
        SDL_RenderClear(confirmRenderer);
        
        int messageY = 40;
        renderCenteredTextAt("WARNING", 250, messageY, colors.error, largeFont, confirmRenderer);
        messageY += 40;
        
        renderCenteredTextAt("Overwrite this file?", 250, messageY, colors.text, 
                           regularFont, confirmRenderer);
        messageY += 20;
        
        renderCenteredTextAt(baseName, 250, messageY, colors.warning, 
                           regularFont, confirmRenderer);
        
        SDL_Color yesColor = yesHover ? SDL_Color{100, 200, 100, 255}
                                      : SDL_Color{60, 150, 60, 255};
        renderFilledRect(yesButton, yesColor, confirmRenderer);
        SDL_Color yesBorder = {150, 255, 150, 255};
        renderOutlineRect(yesButton, yesBorder, confirmRenderer);
        
        int textW, textH;
        getTextSize("YES (Y)", textW, textH);
        renderText("YES (Y)", yesButton.x + (yesButton.w - textW) / 2, 
                  yesButton.y + (yesButton.h - textH) / 2, 
                  colors.text, regularFont, confirmRenderer);
        
        SDL_Color noColor = noHover ? SDL_Color{200, 100, 100, 255}
                                    : SDL_Color{150, 60, 60, 255};
        renderFilledRect(noButton, noColor, confirmRenderer);
        SDL_Color noBorder = {255, 150, 150, 255};
        renderOutlineRect(noButton, noBorder, confirmRenderer);
        
        getTextSize("NO (N)", textW, textH);
        renderText("NO (N)", noButton.x + (noButton.w - textW) / 2, 
                  noButton.y + (noButton.h - textH) / 2, 
                  colors.text, regularFont, confirmRenderer);
        
        SDL_RenderPresent(confirmRenderer);
        SDL_Delay(16);
    }
    
    SDL_DestroyRenderer(confirmRenderer);
    SDL_DestroyWindow(confirmWindow);
    
    return result;
}

// ============================================================================
// File Writing
// ============================================================================

bool ChecksumCalculator::writeChecksumsToFile() {
    std::string outputFile;
    
    if (shouldOverwrite) {
        // Show confirmation dialog
        if (!showOverwriteConfirmation()) {
            std::cout << "\nOverwrite cancelled. Exiting program." << std::endl;
            return false;
        }
        
        // Use original filename
        outputFile = fileName;
    } else {
        // Create edited_files directory if it doesn't exist
        mkdir("edited_files", 0755);
        
        // Generate output filename
        std::string baseName = HexUtils::getBaseName(fileName);
        size_t dotPos = baseName.rfind('.');
        std::string nameWithoutExt = baseName.substr(0, dotPos);
        std::string extension = (dotPos != std::string::npos) ? baseName.substr(dotPos) : "";
        outputFile = "edited_files/" + nameWithoutExt + "_checksum" + extension;
    }
    
    // Create a copy of the buffer to modify
    std::string outputBuffer = fileBuffer;
    
    // Write checksums based on game mode
    switch (gameMode) {
        case GAME_POKEMON_RED_BLUE:
            // Bank 1
            outputBuffer[redBlueBank1ChecksumLocation] = redBlueBank1Checksum;
            
            // Bank 2
            outputBuffer[redBlueBank2.mainChecksumLocation] = redBlueBank2.mainChecksum;
            for (int i = 0; i < 6; i++) {
                outputBuffer[redBlueBank2.subChecksumLocations[i]] = redBlueBank2.subChecksums[i];
            }
            
            // Bank 3
            outputBuffer[redBlueBank3.mainChecksumLocation] = redBlueBank3.mainChecksum;
            for (int i = 0; i < 6; i++) {
                outputBuffer[redBlueBank3.subChecksumLocations[i]] = redBlueBank3.subChecksums[i];
            }
            break;
            
        case GAME_POKEMON_GOLD_SILVER:
            // Write 16-bit checksums in little-endian
            outputBuffer[goldSilverChecksum1Location] = goldSilverChecksum1 & 0xFF;
            outputBuffer[goldSilverChecksum1Location + 1] = (goldSilverChecksum1 >> 8) & 0xFF;
            
            outputBuffer[goldSilverChecksum2Location] = goldSilverChecksum2 & 0xFF;
            outputBuffer[goldSilverChecksum2Location + 1] = (goldSilverChecksum2 >> 8) & 0xFF;
            break;
            
        case GAME_POKEMON_CRYSTAL:
            outputBuffer[crystalChecksum1Location] = crystalChecksum1 & 0xFF;
            outputBuffer[crystalChecksum1Location + 1] = (crystalChecksum1 >> 8) & 0xFF;
            
            outputBuffer[crystalChecksum2Location] = crystalChecksum2 & 0xFF;
            outputBuffer[crystalChecksum2Location + 1] = (crystalChecksum2 >> 8) & 0xFF;
            break;
            
        case GAME_POKEMON_GENERATION3:
            // Write all 28 section checksums (14 for each save block)
            for (int i = 0; i < 14; i++) {
                size_t locA = gen3SaveA.sections[i].checksumLocation;
                outputBuffer[locA] = gen3SaveA.sections[i].calculatedChecksum & 0xFF;
                outputBuffer[locA + 1] = (gen3SaveA.sections[i].calculatedChecksum >> 8) & 0xFF;
                
                size_t locB = gen3SaveB.sections[i].checksumLocation;
                outputBuffer[locB] = gen3SaveB.sections[i].calculatedChecksum & 0xFF;
                outputBuffer[locB + 1] = (gen3SaveB.sections[i].calculatedChecksum >> 8) & 0xFF;
            }
            break;
    }
    
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile) {
        std::cerr << "Failed to create output file: " << outputFile << std::endl;
        return false;
    }
    
    outFile.write(outputBuffer.c_str(), fileSize);
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
        if (event.key.key == SDLK_ESCAPE ||
            event.key.key == SDLK_Q) {
            quit();
        }
    }
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
            
            // Display checksums in a grid (7 per row)
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