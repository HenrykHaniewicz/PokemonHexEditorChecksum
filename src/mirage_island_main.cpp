#include "mirage_island/mirage_island.h"
#include "common/hex_utils.h"
#include <iostream>
#include <vector>
#include <algorithm>

void printUsage(const char* progName) {
    std::cerr << "Mirage Island Editor" << std::endl;
    std::cerr << "\nSets the Mirage Island random number to match your party Pokemon's PID." << std::endl;
    std::cerr << "\nUsage:" << std::endl;
    std::cerr << "  " << progName << " <file> <game> [-o]" << std::endl;
    std::cerr << "\nOptions:" << std::endl;
    std::cerr << "  -o            Overwrite original file (default: saves to edited_files/)" << std::endl;
    std::cerr << "\nSupported games:" << std::endl;
    std::cerr << "  ruby, sapphire, emerald - Pokemon Ruby/Sapphire/Emerald (GBA)" << std::endl;
    std::cerr << "\nNote: Mirage Island only exists in Pokemon Ruby, Sapphire, and Emerald." << std::endl;
    std::cerr << "\nExamples:" << std::endl;
    std::cerr << "  " << progName << " PokemonEmerald.sav emerald" << std::endl;
    std::cerr << "  " << progName << " PokemonRuby.sav ruby -o" << std::endl;
    std::cerr << "  " << progName << " PokemonSapphire.sav sapphire" << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }
    
    bool shouldOverwrite = false;
    std::string filename;
    std::string game;
    
    // Parse arguments
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }
    
    // Check for -o flag
    auto it = std::find(args.begin(), args.end(), "-o");
    if (it != args.end()) {
        shouldOverwrite = true;
        args.erase(it);
    }
    
    // Need at least 2 remaining arguments (file and game)
    if (args.size() < 2) {
        std::cerr << "Error: Missing file and/or game argument" << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    filename = args[0];
    game = args[1];
    
    MirageIslandEditor editor;
    
    if (!editor.init()) {
        return 1;
    }
    
    if (!editor.loadFile(filename.c_str())) {
        return 1;
    }
    
    if (!editor.setGame(game)) {
        return 1;
    }
    
    editor.setOverwriteMode(shouldOverwrite);
    
    if (!editor.execute()) {
        // Still run the window to show the error
        editor.run();
        return 1;
    }
    
    // Run the window to show results
    editor.run();
    
    return 0;
}