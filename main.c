#include "Chip8-emulator.h"
#include <windows.h>
#include <conio.h>
#include <string.h>

// Nasconde il cursore del terminale
void hideCursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

// Mostra il cursore del terminale
void showCursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

// Mappatura dei tasti Windows ai tasti CHIP-8
void updateKeys(Chip8* chip8) {
    memset(chip8->keys, false, 16);
    
    // SINISTRA (tasto 7)
    if (GetAsyncKeyState('A') & 0x8000) chip8->keys[0x7] = true;
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) chip8->keys[0x7] = true;
    
    // DESTRA (tasto 9)
    if (GetAsyncKeyState('D') & 0x8000) chip8->keys[0x9] = true;
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) chip8->keys[0x9] = true;
    
    // SPARA (tasto 5 o 0)
    if (GetAsyncKeyState('Q') & 0x8000) chip8->keys[0x5] = true;
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) chip8->keys[0x0] = true;
    if (GetAsyncKeyState(VK_RETURN) & 0x8000) chip8->keys[0x5] = true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <rom_file>\n", argv[0]);
        return 1;
    }

    hideCursor();
    
    Chip8 chip8;
    initialize(&chip8);
    
    printf("Loading ROM: %s\n", argv[1]);
    loadRom(&chip8, argv[1]);
    
    printf("ROM loaded!\n");
    printf("Controls: A/D (move), Q/SPACE (shoot)\n");
    printf("Press ESC to exit.\n\n");
    
    Sleep(500);
    
    // Speed hack: esegui piu istruzioni per frame
    // CHIP-8 originale gira a ~500-1000 Hz, non 60 Hz!
    while (1) {
        updateKeys(&chip8);
        
        // Esegui piu istruzioni per ciclo (aumenta la velocita)
        for (int i = 0; i < 10; i++) {
            emulateCycle(&chip8);
        }
        
        updateTimers(&chip8);
        drawGraphics(&chip8);
        
        Sleep(2); // ~60fps overall
        
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }
        
        system("cls");
    }
    
    showCursor();
    printf("Exiting emulator...\n");
    return 0;
}

