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
        size_t levelOffset{0};
        size_t speciesOffset{0};
    };

    struct Gen1TrainerData {
        bool format2{false};
        std::vector<Gen1PokemonInfo> party;
    };

    struct Gen2PokemonInfo {
        size_t levelOffset{0};
        size_t speciesOffset{0};
        bool hasItem{false};
        size_t itemOffset{0};
        bool hasMoves{false};
        std::array<size_t,4> moveOffsets{{0,0,0,0}};
    };

    struct Gen2TrainerData {
        std::vector<Gen2PokemonInfo> party;
    };

    struct Gen3TrainerData {
        uint8_t flags{0};
        uint8_t sprite{0};
        std::array<uint16_t,4> items{{0,0,0,0}};
        std::array<uint8_t,4> unknown{{0,0,0,0}};
        uint32_t ai{0};
        uint32_t partyPointer{0};
        size_t partyOffset{0};
    };

    struct TrainerEntry {
        size_t offset{0};
        uint8_t type{0};
        uint8_t classId{0};
        std::string name;
        uint32_t partySize{0};
        Gen1TrainerData gen1;
        Gen2TrainerData gen2;
        Gen3TrainerData gen3;
    };

private:
    std::string fileBuffer;
    std::string fileName;
    size_t fileSize{0};
    bool overwriteMode{false};
    bool hasUnsavedChanges{false};

    std::string gameName;
    GameType gameType{GameType::UNKNOWN};

    std::vector<TrainerEntry> trainers;
    std::vector<size_t> displayOrder;
    std::string searchTerm;
    bool searchMode{false};
    
    enum class SortMode { Memory, Class, Name };
    SortMode sortMode{SortMode::Memory};
    size_t selectedIndex{0};
    size_t selectedField{0};
    bool viewingDetails{false};

    size_t detailsScrollOffset{0};

    bool editingValue{false};
    bool editingByName{false};  // Track if we're in name-based editing mode
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

    // Helper methods for editing
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
    
    // Editing helper methods
    void startEditing(bool byName);
    void cancelEditing();
    bool tryLookupByName(const TrainerEntry& tr, const FieldDescriptor& fd, std::string& result);
    bool applyEdit(TrainerEntry& tr, const FieldDescriptor& fd, const std::string& value);
    void handleEditInput(SDL_Keycode key);

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