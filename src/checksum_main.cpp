//=============================================================================
//  checksum_main.cpp
//=============================================================================

#include "checksum/checksum_calc.h"
#include "common/hex_utils.h"
#include <iostream>
#include <cstring>
#include <vector>

void printUsage(const char* progName) {
    std::cerr << "Checksum Calculator" << std::endl;
    std::cerr << "\nCalculates game-specific checksums with hardcoded addresses." << std::endl;
    std::cerr << "\nUsage:" << std::endl;
    std::cerr << "  " << progName << " <file> <game> [-j] [-w] [-o] [-p]" << std::endl;
    std::cerr << "\nArguments:" << std::endl;
    std::cerr << "  <file>        Path to the save file" << std::endl;
    std::cerr << "  <game>        Game name (see supported games below)" << std::endl;
    std::cerr << "\nOptions:" << std::endl;
    std::cerr << "  -j, --japan   Use Japanese version addresses (Gen 1 and 2 games)" << std::endl;
    std::cerr << "  -w            Write checksums to file (saves in edited_files/)" << std::endl;
    std::cerr << "  -o            Overwrite original file (requires -w)" << std::endl;
    std::cerr << "  -p            Pokemon checksum mode (Gen 3 only; combine with -w / -o to fix)" << std::endl;
    std::cerr << "\nSupported games:" << std::endl;
    std::cerr << "  red, blue, yellow, green - Pokemon Red/Blue/Yellow and Japanese Green (GB)" << std::endl;
    std::cerr << "  gold, silver             - Pokemon Gold/Silver (GBC)" << std::endl;
    std::cerr << "  crystal                  - Pokemon Crystal (GBC)" << std::endl;
    std::cerr << "  ruby, sapphire, emerald, firered, leafgreen - Pokemon Generation 3 (GBA)" << std::endl;
    std::cerr << "                  14 sections per save block (A and B)" << std::endl;
    std::cerr << "                  Each section has independent checksum" << std::endl;
    std::cerr << "\nExamples:" << std::endl;
    std::cerr << "  " << progName << " Pokemon_Red.sav red" << std::endl;
    std::cerr << "  " << progName << " Pokemon_Gold.sav gold -w" << std::endl;
    std::cerr << "  " << progName << " Pokemon_Crystal.sav crystal -w -o" << std::endl;
    std::cerr << "  " << progName << " Pokemon_Crystal_JP.sav crystal -j" << std::endl;
    std::cerr << "  " << progName << " Pokemon_Gold_JP.sav gold -j -w" << std::endl;
    std::cerr << "  " << progName << " Pokemon_Emerald.sav emerald -w" << std::endl;
    std::cerr << "  " << progName << " Pokemon_Emerald.sav emerald -p" << std::endl;
    std::cerr << "  " << progName << " Pokemon_Emerald.sav emerald -p -w" << std::endl;
    std::cerr << "  " << progName << " Pokemon_FireRed.sav firered -p -w -o" << std::endl;
}

int main(int argc, char** argv) {
    if (argc >= 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        printUsage(argv[0]);
        return 0;
    }

    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }

    const char* filename = argv[1];
    std::string game = argv[2];

    bool isJapanese = false;
    bool shouldWrite = false;
    bool shouldOverwrite = false;
    bool pokemonMode = false;

    // Parse optional flags (after <file> and <game>)
    for (int i = 3; i < argc; i++) {
        std::string flag = argv[i];
        if (flag == "-j" || flag == "--japan") {
            isJapanese = true;
        } else if (flag == "-w") {
            shouldWrite = true;
        } else if (flag == "-o") {
            shouldOverwrite = true;
        } else if (flag == "-p") {
            pokemonMode = true;
        } else {
            std::cerr << "Unknown option: " << flag << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    // Check if -o is used without -w
    if (shouldOverwrite && !shouldWrite) {
        std::cout << "Warning: -o flag requires -w flag to write checksums." << std::endl;
        std::cout << "Proceeding with checksum calculation only (no file writing)." << std::endl;
        shouldOverwrite = false;
    }

    ChecksumCalculator calc;

    if (!calc.init()) {
        return 1;
    }

    if (!calc.loadFile(filename)) {
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