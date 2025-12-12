#ifndef DATA_UTILS_H
#define DATA_UTILS_H

#include <string>
#include <vector>

namespace DataUtils {

inline uint8_t readU8(const std::string& buffer, size_t offset) {
    return static_cast<uint8_t>(buffer[offset]);
}

inline uint16_t readU16LE(const std::string& buffer, size_t offset) {
    return static_cast<uint16_t>(readU8(buffer, offset)) |
           (static_cast<uint16_t>(readU8(buffer, offset + 1)) << 8);
}

inline uint32_t readU32LE(const std::string& buffer, size_t offset) {
    return static_cast<uint32_t>(readU8(buffer, offset)) |
           (static_cast<uint32_t>(readU8(buffer, offset + 1)) << 8) |
           (static_cast<uint32_t>(readU8(buffer, offset + 2)) << 16) |
           (static_cast<uint32_t>(readU8(buffer, offset + 3)) << 24);
}

inline void writeU8(std::string& buffer, size_t offset, uint8_t value) {
    buffer[offset] = static_cast<char>(value);
}

inline void writeU16LE(std::string& buffer, size_t offset, uint16_t value) {
    buffer[offset] = static_cast<char>(value & 0xFF);
    buffer[offset + 1] = static_cast<char>((value >> 8) & 0xFF);
}

inline uint16_t readU16BE(const std::string& buffer, size_t offset) {
    return (static_cast<uint16_t>(readU8(buffer, offset)) << 8) |
           static_cast<uint16_t>(readU8(buffer, offset + 1));
}

inline uint32_t readU32BE(const std::string& buffer, size_t offset) {
    return (static_cast<uint32_t>(readU8(buffer, offset)) << 24) |
           (static_cast<uint32_t>(readU8(buffer, offset + 1)) << 16) |
           (static_cast<uint32_t>(readU8(buffer, offset + 2)) << 8) |
           static_cast<uint32_t>(readU8(buffer, offset + 3));
}

inline void writeU16BE(std::string& buffer, size_t offset, uint16_t value) {
    buffer[offset]     = static_cast<char>((value >> 8) & 0xFF);
    buffer[offset + 1] = static_cast<char>(value & 0xFF);
}

inline void writeU32BE(std::string& buffer, size_t offset, uint32_t value) {
    buffer[offset]     = static_cast<char>((value >> 24) & 0xFF);
    buffer[offset + 1] = static_cast<char>((value >> 16) & 0xFF);
    buffer[offset + 2] = static_cast<char>((value >> 8) & 0xFF);
    buffer[offset + 3] = static_cast<char>(value & 0xFF);
}

} // namespace DataUtils


#endif // DATA_UTILS_H