#ifndef TYPES_INDEX_ENG_H_H
#define TYPES_INDEX_ENG_H_H

#include <cstdint>

namespace PokemonTypes {

inline const char* getGen1TypeName(uint8_t type) {
    switch (type) {
        case 0x00: return "Normal";
        case 0x01: return "Fighting";
        case 0x02: return "Flying";
        case 0x03: return "Poison";
        case 0x04: return "Ground";
        case 0x05: return "Rock";
        case 0x06: return "Bird";
        case 0x07: return "Bug";
        case 0x08: return "Ghost";
        case 0x14: return "Fire";
        case 0x15: return "Water";
        case 0x16: return "Grass";
        case 0x17: return "Electric";
        case 0x18: return "Psychic";
        case 0x19: return "Ice";
        case 0x1A: return "Dragon";
        default: return "Unknown";
    }
}

} // namespace PokemonTypes

#endif // TYPES_INDEX_ENG_H_H