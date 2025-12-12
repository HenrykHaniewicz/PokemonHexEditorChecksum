#include "pokemon_party/pokemon_party.h"
#include <iostream>
#include <string>
#include <cstring>

static void printUsage(const char* progName) {
    std::cerr << "Pokemon Party Editor" << std::endl;
    std::cerr << "Usage: " << progName << " <filename> <game> [-j] [-o]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Arguments:" << std::endl;
    std::cerr << "  <filename>   Path to the save file (.sav)" << std::endl;
    std::cerr << "  <game>       Game name:" << std::endl;
    std::cerr << "                 Gen 1: red, blue, yellow, green" << std::endl;
    std::cerr << "                 Gen 2: gold, silver, crystal" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  -j          Use Japanese offsets" << std::endl;
    std::cerr << "  -o          Overwrite the original file instead of writing to edited_files/" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Interactive controls:" << std::endl;
    std::cerr << "  Up/Down        Select a field" << std::endl;
    std::cerr << "  Left/Right     Switch between Pokemon" << std::endl;
    std::cerr << "  Enter          Edit the selected field" << std::endl;
    std::cerr << "  I              Type a name for Species/Moves" << std::endl;
    std::cerr << "  Ctrl/Cmd+S     Save" << std::endl;
    std::cerr << "  Q/Esc          Quit" << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }
    if (argc >= 2 && std::strcmp(argv[1], "-h") == 0) {
        printUsage(argv[0]);
        return 0;
    }
    
    const char* filename = argv[1];
    std::string game = argv[2];
    bool japanese = false;
    bool overwrite = false;
    
    // Parse optional flags
    for (int i = 3; i < argc; i++) {
        if (std::strcmp(argv[i], "-j") == 0) {
            japanese = true;
        } else if (std::strcmp(argv[i], "-o") == 0) {
            overwrite = true;
        } else {
            std::cerr << "Unknown option: " << argv[i] << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    PokemonPartyEditor editor;
    editor.setJapanese(japanese);
    editor.setOverwriteMode(overwrite);
    
    if (!editor.loadFile(filename)) {
        return 1;
    }
    
    if (!editor.setGame(game)) {
        return 1;
    }
    
    // Run the GUI
    if (!editor.init()) {
        return 1;
    }
    
    editor.run();
    editor.cleanup();
    return 0;
}