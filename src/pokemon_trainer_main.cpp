//=============================================================================
//  pokemon_trainer_main.cpp
//=============================================================================

#include "pokemon_trainer/pokemon_trainer.h"
#include <iostream>
#include <cstring>

static void printUsage(const char* progName) {
    std::cerr << "Pokemon Trainer Editor" << std::endl;
    std::cerr << "Usage: " << progName << " <romfile> <game> [-o] [-j]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Arguments:" << std::endl;
    std::cerr << "  <romfile>   Path to the GBA or GBC ROM file" << std::endl;
    std::cerr << "  <game>      Game name (ruby, sapphire, emerald, firered, leafgreen, gold, silver, crystal, red, blue, yellow)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  -o          Overwrite the input ROM instead of writing to edited_files/" << std::endl;
    std::cerr << "  -j          Use Japanese trainer data offsets" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Interactive controls:" << std::endl;
    std::cerr << "  Up/Down        Navigate the trainer list or fields" << std::endl;
    std::cerr << "  Enter          Toggle detail view" << std::endl;
    std::cerr << "  T              Toggle sort between class and name" << std::endl;
    std::cerr << "  S              Activate search; type letters and press Enter to accept" << std::endl;
    std::cerr << "  Cmd/Ctrl+S     Save edits" << std::endl;
    std::cerr << "  Esc            Exit detail view or quit the program" << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }
    if (argc >= 2 && strcmp(argv[1], "-h") == 0) {
        printUsage(argv[0]);
        return 0;
    }
    const char* filename = argv[1];
    std::string game = argv[2];
    bool overwrite = false;
    bool japanese = false;
    // Parse optional flags
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            overwrite = true;
        } else if (strcmp(argv[i], "-j") == 0) {
            japanese = true;
        } else {
            std::cerr << "Unknown option: " << argv[i] << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    PokemonTrainerEditor editor;

    editor.setJapanese(japanese);
    editor.setOverwriteMode(overwrite);
    if (!editor.loadFile(filename)) {
        return 1;
    }
    if (!editor.setGame(game)) {
        return 1;
    }
    if (!editor.init()) {
        return 1;
    }
    editor.run();
    editor.cleanup();
    return 0;
}