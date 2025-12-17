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
    : SDLAppBase("Pokemon Party Editor", 800, 700) {
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
    } else if (g == "ruby" || g == "sapphire" || g == "pokemon_ruby" || g == "pokemon_sapphire") {
        gameType = GameType::GEN3_RS;
        generation = 3;
        gameName = "Pokemon Ruby/Sapphire";
    } else if (g == "emerald" || g == "pokemon_emerald") {
        gameType = GameType::GEN3_EMERALD;
        generation = 3;
        gameName = "Pokemon Emerald";
    } else if (g == "firered" || g == "leafgreen" || g == "pokemon_firered" || 
               g == "pokemon_leafgreen" || g == "frlg") {
        gameType = GameType::GEN3_FRLG;
        generation = 3;
        gameName = "Pokemon FireRed/LeafGreen";
    } else {
        std::cerr << "Unsupported game." << std::endl;
        std::cerr << "Supported games:" << std::endl;
        std::cerr << "  Gen 1: red, blue, yellow, green" << std::endl;
        std::cerr << "  Gen 2: gold, silver, crystal" << std::endl;
        std::cerr << "  Gen 3: ruby, sapphire, emerald, firered, leafgreen" << std::endl;
        return false;
    }

    // Set the encoding based on game and language
    setEncodingForGame();

    // Append Japanese tag if necessary
    if (isJapanese) {
        gameName += " (Japanese)";
    }

    // Initialize Gen 3 save structure if needed
    if (generation == 3) {
        if (!initGen3SaveStructure()) {
            std::cerr << "Failed to initialize Gen 3 save structure" << std::endl;
            return false;
        }
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

bool PokemonPartyEditor::initGen3SaveStructure() {
    if (!Generation3Utils::parseSaveBlocks(fileBuffer, gen3BlockA, gen3BlockB)) {
        return false;
    }
    
    activeGen3Block = Generation3Utils::getActiveSaveBlock(gen3BlockA, gen3BlockB);
    if (!activeGen3Block) {
        return false;
    }
    
    // Find Section 1 which contains the party data
    gen3Section1Offset = Generation3Utils::findSectionOffset(activeGen3Block->sections, 1);
    if (gen3Section1Offset == static_cast<size_t>(-1)) {
        std::cerr << "Could not find Section 1 in save file" << std::endl;
        return false;
    }
    
    // Calculate party offset based on game type
    if (gameType == GameType::GEN3_FRLG) {
        gen3PartyOffset = gen3Section1Offset + Generation3Utils::GEN3_PARTY_OFFSET_FRLG;
    } else {
        gen3PartyOffset = gen3Section1Offset + Generation3Utils::GEN3_PARTY_OFFSET_RSE;
    }
    
    return true;
}

int PokemonPartyEditor::getGen3GameType() const {
    switch (gameType) {
        case GameType::GEN3_RS:
            return Generation3Utils::GEN3_GAME_RS;
        case GameType::GEN3_EMERALD:
            return Generation3Utils::GEN3_GAME_EMERALD;
        case GameType::GEN3_FRLG:
            return Generation3Utils::GEN3_GAME_FRLG;
        default:
            return Generation3Utils::GEN3_GAME_RS;
    }
}

size_t PokemonPartyEditor::getPartyOffset() const {
    if (generation == 3) {
        return gen3PartyOffset;
    }
    
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
    if (generation == 3) return GEN3_POKEMON_DATA_SIZE;
    return (generation == 1) ? GEN1_POKEMON_DATA_SIZE : GEN2_POKEMON_DATA_SIZE;
}

size_t PokemonPartyEditor::getNameLength() const {
    if (generation == 3) return GEN3_NAME_LENGTH;
    return isJapanese ? NAME_LENGTH_JPN : NAME_LENGTH_ENG;
}

size_t PokemonPartyEditor::getMaxNameChars() const {
    if (generation == 3) return GEN3_NAME_LENGTH;
    return isJapanese ? 5 : 10;
}

// ============================================================================
// Party data parsing
// ============================================================================

void PokemonPartyEditor::parsePokemonData() {
    size_t partyOffset = getPartyOffset();
    
    if (generation == 3) {
        // Gen 3: Party count is in the first 4 bytes (only first byte should be used)
        partyCount = DataUtils::readU8(fileBuffer, partyOffset);
        if (partyCount > MAX_PARTY_SIZE) {
            partyCount = MAX_PARTY_SIZE;
        }
        
        // Parse each Pokemon
        size_t pokemonDataOffset = partyOffset + 4;
        for (size_t i = 0; i < MAX_PARTY_SIZE; i++) {
            PokemonData& pkmn = partyPokemon[i];
            
            if (i >= partyCount) {
                pkmn = PokemonData();
                continue;
            }
            
            size_t offset = pokemonDataOffset + (i * GEN3_POKEMON_DATA_SIZE);
            parseGen3Pokemon(pkmn, offset);
        }
        return;
    }
    
    // Gen 1/2 parsing
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
            pkmn = PokemonData();
            continue;
        }
        
        size_t offset = pokemonDataOffset + (i * pokemonDataSize);
        
        if (generation == 1) {
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
    
    pkmn.exp = (static_cast<uint32_t>(DataUtils::readU8(fileBuffer, offset + 0x08)) << 16) |
               (static_cast<uint32_t>(DataUtils::readU8(fileBuffer, offset + 0x09)) << 8) |
               static_cast<uint32_t>(DataUtils::readU8(fileBuffer, offset + 0x0A));
    
    pkmn.hpEV = DataUtils::readU16BE(fileBuffer, offset + 0x0B);
    pkmn.attackEV = DataUtils::readU16BE(fileBuffer, offset + 0x0D);
    pkmn.defenseEV = DataUtils::readU16BE(fileBuffer, offset + 0x0F);
    pkmn.speedEV = DataUtils::readU16BE(fileBuffer, offset + 0x11);
    pkmn.specialEV = DataUtils::readU16BE(fileBuffer, offset + 0x13);
    pkmn.ivData = DataUtils::readU16BE(fileBuffer, offset + 0x15);
    
    for (size_t j = 0; j < 4; j++) {
        pkmn.ppValues[j] = DataUtils::readU8(fileBuffer, offset + 0x17 + j);
    }
    
    pkmn.friendship = DataUtils::readU8(fileBuffer, offset + 0x1B);
    pkmn.pokerus = DataUtils::readU8(fileBuffer, offset + 0x1C);
    pkmn.caughtData = DataUtils::readU16BE(fileBuffer, offset + 0x1D);
    pkmn.level = DataUtils::readU8(fileBuffer, offset + 0x1F);
    pkmn.status = DataUtils::readU8(fileBuffer, offset + 0x20);
    pkmn.currentHP = DataUtils::readU16BE(fileBuffer, offset + 0x22);
    pkmn.maxHP = DataUtils::readU16BE(fileBuffer, offset + 0x24);
    pkmn.attack = DataUtils::readU16BE(fileBuffer, offset + 0x26);
    pkmn.defense = DataUtils::readU16BE(fileBuffer, offset + 0x28);
    pkmn.speed = DataUtils::readU16BE(fileBuffer, offset + 0x2A);
    pkmn.specialAttack = DataUtils::readU16BE(fileBuffer, offset + 0x2C);
    pkmn.specialDefense = DataUtils::readU16BE(fileBuffer, offset + 0x2E);
}

void PokemonPartyEditor::parseGen3Pokemon(PokemonData& pkmn, size_t offset) {
    // Read unencrypted header (bytes 0x00-0x1F)
    pkmn.personalityValue = DataUtils::readU32LE(fileBuffer, offset + Generation3Utils::POKEMON_PID_OFFSET);
    pkmn.otIdFull = DataUtils::readU32LE(fileBuffer, offset + Generation3Utils::POKEMON_OTID_OFFSET);
    
    // Store original nature for comparison when editing
    pkmn.originalNature = Generation3Utils::getNatureFromPID(pkmn.personalityValue);
    
    // Decode nickname (10 bytes at offset 0x08, terminated by 0xFF)
    std::vector<uint8_t> nicknameBytes;
    for (size_t i = 0; i < GEN3_NAME_LENGTH; i++) {
        uint8_t byte = DataUtils::readU8(fileBuffer, offset + Generation3Utils::POKEMON_NICKNAME_OFFSET + i);
        if (byte == 0xFF) break;  // Stop at terminator
        nicknameBytes.push_back(byte);
    }
    pkmn.nickname = decodeText(nicknameBytes, encoding);
    
    pkmn.language = DataUtils::readU8(fileBuffer, offset + Generation3Utils::POKEMON_LANGUAGE_OFFSET);
    pkmn.miscFlags = DataUtils::readU8(fileBuffer, offset + Generation3Utils::POKEMON_MISC_FLAGS_OFFSET);
    
    // Decode OT name (7 bytes at offset 0x14, terminated by 0xFF)
    std::vector<uint8_t> otNameBytes;
    for (size_t i = 0; i < GEN3_OT_NAME_LENGTH; i++) {
        uint8_t byte = DataUtils::readU8(fileBuffer, offset + Generation3Utils::POKEMON_OT_NAME_OFFSET + i);
        if (byte == 0xFF) break;  // Stop at terminator
        otNameBytes.push_back(byte);
    }
    pkmn.otName = decodeText(otNameBytes, encoding);
    
    pkmn.markings = DataUtils::readU8(fileBuffer, offset + Generation3Utils::POKEMON_MARKINGS_OFFSET);
    
    // Decrypt and parse the 48-byte data section
    std::array<uint8_t, 48> decryptedData = Generation3Utils::decryptPokemonData(fileBuffer, offset);
    Generation3Utils::Gen3SubstructureData subData = 
        Generation3Utils::parseSubstructureData(decryptedData, pkmn.personalityValue);
    
    // Copy substructure data to PokemonData
    pkmn.speciesGen3 = subData.species;
    pkmn.heldItemGen3 = subData.heldItem;
    pkmn.exp = subData.experience;
    pkmn.ppBonuses = subData.ppBonuses;
    pkmn.friendship = subData.friendship;
    
    for (int i = 0; i < 4; i++) {
        pkmn.movesGen3[i] = subData.moves[i];
        pkmn.ppValues[i] = subData.pp[i];
    }
    
    // EVs (Gen 3 uses 8-bit EVs, but we store in 16-bit for compatibility)
    pkmn.hpEV = subData.hpEV;
    pkmn.attackEV = subData.attackEV;
    pkmn.defenseEV = subData.defenseEV;
    pkmn.speedEV = subData.speedEV;
    pkmn.spAtkEV = subData.spAtkEV;
    pkmn.spDefEV = subData.spDefEV;
    
    // Contest stats
    pkmn.coolness = subData.coolness;
    pkmn.beauty = subData.beauty;
    pkmn.cuteness = subData.cuteness;
    pkmn.smartness = subData.smartness;
    pkmn.toughness = subData.toughness;
    pkmn.feel = subData.feel;
    
    // Miscellaneous
    pkmn.pokerus = subData.pokerus;
    pkmn.metLocation = subData.metLocation;
    pkmn.originsInfo = subData.originsInfo;
    pkmn.ivsEggAbility = subData.ivsEggAbility;
    pkmn.ribbonsObedience = subData.ribbonsObedience;
    
    // Read party-only data (bytes 0x50-0x63)
    pkmn.statusCondition = DataUtils::readU32LE(fileBuffer, offset + Generation3Utils::POKEMON_STATUS_OFFSET);
    pkmn.level = DataUtils::readU8(fileBuffer, offset + Generation3Utils::POKEMON_LEVEL_OFFSET);
    pkmn.mailId = DataUtils::readU8(fileBuffer, offset + Generation3Utils::POKEMON_MAIL_ID_OFFSET);
    pkmn.currentHP = DataUtils::readU16LE(fileBuffer, offset + Generation3Utils::POKEMON_CURRENT_HP_OFFSET);
    pkmn.maxHP = DataUtils::readU16LE(fileBuffer, offset + Generation3Utils::POKEMON_MAX_HP_OFFSET);
    pkmn.attack = DataUtils::readU16LE(fileBuffer, offset + Generation3Utils::POKEMON_ATTACK_OFFSET);
    pkmn.defense = DataUtils::readU16LE(fileBuffer, offset + Generation3Utils::POKEMON_DEFENSE_OFFSET);
    pkmn.speed = DataUtils::readU16LE(fileBuffer, offset + Generation3Utils::POKEMON_SPEED_OFFSET);
    pkmn.specialAttack = DataUtils::readU16LE(fileBuffer, offset + Generation3Utils::POKEMON_SP_ATTACK_OFFSET);
    pkmn.specialDefense = DataUtils::readU16LE(fileBuffer, offset + Generation3Utils::POKEMON_SP_DEFENSE_OFFSET);
}

// ============================================================================
// Writing data back
// ============================================================================

void PokemonPartyEditor::writePokemonDataToBuffer() {
    if (generation == 3) {
        // Write party count
        DataUtils::writeU8(fileBuffer, gen3PartyOffset, partyCount);
        DataUtils::writeU8(fileBuffer, gen3PartyOffset + 1, 0);
        DataUtils::writeU8(fileBuffer, gen3PartyOffset + 2, 0);
        DataUtils::writeU8(fileBuffer, gen3PartyOffset + 3, 0);
        
        // Write each Pokemon
        size_t pokemonDataOffset = gen3PartyOffset + 4;
        for (size_t i = 0; i < MAX_PARTY_SIZE; i++) {
            const PokemonData& pkmn = partyPokemon[i];
            size_t offset = pokemonDataOffset + (i * GEN3_POKEMON_DATA_SIZE);
            
            if (i >= partyCount || pkmn.isEmpty()) {
                // Clear empty slot
                for (size_t j = 0; j < GEN3_POKEMON_DATA_SIZE; j++) {
                    DataUtils::writeU8(fileBuffer, offset + j, 0);
                }
                continue;
            }
            
            writeGen3Pokemon(pkmn, offset);
        }
        return;
    }
    
    // Gen 1/2 writing (existing code)
    size_t partyOffset = getPartyOffset();
    size_t nameLength = getNameLength();
    size_t maxNameChars = getMaxNameChars();
    size_t pokemonDataSize = getPokemonDataSize();

    size_t secondaryPartyOffset = partyOffset;
    if (generation == 2) {
        if (gameType == GameType::GEN2_GS) {
            secondaryPartyOffset = isJapanese ? 0x7A3E : 0x10E8;
        } else if (gameType == GameType::GEN2_CRYSTAL) {
            secondaryPartyOffset = isJapanese ? 0x7A1A : 0x1A65;
        }
    }

    DataUtils::writeU8(fileBuffer, partyOffset, partyCount);

    for (size_t i = 0; i < 6; i++) {
        DataUtils::writeU8(fileBuffer, partyOffset + 1 + i, partySpecies[i]);
    }

    DataUtils::writeU8(fileBuffer, partyOffset + 7, 0xFF);

    if (generation == 2 && secondaryPartyOffset != partyOffset) {
        DataUtils::writeU8(fileBuffer, secondaryPartyOffset, partyCount);

        for (size_t i = 0; i < 6; i++) {
            DataUtils::writeU8(fileBuffer, secondaryPartyOffset + 1 + i, partySpecies[i]);
        }

        DataUtils::writeU8(fileBuffer, secondaryPartyOffset + 7, 0xFF);
    }

    size_t pokemonDataOffsetPrimary = partyOffset + 8;
    size_t pokemonDataOffsetSecondary = secondaryPartyOffset + 8;

    for (size_t i = 0; i < MAX_PARTY_SIZE; i++) {
        const PokemonData& pkmn = partyPokemon[i];

        size_t offsetPrimary = pokemonDataOffsetPrimary + (i * pokemonDataSize);
        size_t offsetSecondary = pokemonDataOffsetSecondary + (i * pokemonDataSize);

        if (i >= partyCount || pkmn.isEmpty()) {
            for (size_t j = 0; j < pokemonDataSize; j++) {
                DataUtils::writeU8(fileBuffer, offsetPrimary + j, 0);
            }

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
            writeGen2Pokemon(pkmn, offsetPrimary);

            if (secondaryPartyOffset != partyOffset) {
                writeGen2Pokemon(pkmn, offsetSecondary);
            }
        }
    }

    size_t otNamesOffsetPrimary, nicknamesOffsetPrimary;

    if (generation == 1) {
        if (isJapanese) {
            otNamesOffsetPrimary = partyOffset + 0x110;
            nicknamesOffsetPrimary = partyOffset + 0x134;
        } else {
            otNamesOffsetPrimary = partyOffset + 0x110;
            nicknamesOffsetPrimary = partyOffset + 0x152;
        }
    } else {
        size_t totalPokemonData = MAX_PARTY_SIZE * pokemonDataSize;
        otNamesOffsetPrimary = pokemonDataOffsetPrimary + totalPokemonData;
        nicknamesOffsetPrimary = otNamesOffsetPrimary + (MAX_PARTY_SIZE * nameLength);
    }

    size_t otNamesOffsetSecondary = 0, nicknamesOffsetSecondary = 0;
    bool writeSecondaryNames = (generation == 2 && secondaryPartyOffset != partyOffset);

    if (writeSecondaryNames) {
        size_t totalPokemonData = MAX_PARTY_SIZE * pokemonDataSize;
        otNamesOffsetSecondary = pokemonDataOffsetSecondary + totalPokemonData;
        nicknamesOffsetSecondary = otNamesOffsetSecondary + (MAX_PARTY_SIZE * nameLength);
    }

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
    DataUtils::writeU8(fileBuffer, offset + 0x21, 0);
    DataUtils::writeU16BE(fileBuffer, offset + 0x22, pkmn.currentHP);
    DataUtils::writeU16BE(fileBuffer, offset + 0x24, pkmn.maxHP);
    DataUtils::writeU16BE(fileBuffer, offset + 0x26, pkmn.attack);
    DataUtils::writeU16BE(fileBuffer, offset + 0x28, pkmn.defense);
    DataUtils::writeU16BE(fileBuffer, offset + 0x2A, pkmn.speed);
    DataUtils::writeU16BE(fileBuffer, offset + 0x2C, pkmn.specialAttack);
    DataUtils::writeU16BE(fileBuffer, offset + 0x2E, pkmn.specialDefense);
}

void PokemonPartyEditor::writeGen3Pokemon(const PokemonData& pkmn, size_t offset) {
    // Write unencrypted header
    DataUtils::writeU32LE(fileBuffer, offset + Generation3Utils::POKEMON_PID_OFFSET, pkmn.personalityValue);
    DataUtils::writeU32LE(fileBuffer, offset + Generation3Utils::POKEMON_OTID_OFFSET, pkmn.otIdFull);
    
    // Write nickname (10 bytes, with 0xFF terminator followed by 0x00 padding)
    // First, encode the text without any terminator
    std::vector<uint8_t> nicknameEncoded = encodeText(pkmn.nickname, encoding, GEN3_NAME_LENGTH);
    
    // Find actual length (stop at any terminator that encodeText may have added)
    size_t nicknameLen = 0;
    for (size_t i = 0; i < nicknameEncoded.size(); i++) {
        if (nicknameEncoded[i] == 0xFF || nicknameEncoded[i] == 0x50) {
            break;
        }
        nicknameLen++;
    }
    
    // Write the nickname bytes, then terminator, then zero padding
    for (size_t i = 0; i < GEN3_NAME_LENGTH; i++) {
        uint8_t value;
        if (i < nicknameLen) {
            value = nicknameEncoded[i];
        } else if (i == nicknameLen && nicknameLen != GEN3_NAME_LENGTH) {
            value = 0xFF;  // Terminator immediately after name
        } else {
            value = 0x00;  // Zero padding after terminator
        }
        DataUtils::writeU8(fileBuffer, offset + Generation3Utils::POKEMON_NICKNAME_OFFSET + i, value);
    }
    
    DataUtils::writeU8(fileBuffer, offset + Generation3Utils::POKEMON_LANGUAGE_OFFSET, pkmn.language);
    DataUtils::writeU8(fileBuffer, offset + Generation3Utils::POKEMON_MISC_FLAGS_OFFSET, pkmn.miscFlags);
    
    // Write OT name (7 bytes, with 0xFF terminator followed by 0x00 padding)
    std::vector<uint8_t> otNameEncoded = encodeText(pkmn.otName, encoding, GEN3_OT_NAME_LENGTH);
    
    // Find actual length
    size_t otNameLen = 0;
    for (size_t i = 0; i < otNameEncoded.size(); i++) {
        if (otNameEncoded[i] == 0xFF || otNameEncoded[i] == 0x50) {
            break;
        }
        otNameLen++;
    }
    
    // Write the OT name bytes, then terminator, then zero padding
    for (size_t i = 0; i < GEN3_OT_NAME_LENGTH; i++) {
        uint8_t value;
        if (i < otNameLen) {
            value = otNameEncoded[i];
        } else if (i == otNameLen && otNameLen != GEN3_OT_NAME_LENGTH) {
            value = 0xFF;  // Terminator immediately after name
        } else {
            value = 0x00;  // Zero padding after terminator
        }
        DataUtils::writeU8(fileBuffer, offset + Generation3Utils::POKEMON_OT_NAME_OFFSET + i, value);
    }
    
    DataUtils::writeU8(fileBuffer, offset + Generation3Utils::POKEMON_MARKINGS_OFFSET, pkmn.markings);
    
    // Build substructure data
    Generation3Utils::Gen3SubstructureData subData;
    subData.species = pkmn.speciesGen3;
    subData.heldItem = pkmn.heldItemGen3;
    subData.experience = pkmn.exp;
    subData.ppBonuses = pkmn.ppBonuses;
    subData.friendship = pkmn.friendship;
    
    for (int i = 0; i < 4; i++) {
        subData.moves[i] = pkmn.movesGen3[i];
        subData.pp[i] = pkmn.ppValues[i];
    }
    
    subData.hpEV = static_cast<uint8_t>(pkmn.hpEV);
    subData.attackEV = static_cast<uint8_t>(pkmn.attackEV);
    subData.defenseEV = static_cast<uint8_t>(pkmn.defenseEV);
    subData.speedEV = static_cast<uint8_t>(pkmn.speedEV);
    subData.spAtkEV = pkmn.spAtkEV;
    subData.spDefEV = pkmn.spDefEV;
    
    subData.coolness = pkmn.coolness;
    subData.beauty = pkmn.beauty;
    subData.cuteness = pkmn.cuteness;
    subData.smartness = pkmn.smartness;
    subData.toughness = pkmn.toughness;
    subData.feel = pkmn.feel;
    
    subData.pokerus = pkmn.pokerus;
    subData.metLocation = pkmn.metLocation;
    subData.originsInfo = pkmn.originsInfo;
    subData.ivsEggAbility = pkmn.ivsEggAbility;
    subData.ribbonsObedience = pkmn.ribbonsObedience;
    
    // Build and encrypt the 48-byte data section
    std::array<uint8_t, 48> plainData = Generation3Utils::buildSubstructureData(subData, pkmn.personalityValue);
    
    // Calculate and write checksum
    uint16_t checksum = Generation3Utils::calculatePokemonDataChecksum(plainData);
    DataUtils::writeU16LE(fileBuffer, offset + Generation3Utils::POKEMON_CHECKSUM_OFFSET, checksum);
    DataUtils::writeU16LE(fileBuffer, offset + 0x1E, 0); // Unknown/padding
    
    // Encrypt and write data
    Generation3Utils::encryptPokemonData(fileBuffer, offset, plainData);
    
    // Write party-only data
    DataUtils::writeU32LE(fileBuffer, offset + Generation3Utils::POKEMON_STATUS_OFFSET, pkmn.statusCondition);
    DataUtils::writeU8(fileBuffer, offset + Generation3Utils::POKEMON_LEVEL_OFFSET, pkmn.level);
    DataUtils::writeU8(fileBuffer, offset + Generation3Utils::POKEMON_MAIL_ID_OFFSET, pkmn.mailId);
    DataUtils::writeU16LE(fileBuffer, offset + Generation3Utils::POKEMON_CURRENT_HP_OFFSET, pkmn.currentHP);
    DataUtils::writeU16LE(fileBuffer, offset + Generation3Utils::POKEMON_MAX_HP_OFFSET, pkmn.maxHP);
    DataUtils::writeU16LE(fileBuffer, offset + Generation3Utils::POKEMON_ATTACK_OFFSET, pkmn.attack);
    DataUtils::writeU16LE(fileBuffer, offset + Generation3Utils::POKEMON_DEFENSE_OFFSET, pkmn.defense);
    DataUtils::writeU16LE(fileBuffer, offset + Generation3Utils::POKEMON_SPEED_OFFSET, pkmn.speed);
    DataUtils::writeU16LE(fileBuffer, offset + Generation3Utils::POKEMON_SP_ATTACK_OFFSET, pkmn.specialAttack);
    DataUtils::writeU16LE(fileBuffer, offset + Generation3Utils::POKEMON_SP_DEFENSE_OFFSET, pkmn.specialDefense);
}

// ============================================================================
// Tab and field helpers
// ============================================================================

std::string PokemonPartyEditor::getPokemonTabName(int index) const {
    if (index >= static_cast<int>(partyCount) || partyPokemon[index].isEmpty()) {
        return "(Empty)";
    }
    
    if (generation == 3) {
        const char* name = PokemonIndex::getPokemonName(partyPokemon[index].speciesGen3, generation);
        return name ? name : "???";
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
        case EditField::SP_ATK_EV: return "Sp. Atk EV";
        case EditField::SP_DEF_EV: return "Sp. Def EV";

        // Gen 1/2 DVs
        case EditField::DV_ATTACK: return "Attack DV";
        case EditField::DV_DEFENSE: return "Defense DV";
        case EditField::DV_SPEED: return "Speed DV";
        case EditField::DV_SPECIAL: return "Special DV";
        case EditField::DV_HP: return "HP DV";

        case EditField::FRIENDSHIP: return "Friendship";
        case EditField::POKERUS: return "Pokerus";
        case EditField::NICKNAME: return "Nickname";
        case EditField::OT_NAME: return "OT Name";
        case EditField::EXP: return "Experience";
        
        // Gen 3 specific
        case EditField::PID_DISPLAY: return "Personality Value";
        case EditField::SUBSTRUCTURE_ORDER: return "Data Order";
        case EditField::NATURE: return "Nature";
        case EditField::OT_ID: return "OT ID (Full)";
        case EditField::LANGUAGE: return "Language";
        case EditField::MISC_FLAGS: return "Misc. Flags";
        case EditField::MARKINGS: return "Markings";
        case EditField::IV_HP: return "HP IV";
        case EditField::IV_ATTACK: return "Attack IV";
        case EditField::IV_DEFENSE: return "Defense IV";
        case EditField::IV_SPEED: return "Speed IV";
        case EditField::IV_SP_ATK: return "Sp. Atk IV";
        case EditField::IV_SP_DEF: return "Sp. Def IV";
        case EditField::IS_EGG: return "Is Egg";
        case EditField::ABILITY_FLAG: return "Ability Slot";
        case EditField::MET_LOCATION: return "Met Location";
        case EditField::LEVEL_MET: return "Level Met";
        case EditField::GAME_OF_ORIGIN: return "Game of Origin";
        case EditField::POKEBALL: return "Poke Ball";
        case EditField::OT_GENDER: return "OT Gender";
        case EditField::COOLNESS: return "Coolness";
        case EditField::BEAUTY: return "Beauty";
        case EditField::CUTENESS: return "Cuteness";
        case EditField::SMARTNESS: return "Smartness";
        case EditField::TOUGHNESS: return "Toughness";
        case EditField::FEEL: return "Feel";
        case EditField::RIBBONS_DISPLAY: return "Ribbons";
        
        default: return "Unknown";
    }
}

bool PokemonPartyEditor::isFieldVisible(EditField field) const {
    // Gen 1 only fields
    if (generation == 1) {
        if (field == EditField::HELD_ITEM || field == EditField::SPECIAL_ATK ||
            field == EditField::SPECIAL_DEF || field == EditField::FRIENDSHIP ||
            field == EditField::POKERUS || field == EditField::SP_ATK_EV ||
            field == EditField::SP_DEF_EV) {
            return false;
        }
        // Hide all Gen 3 specific fields
        if (field >= EditField::PID_DISPLAY && field <= EditField::RIBBONS_DISPLAY) {
            return false;
        }
    }
    
    // Gen 2 fields
    if (generation == 2) {
        if (field == EditField::TYPE1 || field == EditField::TYPE2 || 
            field == EditField::SPECIAL || field == EditField::SP_ATK_EV ||
            field == EditField::SP_DEF_EV) {
            return false;
        }
        // Hide all Gen 3 specific fields
        if (field >= EditField::PID_DISPLAY && field <= EditField::RIBBONS_DISPLAY) {
            return false;
        }
    }
    
    // Gen 3 fields
    if (generation == 3) {
        // Hide Gen 1/2 only fields
        if (field == EditField::TYPE1 || field == EditField::TYPE2 ||
            field == EditField::SPECIAL || field == EditField::SPECIAL_EV) {
            return false;
        }
        // Hide Gen 1/2 DV fields (Gen 3 uses IVs instead)
        if (field == EditField::DV_ATTACK || field == EditField::DV_DEFENSE ||
            field == EditField::DV_SPEED || field == EditField::DV_SPECIAL ||
            field == EditField::DV_HP) {
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
            if (generation == 3) {
                const char* name = PokemonIndex::getPokemonName(pkmn.speciesGen3, generation);
                ss << (name ? name : "None") << " [" << HexUtils::toHexString(pkmn.speciesGen3, 4) << "]";
            } else {
                const char* name = PokemonIndex::getPokemonName(pkmn.species, generation);
                ss << (name ? name : "None") << " [" << HexUtils::toHexString(pkmn.species, 2) << "]";
            }
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
            if (generation == 3) {
                ss << getGen3StatusName(pkmn.statusCondition) << " [" << HexUtils::toHexString(pkmn.statusCondition, 8) << "]";
            } else {
                ss << getStatusName(pkmn.status) << " [" << HexUtils::toHexString(pkmn.status, 2) << "]";
            }
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
            if (generation == 3) {
                const char* itemName = getItemNameGen3(pkmn.heldItemGen3);
                ss << (itemName ? itemName : "None") << " [" << HexUtils::toHexString(pkmn.heldItemGen3, 4) << "]";
            } else {
                const char* itemName = getItemName(pkmn.heldItem);
                ss << (itemName ? itemName : "None") << " [" << HexUtils::toHexString(pkmn.heldItem, 2) << "]";
            }
            break;
        }
        case EditField::MOVE1:
        case EditField::MOVE2:
        case EditField::MOVE3:
        case EditField::MOVE4: {
            int moveIndex = static_cast<int>(field) - static_cast<int>(EditField::MOVE1);
            if (generation == 3) {
                const char* moveName = getMoveNameGen3(pkmn.movesGen3[moveIndex]);
                ss << (moveName ? moveName : "None") << " [" << HexUtils::toHexString(pkmn.movesGen3[moveIndex], 4) << "]";
            } else {
                const char* moveName = getMoveName(pkmn.moves[moveIndex]);
                ss << (moveName ? moveName : "None") << " [" << HexUtils::toHexString(pkmn.moves[moveIndex], 2) << "]";
            }
            break;
        }
        case EditField::PP1:
        case EditField::PP2:
        case EditField::PP3:
        case EditField::PP4: {
            int ppIndex = static_cast<int>(field) - static_cast<int>(EditField::PP1);
            if (generation == 3) {
                int currentPP = pkmn.ppValues[ppIndex];
                int ppUps = (pkmn.ppBonuses >> (ppIndex * 2)) & 0x03;
                ss << currentPP << " (+" << ppUps << " PP Ups)";
            } else {
                int currentPP = pkmn.ppValues[ppIndex] & 0x3F;
                int ppUps = (pkmn.ppValues[ppIndex] >> 6) & 0x03;
                ss << currentPP << " (+" << ppUps << " PP Ups)";
            }
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
        case EditField::SP_ATK_EV:
            ss << static_cast<int>(pkmn.spAtkEV);
            break;
        case EditField::SP_DEF_EV:
            ss << static_cast<int>(pkmn.spDefEV);
            break;
        case EditField::DV_ATTACK:
            ss << static_cast<int>(getIV(pkmn.ivData, "attack"));
            break;
        case EditField::DV_DEFENSE:
            ss << static_cast<int>(getIV(pkmn.ivData, "defense"));
            break;
        case EditField::DV_SPEED:
            ss << static_cast<int>(getIV(pkmn.ivData, "speed"));
            break;
        case EditField::DV_SPECIAL:
            ss << static_cast<int>(getIV(pkmn.ivData, "special"));
            break;
        case EditField::DV_HP:
            ss << static_cast<int>(getIV(pkmn.ivData, "hp")) << " (derived)";
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
            
        // Gen 3 specific fields
        case EditField::PID_DISPLAY:
            ss << HexUtils::toHexString(pkmn.personalityValue, 8) << " (not editable)";
            break;
        case EditField::SUBSTRUCTURE_ORDER:
            ss << Generation3Utils::getSubstructureOrderString(pkmn.personalityValue);
            ss << " (index " << Generation3Utils::getSubstructureOrderIndex(pkmn.personalityValue) << ")";
            break;
        case EditField::NATURE: {
            uint8_t nature = Generation3Utils::getNatureFromPID(pkmn.personalityValue);
            ss << Generation3Utils::getNatureNameByIndex(nature);
            ss << " [" << static_cast<int>(nature) << "]";
            break;
        }
        case EditField::OT_ID:
            ss << HexUtils::toHexString(pkmn.otIdFull, 8);
            ss << " (TID: " << (pkmn.otIdFull & 0xFFFF);
            ss << ", SID: " << ((pkmn.otIdFull >> 16) & 0xFFFF) << ")";
            break;
        case EditField::LANGUAGE:
            ss << Generation3Utils::getLanguageName(pkmn.language);
            ss << " [" << static_cast<int>(pkmn.language) << "]";
            break;
        case EditField::MISC_FLAGS: {
            ss << HexUtils::toHexString(pkmn.miscFlags, 2);
            if (pkmn.miscFlags & Generation3Utils::MISC_FLAG_BAD_EGG) ss << " BadEgg";
            if (pkmn.miscFlags & Generation3Utils::MISC_FLAG_HAS_SPECIES) ss << " HasSpecies";
            if (pkmn.miscFlags & Generation3Utils::MISC_FLAG_USE_EGG_NAME) ss << " UseEggName";
            if (pkmn.miscFlags & Generation3Utils::MISC_FLAG_BLOCK_BOX_RS) ss << " BlockBoxRS";
            break;
        }
        case EditField::MARKINGS:
            ss << HexUtils::toHexString(pkmn.markings, 2);
            break;
        case EditField::IV_HP:
            ss << static_cast<int>(Generation3Utils::getIVHP(pkmn.ivsEggAbility));
            break;
        case EditField::IV_ATTACK:
            ss << static_cast<int>(Generation3Utils::getIVAttack(pkmn.ivsEggAbility));
            break;
        case EditField::IV_DEFENSE:
            ss << static_cast<int>(Generation3Utils::getIVDefense(pkmn.ivsEggAbility));
            break;
        case EditField::IV_SPEED:
            ss << static_cast<int>(Generation3Utils::getIVSpeed(pkmn.ivsEggAbility));
            break;
        case EditField::IV_SP_ATK:
            ss << static_cast<int>(Generation3Utils::getIVSpAtk(pkmn.ivsEggAbility));
            break;
        case EditField::IV_SP_DEF:
            ss << static_cast<int>(Generation3Utils::getIVSpDef(pkmn.ivsEggAbility));
            break;
        case EditField::IS_EGG:
            ss << (Generation3Utils::isEgg(pkmn.ivsEggAbility) ? "Yes" : "No");
            break;
        case EditField::ABILITY_FLAG:
            ss << (Generation3Utils::hasSecondAbility(pkmn.ivsEggAbility) ? "Slot 2" : "Slot 1");
            break;
        case EditField::MET_LOCATION:
            ss << static_cast<int>(pkmn.metLocation);
            break;
        case EditField::LEVEL_MET:
            ss << static_cast<int>(Generation3Utils::getLevelMet(pkmn.originsInfo));
            break;
        case EditField::GAME_OF_ORIGIN:
            ss << Generation3Utils::getGameOfOriginName(Generation3Utils::getGameOfOrigin(pkmn.originsInfo));
            ss << " [" << static_cast<int>(Generation3Utils::getGameOfOrigin(pkmn.originsInfo)) << "]";
            break;
        case EditField::POKEBALL:
            ss << Generation3Utils::getPokeBallName(Generation3Utils::getPokeBallCaughtIn(pkmn.originsInfo));
            ss << " [" << static_cast<int>(Generation3Utils::getPokeBallCaughtIn(pkmn.originsInfo)) << "]";
            break;
        case EditField::OT_GENDER:
            ss << (Generation3Utils::getOTGender(pkmn.originsInfo) ? "Female" : "Male");
            break;
        case EditField::COOLNESS:
            ss << static_cast<int>(pkmn.coolness);
            break;
        case EditField::BEAUTY:
            ss << static_cast<int>(pkmn.beauty);
            break;
        case EditField::CUTENESS:
            ss << static_cast<int>(pkmn.cuteness);
            break;
        case EditField::SMARTNESS:
            ss << static_cast<int>(pkmn.smartness);
            break;
        case EditField::TOUGHNESS:
            ss << static_cast<int>(pkmn.toughness);
            break;
        case EditField::FEEL:
            ss << static_cast<int>(pkmn.feel);
            break;
        case EditField::RIBBONS_DISPLAY:
            ss << HexUtils::toHexString(pkmn.ribbonsObedience, 8);
            break;
            
        default:
            break;
    }
    
    return ss.str();
}

bool PokemonPartyEditor::isFieldEditable(EditField field) const {
    // PID and substructure order are display-only
    if (field == EditField::PID_DISPLAY || field == EditField::SUBSTRUCTURE_ORDER) {
        return false;
    }
    // HP DV is derived from other DVs and cannot be edited directly
    if (field == EditField::DV_HP) {
        return false;
    }
    return isFieldVisible(field);
}

bool PokemonPartyEditor::isNameEditableField(EditField field) const {
    if (field == EditField::SPECIES ||
        field == EditField::MOVE1 ||
        field == EditField::MOVE2 ||
        field == EditField::MOVE3 ||
        field == EditField::MOVE4) {
        return true;
    }
    if (generation >= 2 && field == EditField::HELD_ITEM) {
        return true;
    }
    if (generation == 3 && field == EditField::NATURE) {
        return true;
    }
    return false;
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

std::string PokemonPartyEditor::getGen3StatusName(uint32_t status) const {
    return Generation3Utils::getStatusConditionString(status);
}

const char* PokemonPartyEditor::getTypeName(uint8_t type) const {
    return PokemonTypes::getGen1TypeName(type);
}

const char* PokemonPartyEditor::getMoveName(uint8_t move) const {
    return PokemonMoves::getMoveName(move, generation);
}

const char* PokemonPartyEditor::getMoveNameGen3(uint16_t move) const {
    return PokemonMoves::getMoveName(move, 3);
}

const char* PokemonPartyEditor::getItemName(uint8_t item) const {
    if (generation == 2) {
        bool isCrystal = (gameType == GameType::GEN2_CRYSTAL);
        return ItemsIndex::getGen2ItemName(item, isCrystal);
    }
    return nullptr;
}

const char* PokemonPartyEditor::getItemNameGen3(uint16_t item) const {
    return ItemsIndex::getGen3ItemName(item);
}

// ============================================================================
// Name lookup helpers
// ============================================================================

uint8_t PokemonPartyEditor::lookupPokemonIdByName(const std::string& name) const {
    std::string query = name;
    std::transform(query.begin(), query.end(), query.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    
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

uint16_t PokemonPartyEditor::lookupPokemonIdByNameGen3(const std::string& name) const {
    std::string query = name;
    std::transform(query.begin(), query.end(), query.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    
    for (const auto& kv : PokemonIndex::GEN3_POKEMON) {
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

uint16_t PokemonPartyEditor::lookupMoveIdByNameGen3(const std::string& name) const {
    std::string query = name;
    std::transform(query.begin(), query.end(), query.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    
    for (const auto& kv : PokemonMoves::GEN3_MOVES) {
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

uint16_t PokemonPartyEditor::lookupItemIdByNameGen3(const std::string& name) const {
    std::string query = name;
    std::transform(query.begin(), query.end(), query.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    
    for (const auto& kv : ItemsIndex::GEN3_ITEMS) {
        const char* itemName = ItemsIndex::getGen3ItemName(kv.first);
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
    value &= 0x0F;
    
    if (stat == "attack") {
        return (ivData & 0x0FFF) | (static_cast<uint16_t>(value) << 12);
    } else if (stat == "defense") {
        return (ivData & 0xF0FF) | (static_cast<uint16_t>(value) << 8);
    } else if (stat == "speed") {
        return (ivData & 0xFF0F) | (static_cast<uint16_t>(value) << 4);
    } else if (stat == "special") {
        return (ivData & 0xFFF0) | value;
    }
    
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

void PokemonPartyEditor::cancelEditAndRedraw() {
    editing = false;
    editingByName = false;
    editBuffer.clear();
    requestRedraw();
}

void PokemonPartyEditor::handleEditInput(SDL_Keycode key) {
    if (!editing) return;
    
    if (key == SDLK_ESCAPE) {
        cancelEditAndRedraw();
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
    
    // Handle text input for names (including NATURE when editing by name)
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
        
        size_t maxChars = getMaxNameChars();
        if ((field == EditField::NICKNAME || field == EditField::OT_NAME) && 
            editBuffer.length() >= maxChars) {
            return;
        }
        
        editBuffer.push_back(c);
        requestRedraw();
        return;
    }
    
    // Handle numeric input for NATURE (when not editing by name)
    if (field == EditField::NATURE) {
        char c = 0;
        if (key >= SDLK_0 && key <= SDLK_9) {
            c = static_cast<char>('0' + (key - SDLK_0));
            // Limit to 2 digits (0-24)
            if (editBuffer.length() < 2) {
                editBuffer.push_back(c);
                requestRedraw();
            }
        }
        return;
    }

    // Handle numeric input for DV fields (0-15, so limit to 2 digits)
    if (field == EditField::DV_ATTACK || field == EditField::DV_DEFENSE ||
        field == EditField::DV_SPEED || field == EditField::DV_SPECIAL) {
        char c = 0;
        if (key >= SDLK_0 && key <= SDLK_9) {
            c = static_cast<char>('0' + (key - SDLK_0));
            // Limit to 2 digits (0-15)
            if (editBuffer.length() < 2) {
                editBuffer.push_back(c);
                requestRedraw();
            }
        }
        return;
    }
       
    
    // Handle numeric/hex input for other fields
    char c = 0;
    if (key >= SDLK_0 && key <= SDLK_9) {
        c = static_cast<char>('0' + (key - SDLK_0));
    } else if (key >= SDLK_A && key <= SDLK_F) {
        // Allow hex input for appropriate fields
        bool allowHex = (field == EditField::SPECIES || field == EditField::STATUS ||
                         field == EditField::TYPE1 || field == EditField::TYPE2 ||
                         field == EditField::MOVE1 || field == EditField::MOVE2 ||
                         field == EditField::MOVE3 || field == EditField::MOVE4 ||
                         field == EditField::HELD_ITEM || field == EditField::OT_ID ||
                         field == EditField::MISC_FLAGS || field == EditField::MARKINGS ||
                         field == EditField::RIBBONS_DISPLAY || field == EditField::POKERUS);
        if (allowHex) {
            c = static_cast<char>('A' + (key - SDLK_A));
        } else {
            return;
        }
    } else {
        return;
    }
    
    editBuffer.push_back(c);
    requestRedraw();
}

void PokemonPartyEditor::commitEdit() {
    if (!editing) return;
    
    EditField field = static_cast<EditField>(selectedField);

    auto cancel = [&] {
        cancelEditAndRedraw();
        return;
    };
    
    // Handle name-based editing
    if (editingByName && !editBuffer.empty()) {
        if (field == EditField::SPECIES) {
            if (generation == 3) {
                uint16_t id = lookupPokemonIdByNameGen3(editBuffer);
                if (id != 0) {
                    editBuffer = HexUtils::toHexString(id, 4);
                } else {
                    return cancel();
                }
            } else {
                uint8_t id = lookupPokemonIdByName(editBuffer);
                if (id != 0) {
                    editBuffer = HexUtils::toHexString(id, 2);
                } else {
                    return cancel();
                }
            }
        } else if (field >= EditField::MOVE1 && field <= EditField::MOVE4) {
            if (generation == 3) {
                uint16_t id = lookupMoveIdByNameGen3(editBuffer);
                if (id != 0 || editBuffer == "-" || editBuffer == "NONE") {
                    editBuffer = HexUtils::toHexString(id, 4);
                } else {
                    return cancel();
                }
            } else {
                uint8_t id = lookupMoveIdByName(editBuffer);
                if (id != 0 || editBuffer == "-" || editBuffer == "NONE") {
                    editBuffer = HexUtils::toHexString(id, 2);
                } else {
                    return cancel();
                }
            }
        } else if (field == EditField::HELD_ITEM) {
            if (generation == 3) {
                uint16_t id = lookupItemIdByNameGen3(editBuffer);
                if (id != 0 || editBuffer == "-" || editBuffer == "NONE") {
                    editBuffer = HexUtils::toHexString(id, 4);
                } else {
                    return cancel();
                }
            } else if (generation >= 2) {
                uint8_t id = lookupItemIdByName(editBuffer);
                if (id != 0 || editBuffer == "-" || editBuffer == "NONE") {
                    editBuffer = HexUtils::toHexString(id, 2);
                } else {
                    return cancel();
                }
            }
        } else if (field == EditField::NATURE && generation == 3) {
            uint8_t natureId = Generation3Utils::lookupNatureByName(editBuffer);
            if (natureId != 255) {
                editBuffer = std::to_string(natureId);
            } else {
                return cancel();
            }
        }
    }
    
    // Special handling for NATURE field - show confirmation dialog if nature changed
    if (field == EditField::NATURE && generation == 3) {
        PokemonData& pkmn = partyPokemon[currentPokemonIndex];
        uint8_t currentNature = Generation3Utils::getNatureFromPID(pkmn.personalityValue);
        
        uint8_t newNature;
        try {
            newNature = static_cast<uint8_t>(std::stoul(editBuffer));
        } catch (...) {
            return cancel();
        }
        
        if (newNature >= 25) {
            return cancel();
        }
        
        // Only show dialog if nature is actually changing
        if (newNature != currentNature) {
            if (!showOverwriteSensitiveDataConfirmDialog()) {
                // User cancelled - revert
                return cancel();
            }
            
            // User confirmed - apply the change
            pkmn.personalityValue = Generation3Utils::calculatePIDForNature(pkmn.personalityValue, newNature);
            hasUnsavedChanges = true;
        }
        
        return cancel();
    }
    
    if (validateAndApplyEdit(currentPokemonIndex, field, editBuffer)) {
        hasUnsavedChanges = true;
    }
    
    cancelEditAndRedraw();
}

bool PokemonPartyEditor::validateAndApplyEdit(int pokemonIndex, EditField field, 
                                               const std::string& value) {
    if (pokemonIndex < 0 || static_cast<size_t>(pokemonIndex) >= MAX_PARTY_SIZE) return false;
    
    PokemonData& pkmn = partyPokemon[pokemonIndex];
    
    try {
        switch (field) {
            case EditField::SPECIES: {
                if (generation == 3) {
                    uint16_t species = static_cast<uint16_t>(std::stoul(value, nullptr, 16));
                    pkmn.speciesGen3 = species;
                    
                    // Update misc flags
                    if (species != 0) {
                        pkmn.miscFlags |= Generation3Utils::MISC_FLAG_HAS_SPECIES;
                    } else {
                        pkmn.miscFlags &= ~Generation3Utils::MISC_FLAG_HAS_SPECIES;
                    }
                    
                    if (species != 0 && pokemonIndex >= partyCount) {
                        partyCount = pokemonIndex + 1;
                    }
                } else {
                    uint8_t species = static_cast<uint8_t>(std::stoul(value, nullptr, 16));
                    pkmn.species = species;
                    partySpecies[pokemonIndex] = species;
                    
                    if (generation == 1) {
                        const PokemonIndex::PokemonInfo* info = PokemonIndex::getPokemonInfo(species, 1);
                        if (info) {
                            pkmn.type1 = info->type1;
                            pkmn.type2 = info->type2;
                        }
                    }
                    
                    if (species != 0 && species != 0xFF && pokemonIndex >= partyCount) {
                        partyCount = pokemonIndex + 1;
                    }
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
                if (generation == 3) {
                    pkmn.statusCondition = static_cast<uint32_t>(std::stoul(value, nullptr, 16));
                } else {
                    pkmn.status = static_cast<uint8_t>(std::stoul(value, nullptr, 16));
                }
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
                if (generation == 3) {
                    pkmn.heldItemGen3 = static_cast<uint16_t>(std::stoul(value, nullptr, 16));
                } else if (generation >= 2) {
                    pkmn.heldItem = static_cast<uint8_t>(std::stoul(value, nullptr, 16));
                }
                break;
            case EditField::MOVE1:
            case EditField::MOVE2:
            case EditField::MOVE3:
            case EditField::MOVE4: {
                int moveIndex = static_cast<int>(field) - static_cast<int>(EditField::MOVE1);
                if (generation == 3) {
                    pkmn.movesGen3[moveIndex] = static_cast<uint16_t>(std::stoul(value, nullptr, 16));
                } else {
                    pkmn.moves[moveIndex] = static_cast<uint8_t>(std::stoul(value, nullptr, 16));
                }
                break;
            }
            case EditField::PP1:
            case EditField::PP2:
            case EditField::PP3:
            case EditField::PP4: {
                int ppIndex = static_cast<int>(field) - static_cast<int>(EditField::PP1);
                uint8_t pp = static_cast<uint8_t>(std::stoul(value));
                if (generation == 3) {
                    if (pp > 63) pp = 63;
                    pkmn.ppValues[ppIndex] = pp;
                } else {
                    if (pp > 63) pp = 63;
                    pkmn.ppValues[ppIndex] = (pkmn.ppValues[ppIndex] & 0xC0) | pp;
                }
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
                pkmn.specialAttack = static_cast<uint16_t>(std::stoul(value));
                if (pkmn.specialAttack > 999) pkmn.specialAttack = 999;
                break;
            case EditField::SPECIAL_DEF:
                pkmn.specialDefense = static_cast<uint16_t>(std::stoul(value));
                if (pkmn.specialDefense > 999) pkmn.specialDefense = 999;
                break;
            case EditField::HP_EV:
                if (generation == 3) {
                    uint8_t ev = static_cast<uint8_t>(std::stoul(value));
                    pkmn.hpEV = ev;
                } else {
                    pkmn.hpEV = static_cast<uint16_t>(std::stoul(value));
                }
                break;
            case EditField::ATTACK_EV:
                if (generation == 3) {
                    uint8_t ev = static_cast<uint8_t>(std::stoul(value));
                    pkmn.attackEV = ev;
                } else {
                    pkmn.attackEV = static_cast<uint16_t>(std::stoul(value));
                }
                break;
            case EditField::DEFENSE_EV:
                if (generation == 3) {
                    uint8_t ev = static_cast<uint8_t>(std::stoul(value));
                    pkmn.defenseEV = ev;
                } else {
                    pkmn.defenseEV = static_cast<uint16_t>(std::stoul(value));
                }
                break;
            case EditField::SPEED_EV:
                if (generation == 3) {
                    uint8_t ev = static_cast<uint8_t>(std::stoul(value));
                    pkmn.speedEV = ev;
                } else {
                    pkmn.speedEV = static_cast<uint16_t>(std::stoul(value));
                }
                break;
            case EditField::SPECIAL_EV:
                pkmn.specialEV = static_cast<uint16_t>(std::stoul(value));
                break;
            case EditField::SP_ATK_EV:
                if (generation == 3) {
                    pkmn.spAtkEV = static_cast<uint8_t>(std::stoul(value));
                }
                break;
            case EditField::SP_DEF_EV:
                if (generation == 3) {
                    pkmn.spDefEV = static_cast<uint8_t>(std::stoul(value));
                }
                break;
            case EditField::DV_ATTACK:
                if (generation <= 2) {
                    uint8_t dv = static_cast<uint8_t>(std::stoul(value));
                    if (dv > 15) dv = 15;
                    pkmn.ivData = setIV(pkmn.ivData, "attack", dv);
                }
                break;
            case EditField::DV_DEFENSE:
                if (generation <= 2) {
                    uint8_t dv = static_cast<uint8_t>(std::stoul(value));
                    if (dv > 15) dv = 15;
                    pkmn.ivData = setIV(pkmn.ivData, "defense", dv);
                }
                break;
            case EditField::DV_SPEED:
                if (generation <= 2) {
                    uint8_t dv = static_cast<uint8_t>(std::stoul(value));
                    if (dv > 15) dv = 15;
                    pkmn.ivData = setIV(pkmn.ivData, "speed", dv);
                }
                break;
            case EditField::DV_SPECIAL:
                if (generation <= 2) {
                    uint8_t dv = static_cast<uint8_t>(std::stoul(value));
                    if (dv > 15) dv = 15;
                    pkmn.ivData = setIV(pkmn.ivData, "special", dv);
                }
                break;
            case EditField::FRIENDSHIP:
                pkmn.friendship = static_cast<uint8_t>(std::stoul(value));
                break;
            case EditField::POKERUS:
                pkmn.pokerus = static_cast<uint8_t>(std::stoul(value, nullptr, 16));
                break;
            case EditField::NICKNAME:
                pkmn.nickname = value;
                break;
            case EditField::OT_NAME:
                pkmn.otName = value;
                break;
            case EditField::EXP: {
                uint32_t exp = static_cast<uint32_t>(std::stoul(value));
                if (generation == 3) {
                    // Gen 3 uses full 32-bit EXP
                } else {
                    if (exp > 0xFFFFFF) exp = 0xFFFFFF;
                }
                pkmn.exp = exp;
                break;
            }
            
            // Gen 3 specific fields
            case EditField::OT_ID:
                if (generation == 3) {
                    pkmn.otIdFull = static_cast<uint32_t>(std::stoul(value, nullptr, 16));
                }
                break;
            case EditField::LANGUAGE:
                if (generation == 3) {
                    pkmn.language = static_cast<uint8_t>(std::stoul(value));
                    if (pkmn.language > 7) pkmn.language = 2; // Default to English
                }
                break;
            case EditField::MISC_FLAGS:
                if (generation == 3) {
                    pkmn.miscFlags = static_cast<uint8_t>(std::stoul(value, nullptr, 16));
                }
                break;
            case EditField::MARKINGS:
                if (generation == 3) {
                    pkmn.markings = static_cast<uint8_t>(std::stoul(value, nullptr, 16));
                }
                break;
            case EditField::IV_HP:
                if (generation == 3) {
                    uint8_t iv = static_cast<uint8_t>(std::stoul(value));
                    if (iv > 31) iv = 31;
                    pkmn.ivsEggAbility = Generation3Utils::setIVHP(pkmn.ivsEggAbility, iv);
                }
                break;
            case EditField::IV_ATTACK:
                if (generation == 3) {
                    uint8_t iv = static_cast<uint8_t>(std::stoul(value));
                    if (iv > 31) iv = 31;
                    pkmn.ivsEggAbility = Generation3Utils::setIVAttack(pkmn.ivsEggAbility, iv);
                }
                break;
            case EditField::IV_DEFENSE:
                if (generation == 3) {
                    uint8_t iv = static_cast<uint8_t>(std::stoul(value));
                    if (iv > 31) iv = 31;
                    pkmn.ivsEggAbility = Generation3Utils::setIVDefense(pkmn.ivsEggAbility, iv);
                }
                break;
            case EditField::IV_SPEED:
                if (generation == 3) {
                    uint8_t iv = static_cast<uint8_t>(std::stoul(value));
                    if (iv > 31) iv = 31;
                    pkmn.ivsEggAbility = Generation3Utils::setIVSpeed(pkmn.ivsEggAbility, iv);
                }
                break;
            case EditField::IV_SP_ATK:
                if (generation == 3) {
                    uint8_t iv = static_cast<uint8_t>(std::stoul(value));
                    if (iv > 31) iv = 31;
                    pkmn.ivsEggAbility = Generation3Utils::setIVSpAtk(pkmn.ivsEggAbility, iv);
                }
                break;
            case EditField::IV_SP_DEF:
                if (generation == 3) {
                    uint8_t iv = static_cast<uint8_t>(std::stoul(value));
                    if (iv > 31) iv = 31;
                    pkmn.ivsEggAbility = Generation3Utils::setIVSpDef(pkmn.ivsEggAbility, iv);
                }
                break;
            case EditField::IS_EGG:
                if (generation == 3) {
                    bool isEggFlag = (value == "1" || value == "Y" || value == "YES");
                    pkmn.ivsEggAbility = Generation3Utils::setEggFlag(pkmn.ivsEggAbility, isEggFlag);
                    if (isEggFlag) {
                        pkmn.miscFlags |= Generation3Utils::MISC_FLAG_USE_EGG_NAME;
                    } else {
                        pkmn.miscFlags &= ~Generation3Utils::MISC_FLAG_USE_EGG_NAME;
                    }
                }
                break;
            case EditField::ABILITY_FLAG:
                if (generation == 3) {
                    bool secondAbility = (value == "1" || value == "2");
                    pkmn.ivsEggAbility = Generation3Utils::setAbilityFlag(pkmn.ivsEggAbility, secondAbility);
                }
                break;
            case EditField::MET_LOCATION:
                if (generation == 3) {
                    pkmn.metLocation = static_cast<uint8_t>(std::stoul(value));
                }
                break;
            case EditField::LEVEL_MET:
                if (generation == 3) {
                    uint8_t lvl = static_cast<uint8_t>(std::stoul(value));
                    if (lvl > 127) lvl = 127;
                    pkmn.originsInfo = Generation3Utils::setLevelMet(pkmn.originsInfo, lvl);
                }
                break;
            case EditField::GAME_OF_ORIGIN:
                if (generation == 3) {
                    uint8_t game = static_cast<uint8_t>(std::stoul(value));
                    if (game > 15) game = 0;
                    pkmn.originsInfo = Generation3Utils::setGameOfOrigin(pkmn.originsInfo, game);
                }
                break;
            case EditField::POKEBALL:
                if (generation == 3) {
                    uint8_t ball = static_cast<uint8_t>(std::stoul(value));
                    if (ball > 15) ball = 4; // Default to Poke Ball
                    pkmn.originsInfo = Generation3Utils::setPokeBallCaughtIn(pkmn.originsInfo, ball);
                }
                break;
            case EditField::OT_GENDER:
                if (generation == 3) {
                    bool female = (value == "1" || value == "F" || value == "FEMALE");
                    pkmn.originsInfo = Generation3Utils::setOTGender(pkmn.originsInfo, female);
                }
                break;
            case EditField::COOLNESS:
                if (generation == 3) {
                    pkmn.coolness = static_cast<uint8_t>(std::stoul(value));
                }
                break;
            case EditField::BEAUTY:
                if (generation == 3) {
                    pkmn.beauty = static_cast<uint8_t>(std::stoul(value));
                }
                break;
            case EditField::CUTENESS:
                if (generation == 3) {
                    pkmn.cuteness = static_cast<uint8_t>(std::stoul(value));
                }
                break;
            case EditField::SMARTNESS:
                if (generation == 3) {
                    pkmn.smartness = static_cast<uint8_t>(std::stoul(value));
                }
                break;
            case EditField::TOUGHNESS:
                if (generation == 3) {
                    pkmn.toughness = static_cast<uint8_t>(std::stoul(value));
                }
                break;
            case EditField::FEEL:
                if (generation == 3) {
                    pkmn.feel = static_cast<uint8_t>(std::stoul(value));
                }
                break;
            case EditField::RIBBONS_DISPLAY:
                if (generation == 3) {
                    pkmn.ribbonsObedience = static_cast<uint32_t>(std::stoul(value, nullptr, 16));
                }
                break;
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
    } else if (generation == 3) {
        updateChecksumGen3();
    }
}

void PokemonPartyEditor::updateChecksumGen1() {
    Generation1Utils::ChecksumConfig config = Generation1Utils::getRedBlueYellowConfig(isJapanese);
    
    if (config.end >= fileSize || config.checksumLocation >= fileSize) return;
    
    uint8_t checksum = Generation1Utils::calculate8BitChecksum(fileBuffer, config.start, config.end);
    DataUtils::writeU8(fileBuffer, config.checksumLocation, checksum);
}

void PokemonPartyEditor::updateChecksumGen2() {
    bool crystal = (gameType == GameType::GEN2_CRYSTAL);
    
    Generation2Utils::ChecksumConfig config;
    if (crystal) {
        config = Generation2Utils::getCrystalConfig(isJapanese);
    } else {
        config = Generation2Utils::getGoldSilverConfig(isJapanese);
    }
    
    uint16_t checksum1 = Generation2Utils::calculate16BitChecksum(
        fileBuffer, config.start1, config.end1);
    
    if (config.checksumLocation1 + 1 < fileSize) {
        DataUtils::writeU16LE(fileBuffer, config.checksumLocation1, checksum1);
    }
    
    uint16_t checksum2 = Generation2Utils::calculate16BitChecksumMultiRange(
        fileBuffer, config.ranges2);
    
    if (config.checksumLocation2 + 1 < fileSize) {
        DataUtils::writeU16LE(fileBuffer, config.checksumLocation2, checksum2);
    }
}

void PokemonPartyEditor::updateChecksumGen3() {
    if (!activeGen3Block) return;
    
    // Update Section 1 checksum (where party data is stored)
    size_t section1DataSize = Generation3Utils::GEN3_SECTION_SIZES[1];
    Generation3Utils::updateSectionChecksum(fileBuffer, gen3Section1Offset, section1DataSize);
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
   std::unordered_map<int, size_t> fieldToVisibleIndex;
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
       
       // Mark non-editable fields
       if (!isFieldEditable(field)) {
           fieldName += " (view)";
       }
       
       // Field name
       renderText(fieldName + ":", rowRect.x + 5, y + 2, colors.text);
       
       // Field value
       int valueX = rowRect.x + 220;  // Increased to accommodate longer field names
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
                
                int headerH = charHeight * 2 + 10;
                int rowH = charHeight + 4;
                int startY = headerH + 10;
                
                if (my >= startY) {
                    int local = (my - startY) / rowH;
                    if (local >= 0 && local < static_cast<int>(scrollbar.visibleItems)) {
                        int idx = static_cast<int>(scrollbar.offset) + local;
                        
                        // Map back from visible index to actual field
                        std::vector<EditField> visibleFields;
                        for (int i = 0; i < static_cast<int>(EditField::FIELD_COUNT); i++) {
                            EditField field = static_cast<EditField>(i);
                            if (isFieldVisible(field)) {
                                visibleFields.push_back(field);
                            }
                        }
                        
                        if (idx >= 0 && idx < static_cast<int>(visibleFields.size())) {
                            selectedField = static_cast<int>(visibleFields[idx]);
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