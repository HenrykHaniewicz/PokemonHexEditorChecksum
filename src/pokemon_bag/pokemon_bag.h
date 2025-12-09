#ifndef POKEMON_BAG_H
#define POKEMON_BAG_H

#include "../common/sdl_app_base.h"
#include "../common/data_utils.h"
#include "../common/hex_utils.h"
#include "../common/generation3_utils.h"
#include "../encodings/items_index_eng.h"
#include <vector>
#include <string>
#include <cstdint>

class PokemonBagEditor : public SDLAppBase {
public:
    // Enumeration of high-level game modes we support
    enum class GameType {
        UNKNOWN,
        GEN1,           // Red/Blue/Yellow
        GOLD_SILVER,    // Generation 2: Gold/Silver
        CRYSTAL,        // Generation 2: Crystal
        RUBY_SAPPHIRE,  // Generation 3: Ruby/Sapphire
        EMERALD,        // Generation 3: Emerald
        FIRERED_LEAFGREEN // Generation 3: FireRed/LeafGreen
    };

    struct BagSlot {
        uint16_t itemId{0};     // 16-bit for Gen 3 compatibility
        uint16_t quantity{0};   // 16-bit for Gen 3 compatibility
    };

    struct PocketInfo {
        std::string name;
        size_t capacity;
        bool hasQuantity;
        bool idStored;
        bool idEditable;
        bool quantityEditable;
        bool nameEditable;
        size_t primaryOffset;
        size_t secondaryOffset;
        std::vector<BagSlot> slots;
        std::vector<size_t> originalIndices;
    };

private:
    std::string fileBuffer;
    std::string fileName;
    size_t fileSize{0};
    std::string gameName;
    GameType gameType{GameType::UNKNOWN};

    // Flags
    bool isJapanese{false};
    bool overwriteMode{false};
    bool hasUnsavedChanges{false};

    // Parsed pockets
    std::vector<PocketInfo> pockets;
    int currentPocket{0};

    // UI state for current pocket
    int selectedIndex{0};
    bool editing{false};
    int editingField{0};  // 0 = ID, 1 = quantity, 2 = name
    std::string editBuffer;
    SDL_Rect saveButtonRect{};
    bool saveButtonHovered{false};

    // Gen 3 specific data
    size_t gen3CurrentSaveOffset{0};      // Offset to current save block (0 or 0xE000)
    size_t gen3Section0Offset{0};         // Absolute offset to Section ID 0
    size_t gen3Section1Offset{0};         // Absolute offset to Section ID 1
    uint32_t gen3SecurityKey{0};          // Security key for E/FRLG encryption
    int gen3GameType{0};                  // 0=RS, 1=E, 2=FRLG
    Generation3Utils::SectionInfo gen3Sections[Generation3Utils::GEN3_NUM_SECTIONS];

    // === Parsing and writing helpers ===
    bool parseAllPockets();
    bool parsePocket(PocketInfo& pocket);
    void writeAllPocketsToBuffer();
    void writePocketToBuffer(const PocketInfo& pocket);

    // Gen 3 specific helpers
    bool findGen3CurrentSave();
    bool parseGen3Sections();
    bool parseGen3Pocket(PocketInfo& pocket);
    void writeGen3PocketToBuffer(const PocketInfo& pocket);
    bool isGen3Game() const;

    // Checksum updaters
    void updateChecksumGen1();
    void updateChecksumGen2();
    void updateChecksumGen3();
    void updateChecksum();

    // File I/O helpers
    bool fileExists(const std::string& path);
    std::string getOutputPath();
    bool saveFile();

    // Item manipulation helpers
    void removeItem(int index);
    void startEditing(int index);
    void handleEditInput(SDL_Keycode key);
    void shiftCurrentPocket();
    bool itemExistsForPocket(uint16_t id, const PocketInfo& pocket) const;
    const char* getItemName(uint16_t id) const;
    uint16_t lookupItemIdByName(const std::string& name, const PocketInfo& pocket) const;

protected:
    // SDLAppBase overrides
    void render() override;
    void handleEvent(SDL_Event& event) override;
    void update(float deltaTime) override;

public:
    PokemonBagEditor();
    // Loads the file into memory; returns false on failure
    bool loadFile(const char* filename);
    // Configures the editor based on the chosen game.  This must be
    // called after loadFile() so that pockets can be parsed.  Returns
    // false on unknown games.
    bool setGame(const std::string& game);
    // Set Japanese mode (affects offsets and names)
    void setJapanese(bool jp) { isJapanese = jp; }
    // Set overwrite mode (write edits in place vs. edited_files/)
    void setOverwriteMode(bool overwrite) { overwriteMode = overwrite; }
};

#endif // POKEMON_BAG_H