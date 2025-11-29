#ifndef HEX_UTILS_H
#define HEX_UTILS_H

#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <vector>

namespace HexUtils {

inline bool isHexDigit(char c) {
    return std::isxdigit(static_cast<unsigned char>(c));
}

inline char toUpperHex(char c) {
    return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
}

inline size_t parseHexAddress(const std::string& str) {
    size_t addr = 0;
    std::string s = str;
    
    // Remove 0x prefix if present
    if (s.length() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s = s.substr(2);
    }
    
    // Remove brackets and commas
    s.erase(std::remove(s.begin(), s.end(), '['), s.end());
    s.erase(std::remove(s.begin(), s.end(), ']'), s.end());
    s.erase(std::remove(s.begin(), s.end(), ','), s.end());
    
    std::stringstream ss;
    ss << std::hex << s;
    ss >> addr;
    return addr;
}

inline bool parseHexBytes(const std::string& hexStr, std::vector<unsigned char>& bytes) {
    bytes.clear();
    std::string hs = hexStr;
    
    if (hs.length() >= 2 && hs[0] == '0' && (hs[1] == 'x' || hs[1] == 'X')) {
        hs = hs.substr(2);
    }
    
    if (hs.length() < 2 || hs.length() % 2 != 0) {
        return false;
    }
    
    for (size_t i = 0; i < hs.length(); i += 2) {
        if (!isHexDigit(hs[i]) || !isHexDigit(hs[i + 1])) {
            return false;
        }
        std::string byteStr = hs.substr(i, 2);
        unsigned char byte = (unsigned char)std::stoul(byteStr, nullptr, 16);
        bytes.push_back(byte);
    }
    
    return true;
}

inline std::string toHexString(size_t value, int width = 0) {
    std::stringstream ss;
    ss << std::hex << std::uppercase;
    if (width > 0) {
        ss << std::setfill('0') << std::setw(width);
    }
    ss << value;
    return ss.str();
}

inline std::string formatFileSize(size_t size) {
    std::stringstream ss;
    ss << size << " bytes";
    if (size >= 1024 * 1024) {
        ss << " (" << std::fixed << std::setprecision(2) 
           << (size / (1024.0 * 1024.0)) << " MB)";
    } else if (size >= 1024) {
        ss << " (" << std::fixed << std::setprecision(2) 
           << (size / 1024.0) << " KB)";
    }
    return ss.str();
}

inline std::string getBaseName(const std::string& path) {
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        return path.substr(lastSlash + 1);
    }
    return path;
}

inline bool loadFileToBuffer(const std::string& filename, std::string& buffer, size_t& fileSize) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        return false;
    }
    
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    buffer.resize(fileSize);
    file.read(&buffer[0], fileSize);
    file.close();
    
    return true;
}

} // namespace HexUtils

#endif // HEX_UTILS_H