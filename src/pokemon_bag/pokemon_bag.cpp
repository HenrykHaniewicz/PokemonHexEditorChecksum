#include "pokemon_bag.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <cstring>

#ifndef _WIN32
#include <sys/stat.h>
#else
#include <direct.h>
#endif

// ============================================================================
// Constructor
// ============================================================================

PokemonBagEditor::PokemonBagEditor()
    : SDLAppBase("Pokemon Bag Editor", 800, 640) {
    // Nothing else to initialize here; pockets are built after setGame()
}

// ============================================================================
// File loading
// ============================================================================

bool PokemonBagEditor::loadFile(const char* filename) {
    if (!HexUtils::loadFileToBuffer(filename, fileBuffer, fileSize)) {
        std::cerr << "Failed to open: " << filename << std::endl;
        return false;
    }
    fileName = filename;
    hasUnsavedChanges = false;
    return true;
}

// ============================================================================
// Gen 3 Helper Functions
// ============================================================================

bool PokemonBagEditor::isGen3Game() const {
    return gameType == GameType::RUBY_SAPPHIRE ||
           gameType == GameType::EMERALD ||
           gameType == GameType::FIRERED_LEAFGREEN;
}

bool PokemonBagEditor::findGen3CurrentSave() {
    if (fileSize < Generation3Utils::GEN3_SAVE_SIZE) {
        std::cerr << "File too small for Gen 3 save" << std::endl;
        return false;
    }

    // Read save index from both blocks to determine which is current
    // Save index is at offset 0xFFC in the first section of each block
    uint32_t saveIndex0 = DataUtils::readU32LE(fileBuffer, 
        0x0000 + Generation3Utils::GEN3_SECTION_SAVE_INDEX_OFFSET);
    uint32_t saveIndex1 = DataUtils::readU32LE(fileBuffer, 
        Generation3Utils::GEN3_BLOCK_SIZE + Generation3Utils::GEN3_SECTION_SAVE_INDEX_OFFSET);

    // The block with the higher save index is the current one
    // Handle wraparound: if one is 0xFFFFFFFF and other is 0, the 0 is newer
    if (saveIndex0 == 0xFFFFFFFF && saveIndex1 == 0) {
        gen3CurrentSaveOffset = Generation3Utils::GEN3_BLOCK_SIZE;
    } else if (saveIndex1 == 0xFFFFFFFF && saveIndex0 == 0) {
        gen3CurrentSaveOffset = 0;
    } else if (saveIndex0 >= saveIndex1) {
        gen3CurrentSaveOffset = 0;
    } else {
        gen3CurrentSaveOffset = Generation3Utils::GEN3_BLOCK_SIZE;
    }

    return true;
}

bool PokemonBagEditor::parseGen3Sections() {
    // Parse all 14 sections to find their locations
    // Sections can be in any order within the save block
    
    for (size_t i = 0; i < Generation3Utils::GEN3_NUM_SECTIONS; i++) {
        size_t sectionBase = gen3CurrentSaveOffset + (i * Generation3Utils::GEN3_SECTION_SIZE);
        
        uint16_t sectionId = DataUtils::readU16LE(fileBuffer, 
            sectionBase + Generation3Utils::GEN3_SECTION_ID_OFFSET);
        
        if (sectionId >= Generation3Utils::GEN3_NUM_SECTIONS) {
            std::cerr << "Invalid section ID " << sectionId << " at index " << i << std::endl;
            return false;
        }
        
        gen3Sections[sectionId].sectionId = sectionId;
        gen3Sections[sectionId].sectionBaseAddress = sectionBase;
    }

    // Store the offsets for sections we need
    gen3Section0Offset = gen3Sections[0].sectionBaseAddress;
    gen3Section1Offset = gen3Sections[1].sectionBaseAddress;

    // Read security key using the utility function
    gen3SecurityKey = Generation3Utils::getSecurityKey(fileBuffer, gen3GameType, gen3Section0Offset);

    return true;
}

bool PokemonBagEditor::parseGen3Pocket(PocketInfo& pocket) {
    // Gen 3 items are 4 bytes each: 2 bytes item ID (LE), 2 bytes quantity (LE)
    // Quantity is encrypted in E/FRLG
    
    size_t baseOffset = gen3Section1Offset + pocket.primaryOffset;
    size_t maxItems = pocket.capacity;
    
    if (maxItems > pocket.capacity) {
        maxItems = pocket.capacity;
    }
    
    pocket.slots.assign(pocket.capacity, BagSlot{});
    
    size_t slotIndex = 0;
    for (size_t i = 0; i < maxItems && slotIndex < pocket.capacity; i++) {
        size_t itemOffset = baseOffset + (i * 4);
        
        if (itemOffset + 4 > fileSize) {
            break;
        }
        
        uint16_t itemId = DataUtils::readU16LE(fileBuffer, itemOffset);
        uint16_t encryptedQty = DataUtils::readU16LE(fileBuffer, itemOffset + 2);
        uint16_t quantity = Generation3Utils::decryptItemQuantity(
            encryptedQty, gen3GameType, gen3SecurityKey);
        
        // Skip empty slots (item ID 0)
        if (itemId == 0) {
            continue;
        }
        
        pocket.slots[slotIndex].itemId = itemId;
        pocket.slots[slotIndex].quantity = quantity;
        slotIndex++;
    }
    
    return true;
}

void PokemonBagEditor::writeGen3PocketToBuffer(const PocketInfo& pocket) {
    size_t baseOffset = gen3Section1Offset + pocket.primaryOffset;
    size_t maxItems = pocket.capacity;
    
    if (maxItems > pocket.capacity) {
        maxItems = pocket.capacity;
    }
    
    // Write all items, including empty slots
    for (size_t i = 0; i < maxItems; i++) {
        size_t itemOffset = baseOffset + (i * 4);
        
        if (itemOffset + 4 > fileSize) {
            break;
        }
        
        uint16_t itemId = 0;
        uint16_t quantity = 0;
        
        if (i < pocket.slots.size()) {
            itemId = pocket.slots[i].itemId;
            quantity = pocket.slots[i].quantity;
        }
        
        uint16_t encryptedQty = Generation3Utils::encryptItemQuantity(
            quantity, gen3GameType, gen3SecurityKey);
        
        DataUtils::writeU16LE(fileBuffer, itemOffset, itemId);
        DataUtils::writeU16LE(fileBuffer, itemOffset + 2, encryptedQty);
    }
}

void PokemonBagEditor::updateChecksumGen3() {
    // Update checksum for Section 1 (where items are stored)
    size_t sectionBase = gen3Section1Offset;
    size_t dataSize = Generation3Utils::GEN3_SECTION_SIZES[1]; // Section 1 data size
    
    uint16_t checksum = Generation3Utils::calculateSectionChecksum(
        fileBuffer, sectionBase, dataSize);
    
    DataUtils::writeU16LE(fileBuffer, 
        sectionBase + Generation3Utils::GEN3_SECTION_CHECKSUM_OFFSET, checksum);
}

// ============================================================================
// Game configuration
// ============================================================================

bool PokemonBagEditor::setGame(const std::string& game) {
    // Normalize game string
    std::string g = game;
    std::transform(g.begin(), g.end(), g.begin(), [](unsigned char c) { 
        return static_cast<char>(std::tolower(c)); 
    });

    gameType = GameType::UNKNOWN;
    gameName.clear();
    
    // Determine game type
    if (g == "red" || g == "blue" || g == "yellow" || g == "green" ||
        g == "pokemon_red" || g == "pokemon_blue" || g == "pokemon_yellow" || 
        g == "pokemon_green" || g == "pokemon_red_blue" || g == "redblue") {
        gameType = GameType::GEN1;
        if (g == "yellow" || g == "pokemon_yellow") {
            gameName = "Pokemon Yellow";
        } else if (g == "green" || g == "pokemon_green") {
            gameName = "Pokemon Green";
        } else {
            gameName = "Pokemon Red/Blue";
        }
    } else if (g == "gold" || g == "pokemon_gold") {
        gameType = GameType::GOLD_SILVER;
        gameName = "Pokemon Gold";
    } else if (g == "silver" || g == "pokemon_silver") {
        gameType = GameType::GOLD_SILVER;
        gameName = "Pokemon Silver";
    } else if (g == "crystal" || g == "pokemon_crystal") {
        gameType = GameType::CRYSTAL;
        gameName = "Pokemon Crystal";
    } else if (g == "ruby" || g == "pokemon_ruby" || 
               g == "sapphire" || g == "pokemon_sapphire" ||
               g == "rubysapphire" || g == "rs") {
        gameType = GameType::RUBY_SAPPHIRE;
        gameName = "Pokemon Ruby/Sapphire";
        gen3GameType = Generation3Utils::GEN3_GAME_RS;
    } else if (g == "emerald" || g == "pokemon_emerald" || g == "e") {
        gameType = GameType::EMERALD;
        gameName = "Pokemon Emerald";
        gen3GameType = Generation3Utils::GEN3_GAME_EMERALD;
    } else if (g == "firered" || g == "pokemon_firered" ||
               g == "leafgreen" || g == "pokemon_leafgreen" ||
               g == "fireredleafgreen" || g == "frlg") {
        gameType = GameType::FIRERED_LEAFGREEN;
        gameName = "Pokemon FireRed/LeafGreen";
        gen3GameType = Generation3Utils::GEN3_GAME_FRLG;
    } else {
        std::cerr << "Unknown game: " << game << std::endl;
        std::cerr << "Supported games: red, blue, yellow, green, gold, silver, crystal, "
                  << "ruby, sapphire, emerald, firered, leafgreen" << std::endl;
        return false;
    }

    // Append Japanese tag if necessary
    if (isJapanese) {
        gameName += " (Japanese)";
    }

    // Build pocket definitions based on game type and language
    pockets.clear();
    currentPocket = 0;
    selectedIndex = 0;
    editing = false;
    editBuffer.clear();

    // For Gen 3, we need to find the current save and parse sections first
    if (isGen3Game()) {
        if (!findGen3CurrentSave()) {
            std::cerr << "Failed to find current save block" << std::endl;
            return false;
        }
        if (!parseGen3Sections()) {
            std::cerr << "Failed to parse save sections" << std::endl;
            return false;
        }
    }

    // For Gen1, there is a single bag
    if (gameType == GameType::GEN1) {
        PocketInfo p;
        p.name = "Items";
        p.capacity = 20;
        p.hasQuantity = true;
        p.idStored = true;
        p.idEditable = true;
        p.quantityEditable = true;
        p.nameEditable = true;
        p.primaryOffset = isJapanese ? 0x25C4 : 0x25C9;
        p.secondaryOffset = 0; // Gen 1 has only one copy
        p.slots.assign(p.capacity, BagSlot{});
        pockets.push_back(p);
    } else if (gameType == GameType::GOLD_SILVER) {
        // Generation 2 Gold/Silver pockets
        if (isJapanese) {
            // Japanese GS
            PocketInfo tm;
            tm.name = "TMs/HMs";
            tm.capacity = 57;
            tm.hasQuantity = true;
            tm.idStored = false;
            tm.idEditable = false;
            tm.quantityEditable = true;
            tm.nameEditable = false;
            tm.primaryOffset = 0x23C7;
            tm.secondaryOffset = 0x75C7;
            tm.slots.assign(tm.capacity, BagSlot{});
            pockets.push_back(tm);

            PocketInfo items;
            items.name = "Items";
            items.capacity = 20;
            items.hasQuantity = true;
            items.idStored = true;
            items.idEditable = true;
            items.quantityEditable = true;
            items.nameEditable = true;
            items.primaryOffset = 0x2400;
            items.secondaryOffset = 0x7600;
            items.slots.assign(items.capacity, BagSlot{});
            pockets.push_back(items);

            PocketInfo keys;
            keys.name = "Key Items";
            keys.capacity = 26;
            keys.hasQuantity = false;
            keys.idStored = true;
            keys.idEditable = true;
            keys.quantityEditable = false;
            keys.nameEditable = true;
            keys.primaryOffset = 0x242A;
            keys.secondaryOffset = 0x762A;
            keys.slots.assign(keys.capacity, BagSlot{});
            pockets.push_back(keys);

            PocketInfo balls;
            balls.name = "Balls";
            balls.capacity = 12;
            balls.hasQuantity = true;
            balls.idStored = true;
            balls.idEditable = true;
            balls.quantityEditable = true;
            balls.nameEditable = true;
            balls.primaryOffset = 0x2445;
            balls.secondaryOffset = 0x7645;
            balls.slots.assign(balls.capacity, BagSlot{});
            pockets.push_back(balls);
        } else {
            // English GS
            PocketInfo tm;
            tm.name = "TMs/HMs";
            tm.capacity = 57;
            tm.hasQuantity = true;
            tm.idStored = false;
            tm.idEditable = false;
            tm.quantityEditable = true;
            tm.nameEditable = false;
            tm.primaryOffset = 0x23E6;
            tm.secondaryOffset = 0x0C78;
            tm.slots.assign(tm.capacity, BagSlot{});
            pockets.push_back(tm);

            PocketInfo items;
            items.name = "Items";
            items.capacity = 20;
            items.hasQuantity = true;
            items.idStored = true;
            items.idEditable = true;
            items.quantityEditable = true;
            items.nameEditable = true;
            items.primaryOffset = 0x241F;
            items.secondaryOffset = 0x0CB1;
            items.slots.assign(items.capacity, BagSlot{});
            pockets.push_back(items);

            PocketInfo keys;
            keys.name = "Key Items";
            keys.capacity = 26;
            keys.hasQuantity = false;
            keys.idStored = true;
            keys.idEditable = true;
            keys.quantityEditable = false;
            keys.nameEditable = true;
            keys.primaryOffset = 0x2449;
            keys.secondaryOffset = 0x0CDB;
            keys.slots.assign(keys.capacity, BagSlot{});
            pockets.push_back(keys);

            PocketInfo balls;
            balls.name = "Balls";
            balls.capacity = 12;
            balls.hasQuantity = true;
            balls.idStored = true;
            balls.idEditable = true;
            balls.quantityEditable = true;
            balls.nameEditable = true;
            balls.primaryOffset = 0x2464;
            balls.secondaryOffset = 0x0CF6;
            balls.slots.assign(balls.capacity, BagSlot{});
            pockets.push_back(balls);
        }
    } else if (gameType == GameType::CRYSTAL) {
        // Generation 2 Crystal pockets
        if (isJapanese) {
            PocketInfo tm;
            tm.name = "TMs/HMs";
            tm.capacity = 57;
            tm.hasQuantity = true;
            tm.idStored = false;
            tm.idEditable = false;
            tm.quantityEditable = true;
            tm.nameEditable = false;
            tm.primaryOffset = 0x23C9;
            tm.secondaryOffset = 0x75C9;
            tm.slots.assign(tm.capacity, BagSlot{});
            pockets.push_back(tm);

            PocketInfo items;
            items.name = "Items";
            items.capacity = 20;
            items.hasQuantity = true;
            items.idStored = true;
            items.idEditable = true;
            items.quantityEditable = true;
            items.nameEditable = true;
            items.primaryOffset = 0x2402;
            items.secondaryOffset = 0x7602;
            items.slots.assign(items.capacity, BagSlot{});
            pockets.push_back(items);

            PocketInfo keys;
            keys.name = "Key Items";
            keys.capacity = 26;
            keys.hasQuantity = false;
            keys.idStored = true;
            keys.idEditable = true;
            keys.quantityEditable = false;
            keys.nameEditable = true;
            keys.primaryOffset = 0x242C;
            keys.secondaryOffset = 0x762C;
            keys.slots.assign(keys.capacity, BagSlot{});
            pockets.push_back(keys);

            PocketInfo balls;
            balls.name = "Balls";
            balls.capacity = 12;
            balls.hasQuantity = true;
            balls.idStored = true;
            balls.idEditable = true;
            balls.quantityEditable = true;
            balls.nameEditable = true;
            balls.primaryOffset = 0x2447;
            balls.secondaryOffset = 0x7647;
            balls.slots.assign(balls.capacity, BagSlot{});
            pockets.push_back(balls);
        } else {
            PocketInfo tm;
            tm.name = "TMs/HMs";
            tm.capacity = 57;
            tm.hasQuantity = true;
            tm.idStored = false;
            tm.idEditable = false;
            tm.quantityEditable = true;
            tm.nameEditable = false;
            tm.primaryOffset = 0x23E7;
            tm.secondaryOffset = 0x15E7;
            tm.slots.assign(tm.capacity, BagSlot{});
            pockets.push_back(tm);

            PocketInfo items;
            items.name = "Items";
            items.capacity = 20;
            items.hasQuantity = true;
            items.idStored = true;
            items.idEditable = true;
            items.quantityEditable = true;
            items.nameEditable = true;
            items.primaryOffset = 0x2420;
            items.secondaryOffset = 0x1620;
            items.slots.assign(items.capacity, BagSlot{});
            pockets.push_back(items);

            PocketInfo keys;
            keys.name = "Key Items";
            keys.capacity = 26;
            keys.hasQuantity = false;
            keys.idStored = true;
            keys.idEditable = true;
            keys.quantityEditable = false;
            keys.nameEditable = true;
            keys.primaryOffset = 0x244A;
            keys.secondaryOffset = 0x164A;
            keys.slots.assign(keys.capacity, BagSlot{});
            pockets.push_back(keys);

            PocketInfo balls;
            balls.name = "Balls";
            balls.capacity = 12;
            balls.hasQuantity = true;
            balls.idStored = true;
            balls.idEditable = true;
            balls.quantityEditable = true;
            balls.nameEditable = true;
            balls.primaryOffset = 0x2465;
            balls.secondaryOffset = 0x1665;
            balls.slots.assign(balls.capacity, BagSlot{});
            pockets.push_back(balls);
        }
    } else if (gameType == GameType::RUBY_SAPPHIRE) {
        // Ruby/Sapphire pocket definitions
        // Offsets are relative to Section 1
        // Items: 20, Key Items: 20, Balls: 16, TM/HM: 64, Berries: 46
        
        PocketInfo items;
        items.name = "Items";
        items.capacity = 20;
        items.hasQuantity = true;
        items.idStored = true;
        items.idEditable = true;
        items.quantityEditable = true;
        items.nameEditable = true;
        items.primaryOffset = 0x0560;
        items.secondaryOffset = 0;
        items.slots.assign(items.capacity, BagSlot{});
        pockets.push_back(items);

        PocketInfo keys;
        keys.name = "Key Items";
        keys.capacity = 20;
        keys.hasQuantity = true;
        keys.idStored = true;
        keys.idEditable = true;
        keys.quantityEditable = true;
        keys.nameEditable = true;
        keys.primaryOffset = 0x05B0;
        keys.secondaryOffset = 0;
        keys.slots.assign(keys.capacity, BagSlot{});
        pockets.push_back(keys);

        PocketInfo balls;
        balls.name = "Poke Balls";
        balls.capacity = 16;
        balls.hasQuantity = true;
        balls.idStored = true;
        balls.idEditable = true;
        balls.quantityEditable = true;
        balls.nameEditable = true;
        balls.primaryOffset = 0x0600;
        balls.secondaryOffset = 0;
        balls.slots.assign(balls.capacity, BagSlot{});
        pockets.push_back(balls);

        PocketInfo tms;
        tms.name = "TMs/HMs";
        tms.capacity = 64;
        tms.hasQuantity = true;
        tms.idStored = true;
        tms.idEditable = true;
        tms.quantityEditable = true;
        tms.nameEditable = true;
        tms.primaryOffset = 0x0640;
        tms.secondaryOffset = 0;
        tms.slots.assign(tms.capacity, BagSlot{});
        pockets.push_back(tms);

        PocketInfo berries;
        berries.name = "Berries";
        berries.capacity = 46;
        berries.hasQuantity = true;
        berries.idStored = true;
        berries.idEditable = true;
        berries.quantityEditable = true;
        berries.nameEditable = true;
        berries.primaryOffset = 0x0740;
        berries.secondaryOffset = 0;
        berries.slots.assign(berries.capacity, BagSlot{});
        pockets.push_back(berries);
    } else if (gameType == GameType::EMERALD) {
        // Emerald pocket definitions
        // Items: 30, Key Items: 30, Balls: 16, TM/HM: 64, Berries: 46
        
        PocketInfo items;
        items.name = "Items";
        items.capacity = 30;
        items.hasQuantity = true;
        items.idStored = true;
        items.idEditable = true;
        items.quantityEditable = true;
        items.nameEditable = true;
        items.primaryOffset = 0x0560;
        items.secondaryOffset = 0;
        items.slots.assign(items.capacity, BagSlot{});
        pockets.push_back(items);

        PocketInfo keys;
        keys.name = "Key Items";
        keys.capacity = 30;
        keys.hasQuantity = true;
        keys.idStored = true;
        keys.idEditable = true;
        keys.quantityEditable = true;
        keys.nameEditable = true;
        keys.primaryOffset = 0x05D8;
        keys.secondaryOffset = 0;
        keys.slots.assign(keys.capacity, BagSlot{});
        pockets.push_back(keys);

        PocketInfo balls;
        balls.name = "Poke Balls";
        balls.capacity = 16;
        balls.hasQuantity = true;
        balls.idStored = true;
        balls.idEditable = true;
        balls.quantityEditable = true;
        balls.nameEditable = true;
        balls.primaryOffset = 0x0650;
        balls.secondaryOffset = 0;
        balls.slots.assign(balls.capacity, BagSlot{});
        pockets.push_back(balls);

        PocketInfo tms;
        tms.name = "TMs/HMs";
        tms.capacity = 64;
        tms.hasQuantity = true;
        tms.idStored = true;
        tms.idEditable = true;
        tms.quantityEditable = true;
        tms.nameEditable = true;
        tms.primaryOffset = 0x0690;
        tms.secondaryOffset = 0;
        tms.slots.assign(tms.capacity, BagSlot{});
        pockets.push_back(tms);

        PocketInfo berries;
        berries.name = "Berries";
        berries.capacity = 46;
        berries.hasQuantity = true;
        berries.idStored = true;
        berries.idEditable = true;
        berries.quantityEditable = true;
        berries.nameEditable = true;
        berries.primaryOffset = 0x0790;
        berries.secondaryOffset = 0;
        berries.slots.assign(berries.capacity, BagSlot{});
        pockets.push_back(berries);
    } else if (gameType == GameType::FIRERED_LEAFGREEN) {
        // FireRed/LeafGreen pocket definitions
        // Items: 42, Key Items: 30, Balls: 13, TM/HM: 58, Berries: 43
        
        PocketInfo items;
        items.name = "Items";
        items.capacity = 42;
        items.hasQuantity = true;
        items.idStored = true;
        items.idEditable = true;
        items.quantityEditable = true;
        items.nameEditable = true;
        items.primaryOffset = 0x0310;
        items.secondaryOffset = 0;
        items.slots.assign(items.capacity, BagSlot{});
        pockets.push_back(items);

        PocketInfo keys;
        keys.name = "Key Items";
        keys.capacity = 30;
        keys.hasQuantity = true;
        keys.idStored = true;
        keys.idEditable = true;
        keys.quantityEditable = true;
        keys.nameEditable = true;
        keys.primaryOffset = 0x03B8;
        keys.secondaryOffset = 0;
        keys.slots.assign(keys.capacity, BagSlot{});
        pockets.push_back(keys);

        PocketInfo balls;
        balls.name = "Poke Balls";
        balls.capacity = 13;
        balls.hasQuantity = true;
        balls.idStored = true;
        balls.idEditable = true;
        balls.quantityEditable = true;
        balls.nameEditable = true;
        balls.primaryOffset = 0x0430;
        balls.secondaryOffset = 0;
        balls.slots.assign(balls.capacity, BagSlot{});
        pockets.push_back(balls);

        PocketInfo tms;
        tms.name = "TMs/HMs";
        tms.capacity = 58;
        tms.hasQuantity = true;
        tms.idStored = true;
        tms.idEditable = true;
        tms.quantityEditable = true;
        tms.nameEditable = true;
        tms.primaryOffset = 0x0464;
        tms.secondaryOffset = 0;
        tms.slots.assign(tms.capacity, BagSlot{});
        pockets.push_back(tms);

        PocketInfo berries;
        berries.name = "Berries";
        berries.capacity = 43;
        berries.hasQuantity = true;
        berries.idStored = true;
        berries.idEditable = true;
        berries.quantityEditable = true;
        berries.nameEditable = true;
        berries.primaryOffset = 0x054C;
        berries.secondaryOffset = 0;
        berries.slots.assign(berries.capacity, BagSlot{});
        pockets.push_back(berries);
    }

    // Now parse pockets from the file
    if (!parseAllPockets()) {
        std::cerr << "Error parsing pockets." << std::endl;
        return false;
    }
    
    // Reset scroll state
    scrollbar.offset = 0;
    hasUnsavedChanges = false;
    setConfirmOnQuit(false);
    return true;
}

// ============================================================================
// Pocket parsing
// ============================================================================

bool PokemonBagEditor::parseAllPockets() {
    for (auto& pocket : pockets) {
        if (!parsePocket(pocket)) {
            return false;
        }
    }
    return true;
}

bool PokemonBagEditor::parsePocket(PocketInfo& pocket) {
    // Use Gen 3 specific parsing for Gen 3 games
    if (isGen3Game()) {
        return parseGen3Pocket(pocket);
    }

    // Ensure offsets are within the file
    if (pocket.primaryOffset >= fileSize) {
        return false;
    }
    
    // Clear slots
    pocket.slots.assign(pocket.capacity, BagSlot{});

    if (!pocket.idStored) {
        // Gen 2 TM/HM pocket handling
        pocket.slots.clear();
        pocket.originalIndices.clear();
        size_t tmMemOffset =  0;
        for (size_t candidate = 0; candidate < 52 && tmMemOffset < 50; candidate++) {
            uint8_t id = static_cast<uint8_t>(0xBF + candidate);
            if (!ItemsIndex::gen2ItemExists(id)) {
                continue;
            }
            size_t ofs = pocket.primaryOffset + tmMemOffset;
            uint8_t qty = 0;
            if (ofs < fileSize) {
                qty = DataUtils::readU8(fileBuffer, ofs);
            }
            pocket.slots.push_back(BagSlot{ id, qty });
            pocket.originalIndices.push_back(tmMemOffset);
            tmMemOffset++;
        }
        // HMs
        size_t hmMemOffset = 0;
        for (size_t candidate = 0; candidate < 7 && hmMemOffset < 7; candidate++) {
            uint8_t id = static_cast<uint8_t>(0xF3 + candidate);
            if (!ItemsIndex::gen2ItemExists(id)) {
                continue;
            }
            size_t ofs = pocket.primaryOffset + 0x32 + hmMemOffset;
            uint8_t qty = 0;
            if (ofs < fileSize) {
                qty = DataUtils::readU8(fileBuffer, ofs);
            }
            pocket.slots.push_back(BagSlot{ id, qty });
            pocket.originalIndices.push_back(0x32 + hmMemOffset);
            hmMemOffset++;
        }
        pocket.capacity = pocket.slots.size();
        return true;
    }

    // List pockets: first byte is count
    uint8_t count = DataUtils::readU8(fileBuffer, pocket.primaryOffset);
    if (count > pocket.capacity) {
        count = static_cast<uint8_t>(pocket.capacity);
    }
    size_t pos = pocket.primaryOffset + 1;
    size_t maxEnd = 0;
    
    if (pocket.hasQuantity) {
        maxEnd = pocket.primaryOffset + 1 + (pocket.capacity * 2) - 1;
    } else {
        maxEnd = pocket.primaryOffset + 1 + pocket.capacity - 1;
    }
    
    int slotIndex = 0;
    while (slotIndex < static_cast<int>(pocket.capacity) && pos <= maxEnd) {
        uint8_t id = DataUtils::readU8(fileBuffer, pos);
        if (id == 0xFF) {
            break;
        }
        if (pocket.hasQuantity) {
            uint8_t qty = 0;
            if (pos + 1 <= maxEnd) {
                qty = DataUtils::readU8(fileBuffer, pos + 1);
            }
            pocket.slots[slotIndex].itemId = id;
            pocket.slots[slotIndex].quantity = qty;
            pos += 2;
        } else {
            pocket.slots[slotIndex].itemId = id;
            pocket.slots[slotIndex].quantity = 1;
            pos += 1;
        }
        slotIndex++;
    }
    
    return true;
}

// ============================================================================
// Writing pockets back to the buffer
// ============================================================================

void PokemonBagEditor::writeAllPocketsToBuffer() {
    for (const auto& pocket : pockets) {
        writePocketToBuffer(pocket);
    }
}

void PokemonBagEditor::writePocketToBuffer(const PocketInfo& pocket) {
    // Use Gen 3 specific writing for Gen 3 games
    if (isGen3Game()) {
        writeGen3PocketToBuffer(pocket);
        return;
    }

    auto writeToOffset = [&](size_t baseOffset) {
        if (baseOffset >= fileSize) return;
        if (!pocket.idStored) {
            size_t count = pocket.originalIndices.size();
            for (size_t i = 0; i < count; i++) {
                size_t origIndex = pocket.originalIndices[i];
                if (i >= pocket.slots.size()) continue;
                size_t ofs = baseOffset + origIndex;
                if (ofs < fileSize) {
                    DataUtils::writeU8(fileBuffer, ofs, 
                        static_cast<uint8_t>(pocket.slots[i].quantity));
                }
            }
            return;
        }
        
        // List pockets
        uint8_t count = 0;
        for (size_t i = 0; i < pocket.capacity; i++) {
            if (pocket.hasQuantity) {
                if (pocket.slots[i].itemId != 0 && pocket.slots[i].quantity != 0) {
                    count++;
                } else {
                    break;
                }
            } else {
                if (pocket.slots[i].itemId != 0) {
                    count++;
                } else {
                    break;
                }
            }
        }
        
        DataUtils::writeU8(fileBuffer, baseOffset, count);
        size_t pos = baseOffset + 1;
        size_t endPos;
        
        if (pocket.hasQuantity) {
            endPos = baseOffset + 1 + (pocket.capacity * 2) - 1;
        } else {
            endPos = baseOffset + 1 + pocket.capacity - 1;
        }
        
        for (size_t i = 0; i < count; i++) {
            if (pocket.hasQuantity) {
                if (pos + 1 <= endPos) {
                    DataUtils::writeU8(fileBuffer, pos, 
                        static_cast<uint8_t>(pocket.slots[i].itemId));
                    DataUtils::writeU8(fileBuffer, pos + 1, 
                        static_cast<uint8_t>(pocket.slots[i].quantity));
                    pos += 2;
                }
            } else {
                if (pos <= endPos) {
                    DataUtils::writeU8(fileBuffer, pos, 
                        static_cast<uint8_t>(pocket.slots[i].itemId));
                    pos += 1;
                }
            }
        }
        
        if (pos <= endPos) {
            DataUtils::writeU8(fileBuffer, pos, 0xFF);
            pos++;
        }
        
        while (pos <= endPos) {
            DataUtils::writeU8(fileBuffer, pos, 0x00);
            pos++;
        }
    };
    
    writeToOffset(pocket.primaryOffset);
    if (pocket.secondaryOffset != 0) {
        writeToOffset(pocket.secondaryOffset);
    }
}

// ============================================================================
// Checksum calculations
// ============================================================================

void PokemonBagEditor::updateChecksum() {
    if (gameType == GameType::GEN1) {
        updateChecksumGen1();
    } else if (gameType == GameType::GOLD_SILVER || gameType == GameType::CRYSTAL) {
        updateChecksumGen2();
    } else if (isGen3Game()) {
        updateChecksumGen3();
    }
}

void PokemonBagEditor::updateChecksumGen1() {
    size_t start = 0x2598;
    size_t end = isJapanese ? 0x3593 : 0x3522;
    size_t checksumPos = isJapanese ? 0x3594 : 0x3523;
    if (end >= fileSize || checksumPos >= fileSize) return;
    
    uint32_t sum = 0;
    for (size_t i = start; i <= end && i < fileSize; i++) {
        sum += static_cast<uint8_t>(fileBuffer[i]);
    }
    uint8_t checksum = static_cast<uint8_t>(~(sum & 0xFF));
    DataUtils::writeU8(fileBuffer, checksumPos, checksum);
}

void PokemonBagEditor::updateChecksumGen2() {
    auto update16BitChecksum = [&](uint32_t start, uint32_t end, uint32_t loc) {
        uint32_t sum = 0;
        for (uint32_t i = start; i <= end && i < fileSize; i++) {
            sum += DataUtils::readU8(fileBuffer, i);
        }
        uint16_t checksum = sum & 0xFFFF;
        if (loc + 1 < fileSize) {
            DataUtils::writeU16LE(fileBuffer, loc, checksum);
        }
    };
    
    auto update16BitChecksumMulti = [&](const std::vector<std::pair<uint32_t,uint32_t>>& ranges,
                                        uint32_t loc) {
        uint32_t sum = 0;
        for (const auto& range : ranges) {
            for (uint32_t i = range.first; i <= range.second && i < fileSize; i++) {
                sum += static_cast<uint8_t>(fileBuffer[i]);
            }
        }
        uint16_t checksum = static_cast<uint16_t>(sum & 0xFFFF);
        if (loc + 1 < fileSize) {
            DataUtils::writeU16LE(fileBuffer, loc, checksum);
        }
    };

    bool crystal = (gameType == GameType::CRYSTAL);
    if (!crystal) {
        if (isJapanese) {
            update16BitChecksum(0x2009, 0x2C8B, 0x2D0D);
            update16BitChecksum(0x7209, 0x7E8B, 0x7F0D);
        } else {
            update16BitChecksum(0x2009, 0x2D68, 0x2D69);
            std::vector<std::pair<uint32_t,uint32_t>> ranges = {
                {0x0C6B, 0x17EC},
                {0x3D96, 0x3F3F},
                {0x7E39, 0x7E6C}
            };
            update16BitChecksumMulti(ranges, 0x7E6D);
        }
    } else {
        if (isJapanese) {
            update16BitChecksum(0x2009, 0x2AE2, 0x2D0D);
            update16BitChecksum(0x7209, 0x7CE2, 0x7F0D);
        } else {
            update16BitChecksum(0x2009, 0x2B82, 0x2D0D);
            update16BitChecksum(0x1209, 0x1D82, 0x1F0D);
        }
    }
}

// ============================================================================
// File helpers
// ============================================================================

bool PokemonBagEditor::fileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

std::string PokemonBagEditor::getOutputPath() {
    std::string baseName = HexUtils::getBaseName(fileName);
    if (overwriteMode) {
        return fileName;
    }
    return std::string("edited_files/") + baseName;
}

bool PokemonBagEditor::saveFile() {
    writeAllPocketsToBuffer();
    updateChecksum();
    
    if (!overwriteMode) {
#ifdef _WIN32
        _mkdir("edited_files");
#else
        mkdir("edited_files", 0755);
#endif
    }
    
    std::string outPath = getOutputPath();
    
    if (fileExists(outPath)) {
        std::string displayName = HexUtils::getBaseName(outPath);
        if (!showOverwriteConfirmDialog(displayName)) {
            std::cout << "Save cancelled." << std::endl;
            return false;
        }
    }
    
    std::ofstream outFile(outPath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Failed to save: " << outPath << std::endl;
        return false;
    }
    outFile.write(fileBuffer.data(), static_cast<std::streamsize>(fileSize));
    outFile.close();
    
    hasUnsavedChanges = false;
    setConfirmOnQuit(false);
    std::cout << "Saved to: " << outPath << std::endl;
    return true;
}

// ============================================================================
// Item manipulation helpers
// ============================================================================

void PokemonBagEditor::removeItem(int index) {
    if (currentPocket < 0 || currentPocket >= static_cast<int>(pockets.size())) return;
    PocketInfo& p = pockets[currentPocket];
    if (index < 0 || index >= static_cast<int>(p.capacity)) return;
    
    if (!p.idStored) {
        if (p.slots[index].quantity != 0) {
            p.slots[index].quantity = 0;
            hasUnsavedChanges = true;
        }
        return;
    }
    
    // For Gen 3 and list pockets: shift items up
    for (int i = index; i < static_cast<int>(p.capacity) - 1; i++) {
        p.slots[i] = p.slots[i + 1];
    }
    p.slots[p.capacity - 1] = BagSlot{};
    hasUnsavedChanges = true;
    
    if (selectedIndex >= static_cast<int>(p.capacity)) {
        selectedIndex = static_cast<int>(p.capacity) - 1;
    }
}

void PokemonBagEditor::startEditing(int index) {
    if (currentPocket < 0 || currentPocket >= static_cast<int>(pockets.size())) return;
    PocketInfo& p = pockets[currentPocket];
    if (index < 0 || index >= static_cast<int>(p.capacity)) return;
    
    selectedIndex = index;
    editing = true;
    editBuffer.clear();
    
    if (p.idEditable) {
        editingField = 0;
    } else if (p.quantityEditable) {
        editingField = 1;
    } else if (p.nameEditable) {
        editingField = 2;
    } else {
        editing = false;
    }
    
    size_t vis = scrollbar.visibleItems;
    if (vis == 0) {
        int rowH = charHeight + 8;
        int headerH = charHeight * 2 + 10;
        int instructionsH = charHeight * 2 + 10;
        int availableH = windowHeight - headerH - instructionsH - 10;
        vis = (availableH > 0) ? (static_cast<size_t>(availableH) / rowH) : 1;
        if (vis == 0) vis = 1;
    }
    
    if (static_cast<int>(scrollbar.offset) > selectedIndex) {
        scrollbar.offset = static_cast<size_t>(selectedIndex);
    } else if (selectedIndex >= static_cast<int>(scrollbar.offset + vis)) {
        if (vis > 0) {
            scrollbar.offset = static_cast<size_t>(selectedIndex) - vis + 1;
        }
    }
    
    requestRedraw();
}

void PokemonBagEditor::shiftCurrentPocket() {
    if (currentPocket < 0 || currentPocket >= static_cast<int>(pockets.size())) return;
    PocketInfo& p = pockets[currentPocket];
    
    if (!p.idStored) return;
    
    std::vector<BagSlot> newList;
    newList.reserve(p.capacity);
    
    for (const auto& slot : p.slots) {
        if (p.hasQuantity) {
            if (slot.itemId != 0 && slot.quantity != 0) {
                newList.push_back(slot);
            }
        } else {
            if (slot.itemId != 0) {
                newList.push_back(slot);
            }
        }
    }
    
    while (newList.size() < p.capacity) {
        newList.push_back(BagSlot{});
    }
    
    p.slots = newList;
}

bool PokemonBagEditor::itemExistsForPocket(uint16_t id, const PocketInfo& pocket) const {
    if (gameType == GameType::GEN1) {
        return ItemsIndex::gen1ItemExists(static_cast<uint8_t>(id));
    }
    
    if (isGen3Game()) {
        // Check if item exists in Gen 3 items list
        if (!ItemsIndex::gen3ItemExists(id)) {
            return false;
        }
        
        // Check pocket compatibility
        uint8_t itemPocket = ItemsIndex::getGen3ItemPocket(id);
        
        // Check game availability
        uint8_t gameFlag = 0;
        if (gameType == GameType::RUBY_SAPPHIRE) {
            gameFlag = ItemsIndex::GEN3_GAME_RS;
        } else if (gameType == GameType::EMERALD) {
            gameFlag = ItemsIndex::GEN3_GAME_E;
        } else if (gameType == GameType::FIRERED_LEAFGREEN) {
            gameFlag = ItemsIndex::GEN3_GAME_FRLG;
        }
        
        if (!ItemsIndex::gen3ItemAvailableIn(id, gameFlag)) {
            return false;
        }
        
        // Match pocket
        if (pocket.name == "Items" && itemPocket == ItemsIndex::GEN3_POCKET_ITEMS) return true;
        if (pocket.name == "Poke Balls" && itemPocket == ItemsIndex::GEN3_POCKET_BALLS) return true;
        if (pocket.name == "Key Items" && itemPocket == ItemsIndex::GEN3_POCKET_KEY_ITEMS) return true;
        if (pocket.name == "TMs/HMs" && itemPocket == ItemsIndex::GEN3_POCKET_TM_HM) return true;
        if (pocket.name == "Berries" && itemPocket == ItemsIndex::GEN3_POCKET_BERRIES) return true;
        
        return false;
    }
    
    // Gen 2
    const ItemsIndex::Gen2ItemInfo* info = ItemsIndex::getGen2ItemInfo(static_cast<uint8_t>(id));
    if (!info) return false;
    
    if (pocket.name == "Items" && info->pocket == ItemsIndex::POCKET_ITEMS) return true;
    if (pocket.name == "Balls" && info->pocket == ItemsIndex::POCKET_BALLS) return true;
    if (pocket.name == "Key Items" && info->pocket == ItemsIndex::POCKET_KEY_ITEMS) return true;
    
    return false;
}

const char* PokemonBagEditor::getItemName(uint16_t id) const {
    if (gameType == GameType::GEN1) {
        return ItemsIndex::getGen1ItemName(static_cast<uint8_t>(id));
    }
    
    if (isGen3Game()) {
        return ItemsIndex::getGen3ItemName(id);
    }
    
    // Gen 2
    if (!ItemsIndex::gen2ItemExists(static_cast<uint8_t>(id))) {
        return nullptr;
    }
    bool isCrystal = (gameType == GameType::CRYSTAL);
    return ItemsIndex::getGen2ItemName(static_cast<uint8_t>(id), isCrystal);
}

uint16_t PokemonBagEditor::lookupItemIdByName(const std::string& name, const PocketInfo& pocket) const {
    std::string query = name;
    std::transform(query.begin(), query.end(), query.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    
    if (gameType == GameType::GEN1) {
        for (const auto& kv : ItemsIndex::GEN1_ITEMS) {
            const char* nm = kv.second;
            if (!nm) continue;
            std::string nameStr(nm);
            std::transform(nameStr.begin(), nameStr.end(), nameStr.begin(), [](unsigned char c) {
                return static_cast<char>(std::toupper(c));
            });
            if (nameStr == query) {
                return kv.first;
            }
        }
        return 0;
    }
    
    if (isGen3Game()) {
        // Search Gen 3 items
        for (const auto& kv : ItemsIndex::GEN3_ITEMS) {
            const char* nm = kv.second.name;
            if (!nm) continue;
            
            std::string nameStr(nm);
            std::transform(nameStr.begin(), nameStr.end(), nameStr.begin(), [](unsigned char c) {
                return static_cast<char>(std::toupper(c));
            });
            
            if (nameStr == query) {
                // Verify it's valid for this pocket and game
                if (itemExistsForPocket(kv.first, pocket)) {
                    return kv.first;
                }
            }
        }
        return 0;
    }
    
    // Gen 2
    bool isCrystal = (gameType == GameType::CRYSTAL);
    for (const auto& kv : ItemsIndex::GEN2_ITEMS) {
        const ItemsIndex::Gen2ItemInfo& info = kv.second;
        const char* nm = nullptr;
        if (isCrystal && info.nameCrystal != nullptr) {
            nm = info.nameCrystal;
        } else {
            nm = info.nameGS;
        }
        if (!nm) continue;
        
        std::string nameStr(nm);
        std::transform(nameStr.begin(), nameStr.end(), nameStr.begin(), [](unsigned char c) {
            return static_cast<char>(std::toupper(c));
        });
        
        if (nameStr == query) {
            if (pocket.name == "Items" && info.pocket == ItemsIndex::POCKET_ITEMS) return kv.first;
            if (pocket.name == "Balls" && info.pocket == ItemsIndex::POCKET_BALLS) return kv.first;
            if (pocket.name == "Key Items" && info.pocket == ItemsIndex::POCKET_KEY_ITEMS) return kv.first;
        }
    }
    
    return 0;
}

// ============================================================================
// Editing input handler
// ============================================================================

void PokemonBagEditor::handleEditInput(SDL_Keycode key) {
    if (!editing) return;
    PocketInfo& pocket = pockets[currentPocket];
    
    if (key == SDLK_ESCAPE) {
        editing = false;
        editBuffer.clear();
        requestRedraw();
        return;
    }
    
    // Name editing (field 2)
    if (editingField == 2) {
        if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
            if (!editBuffer.empty()) {
                uint16_t foundId = lookupItemIdByName(editBuffer, pocket);
                if (foundId != 0) {
                    pocket.slots[selectedIndex].itemId = foundId;
                    if (pocket.hasQuantity) {
                        if (pocket.slots[selectedIndex].quantity == 0) {
                            pocket.slots[selectedIndex].quantity = 1;
                        }
                    } else {
                        pocket.slots[selectedIndex].quantity = 1;
                    }
                    hasUnsavedChanges = true;
                    
                    if (pocket.quantityEditable) {
                        editingField = 1;
                        editBuffer.clear();
                    } else {
                        editing = false;
                        editingField = 0;
                        editBuffer.clear();
                    }
                    shiftCurrentPocket();
                }
            }
            requestRedraw();
            return;
        }
        
        if (key == SDLK_BACKSPACE) {
            if (!editBuffer.empty()) {
                editBuffer.pop_back();
                requestRedraw();
            }
            return;
        }
        
        char c = 0;
        if (key >= SDLK_A && key <= SDLK_Z) {
            c = static_cast<char>('A' + (key - SDLK_A));
        } else if (key >= SDLK_0 && key <= SDLK_9) {
            c = static_cast<char>('0' + (key - SDLK_0));
        } else if (key == SDLK_SPACE) {
            c = ' ';
        } else if (key == SDLK_MINUS) {
            c = '-';
        } else if (key == SDLK_PERIOD) {
            c = '.';
        } else if (key == SDLK_APOSTROPHE) {
            c = '\'';
        } else {
            return;
        }
        editBuffer.push_back(c);
        requestRedraw();
        return;
    }
    
    if (key == SDLK_BACKSPACE) {
        if (!editBuffer.empty()) {
            editBuffer.pop_back();
            requestRedraw();
        }
        return;
    }
    
    char c = 0;
    if (editingField == 0) {
        // Item ID (hex) - 4 digits for Gen 3, 2 for Gen 1/2
        if (key >= SDLK_0 && key <= SDLK_9) {
            c = static_cast<char>('0' + (key - SDLK_0));
        } else if (key >= SDLK_A && key <= SDLK_F) {
            c = static_cast<char>('A' + (key - SDLK_A));
        } else {
            return;
        }
    } else if (editingField == 1) {
        // Quantity (decimal) - up to 5 digits for Gen 3
        if (key >= SDLK_0 && key <= SDLK_9) {
            c = static_cast<char>('0' + (key - SDLK_0));
        } else {
            return;
        }
    }
    
    editBuffer.push_back(c);
    requestRedraw();
    
    // Commit based on generation
    if (editingField == 0) {
        size_t expectedDigits = isGen3Game() ? 4 : 2;
        if (editBuffer.length() == expectedDigits) {
            uint16_t value = 0;
            try {
                value = static_cast<uint16_t>(std::stoul(editBuffer, nullptr, 16));
            } catch (...) {
                editBuffer.clear();
                return;
            }
            
            if (!itemExistsForPocket(value, pocket)) {
                editBuffer.clear();
                return;
            }
            
            pocket.slots[selectedIndex].itemId = value;
            if (pocket.hasQuantity) {
                if (pocket.slots[selectedIndex].quantity == 0) {
                    pocket.slots[selectedIndex].quantity = 1;
                }
            } else {
                pocket.slots[selectedIndex].quantity = 1;
            }
            hasUnsavedChanges = true;
            
            if (pocket.quantityEditable) {
                editingField = 1;
                editBuffer.clear();
            } else {
                editing = false;
                editBuffer.clear();
            }
            shiftCurrentPocket();
        }
    } else if (editingField == 1) {
        // Gen 3: up to 999 quantity (3 digits), Gen 1/2: 99 (2 digits)
        size_t maxDigits = isGen3Game() ? 3 : 2;
        int maxQty = isGen3Game() ? 999 : 99;
        
        if (editBuffer.length() == maxDigits) {
            int qty = 0;
            try {
                qty = std::stoi(editBuffer);
            } catch (...) {
                editBuffer.clear();
                return;
            }
            
            if (qty < 0) qty = 0;
            if (qty > maxQty) qty = maxQty;
            
            if (pocket.idStored) {
                if (pocket.hasQuantity) {
                    if (qty == 0) {
                        removeItem(selectedIndex);
                    } else {
                        pocket.slots[selectedIndex].quantity = static_cast<uint16_t>(qty);
                        hasUnsavedChanges = true;
                    }
                }
            } else {
                pocket.slots[selectedIndex].quantity = static_cast<uint16_t>(qty);
                hasUnsavedChanges = true;
            }
            
            editing = false;
            editBuffer.clear();
            if (pocket.idStored) {
                shiftCurrentPocket();
            }
        }
    }
}

// ============================================================================
// Rendering
// ============================================================================

void PokemonBagEditor::render() {
    SDL_SetRenderDrawColor(renderer, colors.background.r, colors.background.g, 
                           colors.background.b, 255);
    SDL_RenderClear(renderer);

    int headerH = charHeight * 2 + 10;
    SDL_Rect headerRect = {0, 0, windowWidth, headerH};
    renderFilledRect(headerRect, colors.headerBg);

    std::stringstream ss;
    ss << HexUtils::getBaseName(fileName);
    if (!gameName.empty()) {
        ss << " - " << gameName;
    }
    if (overwriteMode) {
        ss << " [OVERWRITE]";
    }
    if (hasUnsavedChanges) {
        ss << " [MODIFIED]";
    }
    
    SDL_Color headerColor = colors.text;
    if (overwriteMode) {
        headerColor = colors.warning;
    } else if (hasUnsavedChanges) {
        headerColor = colors.error;
    }
    renderText(ss.str(), 10, 5, headerColor);

    std::stringstream ps;
    ps << "Pocket: ";
    for (size_t i = 0; i < pockets.size(); i++) {
        if (i > 0) ps << " | ";
        if (static_cast<int>(i) == currentPocket) {
            ps << '[' << pockets[i].name << ']';
        } else {
            ps << pockets[i].name;
        }
    }
    renderText(ps.str(), 10, 5 + charHeight, colors.text);

    int rightX = windowWidth - 10;
    int btnW = 80;
    int btnH = charHeight + 6;
    saveButtonRect = {rightX - btnW, 10, btnW, btnH};
    
    if (saveButtonHovered) {
        SDL_Rect hoverRect = {saveButtonRect.x - 1, saveButtonRect.y - 1,
                              saveButtonRect.w + 2, saveButtonRect.h + 2};
        renderFilledRect(hoverRect, {80, 80, 80, 255});
        renderButton(saveButtonRect, "Save");
        renderOutlineRect(hoverRect, colors.accent);
    } else {
        renderButton(saveButtonRect, "Save");
    }

    renderLine(0, headerH - 1, windowWidth, headerH - 1, {60, 60, 60, 255});

    PocketInfo& pocket = pockets[currentPocket];
    int startY = headerH + 5;
    int rowH = charHeight + 8;
    int instructionsH = charHeight * 2 + 10;
    int availableH = windowHeight - headerH - instructionsH - 10;
    size_t visibleRows = (availableH > 0) ? (static_cast<size_t>(availableH) / rowH) : 1;
    if (visibleRows == 0) visibleRows = 1;
    
    scrollbar.headerOffset = headerH;
    scrollbar.visibleItems = visibleRows;
    scrollbar.totalItems = pocket.capacity;
    
    if (scrollbar.offset > scrollbar.maxOffset()) {
        scrollbar.offset = scrollbar.maxOffset();
    }
    
    int rowWidth = windowWidth - 20;
    if (scrollbar.canScroll()) {
        rowWidth -= scrollbar.width;
    }
    
    size_t startIndex = scrollbar.offset;
    size_t endIndex = std::min(startIndex + scrollbar.visibleItems, 
                               static_cast<size_t>(pocket.capacity));
    
    for (size_t idx = startIndex; idx < endIndex; idx++) {
        size_t local = idx - startIndex;
        int y = startY + static_cast<int>(local) * rowH;
        SDL_Rect rowRect = {10, y, rowWidth, rowH - 2};
        
        if (static_cast<int>(idx) == selectedIndex) {
            renderFilledRect(rowRect, colors.selectedBg);
        }
        
        std::string line;
        line += std::to_string(idx + 1);
        line += ": ";
        
        const BagSlot& slot = pocket.slots[idx];
        bool empty = false;
        
        if (!pocket.idStored) {
            // TM/HM for Gen 2
            const char* name = getItemName(slot.itemId);
            line += name ? name : "Unknown";
            line += " x ";
            line += std::to_string(slot.quantity);
        } else if (pocket.hasQuantity) {
            if (slot.itemId != 0 && slot.quantity != 0) {
                const char* name = getItemName(slot.itemId);
                line += (name ? name : "Unknown");
                line += " [";
                // Gen 3 uses 4-digit hex, Gen 1/2 uses 2-digit
                if (isGen3Game()) {
                    line += HexUtils::toHexString(slot.itemId, 4);
                } else {
                    line += HexUtils::toHexString(static_cast<uint8_t>(slot.itemId), 2);
                }
                line += "] x ";
                line += std::to_string(slot.quantity);
            } else {
                empty = true;
            }
        } else {
            if (slot.itemId != 0) {
                const char* name = getItemName(slot.itemId);
                line += (name ? name : "Unknown");
                line += " [";
                if (isGen3Game()) {
                    line += HexUtils::toHexString(slot.itemId, 4);
                } else {
                    line += HexUtils::toHexString(static_cast<uint8_t>(slot.itemId), 2);
                }
                line += "]";
            } else {
                empty = true;
            }
        }
        
        if (empty) {
            line += "(empty)";
        }
        
        if (editing && static_cast<int>(idx) == selectedIndex) {
            if (editingField == 0) {
                line += "  ID: ";
                line += editBuffer;
                line += "_";
            } else if (editingField == 1) {
                line += "  Qty: ";
                line += editBuffer;
                line += "_";
            } else if (editingField == 2) {
                line += "  Name: ";
                line += editBuffer;
                line += "_";
            }
        }
        
        SDL_Color textColor = empty ? colors.textDim : colors.text;
        renderText(line, rowRect.x + 5, y + 2, textColor);
    }
    
    if (scrollbar.canScroll()) {
        renderScrollbar();
    }
    
    int instrY = startY + static_cast<int>(visibleRows) * rowH + 5;
    std::stringstream inst1;
    inst1 << "Up/Down: Select  Left/Right: Pocket  Enter: Edit  ";
    if (pocket.idStored) {
        inst1 << "Del: Remove  ";
        if (pocket.idEditable && pocket.quantityEditable) {
            inst1 << "A: Add  ";
        }
    }
    if (pocket.nameEditable) {
        inst1 << "I: Name  ";
    }
    renderText(inst1.str(), 10, instrY, colors.textDim);
    renderText("Ctrl/Cmd+S: Save", 10, instrY + charHeight, colors.textDim);
    
    SDL_RenderPresent(renderer);
}

// ============================================================================
// Event handling
// ============================================================================

void PokemonBagEditor::handleEvent(SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_MOUSE_MOTION: {
            int mx = static_cast<int>(event.motion.x);
            int my = static_cast<int>(event.motion.y);
            
            if (scrollbar.dragging) {
                handleScrollbarDrag(my);
                requestRedraw();
                break;
            }
            
            bool hover = (mx >= saveButtonRect.x && mx < saveButtonRect.x + saveButtonRect.w &&
                          my >= saveButtonRect.y && my < saveButtonRect.y + saveButtonRect.h);
            if (hover != saveButtonHovered) {
                saveButtonHovered = hover;
                requestRedraw();
            }
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            if (event.button.button == SDL_BUTTON_LEFT) {
                int mx = static_cast<int>(event.button.x);
                int my = static_cast<int>(event.button.y);
                
                if (mx >= saveButtonRect.x && mx < saveButtonRect.x + saveButtonRect.w &&
                    my >= saveButtonRect.y && my < saveButtonRect.y + saveButtonRect.h) {
                    saveFile();
                    break;
                }
                
                if (handleScrollbarClick(mx, my)) {
                    break;
                }
                
                int headerH = charHeight * 2 + 10;
                int rowH = charHeight + 8;
                int startY = headerH + 5;
                int instructionsH = charHeight * 2 + 10;
                int availableH = windowHeight - headerH - instructionsH - 10;
                size_t visibleRows = (availableH > 0) ? 
                    (static_cast<size_t>(availableH) / rowH) : 1;
                if (visibleRows == 0) visibleRows = 1;
                
                PocketInfo& pocket = pockets[currentPocket];
                if (my >= startY) {
                    int local = (my - startY) / rowH;
                    if (local >= 0 && static_cast<size_t>(local) < visibleRows) {
                        int idx = static_cast<int>(scrollbar.offset) + local;
                        if (idx >= 0 && idx < static_cast<int>(pocket.capacity)) {
                            selectedIndex = idx;
                            requestRedraw();
                        }
                    }
                }
            }
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            if (event.button.button == SDL_BUTTON_LEFT) {
                handleScrollbarRelease();
            }
            break;
        }
        case SDL_EVENT_MOUSE_WHEEL: {
            if (!editing && scrollbar.canScroll()) {
                int direction = event.wheel.y;
                scrollBy(-direction);
                requestRedraw();
            }
            break;
        }
        case SDL_EVENT_KEY_DOWN: {
            SDL_Keycode key = event.key.key;
            
            if (!editing && ((event.key.mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI)) && 
                key == SDLK_S)) {
                saveFile();
                break;
            }
            
            if (editing) {
                handleEditInput(key);
            } else {
                PocketInfo& pocket = pockets[currentPocket];
                
                if (key == SDLK_UP) {
                    selectedIndex--;
                    if (selectedIndex < 0) 
                        selectedIndex = static_cast<int>(pocket.capacity) - 1;
                    
                    if (selectedIndex < static_cast<int>(scrollbar.offset)) {
                        scrollbar.offset = static_cast<size_t>(selectedIndex);
                    } else if (selectedIndex >= 
                               static_cast<int>(scrollbar.offset + scrollbar.visibleItems)) {
                        if (scrollbar.visibleItems > 0) {
                            scrollbar.offset = static_cast<size_t>(selectedIndex) - 
                                scrollbar.visibleItems + 1;
                        }
                    }
                    requestRedraw();
                } else if (key == SDLK_DOWN) {
                    selectedIndex++;
                    if (selectedIndex >= static_cast<int>(pocket.capacity)) 
                        selectedIndex = 0;
                    
                    if (selectedIndex < static_cast<int>(scrollbar.offset)) {
                        scrollbar.offset = static_cast<size_t>(selectedIndex);
                    } else if (selectedIndex >= 
                               static_cast<int>(scrollbar.offset + scrollbar.visibleItems)) {
                        if (scrollbar.visibleItems > 0) {
                            scrollbar.offset = static_cast<size_t>(selectedIndex) - 
                                scrollbar.visibleItems + 1;
                        }
                    }
                    requestRedraw();
                } else if (key == SDLK_LEFT) {
                    currentPocket--;
                    if (currentPocket < 0) 
                        currentPocket = static_cast<int>(pockets.size()) - 1;
                    scrollbar.offset = 0;
                    if (selectedIndex >= static_cast<int>(pockets[currentPocket].capacity)) {
                        selectedIndex = static_cast<int>(pockets[currentPocket].capacity) - 1;
                    }
                    requestRedraw();
                } else if (key == SDLK_RIGHT) {
                    currentPocket++;
                    if (currentPocket >= static_cast<int>(pockets.size())) 
                        currentPocket = 0;
                    scrollbar.offset = 0;
                    if (selectedIndex >= static_cast<int>(pockets[currentPocket].capacity)) {
                        selectedIndex = static_cast<int>(pockets[currentPocket].capacity) - 1;
                    }
                    requestRedraw();
                } else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                    startEditing(selectedIndex);
                } else if (key == SDLK_DELETE || key == SDLK_D) {
                    removeItem(selectedIndex);
                    requestRedraw();
                } else if (key == SDLK_A) {
                    if (pocket.idStored) {
                        for (int i = 0; i < static_cast<int>(pocket.capacity); i++) {
                            if (pocket.hasQuantity) {
                                if (pocket.slots[i].itemId == 0 || 
                                    pocket.slots[i].quantity == 0) {
                                    startEditing(i);
                                    break;
                                }
                            } else {
                                if (pocket.slots[i].itemId == 0) {
                                    startEditing(i);
                                    break;
                                }
                            }
                        }
                    }
                } else if (key == SDLK_I) {
                    if (pocket.nameEditable) {
                        startEditing(selectedIndex);
                        editingField = 2;
                        editBuffer.clear();
                    }
                } else if (key == SDLK_Q || key == SDLK_ESCAPE) {
                    if (hasUnsavedChanges) {
                        if (showQuitConfirmDialog()) {
                            quit();
                        }
                    } else {
                        quit();
                    }
                }
            }
            break;
        }
        default:
            break;
    }
}

// ============================================================================
// Update loop
// ============================================================================

void PokemonBagEditor::update(float deltaTime) {
    setConfirmOnQuit(hasUnsavedChanges);
    SDLAppBase::update(deltaTime);
}