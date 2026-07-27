#ifndef POKEMON_PARTY_H
#define POKEMON_PARTY_H

#include "../common/sdl_app_base.h"
#include "../common/data_utils.h"
#include "../common/hex_utils.h"
#include "../common/generation1_utils.h"
#include "../common/generation2_utils.h"
#include "../common/generation3_utils.h"
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
    enum class GameType {
        UNKNOWN,
        GEN1,
        GEN2_GS,
        GEN2_CRYSTAL,
        GEN3_RS,
        GEN3_EMERALD,
        GEN3_FRLG
    };

    static constexpr size_t GEN1_PARTY_OFFSET_ENG = 0x2F2C;
    static constexpr size_t GEN1_PARTY_OFFSET_JPN = 0x2ED5;

    static constexpr size_t GEN2_GS_PARTY_OFFSET_ENG = 0x288A;
    static constexpr size_t GEN2_GS_PARTY_OFFSET_JPN = 0x283E;
    static constexpr size_t GEN2_CRYSTAL_PARTY_OFFSET_ENG = 0x2865;
    static constexpr size_t GEN2_CRYSTAL_PARTY_OFFSET_JPN = 0x281A;

    static constexpr size_t MAX_PARTY_SIZE = 6;

    static constexpr size_t GEN1_POKEMON_DATA_SIZE = 0x2C;
    static constexpr size_t GEN2_POKEMON_DATA_SIZE = 0x30;
    static constexpr size_t GEN3_POKEMON_DATA_SIZE = 100;

    static constexpr size_t NAME_LENGTH_ENG = 11;
    static constexpr size_t NAME_LENGTH_JPN = 6;
    static constexpr size_t GEN3_NAME_LENGTH = 10;
    static constexpr size_t GEN3_OT_NAME_LENGTH = 7;

    struct PokemonData {
        uint8_t species{0};
        uint16_t speciesGen3{0};
        uint16_t currentHP{0};
        uint8_t level{0};
        uint8_t status{0};
        std::array<uint8_t, 4> moves{0, 0, 0, 0};
        std::array<uint16_t, 4> movesGen3{0, 0, 0, 0};
        uint16_t trainerID{0};
        uint32_t otIdFull{0};
        uint32_t exp{0};
        std::array<uint8_t, 4> ppValues{0, 0, 0, 0};

        uint8_t levelBox{0};
        uint8_t type1{0};
        uint8_t type2{0};
        uint8_t catchRate{0};
        uint16_t special{0};

        uint8_t heldItem{0};
        uint16_t heldItemGen3{0};
        uint8_t friendship{0};
        uint8_t pokerus{0};
        uint16_t caughtData{0};
        uint16_t specialAttack{0};
        uint16_t specialDefense{0};

        uint16_t hpEV{0};
        uint16_t attackEV{0};
        uint16_t defenseEV{0};
        uint16_t speedEV{0};
        uint16_t specialEV{0};
        uint8_t spAtkEV{0};
        uint8_t spDefEV{0};
        uint16_t ivData{0};
        uint16_t maxHP{0};
        uint16_t attack{0};
        uint16_t defense{0};
        uint16_t speed{0};

        uint32_t personalityValue{0};
        uint8_t originalNature{0};
        uint8_t language{0};
        uint8_t miscFlags{0};
        uint8_t markings{0};
        uint32_t statusCondition{0};
        uint8_t mailId{0};
        uint8_t ppBonuses{0};

        uint32_t ivsEggAbility{0};

        uint16_t originsInfo{0};
        uint8_t metLocation{0};

        uint8_t coolness{0};
        uint8_t beauty{0};
        uint8_t cuteness{0};
        uint8_t smartness{0};
        uint8_t toughness{0};
        uint8_t feel{0};

        uint32_t ribbonsObedience{0};

        std::string nickname;
        std::string otName;

        bool isEmpty() const {
            return (species == 0 || species == 0xFF) && (speciesGen3 == 0);
        }
    };

    enum class EditField {
        SPECIES = 0,
        LEVEL,
        CURRENT_HP,
        MAX_HP,
        STATUS,
        TYPE1,
        TYPE2,
        HELD_ITEM,
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
        SPECIAL,
        SPECIAL_ATK,
        SPECIAL_DEF,
        HP_EV,
        ATTACK_EV,
        DEFENSE_EV,
        SPEED_EV,
        SPECIAL_EV,
        SP_ATK_EV,
        SP_DEF_EV,
        DV_ATTACK,
        DV_DEFENSE,
        DV_SPEED,
        DV_SPECIAL,
        DV_HP,

        FRIENDSHIP,
        POKERUS,
        NICKNAME,
        OT_NAME,
        EXP,

        PID_DISPLAY,
        SUBSTRUCTURE_ORDER,
        NATURE,
        OT_ID,
        LANGUAGE,
        MISC_FLAGS,
        MARKINGS,

        IV_HP,
        IV_ATTACK,
        IV_DEFENSE,
        IV_SPEED,
        IV_SP_ATK,
        IV_SP_DEF,

        IS_EGG,
        ABILITY_FLAG,

        MET_LOCATION,
        LEVEL_MET,
        GAME_OF_ORIGIN,
        POKEBALL,
        OT_GENDER,

        COOLNESS,
        BEAUTY,
        CUTENESS,
        SMARTNESS,
        TOUGHNESS,
        FEEL,

        RIBBONS_DISPLAY,

        FIELD_COUNT
    };

private:
    std::string fileBuffer;
    std::string fileName;
    size_t fileSize{0};
    std::string gameName;
    GameType gameType{GameType::UNKNOWN};
    int generation{0};

    bool isJapanese{false};
    bool overwriteMode{false};
    bool hasUnsavedChanges{false};

    uint8_t partyCount{0};
    std::array<uint8_t, 7> partySpecies{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    std::array<PokemonData, 6> partyPokemon;
    std::array<uint8_t, 6> originalPartySpecies{0, 0, 0, 0, 0, 0};
    std::array<uint16_t, 6> originalPartySpeciesGen3{0, 0, 0, 0, 0, 0};

    Generation3Utils::SaveBlock gen3BlockA;
    Generation3Utils::SaveBlock gen3BlockB;
    const Generation3Utils::SaveBlock* activeGen3Block{nullptr};
    size_t gen3Section1Offset{0};
    size_t gen3PartyOffset{0};

    int currentPokemonIndex{0};
    int selectedField{0};
    bool editing{false};
    bool editingByName{false};
    std::string editBuffer;
    SDL_Rect saveButtonRect{};
    bool saveButtonHovered{false};

    TextEncoding encoding;

    size_t getPartyOffset() const;
    size_t getPokemonDataSize() const;
    size_t getNameLength() const;
    size_t getMaxNameChars() const;
    void parsePokemonData();
    void compactPartyData();
    void writePokemonDataToBuffer();
    void updateChecksum();
    void updateChecksumGen1();
    void updateChecksumGen2();
    void updateChecksumGen3();

    void setEncodingForGame();

    void parseGen1Pokemon(PokemonData& pkmn, size_t offset);
    void parseGen2Pokemon(PokemonData& pkmn, size_t offset);
    void parseGen3Pokemon(PokemonData& pkmn, size_t offset);
    void writeGen1Pokemon(const PokemonData& pkmn, size_t offset);
    void writeGen2Pokemon(const PokemonData& pkmn, size_t offset);
    void writeGen3Pokemon(const PokemonData& pkmn, size_t offset);

    bool initGen3SaveStructure();
    int getGen3GameType() const;

    const char* getFieldName(EditField field) const;
    std::string getFieldValue(int pokemonIndex, EditField field) const;
    bool isFieldEditable(EditField field) const;
    bool isFieldVisible(EditField field) const;
    bool isNameEditableField(EditField field) const;
    void startEditing(EditField field, bool byName = false);
    void cancelEditAndRedraw();
    void handleEditInput(SDL_Keycode key);
    void commitEdit();
    bool validateAndApplyEdit(int pokemonIndex, EditField field, const std::string& value);

    enum class LookupType { POKEMON, MOVE, ITEM };
    uint16_t lookupIdByName(LookupType type, const std::string& name) const;

    std::string getPokemonTabName(int index) const;
    const char* getStatusName(uint8_t status) const;
    std::string getGen3StatusName(uint32_t status) const;
    const char* getTypeName(uint8_t type) const;
    const char* getMoveName(uint8_t move) const;
    const char* getMoveNameGen3(uint16_t move) const;
    const char* getItemName(uint8_t item) const;
    const char* getItemNameGen3(uint16_t item) const;

    void updatePokedexForNewPokemon();
    void updatePokedexGen1();
    void updatePokedexGen2();
    void updatePokedexGen3();

    bool isPokedexBitSet(const std::string& buffer, size_t offset, uint16_t pokedexNum) const;
    void setPokedexBit(std::string& buffer, size_t offset, uint16_t pokedexNum);
    void setPokedexBitGen3(std::string& buffer, size_t sectionOffset,
                           size_t dataOffset, uint16_t pokedexNum);
    bool isPokedexBitSetGen3(const std::string& buffer, size_t sectionOffset,
                              size_t dataOffset, uint16_t pokedexNum) const;

    uint16_t getPokedexNumber(uint16_t speciesId) const;

    uint8_t getIV(uint16_t ivData, const std::string& stat) const;
    uint16_t setIV(uint16_t ivData, const std::string& stat, uint8_t value) const;

    bool fileExists(const std::string& path);
    std::string getOutputPath();
    bool saveFile();

    void adjustScrollbarForSelectedField();

protected:
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