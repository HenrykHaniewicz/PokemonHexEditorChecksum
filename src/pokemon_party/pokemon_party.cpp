#include "pokemon_party.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <utility>

#ifndef _WIN32
#include <sys/stat.h>
#else
#include <direct.h>
#endif

// ============================================================================
// Constructor
// ============================================================================

PokemonPartyEditor::PokemonPartyEditor()
    : SDLAppBase("Pokemon Party Editor", 800, 640) {
}

// ============================================================================
// File loading
// ============================================================================

bool PokemonPartyEditor::loadFile(const char* filename) {
    if (!HexUtils::loadFileToBuffer(filename, fileBuffer, fileSize)) {
        std::cerr << "Failed to open: " << filename << std::endl;
        return false;
    }
    fileName = filename;
    hasUnsavedChanges = false;
    return true;
}

// ============================================================================
// Game configuration
// ============================================================================

bool PokemonPartyEditor::setGame(const std::string& game) {
    // Normalize game string
    std::string g = game;
    std::transform(g.begin(), g.end(), g.begin(), [](unsigned char c) { 
        return static_cast<char>(std::tolower(c)); 
    });

    gameType = GameType::UNKNOWN;
    gameName.clear();
    generation = 0;
    
    // Determine game type
    if (g == "red" || g == "blue" || g == "yellow" || g == "green" ||
        g == "pokemon_red" || g == "pokemon_blue" || g == "pokemon_yellow" || 
        g == "pokemon_green") {
        gameType = GameType::GEN1;
        generation = 1;
        if (g == "yellow" || g == "pokemon_yellow") {
            gameName = "Pokemon Yellow";
        } else if (g == "green" || g == "pokemon_green") {
            gameName = "Pokemon Green";
        } else {
            gameName = "Pokemon Red/Blue";
        }
    } else if (g == "gold" || g == "silver" || g == "pokemon_gold" || g == "pokemon_silver") {
        gameType = GameType::GEN2_GS;
        generation = 2;
        gameName = "Pokemon Gold/Silver";
    } else if (g == "crystal" || g == "pokemon_crystal") {
        gameType = GameType::GEN2_CRYSTAL;
        generation = 2;
        gameName = "Pokemon Crystal";
    } else {
        std::cerr << "Unsupported game." << std::endl;
        std::cerr << "Supported games:" << std::endl;
        std::cerr << "  Gen 1: red, blue, yellow, green" << std::endl;
        std::cerr << "  Gen 2: gold, silver, crystal" << std::endl;
        return false;
    }

    // Set the encoding based on game and language
    setEncodingForGame();

    // Append Japanese tag if necessary
    if (isJapanese) {
        gameName += " (Japanese)";
    }

    // Parse the party data
    parsePokemonData();
    
    return true;
}

void PokemonPartyEditor::setEncodingForGame() {
    switch (gameType) {
        case GameType::GEN1:
            encoding = isJapanese ? TextEncoding::JP_G1 : TextEncoding::EN_G1;
            break;
        case GameType::GEN2_GS:
        case GameType::GEN2_CRYSTAL:
            encoding = isJapanese ? TextEncoding::JP_G2 : TextEncoding::EN_G2;
            break;
        case GameType::GEN3_RS:
        case GameType::GEN3_EMERALD:
        case GameType::GEN3_FRLG:
            encoding = isJapanese ? TextEncoding::JP_G3 : TextEncoding::EN_G3;
            break;
        default:
            encoding = TextEncoding::ASCII;
            break;
    }
}

size_t PokemonPartyEditor::getPartyOffset() const {
    switch (gameType) {
        case GameType::GEN1:
            return isJapanese ? GEN1_PARTY_OFFSET_JPN : GEN1_PARTY_OFFSET_ENG;
        case GameType::GEN2_GS:
            return isJapanese ? GEN2_GS_PARTY_OFFSET_JPN : GEN2_GS_PARTY_OFFSET_ENG;
        case GameType::GEN2_CRYSTAL:
            return isJapanese ? GEN2_CRYSTAL_PARTY_OFFSET_JPN : GEN2_CRYSTAL_PARTY_OFFSET_ENG;
        default:
            return 0;
    }
}

size_t PokemonPartyEditor::getPokemonDataSize() const {
    return (generation == 1) ? GEN1_POKEMON_DATA_SIZE : GEN2_POKEMON_DATA_SIZE;
}

// ============================================================================
// Party data parsing
// ============================================================================

void PokemonPartyEditor::parsePokemonData() {
    size_t partyOffset = getPartyOffset();
    size_t nameLength = getNameLength();
    size_t pokemonDataSize = getPokemonDataSize();
    
    // Read party count
    partyCount = DataUtils::readU8(fileBuffer, partyOffset);
    if (partyCount > MAX_PARTY_SIZE) {
        partyCount = MAX_PARTY_SIZE;
    }
    
    // Read species list (capacity + 1 for terminator)
    for (size_t i = 0; i < 7; i++) {
        partySpecies[i] = DataUtils::readU8(fileBuffer, partyOffset + 1 + i);
    }
    
    // Calculate pokemon data offset
    size_t pokemonDataOffset = partyOffset + 8; // 1 byte count + 7 bytes species
    
    // Read each Pokemon's data
    for (size_t i = 0; i < MAX_PARTY_SIZE; i++) {
        PokemonData& pkmn = partyPokemon[i];
        
        if (i >= partyCount || partySpecies[i] == 0 || partySpecies[i] == 0xFF) {
            pkmn = PokemonData(); // Clear data
            continue;
        }
        
        size_t offset = pokemonDataOffset + (i * pokemonDataSize);
        
        if (generation ==1) {
            parseGen1Pokemon(pkmn, offset);
        } else if (generation == 2) {
            parseGen2Pokemon(pkmn, offset);
        }
    }
    
    // Calculate name offsets
    size_t otNamesOffset, nicknamesOffset;
    
    if (generation == 1) {
        if (isJapanese) {
            otNamesOffset = partyOffset + 0x110;
            nicknamesOffset = partyOffset + 0x134;
        } else {
            otNamesOffset = partyOffset + 0x110;
            nicknamesOffset = partyOffset + 0x152;
        }
    } else { // Gen 2
        // Gen 2: Pokemon data, then OT names, then nicknames
        size_t totalPokemonData = MAX_PARTY_SIZE * pokemonDataSize;
        otNamesOffset = pokemonDataOffset + totalPokemonData;
        nicknamesOffset = otNamesOffset + (MAX_PARTY_SIZE * nameLength);
    }
    
    // Read OT names
    for (size_t i = 0; i < MAX_PARTY_SIZE; i++) {
        if (i >= partyCount || partyPokemon[i].isEmpty()) {
            partyPokemon[i].otName.clear();
            continue;
        }
        
        size_t offset = otNamesOffset + (i * nameLength);
        std::vector<uint8_t> nameBytes(nameLength);
        for (size_t j = 0; j < nameLength; j++) {
            nameBytes[j] = DataUtils::readU8(fileBuffer, offset + j);
        }
        partyPokemon[i].otName = decodeText(nameBytes, encoding);
    }
    
    // Read nicknames
    for (size_t i = 0; i < MAX_PARTY_SIZE; i++) {
        if (i >= partyCount || partyPokemon[i].isEmpty()) {
            partyPokemon[i].nickname.clear();
            continue;
        }
        
        size_t offset = nicknamesOffset + (i * nameLength);
        std::vector<uint8_t> nameBytes(nameLength);
        for (size_t j = 0; j < nameLength; j++) {
            nameBytes[j] = DataUtils::readU8(fileBuffer, offset + j);
        }
        partyPokemon[i].nickname = decodeText(nameBytes, encoding);
    }
}

void PokemonPartyEditor::parseGen1Pokemon(PokemonData& pkmn, size_t offset) {
    pkmn.species = DataUtils::readU8(fileBuffer, offset + 0x00);
    pkmn.currentHP = DataUtils::readU16BE(fileBuffer, offset + 0x01);
    pkmn.levelBox = DataUtils::readU8(fileBuffer, offset + 0x03);
    pkmn.status = DataUtils::readU8(fileBuffer, offset + 0x04);
    pkmn.type1 = DataUtils::readU8(fileBuffer, offset + 0x05);
    pkmn.type2 = DataUtils::readU8(fileBuffer, offset + 0x06);
    pkmn.catchRate = DataUtils::readU8(fileBuffer, offset + 0x07);
    
    for (size_t j = 0; j < 4; j++) {
        pkmn.moves[j] = DataUtils::readU8(fileBuffer, offset + 0x08 + j);
    }
    
    pkmn.trainerID = DataUtils::readU16BE(fileBuffer, offset + 0x0C);
    
    // Experience is 3 bytes, big-endian
    pkmn.exp = (static_cast<uint32_t>(DataUtils::readU8(fileBuffer, offset + 0x0E)) << 16) |
               (static_cast<uint32_t>(DataUtils::readU8(fileBuffer, offset + 0x0F)) << 8) |
               static_cast<uint32_t>(DataUtils::readU8(fileBuffer, offset + 0x10));
    
    pkmn.hpEV = DataUtils::readU16BE(fileBuffer, offset + 0x11);
    pkmn.attackEV = DataUtils::readU16BE(fileBuffer, offset + 0x13);
    pkmn.defenseEV = DataUtils::readU16BE(fileBuffer, offset + 0x15);
    pkmn.speedEV = DataUtils::readU16BE(fileBuffer, offset + 0x17);
    pkmn.specialEV = DataUtils::readU16BE(fileBuffer, offset + 0x19);
    pkmn.ivData = DataUtils::readU16BE(fileBuffer, offset + 0x1B);
    
    for (size_t j = 0; j < 4; j++) {
        pkmn.ppValues[j] = DataUtils::readU8(fileBuffer, offset + 0x1D + j);
    }
    
    pkmn.level = DataUtils::readU8(fileBuffer, offset + 0x21);
    pkmn.maxHP = DataUtils::readU16BE(fileBuffer, offset + 0x22);
    pkmn.attack = DataUtils::readU16BE(fileBuffer, offset + 0x24);
    pkmn.defense = DataUtils::readU16BE(fileBuffer, offset + 0x26);
    pkmn.speed = DataUtils::readU16BE(fileBuffer, offset + 0x28);
    pkmn.special = DataUtils::readU16BE(fileBuffer, offset + 0x2A);
}

void PokemonPartyEditor::parseGen2Pokemon(PokemonData& pkmn, size_t offset) {
    pkmn.species = DataUtils::readU8(fileBuffer, offset + 0x00);
    pkmn.heldItem = DataUtils::readU8(fileBuffer, offset + 0x01);
    
    for (size_t j = 0; j < 4; j++) {
        pkmn.moves[j] = DataUtils::readU8(fileBuffer, offset + 0x02 + j);
    }
    
    pkmn.trainerID = DataUtils::readU16BE(fileBuffer, offset + 0x06);
    
    // Experience is 3 bytes, big-endian
    pkmn.exp = (static_cast<uint32_t>(DataUtils::readU8(fileBuffer, offset + 0x08)) << 16) |
               (static_cast<uint32_t>(DataUtils::readU8(fileBuffer, offset + 0x09)) << 8) |
               static_cast<uint32_t>(DataUtils::readU8(fileBuffer, offset + 0x0A));
    
    pkmn.hpEV = DataUtils::readU16BE(fileBuffer, offset + 0x0B);
    pkmn.attackEV = DataUtils::readU16BE(fileBuffer, offset + 0x0D);
    pkmn.defenseEV = DataUtils::readU16BE(fileBuffer, offset + 0x0F);
    pkmn.speedEV = DataUtils::readU16BE(fileBuffer, offset + 0x11);
    pkmn.specialEV = DataUtils::readU16BE(fileBuffer, offset + 0x13); // Special EV in Gen 2
    pkmn.ivData = DataUtils::readU16BE(fileBuffer, offset + 0x15);
    
    for (size_t j = 0; j < 4; j++) {
        pkmn.ppValues[j] = DataUtils::readU8(fileBuffer, offset + 0x17 + j);
    }
    
    pkmn.friendship = DataUtils::readU8(fileBuffer, offset + 0x1B);
    pkmn.pokerus = DataUtils::readU8(fileBuffer, offset + 0x1C);
    pkmn.caughtData = DataUtils::readU16BE(fileBuffer, offset + 0x1D);
    pkmn.level = DataUtils::readU8(fileBuffer, offset + 0x1F);
    pkmn.status = DataUtils::readU8(fileBuffer, offset + 0x20);
    // 0x21 is unused
    pkmn.currentHP = DataUtils::readU16BE(fileBuffer, offset + 0x22);
    pkmn.maxHP = DataUtils::readU16BE(fileBuffer, offset + 0x24);
    pkmn.attack = DataUtils::readU16BE(fileBuffer, offset + 0x26);
    pkmn.defense = DataUtils::readU16BE(fileBuffer, offset + 0x28);
    pkmn.speed = DataUtils::readU16BE(fileBuffer, offset + 0x2A);
    pkmn.specialAttack = DataUtils::readU16BE(fileBuffer, offset + 0x2C);
    pkmn.specialDefense = DataUtils::readU16BE(fileBuffer, offset + 0x2E);
}

// ============================================================================
// Writing data back
// ============================================================================

void PokemonPartyEditor::writePokemonDataToBuffer() {
    size_t partyOffset     = getPartyOffset();      // primary party block
    size_t nameLength      = getNameLength();
    size_t maxNameChars    = getMaxNameChars();
    size_t pokemonDataSize = getPokemonDataSize();

    // Determine secondary party block (Gen 2 only)
    size_t secondaryPartyOffset = partyOffset; // default to primary (or for non-mirrored cases)
    if (generation == 2) {
        // failsafe if somehow we have a Gen 2 game that isn't Gold/Silver/Crystal
        if (gameType == GameType::GEN2_GS) {
            secondaryPartyOffset = isJapanese ? 0x7A3E : 0x10E8;
        } else if (gameType == GameType::GEN2_CRYSTAL) {
            secondaryPartyOffset = isJapanese ? 0x7A1A : 0x1A65;
        }
    }

    // ============================================================
    // PRIMARY BLOCK: count/species/terminator
    // ============================================================
    DataUtils::writeU8(fileBuffer, partyOffset, partyCount);

    for (size_t i = 0; i < 6; i++) {
        DataUtils::writeU8(fileBuffer, partyOffset + 1 + i, partySpecies[i]);
    }

    DataUtils::writeU8(fileBuffer, partyOffset + 7, 0xFF);

    // ============================================================
    // SECONDARY BLOCK: count/species/terminator (Gen 2 only)
    // ============================================================
    if (generation == 2 && secondaryPartyOffset != partyOffset) {
        DataUtils::writeU8(fileBuffer, secondaryPartyOffset, partyCount);

        for (size_t i = 0; i < 6; i++) {
            DataUtils::writeU8(fileBuffer, secondaryPartyOffset + 1 + i, partySpecies[i]);
        }

        DataUtils::writeU8(fileBuffer, secondaryPartyOffset + 7, 0xFF);
    }

    // ============================================================
    // POKÉMON STRUCT DATA
    // ============================================================
    size_t pokemonDataOffsetPrimary   = partyOffset + 8;
    size_t pokemonDataOffsetSecondary = secondaryPartyOffset + 8;

    for (size_t i = 0; i < MAX_PARTY_SIZE; i++) {
        const PokemonData& pkmn = partyPokemon[i];

        size_t offsetPrimary   = pokemonDataOffsetPrimary   + (i * pokemonDataSize);
        size_t offsetSecondary = pokemonDataOffsetSecondary + (i * pokemonDataSize);

        if (i >= partyCount || pkmn.isEmpty()) {
            // Clear empty slots in PRIMARY
            for (size_t j = 0; j < pokemonDataSize; j++) {
                DataUtils::writeU8(fileBuffer, offsetPrimary + j, 0);
            }

            // Clear empty slots in SECONDARY (Gen 2 only, if different)
            if (generation == 2 && secondaryPartyOffset != partyOffset) {
                for (size_t j = 0; j < pokemonDataSize; j++) {
                    DataUtils::writeU8(fileBuffer, offsetSecondary + j, 0);
                }
            }
            continue;
        }

        if (generation == 1) {
            writeGen1Pokemon(pkmn, offsetPrimary);
        } else if (generation == 2) {
            // Write to PRIMARY
            writeGen2Pokemon(pkmn, offsetPrimary);

            // Write to SECONDARY (if different)
            if (secondaryPartyOffset != partyOffset) {
                writeGen2Pokemon(pkmn, offsetSecondary);
            }
        }
    }

    // ============================================================
    // NAME OFFSETS (PRIMARY)
    // ============================================================
    size_t otNamesOffsetPrimary, nicknamesOffsetPrimary;

    if (generation == 1) {
        if (isJapanese) {
            otNamesOffsetPrimary = partyOffset + 0x110;
            nicknamesOffsetPrimary = partyOffset + 0x134;
        } else {
            otNamesOffsetPrimary = partyOffset + 0x110;
            nicknamesOffsetPrimary = partyOffset + 0x152;
        }
    } else { // Gen 2
        size_t totalPokemonData = MAX_PARTY_SIZE * pokemonDataSize;
        otNamesOffsetPrimary = pokemonDataOffsetPrimary + totalPokemonData;
        nicknamesOffsetPrimary = otNamesOffsetPrimary + (MAX_PARTY_SIZE * nameLength);
    }

    // ============================================================
    // NAME OFFSETS (SECONDARY, Gen 2 only)
    // ============================================================
    size_t otNamesOffsetSecondary = 0, nicknamesOffsetSecondary = 0;
    bool writeSecondaryNames = (generation == 2 && secondaryPartyOffset != partyOffset);

    if (writeSecondaryNames) {
        size_t totalPokemonData = MAX_PARTY_SIZE * pokemonDataSize;
        otNamesOffsetSecondary = pokemonDataOffsetSecondary + totalPokemonData;
        nicknamesOffsetSecondary = otNamesOffsetSecondary + (MAX_PARTY_SIZE * nameLength);
    }

    // ============================================================
    // WRITE OT NAMES (PRIMARY + SECONDARY)
    // ============================================================
    for (size_t i = 0; i < MAX_PARTY_SIZE; i++) {
        size_t offsetPrimary = otNamesOffsetPrimary + (i * nameLength);

        if (i >= partyCount || partyPokemon[i].isEmpty()) {
            for (size_t j = 0; j < nameLength; j++) {
                DataUtils::writeU8(fileBuffer, offsetPrimary + j, 0x50);
            }

            if (writeSecondaryNames) {
                size_t offsetSecondary = otNamesOffsetSecondary + (i * nameLength);
                for (size_t j = 0; j < nameLength; j++) {
                    DataUtils::writeU8(fileBuffer, offsetSecondary + j, 0x50);
                }
            }
            continue;
        }

        std::vector<uint8_t> encoded =
            encodeText(partyPokemon[i].otName, encoding, maxNameChars);

        for (size_t j = 0; j < nameLength; j++) {
            uint8_t value = (j < encoded.size()) ? encoded[j] : 0x50;
            DataUtils::writeU8(fileBuffer, offsetPrimary + j, value);
        }

        if (writeSecondaryNames) {
            size_t offsetSecondary = otNamesOffsetSecondary + (i * nameLength);
            for (size_t j = 0; j < nameLength; j++) {
                uint8_t value = (j < encoded.size()) ? encoded[j] : 0x50;
                DataUtils::writeU8(fileBuffer, offsetSecondary + j, value);
            }
        }
    }

    // ============================================================
    // WRITE NICKNAMES (PRIMARY + SECONDARY)
    // ============================================================
    for (size_t i = 0; i < MAX_PARTY_SIZE; i++) {
        size_t offsetPrimary = nicknamesOffsetPrimary + (i * nameLength);

        if (i >= partyCount || partyPokemon[i].isEmpty()) {
            for (size_t j = 0; j < nameLength; j++) {
                DataUtils::writeU8(fileBuffer, offsetPrimary + j, 0x50);
            }

            if (writeSecondaryNames) {
                size_t offsetSecondary = nicknamesOffsetSecondary + (i * nameLength);
                for (size_t j = 0; j < nameLength; j++) {
                    DataUtils::writeU8(fileBuffer, offsetSecondary + j, 0x50);
                }
            }
            continue;
        }

        std::vector<uint8_t> encoded =
            encodeText(partyPokemon[i].nickname, encoding, maxNameChars);

        for (size_t j = 0; j < nameLength; j++) {
            uint8_t value = (j < encoded.size()) ? encoded[j] : 0x50;
            DataUtils::writeU8(fileBuffer, offsetPrimary + j, value);
        }

        if (writeSecondaryNames) {
            size_t offsetSecondary = nicknamesOffsetSecondary + (i * nameLength);
            for (size_t j = 0; j < nameLength; j++) {
                uint8_t value = (j < encoded.size()) ? encoded[j] : 0x50;
                DataUtils::writeU8(fileBuffer, offsetSecondary + j, value);
            }
        }
    }
}

void PokemonPartyEditor::writeGen1Pokemon(const PokemonData& pkmn, size_t offset) {
    DataUtils::writeU8(fileBuffer, offset + 0x00, pkmn.species);
    DataUtils::writeU16BE(fileBuffer, offset + 0x01, pkmn.currentHP);
    DataUtils::writeU8(fileBuffer, offset + 0x03, pkmn.levelBox);
    DataUtils::writeU8(fileBuffer, offset + 0x04, pkmn.status);
    DataUtils::writeU8(fileBuffer, offset + 0x05, pkmn.type1);
    DataUtils::writeU8(fileBuffer, offset + 0x06, pkmn.type2);
    DataUtils::writeU8(fileBuffer, offset + 0x07, pkmn.catchRate);
    
    for (size_t j = 0; j < 4; j++) {
        DataUtils::writeU8(fileBuffer, offset + 0x08 + j, pkmn.moves[j]);
    }
    
    DataUtils::writeU16BE(fileBuffer, offset + 0x0C, pkmn.trainerID);
    
    // Write experience (3 bytes, big-endian)
    DataUtils::writeU8(fileBuffer, offset + 0x0E, (pkmn.exp >> 16) & 0xFF);
    DataUtils::writeU8(fileBuffer, offset + 0x0F, (pkmn.exp >> 8) & 0xFF);
    DataUtils::writeU8(fileBuffer, offset + 0x10, pkmn.exp & 0xFF);
    
    DataUtils::writeU16BE(fileBuffer, offset + 0x11, pkmn.hpEV);
    DataUtils::writeU16BE(fileBuffer, offset + 0x13, pkmn.attackEV);
    DataUtils::writeU16BE(fileBuffer, offset + 0x15, pkmn.defenseEV);
    DataUtils::writeU16BE(fileBuffer, offset + 0x17, pkmn.speedEV);
    DataUtils::writeU16BE(fileBuffer, offset + 0x19, pkmn.specialEV);
    DataUtils::writeU16BE(fileBuffer, offset + 0x1B, pkmn.ivData);
    
    for (size_t j = 0; j < 4; j++) {
        DataUtils::writeU8(fileBuffer, offset + 0x1D + j, pkmn.ppValues[j]);
    }
    
    DataUtils::writeU8(fileBuffer, offset + 0x21, pkmn.level);
    DataUtils::writeU16BE(fileBuffer, offset + 0x22, pkmn.maxHP);
    DataUtils::writeU16BE(fileBuffer, offset + 0x24, pkmn.attack);
    DataUtils::writeU16BE(fileBuffer, offset + 0x26, pkmn.defense);
    DataUtils::writeU16BE(fileBuffer, offset + 0x28, pkmn.speed);
    DataUtils::writeU16BE(fileBuffer, offset + 0x2A, pkmn.special);
}

void PokemonPartyEditor::writeGen2Pokemon(const PokemonData& pkmn, size_t offset) {
    DataUtils::writeU8(fileBuffer, offset + 0x00, pkmn.species);
    DataUtils::writeU8(fileBuffer, offset + 0x01, pkmn.heldItem);
    
    for (size_t j = 0; j < 4; j++) {
        DataUtils::writeU8(fileBuffer, offset + 0x02 + j, pkmn.moves[j]);
    }
    
    DataUtils::writeU16BE(fileBuffer, offset + 0x06, pkmn.trainerID);
    
    // Write experience (3 bytes, big-endian)
    DataUtils::writeU8(fileBuffer, offset + 0x08, (pkmn.exp >> 16) & 0xFF);
    DataUtils::writeU8(fileBuffer, offset + 0x09, (pkmn.exp >> 8) & 0xFF);
    DataUtils::writeU8(fileBuffer, offset + 0x0A, pkmn.exp & 0xFF);
    
    DataUtils::writeU16BE(fileBuffer, offset + 0x0B, pkmn.hpEV);
    DataUtils::writeU16BE(fileBuffer, offset + 0x0D, pkmn.attackEV);
    DataUtils::writeU16BE(fileBuffer, offset + 0x0F, pkmn.defenseEV);
    DataUtils::writeU16BE(fileBuffer, offset + 0x11, pkmn.speedEV);
    DataUtils::writeU16BE(fileBuffer, offset + 0x13, pkmn.specialEV);
    DataUtils::writeU16BE(fileBuffer, offset + 0x15, pkmn.ivData);
    
    for (size_t j = 0; j < 4; j++) {
        DataUtils::writeU8(fileBuffer, offset + 0x17 + j, pkmn.ppValues[j]);
    }
    
    DataUtils::writeU8(fileBuffer, offset + 0x1B, pkmn.friendship);
    DataUtils::writeU8(fileBuffer, offset + 0x1C, pkmn.pokerus);
    DataUtils::writeU16BE(fileBuffer, offset + 0x1D, pkmn.caughtData);
    DataUtils::writeU8(fileBuffer, offset + 0x1F, pkmn.level);
    DataUtils::writeU8(fileBuffer, offset + 0x20, pkmn.status);
    DataUtils::writeU8(fileBuffer, offset + 0x21, 0); // Unused
    DataUtils::writeU16BE(fileBuffer, offset + 0x22, pkmn.currentHP);
    DataUtils::writeU16BE(fileBuffer, offset + 0x24, pkmn.maxHP);
    DataUtils::writeU16BE(fileBuffer, offset + 0x26, pkmn.attack);
    DataUtils::writeU16BE(fileBuffer, offset + 0x28, pkmn.defense);
    DataUtils::writeU16BE(fileBuffer, offset + 0x2A, pkmn.speed);
    DataUtils::writeU16BE(fileBuffer, offset + 0x2C, pkmn.specialAttack);
    DataUtils::writeU16BE(fileBuffer, offset + 0x2E, pkmn.specialDefense);
}

// ============================================================================
// Tab and field helpers
// ============================================================================

std::string PokemonPartyEditor::getPokemonTabName(int index) const {
    if (index >= static_cast<int>(partyCount) || partyPokemon[index].isEmpty()) {
        return "(Empty)";
    }
    
    const char* name = PokemonIndex::getPokemonName(partyPokemon[index].species, generation);
    return name ? name : "???";
}

const char* PokemonPartyEditor::getFieldName(EditField field) const {
    switch (field) {
        case EditField::SPECIES: return "Species";
        case EditField::LEVEL: return "Level";
        case EditField::CURRENT_HP: return "Current HP";
        case EditField::MAX_HP: return "Max HP";
        case EditField::STATUS: return "Status";
        case EditField::TYPE1: return "Type 1";
        case EditField::TYPE2: return "Type 2";
        case EditField::HELD_ITEM: return "Held Item";
        case EditField::MOVE1: return "Move 1";
        case EditField::MOVE2: return "Move 2";
        case EditField::MOVE3: return "Move 3";
        case EditField::MOVE4: return "Move 4";
        case EditField::PP1: return "Move 1 PP";
        case EditField::PP2: return "Move 2 PP";
        case EditField::PP3: return "Move 3 PP";
        case EditField::PP4: return "Move 4 PP";
        case EditField::ATTACK: return "Attack";
        case EditField::DEFENSE: return "Defense";
        case EditField::SPEED: return "Speed";
        case EditField::SPECIAL: return "Special";
        case EditField::SPECIAL_ATK: return "Special Attack";
        case EditField::SPECIAL_DEF: return "Special Defense";
        case EditField::HP_EV: return "HP EV";
        case EditField::ATTACK_EV: return "Attack EV";
        case EditField::DEFENSE_EV: return "Defense EV";
        case EditField::SPEED_EV: return "Speed EV";
        case EditField::SPECIAL_EV: return generation == 1 ? "Special EV" : "Special EV (both)";
        case EditField::FRIENDSHIP: return "Friendship";
        case EditField::POKERUS: return "Pokerus";
        case EditField::NICKNAME: return "Nickname";
        case EditField::OT_NAME: return "OT Name";
        case EditField::EXP: return "Experience";
        default: return "Unknown";
    }
}

bool PokemonPartyEditor::isFieldVisible(EditField field) const {
    // Gen 1 only fields
    if (generation == 1) {
        if (field == EditField::HELD_ITEM || field == EditField::SPECIAL_ATK ||
            field == EditField::SPECIAL_DEF || field == EditField::FRIENDSHIP ||
            field == EditField::POKERUS) {
            return false;
        }
    }
    
    // Gen 2+ only fields
    if (generation >= 2) {
        if (field == EditField::TYPE1 || field == EditField::TYPE2 || 
            field == EditField::SPECIAL) {
            return false;
        }
    }
    
    return true;
}

std::string PokemonPartyEditor::getFieldValue(int pokemonIndex, EditField field) const {
    if (pokemonIndex < 0 || static_cast<size_t>(pokemonIndex) >= MAX_PARTY_SIZE) return "";
    
    if (!isFieldVisible(field)) return "";
    
    const PokemonData& pkmn = partyPokemon[pokemonIndex];
    
    if (pkmn.isEmpty() && field != EditField::SPECIES) {
        return "-";
    }
    
    std::stringstream ss;
    
    switch (field) {
        case EditField::SPECIES: {
            const char* name = PokemonIndex::getPokemonName(pkmn.species, generation);
            ss << (name ? name : "None") << " [" << HexUtils::toHexString(pkmn.species, 2) << "]";
            break;
        }
        case EditField::LEVEL:
            ss << static_cast<int>(pkmn.level);
            break;
        case EditField::CURRENT_HP:
            ss << pkmn.currentHP;
            break;
        case EditField::MAX_HP:
            ss << pkmn.maxHP;
            break;
        case EditField::STATUS: {
            ss << getStatusName(pkmn.status) << " [" << HexUtils::toHexString(pkmn.status, 2) << "]";
            break;
        }
        case EditField::TYPE1: {
            ss << getTypeName(pkmn.type1) << " [" << HexUtils::toHexString(pkmn.type1, 2) << "]";
            break;
        }
        case EditField::TYPE2: {
            ss << getTypeName(pkmn.type2) << " [" << HexUtils::toHexString(pkmn.type2, 2) << "]";
            break;
        }
        case EditField::HELD_ITEM: {
            const char* itemName = getItemName(pkmn.heldItem);
            ss << (itemName ? itemName : "None") << " [" << HexUtils::toHexString(pkmn.heldItem, 2) << "]";
            break;
        }
        case EditField::MOVE1:
        case EditField::MOVE2:
        case EditField::MOVE3:
        case EditField::MOVE4: {
            int moveIndex = static_cast<int>(field) - static_cast<int>(EditField::MOVE1);
            const char* moveName = getMoveName(pkmn.moves[moveIndex]);
            ss << (moveName ? moveName : "None") << " [" << HexUtils::toHexString(pkmn.moves[moveIndex], 2) << "]";
            break;
        }
        case EditField::PP1:
        case EditField::PP2:
        case EditField::PP3:
        case EditField::PP4: {
            int ppIndex = static_cast<int>(field) - static_cast<int>(EditField::PP1);
            int currentPP = pkmn.ppValues[ppIndex] & 0x3F;
            int ppUps = (pkmn.ppValues[ppIndex] >> 6) & 0x03;
            ss << currentPP << " (+" << ppUps << " PP Ups)";
            break;
        }
        case EditField::ATTACK:
            ss << pkmn.attack;
            break;
        case EditField::DEFENSE:
            ss << pkmn.defense;
            break;
        case EditField::SPEED:
            ss << pkmn.speed;
            break;
        case EditField::SPECIAL:
            ss << pkmn.special;
            break;
        case EditField::SPECIAL_ATK:
            ss << pkmn.specialAttack;
            break;
        case EditField::SPECIAL_DEF:
            ss << pkmn.specialDefense;
            break;
        case EditField::HP_EV:
            ss << pkmn.hpEV;
            break;
        case EditField::ATTACK_EV:
            ss << pkmn.attackEV;
            break;
        case EditField::DEFENSE_EV:
            ss << pkmn.defenseEV;
            break;
        case EditField::SPEED_EV:
            ss << pkmn.speedEV;
            break;
        case EditField::SPECIAL_EV:
            ss << pkmn.specialEV;
            break;
        case EditField::FRIENDSHIP:
            ss << static_cast<int>(pkmn.friendship);
            break;
        case EditField::POKERUS:
            ss << HexUtils::toHexString(pkmn.pokerus, 2);
            break;
        case EditField::NICKNAME:
            ss << pkmn.nickname;
            break;
        case EditField::OT_NAME:
            ss << pkmn.otName;
            break;
        case EditField::EXP:
            ss << pkmn.exp;
            break;
        default:
            break;
    }
    
    return ss.str();
}

bool PokemonPartyEditor::isFieldEditable(EditField field) const {
    return isFieldVisible(field);
}

bool PokemonPartyEditor::isNameEditableField(EditField field) const {
    // Species, moves, and items can be edited by name
    return (field == EditField::SPECIES ||
            field == EditField::MOVE1 ||
            field == EditField::MOVE2 ||
            field == EditField::MOVE3 ||
            field == EditField::MOVE4 ||
            (generation >= 2 && field == EditField::HELD_ITEM));
}

const char* PokemonPartyEditor::getStatusName(uint8_t status) const {
    if (status == 0) return "Healthy";
    if (status & 0x04) return "Sleep";
    if (status & 0x08) return "Poison";
    if (status & 0x10) return "Burn";
    if (status & 0x20) return "Freeze";
    if (status & 0x40) return "Paralyze";
    return "Unknown";
}

const char* PokemonPartyEditor::getTypeName(uint8_t type) const {
    // Gen 1 only
    return PokemonTypes::getGen1TypeName(type);
}

const char* PokemonPartyEditor::getMoveName(uint8_t move) const {
    return PokemonMoves::getMoveName(move, generation);
}

const char* PokemonPartyEditor::getItemName(uint8_t item) const {
    if (generation == 2) {
        bool isCrystal = (gameType == GameType::GEN2_CRYSTAL);
        return ItemsIndex::getGen2ItemName(item, isCrystal);
    }
    // Gen 1 doesn't have held items
    return nullptr;
}

// ============================================================================
// Name lookup helpers
// ============================================================================

uint8_t PokemonPartyEditor::lookupPokemonIdByName(const std::string& name) const {
    std::string query = name;
    std::transform(query.begin(), query.end(), query.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    
    // Search through Pokemon based on generation
    const std::unordered_map<uint8_t, PokemonIndex::PokemonInfo>* pokemonMap = nullptr;
    
    if (generation == 1) {
        pokemonMap = &PokemonIndex::GEN1_POKEMON;
    } else if (generation == 2) {
        pokemonMap = &PokemonIndex::GEN2_POKEMON;
    } else {
        return 0;
    }
    
    for (const auto& kv : *pokemonMap) {
        const char* pokemonName = kv.second.name;
        if (!pokemonName) continue;
        
        std::string nameStr(pokemonName);
        std::transform(nameStr.begin(), nameStr.end(), nameStr.begin(), [](unsigned char c) {
            return static_cast<char>(std::toupper(c));
        });
        
        if (nameStr == query) {
            return kv.first;
        }
    }
    
    return 0;
}

uint8_t PokemonPartyEditor::lookupMoveIdByName(const std::string& name) const {
    std::string query = name;
    std::transform(query.begin(), query.end(), query.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    
    // Use generation-appropriate move lookup
    const std::unordered_map<uint8_t, const char*>* moveMap = nullptr;
    
    if (generation == 1) {
        moveMap = &PokemonMoves::GEN1_MOVES;
    } else if (generation == 2) {
        moveMap = &PokemonMoves::GEN2_MOVES;
    } else {
        return 0;
    }
    
    for (const auto& kv : *moveMap) {
        const char* moveName = kv.second;
        if (!moveName) continue;
        
        std::string nameStr(moveName);
        std::transform(nameStr.begin(), nameStr.end(), nameStr.begin(), [](unsigned char c) {
            return static_cast<char>(std::toupper(c));
        });
        
        if (nameStr == query) {
            return kv.first;
        }
    }
    
    return 0;
}

uint8_t PokemonPartyEditor::lookupItemIdByName(const std::string& name) const {
    if (generation != 2) return 0;
    
    std::string query = name;
    std::transform(query.begin(), query.end(), query.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    
    bool isCrystal = (gameType == GameType::GEN2_CRYSTAL);
    
    // Search through Gen 2 items
    for (const auto& kv : ItemsIndex::GEN2_ITEMS) {
        const char* itemName = ItemsIndex::getGen2ItemName(kv.first, isCrystal);
        if (!itemName) continue;
        
        std::string nameStr(itemName);
        std::transform(nameStr.begin(), nameStr.end(), nameStr.begin(), [](unsigned char c) {
            return static_cast<char>(std::toupper(c));
        });
        
        if (nameStr == query) {
            return kv.first;
        }
    }
    
    return 0;
}

// ============================================================================
// IV/DV helpers
// ============================================================================

uint8_t PokemonPartyEditor::getIV(uint16_t ivData, const std::string& stat) const {
    // IV data format (16 bits):
    // Bits 15-12: Attack IV
    // Bits 11-8: Defense IV
    // Bits 7-4: Speed IV
    // Bits 3-0: Special IV
    // HP IV = (Attack IV & 1) << 3 | (Defense IV & 1) << 2 | (Speed IV & 1) << 1 | (Special IV & 1)
    
    if (stat == "attack") {
        return (ivData >> 12) & 0x0F;
    } else if (stat == "defense") {
        return (ivData >> 8) & 0x0F;
    } else if (stat == "speed") {
        return (ivData >> 4) & 0x0F;
    } else if (stat == "special") {
        return ivData & 0x0F;
    } else if (stat == "hp") {
        uint8_t attackIV = (ivData >> 12) & 0x0F;
        uint8_t defenseIV = (ivData >> 8) & 0x0F;
        uint8_t speedIV = (ivData >> 4) & 0x0F;
        uint8_t specialIV = ivData & 0x0F;
        return ((attackIV & 1) << 3) | ((defenseIV & 1) << 2) | 
               ((speedIV & 1) << 1) | (specialIV & 1);
    }
    return 0;
}

uint16_t PokemonPartyEditor::setIV(uint16_t ivData, const std::string& stat, uint8_t value) const {
    value &= 0x0F; // Ensure it's 4-bit
    
    if (stat == "attack") {
        return (ivData & 0x0FFF) | (static_cast<uint16_t>(value) << 12);
    } else if (stat == "defense") {
        return (ivData & 0xF0FF) | (static_cast<uint16_t>(value) << 8);
    } else if (stat == "speed") {
        return (ivData & 0xFF0F) | (static_cast<uint16_t>(value) << 4);
    } else if (stat == "special") {
        return (ivData & 0xFFF0) | value;
    }
    
    // Note: HP IV is derived from other IVs and cannot be set directly
    return ivData;
}

// ============================================================================
// Editing
// ============================================================================

void PokemonPartyEditor::startEditing(EditField field, bool byName) {
    if (!isFieldEditable(field)) return;
    
    editing = true;
    editingByName = byName && isNameEditableField(field);
    selectedField = static_cast<int>(field);
    editBuffer.clear();
    requestRedraw();
}

void PokemonPartyEditor::handleEditInput(SDL_Keycode key) {
    if (!editing) return;
    
    if (key == SDLK_ESCAPE) {
        editing = false;
        editingByName = false;
        editBuffer.clear();
        requestRedraw();
        return;
    }
    
    if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
        commitEdit();
        return;
    }
    
    if (key == SDLK_BACKSPACE) {
        if (!editBuffer.empty()) {
            editBuffer.pop_back();
            requestRedraw();
        }
        return;
    }
    
    EditField field = static_cast<EditField>(selectedField);
    
    // Handle text input for names or name-based editing
    if (field == EditField::NICKNAME || field == EditField::OT_NAME || editingByName) {
        char c = 0;
        if (key >= SDLK_A && key <= SDLK_Z) {
            c = static_cast<char>('A' + (key - SDLK_A));
        } else if (key >= SDLK_0 && key <= SDLK_9) {
            c = static_cast<char>('0' + (key - SDLK_0));
        } else if (key == SDLK_SPACE) {
            c = ' ';
        } else if (key == SDLK_PERIOD) {
            c = '.';
        } else if (key == SDLK_MINUS) {
            c = '-';
        } else if (key == SDLK_APOSTROPHE) {
            c = '\'';
        } else {
            return;
        }
        
        // Limit length for nickname/OT name fields based on language
        size_t maxChars = getMaxNameChars();
        if ((field == EditField::NICKNAME || field == EditField::OT_NAME) && 
            editBuffer.length() >= maxChars) {
            return;
        }
        
        editBuffer.push_back(c);
        requestRedraw();
        return;
    }
    
    // Handle numeric input
    char c = 0;
    if (key >= SDLK_0 && key <= SDLK_9) {
        c = static_cast<char>('0' + (key - SDLK_0));
    } else if ((field == EditField::SPECIES || field == EditField::STATUS || 
                field == EditField::TYPE1 || field == EditField::TYPE2 ||
                field == EditField::MOVE1 || field == EditField::MOVE2 ||
                field == EditField::MOVE3 || field == EditField::MOVE4) &&
               key >= SDLK_A && key <= SDLK_F) {
        // Hex input for certain fields
        c = static_cast<char>('A' + (key - SDLK_A));
    } else {
        return;
    }
    
    editBuffer.push_back(c);
    requestRedraw();
}

void PokemonPartyEditor::commitEdit() {
    if (!editing) return;
    
    EditField field = static_cast<EditField>(selectedField);
    
    // Handle name-based editing
    if (editingByName && !editBuffer.empty()) {
        if (field == EditField::SPECIES) {
            uint8_t id = lookupPokemonIdByName(editBuffer);
            if (id != 0) {
                editBuffer = HexUtils::toHexString(id, 2);
            } else {
                // Invalid name, cancel edit
                editing = false;
                editingByName = false;
                editBuffer.clear();
                requestRedraw();
                return;
            }
        } else if (field >= EditField::MOVE1 && field <= EditField::MOVE4) {
            uint8_t id = lookupMoveIdByName(editBuffer);
            if (id != 0 || editBuffer == "-" || editBuffer == "NONE") {
                editBuffer = HexUtils::toHexString(id, 2);
            } else {
                // Invalid name, cancel edit
                editing = false;
                editingByName = false;
                editBuffer.clear();
                requestRedraw();
                return;
            }
        } else if (field == EditField::HELD_ITEM && generation >= 2) {
            uint8_t id = lookupItemIdByName(editBuffer);
            if (id != 0 || editBuffer == "-" || editBuffer == "NONE") {
                editBuffer = HexUtils::toHexString(id, 2);
            } else {
                // Invalid name, cancel edit
                editing = false;
                editingByName = false;
                editBuffer.clear();
                requestRedraw();
                return;
            }
        }
    }
    
    if (validateAndApplyEdit(currentPokemonIndex, field, editBuffer)) {
        hasUnsavedChanges = true;
    }
    
    editing = false;
    editingByName = false;
    editBuffer.clear();
    requestRedraw();
}

bool PokemonPartyEditor::validateAndApplyEdit(int pokemonIndex, EditField field, 
                                               const std::string& value) {
    if (pokemonIndex < 0 || static_cast<size_t>(pokemonIndex) >= MAX_PARTY_SIZE) return false;
    
    PokemonData& pkmn = partyPokemon[pokemonIndex];
    
    try {
        switch (field) {
            case EditField::SPECIES: {
                uint8_t species = static_cast<uint8_t>(std::stoul(value, nullptr, 16));
                pkmn.species = species;
                partySpecies[pokemonIndex] = species;
                
                // Update types based on species (Gen 1 only)
                if (generation == 1) {
                    const PokemonIndex::PokemonInfo* info = PokemonIndex::getPokemonInfo(species, 1);
                    if (info) {
                        pkmn.type1 = info->type1;
                        pkmn.type2 = info->type2;
                    }
                }
                
                // Update party count if needed
                if (species != 0 && species != 0xFF && pokemonIndex >= partyCount) {
                    partyCount = pokemonIndex + 1;
                }
                break;
            }
            case EditField::LEVEL: {
                uint8_t level = static_cast<uint8_t>(std::stoul(value));
                if (level > 100) level = 100;
                if (level < 1) level = 1;
                pkmn.level = level;
                if (generation == 1) {
                    pkmn.levelBox = level;
                }
                break;
            }
            case EditField::CURRENT_HP: {
                uint16_t hp = static_cast<uint16_t>(std::stoul(value));
                if (hp > pkmn.maxHP) hp = pkmn.maxHP;
                pkmn.currentHP = hp;
                break;
            }
            case EditField::MAX_HP: {
                uint16_t hp = static_cast<uint16_t>(std::stoul(value));
                if (hp > 999) hp = 999;
                pkmn.maxHP = hp;
                if (pkmn.currentHP > hp) pkmn.currentHP = hp;
                break;
            }
            case EditField::STATUS:
                pkmn.status = static_cast<uint8_t>(std::stoul(value, nullptr, 16));
                break;
            case EditField::TYPE1:
                if (generation == 1) {
                    pkmn.type1 = static_cast<uint8_t>(std::stoul(value, nullptr, 16));
                }
                break;
            case EditField::TYPE2:
                if (generation == 1) {
                    pkmn.type2 = static_cast<uint8_t>(std::stoul(value, nullptr, 16));
                }
                break;
            case EditField::HELD_ITEM:
                if (generation >= 2) {
                    pkmn.heldItem = static_cast<uint8_t>(std::stoul(value, nullptr, 16));
                }
                break;
            case EditField::MOVE1:
            case EditField::MOVE2:
            case EditField::MOVE3:
            case EditField::MOVE4: {
                int moveIndex = static_cast<int>(field) - static_cast<int>(EditField::MOVE1);
                pkmn.moves[moveIndex] = static_cast<uint8_t>(std::stoul(value, nullptr, 16));
                break;
            }
            case EditField::PP1:
            case EditField::PP2:
            case EditField::PP3:
            case EditField::PP4: {
                int ppIndex = static_cast<int>(field) - static_cast<int>(EditField::PP1);
                uint8_t pp = static_cast<uint8_t>(std::stoul(value));
                if (pp > 63) pp = 63;
                pkmn.ppValues[ppIndex] = (pkmn.ppValues[ppIndex] & 0xC0) | pp;
                break;
            }
            case EditField::ATTACK:
                pkmn.attack = static_cast<uint16_t>(std::stoul(value));
                if (pkmn.attack > 999) pkmn.attack = 999;
                break;
            case EditField::DEFENSE:
                pkmn.defense = static_cast<uint16_t>(std::stoul(value));
                if (pkmn.defense > 999) pkmn.defense = 999;
                break;
            case EditField::SPEED:
                pkmn.speed = static_cast<uint16_t>(std::stoul(value));
                if (pkmn.speed > 999) pkmn.speed = 999;
                break;
            case EditField::SPECIAL:
                if (generation == 1) {
                    pkmn.special = static_cast<uint16_t>(std::stoul(value));
                    if (pkmn.special > 999) pkmn.special = 999;
                }
                break;
            case EditField::SPECIAL_ATK:
                if (generation >= 2) {
                    pkmn.specialAttack = static_cast<uint16_t>(std::stoul(value));
                    if (pkmn.specialAttack > 999) pkmn.specialAttack = 999;
                }
                break;
            case EditField::SPECIAL_DEF:
                if (generation >= 2) {
                    pkmn.specialDefense = static_cast<uint16_t>(std::stoul(value));
                    if (pkmn.specialDefense > 999) pkmn.specialDefense = 999;
                }
                break;
            case EditField::HP_EV:
                pkmn.hpEV = static_cast<uint16_t>(std::stoul(value));
                if (pkmn.hpEV > 65535) pkmn.hpEV = 65535;
                break;
            case EditField::ATTACK_EV:
                pkmn.attackEV = static_cast<uint16_t>(std::stoul(value));
                if (pkmn.attackEV > 65535) pkmn.attackEV = 65535;
                break;
            case EditField::DEFENSE_EV:
                pkmn.defenseEV = static_cast<uint16_t>(std::stoul(value));
                if (pkmn.defenseEV > 65535) pkmn.defenseEV = 65535;
                break;
            case EditField::SPEED_EV:
                pkmn.speedEV = static_cast<uint16_t>(std::stoul(value));
                if (pkmn.speedEV > 65535) pkmn.speedEV = 65535;
                break;
            case EditField::SPECIAL_EV:
                pkmn.specialEV = static_cast<uint16_t>(std::stoul(value));
                if (pkmn.specialEV > 65535) pkmn.specialEV = 65535;
                break;
            case EditField::FRIENDSHIP:
                if (generation >= 2) {
                    pkmn.friendship = static_cast<uint8_t>(std::stoul(value));
                }
                break;
            case EditField::POKERUS:
                if (generation >= 2) {
                    pkmn.pokerus = static_cast<uint8_t>(std::stoul(value, nullptr, 16));
                }
                break;
            case EditField::NICKNAME:
                pkmn.nickname = value;
                break;
            case EditField::OT_NAME:
                pkmn.otName = value;
                break;
            case EditField::EXP: {
                uint32_t exp = static_cast<uint32_t>(std::stoul(value));
                if (exp > 0xFFFFFF) exp = 0xFFFFFF; // 3 bytes max
                pkmn.exp = exp;
                break;
            }
            default:
                return false;
        }
        
        return true;
    } catch (...) {
        return false;
    }
}


// ============================================================================
// Checksum
// ============================================================================

void PokemonPartyEditor::updateChecksum() {
    if (generation == 1) {
        updateChecksumGen1();
    } else if (generation == 2) {
        updateChecksumGen2();
    }
}

void PokemonPartyEditor::updateChecksumGen1() {
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

void PokemonPartyEditor::updateChecksumGen2() {
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

    bool crystal = (gameType == GameType::GEN2_CRYSTAL);
    if (!crystal) {
        // Gold/Silver
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
        // Crystal
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

bool PokemonPartyEditor::fileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

std::string PokemonPartyEditor::getOutputPath() {
    std::string baseName = HexUtils::getBaseName(fileName);
    if (overwriteMode) {
        return fileName;
    }
    return std::string("edited_files/") + baseName;
}

bool PokemonPartyEditor::saveFile() {
    writePokemonDataToBuffer();
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
// Rendering
// ============================================================================

void PokemonPartyEditor::render() {
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

   // Render Pokemon tabs
   std::stringstream ps;
   ps << "Pokemon: ";
   for (size_t i = 0; i < MAX_PARTY_SIZE; i++) {
       if (i > 0) ps << " | ";
       
       SDL_Color tabColor = colors.text;
       if (i >= partyCount) {
           tabColor = colors.textDim;
       }
       
       if (static_cast<int>(i) == currentPokemonIndex) {
           ps << '[' << getPokemonTabName(static_cast<int>(i)) << ']';
       } else {
           ps << getPokemonTabName(static_cast<int>(i));
       }
   }
   renderText(ps.str(), 10, 5 + charHeight, colors.text);

   // Save button
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

   // Count visible fields and build mapping
   std::vector<EditField> visibleFields;
   std::unordered_map<int, size_t> fieldToVisibleIndex;  // Maps EditField enum value to index in visibleFields
   for (int i = 0; i < static_cast<int>(EditField::FIELD_COUNT); i++) {
       EditField field = static_cast<EditField>(i);
       if (isFieldVisible(field)) {
           fieldToVisibleIndex[i] = visibleFields.size();
           visibleFields.push_back(field);
       }
   }

   // Render Pokemon data fields
   int startY = headerH + 10;
   int rowH = charHeight + 4;
   int fieldCount = static_cast<int>(visibleFields.size());
   
   // Calculate scrollbar
   int instructionsH = charHeight * 3 + 10;
   int availableH = windowHeight - headerH - instructionsH - 20;
   size_t visibleRows = (availableH > 0) ? (static_cast<size_t>(availableH) / rowH) : 1;
   if (visibleRows == 0) visibleRows = 1;
   
   scrollbar.headerOffset = headerH;
   scrollbar.visibleItems = visibleRows;
   scrollbar.totalItems = fieldCount;
   
   if (scrollbar.offset > scrollbar.maxOffset()) {
       scrollbar.offset = scrollbar.maxOffset();
   }
   
   // Find the visible index of the currently selected field
   auto it = fieldToVisibleIndex.find(selectedField);
   if (it != fieldToVisibleIndex.end()) {
       size_t selectedVisibleIndex = it->second;
       
       // Adjust scroll offset to keep selected field visible
       if (selectedVisibleIndex < scrollbar.offset) {
           scrollbar.offset = selectedVisibleIndex;
       } else if (selectedVisibleIndex >= scrollbar.offset + scrollbar.visibleItems) {
           if (scrollbar.visibleItems > 0) {
               scrollbar.offset = selectedVisibleIndex - scrollbar.visibleItems + 1;
           }
       }
   }
   
   int rowWidth = windowWidth - 20;
   if (scrollbar.canScroll()) {
       rowWidth -= scrollbar.width;
   }
   
   size_t startIndex = scrollbar.offset;
   size_t endIndex = std::min(startIndex + scrollbar.visibleItems, 
                              static_cast<size_t>(fieldCount));
   
   for (size_t idx = startIndex; idx < endIndex; idx++) {
       size_t local = idx - startIndex;
       int y = startY + static_cast<int>(local) * rowH;
       SDL_Rect rowRect = {10, y, rowWidth, rowH - 2};
       
       EditField field = visibleFields[idx];
       
       if (static_cast<int>(field) == selectedField) {
           renderFilledRect(rowRect, colors.selectedBg);
       }
       
       std::string fieldName = getFieldName(field);
       std::string fieldValue = getFieldValue(currentPokemonIndex, field);
       
       // Field name
       renderText(fieldName + ":", rowRect.x + 5, y + 2, colors.text);
       
       // Field value
       int valueX = rowRect.x + 200;
       if (editing && static_cast<int>(field) == selectedField) {
           std::string editText = editBuffer;
           if (editingByName) {
               editText = "Name: " + editBuffer;
           }
           editText += "_";
           renderText(editText, valueX, y + 2, colors.accent);
       } else {
           SDL_Color valueColor = colors.text;
           if (currentPokemonIndex >= partyCount || partyPokemon[currentPokemonIndex].isEmpty()) {
               if (field != EditField::SPECIES) {
                   valueColor = colors.textDim;
               }
           }
           
           // Use mixed text rendering for fields that might contain Japanese
           if (isJapanese && japaneseFont && 
               (field == EditField::NICKNAME || field == EditField::OT_NAME || 
               field == EditField::SPECIES || 
               (field >= EditField::MOVE1 && field <= EditField::MOVE4))) {
               renderMixedText(fieldValue, valueX, y + 2, valueColor);
           } else {
               renderText(fieldValue, valueX, y + 2, valueColor);
           }
       }
   }
   
   if (scrollbar.canScroll()) {
       renderScrollbar();
   }
   
   // Instructions
   int instrY = startY + static_cast<int>(visibleRows) * rowH + 10;
   renderText("Up/Down: Select Field  Left/Right: Switch Pokemon  Enter: Edit  Ctrl/Cmd+S: Save", 
              10, instrY, colors.textDim);
   std::string itemInstr = (generation >= 2) ? "I: Type name for Species/Moves/Items  Q/Esc: Quit" :
                                                "I: Type name for Species/Moves  Q/Esc: Quit";
   renderText(itemInstr, 10, instrY + charHeight, colors.textDim);
   
   SDL_RenderPresent(renderer);
}

// ============================================================================
// Event handling
// ============================================================================

void PokemonPartyEditor::handleEvent(SDL_Event& event) {
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
                
                // Check field clicks
                int headerH = charHeight * 2 + 10;
                int rowH = charHeight + 4;
                int startY = headerH + 10;
                
                if (my >= startY) {
                    int local = (my - startY) / rowH;
                    if (local >= 0 && local < static_cast<int>(scrollbar.visibleItems)) {
                        int idx = static_cast<int>(scrollbar.offset) + local;
                        if (idx >= 0 && idx < static_cast<int>(EditField::FIELD_COUNT)) {
                            selectedField = idx;
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
                if (key == SDLK_UP) {
                    do {
                        selectedField--;
                        if (selectedField < 0) {
                            selectedField = static_cast<int>(EditField::FIELD_COUNT) - 1;
                        }
                    } while (!isFieldVisible(static_cast<EditField>(selectedField)));
                    requestRedraw();
                } else if (key == SDLK_DOWN) {
                    do {
                        selectedField++;
                        if (selectedField >= static_cast<int>(EditField::FIELD_COUNT)) {
                            selectedField = 0;
                        }
                    } while (!isFieldVisible(static_cast<EditField>(selectedField)));
                    requestRedraw();
                } else if (key == SDLK_LEFT) {
                    currentPokemonIndex--;
                    if (currentPokemonIndex < 0) {
                        currentPokemonIndex = MAX_PARTY_SIZE - 1;
                    }
                    requestRedraw();
                } else if (key == SDLK_RIGHT) {
                    currentPokemonIndex++;
                    if (currentPokemonIndex >= static_cast<int>(MAX_PARTY_SIZE)) {
                        currentPokemonIndex = 0;
                    }
                    requestRedraw();
                } else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                    startEditing(static_cast<EditField>(selectedField));
                } else if (key == SDLK_I) {
                    EditField field = static_cast<EditField>(selectedField);
                    if (isNameEditableField(field)) {
                        startEditing(field, true);
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
// Update
// ============================================================================

void PokemonPartyEditor::update(float deltaTime) {
    setConfirmOnQuit(hasUnsavedChanges);
    SDLAppBase::update(deltaTime);
}