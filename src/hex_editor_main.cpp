#include "hex_editor/hex_editor.h"
#include "common/hex_utils.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>

void printUsage(const char* progName) {
    std::cerr << "GBA/GB Hex Editor" << std::endl;
    std::cerr << "Usage: " << progName << " <filename> [-g grouping] [-e encoding] [-r address value ...] [-f replacefile] [-o]" << std::endl;
    std::cerr << "\nOptions:" << std::endl;
    std::cerr << "  -g grouping     Group bytes (1, 2, 4, or 8). Default: 1" << std::endl;
    std::cerr << "  -e encoding     Text encoding for decoded display:" << std::endl;
    std::cerr << "                    E1 = English Gen 1 (Pokemon Red/Blue/Yellow)" << std::endl;
    std::cerr << "                    E2 = English Gen 2 (Pokemon Gold/Silver/Crystal)" << std::endl;
    std::cerr << "                    E3 = English Gen 3 (Pokemon Fire Red/Leaf Green/Ruby/Sapphire/Emerald)" << std::endl;
    std::cerr << "                    J1 = Japanese Gen 1" << std::endl;
    std::cerr << "                    J2 = Japanese Gen 2" << std::endl;
    std::cerr << "                    J3 = Japanese Gen 3" << std::endl;
    std::cerr << "                    Default: ASCII" << std::endl;
    std::cerr << "  -r address value  Replace bytes at address with value (batch mode)" << std::endl;
    std::cerr << "                    Can specify multiple address-value pairs" << std::endl;
    std::cerr << "                    Addresses can have 0x prefix (optional)" << std::endl;
    std::cerr << "                    Values can be multiple bytes (e.g., FFD3A1)" << std::endl;
    std::cerr << "  -f filename     Read replacements from file (applied before -r)" << std::endl;
    std::cerr << "                    File format: <address> <values> (one per line)" << std::endl;
    std::cerr << "                    Lines starting with # are comments" << std::endl;
    std::cerr << "  -o              Overwrite mode: save to original file instead of edited_files/" << std::endl;
    std::cerr << "\nExamples:" << std::endl;
    std::cerr << "  " << progName << " game.gb" << std::endl;
    std::cerr << "  " << progName << " pokemon_red.gb -e E1" << std::endl;
    std::cerr << "  " << progName << " pokemon_gold.gb -e E2 -g 4" << std::endl;
    std::cerr << "  " << progName << " pokemon_green.gb -e J1" << std::endl;
    std::cerr << "  " << progName << " game.gba -r FF01 FF DC03 40" << std::endl;
    std::cerr << "  " << progName << " game.gba -g 2 -r 0x100 FFD3A1" << std::endl;
    std::cerr << "  " << progName << " game.gba -f replacements.txt" << std::endl;
    std::cerr << "  " << progName << " game.gba -f replacements.txt -r 0x100 FF -o" << std::endl;
    std::cerr << "\nInteractive controls:" << std::endl;
    std::cerr << "  Click hex      - Select byte for editing" << std::endl;
    std::cerr << "  Type hex       - Edit selected byte (auto-advance)" << std::endl;
    std::cerr << "  Arrow keys     - Navigate bytes" << std::endl;
    std::cerr << "  Tab/Shift+Tab  - Next/previous byte" << std::endl;
    std::cerr << "  Cmd/Ctrl+S     - Save to edited_files/" << std::endl;
    std::cerr << "  Cmd/Ctrl+C     - Copy selected bytes" << std::endl;
    std::cerr << "  Cmd/Ctrl+V     - Paste hex values" << std::endl;
    std::cerr << "  Cmd/Ctrl+Z     - Undo last byte edit" << std::endl;
    std::cerr << "  G              - Go to address" << std::endl;
    std::cerr << "  PgUp/PgDn      - Scroll by page" << std::endl;
    std::cerr << "  Ctrl+Home/End  - Go to start/end" << std::endl;
    std::cerr << "  Cmd/Ctrl++     - Zoom in" << std::endl;
    std::cerr << "  Cmd/Ctrl+-     - Zoom out" << std::endl;
    std::cerr << "  Cmd/Ctrl+0     - Reset zoom to 100%" << std::endl;
    std::cerr << "  Cmd/Ctrl+Scroll- Zoom with mouse wheel" << std::endl;
    std::cerr << "  Esc            - Deselect / Quit" << std::endl;
    std::cerr << "  Q              - Quit (when not editing)" << std::endl;
}

bool fileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool parseReplacementFile(const std::string& filepath, 
                         std::vector<std::pair<size_t, std::vector<unsigned char>>>& edits) {
    std::ifstream inFile(filepath);
    if (!inFile) {
        std::cerr << "Error: Could not open replacement file: " << filepath << std::endl;
        return false;
    }
    
    std::string line;
    int lineNum = 0;
    
    while (std::getline(inFile, line)) {
        lineNum++;
        
        // Trim whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        
        line = line.substr(start);
        
        // Skip comment lines
        if (line[0] == '#') continue;
        
        // Remove end-of-line comments
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        
        // Trim trailing whitespace after removing comments
        size_t end = line.find_last_not_of(" \t\r\n");
        if (end != std::string::npos) {
            line = line.substr(0, end + 1);
        }
        
        if (line.empty()) continue;
        
        // Parse line: location followed by values (space-separated)
        std::istringstream iss(line);
        std::string locationStr;
        
        if (!(iss >> locationStr)) {
            std::cerr << "Error: Invalid format at line " << lineNum 
                      << " in file " << filepath << std::endl;
            std::cerr << "Expected: <location> <values>" << std::endl;
            return false;
        }
        
        // Parse location
        size_t addr = HexUtils::parseHexAddress(locationStr);
        
        // Parse all remaining values (can be space-separated or combined)
        std::vector<unsigned char> bytes;
        std::string valueToken;
        
        while (iss >> valueToken) {
            // Each token can contain multiple bytes
            std::string clean = valueToken;
            
            // Remove 0x prefix if present
            if (clean.length() >= 2 && clean[0] == '0' && (clean[1] == 'x' || clean[1] == 'X')) {
                clean = clean.substr(2);
            }
            
            // Validate hex digits
            for (char c : clean) {
                if (!HexUtils::isHexDigit(c)) {
                    std::cerr << "Error: Invalid hex character '" << c 
                              << "' in value '" << valueToken 
                              << "' at line " << lineNum << " in file " << filepath << std::endl;
                    return false;
                }
            }
            
            // Parse hex bytes (must be even number of digits)
            if (clean.length() % 2 != 0) {
                std::cerr << "Error: Hex value '" << valueToken 
                          << "' has odd number of digits at line " << lineNum 
                          << " in file " << filepath << std::endl;
                return false;
            }
            
            // Convert pairs of hex digits to bytes
            for (size_t i = 0; i < clean.length(); i += 2) {
                std::string byteStr = clean.substr(i, 2);
                unsigned char byte = (unsigned char)std::stoul(byteStr, nullptr, 16);
                bytes.push_back(byte);
            }
        }
        
        if (bytes.empty()) {
            std::cerr << "Error: No values specified at line " << lineNum 
                      << " in file " << filepath << std::endl;
            return false;
        }
        
        edits.push_back({addr, bytes});
    }
    
    inFile.close();
    return true;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    } else if (argc >= 2 && strcmp(argv[1], "-h") == 0) {
        printUsage(argv[0]);
        return 1;
    }
    
    const char* filename = argv[1];
    bool batchMode = false;
    bool overwriteMode = false;
    int batchStartIdx = -1;
    int byteGrouping = 1;
    TextEncoding textEncoding = TextEncoding::ASCII;
    std::string replacementFile;
    
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0) {
            batchMode = true;
            batchStartIdx = i + 1;
            break;
        } else if (strcmp(argv[i], "-g") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "Error: -g flag requires a value (1, 2, 4, or 8)" << std::endl;
                return 1;
            }
            int g = std::atoi(argv[i + 1]);
            if (g != 1 && g != 2 && g != 4 && g != 8) {
                std::cerr << "Error: Invalid grouping value '" << argv[i + 1] << "'" << std::endl;
                std::cerr << "Expected values: 1, 2, 4, or 8" << std::endl;
                return 1;
            }
            byteGrouping = g;
            i++;
        } else if (strcmp(argv[i], "-e") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "Error: -e flag requires a value (E1, E2, E3, J1, J2, or J3)" << std::endl;
                return 1;
            }
            std::string encArg = argv[i + 1];
            if (encArg != "E1" && encArg != "E2" && encArg != "E3" && encArg != "J1" && encArg !=  "J2" && encArg != "J3") {
                std::cerr << "Error: Invalid encoding '" << encArg << "'" << std::endl;
                std::cerr << "Expected values: E1, E2, E3, J1, J2, or J3" << std::endl;
                return 1;
            }
            textEncoding = parseEncodingArg(encArg);
            i++;
        } else if (strcmp(argv[i], "-f") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "Error: -f flag requires a filename" << std::endl;
                return 1;
            }
            replacementFile = argv[i + 1];
            batchMode = true;
            i++;
        } else if (strcmp(argv[i], "-o") == 0) {
            overwriteMode = true;
        }
    }
    
    if (batchMode) {
        std::vector<std::pair<size_t, std::vector<unsigned char>>> edits;
        
        // First, parse replacement file if provided (-f)
        if (!replacementFile.empty()) {
            if (!parseReplacementFile(replacementFile, edits)) {
                return 1;
            }
        }
        
        // Then, parse command-line replacements (-r)
        if (batchStartIdx > 0) {
            for (int i = batchStartIdx; i + 1 < argc; i += 2) {
                // Stop if we hit another flag
                if (argv[i][0] == '-') break;
                if (i + 1 < argc && argv[i + 1][0] == '-') break;
                
                std::string addrStr = argv[i];
                std::string valueStr = argv[i + 1];
                
                size_t addr = HexUtils::parseHexAddress(addrStr);
                std::vector<unsigned char> bytes;
                
                if (!HexUtils::parseHexBytes(valueStr, bytes)) {
                    std::cerr << "Error: Invalid hex value '" << valueStr 
                              << "' (must be at least 2 hex digits and even length)" << std::endl;
                    return 1;
                }
                
                edits.push_back({addr, bytes});
            }
        }
        
        if (edits.empty()) {
            std::cerr << "Error: No replacements specified (use -f or -r)" << std::endl;
            return 1;
        }
        
        std::string baseName = HexUtils::getBaseName(filename);
        std::string outputPath = overwriteMode ? filename : ("edited_files/" + baseName);
        
        if (fileExists(outputPath)) {
            HexEditor editor;
            
            if (!editor.init()) {
                return 1;
            }
            
            editor.setByteGrouping(byteGrouping);
            editor.setTextEncoding(textEncoding);
            editor.setOverwriteMode(overwriteMode);
            
            if (!editor.loadFile(filename)) {
                return 1;
            }
            
            editor.applyBatchEdits(edits);
            editor.runBatchSaveMode();
        } else {
            std::string fileBuffer;
            size_t fileSize;
            
            if (!HexUtils::loadFileToBuffer(filename, fileBuffer, fileSize)) {
                std::cerr << "Failed to open: " << filename << std::endl;
                return 1;
            }
            
            for (const auto& edit : edits) {
                size_t addr = edit.first;
                const std::vector<unsigned char>& bytes = edit.second;
                
                for (size_t j = 0; j < bytes.size(); j++) {
                    if (addr + j < fileSize) {
                        fileBuffer[addr + j] = (char)bytes[j];
                    } else {
                        std::cerr << "Warning: Address 0x" << HexUtils::toHexString(addr + j, 8)
                                  << " is beyond file size (" << fileSize << " bytes)" << std::endl;
                    }
                }
            }
            
            if (!overwriteMode) {
                MKDIR("edited_files");
            }
            
            std::ofstream outFile(outputPath, std::ios::binary);
            if (!outFile) {
                std::cerr << "Failed to save: " << outputPath << std::endl;
                return 1;
            }
            
            outFile.write(fileBuffer.data(), fileSize);
            outFile.close();
            
            std::cout << "Saved to: " << outputPath << std::endl;
        }
        
        return 0;
    }
    
    HexEditor editor;
    
    if (!editor.init()) {
        return 1;
    }
    
    editor.setByteGrouping(byteGrouping);
    editor.setTextEncoding(textEncoding);
    editor.setOverwriteMode(overwriteMode);
    
    if (!editor.loadFile(filename)) {
        return 1;
    }
    
    editor.run();
    
    return 0;
}