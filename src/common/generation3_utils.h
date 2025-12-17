#ifndef GENERATION3_UTILS_H
#define GENERATION3_UTILS_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <array>
#include <utility>

namespace Generation3Utils {

// Gen 3 save structure constants
static constexpr size_t GEN3_SAVE_SIZE = 0x20000;        // 128KB save file
static constexpr size_t GEN3_BLOCK_SIZE = 0xE000;        // Each save block is 57344 bytes
static constexpr size_t GEN3_SECTION_SIZE = 0x1000;      // Each section is 4096 bytes
static constexpr size_t GEN3_NUM_SECTIONS = 14;

// Section structure offsets
static constexpr size_t GEN3_SECTION_ID_OFFSET = 0xFF4;
static constexpr size_t GEN3_SECTION_CHECKSUM_OFFSET = 0xFF6;
static constexpr size_t GEN3_SECTION_SAVE_INDEX_OFFSET = 0xFFC;

// Section sizes for each section ID (0-13)
constexpr size_t GEN3_SECTION_SIZES[14] = {
    3884,  // Section 0: Trainer Info
    3968,  // Section 1: Team / Items
    3968,  // Section 2: Game State
    3968,  // Section 3: Misc Data
    3848,  // Section 4: Rival Info
    3968,  // Section 5: PC Buffer A
    3968,  // Section 6: PC Buffer B
    3968,  // Section 7: PC Buffer C
    3968,  // Section 8: PC Buffer D
    3968,  // Section 9: PC Buffer E
    3968,  // Section 10: PC Buffer F
    3968,  // Section 11: PC Buffer G
    3968,  // Section 12: PC Buffer H
    2000   // Section 13: PC Buffer I
};

// Game type constants for security key handling
constexpr int GEN3_GAME_RS = 0;      // Ruby/Sapphire
constexpr int GEN3_GAME_EMERALD = 1;
constexpr int GEN3_GAME_FRLG = 2;    // FireRed/LeafGreen

// Security key offsets within Section 0
constexpr size_t GEN3_SECURITY_KEY_OFFSET_E = 0x00AC;     // Emerald
constexpr size_t GEN3_SECURITY_KEY_OFFSET_FRLG = 0x0AF8;  // FireRed/LeafGreen

// Party offsets within Section 1
constexpr size_t GEN3_PARTY_OFFSET_RSE = 0x0234;
constexpr size_t GEN3_PARTY_OFFSET_FRLG = 0x0034;
constexpr size_t GEN3_PARTY_MAX_SIZE = 600;  // Max party data size (6 * 100)
constexpr size_t GEN3_PARTY_POKEMON_SIZE = 100;
constexpr size_t BOX_POKEMON_SIZE = 80;

// Pokemon structure offsets
constexpr size_t POKEMON_PID_OFFSET = 0x00;
constexpr size_t POKEMON_OTID_OFFSET = 0x04;
constexpr size_t POKEMON_NICKNAME_OFFSET = 0x08;
constexpr size_t POKEMON_NICKNAME_LENGTH = 10;
constexpr size_t POKEMON_LANGUAGE_OFFSET = 0x12;
constexpr size_t POKEMON_MISC_FLAGS_OFFSET = 0x13;
constexpr size_t POKEMON_OT_NAME_OFFSET = 0x14;
constexpr size_t POKEMON_OT_NAME_LENGTH = 7;
constexpr size_t POKEMON_MARKINGS_OFFSET = 0x1B;
constexpr size_t POKEMON_CHECKSUM_OFFSET = 0x1C;
constexpr size_t POKEMON_DATA_OFFSET = 0x20;
constexpr size_t POKEMON_DATA_SIZE = 48;  // 12 words * 4 bytes
constexpr size_t POKEMON_STATUS_OFFSET = 0x50;
constexpr size_t POKEMON_LEVEL_OFFSET = 0x54;
constexpr size_t POKEMON_MAIL_ID_OFFSET = 0x55;
constexpr size_t POKEMON_CURRENT_HP_OFFSET = 0x56;
constexpr size_t POKEMON_MAX_HP_OFFSET = 0x58;
constexpr size_t POKEMON_ATTACK_OFFSET = 0x5A;
constexpr size_t POKEMON_DEFENSE_OFFSET = 0x5C;
constexpr size_t POKEMON_SPEED_OFFSET = 0x5E;
constexpr size_t POKEMON_SP_ATTACK_OFFSET = 0x60;
constexpr size_t POKEMON_SP_DEFENSE_OFFSET = 0x62;

// Box Pokemon size (unencrypted structure in PC)
constexpr size_t MAX_BOX_POKEMON = 420;  // 14 boxes * 30 slots

// Substructure identifiers
enum class Substructure { 
    GROWTH = 0,   // G
    ATTACKS = 1,  // A
    EVS = 2,      // E
    MISC = 3      // M
};

// Substructure order strings for display
static constexpr const char* SUBSTRUCTURE_ORDERS[24] = {
    "GAEM", "GAME", "GEAM", "GEMA", "GMAE", "GMEA",
    "AGEM", "AGME", "AEGM", "AEMG", "AMGE", "AMEG",
    "EGAM", "EGMA", "EAGM", "EAMG", "EMGA", "EMAG",
    "MGAE", "MGEA", "MAGE", "MAEG", "MEGA", "MEAG"
};

// Language codes
enum class Gen3Language : uint8_t {
    JAPANESE = 1,
    ENGLISH = 2,
    FRENCH = 3,
    ITALIAN = 4,
    GERMAN = 5,
    UNUSED = 6,
    SPANISH = 7
};

// Misc flags bit positions
constexpr uint8_t MISC_FLAG_BAD_EGG = 0x01;
constexpr uint8_t MISC_FLAG_HAS_SPECIES = 0x02;
constexpr uint8_t MISC_FLAG_USE_EGG_NAME = 0x04;
constexpr uint8_t MISC_FLAG_BLOCK_BOX_RS = 0x08;

// Status condition bits
constexpr uint32_t STATUS_SLEEP_MASK = 0x07;
constexpr uint32_t STATUS_POISON = 0x08;
constexpr uint32_t STATUS_BURN = 0x10;
constexpr uint32_t STATUS_FREEZE = 0x20;
constexpr uint32_t STATUS_PARALYSIS = 0x40;
constexpr uint32_t STATUS_BAD_POISON = 0x80;

// IV/Egg/Ability bit positions in the 32-bit field
constexpr uint32_t IV_HP_MASK = 0x0000001F;
constexpr uint32_t IV_ATTACK_MASK = 0x000003E0;
constexpr uint32_t IV_DEFENSE_MASK = 0x00007C00;
constexpr uint32_t IV_SPEED_MASK = 0x000F8000;
constexpr uint32_t IV_SPATK_MASK = 0x01F00000;
constexpr uint32_t IV_SPDEF_MASK = 0x3E000000;
constexpr uint32_t EGG_FLAG = 0x40000000;
constexpr uint32_t ABILITY_FLAG = 0x80000000;

// Nature constants and functions
static constexpr const char* NATURE_NAMES[25] = {
    "Hardy", "Lonely", "Brave", "Adamant", "Naughty",
    "Bold", "Docile", "Relaxed", "Impish", "Lax",
    "Timid", "Hasty", "Serious", "Jolly", "Naive",
    "Modest", "Mild", "Quiet", "Bashful", "Rash",
    "Calm", "Gentle", "Sassy", "Careful", "Quirky"
};


// Section info structure
struct SectionInfo {
    uint16_t sectionId{0};
    uint32_t saveIndex{0};
    size_t dataSize{0};
    size_t sectionBaseAddress{0};
    uint16_t calculatedChecksum{0};
    uint16_t storedChecksum{0};
    size_t checksumLocation{0};
    bool matches{false};
};

// Save block structure (contains 14 sections)
struct SaveBlock {
    SectionInfo sections[14];
    uint32_t saveIndex{0};
    bool valid{false};
};

// Pokemon checksum result for individual Pokemon verification
struct PokemonChecksumResult {
    size_t location{0};
    uint16_t calculated{0};
    uint16_t stored{0};
    bool valid{false};
    std::string locationStr;
};

// Decrypted Gen 3 Pokemon substructure data
struct Gen3SubstructureData {
    // Growth substructure
    uint16_t species{0};
    uint16_t heldItem{0};
    uint32_t experience{0};
    uint8_t ppBonuses{0};
    uint8_t friendship{0};
    
    // Attacks substructure
    uint16_t moves[4]{0, 0, 0, 0};
    uint8_t pp[4]{0, 0, 0, 0};
    
    // EVs & Condition substructure
    uint8_t hpEV{0};
    uint8_t attackEV{0};
    uint8_t defenseEV{0};
    uint8_t speedEV{0};
    uint8_t spAtkEV{0};
    uint8_t spDefEV{0};
    uint8_t coolness{0};
    uint8_t beauty{0};
    uint8_t cuteness{0};
    uint8_t smartness{0};
    uint8_t toughness{0};
    uint8_t feel{0};
    
    // Miscellaneous substructure
    uint8_t pokerus{0};
    uint8_t metLocation{0};
    uint16_t originsInfo{0};
    uint32_t ivsEggAbility{0};
    uint32_t ribbonsObedience{0};
};

// ============================================================================
// Cross-Section Reader
// ============================================================================

// Class for reading data that spans across multiple save sections
class CrossSectionReader {
public:
    using DataRange = std::pair<size_t, size_t>;
    
    CrossSectionReader(const std::string& buffer, const std::vector<DataRange>& ranges);
    
    std::vector<uint8_t> readBytes(size_t logicalOffset, size_t length) const;
    uint8_t readU8(size_t logicalOffset) const;
    uint16_t readU16LE(size_t logicalOffset) const;
    uint32_t readU32LE(size_t logicalOffset) const;
    size_t getPhysicalAddress(size_t logicalOffset) const;
    size_t getTotalSize() const;
    
private:
    const std::string& buffer_;
    std::vector<DataRange> ranges_;
    size_t totalSize_{0};
};

// ============================================================================
// Save Block Functions
// ============================================================================

// Parse save blocks from buffer
bool parseSaveBlocks(const std::string& buffer, SaveBlock& blockA, SaveBlock& blockB);

// Get the active (current) save block
const SaveBlock* getActiveSaveBlock(const SaveBlock& blockA, const SaveBlock& blockB);

// Find section offset by ID
size_t findSectionOffset(const SectionInfo* sections, uint16_t sectionId);
size_t findSectionOffset(const SectionInfo sections[], size_t numSections, uint16_t sectionId);

// Calculate section checksum
uint16_t calculateSectionChecksum(const std::string& buffer, size_t baseAddr, size_t dataSize);

// Update section checksum
void updateSectionChecksum(std::string& buffer, size_t sectionBaseAddr, size_t dataSize);

// Build data ranges for box Pokemon sections
std::vector<CrossSectionReader::DataRange> buildBoxDataRanges(const SaveBlock& saveBlock);

// ============================================================================
// Pokemon Data Functions
// ============================================================================

// Get Pokemon Personality ID
uint32_t getPID(const std::string& buffer, size_t pokemonBaseAddr);

// Get Pokemon Original Trainer ID
uint32_t getOTID(const std::string& buffer, size_t pokemonBaseAddr);

// Get decryption key (PID XOR OTID)
uint32_t getDecryptionKey(const std::string& buffer, size_t pokemonBaseAddr);
uint32_t getDecryptionKey(uint32_t pid, uint32_t otid);

// Get stored checksum from Pokemon structure
uint16_t getStoredPokemonChecksum(const std::string& buffer, size_t pokemonBaseAddr);

// Get nature index from PID (0-24)
uint8_t getNatureFromPID(uint32_t pid);

// Get nature name from PID
const char* getNatureName(uint32_t pid);

// Get nature name from index
const char* getNatureNameByIndex(uint8_t natureIndex);

// Lookup nature index by name (returns 255 if not found)
uint8_t lookupNatureByName(const std::string& name);

// Calculate a new PID that produces the desired nature while preserving other PID-derived traits
// Note: This will change the PID, which affects shininess, gender, ability, etc.
uint32_t calculatePIDForNature(uint32_t currentPID, uint8_t desiredNature);

// ============================================================================
// Substructure Functions
// ============================================================================

// Get substructure order index (0-23) for a given PID
int getSubstructureOrderIndex(uint32_t pid);

// Get the substructure order string (e.g., "GAEM") for a given PID
const char* getSubstructureOrderString(uint32_t pid);

// Get the byte offset of a specific substructure within the 48-byte data
// Returns offset relative to start of 48-byte data section (0, 12, 24, or 36)
int getSubstructureOffset(uint32_t pid, Substructure sub);

// ============================================================================
// Encryption/Decryption Functions
// ============================================================================

// Decrypt the 48-byte Pokemon data section
std::array<uint8_t, 48> decryptPokemonData(const std::string& buffer, size_t pokemonBaseAddr);
std::array<uint8_t, 48> decryptPokemonData(const std::string& buffer, size_t pokemonBaseAddr, uint32_t decryptionKey);

// Encrypt and write the 48-byte Pokemon data section
void encryptPokemonData(std::string& buffer, size_t pokemonBaseAddr, const std::array<uint8_t, 48>& data);
void encryptPokemonData(std::string& buffer, size_t pokemonBaseAddr, const std::array<uint8_t, 48>& data, uint32_t decryptionKey);

// Parse decrypted data into substructure fields
Gen3SubstructureData parseSubstructureData(const std::array<uint8_t, 48>& decryptedData, uint32_t pid);

// Build encrypted data from substructure fields
std::array<uint8_t, 48> buildSubstructureData(const Gen3SubstructureData& data, uint32_t pid);

// ============================================================================
// Pokemon Checksum Functions
// ============================================================================

// Calculate checksum for decrypted Pokemon data
uint16_t calculatePokemonDataChecksum(const std::array<uint8_t, 48>& decryptedData);

// Calculate checksum directly from buffer (handles decryption internally)
uint16_t calculatePokemonDataChecksum(const std::string& buffer, size_t pokemonBaseAddr, uint32_t decryptionKey);

// Calculate checksum for box Pokemon using CrossSectionReader
uint16_t calculateBoxPokemonChecksum(const CrossSectionReader& reader, size_t logicalOffset, uint32_t decryptionKey);

// Update the Pokemon's checksum in the buffer
void updatePokemonChecksum(std::string& buffer, size_t pokemonBaseAddr);

// Verify a Pokemon's checksum
bool verifyPokemonChecksum(const std::string& buffer, size_t pokemonBaseAddr);

// ============================================================================
// IV Extraction/Setting Functions
// ============================================================================

// Extract individual IVs from the combined 32-bit field
uint8_t getIVHP(uint32_t ivsEggAbility);
uint8_t getIVAttack(uint32_t ivsEggAbility);
uint8_t getIVDefense(uint32_t ivsEggAbility);
uint8_t getIVSpeed(uint32_t ivsEggAbility);
uint8_t getIVSpAtk(uint32_t ivsEggAbility);
uint8_t getIVSpDef(uint32_t ivsEggAbility);
bool isEgg(uint32_t ivsEggAbility);
bool hasSecondAbility(uint32_t ivsEggAbility);

// Set individual IVs in the combined 32-bit field
uint32_t setIVHP(uint32_t ivsEggAbility, uint8_t value);
uint32_t setIVAttack(uint32_t ivsEggAbility, uint8_t value);
uint32_t setIVDefense(uint32_t ivsEggAbility, uint8_t value);
uint32_t setIVSpeed(uint32_t ivsEggAbility, uint8_t value);
uint32_t setIVSpAtk(uint32_t ivsEggAbility, uint8_t value);
uint32_t setIVSpDef(uint32_t ivsEggAbility, uint8_t value);
uint32_t setEggFlag(uint32_t ivsEggAbility, bool isEgg);
uint32_t setAbilityFlag(uint32_t ivsEggAbility, bool secondAbility);

// ============================================================================
// Origins Info Functions
// ============================================================================

// Extract fields from origins info
uint8_t getLevelMet(uint16_t originsInfo);
uint8_t getGameOfOrigin(uint16_t originsInfo);
uint8_t getPokeBallCaughtIn(uint16_t originsInfo);
bool getOTGender(uint16_t originsInfo);

// Set fields in origins info
uint16_t setLevelMet(uint16_t originsInfo, uint8_t level);
uint16_t setGameOfOrigin(uint16_t originsInfo, uint8_t game);
uint16_t setPokeBallCaughtIn(uint16_t originsInfo, uint8_t ball);
uint16_t setOTGender(uint16_t originsInfo, bool female);

// ============================================================================
// Security/Encryption Functions
// ============================================================================

// Get security key for item quantity encryption
uint32_t getSecurityKey(const std::string& buffer, int game, size_t section0Offset);

// Decrypt/encrypt item quantity
uint16_t decryptItemQuantity(uint16_t encryptedQty, int game, uint32_t securityKey);
uint16_t encryptItemQuantity(uint16_t quantity, int game, uint32_t securityKey);

// ============================================================================
// Helper Functions
// ============================================================================

// Get language name from code
const char* getLanguageName(uint8_t languageCode);

// Get Poke Ball name from index
const char* getPokeBallName(uint8_t ballIndex);

// Get game of origin name from index
const char* getGameOfOriginName(uint8_t gameIndex);

// Get status condition string
std::string getStatusConditionString(uint32_t status);

} // namespace Generation3Utils

#endif // GENERATION3_UTILS_H