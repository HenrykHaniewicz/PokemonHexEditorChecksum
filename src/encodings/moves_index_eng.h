#ifndef MOVES_INDEX_ENG_H
#define MOVES_INDEX_ENG_H

#include <cstdint>
#include <unordered_map>

namespace PokemonMoves {

static const std::unordered_map<uint8_t, const char*> GEN1_MOVES = {
    {0x00, "-"},
    {0x01, "Pound"},
    {0x02, "Karate Chop"},
    {0x03, "Double Slap"},
    {0x04, "Comet Punch"},
    {0x05, "Mega Punch"},
    {0x06, "Pay Day"},
    {0x07, "Fire Punch"},
    {0x08, "Ice Punch"},
    {0x09, "Thunder Punch"},
    {0x0A, "Scratch"},
    {0x0B, "Vice Grip"},
    {0x0C, "Guillotine"},
    {0x0D, "Razor Wind"},
    {0x0E, "Swords Dance"},
    {0x0F, "Cut"},
    {0x10, "Gust"},
    {0x11, "Wing Attack"},
    {0x12, "Whirlwind"},
    {0x13, "Fly"},
    {0x14, "Bind"},
    {0x15, "Slam"},
    {0x16, "Vine Whip"},
    {0x17, "Stomp"},
    {0x18, "Double Kick"},
    {0x19, "Mega Kick"},
    {0x1A, "Jump Kick"},
    {0x1B, "Rolling Kick"},
    {0x1C, "Sand Attack"},
    {0x1D, "Headbutt"},
    {0x1E, "Horn Attack"},
    {0x1F, "Fury Attack"},
    {0x20, "Horn Drill"},
    {0x21, "Tackle"},
    {0x22, "Body Slam"},
    {0x23, "Wrap"},
    {0x24, "Take Down"},
    {0x25, "Thrash"},
    {0x26, "Double-Edge"},
    {0x27, "Tail Whip"},
    {0x28, "Poison Sting"},
    {0x29, "Twineedle"},
    {0x2A, "Pin Missile"},
    {0x2B, "Leer"},
    {0x2C, "Bite"},
    {0x2D, "Growl"},
    {0x2E, "Roar"},
    {0x2F, "Sing"},
    {0x30, "Supersonic"},
    {0x31, "Sonic Boom"},
    {0x32, "Disable"},
    {0x33, "Acid"},
    {0x34, "Ember"},
    {0x35, "Flamethrower"},
    {0x36, "Mist"},
    {0x37, "Water Gun"},
    {0x38, "Hydro Pump"},
    {0x39, "Surf"},
    {0x3A, "Ice Beam"},
    {0x3B, "Blizzard"},
    {0x3C, "Psybeam"},
    {0x3D, "Bubble Beam"},
    {0x3E, "Aurora Beam"},
    {0x3F, "Hyper Beam"},
    {0x40, "Peck"},
    {0x41, "Drill Peck"},
    {0x42, "Submission"},
    {0x43, "Low Kick"},
    {0x44, "Counter"},
    {0x45, "Seismic Toss"},
    {0x46, "Strength"},
    {0x47, "Absorb"},
    {0x48, "Mega Drain"},
    {0x49, "Leech Seed"},
    {0x4A, "Growth"},
    {0x4B, "Razor Leaf"},
    {0x4C, "Solar Beam"},
    {0x4D, "Poison Powder"},
    {0x4E, "Stun Spore"},
    {0x4F, "Sleep Powder"},
    {0x50, "Petal Dance"},
    {0x51, "String Shot"},
    {0x52, "Dragon Rage"},
    {0x53, "Fire Spin"},
    {0x54, "Thunder Shock"},
    {0x55, "Thunderbolt"},
    {0x56, "Thunder Wave"},
    {0x57, "Thunder"},
    {0x58, "Rock Throw"},
    {0x59, "Earthquake"},
    {0x5A, "Fissure"},
    {0x5B, "Dig"},
    {0x5C, "Toxic"},
    {0x5D, "Confusion"},
    {0x5E, "Psychic"},
    {0x5F, "Hypnosis"},
    {0x60, "Meditate"},
    {0x61, "Agility"},
    {0x62, "Quick Attack"},
    {0x63, "Rage"},
    {0x64, "Teleport"},
    {0x65, "Night Shade"},
    {0x66, "Mimic"},
    {0x67, "Screech"},
    {0x68, "Double Team"},
    {0x69, "Recover"},
    {0x6A, "Harden"},
    {0x6B, "Minimize"},
    {0x6C, "Smokescreen"},
    {0x6D, "Confuse Ray"},
    {0x6E, "Withdraw"},
    {0x6F, "Defense Curl"},
    {0x70, "Barrier"},
    {0x71, "Light Screen"},
    {0x72, "Haze"},
    {0x73, "Reflect"},
    {0x74, "Focus Energy"},
    {0x75, "Bide"},
    {0x76, "Metronome"},
    {0x77, "Mirror Move"},
    {0x78, "Self-Destruct"},
    {0x79, "Egg Bomb"},
    {0x7A, "Lick"},
    {0x7B, "Smog"},
    {0x7C, "Sludge"},
    {0x7D, "Bone Club"},
    {0x7E, "Fire Blast"},
    {0x7F, "Waterfall"},
    {0x80, "Clamp"},
    {0x81, "Swift"},
    {0x82, "Skull Bash"},
    {0x83, "Spike Cannon"},
    {0x84, "Constrict"},
    {0x85, "Amnesia"},
    {0x86, "Kinesis"},
    {0x87, "Soft-Boiled"},
    {0x88, "High Jump Kick"},
    {0x89, "Glare"},
    {0x8A, "Dream Eater"},
    {0x8B, "Poison Gas"},
    {0x8C, "Barrage"},
    {0x8D, "Leech Life"},
    {0x8E, "Lovely Kiss"},
    {0x8F, "Sky Attack"},
    {0x90, "Transform"},
    {0x91, "Bubble"},
    {0x92, "Dizzy Punch"},
    {0x93, "Spore"},
    {0x94, "Flash"},
    {0x95, "Psywave"},
    {0x96, "Splash"},
    {0x97, "Acid Armor"},
    {0x98, "Crabhammer"},
    {0x99, "Explosion"},
    {0x9A, "Fury Swipes"},
    {0x9B, "Bonemerang"},
    {0x9C, "Rest"},
    {0x9D, "Rock Slide"},
    {0x9E, "Hyper Fang"},
    {0x9F, "Sharpen"},
    {0xA0, "Conversion"},
    {0xA1, "Tri Attack"},
    {0xA2, "Super Fang"},
    {0xA3, "Slash"},
    {0xA4, "Substitute"},
    {0xA5, "Struggle"},
};

// Helper function to build combined generation maps
static std::unordered_map<uint8_t, const char*> buildGenerationMap(
    const std::unordered_map<uint8_t, const char*>& prevGen,
    const std::unordered_map<uint8_t, const char*>& newMoves) {
    std::unordered_map<uint8_t, const char*> combined = prevGen;
    combined.insert(newMoves.begin(), newMoves.end());
    return combined;
}

// New moves introduced in Generation 2
static const std::unordered_map<uint8_t, const char*> GEN2_NEW_MOVES = {
    {0xA6, "Sketch"},
    {0xA7, "Triple Kick"},
    {0xA8, "Thief"},
    {0xA9, "Spider Web"},
    {0xAA, "Mind Reader"},
    {0xAB, "Nightmare"},
    {0xAC, "Flame Wheel"},
    {0xAD, "Snore"},
    {0xAE, "Curse"},
    {0xAF, "Flail"},
    {0xB0, "Conversion 2"},
    {0xB1, "Aeroblast"},
    {0xB2, "Cotton Spore"},
    {0xB3, "Reversal"},
    {0xB4, "Spite"},
    {0xB5, "Powder Snow"},
    {0xB6, "Protect"},
    {0xB7, "Mach Punch"},
    {0xB8, "Scary Face"},
    {0xB9, "Feint Attack"},
    {0xBA, "Sweet Kiss"},
    {0xBB, "Belly Drum"},
    {0xBC, "Sludge Bomb"},
    {0xBD, "Mud-Slap"},
    {0xBE, "Octazooka"},
    {0xBF, "Spikes"},
    {0xC0, "Zap Cannon"},
    {0xC1, "Foresight"},
    {0xC2, "Destiny Bond"},
    {0xC3, "Perish Song"},
    {0xC4, "Icy Wind"},
    {0xC5, "Detect"},
    {0xC6, "Bone Rush"},
    {0xC7, "Lock-On"},
    {0xC8, "Outrage"},
    {0xC9, "Sandstorm"},
    {0xCA, "Giga Drain"},
    {0xCB, "Endure"},
    {0xCC, "Charm"},
    {0xCD, "Rollout"},
    {0xCE, "False Swipe"},
    {0xCF, "Swagger"},
    {0xD0, "Milk Drink"},
    {0xD1, "Spark"},
    {0xD2, "Fury Cutter"},
    {0xD3, "Steel Wing"},
    {0xD4, "Mean Look"},
    {0xD5, "Attract"},
    {0xD6, "Sleep Talk"},
    {0xD7, "Heal Bell"},
    {0xD8, "Return"},
    {0xD9, "Present"},
    {0xDA, "Frustration"},
    {0xDB, "Safeguard"},
    {0xDC, "Pain Split"},
    {0xDD, "Sacred Fire"},
    {0xDE, "Magnitude"},
    {0xDF, "Dynamic Punch"},
    {0xE0, "Megahorn"},
    {0xE1, "Dragon Breath"},
    {0xE2, "Baton Pass"},
    {0xE3, "Encore"},
    {0xE4, "Pursuit"},
    {0xE5, "Rapid Spin"},
    {0xE6, "Sweet Scent"},
    {0xE7, "Iron Tail"},
    {0xE8, "Metal Claw"},
    {0xE9, "Vital Throw"},
    {0xEA, "Morning Sun"},
    {0xEB, "Synthesis"},
    {0xEC, "Moonlight"},
    {0xED, "Hidden Power"},
    {0xEE, "Cross Chop"},
    {0xEF, "Twister"},
    {0xF0, "Rain Dance"},
    {0xF1, "Sunny Day"},
    {0xF2, "Crunch"},
    {0xF3, "Mirror Coat"},
    {0xF4, "Psych Up"},
    {0xF5, "Extreme Speed"},
    {0xF6, "Ancient Power"},
    {0xF7, "Shadow Ball"},
    {0xF8, "Future Sight"},
    {0xF9, "Rock Smash"},
    {0xFA, "Whirlpool"},
    {0xFB, "Beat Up"},
};

// New moves introduced in Generation 3
static const std::unordered_map<uint16_t, const char*> GEN3_NEW_MOVES = {
    {0xFC, "Fake Out"},
    {0xFD, "Uproar"},
    {0xFE, "Stockpile"},
    {0xFF, "Spit Up"},
    {0x100, "Swallow"},
    {0x101, "Heat Wave"},
    {0x102, "Hail"},
    {0x103, "Torment"},
    {0x104, "Flatter"},
    {0x105, "Will-O-Wisp"},
    {0x106, "Memento"},
    {0x107, "Facade"},
    {0x108, "Focus Punch"},
    {0x109, "Smelling Salts"},
    {0x10A, "Follow Me"},
    {0x10B, "Nature Power"},
    {0x10C, "Charge"},
    {0x10D, "Taunt"},
    {0x10E, "Helping Hand"},
    {0x10F, "Trick"},
    {0x110, "Role Play"},
    {0x111, "Wish"},
    {0x112, "Assist"},
    {0x113, "Ingrain"},
    {0x114, "Superpower"},
    {0x115, "Magic Coat"},
    {0x116, "Recycle"},
    {0x117, "Revenge"},
    {0x118, "Brick Break"},
    {0x119, "Yawn"},
    {0x11A, "Knock Off"},
    {0x11B, "Endeavor"},
    {0x11C, "Eruption"},
    {0x11D, "Skill Swap"},
    {0x11E, "Imprison"},
    {0x11F, "Refresh"},
    {0x120, "Grudge"},
    {0x121, "Snatch"},
    {0x122, "Secret Power"},
    {0x123, "Dive"},
    {0x124, "Arm Thrust"},
    {0x125, "Camouflage"},
    {0x126, "Tail Glow"},
    {0x127, "Luster Purge"},
    {0x128, "Mist Ball"},
    {0x129, "Feather Dance"},
    {0x12A, "Teeter Dance"},
    {0x12B, "Blaze Kick"},
    {0x12C, "Mud Sport"},
    {0x12D, "Ice Ball"},
    {0x12E, "Needle Arm"},
    {0x12F, "Slack Off"},
    {0x130, "Hyper Voice"},
    {0x131, "Poison Fang"},
    {0x132, "Crush Claw"},
    {0x133, "Blast Burn"},
    {0x134, "Hydro Cannon"},
    {0x135, "Meteor Mash"},
    {0x136, "Astonish"},
    {0x137, "Weather Ball"},
    {0x138, "Aromatherapy"},
    {0x139, "Fake Tears"},
    {0x13A, "Air Cutter"},
    {0x13B, "Overheat"},
    {0x13C, "Odor Sleuth"},
    {0x13D, "Rock Tomb"},
    {0x13E, "Silver Wind"},
    {0x13F, "Metal Sound"},
    {0x140, "Grass Whistle"},
    {0x141, "Tickle"},
    {0x142, "Cosmic Power"},
    {0x143, "Water Spout"},
    {0x144, "Signal Beam"},
    {0x145, "Shadow Punch"},
    {0x146, "Extrasensory"},
    {0x147, "Sky Uppercut"},
    {0x148, "Sand Tomb"},
    {0x149, "Sheer Cold"},
    {0x14A, "Muddy Water"},
    {0x14B, "Bullet Seed"},
    {0x14C, "Aerial Ace"},
    {0x14D, "Icicle Spear"},
    {0x14E, "Iron Defense"},
    {0x14F, "Block"},
    {0x150, "Howl"},
    {0x151, "Dragon Claw"},
    {0x152, "Frenzy Plant"},
    {0x153, "Bulk Up"},
    {0x154, "Bounce"},
    {0x155, "Mud Shot"},
    {0x156, "Poison Tail"},
    {0x157, "Covet"},
    {0x158, "Volt Tackle"},
    {0x159, "Magical Leaf"},
    {0x15A, "Water Sport"},
    {0x15B, "Calm Mind"},
    {0x15C, "Leaf Blade"},
    {0x15D, "Dragon Dance"},
    {0x15E, "Rock Blast"},
    {0x15F, "Shock Wave"},
    {0x160, "Water Pulse"},
    {0x161, "Doom Desire"},
    {0x162, "Psycho Boost"},
};

// Combined maps
static const std::unordered_map<uint8_t, const char*> GEN2_MOVES = buildGenerationMap(GEN1_MOVES, GEN2_NEW_MOVES);

// Build GEN3_MOVES from GEN2_MOVES + GEN3_NEW_MOVES
// Note: GEN3_MOVES needs to handle 16-bit indices
static std::unordered_map<uint16_t, const char*> buildGen3Moves() {
    std::unordered_map<uint16_t, const char*> gen3;
    
    // Copy all Gen1 and Gen2 moves
    for (const auto& [key, value] : GEN2_MOVES) {
        gen3[key] = value;
    }
    
    // Add Gen3 new moves
    gen3.insert(GEN3_NEW_MOVES.begin(), GEN3_NEW_MOVES.end());
    
    return gen3;
}

static const std::unordered_map<uint16_t, const char*> GEN3_MOVES = buildGen3Moves();

// Helper functions
inline const char* getGen1MoveName(uint8_t move) {
    auto it = GEN1_MOVES.find(move);
    if (it != GEN1_MOVES.end()) {
        return it->second;
    }
    return nullptr;
}

inline const char* getGen2MoveName(uint8_t move) {
    auto it = GEN2_MOVES.find(move);
    if (it != GEN2_MOVES.end()) {
        return it->second;
    }
    return nullptr;
}

inline const char* getGen3MoveName(uint16_t move) {
    auto it = GEN3_MOVES.find(move);
    if (it != GEN3_MOVES.end()) {
        return it->second;
    }
    return nullptr;
}

// Generic function that can handle any generation
inline const char* getMoveName(uint16_t move, int generation) {
    switch (generation) {
        case 1:
            if (move <= 0xA5) return getGen1MoveName(static_cast<uint8_t>(move));
            break;
        case 2:
            if (move <= 0xFB) return getGen2MoveName(static_cast<uint8_t>(move));
            break;
        case 3:
            return getGen3MoveName(move);
        default:
            break;
    }
    return nullptr;
}

} // namespace PokemonMoves

#endif // MOVES_INDEX_ENG_H