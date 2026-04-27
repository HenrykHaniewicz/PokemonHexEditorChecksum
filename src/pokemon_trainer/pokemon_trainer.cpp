//=============================================================================
//  pokemon_trainer.cpp
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
    return decodeText(bytes, TextEncoding::EN_G3, 0xFF);
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
        size_t offset = baseOffset + (static_cast<size_t>(classId) * 13);
        if (offset + 13 > fileBuffer.size()) {
            return std::string("Class ") + std::to_string(classId);
        }
        std::vector<unsigned char> bytes;
        bytes.reserve(13);
        for (size_t i = 0; i < 13; i++) {
            bytes.push_back(static_cast<unsigned char>(fileBuffer[offset + i]));
        }
        std::string decoded = decodeText(bytes, TextEncoding::EN_G3, 0xFF);
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
    {
        std::string lower = g;
        // Already lowercased and whitespace removed above
        if (isJapanese) {
            if (lower == "ruby" || lower == "pokemonruby" || lower == "rubyversion" ||
                lower == "sapphire" || lower == "pokemonsapphire" || lower == "sapphireversion" ||
                lower == "emerald" || lower == "firered" || lower == "fireredversion" ||
                lower == "leafgreen" || lower == "leafgreenversion") {
                std::cerr << "Japanese games are not supported for Gen 3 in pokemon_trainer." << std::endl;
                return false;
            }
        }
    }
    if (g == "ruby" || g == "pokemonruby" || g == "rubyversion") {
        gameType = GameType::GEN3_RS;
        gameName = "Pokemon Ruby";
        trainer3Addresses = Generation3Utils::TRAINER_ADDRESSES_RUBY;
    } else if (g == "sapphire" || g == "pokemonsapphire" || g == "sapphireversion") {
        gameType = GameType::GEN3_RS;
        gameName = "Pokemon Sapphire";
        trainer3Addresses = Generation3Utils::TRAINER_ADDRESSES_SAPPHIRE;
    } else if (g == "emerald") {
        gameType = GameType::GEN3_EMERALD;
        gameName = "Pokemon Emerald";
        trainer3Addresses = Generation3Utils::TRAINER_ADDRESSES_EMERALD;
    } else if (g == "firered" || g == "fireredversion") {
        gameType = GameType::GEN3_FRLG;
        gameName = "Pokemon FireRed";
        trainer3Addresses = Generation3Utils::TRAINER_ADDRESSES_FIRERED;
    } else if (g == "leafgreen" || g == "leafgreenversion") {
        gameType = GameType::GEN3_FRLG;
        gameName = "Pokemon LeafGreen";
        trainer3Addresses = Generation3Utils::TRAINER_ADDRESSES_LEAFGREEN;
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
            // Format 1: first byte is level, then species list until 0x00
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
                // Skip the 0x00 terminator
                cur = speciesPtr + 1;
            } else {
                // Format 2: level/species pairs until 0x00
                size_t ptr = offset + 1;
                while (ptr + 1 <= endAddr && static_cast<unsigned char>(fileBuffer[ptr]) != 0x00) {
                    Gen1PokemonInfo info;
                    info.levelOffset = ptr;
                    info.speciesOffset = ptr + 1;
                    party.push_back(info);
                    ptr += 2;
                }
                // Skip the 0x00 terminator
                cur = ptr + 1;
            }
            if (cur > endAddr + 1) {
                cur = endAddr + 1;
            }
            TrainerEntry entry;
            entry.offset = offset;
            // Use 0 for Format 1 and 1 for Format 2 for consistency
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
        // Decode class names using English or Japanese Gen 2 encoding
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
    const size_t recordSize = 0x28;
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

        std::vector<unsigned char> nameBytes;
        nameBytes.reserve(12);
        for (size_t i = 0; i < 12 && offset + 0x04 + i < fileBuffer.size(); i++) {
            nameBytes.push_back(static_cast<unsigned char>(fileBuffer[offset + 0x04 + i]));
        }
        entry.name = decodeTrainerName(nameBytes);

        for (size_t i = 0; i < 4; i++) {
            entry.gen3.items[i] = DataUtils::readU16LE(fileBuffer, offset + 0x10 + (i * 2));
        }
        for (size_t i = 0; i < 4; i++) {
            entry.gen3.unknown[i] = DataUtils::readU8(fileBuffer, offset + 0x18 + i);
        }
        entry.gen3.ai = DataUtils::readU32LE(fileBuffer, offset + 0x1C);
        entry.partySize = DataUtils::readU32LE(fileBuffer, offset + 0x20);
        entry.gen3.partyPointer = DataUtils::readU32LE(fileBuffer, offset + 0x24);
        if (entry.gen3.partyPointer >= 0x08000000) {
            uint32_t romOffset = entry.gen3.partyPointer - 0x08000000;
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
        if (tr.gen3.partyPointer == 0 || tr.gen3.partyPointer < 0x08000000) continue;
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

    uint32_t newPointer = static_cast<uint32_t>(candidate + 0x08000000);
    DataUtils::writeU32LE(fileBuffer, entry.offset + 0x24, newPointer);
    DataUtils::writeU32LE(fileBuffer, entry.offset + 0x20, newPartySize);
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
        // When in search mode, display the search prompt. Use mixed text
        // rendering when in Japanese mode so that multi-byte characters in
        // the search term render correctly.
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
        std::string infoText = sortLabel + "   Press S to search, T to toggle sort, Enter to view details, Ctrl+S to save";
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
//  Event handling
//=============================================================================

void PokemonTrainerEditor::handleEvent(SDL_Event& event) {
    if (event.type == SDL_EVENT_QUIT) {
        quit();
        return;
    }
    // ---------------------------------------------------------------------
    // Mouse button press: selection and scrollbar interactions
    // ---------------------------------------------------------------------
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        int mx = event.button.x;
        int my = event.button.y;
        lastMouseX = mx;
        lastMouseY = my;

        // Compute geometry for list and details panes
        int headerHeight = charHeight * 2 + 10;
        int listX = 0;
        int listY = headerHeight + 10;
        int listWidth = 300;
        int listHeight = windowHeight - listY - 10;
        int rowHeight = charHeight + 4;

        // Determine whether the click selects a trainer entry (only when not viewing details)
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

        // Details pane geometry
        int rightX = listWidth + scrollbar.width + 10;
        int rightWidth = windowWidth - rightX - 10;
        int detailsY = listY;
        int detailsHeight = windowHeight - detailsY - 10;
        int detailsSbX = rightX + rightWidth - detailsScrollbar.width;

        // First attempt to handle click on the details scrollbar
        bool consumed = false;
        setScrollbarArea(detailsSbX, detailsY, detailsHeight, &detailsScrollbar);
        if (handleScrollbarClick(mx, my)) {
            detailsScrollOffset = detailsScrollbar.offset;
            consumed = true;
        }
        resetScrollbarArea();
        if (consumed) {
            return;
        }

        setScrollbarArea(listSbX, listSbY, listSbHeight, &scrollbar);
        if (handleScrollbarClick(mx, my)) {
            resetScrollbarArea();
            return;
        }
        resetScrollbarArea();
    }
    // ---------------------------------------------------------------------
    // Mouse button release: end any scrollbar dragging
    // ---------------------------------------------------------------------
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

        // Release details scrollbar drag
        setScrollbarArea(detailsSbX, detailsY, detailsHeight, &detailsScrollbar);
        handleScrollbarRelease();
        resetScrollbarArea();

        // Release list scrollbar drag
        setScrollbarArea(listSbX, listSbY, listSbHeight, &scrollbar);
        handleScrollbarRelease();
        resetScrollbarArea();
    }
    // ---------------------------------------------------------------------
    // Mouse movement: update drag actions for scrollbars
    // ---------------------------------------------------------------------
    if (event.type == SDL_EVENT_MOUSE_MOTION) {
        lastMouseX = event.motion.x;
        lastMouseY = event.motion.y;

        // Compute geometry for scrollbars
        int headerHeight = charHeight * 2 + 10;
        int listX = 0;
        int listY = headerHeight + 10;
        int listWidth = 300;
        int listHeight = windowHeight - listY - 10;
        int listSbX = listX + listWidth - scrollbar.width;
        int listSbY = listY;
        int listSbHeight = listHeight;

        int rightX = listWidth + scrollbar.width + 10;
        int rightWidth = windowWidth - rightX - 10;
        int detailsY = listY;
        int detailsHeight = windowHeight - detailsY - 10;
        int detailsSbX = rightX + rightWidth - detailsScrollbar.width;

        // Drag the details scrollbar if it's currently being dragged
        if (detailsScrollbar.dragging) {
            setScrollbarArea(detailsSbX, detailsY, detailsHeight, &detailsScrollbar);
            handleScrollbarDrag(event.motion.y);
            resetScrollbarArea();
            detailsScrollOffset = detailsScrollbar.offset;
            requestRedraw();
        }

        // Drag the list scrollbar if it's currently being dragged
        if (scrollbar.dragging) {
            setScrollbarArea(listSbX, listSbY, listSbHeight, &scrollbar);
            handleScrollbarDrag(event.motion.y);
            resetScrollbarArea();
            requestRedraw();
        }
    }
    // ---------------------------------------------------------------------
    // Mouse wheel: momentum scrolling for list or details panes
    // ---------------------------------------------------------------------
    if (event.type == SDL_EVENT_MOUSE_WHEEL) {
        int mx = lastMouseX;
        int my = lastMouseY;

        // Compute geometry for list and details regions
        int headerHeight = charHeight * 2 + 10;
        //int listX = 0;
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
    if (event.type == SDL_EVENT_KEY_DOWN) {
        SDL_Keycode key = event.key.key;
        SDL_Keymod mod = event.key.mod;

        if (editingValue) {
            // Cancel editing with ESCAPE
            if (key == SDLK_ESCAPE) {
                editingValue = false;
                editBuffer.clear();
                requestRedraw();
                return;
            }
            // Commit edit with Enter/Return
            if (key == SDLK_RETURN || key == SDLK_RETURN2 || key == SDLK_KP_ENTER) {
                if (!displayOrder.empty() && selectedField < fields.size()) {
                    size_t trainerIndex = displayOrder[selectedIndex];
                    TrainerEntry& tr = trainers[trainerIndex];
                    const FieldDescriptor& fd = fields[selectedField];
                    try {
                        switch (fd.kind) {
                            case FieldKind::Name: {
                                std::string newName = editBuffer;
                                if (!newName.empty()) {
                                    newName.erase(newName.find_last_not_of(' ') + 1);
                                    newName.erase(0, newName.find_first_not_of(' '));
                                }
                                std::vector<unsigned char> encoded = encodeText(newName, TextEncoding::EN_G3, 12, 0xFF);
                                for (size_t i = 0; i < 12 && i < encoded.size(); i++) {
                                    DataUtils::writeU8(fileBuffer, tr.offset + 0x04 + i, encoded[i]);
                                }
                                tr.name = decodeTrainerName(encoded);
                                hasUnsavedChanges = true;
                                refreshDisplayOrder();
                                break;
                            }
                            case FieldKind::Class: {
                                // In Gen 1 the trainer class is fixed and cannot be edited.
                                if (isGen1Game()) {
                                    break;
                                }
                                if (!editBuffer.empty()) {
                                    unsigned long value = std::stoul(editBuffer, nullptr, 16);
                                    if (value > 0xFFUL) value = 0xFFUL;
                                    uint8_t newVal = static_cast<uint8_t>(value);
                                    if (tr.classId != newVal) {
                                        tr.classId = newVal;
                                        DataUtils::writeU8(fileBuffer, tr.offset + 0x01, newVal);
                                        hasUnsavedChanges = true;
                                        refreshDisplayOrder();
                                    }
                                }
                                break;
                            }
                            case FieldKind::Type: {
                                // Gen 1 trainers do not have a configurable type field
                                if (isGen1Game()) {
                                    break;
                                }
                                if (!editBuffer.empty()) {
                                    unsigned long value = std::stoul(editBuffer, nullptr, 16);
                                    if (value > 3UL) value = 3UL;
                                    uint8_t newType = static_cast<uint8_t>(value);
                                    if (tr.type != newType) {
                                        tr.type = newType;
                                        allocateNewParty(tr, tr.partySize);
                                        DataUtils::writeU8(fileBuffer, tr.offset + 0x00, newType);
                                        hasUnsavedChanges = true;
                                    }
                                }
                                break;
                            }
                            case FieldKind::PartySize: {
                                // Party size in Gen 1 and Gen 2 is implicit and cannot be changed
                                if (isGen1Game() || isGen2Game()) {
                                    break;
                                }
                                if (!editBuffer.empty()) {
                                    int v = std::stoi(editBuffer);
                                    if (v < 1) v = 1;
                                    if (v > 6) v = 6;
                                    uint32_t newSize = static_cast<uint32_t>(v);
                                    if (tr.partySize != newSize) {
                                        allocateNewParty(tr, newSize);
                                        hasUnsavedChanges = true;
                                    }
                                }
                                break;
                            }
                            case FieldKind::Flags: {
                                if (!editBuffer.empty()) {
                                    unsigned long value = std::stoul(editBuffer, nullptr, 16);
                                    if (value > 0xFFUL) value = 0xFFUL;
                                    uint8_t newVal = static_cast<uint8_t>(value);
                                    if (tr.gen3.flags != newVal) {
                                        tr.gen3.flags = newVal;
                                        DataUtils::writeU8(fileBuffer, tr.offset + 0x02, newVal);
                                        hasUnsavedChanges = true;
                                    }
                                }
                                break;
                            }
                            case FieldKind::Sprite: {
                                if (!editBuffer.empty()) {
                                    unsigned long value = std::stoul(editBuffer, nullptr, 16);
                                    if (value > 0xFFUL) value = 0xFFUL;
                                    uint8_t newVal = static_cast<uint8_t>(value);
                                    if (tr.gen3.sprite != newVal) {
                                        tr.gen3.sprite = newVal;
                                        DataUtils::writeU8(fileBuffer, tr.offset + 0x03, newVal);
                                        hasUnsavedChanges = true;
                                    }
                                }
                                break;
                            }
                            case FieldKind::AI: {
                                if (!editBuffer.empty()) {
                                    unsigned long value = std::stoul(editBuffer, nullptr, 10);
                                    if (value > 0xFFFFFFFFUL) value = 0xFFFFFFFFUL;
                                    uint32_t newVal = static_cast<uint32_t>(value);
                                    if (tr.gen3.ai != newVal) {
                                        tr.gen3.ai = newVal;
                                        DataUtils::writeU32LE(fileBuffer, tr.offset + 0x1C, newVal);
                                        hasUnsavedChanges = true;
                                    }
                                }
                                break;
                            }
                            case FieldKind::TrainerItem: {
                                if (!editBuffer.empty()) {
                                    unsigned long value = std::stoul(editBuffer, nullptr, 16);
                                    if (value > 0xFFFFUL) value = 0xFFFFUL;
                                    uint16_t newItem = static_cast<uint16_t>(value);
                                    size_t slot = fd.subIndex;
                                    if (slot < tr.gen3.items.size() && tr.gen3.items[slot] != newItem) {
                                        tr.gen3.items[slot] = newItem;
                                        DataUtils::writeU16LE(fileBuffer, tr.offset + 0x10 + slot * 2, newItem);
                                        hasUnsavedChanges = true;
                                    }
                                }
                                break;
                            }
                            case FieldKind::PokemonLevel: {
                                if (!editBuffer.empty()) {
                                    int lv = std::stoi(editBuffer);
                                    if (lv < 1) lv = 1;
                                    if (lv > 255) lv = 255;
                                    if (isGen1Game()) {
                                        size_t pi = fd.pokemonIndex;
                                        if (pi < tr.gen1.party.size()) {
                                            size_t off = tr.gen1.party[pi].levelOffset;
                                            if (off < fileBuffer.size()) {
                                                DataUtils::writeU8(fileBuffer, off, static_cast<uint8_t>(lv));
                                                hasUnsavedChanges = true;
                                            }
                                        }
                                    } else if (gameType == GameType::GEN2_GS || gameType == GameType::GEN2_CRYSTAL) {
                                        size_t pi = fd.pokemonIndex;
                                        if (pi < tr.gen2.party.size()) {
                                            size_t off = tr.gen2.party[pi].levelOffset;
                                            if (off < fileBuffer.size()) {
                                                DataUtils::writeU8(fileBuffer, off, static_cast<uint8_t>(lv));
                                                hasUnsavedChanges = true;
                                            }
                                        }
                                    } else {
                                        // Generation 3 editing
                                        if (tr.gen3.partyOffset != 0) {
                                            size_t perSz = ((tr.type == 0) || (tr.type == 2)) ? 8 : 16;
                                            size_t poff = tr.gen3.partyOffset + fd.pokemonIndex * perSz;
                                            if (poff + 3 < fileBuffer.size()) {
                                                DataUtils::writeU8(fileBuffer, poff + 0x02, static_cast<uint8_t>(lv));
                                                hasUnsavedChanges = true;
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                            case FieldKind::PokemonSpecies: {
                                // Only process when a value is entered
                                if (!editBuffer.empty()) {
                                    unsigned long value = std::stoul(editBuffer, nullptr, 16);
                                    if (isGen1Game()) {
                                        if (value > 0xFFUL) value = 0xFFUL;
                                        uint8_t newSpecies = static_cast<uint8_t>(value);
                                        size_t pi = fd.pokemonIndex;
                                        if (pi < tr.gen1.party.size()) {
                                            size_t off = tr.gen1.party[pi].speciesOffset;
                                            if (off < fileBuffer.size()) {
                                                DataUtils::writeU8(fileBuffer, off, newSpecies);
                                                hasUnsavedChanges = true;
                                            }
                                        }
                                    } else if (gameType == GameType::GEN2_GS || gameType == GameType::GEN2_CRYSTAL) {
                                        if (value > 0xFFUL) value = 0xFFUL;
                                        uint8_t newSpecies = static_cast<uint8_t>(value);
                                        size_t pi = fd.pokemonIndex;
                                        if (pi < tr.gen2.party.size()) {
                                            size_t off = tr.gen2.party[pi].speciesOffset;
                                            if (off < fileBuffer.size()) {
                                                DataUtils::writeU8(fileBuffer, off, newSpecies);
                                                hasUnsavedChanges = true;
                                            }
                                        }
                                    } else {
                                        if (value > 0xFFFFUL) value = 0xFFFFUL;
                                        uint16_t newSpecies = static_cast<uint16_t>(value);
                                        if (tr.gen3.partyOffset != 0) {
                                            size_t perSz = ((tr.type == 0) || (tr.type == 2)) ? 8 : 16;
                                            size_t poff = tr.gen3.partyOffset + fd.pokemonIndex * perSz;
                                            if (poff + 5 < fileBuffer.size()) {
                                                DataUtils::writeU16LE(fileBuffer, poff + 0x04, newSpecies);
                                                hasUnsavedChanges = true;
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                            case FieldKind::PokemonItem: {
                                // Only update when a value is entered
                                if (!editBuffer.empty()) {
                                    if (isGen1Game()) {
                                        break;
                                    }
                                    unsigned long value = std::stoul(editBuffer, nullptr, 16);
                                    if (gameType == GameType::GEN2_GS || gameType == GameType::GEN2_CRYSTAL) {
                                        // Gen2 items are 1 byte
                                        if (value > 0xFFUL) value = 0xFFUL;
                                        uint8_t newItem = static_cast<uint8_t>(value);
                                        size_t pi = fd.pokemonIndex;
                                        if (pi < tr.gen2.party.size()) {
                                            const auto& pinfo = tr.gen2.party[pi];
                                            if (pinfo.hasItem && pinfo.itemOffset < fileBuffer.size()) {
                                                DataUtils::writeU8(fileBuffer, pinfo.itemOffset, newItem);
                                                hasUnsavedChanges = true;
                                            }
                                        }
                                    } else {
                                        // Gen3 items are 2 bytes
                                        if (value > 0xFFFFUL) value = 0xFFFFUL;
                                        uint16_t newItem = static_cast<uint16_t>(value);
                                        if (tr.gen3.partyOffset != 0) {
                                            size_t perSz = ((tr.type == 0) || (tr.type == 2)) ? 8 : 16;
                                            size_t poff = tr.gen3.partyOffset + fd.pokemonIndex * perSz;
                                            // Types 2 and 3 store the item before the moves at +0x06.
                                            // Type 1 and other types place the item after four moves at +0x0E.
                                            if (tr.type == 2 || tr.type == 3) {
                                                if (poff + 7 < fileBuffer.size()) {
                                                    DataUtils::writeU16LE(fileBuffer, poff + 0x06, newItem);
                                                    hasUnsavedChanges = true;
                                                }
                                            } else {
                                                if (poff + 0x0F < fileBuffer.size()) {
                                                    DataUtils::writeU16LE(fileBuffer, poff + 0x0E, newItem);
                                                    hasUnsavedChanges = true;
                                                }
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                            case FieldKind::PokemonMove: {
                                if (!editBuffer.empty()) {
                                    if (isGen1Game()) {
                                        break;
                                    }
                                    unsigned long value = std::stoul(editBuffer, nullptr, 16);
                                    if (gameType == GameType::GEN2_GS || gameType == GameType::GEN2_CRYSTAL) {
                                        // Gen2 moves are 1 byte each
                                        if (value > 0xFFUL) value = 0xFFUL;
                                        uint8_t newMove = static_cast<uint8_t>(value);
                                        size_t pi = fd.pokemonIndex;
                                        size_t mj = fd.subIndex;
                                        if (pi < tr.gen2.party.size()) {
                                            const auto& pinfo = tr.gen2.party[pi];
                                            if (pinfo.hasMoves && mj < 4) {
                                                size_t moff = pinfo.moveOffsets[mj];
                                                if (moff < fileBuffer.size()) {
                                                    DataUtils::writeU8(fileBuffer, moff, newMove);
                                                    hasUnsavedChanges = true;
                                                }
                                            }
                                        }
                                    } else {
                                        // Gen3 moves are 2 bytes
                                        if (value > 0xFFFFUL) value = 0xFFFFUL;
                                        uint16_t newMove = static_cast<uint16_t>(value);
                                        if (tr.gen3.partyOffset != 0) {
                                            size_t perSz = ((tr.type == 0) || (tr.type == 2)) ? 8 : 16;
                                            size_t poff = tr.gen3.partyOffset + fd.pokemonIndex * perSz;
                                            // Type 3 stores moves after the item at +0x08.
                                            // Type 1 and other types store moves immediately after level/species at +0x06.
                                            if (tr.type == 3) {
                                                size_t moff = poff + 0x08 + fd.subIndex * 2;
                                                if (moff + 1 < fileBuffer.size()) {
                                                    DataUtils::writeU16LE(fileBuffer, moff, newMove);
                                                    hasUnsavedChanges = true;
                                                }
                                            } else {
                                                size_t moff = poff + 0x06 + fd.subIndex * 2;
                                                if (moff + 1 < fileBuffer.size()) {
                                                    DataUtils::writeU16LE(fileBuffer, moff, newMove);
                                                    hasUnsavedChanges = true;
                                                }
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                        }
                    } catch (...) {
                        // Ignore parse errors for invalid numeric input
                    }
                }
                editingValue = false;
                editBuffer.clear();
                requestRedraw();
                return;
            }
            // Handle backspace when editing
            if (key == SDLK_BACKSPACE) {
                if (!editBuffer.empty()) {
                    editBuffer.pop_back();
                    requestRedraw();
                }
                return;
            }
            // Input handling for the edit buffer
            if (selectedField < fields.size()) {
                const FieldDescriptor& fd = fields[selectedField];
                // Name editing: allow alphanumeric, space, hyphen, period, apostrophe
                if (fd.kind == FieldKind::Name) {
                    char c = 0;
                    if (key >= SDLK_A && key <= SDLK_Z) {
                        c = static_cast<char>('A' + (key - SDLK_A));
                    } else if (key >= SDLK_A && key <= SDLK_Z) {
                        c = static_cast<char>('a' + (key - SDLK_A));
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
                    if (c != 0) {
                        if (editBuffer.length() < 11) {
                            editBuffer.push_back(c);
                            requestRedraw();
                        }
                        return;
                    }
                } else if (fd.kind == FieldKind::PartySize || fd.kind == FieldKind::PokemonLevel || fd.kind == FieldKind::AI) {
                    // Decimal numeric fields
                    if (key >= SDLK_0 && key <= SDLK_9) {
                        char c = static_cast<char>('0' + (key - SDLK_0));
                        size_t maxLen = 10;
                        if (fd.kind == FieldKind::PartySize) {
                            maxLen = 1;
                        } else if (fd.kind == FieldKind::PokemonLevel) {
                            maxLen = 3;
                        }
                        if (editBuffer.length() < maxLen) {
                            editBuffer.push_back(c);
                            requestRedraw();
                        }
                        return;
                    }
                } else {
                    // Hexadecimal numeric fields
                    char c = 0;
                    if (key >= SDLK_0 && key <= SDLK_9) {
                        c = static_cast<char>('0' + (key - SDLK_0));
                    } else if (key >= SDLK_A && key <= SDLK_F) {
                        c = static_cast<char>('A' + (key - SDLK_A));
                    } else if (key >= SDLK_A && key <= SDLK_F) {
                        c = static_cast<char>('a' + (key - SDLK_A));
                    }
                    if (c != 0) {
                        size_t maxLen = 2;
                        if (fd.kind == FieldKind::TrainerItem || fd.kind == FieldKind::PokemonItem || fd.kind == FieldKind::PokemonSpecies || fd.kind == FieldKind::PokemonMove) {
                            maxLen = 4;
                        } else if (fd.kind == FieldKind::Class || fd.kind == FieldKind::Flags || fd.kind == FieldKind::Sprite || fd.kind == FieldKind::Type) {
                            maxLen = 2;
                        }
                        if (editBuffer.length() < maxLen) {
                            editBuffer.push_back(static_cast<char>(std::toupper(c)));
                            requestRedraw();
                        }
                        return;
                    }
                }
            }
            // Ignore other keys while editing
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
            } else {
                // Append printable characters
                if (key >= 32 && key < 127) {
                    char ch = static_cast<char>(key);
                    searchTerm.push_back(static_cast<char>(tolower(ch)));
                    refreshDisplayOrder();
                    requestRedraw();
                }
                return;
            }
        }

        // Not editing or in search mode: handle global commands and navigation
        // Universal quit via Escape
        if (key == SDLK_ESCAPE) {
            if (viewingDetails) {
                // Exit details mode without quitting the application
                viewingDetails = false;
                selectedField = 0;
                requestRedraw();
            } else {
                // Prompt to quit when there are unsaved changes.  Otherwise quit immediately.
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

        // Toggle sorting: cycle through memory → class → name
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
        // Handle Enter: either open details or start editing a field
        if (key == SDLK_RETURN || key == SDLK_RETURN2 || key == SDLK_KP_ENTER) {
            if (!displayOrder.empty()) {
                if (!viewingDetails) {
                    // Open details view and select the first field
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
                            if (isGen1) {
                                if (fd.kind == FieldKind::PokemonItem || fd.kind == FieldKind::PokemonMove || fd.kind == FieldKind::TrainerItem) {
                                    readOnly = true;
                                }
                            }
                        }
                        if (!readOnly) {
                            editingValue = true;
                            editBuffer.clear();
                            requestRedraw();
                        }
                        // If readOnly, ignore the Enter key and stay in details view.
                    }
                }
            }
            return;
        }
        // Navigation when not editing and not in search mode
        if (!viewingDetails) {
            if (key == SDLK_UP) {
                if (displayOrder.empty()) return;
                if (selectedIndex > 0) {
                    selectedIndex--;
                } else {
                    // wrap to last
                    selectedIndex = displayOrder.size() - 1;
                    // ensure the last item is visible by scrolling to bottom
                    if (scrollbar.visibleItems < scrollbar.totalItems) {
                        scrollbar.offset = scrollbar.totalItems - scrollbar.visibleItems;
                    } else {
                        scrollbar.offset = 0;
                    }
                }
                // adjust scroll if needed
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
                    // wrap to first
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
            // Navigation when viewing details and not editing
            if (key == SDLK_UP) {
                if (fields.empty()) return;
                if (selectedField > 0) {
                    selectedField--;
                } else {
                    // wrap to last field
                    selectedField = fields.size() - 1;
                    // Compute visible rows for details panel
                    int headerHeight = charHeight * 2 + 10;
                    int listY = headerHeight + 10;
                    int detailsY = listY;
                    int rowHeight = charHeight + 6;
                    int detailsHeight = windowHeight - detailsY - 10;
                    size_t visibleRows = (detailsHeight / rowHeight);
                    // scroll to bottom
                    if (visibleRows < fields.size()) {
                        detailsScrollOffset = fields.size() - visibleRows;
                    } else {
                        detailsScrollOffset = 0;
                    }
                    detailsScrollbar.offset = detailsScrollOffset;
                    requestRedraw();
                    return;
                }
                // After moving selection upwards, ensure the selected field is visible
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
                    // wrap to first field
                    selectedField = 0;
                    detailsScrollOffset = 0;
                    detailsScrollbar.offset = 0;
                    requestRedraw();
                    return;
                }
                // After moving selection downwards, ensure the selected field is visible
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