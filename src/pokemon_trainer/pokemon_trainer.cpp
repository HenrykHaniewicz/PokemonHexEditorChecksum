//=============================================================================
//  pokemon_trainer.cpp - Refactored with helper functions
//=============================================================================

#include "pokemon_trainer.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <filesystem>

#include "../encodings/pokemon_index_eng.h"
#include "../encodings/moves_index_eng.h"
#include "../encodings/items_index_eng.h"

//=============================================================================
//  Constructor
//=============================================================================

PokemonTrainerEditor::PokemonTrainerEditor()
    : SDLAppBase("Pokemon Trainer Editor", 1200, 640) {
    scrollbar.headerOffset = 60;
}

std::string PokemonTrainerEditor::decodeTrainerName(const std::vector<unsigned char>& bytes) const {
    TextEncoding enc = isJapanese ? TextEncoding::JP_G3 : TextEncoding::EN_G3;
    return decodeText(bytes, enc, 0xFF);
}

bool PokemonTrainerEditor::isGen1Game() const {
    return gameType == GameType::GEN1_RB || gameType == GameType::GEN1_YELLOW;
}

bool PokemonTrainerEditor::isGen2Game() const {
    return gameType == GameType::GEN2_GS || gameType == GameType::GEN2_CRYSTAL;
}

bool PokemonTrainerEditor::isGen3Game() const {
    return gameType == GameType::GEN3_RS ||
           gameType == GameType::GEN3_EMERALD ||
           gameType == GameType::GEN3_FRLG;
}

//=============================================================================
//  Helper: fetch the class name associated with a given classId
//=============================================================================

std::string PokemonTrainerEditor::getTrainerClassName(uint8_t classId) const {
    if (isGen1Game()) {
        if (classId < gen1ClassNames.size()) {
            return gen1ClassNames[classId];
        }
        return std::string("Class ") + std::to_string(classId);
    }
    if (isGen2Game()) {
        if (classId < gen2ClassNames.size()) {
            return gen2ClassNames[classId];
        }
        return std::string("Class ") + std::to_string(classId);
    }
    if (isGen3Game()) {
        const size_t baseOffset = static_cast<size_t>(trainer3Addresses.classNames);
        size_t maxNameLength = isJapanese ? 11 : 13;
        size_t offset = baseOffset + (static_cast<size_t>(classId) * maxNameLength);
        if (offset + maxNameLength > fileBuffer.size()) {
            return std::string("Class ") + std::to_string(classId);
        }
        std::vector<unsigned char> bytes;
        bytes.reserve(maxNameLength);
        for (size_t i = 0; i < maxNameLength; i++) {
            bytes.push_back(static_cast<unsigned char>(fileBuffer[offset + i]));
        }
        TextEncoding enc = isJapanese ? TextEncoding::JP_G3 : TextEncoding::EN_G3;
        std::string decoded = decodeText(bytes, enc, 0xFF);
        if (decoded.empty()) {
            decoded = std::string("Class ") + std::to_string(classId);
        }
        return decoded;
    }
    return std::string("???");
}

//=============================================================================
//  Load the ROM into memory
//=============================================================================

bool PokemonTrainerEditor::loadFile(const char* filename) {
    if (!HexUtils::loadFileToBuffer(filename, fileBuffer, fileSize)) {
        std::cerr << "Failed to open: " << filename << std::endl;
        return false;
    }
    fileName = filename;
    hasUnsavedChanges = false;
    return true;
}

//=============================================================================
//  Determine the game being edited.
//=============================================================================

bool PokemonTrainerEditor::setGame(const std::string& game) {
    std::string g = game;
    transform(g.begin(), g.end(), g.begin(), [](unsigned char c) {
        return static_cast<char>(tolower(c));
    });
    g.erase(remove_if(g.begin(), g.end(), ::isspace), g.end());

    gameType = GameType::UNKNOWN;
    gameName.clear();

    if (g == "ruby" || g == "pokemonruby" || g == "rubyversion") {
        gameType = GameType::GEN3_RS;
        gameName = "Pokemon Ruby";
        if (isJapanese) {
            trainer3Addresses = Generation3Utils::TRAINER_ADDRESSES_RUBY_J;
        } else {
            trainer3Addresses = Generation3Utils::TRAINER_ADDRESSES_RUBY;
        }
    } else if (g == "sapphire" || g == "pokemonsapphire" || g == "sapphireversion") {
        gameType = GameType::GEN3_RS;
        gameName = "Pokemon Sapphire";
        if (isJapanese) {
            trainer3Addresses = Generation3Utils::TRAINER_ADDRESSES_SAPPHIRE_J;
        } else {
            trainer3Addresses = Generation3Utils::TRAINER_ADDRESSES_SAPPHIRE;
        }
    } else if (g == "emerald") {
        gameType = GameType::GEN3_EMERALD;
        gameName = "Pokemon Emerald";
        if (isJapanese) {
            trainer3Addresses = Generation3Utils::TRAINER_ADDRESSES_EMERALD_J;
        } else {
            trainer3Addresses = Generation3Utils::TRAINER_ADDRESSES_EMERALD;
        }
    } else if (g == "firered" || g == "fireredversion") {
        gameType = GameType::GEN3_FRLG;
        gameName = "Pokemon FireRed";
        if (isJapanese) {
            trainer3Addresses = Generation3Utils::TRAINER_ADDRESSES_FIRERED_J;
        } else {
            trainer3Addresses = Generation3Utils::TRAINER_ADDRESSES_FIRERED;
        }
    } else if (g == "leafgreen" || g == "leafgreenversion") {
        gameType = GameType::GEN3_FRLG;
        gameName = "Pokemon LeafGreen";
        if (isJapanese) {
            trainer3Addresses = Generation3Utils::TRAINER_ADDRESSES_LEAFGREEN_J;
        } else {
            trainer3Addresses = Generation3Utils::TRAINER_ADDRESSES_LEAFGREEN;
        }
    } else if (g == "yellow" || g == "pokemonyellow" || g == "yellowversion") {
        gameType = GameType::GEN1_YELLOW;
        gameName = "Pokemon Yellow";
        if (isJapanese) {
            trainer1Addresses = Generation1Utils::TRAINER1_ADDRESSES_YELLOW_J;
        } else {
            trainer1Addresses = Generation1Utils::TRAINER1_ADDRESSES_YELLOW;
        }
    } else if (g == "red" || g == "pokemonred" || g == "redversion" ||
               g == "blue" || g == "pokemonblue" || g == "blueversion" ||
               g == "green" || g == "pokemongreen" || g == "greenversion") {
        gameType = GameType::GEN1_RB;
        if (g.find("red") != std::string::npos) {
            gameName = "Pokemon Red";
        } else if (g.find("blue") != std::string::npos) {
            gameName = "Pokemon Blue";
        } else if (g.find("green") != std::string::npos) {
            gameName = "Pokemon Green";
        } else {
            gameName = "Pokemon Red/Blue";
        }
        if (isJapanese) {
            if (g.find("blue") != std::string::npos) {
                trainer1Addresses = Generation1Utils::TRAINER1_ADDRESSES_Blue_J;
            } else {
                trainer1Addresses = Generation1Utils::TRAINER1_ADDRESSES_RG_J;
            }
        } else {
            trainer1Addresses = Generation1Utils::TRAINER1_ADDRESSES_RB;
        }
    } else if (g == "gold" || g == "pokemongold" || g == "goldversion" ||
               g == "silver" || g == "pokemonsilver" || g == "silverversion") {
        gameType = GameType::GEN2_GS;
        if (g.find("gold") != std::string::npos) {
            gameName = "Pokemon Gold";
        } else if (g.find("silver") != std::string::npos) {
            gameName = "Pokemon Silver";
        } else {
            gameName = "Pokemon Gold/Silver";
        }
        if (isJapanese) {
            trainer2Addresses = Generation2Utils::TRAINER2_ADDRESSES_GS_J;
        } else {
            trainer2Addresses = Generation2Utils::TRAINER2_ADDRESSES_GS;
        }
    } else if (g == "crystal" || g == "pokemoncrystal" || g == "crystalversion") {
        gameType = GameType::GEN2_CRYSTAL;
        gameName = "Pokemon Crystal";
        if (isJapanese) {
            trainer2Addresses = Generation2Utils::TRAINER2_ADDRESSES_CRYSTAL_J;
        } else {
            trainer2Addresses = Generation2Utils::TRAINER2_ADDRESSES_CRYSTAL;
        }
    } else {
        std::cerr << "Unsupported or unknown game: " << game << std::endl;
        return false;
    }

    bool ok = parseTrainers();
    refreshDisplayOrder();
    requestRedraw();
    return ok;
}

//=============================================================================
//  Parse all trainer records from the ROM
//=============================================================================

bool PokemonTrainerEditor::parseTrainers() {
    trainers.clear();

    if (isGen1Game()) {
        return parseGen1Trainers();
    }
    if (isGen2Game()) {
        return parseGen2Trainers();
    }
    if (isGen3Game()) {
        return parseGen3Trainers();
    }
    return false;
}

bool PokemonTrainerEditor::parseGen1Trainers() {
    gen1ClassNames.clear();
    if (trainer1Addresses.classNamesStart == 0 || trainer1Addresses.pointerStart == 0) {
        return false;
    }
    size_t pos = static_cast<size_t>(trainer1Addresses.classNamesStart);
    size_t end = static_cast<size_t>(trainer1Addresses.classNamesEnd);
    if (end > fileBuffer.size()) end = fileBuffer.size();
    std::vector<std::string> allClasses;
    while (pos < end) {
        std::vector<unsigned char> bytes;
        while (pos < end && static_cast<unsigned char>(fileBuffer[pos]) != 0x50) {
            bytes.push_back(static_cast<unsigned char>(fileBuffer[pos]));
            pos++;
        }
        if (pos < end && static_cast<unsigned char>(fileBuffer[pos]) == 0x50) {
            pos++;
        }

        TextEncoding enc = isJapanese ? TextEncoding::JP_G1 : TextEncoding::EN_G1;
        std::string cls = decodeText(bytes, enc, 0x50);
        if (cls.empty()) {
            cls = std::string("Class ") + std::to_string(allClasses.size());
        }
        allClasses.push_back(cls);
    }

    std::vector<uint32_t> rawPointers;
    size_t pStart = static_cast<size_t>(trainer1Addresses.pointerStart);
    size_t pEnd = static_cast<size_t>(trainer1Addresses.pointerEnd);
    if (pEnd + 1 > fileBuffer.size()) pEnd = (fileBuffer.empty() ? 0 : fileBuffer.size() - 1);
    if (pEnd < pStart) return false;
    for (size_t off = pStart; off + 1 <= pEnd; off += 2) {
        uint16_t val = DataUtils::readU16LE(fileBuffer, off);
        uint32_t addr = static_cast<uint32_t>(val) + Generation1Utils::TRAINER1_POINTER_BASE;
        rawPointers.push_back(addr);
    }
    std::sort(rawPointers.begin(), rawPointers.end());

    std::vector<std::string> usedClasses;
    std::vector<uint32_t> usedPointers;
    size_t count = std::min(allClasses.size(), rawPointers.size());
    for (size_t i = 0; i < count; i++) {
        std::string cls = allClasses[i];
        std::string upperCls = cls;
        std::transform(upperCls.begin(), upperCls.end(), upperCls.begin(), [](unsigned char c){ return static_cast<char>(::toupper(c)); });
        if (upperCls == "PROF.OAK") {
            continue;
        }
        usedClasses.push_back(cls);
        if (i < rawPointers.size()) {
            usedPointers.push_back(rawPointers[i]);
        }
    }
    gen1ClassNames = usedClasses;

    std::vector<uint32_t> regionEnds;
    for (size_t i = 0; i < usedPointers.size(); i++) {
        uint32_t endAddr;
        if (i + 1 < usedPointers.size()) {
            endAddr = usedPointers[i + 1] - 1;
        } else {
            endAddr = trainer1Addresses.lastTrainerByte;
        }
        regionEnds.push_back(endAddr);
    }

    size_t maxTrainerCount = 2000;
    size_t trainerIdx = 0;
    for (size_t ci = 0; ci < usedPointers.size() && trainerIdx < maxTrainerCount; ci++) {
        uint32_t startAddr = usedPointers[ci];
        uint32_t endAddr = regionEnds[ci];
        if (startAddr >= fileBuffer.size()) continue;
        if (endAddr >= fileBuffer.size()) endAddr = static_cast<uint32_t>(fileBuffer.size() - 1);
        size_t offset = static_cast<size_t>(startAddr);
        while (offset <= endAddr && trainerIdx < maxTrainerCount) {
            if (static_cast<unsigned char>(fileBuffer[offset]) == 0x00) {
                offset++;
                continue;
            }
            bool format2 = false;
            size_t cur = offset;
            if (static_cast<unsigned char>(fileBuffer[cur]) == 0xFF) {
                format2 = true;
                cur++;
            }
            std::vector<Gen1PokemonInfo> party;
            if (!format2) {
                size_t levelOffset = offset;
                size_t speciesPtr = offset + 1;
                while (speciesPtr <= endAddr && static_cast<unsigned char>(fileBuffer[speciesPtr]) != 0x00) {
                    Gen1PokemonInfo info;
                    info.levelOffset = levelOffset;
                    info.speciesOffset = speciesPtr;
                    party.push_back(info);
                    speciesPtr++;
                }
                cur = speciesPtr + 1;
            } else {
                size_t ptr = offset + 1;
                while (ptr + 1 <= endAddr && static_cast<unsigned char>(fileBuffer[ptr]) != 0x00) {
                    Gen1PokemonInfo info;
                    info.levelOffset = ptr;
                    info.speciesOffset = ptr + 1;
                    party.push_back(info);
                    ptr += 2;
                }
                cur = ptr + 1;
            }
            if (cur > endAddr + 1) {
                cur = endAddr + 1;
            }
            TrainerEntry entry;
            entry.offset = offset;
            entry.type = static_cast<uint8_t>(format2 ? 1 : 0);
            entry.classId = static_cast<uint8_t>(ci);
            entry.name.clear();
            entry.partySize = static_cast<uint32_t>(party.size());
            entry.gen1.format2 = format2;
            entry.gen1.party = party;
            trainers.push_back(entry);
            trainerIdx++;
            offset = cur;
        }
    }
    return true;
}

bool PokemonTrainerEditor::parseGen2Trainers() {
    gen2ClassNames.clear();
    if (trainer2Addresses.classNamesStart == 0 || trainer2Addresses.pointerStart == 0) {
        return false;
    }

    static const std::vector<unsigned char> profOakClassBytes{
        0x54, 0x8C, 0x8E, 0x8D, 0x7F, 0x8F, 0x91, 0x8E, 0x85, 0xE8
    };

    std::vector<std::string> allClassNames;
    std::vector<std::vector<unsigned char>> allClassNameBytes;
    size_t pos = static_cast<size_t>(trainer2Addresses.classNamesStart);
    size_t end = static_cast<size_t>(trainer2Addresses.classNamesEnd);
    if (end > fileBuffer.size()) end = fileBuffer.size();
    while (pos < end) {
        std::vector<unsigned char> bytes;
        while (pos < end && static_cast<unsigned char>(fileBuffer[pos]) != 0x50) {
            bytes.push_back(static_cast<unsigned char>(fileBuffer[pos]));
            pos++;
        }
        if (pos < end && static_cast<unsigned char>(fileBuffer[pos]) == 0x50) {
            pos++;
        }
        TextEncoding classEnc = isJapanese ? TextEncoding::JP_G2 : TextEncoding::EN_G2;
        std::string cls = decodeText(bytes, classEnc, 0x50);
        if (cls.empty()) {
            cls = std::string("Class ") + std::to_string(allClassNames.size());
        }
        allClassNames.push_back(cls);
        allClassNameBytes.push_back(bytes);
    }

    std::vector<uint32_t> rawPointers;
    size_t pStart = static_cast<size_t>(trainer2Addresses.pointerStart);
    size_t pEnd = static_cast<size_t>(trainer2Addresses.pointerEnd);
    if (pEnd + 1 > fileBuffer.size()) pEnd = (fileBuffer.empty() ? 0 : fileBuffer.size() - 1);
    if (pEnd < pStart) return false;

    const uint32_t bankBase = static_cast<uint32_t>(pStart & ~static_cast<size_t>(0x3FFF));
    for (size_t off = pStart; off + 1 <= pEnd; off += 2) {
        uint16_t val = DataUtils::readU16LE(fileBuffer, off);
        uint32_t addr = (val >= 0x4000)
            ? bankBase + static_cast<uint32_t>(val - 0x4000)
            : static_cast<uint32_t>(val);
        rawPointers.push_back(addr);
    }

    std::vector<uint32_t> sortedPointers = rawPointers;
    std::sort(sortedPointers.begin(), sortedPointers.end());

    std::vector<uint32_t> usedPointers;
    const size_t count = std::min(allClassNames.size(), sortedPointers.size());
    for (size_t i = 0; i < count; i++) {
        if (allClassNameBytes[i] == profOakClassBytes) {
            continue;
        }
        gen2ClassNames.push_back(allClassNames[i]);
        usedPointers.push_back(sortedPointers[i]);
    }

    std::vector<uint32_t> regionEnds;
    for (size_t i = 0; i < usedPointers.size(); i++) {
        uint32_t endAddr = (i + 1 < usedPointers.size())
            ? usedPointers[i + 1] - 1
            : trainer2Addresses.lastTrainerByte;
        regionEnds.push_back(endAddr);
    }

    const size_t maxTrainerCount = 2000;
    size_t trainerIdx = 0;
    for (size_t ci = 0; ci < usedPointers.size() && trainerIdx < maxTrainerCount; ci++) {
        uint32_t startAddr = usedPointers[ci];
        uint32_t endAddr = regionEnds[ci];
        if (startAddr >= fileBuffer.size()) continue;
        if (endAddr >= fileBuffer.size()) endAddr = static_cast<uint32_t>(fileBuffer.size() - 1);

        size_t offset = static_cast<size_t>(startAddr);
        while (offset <= endAddr && trainerIdx < maxTrainerCount) {
            if (static_cast<unsigned char>(fileBuffer[offset]) == 0xFF) {
                offset++;
                continue;
            }

            std::vector<unsigned char> nameBytes;
            size_t cur = offset;
            bool foundNameEnd = false;
            for (int i = 0; cur <= endAddr && i < 10; i++) {
                unsigned char b = static_cast<unsigned char>(fileBuffer[cur]);
                cur++;
                if (b == 0x50) {
                    foundNameEnd = true;
                    break;
                }
                nameBytes.push_back(b);
            }
            if (!foundNameEnd || cur > endAddr) break;

            uint8_t structType = static_cast<uint8_t>(fileBuffer[cur]);
            cur++;
            if (structType > 0x03) break;

            std::vector<Gen2PokemonInfo> party;
            bool malformed = false;
            while (cur <= endAddr && static_cast<unsigned char>(fileBuffer[cur]) != 0xFF) {
                if (cur + 1 > endAddr) {
                    malformed = true;
                    break;
                }

                Gen2PokemonInfo info;
                info.levelOffset = cur;
                info.speciesOffset = cur + 1;
                cur += 2;

                if (structType == 0x02 || structType == 0x03) {
                    if (cur > endAddr) {
                        malformed = true;
                        break;
                    }
                    info.hasItem = true;
                    info.itemOffset = cur;
                    cur++;
                }
                if (structType == 0x01 || structType == 0x03) {
                    if (cur + 3 > endAddr) {
                        malformed = true;
                        break;
                    }
                    info.hasMoves = true;
                    for (int m = 0; m < 4; m++) {
                        info.moveOffsets[m] = cur;
                        cur++;
                    }
                }

                party.push_back(info);
            }

            if (malformed) break;
            if (cur <= endAddr && static_cast<unsigned char>(fileBuffer[cur]) == 0xFF) {
                cur++;
            }
            if (cur <= offset) break;

            TextEncoding encName = isJapanese ? TextEncoding::JP_G2 : TextEncoding::EN_G2;
            std::string trainerName = decodeText(nameBytes, encName, 0x50);
            if (trainerName.empty()) {
                trainerName = std::string("Trainer ") + std::to_string(trainerIdx);
            }

            TrainerEntry entry;
            entry.offset = offset;
            entry.type = structType;
            entry.classId = static_cast<uint8_t>(ci);
            entry.name = trainerName;
            entry.partySize = static_cast<uint32_t>(party.size());
            entry.gen2.party = party;
            trainers.push_back(entry);

            trainerIdx++;
            offset = cur;
        }
    }

    return true;
}

bool PokemonTrainerEditor::parseGen3Trainers() {
    const size_t baseOffset = static_cast<size_t>(trainer3Addresses.trainerDataStart);
    const size_t tableEndOffset = static_cast<size_t>(trainer3Addresses.trainerDataEnd);
    const size_t recordSize = isJapanese ? 0x20 : 0x28;
    size_t offset = baseOffset;
    size_t recordCount = 0;
    static const size_t maxRecords = 2000;

    size_t maxTableOffset = (fileBuffer.size() < tableEndOffset) ? fileBuffer.size() : tableEndOffset;
    while (offset + recordSize <= maxTableOffset && recordCount < maxRecords) {
        TrainerEntry entry;
        entry.offset = offset;
        entry.type = DataUtils::readU8(fileBuffer, offset + 0x00);
        entry.classId = DataUtils::readU8(fileBuffer, offset + 0x01);
        entry.gen3.flags = DataUtils::readU8(fileBuffer, offset + 0x02);
        entry.gen3.sprite = DataUtils::readU8(fileBuffer, offset + 0x03);

        const size_t nameLen = isJapanese ? 6 : 12;
        const size_t nameStart = 0x04;
        const size_t itemsOffset = isJapanese ? 0x0A : 0x10;
        const size_t unknownOffset = isJapanese ? 0x12 : 0x18;
        const size_t unknownCount = isJapanese ? 2 : 4;
        const size_t aiOffset = isJapanese ? 0x14 : 0x1C;
        const size_t partySizeOffset = isJapanese ? 0x18 : 0x20;
        const size_t pointerOffset = isJapanese ? 0x1C : 0x24;

        std::vector<unsigned char> nameBytes;
        nameBytes.reserve(nameLen);
        for (size_t i = 0; i < nameLen && offset + nameStart + i < fileBuffer.size(); i++) {
            nameBytes.push_back(static_cast<unsigned char>(fileBuffer[offset + nameStart + i]));
        }
        entry.name = decodeTrainerName(nameBytes);

        for (size_t i = 0; i < 4; i++) {
            entry.gen3.items[i] = DataUtils::readU16LE(fileBuffer, offset + itemsOffset + (i * 2));
        }

        for (size_t i = 0; i < 4; i++) {
            if (i < unknownCount) {
                entry.gen3.unknown[i] = DataUtils::readU8(fileBuffer, offset + unknownOffset + i);
            } else {
                entry.gen3.unknown[i] = 0;
            }
        }

        entry.gen3.ai = DataUtils::readU32LE(fileBuffer, offset + aiOffset);
        entry.partySize = DataUtils::readU32LE(fileBuffer, offset + partySizeOffset);
        entry.gen3.partyPointer = DataUtils::readU32LE(fileBuffer, offset + pointerOffset);
        if (entry.gen3.partyPointer >= Generation3Utils::GEN3_ROM_ABSOLUTE_ADDRESS) {
            uint32_t romOffset = entry.gen3.partyPointer - Generation3Utils::GEN3_ROM_ABSOLUTE_ADDRESS;
            entry.gen3.partyOffset = (romOffset < fileBuffer.size()) ? romOffset : 0;
        } else {
            entry.gen3.partyOffset = 0;
        }

        trainers.push_back(entry);
        recordCount++;
        offset += recordSize;

        bool emptyName = entry.name.empty();
        bool noPointer = (entry.gen3.partyPointer == 0);
        bool noItems = true;
        for (uint16_t it : entry.gen3.items) {
            if (it != 0) {
                noItems = false;
                break;
            }
        }
        if (emptyName && noPointer && entry.type == 0 && entry.classId == 0 && noItems) {
            break;
        }
    }
    return true;
}

//=============================================================================
//  Build the display list based on current filter and sort settings
//=============================================================================

void PokemonTrainerEditor::refreshDisplayOrder() {
    displayOrder.clear();
    displayOrder.reserve(trainers.size());

    for (size_t i = 0; i < trainers.size(); i++) {
        if (!searchTerm.empty()) {
            std::string nameLower = trainers[i].name;
            std::string termLower = searchTerm;
            transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            if (nameLower.find(termLower) == std::string::npos) {
                continue;
            }
        }
        displayOrder.push_back(i);
    }

    if (sortMode == SortMode::Class || sortMode == SortMode::Name) {
        sort(displayOrder.begin(), displayOrder.end(), [&](size_t a, size_t b) {
            const TrainerEntry& ta = trainers[a];
            const TrainerEntry& tb = trainers[b];
            if (sortMode == SortMode::Class) {
                std::string classA = getTrainerClassName(ta.classId);
                std::string classB = getTrainerClassName(tb.classId);
                std::string ca = classA;
                std::string cb = classB;
                transform(ca.begin(), ca.end(), ca.begin(), ::tolower);
                transform(cb.begin(), cb.end(), cb.begin(), ::tolower);
                if (ca != cb) return ca < cb;
            }

            std::string na = ta.name;
            std::string nb = tb.name;
            transform(na.begin(), na.end(), na.begin(), ::tolower);
            transform(nb.begin(), nb.end(), nb.begin(), ::tolower);
            return na < nb;
        });
    }

    if (selectedIndex >= displayOrder.size()) {
        selectedIndex = (displayOrder.empty() ? 0 : displayOrder.size() - 1);
    }

    scrollbar.totalItems = displayOrder.size();
}

//=============================================================================
//  Allocate a new party memory region and update pointer
//=============================================================================

void PokemonTrainerEditor::allocateNewParty(TrainerEntry& entry, uint32_t newPartySize) {
    size_t perSize = ((entry.type == 0) || (entry.type == 2)) ? 8 : 16;
    size_t oldSize = static_cast<size_t>(entry.partySize) * perSize;
    size_t newSize = static_cast<size_t>(newPartySize) * perSize;

    std::vector<std::pair<size_t,size_t>> used;
    used.reserve(trainers.size());
    for (const auto& tr : trainers) {
        if (tr.gen3.partyPointer == 0 || tr.gen3.partyPointer < Generation3Utils::GEN3_ROM_ABSOLUTE_ADDRESS) continue;
        size_t start = tr.gen3.partyOffset;
        size_t size = static_cast<size_t>(tr.partySize) * ((tr.type == 0 || tr.type == 2) ? 8 : 16);
        used.push_back({start, start + size});
    }

    size_t base = 0xEB0000;
    size_t candidate = base;
    auto overlaps = [&](size_t start, size_t end) {
        for (const auto& u : used) {
            if (start < u.second && end > u.first) {
                return true;
            }
        }
        return false;
    };
    while (overlaps(candidate, candidate + 100)) {
        candidate += 100;
    }

    size_t required = candidate + std::max(newSize, static_cast<size_t>(100));
    if (required > fileBuffer.size()) {
        fileBuffer.resize(required, 0);
        fileSize = fileBuffer.size();
    }

    size_t copyBytes = (oldSize < newSize ? oldSize : newSize);
    if (copyBytes > 0 && entry.gen3.partyOffset + copyBytes <= fileBuffer.size()) {
        if (candidate != entry.gen3.partyOffset) {
            memmove(&fileBuffer[candidate], &fileBuffer[entry.gen3.partyOffset], copyBytes);
        }
    }

    if (newSize > copyBytes) {
        memset(&fileBuffer[candidate + copyBytes], 0, newSize - copyBytes);
    }

    uint32_t newPointer = static_cast<uint32_t>(candidate + Generation3Utils::GEN3_ROM_ABSOLUTE_ADDRESS);
    size_t pointerOffset = isJapanese ? 0x1C : 0x24;
    size_t sizeOffset    = isJapanese ? 0x18 : 0x20;
    DataUtils::writeU32LE(fileBuffer, entry.offset + pointerOffset, newPointer);
    DataUtils::writeU32LE(fileBuffer, entry.offset + sizeOffset, newPartySize);
    entry.gen3.partyPointer = newPointer;
    entry.gen3.partyOffset = candidate;
    entry.partySize = newPartySize;

    DataUtils::writeU8(fileBuffer, entry.offset + 0x00, entry.type);
    hasUnsavedChanges = true;
}

//=============================================================================
//  Determine the output path for saving edits
//=============================================================================

std::string PokemonTrainerEditor::getOutputPath() const {
    std::string baseName = HexUtils::getBaseName(fileName);
    if (overwriteMode) {
        return fileName;
    }
    return std::string("edited_files/") + baseName;
}

//=============================================================================
//  Write fileBuffer to disk
//=============================================================================

bool PokemonTrainerEditor::writeFile(const std::string& path) {
    if (!overwriteMode) {
        std::filesystem::create_directories("edited_files");
    }
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        std::cerr << "Failed to save to " << path << std::endl;
        return false;
    }
    out.write(fileBuffer.data(), static_cast<std::streamsize>(fileBuffer.size()));
    out.close();
    hasUnsavedChanges = false;
    setConfirmOnQuit(false);  // Clear the quit confirmation flag when saved
    requestRedraw();  // Request immediate redraw to update [MODIFIED] indicator
    std::cout << "File saved to " << path << std::endl;
    return true;
}

//=============================================================================
//  Rendering function
//=============================================================================

void PokemonTrainerEditor::buildGen1TrainerFields(const TrainerEntry& trainer) {
    for (size_t pi = 0; pi < trainer.gen1.party.size(); ++pi) {
        std::string base = "P" + std::to_string(pi + 1) + " ";
        fields.push_back({FieldKind::PokemonLevel, pi, 0, base + "Level"});
        fields.push_back({FieldKind::PokemonSpecies, pi, 0, base + "Species"});
    }
}

void PokemonTrainerEditor::buildGen2TrainerFields(const TrainerEntry& trainer) {
    for (size_t pi = 0; pi < trainer.gen2.party.size(); ++pi) {
        const auto& pokemon = trainer.gen2.party[pi];
        std::string base = "P" + std::to_string(pi + 1) + " ";
        fields.push_back({FieldKind::PokemonLevel, pi, 0, base + "Level"});
        fields.push_back({FieldKind::PokemonSpecies, pi, 0, base + "Species"});
        if (pokemon.hasItem) {
            fields.push_back({FieldKind::PokemonItem, pi, 0, base + "Item"});
        }
        if (pokemon.hasMoves) {
            for (size_t moveIndex = 0; moveIndex < 4; ++moveIndex) {
                fields.push_back({FieldKind::PokemonMove, pi, moveIndex, base + "Move" + std::to_string(moveIndex + 1)});
            }
        }
    }
}

void PokemonTrainerEditor::buildGen3TrainerFields(const TrainerEntry& trainer) {
    fields.push_back({FieldKind::Flags, 0, 0, "Flags"});
    fields.push_back({FieldKind::Sprite, 0, 0, "Sprite"});
    fields.push_back({FieldKind::AI, 0, 0, "AI"});
    for (size_t i = 0; i < 4; ++i) {
        fields.push_back({FieldKind::TrainerItem, 0, i, "Item " + std::to_string(i + 1)});
    }

    size_t perSize = ((trainer.type == 0) || (trainer.type == 2)) ? 8 : 16;
    for (uint32_t pi = 0; pi < trainer.partySize && trainer.gen3.partyOffset + (pi + 1) * perSize <= fileBuffer.size(); ++pi) {
        std::string base = "P" + std::to_string(pi + 1) + " ";
        fields.push_back({FieldKind::PokemonLevel, pi, 0, base + "Level"});
        fields.push_back({FieldKind::PokemonSpecies, pi, 0, base + "Species"});

        if (trainer.type == 0) {
            continue;
        }
        if (trainer.type == 2) {
            fields.push_back({FieldKind::PokemonItem, pi, 0, base + "Item"});
            continue;
        }
        if (trainer.type == 3) {
            fields.push_back({FieldKind::PokemonItem, pi, 0, base + "Item"});
            for (size_t moveIndex = 0; moveIndex < 4; ++moveIndex) {
                fields.push_back({FieldKind::PokemonMove, pi, moveIndex, base + "Move" + std::to_string(moveIndex + 1)});
            }
            continue;
        }
        for (size_t moveIndex = 0; moveIndex < 4; ++moveIndex) {
            fields.push_back({FieldKind::PokemonMove, pi, moveIndex, base + "Move" + std::to_string(moveIndex + 1)});
        }
        if (trainer.type != 1) {
            fields.push_back({FieldKind::PokemonItem, pi, 0, base + "Item"});
        }
    }
}

void PokemonTrainerEditor::buildTrainerFields(const TrainerEntry& trainer) {
    fields.clear();
    fields.push_back({FieldKind::Name, 0, 0, "Name"});
    fields.push_back({FieldKind::Class, 0, 0, "Class"});
    fields.push_back({FieldKind::Type, 0, 0, "Type"});
    fields.push_back({FieldKind::PartySize, 0, 0, "Party Size"});

    if (isGen1Game()) {
        buildGen1TrainerFields(trainer);
    } else if (isGen2Game()) {
        buildGen2TrainerFields(trainer);
    } else if (isGen3Game()) {
        buildGen3TrainerFields(trainer);
    }
}

std::string PokemonTrainerEditor::getGen1FieldValueString(const TrainerEntry& tr, const FieldDescriptor& fd, size_t fieldIndex) const {
    switch (fd.kind) {
        case FieldKind::PokemonLevel: {
            uint8_t lvl = 0;
            if (fd.pokemonIndex < tr.gen1.party.size()) {
                size_t off = tr.gen1.party[fd.pokemonIndex].levelOffset;
                if (off < fileBuffer.size()) lvl = DataUtils::readU8(fileBuffer, off);
            }
            return (editingValue && fieldIndex == selectedField) ? editBuffer + "_" : std::to_string(static_cast<unsigned>(lvl));
        }
        case FieldKind::PokemonSpecies: {
            uint8_t species = 0;
            if (fd.pokemonIndex < tr.gen1.party.size()) {
                size_t off = tr.gen1.party[fd.pokemonIndex].speciesOffset;
                if (off < fileBuffer.size()) species = DataUtils::readU8(fileBuffer, off);
            }
            if (editingValue && fieldIndex == selectedField) return editBuffer + "_";
            const char* name = PokemonIndex::getGen1PokemonName(species);
            if (name && name[0] != '\0') {
                return std::string(name);
            }
            return "0x" + HexUtils::toHexString(static_cast<size_t>(species), 2);
        }
        default:
            break;
    }
    return {};
}

std::string PokemonTrainerEditor::getGen2FieldValueString(const TrainerEntry& tr, const FieldDescriptor& fd, size_t fieldIndex) const {
    switch (fd.kind) {
        case FieldKind::PokemonLevel: {
            uint8_t lvl = 0;
            if (fd.pokemonIndex < tr.gen2.party.size()) {
                size_t off = tr.gen2.party[fd.pokemonIndex].levelOffset;
                if (off < fileBuffer.size()) lvl = DataUtils::readU8(fileBuffer, off);
            }
            return (editingValue && fieldIndex == selectedField) ? editBuffer + "_" : std::to_string(static_cast<unsigned>(lvl));
        }
        case FieldKind::PokemonSpecies: {
            uint16_t species = 0;
            if (fd.pokemonIndex < tr.gen2.party.size()) {
                size_t off = tr.gen2.party[fd.pokemonIndex].speciesOffset;
                if (off < fileBuffer.size()) species = static_cast<uint16_t>(DataUtils::readU8(fileBuffer, off));
            }
            if (editingValue && fieldIndex == selectedField) return editBuffer + "_";
            const char* name = PokemonIndex::getGen3PokemonName(species);
            return (name && name[0] != '\0') ? std::string(name) : "0x" + HexUtils::toHexString(static_cast<size_t>(species), 2);
        }
        case FieldKind::PokemonItem: {
            uint16_t item = 0;
            if (fd.pokemonIndex < tr.gen2.party.size()) {
                const auto& pinfo = tr.gen2.party[fd.pokemonIndex];
                if (pinfo.hasItem && pinfo.itemOffset < fileBuffer.size()) {
                    item = static_cast<uint16_t>(DataUtils::readU8(fileBuffer, pinfo.itemOffset));
                }
            }
            if (editingValue && fieldIndex == selectedField) return editBuffer + "_";
            if (item == 0) return "None";
            const char* name = ItemsIndex::getGen3ItemName(item);
            std::string hex = "0x" + HexUtils::toHexString(static_cast<size_t>(item), 2);
            return (name && name[0] != '\0') ? std::string(name) + " (" + hex + ")" : hex;
        }
        case FieldKind::PokemonMove: {
            uint16_t move = 0;
            if (fd.pokemonIndex < tr.gen2.party.size()) {
                const auto& pinfo = tr.gen2.party[fd.pokemonIndex];
                if (pinfo.hasMoves && fd.subIndex < 4) {
                    size_t off = pinfo.moveOffsets[fd.subIndex];
                    if (off < fileBuffer.size()) move = static_cast<uint16_t>(DataUtils::readU8(fileBuffer, off));
                }
            }
            if (editingValue && fieldIndex == selectedField) return editBuffer + "_";
            if (move == 0) return "--";
            const char* name = PokemonMoves::getMoveName(move, 3);
            return (name && name[0] != '\0') ? std::string(name) : "0x" + HexUtils::toHexString(static_cast<size_t>(move), 2);
        }
        default:
            break;
    }
    return {};
}

std::string PokemonTrainerEditor::getGen3FieldValueString(const TrainerEntry& tr, const FieldDescriptor& fd, size_t fieldIndex) const {
    switch (fd.kind) {
        case FieldKind::Flags:
            return (editingValue && fieldIndex == selectedField) ? editBuffer + "_" : "0x" + HexUtils::toHexString(static_cast<size_t>(tr.gen3.flags), 2);
        case FieldKind::Sprite:
            return (editingValue && fieldIndex == selectedField) ? editBuffer + "_" : "0x" + HexUtils::toHexString(static_cast<size_t>(tr.gen3.sprite), 2);
        case FieldKind::AI:
            return (editingValue && fieldIndex == selectedField) ? editBuffer + "_" : std::to_string(tr.gen3.ai);
        case FieldKind::TrainerItem: {
            uint16_t item = (fd.subIndex < 4) ? tr.gen3.items[fd.subIndex] : 0;
            if (editingValue && fieldIndex == selectedField) return editBuffer + "_";
            if (item == 0) return "None";
            const char* name = ItemsIndex::getGen3ItemName(item);
            std::string hex = "0x" + HexUtils::toHexString(item, 4);
            return (name && name[0] != '\0') ? std::string(name) + " (" + hex + ")" : hex;
        }
        case FieldKind::PokemonLevel: {
            size_t perSz = ((tr.type == 0) || (tr.type == 2)) ? 8 : 16;
            size_t poff = tr.gen3.partyOffset + fd.pokemonIndex * perSz;
            uint8_t lvl = (poff + 3 < fileBuffer.size()) ? DataUtils::readU8(fileBuffer, poff + 0x02) : 0;
            return (editingValue && fieldIndex == selectedField) ? editBuffer + "_" : std::to_string(static_cast<unsigned>(lvl));
        }
        case FieldKind::PokemonSpecies: {
            size_t perSz = ((tr.type == 0) || (tr.type == 2)) ? 8 : 16;
            size_t poff = tr.gen3.partyOffset + fd.pokemonIndex * perSz;
            uint16_t species = (poff + 5 < fileBuffer.size()) ? DataUtils::readU16LE(fileBuffer, poff + 0x04) : 0;
            if (editingValue && fieldIndex == selectedField) return editBuffer + "_";
            const char* name = PokemonIndex::getGen3PokemonName(species);
            return (name && name[0] != '\0') ? std::string(name) : "0x" + HexUtils::toHexString(species, 4);
        }
        case FieldKind::PokemonItem: {
            size_t perSz = ((tr.type == 0) || (tr.type == 2)) ? 8 : 16;
            size_t poff = tr.gen3.partyOffset + fd.pokemonIndex * perSz;
            uint16_t item = 0;
            if (tr.type == 0 || tr.type == 2 || tr.type == 3) {
                if (poff + 7 < fileBuffer.size()) item = DataUtils::readU16LE(fileBuffer, poff + 0x06);
            } else {
                if (poff + 0x0F < fileBuffer.size()) item = DataUtils::readU16LE(fileBuffer, poff + 0x0E);
            }
            if (editingValue && fieldIndex == selectedField) return editBuffer + "_";
            if (item == 0) return "None";
            const char* name = ItemsIndex::getGen3ItemName(item);
            std::string hex = "0x" + HexUtils::toHexString(item, 4);
            return (name && name[0] != '\0') ? std::string(name) + " (" + hex + ")" : hex;
        }
        case FieldKind::PokemonMove: {
            size_t perSz = ((tr.type == 0) || (tr.type == 2)) ? 8 : 16;
            size_t poff = tr.gen3.partyOffset + fd.pokemonIndex * perSz;
            size_t moveOffset = (tr.type == 3) ? (poff + 0x08 + fd.subIndex * 2) : (poff + 0x06 + fd.subIndex * 2);
            uint16_t move = (moveOffset + 1 < fileBuffer.size()) ? DataUtils::readU16LE(fileBuffer, moveOffset) : 0;
            if (editingValue && fieldIndex == selectedField) return editBuffer + "_";
            if (move == 0) return "--";
            const char* name = PokemonMoves::getMoveName(move, 3);
            return (name && name[0] != '\0') ? std::string(name) : "0x" + HexUtils::toHexString(move, 4);
        }
        default:
            break;
    }
    return {};
}

std::string PokemonTrainerEditor::getFieldValueString(const TrainerEntry& tr, const FieldDescriptor& fd, size_t fieldIndex) const {
    switch (fd.kind) {
        case FieldKind::Name:
            return (editingValue && fieldIndex == selectedField) ? editBuffer + "_" : tr.name;
        case FieldKind::Class:
            if (editingValue && fieldIndex == selectedField) return editBuffer + "_";
            return getTrainerClassName(tr.classId) + " (0x" + HexUtils::toHexString(static_cast<size_t>(tr.classId), 2) + ")";
        case FieldKind::Type:
            return (editingValue && fieldIndex == selectedField) ? editBuffer + "_" : "0x" + HexUtils::toHexString(static_cast<size_t>(tr.type), 2);
        case FieldKind::PartySize:
            return (editingValue && fieldIndex == selectedField) ? editBuffer + "_" : std::to_string(tr.partySize);
        default:
            break;
    }
    if (isGen1Game()) return getGen1FieldValueString(tr, fd, fieldIndex);
    if (isGen2Game()) return getGen2FieldValueString(tr, fd, fieldIndex);
    if (isGen3Game()) return getGen3FieldValueString(tr, fd, fieldIndex);
    return {};
}

void PokemonTrainerEditor::render() {
    renderFilledRect({0, 0, windowWidth, windowHeight}, colors.background);

    int headerHeight = charHeight * 2 + 10;
    renderFilledRect({0, 0, windowWidth, headerHeight}, colors.headerBg);
    std::string headerLabel = "Pokemon Trainer Editor";
    if (!gameName.empty()) {
        headerLabel += " - " + gameName;
    }
    if (overwriteMode) {
        headerLabel += " [OVERWRITE]";
    }
    if (hasUnsavedChanges) {
        headerLabel += " [MODIFIED]";
    }

    SDL_Color headerColor = colors.text;
    if (overwriteMode) {
        headerColor = colors.warning;
    } else if (hasUnsavedChanges) {
        headerColor = colors.error;
    }

    renderText(headerLabel, 10, 5, headerColor);

    if (searchMode) {
        std::string searchLabel = "Search: " + searchTerm;
        if (isJapanese && japaneseFont) {
            renderMixedText(searchLabel, 10, 5 + charHeight, colors.accent);
        } else {
            renderText(searchLabel, 10, 5 + charHeight, colors.accent, font);
        }
    } else {
        std::string sortLabel;
        switch (sortMode) {
            case SortMode::Memory: sortLabel = "Sorting: Memory"; break;
            case SortMode::Class:  sortLabel = "Sorting: Class"; break;
            case SortMode::Name:   sortLabel = "Sorting: Name"; break;
        }
        std::string infoText = sortLabel + "   Press S to search, T to toggle sort, Enter to view details, I to edit by name, Ctrl+S to save";
        if (isJapanese && japaneseFont) {
            renderMixedText(infoText, 10, 5 + charHeight, colors.textDim);
        } else {
            renderText(infoText, 10, 5 + charHeight, colors.textDim, font);
        }
    }

    int listX = 0;
    int listY = headerHeight + 10;
    int listWidth = 300;
    int listHeight = windowHeight - listY - 10;
    int rightX = listX + listWidth + scrollbar.width + 10;
    int rightWidth = windowWidth - rightX - 10;

    int rowHeight = charHeight + 4;
    size_t visibleRows = (listHeight / rowHeight);
    scrollbar.visibleItems = visibleRows;
    scrollbar.totalItems = displayOrder.size();
    scrollbar.headerOffset = listY;

    renderFilledRect({listX, listY, listWidth, static_cast<int>(visibleRows * rowHeight)}, colors.dialogBg);
    for (size_t i = 0; i < visibleRows; i++) {
        size_t index = scrollbar.offset + i;
        if (index >= displayOrder.size()) break;
        const TrainerEntry& tr = trainers[displayOrder[index]];
        int y = listY + static_cast<int>(i * rowHeight);
        if (!viewingDetails && index == selectedIndex) {
            renderFilledRect({listX, y, listWidth, rowHeight}, colors.selectedBg);
        }
        std::string display = getTrainerClassName(tr.classId) + ": " + tr.name;
        int textW = 0, textH = 0;
        getTextSize(display, textW, textH, font);
        while (textW > listWidth - 10 && !display.empty()) {
            display.pop_back();
            getTextSize(display + "…", textW, textH, font);
            if (textW <= listWidth - 10) {
                display += "…";
                break;
            }
        }

        if (isJapanese && japaneseFont) {
            renderMixedText(display, listX + 5, y + 2, colors.text);
        } else {
            renderText(display, listX + 5, y + 2, colors.text, font);
        }
    }

    {
        int sbX = listX + listWidth - scrollbar.width;
        int sbY = listY;
        int sbHeight = listHeight;

        setScrollbarArea(sbX, sbY, sbHeight, &scrollbar);
        renderScrollbar();
        resetScrollbarArea();
    }

    if (!displayOrder.empty()) {
        TrainerEntry& tr = trainers[displayOrder[selectedIndex]];
        buildTrainerFields(tr);

        int detailsY = listY;
        int detailsRowHeight = charHeight + 6;
        int detailsHeight = windowHeight - detailsY - 10;
        size_t visibleDetailRows = (detailsHeight / detailsRowHeight);

        if (detailsScrollOffset > 0 && detailsScrollOffset + visibleDetailRows > fields.size()) {
            detailsScrollOffset = (visibleDetailRows > fields.size()) ? 0 : fields.size() - visibleDetailRows;
        }

        detailsScrollbar.visibleItems = visibleDetailRows;
        detailsScrollbar.totalItems = fields.size();
        if (detailsScrollOffset > detailsScrollbar.maxOffset()) {
            detailsScrollOffset = detailsScrollbar.maxOffset();
        }
        detailsScrollbar.offset = detailsScrollOffset;

        for (size_t i = 0; i < visibleDetailRows && detailsScrollOffset + i < fields.size(); i++) {
            size_t fi = detailsScrollOffset + i;
            const FieldDescriptor& fd = fields[fi];
            int y = detailsY + static_cast<int>(i * detailsRowHeight);
            if (viewingDetails && fi == selectedField) {
                renderFilledRect({rightX, y - 2, rightWidth, charHeight + 4}, colors.selectedBg);
            }
            renderText(fd.label, rightX + 5, y, colors.accent, font);
            std::string valueStr = getFieldValueString(tr, fd, fi);
            if (isJapanese && japaneseFont) {
                renderMixedText(valueStr, rightX + 200, y, colors.text);
            } else {
                renderText(valueStr, rightX + 200, y, colors.text, font);
            }
        }

        {
            int detailsY = listY;
            int detailsHeight = windowHeight - detailsY - 10;
            int localListWidth = 300;
            int sbSpace = scrollbar.width;
            int localRightX = localListWidth + sbSpace + 10;
            int localRightWidth = windowWidth - localRightX - 10;
            int sbX = localRightX + localRightWidth - detailsScrollbar.width;
            setScrollbarArea(sbX, detailsY, detailsHeight, &detailsScrollbar);
            renderScrollbar();
            resetScrollbarArea();
        }
    }

    SDL_RenderPresent(renderer);
}

//=============================================================================
//  Editing helper methods - refactored for clarity
//=============================================================================

void PokemonTrainerEditor::startEditing(bool byName) {
    editingValue = true;
    editingByName = byName;
    editBuffer.clear();
    requestRedraw();
}

void PokemonTrainerEditor::cancelEditing() {
    editingValue = false;
    editingByName = false;
    editBuffer.clear();
    requestRedraw();
}

bool PokemonTrainerEditor::tryLookupByName(const TrainerEntry& tr, const FieldDescriptor& fd, std::string& result) {
    if (!editingByName || editBuffer.empty()) {
        return false;
    }
    
    std::string upperBuffer = editBuffer;
    std::transform(upperBuffer.begin(), upperBuffer.end(), upperBuffer.begin(), ::toupper);
    
    if (isGen3Game() && fd.kind == FieldKind::Class) {
        size_t maxNameLength = isJapanese ? 11 : 13;
        size_t baseOffset = static_cast<size_t>(trainer3Addresses.classNames);
        size_t maxClasses = 100;
        
        for (size_t i = 0; i < maxClasses; i++) {
            size_t offset = baseOffset + (i * maxNameLength);
            if (offset + maxNameLength > fileBuffer.size()) break;
            
            std::vector<unsigned char> bytes;
            for (size_t j = 0; j < maxNameLength; j++) {
                bytes.push_back(static_cast<unsigned char>(fileBuffer[offset + j]));
            }
            TextEncoding enc = isJapanese ? TextEncoding::JP_G3 : TextEncoding::EN_G3;
            std::string className = decodeText(bytes, enc, 0xFF);
            
            std::string upperClass = className;
            std::transform(upperClass.begin(), upperClass.end(), upperClass.begin(), ::toupper);
            
            if (upperClass == upperBuffer) {
                result = HexUtils::toHexString(i, 2);
                return true;
            }
        }
    } else if (fd.kind == FieldKind::PokemonSpecies) {
        if (isGen1Game()) {
            for (const auto& kv : PokemonIndex::GEN1_POKEMON) {
                const char* pokeName = kv.second.name;
                if (pokeName && pokeName[0] != '\0') {
                    std::string upperName = pokeName;
                    std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
                    if (upperName == upperBuffer) {
                        result = HexUtils::toHexString(kv.first, 2);
                        return true;
                    }
                }
            }
        } else if (isGen2Game()) {
            for (const auto& kv : PokemonIndex::GEN3_POKEMON) {
                const char* pokeName = kv.second.name;
                if (pokeName && pokeName[0] != '\0') {
                    std::string upperName = pokeName;
                    std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
                    if (upperName == upperBuffer) {
                        result = HexUtils::toHexString(static_cast<size_t>(kv.first), 2);
                        return true;
                    }
                }
            }
        } else if (isGen3Game()) {
            for (const auto& kv : PokemonIndex::GEN3_POKEMON) {
                const char* pokeName = kv.second.name;
                if (pokeName && pokeName[0] != '\0') {
                    std::string upperName = pokeName;
                    std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
                    if (upperName == upperBuffer) {
                        result = HexUtils::toHexString(kv.first, 4);
                        return true;
                    }
                }
            }
        }
    } else if (fd.kind == FieldKind::TrainerItem || fd.kind == FieldKind::PokemonItem) {
        if (isGen2Game()) {
            bool isCrystal = (gameType == GameType::GEN2_CRYSTAL);
            for (const auto& kv : ItemsIndex::GEN2_ITEMS) {
                const char* itemName = ItemsIndex::getGen2ItemName(kv.first, isCrystal);
                if (itemName && itemName[0] != '\0') {
                    std::string upperName = itemName;
                    std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
                    if (upperName == upperBuffer || upperBuffer == "NONE") {
                        result = (upperBuffer == "NONE") ? "00" : HexUtils::toHexString(kv.first, 2);
                        return true;
                    }
                }
            }
        } else if (isGen3Game()) {
            for (const auto& kv : ItemsIndex::GEN3_ITEMS) {
                const char* itemName = ItemsIndex::getGen3ItemName(kv.first);
                if (itemName && itemName[0] != '\0') {
                    std::string upperName = itemName;
                    std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
                    if (upperName == upperBuffer || upperBuffer == "NONE") {
                        result = (upperBuffer == "NONE") ? "0000" : HexUtils::toHexString(kv.first, 4);
                        return true;
                    }
                }
            }
        }
    } else if (fd.kind == FieldKind::PokemonMove) {
        if (isGen2Game()) {
            for (const auto& kv : PokemonMoves::GEN2_MOVES) {
                const char* moveName = kv.second;
                if (moveName && moveName[0] != '\0') {
                    std::string upperName = moveName;
                    std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
                    if (upperName == upperBuffer || upperBuffer == "-" || upperBuffer == "--") {
                        result = (upperBuffer == "-" || upperBuffer == "--") ? "00" : HexUtils::toHexString(kv.first, 2);
                        return true;
                    }
                }
            }
        } else if (isGen3Game()) {
            for (const auto& kv : PokemonMoves::GEN3_MOVES) {
                const char* moveName = kv.second;
                if (moveName && moveName[0] != '\0') {
                    std::string upperName = moveName;
                    std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
                    if (upperName == upperBuffer || upperBuffer == "-" || upperBuffer == "--") {
                        result = (upperBuffer == "-" || upperBuffer == "--") ? "0000" : HexUtils::toHexString(kv.first, 4);
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}

bool PokemonTrainerEditor::applyEdit(TrainerEntry& tr, const FieldDescriptor& fd, const std::string& value) {
    if (value.empty()) return false;
    
    try {
        switch (fd.kind) {
            case FieldKind::Name: {
                std::string newName = value;
                if (!newName.empty()) {
                    size_t end = newName.find_last_not_of(' ');
                    if (end != std::string::npos) newName.erase(end + 1);
                    size_t start = newName.find_first_not_of(' ');
                    if (start != std::string::npos) newName.erase(0, start);
                }
                TextEncoding enc = isJapanese ? TextEncoding::JP_G3 : TextEncoding::EN_G3;
                size_t nameLen = isJapanese ? 6 : 12;
                std::vector<unsigned char> encoded = encodeText(newName, enc, nameLen, 0xFF);
                for (size_t i = 0; i < nameLen && i < encoded.size(); i++) {
                    DataUtils::writeU8(fileBuffer, tr.offset + 0x04 + i, encoded[i]);
                }
                tr.name = decodeTrainerName(encoded);
                refreshDisplayOrder();
                return true;
            }
            case FieldKind::Class: {
                if (isGen1Game()) return false;
                unsigned long v = std::stoul(value, nullptr, 16);
                if (v > 0xFFUL) v = 0xFFUL;
                uint8_t newVal = static_cast<uint8_t>(v);
                if (tr.classId != newVal) {
                    tr.classId = newVal;
                    DataUtils::writeU8(fileBuffer, tr.offset + 0x01, newVal);
                    refreshDisplayOrder();
                }
                return true;
            }
            case FieldKind::Type: {
                if (isGen1Game()) return false;
                unsigned long v = std::stoul(value, nullptr, 16);
                if (v > 3UL) v = 3UL;
                uint8_t newType = static_cast<uint8_t>(v);
                if (tr.type != newType) {
                    tr.type = newType;
                    allocateNewParty(tr, tr.partySize);
                    DataUtils::writeU8(fileBuffer, tr.offset + 0x00, newType);
                }
                return true;
            }
            case FieldKind::PartySize: {
                if (isGen1Game() || isGen2Game()) return false;
                int v = std::stoi(value);
                if (v < 1) v = 1;
                if (v > 6) v = 6;
                uint32_t newSize = static_cast<uint32_t>(v);
                if (tr.partySize != newSize) {
                    allocateNewParty(tr, newSize);
                }
                return true;
            }
            case FieldKind::Flags: {
                unsigned long v = std::stoul(value, nullptr, 16);
                if (v > 0xFFUL) v = 0xFFUL;
                uint8_t newVal = static_cast<uint8_t>(v);
                if (tr.gen3.flags != newVal) {
                    tr.gen3.flags = newVal;
                    DataUtils::writeU8(fileBuffer, tr.offset + 0x02, newVal);
                }
                return true;
            }
            case FieldKind::Sprite: {
                unsigned long v = std::stoul(value, nullptr, 16);
                if (v > 0xFFUL) v = 0xFFUL;
                uint8_t newVal = static_cast<uint8_t>(v);
                if (tr.gen3.sprite != newVal) {
                    tr.gen3.sprite = newVal;
                    DataUtils::writeU8(fileBuffer, tr.offset + 0x03, newVal);
                }
                return true;
            }
            case FieldKind::AI: {
                unsigned long v = std::stoul(value, nullptr, 10);
                if (v > 0xFFFFFFFFUL) v = 0xFFFFFFFFUL;
                uint32_t newVal = static_cast<uint32_t>(v);
                if (tr.gen3.ai != newVal) {
                    tr.gen3.ai = newVal;
                    size_t aiOffset = isJapanese ? 0x14 : 0x1C;
                    DataUtils::writeU32LE(fileBuffer, tr.offset + aiOffset, newVal);
                }
                return true;
            }
            case FieldKind::TrainerItem: {
                unsigned long v = std::stoul(value, nullptr, 16);
                if (v > 0xFFFFUL) v = 0xFFFFUL;
                uint16_t newItem = static_cast<uint16_t>(v);
                size_t slot = fd.subIndex;
                if (slot < tr.gen3.items.size() && tr.gen3.items[slot] != newItem) {
                    tr.gen3.items[slot] = newItem;
                    size_t itemsOffset = isJapanese ? 0x0A : 0x10;
                    DataUtils::writeU16LE(fileBuffer, tr.offset + itemsOffset + slot * 2, newItem);
                }
                return true;
            }
            case FieldKind::PokemonLevel: {
                int lv = std::stoi(value);
                if (lv < 1) lv = 1;
                if (lv > 255) lv = 255;
                if (isGen1Game()) {
                    size_t pi = fd.pokemonIndex;
                    if (pi < tr.gen1.party.size()) {
                        size_t off = tr.gen1.party[pi].levelOffset;
                        if (off < fileBuffer.size()) {
                            DataUtils::writeU8(fileBuffer, off, static_cast<uint8_t>(lv));
                        }
                    }
                } else if (isGen2Game()) {
                    size_t pi = fd.pokemonIndex;
                    if (pi < tr.gen2.party.size()) {
                        size_t off = tr.gen2.party[pi].levelOffset;
                        if (off < fileBuffer.size()) {
                            DataUtils::writeU8(fileBuffer, off, static_cast<uint8_t>(lv));
                        }
                    }
                } else {
                    if (tr.gen3.partyOffset != 0) {
                        size_t perSz = ((tr.type == 0) || (tr.type == 2)) ? 8 : 16;
                        size_t poff = tr.gen3.partyOffset + fd.pokemonIndex * perSz;
                        if (poff + 3 < fileBuffer.size()) {
                            DataUtils::writeU8(fileBuffer, poff + 0x02, static_cast<uint8_t>(lv));
                        }
                    }
                }
                return true;
            }
            case FieldKind::PokemonSpecies: {
                if (isGen1Game()) {
                    uint8_t newSpecies = static_cast<uint8_t>(std::stoul(value, nullptr, 16));
                    size_t pi = fd.pokemonIndex;
                    if (pi < tr.gen1.party.size()) {
                        size_t off = tr.gen1.party[pi].speciesOffset;
                        if (off < fileBuffer.size()) {
                            DataUtils::writeU8(fileBuffer, off, newSpecies);
                        }
                    }
                } else if (isGen2Game()) {
                    uint8_t newSpecies = static_cast<uint8_t>(std::stoul(value, nullptr, 16));
                    size_t pi = fd.pokemonIndex;
                    if (pi < tr.gen2.party.size()) {
                        size_t off = tr.gen2.party[pi].speciesOffset;
                        if (off < fileBuffer.size()) {
                            DataUtils::writeU8(fileBuffer, off, newSpecies);
                        }
                    }
                } else {
                    uint16_t newSpecies = static_cast<uint16_t>(std::stoul(value, nullptr, 16));
                    if (tr.gen3.partyOffset != 0) {
                        size_t perSz = ((tr.type == 0) || (tr.type == 2)) ? 8 : 16;
                        size_t poff = tr.gen3.partyOffset + fd.pokemonIndex * perSz;
                        if (poff + 5 < fileBuffer.size()) {
                            DataUtils::writeU16LE(fileBuffer, poff + 0x04, newSpecies);
                        }
                    }
                }
                return true;
            }
            case FieldKind::PokemonItem: {
                if (isGen1Game()) return false;
                if (isGen2Game()) {
                    uint8_t newItem = static_cast<uint8_t>(std::stoul(value, nullptr, 16));
                    size_t pi = fd.pokemonIndex;
                    if (pi < tr.gen2.party.size()) {
                        const auto& pinfo = tr.gen2.party[pi];
                        if (pinfo.hasItem && pinfo.itemOffset < fileBuffer.size()) {
                            DataUtils::writeU8(fileBuffer, pinfo.itemOffset, newItem);
                        }
                    }
                } else {
                    uint16_t newItem = static_cast<uint16_t>(std::stoul(value, nullptr, 16));
                    if (tr.gen3.partyOffset != 0) {
                        size_t perSz = ((tr.type == 0) || (tr.type == 2)) ? 8 : 16;
                        size_t poff = tr.gen3.partyOffset + fd.pokemonIndex * perSz;
                        if (tr.type == 2 || tr.type == 3) {
                            if (poff + 7 < fileBuffer.size()) {
                                DataUtils::writeU16LE(fileBuffer, poff + 0x06, newItem);
                            }
                        } else {
                            if (poff + 0x0F < fileBuffer.size()) {
                                DataUtils::writeU16LE(fileBuffer, poff + 0x0E, newItem);
                            }
                        }
                    }
                }
                return true;
            }
            case FieldKind::PokemonMove: {
                if (isGen1Game()) return false;
                if (isGen2Game()) {
                    uint8_t newMove = static_cast<uint8_t>(std::stoul(value, nullptr, 16));
                    size_t pi = fd.pokemonIndex;
                    size_t mj = fd.subIndex;
                    if (pi < tr.gen2.party.size()) {
                        const auto& pinfo = tr.gen2.party[pi];
                        if (pinfo.hasMoves && mj < 4) {
                            size_t moff = pinfo.moveOffsets[mj];
                            if (moff < fileBuffer.size()) {
                                DataUtils::writeU8(fileBuffer, moff, newMove);
                            }
                        }
                    }
                } else {
                    uint16_t newMove = static_cast<uint16_t>(std::stoul(value, nullptr, 16));
                    if (tr.gen3.partyOffset != 0) {
                        size_t perSz = ((tr.type == 0) || (tr.type == 2)) ? 8 : 16;
                        size_t poff = tr.gen3.partyOffset + fd.pokemonIndex * perSz;
                        if (tr.type == 3) {
                            size_t moff = poff + 0x08 + fd.subIndex * 2;
                            if (moff + 1 < fileBuffer.size()) {
                                DataUtils::writeU16LE(fileBuffer, moff, newMove);
                            }
                        } else {
                            size_t moff = poff + 0x06 + fd.subIndex * 2;
                            if (moff + 1 < fileBuffer.size()) {
                                DataUtils::writeU16LE(fileBuffer, moff, newMove);
                            }
                        }
                    }
                }
                return true;
            }
            default:
                return false;
        }
    } catch (...) {
        return false;
    }
}

void PokemonTrainerEditor::handleEditInput(SDL_Keycode key) {
    if (!editingValue) return;
    
    // Cancel editing
    if (key == SDLK_ESCAPE) {
        cancelEditing();
        return;
    }
    
    // Commit edit
    if (key == SDLK_RETURN || key == SDLK_RETURN2 || key == SDLK_KP_ENTER) {
        if (!displayOrder.empty() && selectedField < fields.size()) {
            size_t trainerIndex = displayOrder[selectedIndex];
            TrainerEntry& tr = trainers[trainerIndex];
            const FieldDescriptor& fd = fields[selectedField];
            
            std::string valueToApply = editBuffer;
            
            // If in name-based mode, try to lookup by name first
            if (editingByName) {
                std::string lookedUp;
                if (tryLookupByName(tr, fd, lookedUp)) {
                    valueToApply = lookedUp;
                } else {
                    // Name not found, swallow input
                    cancelEditing();
                    return;
                }
            }
            
            // Apply the edit
            if (applyEdit(tr, fd, valueToApply)) {
                hasUnsavedChanges = true;
            }
        }
        cancelEditing();
        return;
    }
    
    // Handle backspace
    if (key == SDLK_BACKSPACE) {
        if (!editBuffer.empty()) {
            editBuffer.pop_back();
            requestRedraw();
        }
        return;
    }
    
    // Handle text/hex input based on editing mode and field type
    if (selectedField < fields.size()) {
        const FieldDescriptor& fd = fields[selectedField];
        
        // Name field always uses text input
        if (fd.kind == FieldKind::Name) {
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
            }
            if (c != 0 && editBuffer.length() < 11) {
                editBuffer.push_back(c);
                requestRedraw();
            }
            return;
        }
        
        // Name-based editing mode (via 'I'): accept text input for lookups
        if (editingByName) {
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
            }
            if (c != 0 && editBuffer.length() < 30) {
                editBuffer.push_back(c);
                requestRedraw();
            }
            return;
        }
        
        // Hex-based editing mode (via Enter): accept hex digits only
        if (fd.kind == FieldKind::PartySize || fd.kind == FieldKind::PokemonLevel || fd.kind == FieldKind::AI) {
            // Decimal numeric fields
            if (key >= SDLK_0 && key <= SDLK_9) {
                char c = static_cast<char>('0' + (key - SDLK_0));
                size_t maxLen = 10;
                if (fd.kind == FieldKind::PartySize) maxLen = 1;
                else if (fd.kind == FieldKind::PokemonLevel) maxLen = 3;
                if (editBuffer.length() < maxLen) {
                    editBuffer.push_back(c);
                    requestRedraw();
                }
            }
        } else {
            // Hex numeric fields (0-F only)
            char c = 0;
            if (key >= SDLK_0 && key <= SDLK_9) {
                c = static_cast<char>('0' + (key - SDLK_0));
            } else if (key >= SDLK_A && key <= SDLK_F) {
                c = static_cast<char>('A' + (key - SDLK_A));
            }
            if (c != 0) {
                size_t maxLen = 2;
                if (fd.kind == FieldKind::TrainerItem || fd.kind == FieldKind::PokemonItem || 
                    fd.kind == FieldKind::PokemonSpecies || fd.kind == FieldKind::PokemonMove) {
                    maxLen = 4;
                }
                if (editBuffer.length() < maxLen) {
                    editBuffer.push_back(static_cast<char>(std::toupper(c)));
                    requestRedraw();
                }
            }
        }
    }
}

//=============================================================================
//  Event handling
//=============================================================================

void PokemonTrainerEditor::handleEvent(SDL_Event& event) {
    if (event.type == SDL_EVENT_QUIT) {
        quit();
        return;
    }
    
    // Mouse events
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        int mx = event.button.x;
        int my = event.button.y;
        lastMouseX = mx;
        lastMouseY = my;

        int headerHeight = charHeight * 2 + 10;
        int listX = 0;
        int listY = headerHeight + 10;
        int listWidth = 300;
        int listHeight = windowHeight - listY - 10;
        int rowHeight = charHeight + 4;

        if (!viewingDetails && mx < listWidth && my >= listY) {
            size_t index = scrollbar.offset + static_cast<size_t>((my - listY) / rowHeight);
            if (index < displayOrder.size()) {
                selectedIndex = index;
                requestRedraw();
            }
        }

        int listSbX = listX + listWidth - scrollbar.width;
        int listSbY = listY;
        int listSbHeight = listHeight;

        int rightX = listWidth + scrollbar.width + 10;
        int rightWidth = windowWidth - rightX - 10;
        int detailsY = listY;
        int detailsHeight = windowHeight - detailsY - 10;
        int detailsSbX = rightX + rightWidth - detailsScrollbar.width;

        setScrollbarArea(detailsSbX, detailsY, detailsHeight, &detailsScrollbar);
        if (handleScrollbarClick(mx, my)) {
            detailsScrollOffset = detailsScrollbar.offset;
            resetScrollbarArea();
            return;
        }
        resetScrollbarArea();

        setScrollbarArea(listSbX, listSbY, listSbHeight, &scrollbar);
        if (handleScrollbarClick(mx, my)) {
            resetScrollbarArea();
            return;
        }
        resetScrollbarArea();
    }
    
    if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        int headerHeight = charHeight * 2 + 10;
        int listY = headerHeight + 10;
        int listWidth = 300;
        int listHeight = windowHeight - listY - 10;
        int listSbX = listWidth - scrollbar.width;
        int listSbY = listY;
        int listSbHeight = listHeight;

        int rightX = listWidth + scrollbar.width + 10;
        int rightWidth = windowWidth - rightX - 10;
        int detailsY = listY;
        int detailsHeight = windowHeight - detailsY - 10;
        int detailsSbX = rightX + rightWidth - detailsScrollbar.width;

        setScrollbarArea(detailsSbX, detailsY, detailsHeight, &detailsScrollbar);
        handleScrollbarRelease();
        resetScrollbarArea();

        setScrollbarArea(listSbX, listSbY, listSbHeight, &scrollbar);
        handleScrollbarRelease();
        resetScrollbarArea();
    }
    
    if (event.type == SDL_EVENT_MOUSE_MOTION) {
        lastMouseX = event.motion.x;
        lastMouseY = event.motion.y;

        int headerHeight = charHeight * 2 + 10;
        int listY = headerHeight + 10;
        int listWidth = 300;
        int listHeight = windowHeight - listY - 10;
        int listSbX = listWidth - scrollbar.width;
        int listSbY = listY;
        int listSbHeight = listHeight;

        int rightX = listWidth + scrollbar.width + 10;
        int rightWidth = windowWidth - rightX - 10;
        int detailsY = listY;
        int detailsHeight = windowHeight - detailsY - 10;
        int detailsSbX = rightX + rightWidth - detailsScrollbar.width;

        if (detailsScrollbar.dragging) {
            setScrollbarArea(detailsSbX, detailsY, detailsHeight, &detailsScrollbar);
            handleScrollbarDrag(event.motion.y);
            resetScrollbarArea();
            detailsScrollOffset = detailsScrollbar.offset;
            requestRedraw();
        }

        if (scrollbar.dragging) {
            setScrollbarArea(listSbX, listSbY, listSbHeight, &scrollbar);
            handleScrollbarDrag(event.motion.y);
            resetScrollbarArea();
            requestRedraw();
        }
    }
    
    if (event.type == SDL_EVENT_MOUSE_WHEEL) {
        int mx = lastMouseX;
        int my = lastMouseY;

        int headerHeight = charHeight * 2 + 10;
        int listY = headerHeight + 10;
        int listWidth = 300;
        int listHeight = windowHeight - listY - 10;

        int rightX = listWidth + scrollbar.width + 10;
        int rightWidth = windowWidth - rightX - 10;
        int detailsY = listY;
        int detailsHeight = windowHeight - detailsY - 10;

        bool inDetails = (mx >= rightX && mx < rightX + rightWidth &&
                          my >= detailsY && my < detailsY + detailsHeight);

        int wheelY = event.wheel.y;
        if (wheelY != 0) {
            float scrollAmount = -static_cast<float>(wheelY) * 0.2f;

            if (inDetails) {
                int detailsSbX = rightX + rightWidth - detailsScrollbar.width;
                setScrollbarArea(detailsSbX, detailsY, detailsHeight, &detailsScrollbar);
                addScrollVelocity(scrollAmount);
                resetScrollbarArea();
            } else {
                int listSbX = listWidth - scrollbar.width;
                int listSbY = listY;
                int listSbHeight = listHeight;
                setScrollbarArea(listSbX, listSbY, listSbHeight, &scrollbar);
                addScrollVelocity(scrollAmount);
                resetScrollbarArea();
            }
            requestRedraw();
        }
    }
    
    // Keyboard events
    if (event.type == SDL_EVENT_KEY_DOWN) {
        SDL_Keycode key = event.key.key;
        SDL_Keymod mod = event.key.mod;

        // Handle editing input
        if (editingValue) {
            handleEditInput(key);
            return;
        }

        // Save on Ctrl/Cmd+S
        if ((mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI)) && (key == SDLK_S)) {
            std::string outPath = getOutputPath();
            writeFile(outPath);
            return;
        }
        
        // Toggle search mode
        if (!searchMode && (key == SDLK_S) && !(mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI))) {
            searchMode = true;
            searchTerm.clear();
            requestRedraw();
            return;
        }
        
        // Handle search input
        if (searchMode) {
            if (key == SDLK_RETURN || key == SDLK_RETURN2 || key == SDLK_KP_ENTER) {
                searchMode = false;
                refreshDisplayOrder();
                requestRedraw();
                return;
            } else if (key == SDLK_BACKSPACE) {
                if (!searchTerm.empty()) {
                    searchTerm.pop_back();
                    refreshDisplayOrder();
                    requestRedraw();
                }
                return;
            } else if (key == SDLK_ESCAPE) {
                searchMode = false;
                refreshDisplayOrder();
                requestRedraw();
                return;
            } else if (key >= 32 && key < 127) {
                searchTerm.push_back(static_cast<char>(tolower(static_cast<char>(key))));
                refreshDisplayOrder();
                requestRedraw();
            }
            return;
        }

        // Escape - quit or exit details
        if (key == SDLK_ESCAPE) {
            if (viewingDetails) {
                viewingDetails = false;
                selectedField = 0;
                requestRedraw();
            } else {
                if (hasUnsavedChanges) {
                    if (showQuitConfirmDialog()) {
                        quit();
                    }
                } else {
                    quit();
                }
            }
            return;
        }

        // Toggle sorting
        if (key == SDLK_T) {
            if (sortMode == SortMode::Memory) {
                sortMode = SortMode::Class;
            } else if (sortMode == SortMode::Class) {
                sortMode = SortMode::Name;
            } else {
                sortMode = SortMode::Memory;
            }
            refreshDisplayOrder();
            requestRedraw();
            return;
        }
        
        // Press 'I' to start editing by name (when viewing details)
        if (key == SDLK_I && viewingDetails && !editingValue) {
            if (selectedField < fields.size()) {
                const FieldDescriptor& fd = fields[selectedField];
                bool canEditByName = false;
                if (isGen3Game() && fd.kind == FieldKind::Class) {
                    canEditByName = true;
                } else if (fd.kind == FieldKind::PokemonSpecies) {
                    canEditByName = true;
                } else if (fd.kind == FieldKind::TrainerItem || fd.kind == FieldKind::PokemonItem) {
                    canEditByName = true;
                } else if (fd.kind == FieldKind::PokemonMove) {
                    canEditByName = true;
                }
                
                if (canEditByName) {
                    startEditing(true);
                }
            }
            return;
        }
        
        // Handle Enter - start hex-based editing or open details
        if (key == SDLK_RETURN || key == SDLK_RETURN2 || key == SDLK_KP_ENTER) {
            if (!displayOrder.empty()) {
                if (!viewingDetails) {
                    viewingDetails = true;
                    selectedField = 0;
                    requestRedraw();
                } else {
                    if (selectedField < fields.size()) {
                        const FieldDescriptor& fd = fields[selectedField];
                        bool isGen2 = isGen2Game();
                        bool isGen1 = isGen1Game();
                        bool readOnly = false;
                        
                        if (isGen2 || isGen1) {
                            if (fd.kind == FieldKind::Name || fd.kind == FieldKind::Class ||
                                fd.kind == FieldKind::Type || fd.kind == FieldKind::PartySize) {
                                readOnly = true;
                            }
                            if (isGen1 && (fd.kind == FieldKind::PokemonItem || 
                                          fd.kind == FieldKind::PokemonMove || 
                                          fd.kind == FieldKind::TrainerItem)) {
                                readOnly = true;
                            }
                        }
                        
                        // Name field uses text mode
                        if (fd.kind == FieldKind::Name) {
                            startEditing(false);
                        } else if (!readOnly) {
                            // Other fields use hex mode
                            startEditing(false);
                        }
                    }
                }
            }
            return;
        }
        
        // Navigation
        if (!viewingDetails) {
            if (key == SDLK_UP) {
                if (displayOrder.empty()) return;
                if (selectedIndex > 0) {
                    selectedIndex--;
                } else {
                    selectedIndex = displayOrder.size() - 1;
                    if (scrollbar.visibleItems < scrollbar.totalItems) {
                        scrollbar.offset = scrollbar.totalItems - scrollbar.visibleItems;
                    } else {
                        scrollbar.offset = 0;
                    }
                }
                if (selectedIndex < scrollbar.offset) {
                    scrollBy(-1);
                } else if (selectedIndex >= scrollbar.offset + scrollbar.visibleItems) {
                    scrollBy(1);
                }
                requestRedraw();
                return;
            }
            if (key == SDLK_DOWN) {
                if (displayOrder.empty()) return;
                if (selectedIndex + 1 < displayOrder.size()) {
                    selectedIndex++;
                } else {
                    selectedIndex = 0;
                    scrollbar.offset = 0;
                }
                if (selectedIndex < scrollbar.offset) {
                    scrollBy(-1);
                } else if (selectedIndex >= scrollbar.offset + scrollbar.visibleItems) {
                    scrollBy(1);
                }
                requestRedraw();
                return;
            }
        } else {
            if (key == SDLK_UP) {
                if (fields.empty()) return;
                if (selectedField > 0) {
                    selectedField--;
                } else {
                    selectedField = fields.size() - 1;
                    int headerHeight = charHeight * 2 + 10;
                    int listY = headerHeight + 10;
                    int detailsY = listY;
                    int rowHeight = charHeight + 6;
                    int detailsHeight = windowHeight - detailsY - 10;
                    size_t visibleRows = (detailsHeight / rowHeight);
                    if (visibleRows < fields.size()) {
                        detailsScrollOffset = fields.size() - visibleRows;
                    } else {
                        detailsScrollOffset = 0;
                    }
                    detailsScrollbar.offset = detailsScrollOffset;
                    requestRedraw();
                    return;
                }
                {
                    int headerHeight = charHeight * 2 + 10;
                    int listY = headerHeight + 10;
                    int detailsY = listY;
                    int rowHeight = charHeight + 6;
                    int detailsHeight = windowHeight - detailsY - 10;
                    size_t visibleRows = (detailsHeight / rowHeight);
                    if (selectedField < detailsScrollOffset) {
                        detailsScrollOffset = selectedField;
                        detailsScrollbar.offset = detailsScrollOffset;
                    } else if (selectedField >= detailsScrollOffset + visibleRows) {
                        detailsScrollOffset = selectedField + 1 - visibleRows;
                        detailsScrollbar.offset = detailsScrollOffset;
                    }
                }
                requestRedraw();
                return;
            }
            if (key == SDLK_DOWN) {
                if (fields.empty()) return;
                if (selectedField + 1 < fields.size()) {
                    selectedField++;
                } else {
                    selectedField = 0;
                    detailsScrollOffset = 0;
                    detailsScrollbar.offset = 0;
                    requestRedraw();
                    return;
                }
                {
                    int headerHeight = charHeight * 2 + 10;
                    int listY = headerHeight + 10;
                    int detailsY = listY;
                    int rowHeight = charHeight + 6;
                    int detailsHeight = windowHeight - detailsY - 10;
                    size_t visibleRows = (detailsHeight / rowHeight);
                    if (selectedField < detailsScrollOffset) {
                        detailsScrollOffset = selectedField;
                        detailsScrollbar.offset = detailsScrollOffset;
                    } else if (selectedField >= detailsScrollOffset + visibleRows) {
                        detailsScrollOffset = selectedField + 1 - visibleRows;
                        detailsScrollbar.offset = detailsScrollOffset;
                    }
                }
                requestRedraw();
                return;
            }
        }
    }
}

//=============================================================================
//  Update function
//=============================================================================

void PokemonTrainerEditor::update(float deltaTime) {
    setConfirmOnQuit(hasUnsavedChanges);

    setScrollbarArea(0, 0, 0, &detailsScrollbar);
    updateMomentumScroll(deltaTime);
    resetScrollbarArea();

    detailsScrollOffset = detailsScrollbar.offset;

    SDLAppBase::update(deltaTime);
}