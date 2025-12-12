#ifndef POKEMON_PARTY_H
#define POKEMON_PARTY_H

#include "../common/sdl_app_base.h"
#include "../common/data_utils.h"
#include "../common/hex_utils.h"
#include "../encodings/pokemon_index_eng.h"
#include "../encodings/moves_index_eng.h"
#include "../encodings/types_index_eng.h"
#include "../encodings/items_index_eng.h"
#include "../encodings/text_encodings.h"
#include <vector>
#include <string>
#include <cstdint>
#include <array>

class PokemonPartyEditor : public SDLAppBase {
public:
    // Enumeration of high-level game modes we support
    enum class GameType {
        UNKNOWN,
        GEN1,           // Red/Blue/Yellow/Green
        GEN2_GS,        // Gold/Silver
        GEN2_CRYSTAL,   // Crystal
        GEN3_RS,        // Ruby/Sapphire
        GEN3_EMERALD,   // Emerald
        GEN3_FRLG       // FireRed/LeafGreen
    };

    // Gen 1 offsets
    static constexpr size_t GEN1_PARTY_OFFSET_ENG = 0x2F2C;
    static constexpr size_t GEN1_PARTY_OFFSET_JPN = 0x2ED5;
    
    // Gen 2 offsets
    static constexpr size_t GEN2_GS_PARTY_OFFSET_ENG = 0x288A;
    static constexpr size_t GEN2_GS_PARTY_OFFSET_JPN = 0x283E;
    static constexpr size_t GEN2_CRYSTAL_PARTY_OFFSET_ENG = 0x2865;
    static constexpr size_t GEN2_CRYSTAL_PARTY_OFFSET_JPN = 0x281A;
    
    static constexpr size_t MAX_PARTY_SIZE = 6;
    
    // Data sizes vary by generation
    static constexpr size_t GEN1_POKEMON_DATA_SIZE = 0x2C;
    static constexpr size_t GEN2_POKEMON_DATA_SIZE = 0x30; // 48 bytes
    
    static constexpr size_t NAME_LENGTH_ENG = 11;  // 10 chars + terminator
    static constexpr size_t NAME_LENGTH_JPN = 6;   // 5 chars + terminator

    struct PokemonData {
        // Common fields
        uint8_t species{0};
        uint16_t currentHP{0};
        uint8_t level{0};
        uint8_t status{0};
        std::array<uint8_t, 4> moves{0, 0, 0, 0};
        uint16_t trainerID{0};
        uint32_t exp{0};
        std::array<uint8_t, 4> ppValues{0, 0, 0, 0};
        
        // Gen 1 specific
        uint8_t levelBox{0};  // Level stored in box data
        uint8_t type1{0};
        uint8_t type2{0};
        uint8_t catchRate{0};
        uint16_t special{0};  // Gen 1 has combined Special stat
        
        // Gen 2 specific
        uint8_t heldItem{0};
        uint8_t friendship{0};
        uint8_t pokerus{0};
        uint16_t caughtData{0};
        uint16_t specialAttack{0};
        uint16_t specialDefense{0};
        
        // Common stats
        uint16_t hpEV{0};
        uint16_t attackEV{0};
        uint16_t defenseEV{0};
        uint16_t speedEV{0};
        uint16_t specialEV{0};  // Gen 1 or Gen 2 Special
        uint16_t ivData{0};
        uint16_t maxHP{0};
        uint16_t attack{0};
        uint16_t defense{0};
        uint16_t speed{0};
        
        // Decoded names
        std::string nickname;
        std::string otName;  // Original Trainer name
        
        bool isEmpty() const { return species == 0 || species == 0xFF; }
    };

    enum class EditField {
        SPECIES = 0,
        LEVEL,
        CURRENT_HP,
        MAX_HP,
        STATUS,
        TYPE1,          // Gen 1 only
        TYPE2,          // Gen 1 only
        HELD_ITEM,      // Gen 2+ only
        MOVE1,
        MOVE2,
        MOVE3,
        MOVE4,
        PP1,
        PP2,
        PP3,
        PP4,
        ATTACK,
        DEFENSE,
        SPEED,
        SPECIAL,        // Gen 1 only
        SPECIAL_ATK,    // Gen 2+ only
        SPECIAL_DEF,    // Gen 2+ only
        HP_EV,
        ATTACK_EV,
        DEFENSE_EV,
        SPEED_EV,
        SPECIAL_EV,
        FRIENDSHIP,     // Gen 2+ only
        POKERUS,        // Gen 2+ only
        NICKNAME,
        OT_NAME,
        EXP,
        FIELD_COUNT
    };

private:
    std::string fileBuffer;
    std::string fileName;
    size_t fileSize{0};
    std::string gameName;
    GameType gameType{GameType::UNKNOWN};
    int generation{0};  // 1, 2, or 3
    
    // Flags
    bool isJapanese{false};
    bool overwriteMode{false};
    bool hasUnsavedChanges{false};
    
    // Party data
    uint8_t partyCount{0};
    std::array<uint8_t, 7> partySpecies{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // +1 for terminator
    std::array<PokemonData, 6> partyPokemon;
    
    // UI state
    int currentPokemonIndex{0};
    int selectedField{0};
    bool editing{false};
    bool editingByName{false};  // True when editing by name (via 'i' key)
    std::string editBuffer;
    SDL_Rect saveButtonRect{};
    bool saveButtonHovered{false};

    TextEncoding encoding;
    
    // Helper functions
    size_t getPartyOffset() const;
    size_t getPokemonDataSize() const;
    size_t getNameLength() const { return isJapanese ? NAME_LENGTH_JPN : NAME_LENGTH_ENG; }
    size_t getMaxNameChars() const { return isJapanese ? 5 : 10; }
    void parsePokemonData();
    void writePokemonDataToBuffer();
    void updateChecksum();
    void updateChecksumGen1();
    void updateChecksumGen2();
    
    void setEncodingForGame();

    void parseGen1Pokemon(PokemonData& pkmn, size_t offset);
    void parseGen2Pokemon(PokemonData& pkmn, size_t offset);
    void writeGen1Pokemon(const PokemonData& pkmn, size_t offset);
    void writeGen2Pokemon(const PokemonData& pkmn, size_t offset);
    
    // Field helpers
    const char* getFieldName(EditField field) const;
    std::string getFieldValue(int pokemonIndex, EditField field) const;
    bool isFieldEditable(EditField field) const;
    bool isFieldVisible(EditField field) const;
    bool isNameEditableField(EditField field) const;
    void startEditing(EditField field, bool byName = false);
    void handleEditInput(SDL_Keycode key);
    void commitEdit();
    bool validateAndApplyEdit(int pokemonIndex, EditField field, const std::string& value);
    
    // Name lookup helpers
    uint8_t lookupPokemonIdByName(const std::string& name) const;
    uint8_t lookupMoveIdByName(const std::string& name) const;
    uint8_t lookupItemIdByName(const std::string& name) const;
    
    // Pokemon helpers
    std::string getPokemonTabName(int index) const;
    const char* getStatusName(uint8_t status) const;
    const char* getTypeName(uint8_t type) const;
    const char* getMoveName(uint8_t move) const;
    const char* getItemName(uint8_t item) const;
    
    // IV/DV helpers
    uint8_t getIV(uint16_t ivData, const std::string& stat) const;
    uint16_t setIV(uint16_t ivData, const std::string& stat, uint8_t value) const;
    
    // File I/O helpers
    bool fileExists(const std::string& path);
    std::string getOutputPath();
    bool saveFile();

protected:
    // SDLAppBase overrides
    void render() override;
    void handleEvent(SDL_Event& event) override;
    void update(float deltaTime) override;

public:
    PokemonPartyEditor();
    bool loadFile(const char* filename);
    bool setGame(const std::string& game);
    void setJapanese(bool jp) { isJapanese = jp; }
    void setOverwriteMode(bool overwrite) { overwriteMode = overwrite; }
};

#endif // POKEMON_PARTY_H