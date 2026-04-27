//=============================================================================
//  Pokemon Trainer Editor
//=============================================================================

#ifndef POKEMON_TRAINER_H
#define POKEMON_TRAINER_H

#include "../common/sdl_app_base.h"
#include "../common/data_utils.h"
#include "../common/hex_utils.h"
#include "../common/generation3_utils.h"
#include "../common/generation2_utils.h"
#include "../common/generation1_utils.h"
#include "../encodings/text_encodings.h"
#include <vector>
#include <string>
#include <cstdint>
#include <array>


class PokemonTrainerEditor : public SDLAppBase {
public:
    enum class GameType {
        UNKNOWN,
        GEN1_RB,
        GEN1_YELLOW,
        GEN2_GS,
        GEN2_CRYSTAL,
        GEN3_RS,
        GEN3_EMERALD,
        GEN3_FRLG
    };

    struct Gen1PokemonInfo {
        size_t levelOffset{0};    // Offset of the Pokémon's level byte
        size_t speciesOffset{0};  // Offset of the Pokémon's species byte
    };

    struct Gen1TrainerData {
        bool format2{false};                 // True if Format 2 (multiple levels)
        std::vector<Gen1PokemonInfo> party;  // Per Pokémon level/species offsets
    };

    struct Gen2PokemonInfo {
        size_t levelOffset{0};    // File offset of the Pokémon's level byte
        size_t speciesOffset{0};  // File offset of the species byte
        bool hasItem{false};
        size_t itemOffset{0};     // File offset of the held item byte (if any)
        bool hasMoves{false};
        std::array<size_t,4> moveOffsets{{0,0,0,0}}; // Offsets for the four move bytes (if any)
    };

    struct Gen2TrainerData {
        std::vector<Gen2PokemonInfo> party;
    };

    struct Gen3TrainerData {
        uint8_t flags{0};         // Gender/double battle flags
        uint8_t sprite{0};        // Sprite ID
        std::array<uint16_t,4> items{{0,0,0,0}};  // Up to four held items
        std::array<uint8_t,4> unknown{{0,0,0,0}}; // Unused bytes (0x18-0x1B)
        uint32_t ai{0};           // AI or intelligence level
        uint32_t partyPointer{0}; // Raw pointer (Little-endian) into ROM
        size_t partyOffset{0};    // Calculated file offset (pointer - 0x08000000)
    };

    // Structure representing a single trainer. Common fields live at the
    // top level, while generation specific storage lives in dedicated blocks.
    struct TrainerEntry {
        size_t offset{0};         // Location of the trainer record
        uint8_t type{0};          // Trainer type / structure byte
        uint8_t classId{0};       // Index into the trainer class name table
        std::string name;         // Trainer name decoded from ROM text
        uint32_t partySize{0};    // Number of Pokémon in the party
        Gen1TrainerData gen1;
        Gen2TrainerData gen2;
        Gen3TrainerData gen3;
    };

private:
    // =====================================================================
    // ROM and file state
    // =====================================================================
    std::string fileBuffer;       // Entire ROM contents loaded from disk
    std::string fileName;         // Path passed on the command line
    size_t fileSize{0};           // Size of fileBuffer in bytes
    bool overwriteMode{false};    // Whether to overwrite the input file when saving
    bool hasUnsavedChanges{false}; // Tracks if any edits have been made

    // =====================================================================
    // Game state
    // =====================================================================
    std::string gameName;         // Normalized game identifier (ruby, sapphire, etc.)
    GameType gameType{GameType::UNKNOWN}; // Parsed game enumeration

    // =====================================================================
    // Trainer lists and UI state
    // =====================================================================
    std::vector<TrainerEntry> trainers;        // All parsed trainers
    std::vector<size_t> displayOrder;          // Indices into trainers for current sort/filter
    std::string searchTerm;                    // Lower-cased search string
    bool searchMode{false};                    // True when the search box is active
    
    // Sorting mode for the trainer list. Memory preserves the original
    // order of records in the ROM. Class sorts alphabetically by trainer
    // class. Name sorts alphabetically by trainer name.
    enum class SortMode { Memory, Class, Name };
    SortMode sortMode{SortMode::Memory};       // Current sorting mode
    size_t selectedIndex{0};                   // Index into displayOrder for selected trainer
    size_t selectedField{0};                   // Field index within the details pane
    bool viewingDetails{false};                // True when user is editing trainer details

    size_t detailsScrollOffset{0};

    bool editingValue{false};
    std::string editBuffer;

    ScrollbarState detailsScrollbar;
    int lastMouseX{0};
    int lastMouseY{0};

    Generation1Utils::TrainerMemoryAddresses trainer1Addresses{0,0,0,0,0};

    Generation2Utils::TrainerMemoryAddresses trainer2Addresses{0,0,0,0,0};

    Generation3Utils::TrainerMemoryAddresses trainer3Addresses{Generation3Utils::TRAINER_ADDRESSES_RUBY};

    std::vector<std::string> gen1ClassNames;
    std::vector<std::string> gen2ClassNames;

    bool isJapanese{false};


    enum class FieldKind {
        Name,
        Class,
        Type,
        Flags,
        Sprite,
        AI,
        PartySize,
        TrainerItem,
        PokemonLevel,
        PokemonSpecies,
        PokemonItem,
        PokemonMove
    };

    struct FieldDescriptor {
        FieldKind kind;
        size_t pokemonIndex{0};
        size_t subIndex{0};
        std::string label;
    };

    std::vector<FieldDescriptor> fields;

    // =====================================================================
    // Internal helpers
    // =====================================================================
    bool isGen1Game() const;
    bool isGen2Game() const;
    bool isGen3Game() const;
    bool parseTrainers();
    bool parseGen1Trainers();
    bool parseGen2Trainers();
    bool parseGen3Trainers();
    void buildGen1TrainerFields(const TrainerEntry& trainer);
    void buildGen2TrainerFields(const TrainerEntry& trainer);
    void buildGen3TrainerFields(const TrainerEntry& trainer);
    void buildTrainerFields(const TrainerEntry& trainer);
    std::string getGen1FieldValueString(const TrainerEntry& trainer, const FieldDescriptor& field, size_t fieldIndex) const;
    std::string getGen2FieldValueString(const TrainerEntry& trainer, const FieldDescriptor& field, size_t fieldIndex) const;
    std::string getGen3FieldValueString(const TrainerEntry& trainer, const FieldDescriptor& field, size_t fieldIndex) const;
    std::string getFieldValueString(const TrainerEntry& trainer, const FieldDescriptor& field, size_t fieldIndex) const;
    std::string decodeTrainerName(const std::vector<unsigned char>& bytes) const;
    std::string getTrainerClassName(uint8_t classId) const;
    void refreshDisplayOrder();
    void allocateNewParty(TrainerEntry& entry, uint32_t newPartySize);
    bool writeFile(const std::string& path);
    std::string getOutputPath() const;

protected:
    void render() override;
    void handleEvent(SDL_Event& event) override;
    void update(float deltaTime) override;

public:
    PokemonTrainerEditor();
    bool loadFile(const char* filename);
    bool setGame(const std::string& game);
    void setJapanese(bool jp) { isJapanese = jp; }
    void setOverwriteMode(bool b) { overwriteMode = b; }
};

#endif // POKEMON_TRAINER_H