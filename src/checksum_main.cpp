#include "checksum/checksum_calc.h"
#include "common/hex_utils.h"
#include <iostream>
#include <vector>

void printUsage(const char* progName) {
    std::cerr << "Checksum Calculator" << std::endl;
    std::cerr << "\nCalculates game-specific checksums with hardcoded addresses." << std::endl;
    std::cerr << "\nUsage:" << std::endl;
    std::cerr << "  " << progName << " [-j] [-w] [-o] [-p] <file> <game>" << std::endl;
    std::cerr << "\nOptions:" << std::endl;
    std::cerr << "  -j, --japan   Use Japanese version addresses (Gen 1 and 2 games)" << std::endl;
    std::cerr << "  -w            Write checksums to file (saves in edited_files/)" << std::endl;
    std::cerr << "  -o            Overwrite original file (requires -w)" << std::endl;
    std::cerr << "  -p            Pokemon checksum mode (doesn't work with -w)" << std::endl;
    std::cerr << "\nSupported games:" << std::endl;
    std::cerr << "  red, blue, yellow, green - Pokemon Red/Blue/Yellow and Japanese Green (GB)" << std::endl;
    std::cerr << "  gold, silver             - Pokemon Gold/Silver (GBC)" << std::endl;
    std::cerr << "  crystal                  - Pokemon Crystal (GBC)" << std::endl;
    std::cerr << "  ruby, sapphire, emerald, firered, leafgreen - Pokemon Generation 3 (GBA)" << std::endl;
    std::cerr << "                  14 sections per save block (A and B)" << std::endl;
    std::cerr << "                  Each section has independent checksum" << std::endl;
    std::cerr << "\nExamples:" << std::endl;
    std::cerr << "  " << progName << " Pokemon_Red.sav red" << std::endl;
    std::cerr << "  " << progName << " -w Pokemon_Gold.sav gold" << std::endl;
    std::cerr << "  " << progName << " -w -o Pokemon_Crystal.sav crystal" << std::endl;
    std::cerr << "  " << progName << " -j Pokemon_Crystal_JP.sav crystal" << std::endl;
    std::cerr << "  " << progName << " -j -w Pokemon_Gold_JP.sav gold" << std::endl;
    std::cerr << "  " << progName << " -w Pokemon_Emerald.sav emerald" << std::endl;
    std::cerr << "  " << progName << " -p Pokemon_Emerald.sav emerald" << std::endl;
    std::cerr << "  " << progName << " Pokemon_FireRed.sav firered" << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }
    
    bool isJapanese = false;
    bool shouldWrite = false;
    bool shouldOverwrite = false;
    bool pokemonMode = false; 
    int argIndex = 1;
    
    // Parse flags
    while (argIndex < argc && argv[argIndex][0] == '-') {
        std::string flag = argv[argIndex];
        if (flag == "-j" || flag == "--japan") {
            isJapanese = true;
        } else if (flag == "-w") {
            shouldWrite = true;
        } else if (flag == "-o") {
            shouldOverwrite = true;
        } else if (flag == "-p") {
            pokemonMode = true;
        } else {
            std::cerr << "Unknown flag: " << flag << std::endl;
            printUsage(argv[0]);
            return 1;
        }
        argIndex++;
    }
    
    // Check if -o is used without -w
    if (shouldOverwrite && !shouldWrite) {
        std::cout << "Warning: -o flag requires -w flag to write checksums." << std::endl;
        std::cout << "Proceeding with checksum calculation only (no file writing)." << std::endl;
        shouldOverwrite = false;
    }
    
    // Check remaining arguments
    if (argc - argIndex < 2) {
        std::cerr << "Error: Missing file and/or game argument" << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    std::string filename = argv[argIndex];
    std::string game = argv[argIndex + 1];
    
    ChecksumCalculator calc;
    
    if (!calc.init()) {
        return 1;
    }
    
    if (!calc.loadFile(filename.c_str())) {
        return 1;
    }
    
    calc.setJapanese(isJapanese);
    calc.setWriteMode(shouldWrite);
    calc.setOverwriteMode(shouldOverwrite);
    calc.setPokemonMode(pokemonMode);
    
    if (!calc.setGame(game)) {
        return 1;
    }
    
    if (!calc.calculateChecksum()) {
        return 1;
    }
    
    calc.run();
    
    return 0;
}